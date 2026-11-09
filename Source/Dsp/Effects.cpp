#include "Effects.h"

#include <cmath>
#include <vector>

namespace knobz {

namespace {
constexpr float twoPi = 6.28318530718f;

float smoothstep(float x) {
    x = juce::jlimit(0.0f, 1.0f, x);
    return x * x * (3.0f - 2.0f * x);
}

float onePoleCoeff(double sampleRate, double hz) {
    return (float) (1.0 - std::exp(-twoPi * hz / sampleRate));
}
} // namespace

// ---------------------------------------------------------------------------
float kickCurve(int mode, float x) {
    x = x - std::floor(x);
    switch (mode) {
        case 0: { // CLASSIC: drops on the beat, comes back fast then eases in (compressor release)
            const float t = juce::jmin(1.0f, x / 0.55f);
            return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
        }
        case 1: // SMOOTH: a soft sine breath, no hard edge anywhere
            return 0.5f - 0.5f * std::cos(twoPi * x);
        case 2: // HARD: fully off for a moment, then snaps back
            if (x < 0.16f) return 0.0f;
            return smoothstep((x - 0.16f) / 0.07f);
        case 3: // LONG: slow S-shaped return across almost the whole division
            return smoothstep(x / 0.92f);
        case 4: // CHOP: rhythmic gate, off for the first half
            return x < 0.5f ? 0.0f : 1.0f;
        default:
            return 1.0f;
    }
}

// ---------------------------------------------------------------------------
void AutoGain::prepare(double sampleRate) {
    coeff = 1.0 / (0.3 * sampleRate);
    reset();
}

void AutoGain::reset() {
    dryMs = wetMs = 0.0;
    gain = 1.0f;
}

void AutoGain::apply(const juce::AudioBuffer<float>& dry, juce::AudioBuffer<float>& wet, int numChannels, int numSamples) {
    double d = 0.0, w = 0.0;
    for (int c = 0; c < numChannels; ++c) {
        const float* a = dry.getReadPointer(c);
        const float* b = wet.getReadPointer(c);
        for (int i = 0; i < numSamples; ++i) {
            d += (double) a[i] * a[i];
            w += (double) b[i] * b[i];
        }
    }
    const double n = (double) juce::jmax(1, numChannels * numSamples);
    const double k = 1.0 - std::exp(-coeff * numSamples);
    dryMs += k * (d / n - dryMs);
    wetMs += k * (w / n - wetMs);

    float target = gain;
    if (dryMs > 1.0e-9 && wetMs > 1.0e-12)
        target = juce::jlimit(0.15f, 3.5f, (float) std::sqrt(dryMs / wetMs));

    for (int c = 0; c < numChannels; ++c)
        wet.applyGainRamp(c, 0, numSamples, gain, target);
    gain = target;
}

// ---------------------------------------------------------------------------
// Base for the two saturators: 4x / 2x oversampling, latency-matched dry path,
// automatic loudness match and MIX.
class Oversampled : public Effect {
public:
    explicit Oversampled(size_t factorLog2)
        : os(2, factorLog2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true) {}

    void prepare(double sampleRate, int maxBlock) override {
        sr = sampleRate;
        os.initProcessing((size_t) maxBlock);
        latency = (int) std::lround(os.getLatencyInSamples());
        dry.setSize(2, maxBlock);
        delayLine.assign(2, std::vector<float>((size_t) latency + 1, 0.0f));
        autoGain.prepare(sampleRate);
        prepareInner(sampleRate * (double) os.getOversamplingFactor());
        reset();
    }

    void reset() override {
        os.reset();
        for (auto& d : delayLine) std::fill(d.begin(), d.end(), 0.0f);
        writePos = 0;
        autoGain.reset();
        resetInner();
    }

    int latencySamples() const override { return latency; }

    void process(juce::AudioBuffer<float>& buffer, const Settings& s, const HostTime&) override {
        const int ch = juce::jmin(2, buffer.getNumChannels());
        const int n = buffer.getNumSamples();
        if (ch == 0 || n == 0) return;

        // dry copy, delayed by the oversampling latency so MIX never combs
        const int len = (int) delayLine[0].size();
        for (int c = 0; c < ch; ++c) {
            const float* in = buffer.getReadPointer(c);
            float* out = dry.getWritePointer(c);
            auto& d = delayLine[(size_t) c];
            int pos = writePos;
            for (int i = 0; i < n; ++i) {
                d[(size_t) pos] = in[i];
                int rp = pos - latency;
                if (rp < 0) rp += len;
                out[i] = d[(size_t) rp];
                if (++pos == len) pos = 0;
            }
        }
        writePos = (writePos + n) % len;

        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), (size_t) ch, (size_t) n);
        auto up = os.processSamplesUp(block);
        shape(up, ch, s);
        os.processSamplesDown(block);
        post(buffer, ch, n, s);

        autoGain.apply(dry, buffer, ch, n);

        const float mix = juce::jlimit(0.0f, 1.0f, s.extra);
        for (int c = 0; c < ch; ++c) {
            float* w = buffer.getWritePointer(c);
            const float* d = dry.getReadPointer(c);
            for (int i = 0; i < n; ++i)
                w[i] = softCeiling(d[i] + mix * (w[i] - d[i]));
        }
    }

protected:
    virtual void prepareInner(double oversampledRate) = 0;
    virtual void resetInner() = 0;
    virtual void shape(juce::dsp::AudioBlock<float>& up, int ch, const Settings&) = 0;
    virtual void post(juce::AudioBuffer<float>&, int, int, const Settings&) {}

    double sr = 44100.0;

private:
    juce::dsp::Oversampling<float> os;
    juce::AudioBuffer<float> dry;
    std::vector<std::vector<float>> delayLine;
    int writePos = 0;
    int latency = 0;
    AutoGain autoGain;
};

// ---------------------------------------------------------------------------
// BITE: saturation. Modes TAPE, TUBE, DRIVE, CLIP.
class Bite final : public Oversampled {
public:
    Bite() : Oversampled(2) {}

protected:
    void prepareInner(double) override {}
    void resetInner() override {
        lp[0] = lp[1] = 0.0f;
        dcX[0] = dcX[1] = dcY[0] = dcY[1] = 0.0f;
    }

    void shape(juce::dsp::AudioBlock<float>& up, int ch, const Settings& s) override {
        const float a = juce::jlimit(0.0f, 1.0f, s.amount);
        const int n = (int) up.getNumSamples();
        const float g = juce::Decibels::decibelsToGain(a * (s.mode == 3 ? 20.0f : 24.0f));
        for (int c = 0; c < ch; ++c) {
            float* x = up.getChannelPointer((size_t) c);
            switch (s.mode) {
                case 0: // TAPE: symmetric, soft
                    for (int i = 0; i < n; ++i) x[i] = std::tanh(g * x[i] * 0.9f) / 0.9f;
                    break;
                case 1: { // TUBE: biased tanh, even harmonics
                    const float bias = 0.18f + 0.22f * a;
                    const float off = std::tanh(bias);
                    for (int i = 0; i < n; ++i) x[i] = std::tanh(g * x[i] + bias) - off;
                    break;
                }
                case 2: // DRIVE: harder knee
                    for (int i = 0; i < n; ++i) x[i] = 0.63662f * std::atan(1.6f * g * x[i]);
                    break;
                default: // CLIP: cubic soft clip
                    for (int i = 0; i < n; ++i) {
                        const float v = g * x[i];
                        x[i] = std::abs(v) >= 1.0f ? std::copysign(1.0f, v) : 1.5f * (v - v * v * v / 3.0f);
                        x[i] = juce::jlimit(-1.0f, 1.0f, x[i]);
                    }
                    break;
            }
        }
    }

    void post(juce::AudioBuffer<float>& buffer, int ch, int n, const Settings& s) override {
        const float a = juce::jlimit(0.0f, 1.0f, s.amount);
        float cutoff = 20000.0f;
        if (s.mode == 0) cutoff = 20000.0f - 11500.0f * a;   // tape rolls off the top
        if (s.mode == 2) cutoff = 14000.0f - 3000.0f * a;
        cutoff = juce::jmin(cutoff, (float) (sr * 0.45));
        const float k = onePoleCoeff(sr, cutoff);
        const float r = 1.0f - (float) (twoPi * 12.0 / sr);   // DC blocker for TUBE
        for (int c = 0; c < ch; ++c) {
            float* x = buffer.getWritePointer(c);
            for (int i = 0; i < n; ++i) {
                if (s.mode == 1) {
                    const float y = x[i] - dcX[c] + r * dcY[c];
                    dcX[c] = x[i];
                    dcY[c] = y;
                    x[i] = y;
                }
                if (s.mode == 0 || s.mode == 2) {
                    lp[c] += k * (x[i] - lp[c]);
                    x[i] = lp[c];
                }
            }
        }
    }

private:
    float lp[2] {}, dcX[2] {}, dcY[2] {};
};

// ---------------------------------------------------------------------------
// PHAT: 3-band saturation + glue compression. Modes WARM, PUNCH, CRUSH, AIR.
class Phat final : public Oversampled {
public:
    Phat() : Oversampled(1) {}

protected:
    struct Recipe {
        float drive[3];   // dB per band (low, mid, high) at full knob
        float gain[3];    // dB per band at full knob
        float ratio, thresholdDb, attackMs, releaseMs;
    };

    static const Recipe& recipe(int mode) {
        static const Recipe r[4] {
            { { 14.0f, 4.0f, 0.0f },   { 4.0f, 0.0f, -1.0f }, 2.5f, -20.0f, 30.0f, 200.0f },  // WARM
            { { 8.0f, 6.0f, 2.0f },    { 2.0f, 1.0f, 0.0f },  4.0f, -22.0f, 18.0f, 90.0f },   // PUNCH
            { { 20.0f, 18.0f, 12.0f }, { 2.0f, 2.0f, 1.0f },  8.0f, -26.0f, 2.0f, 60.0f },    // CRUSH
            { { 0.0f, 2.0f, 14.0f },   { 0.0f, 0.0f, 6.0f },  1.5f, -18.0f, 10.0f, 120.0f },  // AIR
        };
        return r[juce::jlimit(0, 3, mode)];
    }

    void prepareInner(double rate) override {
        osRate = rate;
        juce::dsp::ProcessSpec spec { rate, 4096, 2 };
        for (auto* f : { &lowLp, &lowHp, &highLp, &highHp }) f->prepare(spec);
        lowLp.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        lowHp.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        highLp.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        highHp.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        lowLp.setCutoffFrequency(180.0f);
        lowHp.setCutoffFrequency(180.0f);
        highLp.setCutoffFrequency(3500.0f);
        highHp.setCutoffFrequency(3500.0f);
    }

    void resetInner() override {
        for (auto* f : { &lowLp, &lowHp, &highLp, &highHp }) f->reset();
        env = 0.0f;
    }

    static float sat(float x, float driveDb) {
        if (driveDb < 0.05f) return x;
        const float g = juce::Decibels::decibelsToGain(driveDb);
        return std::tanh(g * x) / std::sqrt(g);
    }

    void shape(juce::dsp::AudioBlock<float>& up, int ch, const Settings& s) override {
        const float a = juce::jlimit(0.0f, 1.0f, s.amount);
        const auto& r = recipe(s.mode);
        const int n = (int) up.getNumSamples();
        float drive[3], gain[3];
        for (int b = 0; b < 3; ++b) {
            drive[b] = r.drive[b] * a;
            gain[b] = juce::Decibels::decibelsToGain(r.gain[b] * a);
        }
        const float ratio = 1.0f + (r.ratio - 1.0f) * a;
        const float makeupDb = -r.thresholdDb * (1.0f - 1.0f / ratio) * 0.5f;
        const float att = (float) std::exp(-1.0 / (osRate * r.attackMs * 0.001));
        const float rel = (float) std::exp(-1.0 / (osRate * r.releaseMs * 0.001));

        for (int i = 0; i < n; ++i) {
            float peak = 0.0f;
            for (int c = 0; c < ch; ++c) {
                float* x = up.getChannelPointer((size_t) c);
                const float in = x[i];
                const float low = lowLp.processSample(c, in);
                const float rest = lowHp.processSample(c, in);
                const float mid = highLp.processSample(c, rest);
                const float high = highHp.processSample(c, rest);
                float y = gain[0] * sat(low, drive[0]) + gain[1] * sat(mid, drive[1]);
                y += s.mode == 3 ? gain[2] * (high + sat(high, drive[2]) * 0.6f)   // AIR adds harmonics on top
                                 : gain[2] * sat(high, drive[2]);
                x[i] = y;
                peak = juce::jmax(peak, std::abs(y));
            }
            // stereo-linked compressor
            env = peak > env ? att * env + (1.0f - att) * peak : rel * env + (1.0f - rel) * peak;
            const float levelDb = juce::Decibels::gainToDecibels(env, -120.0f);
            const float over = levelDb - r.thresholdDb;
            const float grDb = over > 0.0f ? over * (1.0f - 1.0f / ratio) : 0.0f;
            const float gr = juce::Decibels::decibelsToGain(makeupDb - grDb);
            for (int c = 0; c < ch; ++c) up.getChannelPointer((size_t) c)[i] *= gr;
        }
    }

private:
    double osRate = 88200.0;
    juce::dsp::LinkwitzRileyFilter<float> lowLp, lowHp, highLp, highHp;
    float env = 0.0f;
};

// ---------------------------------------------------------------------------
// KICKSIDE: tempo-synced volume pump.
class Kickside final : public Effect {
public:
    void prepare(double sampleRate, int) override {
        sr = sampleRate;
        smooth = (float) (1.0 - std::exp(-1.0 / (0.0012 * sampleRate)));
        reset();
    }
    void reset() override {
        phase = 0.0;
        gain = 1.0f;
    }

    void process(juce::AudioBuffer<float>& buffer, const Settings& s, const HostTime& t) override {
        const int ch = buffer.getNumChannels();
        const int n = buffer.getNumSamples();
        const int tempo = juce::jlimit(0, 4, (int) std::lround(s.extra));
        const double beats = tempoBeats[(size_t) tempo];
        const double bpm = t.bpm > 1.0 ? t.bpm : 120.0;
        if (t.playing && t.hasPpq) phase = t.ppq / beats - std::floor(t.ppq / beats);
        const double step = bpm / 60.0 / sr / beats;
        const float depth = juce::jlimit(0.0f, 1.0f, s.amount);

        for (int i = 0; i < n; ++i) {
            const float v = kickCurve(s.mode, (float) phase);
            const float target = 1.0f - depth * (1.0f - v);
            gain += smooth * (target - gain);
            for (int c = 0; c < ch; ++c) {
                float* x = buffer.getWritePointer(c);
                x[i] = softCeiling(x[i] * gain);
            }
            phase += step;
            if (phase >= 1.0) phase -= 1.0;
        }
        uiPhase.store((float) phase, std::memory_order_relaxed);
    }

    float meter() const override { return uiPhase.load(std::memory_order_relaxed); }

private:
    double sr = 44100.0, phase = 0.0;
    float gain = 1.0f, smooth = 0.02f;
    std::atomic<float> uiPhase { 0.0f };
};

// ---------------------------------------------------------------------------
// WIDE: works only on the side signal, so L+R is never changed: always mono-safe.
class Wide final : public Effect {
public:
    void prepare(double sampleRate, int) override {
        sr = sampleRate;
        delay.assign((size_t) (sampleRate * 0.05) + 4, 0.0f);
        juce::dsp::ProcessSpec spec { sampleRate, 4096, 1 };
        for (auto* f : { &sideHp, &doubleHp, &splitHp, &airHp }) f->prepare(spec);
        splitHp.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 2500.0f);
        airHp.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 3000.0f);
        currentCut = -1.0f;
        reset();
    }

    void reset() override {
        std::fill(delay.begin(), delay.end(), 0.0f);
        writePos = 0;
        lfo = 0.0;
        for (auto* f : { &sideHp, &doubleHp, &splitHp, &airHp }) f->reset();
        corr = 1.0f;
    }

    void process(juce::AudioBuffer<float>& buffer, const Settings& s, const HostTime&) override {
        if (buffer.getNumChannels() < 2) return;
        const int n = buffer.getNumSamples();
        const float cut = juce::jlimit(20.0f, 400.0f, s.extra);
        if (std::abs(cut - currentCut) > 0.5f) {
            currentCut = cut;
            sideHp.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, cut);
            doubleHp.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, juce::jmax(cut, 150.0f));
        }
        const float w = juce::jlimit(0.0f, 1.0f, s.amount);
        float* L = buffer.getWritePointer(0);
        float* R = buffer.getWritePointer(1);
        const int len = (int) delay.size();
        const double lfoStep = 0.37 / sr;
        double sLR = 0.0, sLL = 0.0, sRR = 0.0;

        for (int i = 0; i < n; ++i) {
            const float m = 0.5f * (L[i] + R[i]);
            const float sd = 0.5f * (L[i] - R[i]);
            delay[(size_t) writePos] = m;

            float side = sideHp.processSample(sd);
            float out;
            if (s.mode == 0) {            // CLEAN
                out = side * (1.0f + 1.6f * w);
            } else if (s.mode == 1) {     // DOUBLE: a delayed, slowly moving copy of the mid, only in the side
                const float dMs = 18.0f + 0.4f * (float) std::sin(6.2831853 * lfo);
                const float d = doubleHp.processSample(read(dMs));
                out = side * (1.0f + 0.6f * w) + d * 0.6f * w;
            } else {                      // SPREAD: opens the highs more
                const float high = splitHp.processSample(side);
                const float low = side - high;
                const float air = airHp.processSample(read(7.0f));
                out = low * (1.0f + 0.8f * w) + high * (1.0f + 2.2f * w) + air * 0.35f * w;
            }
            L[i] = softCeiling(m + out);
            R[i] = softCeiling(m - out);
            sLR += (double) L[i] * R[i];
            sLL += (double) L[i] * L[i];
            sRR += (double) R[i] * R[i];
            if (++writePos == len) writePos = 0;
            lfo += lfoStep;
            if (lfo >= 1.0) lfo -= 1.0;
        }
        if (sLL > 1.0e-9 && sRR > 1.0e-9) {
            const float c = (float) (sLR / std::sqrt(sLL * sRR));
            corr += 0.2f * (c - corr);
        }
        uiCorr.store(corr, std::memory_order_relaxed);
    }

    float meter() const override { return uiCorr.load(std::memory_order_relaxed); }

private:
    float read(float ms) const {
        const int len = (int) delay.size();
        const float pos = (float) writePos - ms * 0.001f * (float) sr;
        float p = pos < 0.0f ? pos + (float) len : pos;
        const int i0 = (int) p;
        const float f = p - (float) i0;
        const float a = delay[(size_t) (i0 % len)];
        const float b = delay[(size_t) ((i0 + 1) % len)];
        return a + f * (b - a);
    }

    double sr = 44100.0, lfo = 0.0;
    std::vector<float> delay;
    int writePos = 0;
    juce::dsp::IIR::Filter<float> sideHp, doubleHp, splitHp, airHp;
    float currentCut = -1.0f, corr = 1.0f;
    std::atomic<float> uiCorr { 1.0f };
};

// ---------------------------------------------------------------------------
// GASP has no live processing (it renders files): it passes audio through.
class PassThrough final : public Effect {
public:
    void prepare(double, int) override {}
    void reset() override {}
    void process(juce::AudioBuffer<float>&, const Settings&, const HostTime&) override {}
};

std::unique_ptr<Effect> makeEffect(PluginId id) {
    switch (id) {
        case PluginId::bite: return std::make_unique<Bite>();
        case PluginId::phat: return std::make_unique<Phat>();
        case PluginId::kickside: return std::make_unique<Kickside>();
        case PluginId::wide: return std::make_unique<Wide>();
        case PluginId::gasp: return std::make_unique<PassThrough>();
    }
    return std::make_unique<PassThrough>();
}

} // namespace knobz
