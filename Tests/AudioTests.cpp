// Audio checks for every Mini Knobz plugin, mode and knob position.
// KNOBZ_SNAPSHOT_DIR=<dir> also writes PNGs of every plugin window (run under xvfb-run on Linux).
// KNOBZ_DEMO_DIR=<dir> writes before/after WAVs to listen to.

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <cstdio>
#include <random>
#include "Common/Editor.h"
#include "Common/Processor.h"
#include "Dsp/Effects.h"
#include "Dsp/Gasp.h"

using namespace knobz;

namespace {
int failures = 0;
int checks = 0;

void expect(bool ok, const juce::String& what) {
    ++checks;
    if (!ok) {
        ++failures;
        std::printf("FAIL: %s\n", what.toRawUTF8());
    }
}

constexpr double sr = 48000.0;

// a little beat: kick every beat, a saw chord, some hats
juce::AudioBuffer<float> testSignal(double seconds, float peak) {
    const int n = (int) (seconds * sr);
    juce::AudioBuffer<float> b(2, n);
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    const double beat = sr * 0.5;
    for (int i = 0; i < n; ++i) {
        const double t = i / sr;
        const double inBeat = std::fmod((double) i, beat) / sr;
        const float kick = (float) (std::sin(2.0 * juce::MathConstants<double>::pi * (55.0 + 90.0 * std::exp(-inBeat * 30.0)) * inBeat)
                                    * std::exp(-inBeat * 7.0));
        float chord = 0.0f;
        for (double f : { 220.0, 277.18, 329.63 }) chord += (float) (2.0 * std::fmod(t * f, 1.0) - 1.0) * 0.12f;
        const float hat = noise(rng) * (float) std::exp(-std::fmod(inBeat, 0.125) * 60.0) * 0.15f;
        const float pan = (float) std::sin(t * 1.3) * 0.3f;
        b.setSample(0, i, kick * 0.8f + chord * (1.0f - pan) + hat);
        b.setSample(1, i, kick * 0.8f + chord * (1.0f + pan) - hat * 0.5f);
    }
    const float mag = b.getMagnitude(0, n);
    b.applyGain(peak / mag);
    return b;
}

float rms(const juce::AudioBuffer<float>& b, int start, int len) {
    double s = 0.0;
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = start; i < start + len; ++i) s += (double) b.getSample(c, i) * b.getSample(c, i);
    return (float) std::sqrt(s / (b.getNumChannels() * len));
}

bool finite(const juce::AudioBuffer<float>& b) {
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (!std::isfinite(b.getSample(c, i))) return false;
    return true;
}

juce::AudioBuffer<float> run(PluginId id, const Settings& s, const juce::AudioBuffer<float>& in, int* latency = nullptr) {
    auto fx = makeEffect(id);
    const int block = 256;
    fx->prepare(sr, block);
    if (latency) *latency = fx->latencySamples();
    juce::AudioBuffer<float> out(in);
    HostTime t;
    t.bpm = 120.0;
    t.playing = true;
    t.hasPpq = true;
    for (int start = 0; start < out.getNumSamples(); start += block) {
        const int n = juce::jmin(block, out.getNumSamples() - start);
        juce::AudioBuffer<float> view(out.getArrayOfWritePointers(), 2, start, n);
        t.ppq = start / sr * 2.0;
        fx->process(view, s, t);
    }
    return out;
}

void testLiveEffects() {
    const auto input = testSignal(3.0, 0.9f);
    const float inRms = rms(input, (int) sr, (int) sr * 2);
    for (auto id : { PluginId::bite, PluginId::phat, PluginId::kickside, PluginId::wide }) {
        const auto& sp = spec(id);
        for (int mode = 0; mode < (int) sp.modes.size(); ++mode) {
            for (float amount : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f }) {
                Settings s;
                s.amount = amount;
                s.mode = mode;
                s.extra = id == PluginId::kickside ? (float) tempoDefault : (id == PluginId::wide ? 120.0f : 1.0f);
                const auto out = run(id, s, input);
                const juce::String tag = juce::String(sp.name) + " " + sp.modes[(size_t) mode] + " " + juce::String(amount);
                expect(finite(out), tag + ": NaN/inf");
                const float peak = out.getMagnitude(0, out.getNumSamples());
                expect(peak <= 1.0f, tag + ": peak " + juce::String(peak));
                const float outRms = rms(out, (int) sr, (int) sr * 2);
                expect(outRms > inRms * 0.15f, tag + ": too quiet " + juce::String(outRms / inRms));
                if (id == PluginId::bite || id == PluginId::phat) {
                    const float db = juce::Decibels::gainToDecibels(outRms / inRms);
                    expect(std::abs(db) < 3.5f, tag + ": loudness changed by " + juce::String(db, 2) + " dB");
                }
            }
        }
    }

    // Bite / Phat at MIX 0 = input delayed by the reported latency (dry path is aligned)
    for (auto id : { PluginId::bite, PluginId::phat }) {
        Settings s;
        s.amount = 1.0f;
        s.extra = 0.0f;
        int lat = 0;
        const auto out = run(id, s, input, &lat);
        float worst = 0.0f;
        for (int i = lat; i < out.getNumSamples(); ++i)
            worst = juce::jmax(worst, std::abs(out.getSample(0, i) - juce::jlimit(-1.0f, 1.0f, input.getSample(0, i - lat))));
        expect(worst < 0.002f, juce::String(spec(id).name) + ": dry path misaligned, diff " + juce::String(worst));
    }

    // Wide never changes the mono sum
    {
        const auto quiet = testSignal(2.0, 0.4f);
        for (int mode = 0; mode < 3; ++mode) {
            Settings s;
            s.amount = 1.0f;
            s.mode = mode;
            s.extra = 120.0f;
            const auto out = run(PluginId::wide, s, quiet);
            float worst = 0.0f;
            for (int i = 0; i < out.getNumSamples(); ++i) {
                const float a = out.getSample(0, i) + out.getSample(1, i);
                const float b = quiet.getSample(0, i) + quiet.getSample(1, i);
                worst = juce::jmax(worst, std::abs(a - b));
            }
            expect(worst < 1.0e-3f, "WIDE " + juce::String(spec(PluginId::wide).modes[(size_t) mode]) + ": mono sum changed " + juce::String(worst));
        }
    }

    // Kickside: knob at 0 is transparent, at 100 the beat ducks hard
    {
        Settings s;
        s.amount = 0.0f;
        s.extra = (float) tempoDefault;
        const auto out = run(PluginId::kickside, s, input);
        float worst = 0.0f;
        for (int i = 0; i < out.getNumSamples(); ++i) worst = juce::jmax(worst, std::abs(out.getSample(0, i) - input.getSample(0, i)));
        expect(worst < 1.0e-5f, "KICKSIDE at 0 is not transparent");
        juce::AudioBuffer<float> dc(2, (int) sr);
        for (int c = 0; c < 2; ++c) for (int i = 0; i < dc.getNumSamples(); ++i) dc.setSample(c, i, 0.5f);
        s.amount = 1.0f;
        const auto ducked = run(PluginId::kickside, s, dc);
        const int beat = (int) (sr * 0.5);
        expect(std::abs(ducked.getSample(0, beat + 200)) < 0.06f, "KICKSIDE CLASSIC does not duck on the beat");
        expect(std::abs(ducked.getSample(0, beat + beat - 200)) > 0.45f, "KICKSIDE CLASSIC does not come back");
    }

    for (int m = 0; m < 5; ++m) {
        expect(kickCurve(m, 0.001f) < 0.05f, "curve " + juce::String(m) + " not low on the beat");
        expect(kickCurve(m, 0.75f) > 0.4f || m == 1, "curve " + juce::String(m) + " not back up");
    }
}

void testGasp() {
    juce::AudioBuffer<float> clip(2, (int) (sr * 0.6));
    for (int i = 0; i < clip.getNumSamples(); ++i) {
        const float env = (float) std::exp(-i / sr * 4.0);
        const float v = 0.7f * env * (float) std::sin(2.0 * juce::MathConstants<double>::pi * 330.0 * i / sr);
        clip.setSample(0, i, v);
        clip.setSample(1, i, v * 0.9f);
    }
    for (int style = 0; style < 4; ++style) {
        for (float size : { 0.0f, 0.3f, 0.5f, 0.7f, 1.0f }) {
            for (bool original : { false, true }) {
                GaspRequest r;
                r.sampleRate = sr;
                r.bpm = 120.0;
                r.size = size;
                r.style = style;
                r.includeOriginal = original;
                const auto res = renderGasp(clip, r);
                const juce::String tag = "GASP " + juce::String(spec(PluginId::gasp).modes[(size_t) style]) + " " + juce::String(size)
                                         + (original ? " +orig" : "");
                const int expected = (int) std::lround(gaspBeats[(size_t) gaspLengthIndex(size)] * 0.5 * sr);
                expect(res.swellSamples == expected, tag + ": swell length " + juce::String(res.swellSamples) + " vs " + juce::String(expected));
                expect(res.audio.getNumSamples() == expected + (original ? clip.getNumSamples() : 0), tag + ": total length");
                expect(finite(res.audio), tag + ": NaN");
                const float peak = res.audio.getMagnitude(0, res.swellSamples);
                expect(peak <= 0.9f && peak > 0.05f, tag + ": swell peak " + juce::String(peak));
                const int tenth = expected / 10;
                expect(rms(res.audio, expected - tenth, tenth) > rms(res.audio, 0, tenth) * 2.0f, tag + ": does not swell up");
            }
        }
    }
}

void writeWav(const juce::File& f, const juce::AudioBuffer<float>& b) {
    f.deleteFile();
    std::unique_ptr<juce::OutputStream> os(f.createOutputStream());
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> w(wav.createWriterFor(os.get(), sr, 2, 24, {}, 0));
    os.release();
    w->writeFromAudioSampleBuffer(b, 0, b.getNumSamples());
}

void writeDemos(const juce::File& dir) {
    dir.createDirectory();
    const auto in = testSignal(8.0, 0.8f);
    writeWav(dir.getChildFile("0 original.wav"), in);
    struct D { PluginId id; int mode; float amount; float extra; };
    for (auto d : { D { PluginId::bite, 0, 0.6f, 1.0f }, D { PluginId::bite, 3, 0.8f, 1.0f }, D { PluginId::phat, 0, 0.7f, 1.0f },
                    D { PluginId::phat, 2, 0.6f, 1.0f }, D { PluginId::kickside, 0, 0.8f, 2.0f }, D { PluginId::kickside, 4, 0.9f, 3.0f },
                    D { PluginId::wide, 1, 0.8f, 120.0f } }) {
        Settings s { d.amount, d.mode, d.extra };
        writeWav(dir.getChildFile(juce::String(spec(d.id).name) + " " + spec(d.id).modes[(size_t) d.mode] + ".wav"), run(d.id, s, in));
    }
    // GASP: a chord stab, then the swell that leads into it
    juce::AudioBuffer<float> stab(2, (int) (sr * 1.6));
    stab.clear();
    for (double f0 : { 261.63, 329.63, 392.0, 493.88 })
        for (int h = 1; h <= 8; ++h)
            for (int i = 0; i < stab.getNumSamples(); ++i) {
                const double t = i / sr;
                const float v = (float) (0.09 / h * std::exp(-t * (2.2 + 0.4 * h)) * std::sin(2.0 * juce::MathConstants<double>::pi * f0 * h * t)
                                         * juce::jmin(1.0, t * 400.0));
                stab.addSample(0, i, v);
                stab.addSample(1, i, v * (h % 2 ? 1.0f : 0.85f));
            }
    writeWav(dir.getChildFile("GASP original.wav"), stab);
    for (int style : { 0, 1, 3 }) {
        GaspRequest r;
        r.sampleRate = sr;
        r.bpm = 100.0;
        r.size = 0.7f;
        r.style = style;
        r.includeOriginal = true;
        writeWav(dir.getChildFile("GASP " + juce::String(spec(PluginId::gasp).modes[(size_t) style]) + ".wav"), renderGasp(stab, r).audio);
    }
}

// ---- editor snapshots -------------------------------------------------------
void snap(juce::Component& c, const juce::File& f) {
    auto img = c.createComponentSnapshot(c.getLocalBounds(), true, 2.0f);
    f.deleteFile();
    juce::FileOutputStream os(f);
    juce::PNGImageFormat().writeImageToStream(img, os);
}

juce::Component* findChild(juce::Component& root, const std::function<bool(juce::Component&)>& pred) {
    for (auto* c : root.getChildren()) {
        if (pred(*c)) return c;
        if (auto* r = findChild(*c, pred)) return r;
    }
    return nullptr;
}

void writeSnapshots(const juce::File& dir) {
    dir.createDirectory();
    for (int i = 0; i < 5; ++i) {
        Processor proc((PluginId) i);
        proc.prepareToPlay(sr, 512);
        if ((PluginId) i == PluginId::gasp) {
            const auto clipFile = dir.getChildFile("vocal hey.wav");
            juce::AudioBuffer<float> clip(2, (int) (sr * 0.5));
            for (int n = 0; n < clip.getNumSamples(); ++n) {
                const float v = 0.6f * (float) std::exp(-n / sr * 5.0) * (float) std::sin(2.0 * juce::MathConstants<double>::pi * 300.0 * n / sr);
                clip.setSample(0, n, v);
                clip.setSample(1, n, v);
            }
            {
                std::unique_ptr<Editor> empty(dynamic_cast<Editor*>(proc.createEditor()));
                snap(*empty, dir.getChildFile("gasp-vazio.png"));
            }
            writeWav(clipFile, clip);
            proc.loadClip(clipFile);
        }
        std::unique_ptr<Editor> ed(dynamic_cast<Editor*>(proc.createEditor()));
        const juce::String name = juce::String(spec((PluginId) i).name).toLowerCase();
        snap(*ed, dir.getChildFile(name + ".png"));
        if ((PluginId) i == PluginId::kickside) {
            ed->getSurface();
            if (auto* infoButton = findChild(*ed, [](juce::Component& c) { return c.getTitle() == juce::String("O que este plugin faz"); }))
                if (auto* b = dynamic_cast<juce::Button*>(infoButton)) if (b->onClick) b->onClick();
                snap(*ed, dir.getChildFile("kickside-ajuda.png"));
        }
    }
}
} // namespace

int main() {
    juce::ScopedJuceInitialiser_GUI gui;
    testLiveEffects();
    testGasp();
    if (auto* d = std::getenv("KNOBZ_DEMO_DIR")) writeDemos(juce::File(d));
    if (auto* d = std::getenv("KNOBZ_SNAPSHOT_DIR")) writeSnapshots(juce::File(d));
    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
