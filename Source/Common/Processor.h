#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <memory>
#include "Dsp/Effects.h"
#include "Dsp/Gasp.h"
#include "Dsp/Spec.h"

namespace knobz {

class Processor final : public juce::AudioProcessor {
public:
    explicit Processor(PluginId);
    ~Processor() override;

    const Spec& getSpec() const { return spec(id); }
    PluginId getId() const { return id; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return getSpec().productName; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    juce::AudioProcessorValueTreeState state;

    float meter() const { return effect ? effect->meter() : 0.0f; }
    bool isBypassed() const { return bypassParam->get(); }

    // ---- GASP (message thread) ----
    bool loadClip(const juce::File&);
    bool hasClip() const { return clip != nullptr; }
    juce::String clipName() const { return clipFile.getFileNameWithoutExtension(); }
    void updateGaspIfNeeded();             // re-renders when the knob / style / bpm changed
    std::shared_ptr<const GaspResult> gaspResult() const;
    juce::File writeGaspWav(const juce::File& target = {}) const;
    void togglePreview();
    bool isPreviewing() const { return previewing.load(); }
    float previewProgress() const;
    double effectiveBpm() const;
    bool hostHasTempo() const { return hostBpmValid.load(); }
    bool includeOriginal = false;
    double manualBpm = 120.0;
    double clipSampleRate = 44100.0;

    static juce::String versionString() { return JucePlugin_VersionString; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout(PluginId);
    Settings readSettings() const;

    PluginId id;
    std::unique_ptr<Effect> effect;
    juce::AudioParameterBool* bypassParam = nullptr;
    juce::RangedAudioParameter* amountParam = nullptr;
    juce::RangedAudioParameter* modeParam = nullptr;
    juce::RangedAudioParameter* extraParam = nullptr;
    int maxBlock = 512;
    double sampleRate = 44100.0;
    juce::AudioBuffer<float> chunkBuffer;

    std::atomic<double> hostBpm { 120.0 };
    std::atomic<bool> hostBpmValid { false };

    // GASP
    juce::File clipFile;
    std::unique_ptr<juce::AudioBuffer<float>> clip;
    std::shared_ptr<const GaspResult> rendered;
    mutable juce::SpinLock renderLock;
    struct RenderKey { int length = -1; int style = -1; bool original = false; double bpm = 0; float size = -1; } lastKey;
    std::atomic<bool> previewing { false };
    double previewPos = 0.0;
    std::atomic<float> previewFraction { 0.0f };
};

} // namespace knobz
