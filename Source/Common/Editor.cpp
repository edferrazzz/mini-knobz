#include "Editor.h"
#include "UpdateChecker.h"
#include "BinaryData.h"

#include <cmath>
#include <cstring>

namespace knobz {

// ===========================================================================
// Look: colours, fonts, small drawing helpers
// ===========================================================================
namespace look {
const juce::Colour bgTop { 0xff121318 }, bgBottom { 0xff08080b };
const juce::Colour wellFill { 0xff060608 }, wellLine { 0xff1b1c23 };
const juce::Colour chip { 0xff111217 }, chipLine { 0xff262833 };
const juce::Colour text { 0xffecedf3 }, textSoft { 0xffc9cbd8 }, textDim { 0xff8b8ea0 };
const juce::Colour off { 0xff4a4c58 };

juce::Typeface::Ptr face(const char* data, int size) {
    return juce::Typeface::createSystemTypefaceFor(data, (size_t) size);
}
juce::Font spaceBold(float h) {
    static auto tf = face(BinaryData::SpaceGroteskBold_ttf, BinaryData::SpaceGroteskBold_ttfSize);
    return juce::Font(juce::FontOptions(tf).withPointHeight(h));
}
juce::Font spaceMedium(float h) {
    static auto tf = face(BinaryData::SpaceGroteskMedium_ttf, BinaryData::SpaceGroteskMedium_ttfSize);
    return juce::Font(juce::FontOptions(tf).withPointHeight(h));
}
juce::Font interBold(float h) {
    static auto tf = face(BinaryData::InterBold_ttf, BinaryData::InterBold_ttfSize);
    return juce::Font(juce::FontOptions(tf).withPointHeight(h));
}
juce::Font interRegular(float h) {
    static auto tf = face(BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize);
    return juce::Font(juce::FontOptions(tf).withPointHeight(h));
}

void tracked(juce::Graphics& g, const juce::String& s, juce::Font f, float tracking, juce::Rectangle<float> r,
             juce::Justification j, juce::Colour c) {
    g.setColour(c);
    g.setFont(f.withExtraKerningFactor(tracking));
    g.drawText(s, r, j, false);
}

void well(juce::Graphics& g, juce::Rectangle<float> r, float radius) {
    g.setColour(look::wellFill);
    g.fillRoundedRectangle(r, radius);
    g.setColour(look::wellLine);
    g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
}

// The Kickside curve, drawn from the same function the audio uses.
// Two divisions are shown so you see the drop on each beat.
void drawCurve(juce::Graphics& g, juce::Rectangle<float> r, int mode, juce::Colour c, bool selected, float dotPhase) {
    constexpr int steps = 260;
    constexpr float cycles = 2.0f;
    // faint beat markers: where the kick lands
    for (int b = 0; b < (int) cycles; ++b) {
        const float x = r.getX() + r.getWidth() * (float) b / cycles;
        g.setColour(c.withAlpha(selected ? 0.35f : 0.18f));
        g.fillRoundedRectangle(x - 0.6f, r.getBottom() + 2.0f, 1.2f, 2.5f, 0.6f);
    }
    juce::Path line, area;
    auto pointAt = [&](float u) {
        const float v = kickCurve(mode, std::fmod(u * cycles, 1.0f));
        return juce::Point<float>(r.getX() + u * r.getWidth(), r.getBottom() - v * r.getHeight());
    };
    area.startNewSubPath(r.getX(), r.getBottom());
    for (int i = 0; i <= steps; ++i) {
        const float u = (float) i / steps * 0.9999f;
        auto pt = pointAt(u);
        if (i == 0) line.startNewSubPath(pt); else line.lineTo(pt);
        area.lineTo(pt);
    }
    area.lineTo(r.getRight(), r.getBottom());
    area.closeSubPath();
    g.setGradientFill(juce::ColourGradient(c.withAlpha(selected ? 0.42f : 0.14f), r.getX(), r.getY(),
                                           c.withAlpha(0.0f), r.getX(), r.getBottom(), false));
    g.fillPath(area);
    if (selected) {   // soft glow under the selected curve
        g.setColour(c.withAlpha(0.25f));
        g.strokePath(line, juce::PathStrokeType(3.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour(c);
    g.strokePath(line, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (dotPhase >= 0.0f) {
        auto pt = pointAt(dotPhase);
        g.setColour(c.withAlpha(0.35f));
        g.fillEllipse(juce::Rectangle<float>(8.0f, 8.0f).withCentre(pt));
        g.setColour(juce::Colours::white);
        g.fillEllipse(juce::Rectangle<float>(3.6f, 3.6f).withCentre(pt));
    }
}

void drawGear(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c) {
    const auto b = r.withSizeKeepingCentre(15.0f, 15.0f);
    juce::Path p;
    p.startNewSubPath(b.getX() + 0.5f, b.getY() + 4.5f);  p.lineTo(b.getRight() - 0.5f, b.getY() + 4.5f);
    p.startNewSubPath(b.getX() + 0.5f, b.getBottom() - 4.5f);  p.lineTo(b.getRight() - 0.5f, b.getBottom() - 4.5f);
    g.setColour(c);
    g.strokePath(p, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    for (auto pt : { juce::Point<float>(b.getX() + 5.0f, b.getY() + 4.5f), juce::Point<float>(b.getRight() - 5.0f, b.getBottom() - 4.5f) }) {
        g.setColour(look::chip);
        g.fillEllipse(juce::Rectangle<float>(5.4f, 5.4f).withCentre(pt));
        g.setColour(c);
        g.drawEllipse(juce::Rectangle<float>(5.4f, 5.4f).withCentre(pt), 1.6f);
    }
}

void drawClose(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c) {
    const auto b = r.withSizeKeepingCentre(10.0f, 10.0f);
    juce::Path p;
    p.startNewSubPath(b.getTopLeft()); p.lineTo(b.getBottomRight());
    p.startNewSubPath(b.getTopRight()); p.lineTo(b.getBottomLeft());
    g.setColour(c);
    g.strokePath(p, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
} // namespace look

// ===========================================================================
// Small widgets
// ===========================================================================
class IconButton final : public juce::Button {
public:
    enum Kind { info, gear, close };
    explicit IconButton(Kind k) : juce::Button("icon"), kind(k) { setMouseCursor(juce::MouseCursor::PointingHandCursor); }
    void paintButton(juce::Graphics& g, bool over, bool down) override {
        auto r = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(down ? look::chipLine : look::chip);
        g.fillRoundedRectangle(r, 10.0f);
        g.setColour(over ? juce::Colour(0xff3a3d4b) : look::chipLine);
        g.drawRoundedRectangle(r, 10.0f, 1.0f);
        const auto c = over ? look::text : juce::Colour(0xffa3a6b8);
        if (kind == info) look::tracked(g, "i", look::spaceBold(14.0f), 0.0f, r, juce::Justification::centred, c);
        else if (kind == gear) look::drawGear(g, r, c);
        else look::drawClose(g, r, c);
        if (badge) {
            g.setColour(juce::Colour(0xffffd23f));
            g.fillEllipse(r.getRight() - 9.0f, r.getY() + 3.0f, 6.0f, 6.0f);
        }
    }
    bool badge = false;
private:
    Kind kind;
};

class PillButton final : public juce::Button {
public:
    PillButton() : juce::Button("pill") { setMouseCursor(juce::MouseCursor::PointingHandCursor); }
    void paintButton(juce::Graphics& g, bool over, bool down) override {
        auto r = getLocalBounds().toFloat().reduced(0.5f);
        const bool on = getToggleState() || filled;
        g.setColour(on ? accent.withAlpha(down ? 0.3f : 0.18f) : (down ? look::chipLine : look::chip));
        g.fillRoundedRectangle(r, r.getHeight() * 0.5f);
        g.setColour(on ? accent.withAlpha(0.6f) : (over ? juce::Colour(0xff3a3d4b) : look::chipLine));
        g.drawRoundedRectangle(r, r.getHeight() * 0.5f, 1.0f);
        look::tracked(g, label, look::interBold(10.5f), 0.06f, r, juce::Justification::centred, on ? accent : look::textSoft);
    }
    juce::String label;
    juce::Colour accent { 0xffffffff };
    bool filled = false;
};

class PowerButton final : public juce::ToggleButton {
public:
    PowerButton() { setMouseCursor(juce::MouseCursor::PointingHandCursor); setTitle("Bypass"); }
    void paintButton(juce::Graphics& g, bool over, bool) override {
        auto r = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(look::chip);
        g.fillEllipse(r);
        g.setColour(over ? juce::Colour(0xff3a3d4b) : look::chipLine);
        g.drawEllipse(r, 1.0f);
        const auto c = getToggleState() ? juce::Colour(0xff5a5c68) : accent;
        auto b = r.withSizeKeepingCentre(14.0f, 14.0f);
        juce::Path p;
        p.addCentredArc(b.getCentreX(), b.getCentreY() + 0.8f, 6.0f, 6.0f, 0.0f, 0.75f, juce::MathConstants<float>::twoPi - 0.75f, true);
        p.startNewSubPath(b.getCentreX(), b.getY() - 0.5f);
        p.lineTo(b.getCentreX(), b.getCentreY() + 0.5f);
        g.setColour(c);
        g.strokePath(p, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    juce::Colour accent { 0xffffffff };
};

// Segmented control attached to a choice parameter. Kickside draws its curves in it.
class Segmented final : public juce::Component {
public:
    Segmented(juce::RangedAudioParameter& param, juce::StringArray labels, bool curves)
        : items(std::move(labels)), drawCurves(curves),
          attachment(param, [this](float v) { selected = (int) std::lround(v); repaint(); }, nullptr) {
        attachment.sendInitialUpdate();
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setTitle(param.getName(64));
    }

    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat();
        look::well(g, r, 11.0f);
        const auto inner = r.reduced(3.0f);
        const float w = (inner.getWidth() - 3.0f * (items.size() - 1)) / (float) items.size();
        for (int i = 0; i < items.size(); ++i) {
            auto cell = juce::Rectangle<float>(inner.getX() + i * (w + 3.0f), inner.getY(), w, inner.getHeight());
            const bool sel = i == selected;
            if (sel) {
                g.setColour(accent.withAlpha(0.13f));
                g.fillRoundedRectangle(cell, 8.0f);
                g.setColour(accent.withAlpha(0.35f));
                g.drawRoundedRectangle(cell.reduced(0.5f), 8.0f, 1.0f);
            } else if (i == hover) {
                g.setColour(juce::Colour(0xff15161c));
                g.fillRoundedRectangle(cell, 8.0f);
            }
            const auto c = sel ? accent : (i == hover ? look::textSoft : look::textDim);
            if (drawCurves) {
                look::drawCurve(g, cell.reduced(6.0f, 6.0f).withTrimmedBottom(3.0f), i, c, sel, sel ? dotPhase : -1.0f);
            } else {
                look::tracked(g, items[i], look::interBold(fontSize), 0.08f, cell, juce::Justification::centred, c);
            }
        }
    }

    void mouseMove(const juce::MouseEvent& e) override { setHover(indexAt(e.position)); }
    void mouseExit(const juce::MouseEvent&) override { setHover(-1); }
    void mouseDown(const juce::MouseEvent& e) override {
        const int i = indexAt(e.position);
        if (i >= 0 && i != selected) attachment.setValueAsCompleteGesture((float) i);
    }

    juce::Colour accent { 0xffffffff };
    float dotPhase = -1.0f;
    float fontSize = 10.0f;

private:
    int indexAt(juce::Point<float> p) const {
        const float w = (float) getWidth() / (float) items.size();
        return juce::jlimit(0, items.size() - 1, (int) (p.x / w));
    }
    void setHover(int i) { if (i != hover) { hover = i; repaint(); } }

    juce::StringArray items;
    bool drawCurves;
    int selected = 0, hover = -1;
    juce::ParameterAttachment attachment;
};

// ===========================================================================
// The knob and the row slider are juce::Sliders painted by this look-and-feel.
// ===========================================================================
class KnobLook final : public juce::LookAndFeel_V4 {
public:
    std::function<juce::String()> centreText;
    juce::Colour accent { 0xffffffff };
    bool showGlow = true;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end,
                          juce::Slider&) override {
        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h);
        const float size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const float s = size / 200.0f;
        const auto c = bounds.getCentre();
        auto at = [&](float angle, float radius) {
            return juce::Point<float>(c.x + radius * s * std::sin(angle), c.y - radius * s * std::cos(angle));
        };

        if (showGlow) {
            juce::ColourGradient glow(accent.withAlpha(0.16f), c.x, c.y, accent.withAlpha(0.0f), c.x + 98.0f * s, c.y, true);
            g.setGradientFill(glow);
            g.fillEllipse(juce::Rectangle<float>(196.0f * s, 196.0f * s).withCentre(c));
        }

        for (int i = 0; i <= 30; ++i) {
            const float t = (float) i / 30.0f;
            const float a = start + t * (end - start);
            const bool lit = t <= pos + 1.0e-4f;
            g.setColour(lit ? accent : juce::Colour(0xff23252e));
            g.drawLine(juce::Line<float>(at(a, 86.0f), at(a, 95.0f)), 3.0f * s);
        }

        const auto body = juce::Rectangle<float>(136.0f * s, 136.0f * s).withCentre(c);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff22242c), c.x, body.getY(),
                                               juce::Colour(0xff14151b), c.x, body.getBottom(), false));
        g.fillEllipse(body);
        g.setColour(juce::Colour(0xff2a2c37));
        g.drawEllipse(body, 1.5f * s);
        g.setColour(juce::Colour(0xff101116));
        g.fillEllipse(juce::Rectangle<float>(120.0f * s, 120.0f * s).withCentre(c));
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        g.drawEllipse(juce::Rectangle<float>(120.0f * s, 120.0f * s).withCentre(c), 1.0f);

        const float a = start + pos * (end - start);
        g.setColour(accent);
        g.drawLine(juce::Line<float>(at(a, 44.0f), at(a, 56.0f)), 5.0f * s);
        g.fillEllipse(juce::Rectangle<float>(5.0f * s, 5.0f * s).withCentre(at(a, 44.0f)));
        g.fillEllipse(juce::Rectangle<float>(5.0f * s, 5.0f * s).withCentre(at(a, 56.0f)));

        if (centreText) {
            const auto t = centreText();
            const float fs = t.length() > 3 ? 19.0f : 30.0f;
            look::tracked(g, t, look::spaceBold(fs * s / 0.88f), -0.02f,
                          juce::Rectangle<float>(110.0f * s, 44.0f * s).withCentre(c), juce::Justification::centred, look::text);
        }
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override {
        const float cy = (float) y + (float) h * 0.5f;
        const auto track = juce::Rectangle<float>((float) x, cy - 2.0f, (float) w, 4.0f);
        g.setColour(juce::Colour(0xff1e2029));
        g.fillRoundedRectangle(track, 2.0f);
        g.setColour(accent);
        g.fillRoundedRectangle(track.withRight(sliderPos), 2.0f);
        g.setColour(look::text);
        g.fillEllipse(juce::Rectangle<float>(14.0f, 14.0f).withCentre({ sliderPos, cy }));
    }

    int getSliderThumbRadius(juce::Slider&) override { return 7; }
};

// ===========================================================================
// GASP waveform strip: play, see, drag the WAV out.
// ===========================================================================
class WaveStrip final : public juce::Component, public juce::DragAndDropContainer {
public:
    explicit WaveStrip(Processor& proc) : p(proc) {}

    void refresh() {
        auto res = p.gaspResult();
        if (res != lastResult) {
            lastResult = res;
            peaks.clear();
            if (res != nullptr && res->audio.getNumSamples() > 0) {
                const int bins = 34;
                const int len = res->audio.getNumSamples();
                for (int b = 0; b < bins; ++b) {
                    const int s0 = len * b / bins, s1 = juce::jmax(s0 + 1, len * (b + 1) / bins);
                    peaks.push_back(res->audio.getMagnitude(s0, s1 - s0));
                }
                float mx = 0.0f;
                for (float v : peaks) mx = juce::jmax(mx, v);
                if (mx > 0.0f) for (float& v : peaks) v /= mx;
            }
        }
        repaint();
    }

    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat();
        look::well(g, r, 10.0f);
        if (!p.hasClip() || peaks.empty()) {
            look::tracked(g, juce::String::fromUTF8("Nenhum \xc3\xa1udio carregado"), look::interRegular(11.0f), 0.0f,
                          r.reduced(12.0f, 0.0f), juce::Justification::centredLeft, look::textDim);
            return;
        }
        // play / stop
        auto play = playArea().toFloat();
        g.setColour(accent.withAlpha(0.16f));
        g.fillRoundedRectangle(play, 7.0f);
        g.setColour(accent);
        if (p.isPreviewing()) {
            g.fillRoundedRectangle(play.withSizeKeepingCentre(9.0f, 9.0f), 1.5f);
        } else {
            juce::Path tri;
            const auto c = play.getCentre();
            tri.addTriangle(c.x - 3.0f, c.y - 5.0f, c.x - 3.0f, c.y + 5.0f, c.x + 5.0f, c.y);
            g.fillPath(tri);
        }
        // bars
        auto bars = barsArea().toFloat();
        const float bw = 2.0f;
        const float gap = (bars.getWidth() - bw * peaks.size()) / (float) juce::jmax<size_t>(1, peaks.size() - 1);
        const float progress = p.isPreviewing() ? p.previewProgress() : 1.0f;
        for (size_t i = 0; i < peaks.size(); ++i) {
            const float h = juce::jmax(2.0f, peaks[i] * (bars.getHeight()));
            const float x = bars.getX() + (float) i * (bw + gap);
            const bool played = (float) i / (float) peaks.size() <= progress;
            g.setColour(played ? accent : juce::Colour(0xff3a3550));
            g.fillRoundedRectangle(x, bars.getCentreY() - h * 0.5f, bw, h, 1.0f);
        }
        // WAV chip
        auto wav = wavArea().toFloat();
        g.setColour(dragHover ? accent.brighter(0.15f) : accent);
        g.fillRoundedRectangle(wav, 7.0f);
        juce::Path arrow;
        const float ax = wav.getX() + 12.0f, ay = wav.getCentreY();
        arrow.startNewSubPath(ax, ay - 5.0f); arrow.lineTo(ax, ay + 4.0f);
        arrow.startNewSubPath(ax - 3.5f, ay + 0.5f); arrow.lineTo(ax, ay + 4.0f); arrow.lineTo(ax + 3.5f, ay + 0.5f);
        g.setColour(juce::Colour(0xff0b0a12));
        g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        look::tracked(g, "WAV", look::interBold(10.0f), 0.1f, wav.withTrimmedLeft(18.0f), juce::Justification::centred,
                      juce::Colour(0xff0b0a12));
    }

    void mouseMove(const juce::MouseEvent& e) override {
        const bool h = wavArea().contains(e.getPosition());
        setMouseCursor(h ? juce::MouseCursor::DraggingHandCursor
                         : (playArea().contains(e.getPosition()) ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor));
        if (h != dragHover) { dragHover = h; repaint(); }
    }
    void mouseExit(const juce::MouseEvent&) override { dragHover = false; repaint(); }

    void mouseDown(const juce::MouseEvent& e) override {
        dragStarted = false;
        if (!p.hasClip()) return;
        if (playArea().contains(e.getPosition())) { p.togglePreview(); repaint(); }
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        if (dragStarted || !p.hasClip() || !wavArea().contains(e.getMouseDownPosition())) return;
        if (e.getDistanceFromDragStart() < 4) return;
        dragStarted = true;
        const auto file = p.writeGaspWav();
        if (file.existsAsFile())
            juce::DragAndDropContainer::performExternalDragDropOfFiles({ file.getFullPathName() }, false, this);
    }

    void mouseUp(const juce::MouseEvent& e) override {
        if (dragStarted || !p.hasClip() || !wavArea().contains(e.getPosition())) return;
        // a click (no drag) saves the WAV where you choose
        const auto suggested = p.writeGaspWav();
        chooser = std::make_unique<juce::FileChooser>("Salvar WAV",
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile(suggested.getFileName()), "*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                             [this](const juce::FileChooser& fc) {
                                 const auto f = fc.getResult();
                                 if (f != juce::File()) p.writeGaspWav(f.withFileExtension("wav"));
                             });
    }

    juce::Colour accent { 0xffa98bff };

private:
    juce::Rectangle<int> playArea() const { return { 4, 4, 28, getHeight() - 8 }; }
    juce::Rectangle<int> wavArea() const { return { getWidth() - 4 - 62, 4, 62, getHeight() - 8 }; }
    juce::Rectangle<int> barsArea() const { return { 40, 7, getWidth() - 40 - 74, getHeight() - 14 }; }

    Processor& p;
    std::shared_ptr<const GaspResult> lastResult;
    std::vector<float> peaks;
    bool dragHover = false, dragStarted = false;
    std::unique_ptr<juce::FileChooser> chooser;
};

// ===========================================================================
// Overlays: help ("i") and settings (gear)
// ===========================================================================
class Overlay : public juce::Component {
public:
    Overlay() : closeButton(IconButton::close) {
        addAndMakeVisible(closeButton);
        closeButton.setTitle("Fechar");
        closeButton.onClick = [this] { setVisible(false); };
        setInterceptsMouseClicks(true, true);
    }
    void resized() override { closeButton.setBounds(getWidth() - 20 - 32, 18, 32, 32); }
    void paintBackdrop(juce::Graphics& g) {
        g.setColour(juce::Colour(0xf4060609));
        g.fillRect(getLocalBounds());
    }
    IconButton closeButton;
};

class InfoOverlay final : public Overlay {
public:
    explicit InfoOverlay(const Spec& s) : spec(s) {}
    void paint(juce::Graphics& g) override {
        paintBackdrop(g);
        const juce::Colour accent(spec.colour);
        look::tracked(g, spec.name, look::spaceBold(20.0f), 0.04f, { 20.0f, 18.0f, 200.0f, 32.0f },
                      juce::Justification::centredLeft, accent);

        float y = 66.0f;
        {
            juce::AttributedString a;
            a.append(juce::String::fromUTF8(spec.lead), look::spaceMedium(15.5f), look::text);
            a.setLineSpacing(3.0f);
            juce::TextLayout tl;
            tl.createLayout(a, 260.0f);
            tl.draw(g, { 20.0f, y, 260.0f, tl.getHeight() });
            y += tl.getHeight() + 18.0f;
        }
        for (const auto& t : spec.tips) {
            look::tracked(g, juce::String::fromUTF8(t.key), look::interBold(9.5f), 0.1f, { 20.0f, y + 1.0f, 74.0f, 14.0f },
                          juce::Justification::topLeft, accent);
            juce::AttributedString a;
            a.append(juce::String::fromUTF8(t.text), look::interRegular(12.0f), juce::Colour(0xffb4b7c7));
            a.setLineSpacing(2.0f);
            juce::TextLayout tl;
            tl.createLayout(a, 176.0f);
            tl.draw(g, { 100.0f, y, 176.0f, tl.getHeight() });
            y += juce::jmax(16.0f, tl.getHeight()) + 11.0f;
        }
    }
private:
    const Spec& spec;
};

class SettingsOverlay final : public Overlay {
public:
    explicit SettingsOverlay(Processor& proc) : p(proc) {
        const juce::Colour accent(p.getSpec().colour);
        for (auto* b : { &updateButton, &originalButton, &minus, &plus }) {
            b->accent = accent;
            addAndMakeVisible(*b);
        }
        updateButton.label = juce::String::fromUTF8("VERIFICAR ATUALIZA\xc3\x87\xc3\x83O");
        updateButton.onClick = [this] { checkForUpdate(); };
        originalButton.label = "INCLUIR O SOM ORIGINAL";
        originalButton.setClickingTogglesState(true);
        originalButton.setToggleState(p.includeOriginal, juce::dontSendNotification);
        originalButton.onClick = [this] { p.includeOriginal = originalButton.getToggleState(); };
        minus.label = "-";
        plus.label = "+";
        minus.onClick = [this] { p.manualBpm = juce::jmax(60.0, p.manualBpm - 1.0); repaint(); };
        plus.onClick = [this] { p.manualBpm = juce::jmin(200.0, p.manualBpm + 1.0); repaint(); };
        const bool gasp = p.getId() == PluginId::gasp;
        originalButton.setVisible(gasp);
        minus.setVisible(gasp);
        plus.setVisible(gasp);
    }

    void resized() override {
        Overlay::resized();
        updateButton.setBounds(20, 116, 260, 32);
        originalButton.setBounds(20, 222, 260, 32);
        minus.setBounds(188, 284, 40, 28);
        plus.setBounds(240, 284, 40, 28);
    }

    void paint(juce::Graphics& g) override {
        paintBackdrop(g);
        look::tracked(g, "AJUSTES", look::interBold(11.0f), 0.2f, { 20.0f, 18.0f, 200.0f, 32.0f },
                      juce::Justification::centredLeft, look::textDim);
        look::tracked(g, p.getSpec().productName, look::spaceBold(20.0f), 0.0f, { 20.0f, 62.0f, 260.0f, 24.0f },
                      juce::Justification::centredLeft, look::text);
        look::tracked(g, juce::String::fromUTF8("Vers\xc3\xa3o ") + Processor::versionString(), look::interRegular(12.0f), 0.0f,
                      { 20.0f, 86.0f, 260.0f, 18.0f }, juce::Justification::centredLeft, look::textDim);
        look::tracked(g, status, look::interRegular(11.5f), 0.0f, { 20.0f, 154.0f, 260.0f, 32.0f },
                      juce::Justification::topLeft, statusColour);
        if (p.getId() == PluginId::gasp) {
            look::tracked(g, "GASP", look::interBold(10.0f), 0.18f, { 20.0f, 196.0f, 200.0f, 16.0f },
                          juce::Justification::centredLeft, juce::Colour(p.getSpec().colour));
            const bool host = p.hostHasTempo();
            look::tracked(g, host ? juce::String("BPM ") + juce::String(p.effectiveBpm(), 1) + " (da DAW)"
                                  : juce::String("BPM ") + juce::String((int) p.manualBpm) + juce::String::fromUTF8(" (sem DAW)"),
                          look::interRegular(12.5f), 0.0f, { 20.0f, 284.0f, 160.0f, 28.0f }, juce::Justification::centredLeft,
                          look::textSoft);
            minus.setEnabled(!host);
            plus.setEnabled(!host);
            minus.setAlpha(host ? 0.35f : 1.0f);
            plus.setAlpha(host ? 0.35f : 1.0f);
        }
        look::tracked(g, "by audiomakers", look::interRegular(11.0f), 0.04f,
                      { 20.0f, (float) getHeight() - 46.0f, 260.0f, 20.0f }, juce::Justification::centred, look::textDim);
    }

    std::function<void(bool)> onUpdateBadge;

    void checkForUpdate() {
        status = "Verificando...";
        statusColour = look::textDim;
        repaint();
        checker.start(Processor::versionString(), [safe = juce::Component::SafePointer<SettingsOverlay>(this)](UpdateChecker::Result r) {
            if (safe == nullptr) return;
            if (!r.succeeded) {
                safe->status = juce::String::fromUTF8("Sem conex\xc3\xa3o. Tente de novo.");
                safe->statusColour = look::textDim;
            } else if (r.updateAvailable) {
                safe->status = juce::String::fromUTF8("Vers\xc3\xa3o ") + r.latestVersion + juce::String::fromUTF8(" dispon\xc3\xadvel. Abrindo o download...");
                safe->statusColour = juce::Colour(0xffffd23f);
                juce::URL(r.downloadUrl).launchInDefaultBrowser();
            } else {
                safe->status = juce::String::fromUTF8("Voc\xc3\xaa est\xc3\xa1 na vers\xc3\xa3o mais recente.");
                safe->statusColour = juce::Colour(0xff3de08a);
            }
            if (safe->onUpdateBadge) safe->onUpdateBadge(r.updateAvailable);
            safe->repaint();
        });
    }

private:
    Processor& p;
    PillButton updateButton, originalButton, minus, plus;
    juce::String status;
    juce::Colour statusColour { look::textDim };
    UpdateChecker checker;
};

// ===========================================================================
// Surface: the whole window in design units (300 x 480)
// ===========================================================================
class Surface final : public juce::Component, private juce::Timer, public juce::FileDragAndDropTarget {
public:
    // vertical rhythm, from the approved mock-up
    static constexpr float topY = 18.0f, nameY = 64.8f, modesY = 119.6f, knobY = 168.4f, rowY = 379.2f, bottomY = 430.0f;

    explicit Surface(Processor& proc)
        : p(proc), spec(proc.getSpec()), accent(spec.colour),
          infoButton(IconButton::info), gearButton(IconButton::gear),
          modes(*p.state.getParameter("mode"), toArray(spec.modes), spec.id == PluginId::kickside),
          info(spec), settings(proc), wave(proc) {
        setSize(Editor::designW, Editor::designH);
        setOpaque(true);

        addAndMakeVisible(infoButton);
        addAndMakeVisible(gearButton);
        infoButton.setTitle("O que este plugin faz");
        gearButton.setTitle("Ajustes");
        infoButton.onClick = [this] { settings.setVisible(false); info.setVisible(true); };
        gearButton.onClick = [this] { info.setVisible(false); settings.setVisible(true); settings.repaint(); };

        addAndMakeVisible(modes);
        for (auto* m : spec.modes)
            if (std::strlen(m) >= 7) modes.fontSize = 9.5f;

        knob.setLookAndFeel(&knobLook);
        knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        knob.setMouseDragSensitivity(240);
        knob.setDoubleClickReturnValue(true, spec.knobDefault);
        knob.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        knob.setTitle(spec.knobLabel);
        knobAttachment = std::make_unique<juce::SliderParameterAttachment>(*p.state.getParameter("amount"), knob);
        knobLook.centreText = [this] {
            if (spec.id == PluginId::gasp) return juce::String(gaspLabels[(size_t) gaspLengthIndex((float) knob.getValue() / 100.0f)]);
            return juce::String((int) std::lround(knob.getValue()));
        };
        addAndMakeVisible(knob);

        if (auto* extra = p.state.getParameter("extra")) {
            if (spec.rowType == RowType::slider) {
                rowSlider.setLookAndFeel(&knobLook);
                rowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
                rowSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
                rowSlider.setTitle(spec.rowLabel);
                rowSlider.setDoubleClickReturnValue(true, spec.id == PluginId::wide ? 120.0 : 100.0);
                rowAttachment = std::make_unique<juce::SliderParameterAttachment>(*extra, rowSlider);
                rowSlider.onValueChange = [this] { repaint(); };
                addAndMakeVisible(rowSlider);
            } else if (spec.rowType == RowType::tempo) {
                juce::StringArray t;
                for (auto* l : tempoLabels) t.add(l);
                tempo = std::make_unique<Segmented>(*extra, t, false);
                tempo->fontSize = 11.0f;
                addAndMakeVisible(*tempo);
            }
        }
        if (spec.rowType == RowType::wave) addAndMakeVisible(wave);

        addAndMakeVisible(power);
        powerAttachment = std::make_unique<juce::ButtonParameterAttachment>(*p.state.getParameter("bypass"), power);
        power.onStateChange = [this] { applyAccent(); };

        addChildComponent(info);
        addChildComponent(settings);
        settings.onUpdateBadge = [this](bool b) { gearButton.badge = b; gearButton.repaint(); };

        applyAccent();
        layout();
        wave.refresh();
        startTimerHz(30);
    }

    ~Surface() override {
        knob.setLookAndFeel(nullptr);
        rowSlider.setLookAndFeel(nullptr);
    }

    // ---- painting ----
    void paint(juce::Graphics& g) override {
        g.setGradientFill(juce::ColourGradient(look::bgTop, 0.0f, 0.0f, look::bgBottom, 0.0f, (float) getHeight(), false));
        g.fillAll();
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawHorizontalLine(0, 0.0f, (float) getWidth());

        // top bar: logo + knobz
        {
            const auto c = juce::Point<float>(28.0f, topY + 16.0f);
            g.setColour(look::text);
            g.drawEllipse(juce::Rectangle<float>(13.0f, 13.0f).withCentre(c), 1.6f);
            g.setColour(currentAccent);
            g.drawLine(c.x, c.y, c.x, c.y - 4.5f, 2.0f);
            look::tracked(g, "knobz", look::spaceBold(15.0f), -0.01f, { 40.0f, topY, 100.0f, 32.0f },
                          juce::Justification::centredLeft, look::text);
        }

        // name
        look::tracked(g, spec.name, look::spaceBold(34.0f), 0.06f, { 0.0f, nameY, 300.0f, 40.0f },
                      juce::Justification::centred, currentAccent);

        // knob caption or GASP drop zone
        const auto knobArea = juce::Rectangle<float>(20.0f, knobY, 260.0f, 196.0f);
        if (showDropZone()) {
            auto z = knobArea.withSizeKeepingCentre(260.0f, 176.0f);
            g.setColour(fileHover ? juce::Colour(0xff171329) : juce::Colour(0xff0d0b16));
            g.fillRoundedRectangle(z, 20.0f);
            juce::Path border;
            border.addRoundedRectangle(z.reduced(0.75f), 20.0f);
            juce::Path dashed;
            const float dashes[] = { 6.0f, 5.0f };
            juce::PathStrokeType(1.5f).createDashedStroke(dashed, border, dashes, 2);
            g.setColour(fileHover ? currentAccent : juce::Colour(0xff4a3f73));
            g.fillPath(dashed);
            juce::Path arrow;
            const float cx = z.getCentreX(), cy = z.getCentreY() - 22.0f;
            arrow.startNewSubPath(cx, cy - 13.0f); arrow.lineTo(cx, cy + 7.0f);
            arrow.startNewSubPath(cx - 8.0f, cy - 1.0f); arrow.lineTo(cx, cy + 7.0f); arrow.lineTo(cx + 8.0f, cy - 1.0f);
            arrow.startNewSubPath(cx - 12.0f, cy + 14.0f); arrow.lineTo(cx + 12.0f, cy + 14.0f);
            g.setColour(currentAccent);
            g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            look::tracked(g, juce::String::fromUTF8("Arraste o \xc3\xa1udio aqui"), look::interBold(13.0f), 0.0f,
                          { z.getX(), cy + 26.0f, z.getWidth(), 18.0f }, juce::Justification::centred, look::text);
            look::tracked(g, "ou clique para abrir", look::interRegular(11.0f), 0.0f,
                          { z.getX(), cy + 46.0f, z.getWidth(), 16.0f }, juce::Justification::centred, look::textDim);
        } else {
            look::tracked(g, spec.knobLabel, look::interBold(10.0f), 0.22f, { 0.0f, knobY + 178.0f, 300.0f, 16.0f },
                          juce::Justification::centred, look::textDim);
        }

        // row label / value for sliders
        if (spec.rowType == RowType::slider) {
            look::tracked(g, spec.rowLabel, look::interBold(10.0f), 0.14f, { 20.0f, rowY, 80.0f, 36.0f },
                          juce::Justification::centredLeft, look::textDim);
            juce::String v = spec.id == PluginId::wide ? juce::String((int) std::lround(rowSlider.getValue())) + " Hz"
                                                       : juce::String((int) std::lround(rowSlider.getValue())) + "%";
            look::tracked(g, v, look::interBold(11.0f), 0.0f, { 230.0f, rowY, 50.0f, 36.0f },
                          juce::Justification::centredRight, look::textSoft);
        }

        // bottom: by audiomakers + right slot
        {
            juce::AttributedString a;
            a.setJustification(juce::Justification::centred);
            a.append("by ", look::interRegular(11.0f), look::textDim);
            a.append("audiomakers", look::interBold(11.0f), look::text);
            a.draw(g, { 60.0f, bottomY, 180.0f, 32.0f });
            if (spec.id == PluginId::wide) {
                const float corr = p.meter();
                const juce::Colour c = corr > 0.2f ? juce::Colour(0xff3de08a) : (corr > -0.05f ? juce::Colour(0xffffc23f) : juce::Colour(0xffff4d4d));
                g.setColour(c.withAlpha(0.25f));
                g.fillEllipse(juce::Rectangle<float>(14.0f, 14.0f).withCentre({ 273.0f, bottomY + 16.0f }));
                g.setColour(c);
                g.fillEllipse(juce::Rectangle<float>(7.0f, 7.0f).withCentre({ 273.0f, bottomY + 16.0f }));
                look::tracked(g, "MONO", look::interBold(8.5f), 0.1f, { 228.0f, bottomY, 36.0f, 32.0f },
                              juce::Justification::centredRight, look::textDim);
            } else {
                look::tracked(g, Processor::versionString().upToLastOccurrenceOf(".", false, false), look::interRegular(10.0f), 0.0f,
                              { 248.0f, bottomY, 32.0f, 32.0f }, juce::Justification::centredRight, look::textDim);
            }
        }
    }

    // ---- GASP file loading ----
    bool isInterestedInFileDrag(const juce::StringArray& files) override {
        return spec.id == PluginId::gasp && files.size() > 0 && isAudioFile(files[0]);
    }
    void fileDragEnter(const juce::StringArray&, int, int) override { fileHover = true; repaint(); }
    void fileDragExit(const juce::StringArray&) override { fileHover = false; repaint(); }
    void filesDropped(const juce::StringArray& files, int, int) override {
        fileHover = false;
        if (files.size() > 0) loadFile(juce::File(files[0]));
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override {
        if (!showDropZone()) return;
        if (!juce::Rectangle<float>(20.0f, knobY, 260.0f, 196.0f).contains(e.position)) return;
        chooser = std::make_unique<juce::FileChooser>(juce::String::fromUTF8("Abrir \xc3\xa1udio"), juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser& fc) {
                                 if (fc.getResult().existsAsFile()) loadFile(fc.getResult());
                             });
    }

    void loadFile(const juce::File& f) {
        if (p.loadClip(f)) {
            layout();
            wave.refresh();
        }
        repaint();
    }

    Processor& processor() { return p; }
    void showInfo(bool b) { info.setVisible(b); }
    void showSettings(bool b) { settings.setVisible(b); }

private:
    static juce::StringArray toArray(const std::vector<const char*>& v) {
        juce::StringArray a;
        for (auto* s : v) a.add(s);
        return a;
    }
    static bool isAudioFile(const juce::String& path) {
        return juce::File(path).hasFileExtension("wav;aif;aiff;flac;mp3;ogg");
    }
    bool showDropZone() const { return spec.id == PluginId::gasp && !p.hasClip(); }

    void layout() {
        infoButton.setBounds(300 - 20 - 32 - 6 - 32, (int) topY, 32, 32);
        gearButton.setBounds(300 - 20 - 32, (int) topY, 32, 32);
        modes.setBounds(20, (int) modesY, 260, 34);
        knob.setBounds(62, (int) knobY, 176, 176);
        knob.setVisible(!showDropZone());
        rowSlider.setBounds(108, (int) rowY, 116, 36);
        if (tempo) tempo->setBounds(20, (int) rowY, 260, 36);
        wave.setBounds(20, (int) rowY, 260, 36);
        power.setBounds(20, (int) bottomY, 32, 32);
        info.setBounds(getLocalBounds());
        settings.setBounds(getLocalBounds());
    }

    void applyAccent() {
        currentAccent = p.isBypassed() ? look::off : accent;
        knobLook.accent = currentAccent;
        knobLook.showGlow = !p.isBypassed();
        modes.accent = currentAccent;
        if (tempo) tempo->accent = currentAccent;
        wave.accent = currentAccent;
        power.accent = accent;
        repaint();
        modes.repaint();
        if (tempo) tempo->repaint();
    }

    void timerCallback() override {
        if (p.isBypassed() != (currentAccent == look::off)) applyAccent();
        if (spec.id == PluginId::kickside) {
            const float ph = p.meter();
            if (ph < lastPhase - 0.5f) secondCycle = !secondCycle;
            lastPhase = ph;
            modes.dotPhase = (ph + (secondCycle ? 1.0f : 0.0f)) * 0.5f;
            modes.repaint();
        } else if (spec.id == PluginId::wide) {
            repaint(220, (int) bottomY, 70, 32);
        } else if (spec.id == PluginId::gasp) {
            if (!knob.isMouseButtonDown()) p.updateGaspIfNeeded();
            const bool dz = showDropZone();
            if (knob.isVisible() == dz) layout();
            wave.refresh();
        }
    }

    Processor& p;
    const Spec& spec;
    juce::Colour accent, currentAccent;
    KnobLook knobLook;
    IconButton infoButton, gearButton;
    Segmented modes;
    juce::Slider knob, rowSlider;
    std::unique_ptr<juce::SliderParameterAttachment> knobAttachment, rowAttachment;
    std::unique_ptr<Segmented> tempo;
    PowerButton power;
    std::unique_ptr<juce::ButtonParameterAttachment> powerAttachment;
    InfoOverlay info;
    SettingsOverlay settings;
    WaveStrip wave;
    bool fileHover = false, secondCycle = false;
    float lastPhase = 0.0f;
    std::unique_ptr<juce::FileChooser> chooser;
};

// ===========================================================================
Editor::Editor(Processor& p) : juce::AudioProcessorEditor(p), surface(std::make_unique<Surface>(p)) {
    addAndMakeVisible(*surface);
    surface->setTransform(juce::AffineTransform::scale(scale));
    setSize((int) std::lround(designW * scale), (int) std::lround(designH * scale));
}

Editor::~Editor() = default;

void Editor::resized() {}

void Editor::paint(juce::Graphics& g) { g.fillAll(look::bgBottom); }

} // namespace knobz
