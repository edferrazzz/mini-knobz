#pragma once

// GASP: offline reverse-reverb renderer. Takes a short clip and returns the swell
// that leads into it, ending exactly where the clip starts.

#include <juce_audio_basics/juce_audio_basics.h>

namespace knobz {

struct GaspRequest {
    double sampleRate = 44100.0;
    double bpm = 120.0;
    float size = 0.5f;          // knob 0..1: swell length (snapped) and reverb size
    int style = 0;              // 0 SMOOTH, 1 STUTTER, 2 TAPE, 3 RISE
    bool includeOriginal = false;
};

struct GaspResult {
    juce::AudioBuffer<float> audio;   // stereo
    int swellSamples = 0;             // where the original starts (when included)
};

GaspResult renderGasp(const juce::AudioBuffer<float>& clip, const GaspRequest&);

} // namespace knobz
