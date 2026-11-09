#include "Gasp.h"
#include "Spec.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <vector>

namespace knobz {

namespace {
float clamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }

// reads a reversed tail backwards from its end with a varying speed (tape / pitch rise)
void varispeed(const std::vector<float>& rev, std::vector<float>& out, int n,
               float (*speedAt)(float u)) {
    const int len = (int) rev.size();
    double pos = 0.0;   // distance from the end of rev
    for (int k = 0; k < n; ++k) {
        const int o = n - 1 - k;
        const float u = (float) o / (float) juce::jmax(1, n - 1);
        const double p = (double) (len - 1) - pos;
        if (p <= 0.0) { out[(size_t) o] = 0.0f; continue; }
        const int i0 = (int) p;
        const float f = (float) (p - i0);
        const float a = rev[(size_t) i0];
        const float b = rev[(size_t) juce::jmin(len - 1, i0 + 1)];
        out[(size_t) o] = a + f * (b - a);
        pos += speedAt(u);
    }
}

float tapeSpeed(float u) { return 0.12f + 0.88f * u * u; }            // spins up at the end
float riseSpeed(float u) { return std::pow(2.0f, -(1.0f - u)); }      // one octave up to the note
} // namespace

GaspResult renderGasp(const juce::AudioBuffer<float>& clip, const GaspRequest& req) {
    GaspResult result;
    const int clipLen = clip.getNumSamples();
    const int clipCh = clip.getNumChannels();
    if (clipLen == 0 || clipCh == 0) return result;

    const double sr = req.sampleRate > 0.0 ? req.sampleRate : 44100.0;
    const double bpm = juce::jlimit(40.0, 240.0, req.bpm > 1.0 ? req.bpm : 120.0);
    const float size = clamp01(req.size);
    const double beats = gaspBeats[(size_t) gaspLengthIndex(size)];
    const int n = juce::jmax(64, (int) std::lround(beats * 60.0 / bpm * sr));

    // 1) reverb of the clip, 100 % wet, with room to ring out
    const int tailLen = clipLen + n + (int) (0.5 * sr);
    juce::Reverb reverb;
    juce::Reverb::Parameters p;
    p.roomSize = 0.62f + 0.36f * size;
    p.damping = 0.45f - 0.2f * size;
    p.wetLevel = 1.0f;
    p.dryLevel = 0.0f;
    p.width = 1.0f;
    reverb.setSampleRate(sr);
    reverb.setParameters(p);

    std::vector<float> tail[2];
    for (auto& t : tail) t.assign((size_t) tailLen, 0.0f);
    for (int i = 0; i < clipLen; ++i) {
        tail[0][(size_t) i] = clip.getSample(0, i);
        tail[1][(size_t) i] = clip.getSample(juce::jmin(1, clipCh - 1), i);
    }
    const int chunk = 512;
    for (int i = 0; i < tailLen; i += chunk) {
        const int len = juce::jmin(chunk, tailLen - i);
        reverb.processStereo(tail[0].data() + i, tail[1].data() + i, len);
    }

    // 2) reverse, 3) take the swell that leads into the clip start, 4) style
    float peakClip = clip.getMagnitude(0, clipLen);
    std::vector<float> swell[2];
    for (int c = 0; c < 2; ++c) {
        std::vector<float> rev(tail[(size_t) c].rbegin(), tail[(size_t) c].rend());
        auto& s = swell[(size_t) c];
        s.assign((size_t) n, 0.0f);
        if (req.style == 2 || req.style == 3) {
            varispeed(rev, s, n, req.style == 2 ? &tapeSpeed : &riseSpeed);
        } else {
            for (int i = 0; i < n; ++i) {
                const int src = tailLen - n + i;
                s[(size_t) i] = src >= 0 ? rev[(size_t) src] : 0.0f;
            }
        }
    }

    const double sixteenth = 0.25 * 60.0 / bpm * sr;
    const float ramp = (float) (0.002 * sr);
    float lpState[2] {};
    for (int i = 0; i < n; ++i) {
        const float u = (float) i / (float) n;
        float g = std::pow(clamp01(u / 0.35f), 2.0f);      // fade in from silence
        if (req.style == 1 && u < 0.78f) {                  // STUTTER: 1/16 gate, opens fully near the hit
            const double inStep = std::fmod((double) i, sixteenth);
            const double open = sixteenth * 0.5;
            float gate = inStep < open ? 1.0f : 0.0f;
            if (inStep < ramp) gate = (float) (inStep / ramp);
            else if (inStep >= open && inStep < open + ramp) gate = 1.0f - (float) ((inStep - open) / ramp);
            g *= gate;
        }
        float cutoff = 20000.0f;
        if (req.style == 2) cutoff = 900.0f + 17000.0f * u * u;   // TAPE: dull at the start
        if (req.style == 3) cutoff = 400.0f + 19000.0f * u;       // RISE: filter opens with the pitch
        const float k = (float) (1.0 - std::exp(-6.2831853 * juce::jmin((double) cutoff, sr * 0.45) / sr));
        for (int c = 0; c < 2; ++c) {
            float x = swell[(size_t) c][(size_t) i] * g;
            if (req.style >= 2) {
                lpState[c] += k * (x - lpState[c]);
                x = lpState[c];
            }
            swell[(size_t) c][(size_t) i] = x;
        }
    }

    // 5) level: a little under the clip's own peak, never above -1 dBFS
    float peak = 0.0f;
    for (auto& s : swell) for (float v : s) peak = juce::jmax(peak, std::abs(v));
    const float target = juce::jmin(0.89f, juce::jmax(0.05f, peakClip * 0.85f));
    const float norm = peak > 1.0e-6f ? target / peak : 0.0f;

    const int total = n + (req.includeOriginal ? clipLen : 0);
    result.audio.setSize(2, total);
    result.audio.clear();
    for (int c = 0; c < 2; ++c) {
        float* out = result.audio.getWritePointer(c);
        for (int i = 0; i < n; ++i) out[i] = swell[(size_t) c][(size_t) i] * norm;
        if (req.includeOriginal) {
            const int src = juce::jmin(c, clipCh - 1);
            for (int i = 0; i < clipLen; ++i) out[n + i] = juce::jlimit(-1.0f, 1.0f, clip.getSample(src, i));
        }
    }
    result.swellSamples = n;
    return result;
}

} // namespace knobz
