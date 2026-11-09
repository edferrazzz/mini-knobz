#pragma once

// Everything that defines one Mini Knobz plugin: name, colour, options, help text.
// Shared by the processors, the editor and the tests.

#include <array>
#include <cstdint>
#include <vector>

namespace knobz {

enum class PluginId { bite = 0, phat = 1, kickside = 2, wide = 3, gasp = 4 };
enum class RowType { slider, tempo, wave };

struct Tip {
    const char* key;
    const char* text;  // UTF-8
};

struct Spec {
    PluginId id;
    const char* name;          // shown big on the window
    const char* productName;   // DAW / installer name
    uint32_t colour;           // 0xAARRGGBB
    const char* knobLabel;
    float knobDefault;         // 0..100
    std::vector<const char*> modes;
    RowType rowType;
    const char* rowLabel;
    const char* lead;          // first line of the help popup (UTF-8)
    std::vector<Tip> tips;
};

const Spec& spec(PluginId id);

// Kickside tempo divisions, in beats (quarter notes).
constexpr std::array<const char*, 5> tempoLabels { "1/1", "1/2", "1/4", "1/8", "1/16" };
constexpr std::array<double, 5> tempoBeats { 4.0, 2.0, 1.0, 0.5, 0.25 };
constexpr int tempoDefault = 2;

// Gasp swell lengths chosen by the knob, in beats.
constexpr std::array<double, 5> gaspBeats { 0.5, 1.0, 2.0, 4.0, 8.0 };
constexpr std::array<const char*, 5> gaspLabels { "1/8", "1/4", "1/2", "1 BAR", "2 BARS" };
inline int gaspLengthIndex(float amount01) {
    int i = (int) (amount01 * 5.0f);
    return i < 0 ? 0 : (i > 4 ? 4 : i);
}

} // namespace knobz
