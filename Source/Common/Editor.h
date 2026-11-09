#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include "Processor.h"

namespace knobz {

class Surface;

// The plugin window: one 300 x 480 surface (design units) shown at 125 %.
class Editor final : public juce::AudioProcessorEditor {
public:
    explicit Editor(Processor&);
    ~Editor() override;
    void resized() override;
    void paint(juce::Graphics&) override;

    static constexpr int designW = 300, designH = 480;
    static constexpr float scale = 1.25f;

    Surface& getSurface() { return *surface; }

private:
    std::unique_ptr<Surface> surface;
};

} // namespace knobz
