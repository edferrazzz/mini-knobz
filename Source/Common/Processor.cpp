#include "Processor.h"
#include "Editor.h"

namespace knobz {

namespace {
const juce::ParameterID amountId { "amount", 1 };
const juce::ParameterID modeId { "mode", 1 };
const juce::ParameterID extraId { "extra", 1 };
const juce::ParameterID bypassId { "bypass", 1 };

juce::StringArray toArray(const std::vector<const char*>& v) {
    juce::StringArray a;
    for (auto* s : v) a.add(s);
    return a;
}
} // namespace

Processor::Processor(PluginId pluginId)
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "KNOBZ", createLayout(pluginId)),
      id(pluginId),
      effect(makeEffect(pluginId)) {
    bypassParam = dynamic_cast<juce::AudioParameterBool*>(state.getParameter("bypass"));
    amountParam = state.getParameter("amount");
    modeParam = state.getParameter("mode");
    extraParam = state.getParameter("extra");
}

Processor::~Processor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout Processor::createLayout(PluginId id) {
    const auto& s = spec(id);
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        amountId, s.knobLabel, juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), s.knobDefault,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterChoice>(modeId, "Mode", toArray(s.modes), 0));
    switch (id) {
        case PluginId::bite:
        case PluginId::phat:
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                extraId, "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f,
                juce::AudioParameterFloatAttributes().withLabel("%")));
            break;
        case PluginId::kickside: {
            juce::StringArray t;
            for (auto* l : tempoLabels) t.add(l);
            layout.add(std::make_unique<juce::AudioParameterChoice>(extraId, "Tempo", t, tempoDefault));
            break;
        }
        case PluginId::wide: {
            juce::NormalisableRange<float> r(20.0f, 300.0f, 1.0f);
            r.setSkewForCentre(110.0f);
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                extraId, "Mono Bass", r, 120.0f, juce::AudioParameterFloatAttributes().withLabel("Hz")));
            break;
        }
        case PluginId::gasp:
            break;
    }
    layout.add(std::make_unique<juce::AudioParameterBool>(bypassId, "Bypass", false));
    return layout;
}

bool Processor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono()) return false;
    if (id == PluginId::wide && out != juce::AudioChannelSet::stereo()) return false;
    return layouts.getMainInputChannelSet() == out;
}

void Processor::prepareToPlay(double sr, int samplesPerBlock) {
    sampleRate = sr;
    maxBlock = juce::jmax(32, samplesPerBlock);
    effect->prepare(sr, maxBlock);
    setLatencySamples(effect->latencySamples());
    chunkBuffer.setSize(2, maxBlock, false, false, true);
}

Settings Processor::readSettings() const {
    Settings s;
    s.amount = amountParam->convertFrom0to1(amountParam->getValue()) / 100.0f;
    s.mode = (int) std::lround(modeParam->convertFrom0to1(modeParam->getValue()));
    if (extraParam != nullptr) {
        const float v = extraParam->convertFrom0to1(extraParam->getValue());
        s.extra = (id == PluginId::bite || id == PluginId::phat) ? v / 100.0f : v;
    }
    return s;
}

void Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    const int total = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c) buffer.clear(c, 0, total);

    HostTime time;
    if (auto* ph = getPlayHead()) {
        if (auto pos = ph->getPosition()) {
            if (auto bpm = pos->getBpm()) {
                time.bpm = *bpm;
                hostBpm.store(*bpm);
                hostBpmValid.store(true);
            }
            if (auto ppq = pos->getPpqPosition()) {
                time.ppq = *ppq;
                time.hasPpq = true;
            }
            time.playing = pos->getIsPlaying();
        }
    }

    Settings s = readSettings();
    const bool bypassed = bypassParam->get();
    const bool saturator = id == PluginId::bite || id == PluginId::phat;
    if (bypassed) {
        if (saturator) s.extra = 0.0f;      // keeps the latency-matched dry path
        else if (id == PluginId::kickside) s.amount = 0.0f;
    }

    if (!(bypassed && (id == PluginId::wide || id == PluginId::gasp))) {
        const int ch = buffer.getNumChannels();
        for (int start = 0; start < total; start += maxBlock) {
            const int n = juce::jmin(maxBlock, total - start);
            juce::AudioBuffer<float> view(buffer.getArrayOfWritePointers(), ch, start, n);
            HostTime t = time;
            if (t.hasPpq) t.ppq += (double) start / sampleRate * t.bpm / 60.0;
            effect->process(view, s, t);
        }
    }

    // GASP preview playback
    if (id == PluginId::gasp && previewing.load()) {
        const juce::SpinLock::ScopedTryLockType lock(renderLock);
        if (lock.isLocked() && rendered != nullptr && rendered->audio.getNumSamples() > 0) {
            const auto& a = rendered->audio;
            const double step = clipSampleRate / sampleRate;
            const int len = a.getNumSamples();
            for (int i = 0; i < total; ++i) {
                const int i0 = (int) previewPos;
                if (i0 >= len - 1) {
                    previewing.store(false);
                    previewPos = 0.0;
                    break;
                }
                const float f = (float) (previewPos - i0);
                for (int c = 0; c < buffer.getNumChannels(); ++c) {
                    const int src = juce::jmin(c, a.getNumChannels() - 1);
                    const float v = a.getSample(src, i0) + f * (a.getSample(src, i0 + 1) - a.getSample(src, i0));
                    buffer.addSample(c, i, v);
                }
                previewPos += step;
            }
            previewFraction.store((float) (previewPos / juce::jmax(1, len)));
        }
    }
}

// ---------------------------------------------------------------------------
double Processor::effectiveBpm() const {
    return hostBpmValid.load() ? hostBpm.load() : manualBpm;
}

bool Processor::loadClip(const juce::File& file) {
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(file));
    if (reader == nullptr) return false;
    const auto maxLen = (juce::int64) (reader->sampleRate * 20.0);   // 20 s is plenty for a phrase
    const int len = (int) juce::jmin(reader->lengthInSamples, maxLen);
    if (len <= 0) return false;
    auto buf = std::make_unique<juce::AudioBuffer<float>>((int) juce::jmin(2u, reader->numChannels), len);
    reader->read(buf.get(), 0, len, 0, true, buf->getNumChannels() > 1);
    clipSampleRate = reader->sampleRate;
    clip = std::move(buf);
    clipFile = file;
    lastKey = {};
    previewing.store(false);
    updateGaspIfNeeded();
    return true;
}

void Processor::updateGaspIfNeeded() {
    if (clip == nullptr) return;
    const Settings s = readSettings();
    RenderKey key { gaspLengthIndex(s.amount), s.mode, includeOriginal, effectiveBpm(), s.amount };
    if (key.length == lastKey.length && key.style == lastKey.style && key.original == lastKey.original
        && std::abs(key.bpm - lastKey.bpm) < 0.01 && std::abs(key.size - lastKey.size) < 0.015f)
        return;
    lastKey = key;
    GaspRequest req;
    req.sampleRate = clipSampleRate;
    req.bpm = key.bpm;
    req.size = s.amount;
    req.style = s.mode;
    req.includeOriginal = includeOriginal;
    auto result = std::make_shared<GaspResult>(renderGasp(*clip, req));
    const juce::SpinLock::ScopedLockType lock(renderLock);
    rendered = std::move(result);
    previewPos = 0.0;
}

std::shared_ptr<const GaspResult> Processor::gaspResult() const {
    const juce::SpinLock::ScopedLockType lock(renderLock);
    return rendered;
}

void Processor::togglePreview() {
    if (previewing.load()) {
        previewing.store(false);
        return;
    }
    {
        const juce::SpinLock::ScopedLockType lock(renderLock);
        previewPos = 0.0;
    }
    previewFraction.store(0.0f);
    previewing.store(rendered != nullptr);
}

float Processor::previewProgress() const { return previewFraction.load(); }

juce::File Processor::writeGaspWav(const juce::File& target) const {
    auto result = gaspResult();
    if (result == nullptr || result->audio.getNumSamples() == 0) return {};
    const Settings s = readSettings();
    juce::String name = clipName() + " GASP " + spec(id).modes[(size_t) juce::jlimit(0, 3, s.mode)] + " "
                        + juce::String(gaspLabels[(size_t) gaspLengthIndex(s.amount)]).replaceCharacter('/', '-')
                        + " " + juce::String((int) std::lround(effectiveBpm())) + "bpm.wav";
    juce::File file = target;
    if (file == juce::File()) {
        auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("Knobz Gasp");
        dir.createDirectory();
        file = dir.getChildFile(juce::File::createLegalFileName(name));
    }
    file.deleteFile();
    std::unique_ptr<juce::OutputStream> out(file.createOutputStream());
    if (out == nullptr) return {};
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> w(wav.createWriterFor(out.get(), clipSampleRate, 2, 24, {}, 0));
    if (w == nullptr) return {};
    out.release();
    w->writeFromAudioSampleBuffer(result->audio, 0, result->audio.getNumSamples());
    return file;
}

// ---------------------------------------------------------------------------
void Processor::getStateInformation(juce::MemoryBlock& dest) {
    auto tree = state.copyState();
    tree.setProperty("gaspFile", clipFile.getFullPathName(), nullptr);
    tree.setProperty("gaspOriginal", includeOriginal, nullptr);
    tree.setProperty("gaspBpm", manualBpm, nullptr);
    if (auto xml = tree.createXml()) copyXmlToBinary(*xml, dest);
}

void Processor::setStateInformation(const void* data, int size) {
    if (auto xml = getXmlFromBinary(data, size)) {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (!tree.isValid()) return;
        includeOriginal = (bool) tree.getProperty("gaspOriginal", false);
        manualBpm = (double) tree.getProperty("gaspBpm", 120.0);
        const juce::String path = tree.getProperty("gaspFile", "").toString();
        state.replaceState(tree);
        if (id == PluginId::gasp && path.isNotEmpty() && juce::File::isAbsolutePath(path)) {
            const juce::File f(path);
            if (f.existsAsFile()) loadClip(f);
        }
    }
}

juce::AudioProcessorEditor* Processor::createEditor() { return new Editor(*this); }

} // namespace knobz

// One plugin per build target: KNOBZ_PLUGIN selects which.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new knobz::Processor((knobz::PluginId) KNOBZ_PLUGIN);
}
