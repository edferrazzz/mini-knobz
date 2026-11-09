#pragma once

// Real-time DSP of the four live Mini Knobz (Bite, Phat, Kickside, Wide).
// Pure audio code: no UI, no parameters tree, so the tests can drive it directly.

#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>
#include "Spec.h"

namespace knobz {

struct Settings {
    float amount = 0.5f;   // knob, 0..1
    int mode = 0;          // option index
    float extra = 1.0f;    // MIX 0..1 / MONO BASS Hz / tempo index (as float)
};

struct HostTime {
    double bpm = 120.0;
    double ppq = 0.0;      // position in quarter notes at the start of the block
    bool hasPpq = false;
    bool playing = false;
};

class Effect {
public:
    virtual ~Effect() = default;
    virtual void prepare(double sampleRate, int maxBlock) = 0;
    virtual void reset() = 0;
    virtual void process(juce::AudioBuffer<float>& buffer, const Settings&, const HostTime&) = 0;
    virtual int latencySamples() const { return 0; }
    // Value for the editor: Kickside = position in the division (0..1), Wide = correlation (-1..1).
    virtual float meter() const { return 0.0f; }
};

std::unique_ptr<Effect> makeEffect(PluginId id);

// Kickside volume curves. x = position inside one division (0..1), result = volume (0..1).
// The editor draws its icons from this same function, so the drawing is the sound.
float kickCurve(int mode, float x);

// Shared helpers -------------------------------------------------------------

// Matches the loudness of the processed signal to the dry one (slow RMS follower).
class AutoGain {
public:
    void prepare(double sampleRate);
    void reset();
    // Measures dry and wet and scales wet (with a ramp) so both have the same loudness.
    void apply(const juce::AudioBuffer<float>& dry, juce::AudioBuffer<float>& wet, int numChannels, int numSamples);
private:
    double coeff = 0.0;
    double dryMs = 0.0, wetMs = 0.0;
    float gain = 1.0f;
};

// Soft ceiling: transparent below 0.89 (-1 dBFS), never above 1.0.
inline float softCeiling(float x) {
    const float a = std::abs(x);
    if (a <= 0.89f) return x;
    const float over = (a - 0.89f) / 0.11f;
    return std::copysign(0.89f + 0.11f * std::tanh(over), x);
}

} // namespace knobz
