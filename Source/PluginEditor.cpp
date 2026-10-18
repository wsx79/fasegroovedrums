// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginEditor.h"

#include <BinaryData.h>

namespace
{
    juce::Font font (float height, bool bold = false)
    {
        return juce::Font (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    }

    void styleCaption (juce::Label& label, const juce::String& text)
    {
        label.setText (text.toUpperCase(), juce::dontSendNotification);
        label.setFont (font (11.0f, true));
        label.setColour (juce::Label::textColourId, Palette::dim);
        label.setInterceptsMouseClicks (false, false);
    }

    void drawPanel (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title)
    {
        const auto r = area.toFloat();
        g.setColour (Palette::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (Palette::panelEdge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);
        if (title.isNotEmpty())
        {
            g.setColour (Palette::accent);
            g.fillRoundedRectangle ((float) area.getX() + 12.0f, (float) area.getY() + 11.0f, 3.0f, 12.0f, 1.0f);
            g.setColour (Palette::text.withAlpha (0.92f));
            g.setFont (font (11.5f, true));
            g.drawText (title, area.withHeight (34).withTrimmedLeft (21).withTrimmedRight (12), juce::Justification::centredLeft);
        }
    }

    const int clipBarChoices[] { 0, 1, 2, 4, 8, 16, FaseGrooveDrumsProcessor::songArrangement };
    const int songBarChoices[] { 0, 1, 2, 4, 8, 12, 16, 24, 32 };
}

juce::Colour laneColour (int lane)
{
    switch ((fgd::Lane) lane)
    {
        case fgd::Lane::kick:       return Palette::accent;
        case fgd::Lane::snare:
        case fgd::Lane::clap:
        case fgd::Lane::rim:        return Palette::accent2;
        case fgd::Lane::closedHat:
        case fgd::Lane::openHat:
        case fgd::Lane::ride:
        case fgd::Lane::crash:      return Palette::cymbals;
        case fgd::Lane::tom:        return Palette::toms;
        case fgd::Lane::shaker:
        case fgd::Lane::tambourine:
        case fgd::Lane::conga:
        case fgd::Lane::cowbell:
        default:                    return Palette::percussion;
    }
}

//==============================================================================
GrooveLookAndFeel::GrooveLookAndFeel()
{
    regular = juce::Typeface::createSystemTypefaceFor (BinaryData::InterRegular_otf, BinaryData::InterRegular_otfSize);
    semiBold = juce::Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_otf, BinaryData::InterSemiBold_otfSize);

    setColour (juce::ResizableWindow::backgroundColourId, Palette::background);
    setColour (juce::TextButton::buttonColourId, Palette::control);
    setColour (juce::TextButton::buttonOnColourId, Palette::accent);
    setColour (juce::TextButton::textColourOffId, Palette::text);
    setColour (juce::TextButton::textColourOnId, Palette::background);
    setColour (juce::ComboBox::backgroundColourId, Palette::field);
    setColour (juce::ComboBox::outlineColourId, Palette::panelEdge);
    setColour (juce::ComboBox::textColourId, Palette::text);
    setColour (juce::ComboBox::arrowColourId, Palette::accent);
    setColour (juce::ComboBox::focusedOutlineColourId, Palette::accent);
    setColour (juce::PopupMenu::backgroundColourId, Palette::panel);
    setColour (juce::PopupMenu::textColourId, Palette::text);
    setColour (juce::PopupMenu::headerTextColourId, Palette::accent);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.85f));
    setColour (juce::PopupMenu::highlightedTextColourId, Palette::background);
    setColour (juce::Slider::textBoxTextColourId, Palette::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::trackColourId, Palette::accent.withAlpha (0.35f));
    setColour (juce::Slider::backgroundColourId, Palette::field);
    setColour (juce::Label::textColourId, Palette::text);
}

juce::Typeface::Ptr GrooveLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    // The default sans-serif becomes Inter (bold styles: Inter SemiBold), the same on every system.
    if (f.getTypefaceName() == juce::Font::getDefaultSansSerifFontName() && regular != nullptr && semiBold != nullptr)
        return f.isBold() || f.getTypefaceStyle().containsIgnoreCase ("bold") ? semiBold : regular;
    return juce::LookAndFeel_V4::getTypefaceForFont (f);
}

void GrooveLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& background, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto fillColour = b.getToggleState() ? b.findColour (juce::TextButton::buttonOnColourId) : background;
    if (! b.isEnabled())
        fillColour = fillColour.withMultipliedAlpha (0.5f);
    else if (down)
        fillColour = fillColour.darker (0.15f);
    else if (highlighted)
        fillColour = fillColour.brighter (0.08f);

    g.setColour (fillColour);
    g.fillRoundedRectangle (r, 4.0f);
    const bool coloured = fillColour.getPerceivedBrightness() > 0.35f;
    g.setColour (coloured ? fillColour.darker (0.25f) : (highlighted ? Palette::panelEdge.brighter (0.35f) : Palette::panelEdge));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
}

juce::Font GrooveLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return font (juce::jlimit (10.0f, 13.5f, buttonHeight * 0.45f), true);
}

void GrooveLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    const bool on = b.getToggleState();
    const auto fillColour = b.findColour (on ? juce::TextButton::buttonOnColourId : juce::TextButton::buttonColourId);
    auto colour = fillColour.getPerceivedBrightness() > 0.35f ? Palette::background : Palette::text;
    g.setColour (colour.withMultipliedAlpha (b.isEnabled() ? 1.0f : 0.5f));
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.drawFittedText (b.getButtonText(), b.getLocalBounds().reduced (4, 2), juce::Justification::centred, 1, 0.75f);
}

void GrooveLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (box.isMouseOver (true) ? Palette::panelEdge.brighter (0.4f) : Palette::panelEdge);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    const float cx = (float) width - 12.0f, cy = height * 0.5f;
    juce::Path chevron;
    chevron.startNewSubPath (cx - 3.5f, cy - 1.8f);
    chevron.lineTo (cx, cy + 1.8f);
    chevron.lineTo (cx + 3.5f, cy - 1.8f);
    g.setColour (box.isEnabled() ? Palette::dim : Palette::dim.withAlpha (0.4f));
    g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font GrooveLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return font (juce::jlimit (11.0f, 14.0f, box.getHeight() * 0.46f));
}

void GrooveLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (2, 1, box.getWidth() - 22, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void GrooveLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (Palette::panel);
    g.setColour (Palette::panelEdge);
    g.drawRect (0, 0, width, height, 1);
}

juce::Font GrooveLookAndFeel::getPopupMenuFont()
{
    return font (14.0f);
}

void GrooveLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
    g.setColour (Palette::field);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (Palette::text);
    g.setFont (font (13.0f));
    g.drawFittedText (text, r.reduced (10.0f, 6.0f).toNearestInt(), juce::Justification::centredLeft, 8, 1.0f);
}

juce::Rectangle<int> GrooveLookAndFeel::getTooltipBounds (const juce::String& text, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const auto f = font (13.0f);
    const float width = juce::GlyphArrangement::getStringWidth (f, text);
    const int w = juce::jmin (420, juce::roundToInt (width) + 22);
    const int lines = juce::jmax (1, juce::roundToInt (width / 398.0f + 0.5f));
    const int h = juce::roundToInt ((float) lines * f.getHeight() * 1.2f) + 12;
    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 6, w, h)
        .constrainedWithin (parentArea);
}

juce::Font GrooveLookAndFeel::getLabelFont (juce::Label& label)
{
    return label.getFont();
}

void GrooveLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat();
    auto pill = r.removeFromLeft (34.0f).withSizeKeepingCentre (30.0f, 16.0f);
    const bool on = b.getToggleState();

    g.setColour (on ? Palette::accent : Palette::field);
    g.fillRoundedRectangle (pill, 8.0f);
    g.setColour (on ? Palette::accent.darker (0.2f) : (highlighted ? Palette::panelEdge.brighter (0.4f) : Palette::panelEdge));
    g.drawRoundedRectangle (pill.reduced (0.5f), 8.0f, 1.0f);

    const float d = 12.0f;
    const float cx = on ? pill.getRight() - d - 2.0f : pill.getX() + 2.0f;
    g.setColour (on ? Palette::background : Palette::dim);
    g.fillEllipse (cx, pill.getCentreY() - d * 0.5f, d, d);

    g.setColour (Palette::text);
    g.setFont (font (13.0f));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (6.0f), juce::Justification::centredLeft);
}

void GrooveLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                                          float startAngle, float endAngle, juce::Slider&)
{
    // A studio knob: thin track, accent value arc, a dark body with a soft top light and a pointer.
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float lineW = juce::jlimit (2.0f, 3.5f, radius * 0.11f);
    const float arcR = radius - lineW * 0.5f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Palette::panelEdge);
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (Palette::accent);
    g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobR = arcR - lineW - 3.0f;
    const auto body = juce::Rectangle<float> (knobR * 2.0f, knobR * 2.0f).withCentre (centre);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (body.translated (0.0f, 1.5f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c313b), centre.x, body.getY(),
                                             juce::Colour (0xff181b21), centre.x, body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (juce::Colour (0xff3a404b));
    g.drawEllipse (body.reduced (0.5f), 1.0f);

    g.setColour (Palette::text);
    g.drawLine ({ centre.getPointOnCircumference (knobR * 0.35f, angle), centre.getPointOnCircumference (knobR * 0.85f, angle) }, 2.0f);
}

//==============================================================================
Knob::Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption)
    : attachment (state, paramId, slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 15);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, Palette::text);
    addAndMakeVisible (slider);

    styleCaption (label, caption);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromBottom (14));
    slider.setBounds (r);
}

ParamCombo::ParamCombo (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption)
    : parameter (state.getParameter (paramId))
{
    if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (parameter))
    {
        for (int i = 0; i < c->choices.size(); ++i)
            box.addItem (c->choices[i], i + 1);
    }
    else if (auto* n = dynamic_cast<juce::AudioParameterInt*> (parameter))
    {
        for (int v = n->getRange().getStart(); v <= n->getRange().getEnd(); ++v)
            box.addItem (juce::String (v), v - n->getRange().getStart() + 1);
    }

    box.onChange = [this]
    {
        if (parameter == nullptr || box.getSelectedId() <= 0)
            return;
        const float plain = (float) (box.getSelectedId() - 1);
        const float base = dynamic_cast<juce::AudioParameterInt*> (parameter) != nullptr
                               ? (float) dynamic_cast<juce::AudioParameterInt*> (parameter)->getRange().getStart() : 0.0f;
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (plain + base));
        parameter->endChangeGesture();
    };
    addAndMakeVisible (box);

    hasCaption = caption.isNotEmpty();
    styleCaption (label, caption);
    addAndMakeVisible (label);
    label.setVisible (hasCaption);
    refresh();
}

void ParamCombo::resized()
{
    auto r = getLocalBounds();
    if (hasCaption)
        label.setBounds (r.removeFromTop (15));
    box.setBounds (r.removeFromTop (juce::jmin (30, r.getHeight())));
}

void ParamCombo::refresh()
{
    if (parameter == nullptr)
        return;
    const float plain = parameter->convertFrom0to1 (parameter->getValue());
    const float base = dynamic_cast<juce::AudioParameterInt*> (parameter) != nullptr
                           ? (float) dynamic_cast<juce::AudioParameterInt*> (parameter)->getRange().getStart() : 0.0f;
    const int id = juce::roundToInt (plain - base) + 1;
    if (box.getSelectedId() != id)
        box.setSelectedId (id, juce::dontSendNotification);
}

//==============================================================================
namespace
{
    /** Icons are drawn in a unit square, then fitted to the area. */
    juce::Path unitIcon (IconButton::Icon icon, float& strokeWidth, bool& filled)
    {
        using I = IconButton::Icon;
        juce::Path p;
        strokeWidth = 0.09f;
        filled = false;
        switch (icon)
        {
            case I::play:   p.addTriangle (0.28f, 0.18f, 0.28f, 0.82f, 0.82f, 0.5f); filled = true; break;
            case I::pause:  p.addRoundedRectangle (0.24f, 0.2f, 0.17f, 0.6f, 0.03f); p.addRoundedRectangle (0.59f, 0.2f, 0.17f, 0.6f, 0.03f); filled = true; break;
            case I::stop:   p.addRoundedRectangle (0.24f, 0.24f, 0.52f, 0.52f, 0.05f); filled = true; break;
            case I::rewind: p.addRectangle (0.2f, 0.22f, 0.11f, 0.56f); p.addTriangle (0.8f, 0.22f, 0.8f, 0.78f, 0.33f, 0.5f); filled = true; break;
            case I::dice:
                p.addRoundedRectangle (0.14f, 0.14f, 0.72f, 0.72f, 0.14f);
                for (auto pip : { juce::Point<float> (0.33f, 0.33f), { 0.67f, 0.33f }, { 0.5f, 0.5f }, { 0.33f, 0.67f }, { 0.67f, 0.67f } })
                    p.addEllipse (pip.x - 0.065f, pip.y - 0.065f, 0.13f, 0.13f);
                strokeWidth = 0.08f;
                break;
            case I::undo:
            case I::redo:
            {
                juce::Path a;
                a.startNewSubPath (0.16f, 0.36f);
                a.lineTo (0.6f, 0.36f);
                a.addCentredArc (0.6f, 0.57f, 0.21f, 0.21f, 0.0f, 0.0f, juce::MathConstants<float>::pi, false);
                a.lineTo (0.34f, 0.78f);
                a.startNewSubPath (0.34f, 0.18f); a.lineTo (0.16f, 0.36f); a.lineTo (0.34f, 0.54f);
                if (icon == I::redo)
                    a.applyTransform (juce::AffineTransform::scale (-1.0f, 1.0f).translated (1.0f, 0.0f));
                p = a;
                break;
            }
            case I::gear:
            {
                const int teeth = 8;
                const float outer = 0.46f, inner = 0.34f;
                for (int k = 0; k < teeth * 4; ++k)
                {
                    const float a = ((float) k / (float) (teeth * 4)) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
                    const float r = (k % 4 == 0 || k % 4 == 1) ? outer : inner;
                    const juce::Point<float> pt (0.5f + r * std::cos (a), 0.5f + r * std::sin (a));
                    if (k == 0) p.startNewSubPath (pt); else p.lineTo (pt);
                }
                p.closeSubPath();
                p.addEllipse (0.36f, 0.36f, 0.28f, 0.28f);
                p.setUsingNonZeroWinding (false);
                filled = true;
                break;
            }
            case I::close:
                p.startNewSubPath (0.24f, 0.24f); p.lineTo (0.76f, 0.76f);
                p.startNewSubPath (0.76f, 0.24f); p.lineTo (0.24f, 0.76f);
                strokeWidth = 0.1f;
                break;
            case I::fullScreen:
            case I::exitFullScreen:
            {
                const bool in = icon == I::exitFullScreen;
                for (int c = 0; c < 4; ++c)
                {
                    const float x = c % 2 == 0 ? 0.16f : 0.84f, y = c < 2 ? 0.16f : 0.84f;
                    const float dx = c % 2 == 0 ? 0.24f : -0.24f, dy = c < 2 ? 0.24f : -0.24f;
                    if (! in) { p.startNewSubPath (x + dx, y); p.lineTo (x, y); p.lineTo (x, y + dy); }
                    else      { p.startNewSubPath (x + dx, y); p.lineTo (x + dx, y + dy); p.lineTo (x, y + dy); }
                }
                break;
            }
            case I::lock:
            case I::unlock:
                p.addRoundedRectangle (0.2f, 0.46f, 0.6f, 0.42f, 0.06f);
                p.startNewSubPath (0.32f, 0.46f);
                if (icon == I::lock)
                {
                    p.lineTo (0.32f, 0.32f);
                    p.addCentredArc (0.5f, 0.32f, 0.18f, 0.18f, 0.0f, -juce::MathConstants<float>::halfPi, juce::MathConstants<float>::halfPi, false);
                    p.lineTo (0.68f, 0.46f);
                }
                else
                {
                    p.lineTo (0.32f, 0.3f);
                    p.addCentredArc (0.5f, 0.3f, 0.18f, 0.18f, 0.0f, -juce::MathConstants<float>::halfPi, juce::MathConstants<float>::halfPi, false);
                    p.lineTo (0.68f, 0.34f);
                }
                strokeWidth = 0.09f;
                break;
            case I::left:  p.addTriangle (0.7f, 0.2f, 0.7f, 0.8f, 0.26f, 0.5f); filled = true; break;
            case I::right: p.addTriangle (0.3f, 0.2f, 0.3f, 0.8f, 0.74f, 0.5f); filled = true; break;
            case I::song:
                // The song timeline: blocks of different lengths.
                p.addRoundedRectangle (0.06f, 0.36f, 0.2f, 0.28f, 0.04f);
                p.addRoundedRectangle (0.3f, 0.36f, 0.36f, 0.28f, 0.04f);
                p.addRoundedRectangle (0.7f, 0.36f, 0.24f, 0.28f, 0.04f);
                filled = true;
                break;
            case I::folder:
                p.startNewSubPath (0.1f, 0.24f); p.lineTo (0.4f, 0.24f); p.lineTo (0.48f, 0.34f); p.lineTo (0.9f, 0.34f);
                p.lineTo (0.9f, 0.8f); p.lineTo (0.1f, 0.8f); p.closeSubPath();
                strokeWidth = 0.08f;
                break;
            case I::none:
            default: break;
        }
        return p;
    }
}

void IconButton::drawIcon (juce::Graphics& g, Icon icon, juce::Rectangle<float> r, juce::Colour colour)
{
    if (icon == Icon::none)
        return;
    float stroke = 0.09f;
    bool filled = false;
    auto p = unitIcon (icon, stroke, filled);
    const float s = juce::jmin (r.getWidth(), r.getHeight());
    const auto box = r.withSizeKeepingCentre (s, s);
    p.applyTransform (juce::AffineTransform::scale (s).translated (box.getX(), box.getY()));
    g.setColour (colour);
    if (filled)
        g.fillPath (p);
    else
        g.strokePath (p, juce::PathStrokeType (juce::jmax (1.2f, stroke * s), juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void IconButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    const bool on = getToggleState();
    const bool filled = on || primary;
    const bool flatLeft = isConnectedOnLeft(), flatRight = isConnectedOnRight();

    auto fillColour = filled ? onColour : Palette::control;
    if (down)             fillColour = fillColour.darker (0.15f);
    else if (highlighted) fillColour = fillColour.brighter (0.08f);
    if (! isEnabled())    fillColour = fillColour.withMultipliedAlpha (0.5f);

    juce::Path shape;
    shape.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), 4.0f, 4.0f, ! flatLeft, ! flatRight, ! flatLeft, ! flatRight);
    g.setColour (fillColour);
    g.fillPath (shape);
    g.setColour (filled ? fillColour.darker (0.3f) : (highlighted ? Palette::panelEdge.brighter (0.35f) : Palette::panelEdge));
    g.strokePath (shape, juce::PathStrokeType (1.0f));

    auto ink = filled ? Palette::background : Palette::text.withAlpha (0.92f);
    if (! isEnabled())
        ink = ink.withMultipliedAlpha (0.4f);
    const auto iconInk = ! filled && ! iconColour.isTransparent() ? iconColour.withMultipliedAlpha (isEnabled() ? 1.0f : 0.4f) : ink;
    const float iconSize = juce::jmin (r.getHeight() * 0.56f, 18.0f);
    const auto label = getButtonText();

    auto content = r.reduced (6.0f, 0.0f);
    if (label.isEmpty())
    {
        drawIcon (g, icon, content.withSizeKeepingCentre (iconSize, iconSize), iconInk);
        return;
    }

    const auto f = font (juce::jlimit (10.0f, 13.0f, r.getHeight() * 0.42f), true);
    const float textW = juce::GlyphArrangement::getStringWidth (f, label);
    const float iconsW = icon == Icon::none ? 0.0f : iconSize + 6.0f;
    auto area = content.withSizeKeepingCentre (juce::jmin (content.getWidth(), iconsW + textW), content.getHeight());
    if (icon != Icon::none)
    {
        drawIcon (g, icon, area.removeFromLeft (iconSize).withSizeKeepingCentre (iconSize, iconSize), iconInk);
        area.removeFromLeft (6.0f);
    }
    g.setColour (ink);
    g.setFont (f);
    g.drawFittedText (label, area.toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);
}

//==============================================================================
DrumGrid::DrumGrid (FaseGrooveDrumsProcessor& p) : processor (p)
{
    lights.fill (0.0f);
    refresh();
}

void DrumGrid::refresh()
{
    pattern = processor.displayedPattern();
    for (int l = 0; l < fgd::numLanes; ++l)
        muted[(size_t) l] = processor.isLaneMuted (l);
    playStep = processor.uiStep.load();
    repaint();
}

juce::Rectangle<float> DrumGrid::headerBounds (int lane) const
{
    const float rowH = (float) getHeight() / (float) fgd::numLanes;
    return { 0.0f, rowH * (float) lane, (float) headerWidth, rowH };
}

juce::Rectangle<float> DrumGrid::diceBounds (int lane) const
{
    auto h = headerBounds (lane).reduced (2.0f, 1.5f);
    return h.removeFromRight (h.getHeight()).reduced (4.0f);
}

juce::Rectangle<float> DrumGrid::cellBounds (int lane, int step) const
{
    const float rowH = (float) getHeight() / (float) fgd::numLanes;
    const float cellW = (float) (getWidth() - headerWidth) / (float) FaseGrooveDrumsProcessor::gridSteps;
    return { (float) headerWidth + cellW * (float) step, rowH * (float) lane, cellW, rowH };
}

bool DrumGrid::cellAt (juce::Point<float> p, int& lane, int& step) const
{
    const float rowH = (float) getHeight() / (float) fgd::numLanes;
    const float cellW = (float) (getWidth() - headerWidth) / (float) FaseGrooveDrumsProcessor::gridSteps;
    lane = (int) std::floor (p.y / rowH);
    step = (int) std::floor ((p.x - (float) headerWidth) / cellW);
    return p.x >= (float) headerWidth && juce::isPositiveAndBelow (lane, fgd::numLanes)
        && juce::isPositiveAndBelow (step, FaseGrooveDrumsProcessor::gridSteps);
}

void DrumGrid::paint (juce::Graphics& g)
{
    const int steps = FaseGrooveDrumsProcessor::gridSteps;

    // The playhead column under everything.
    if (juce::isPositiveAndBelow (playStep, steps))
    {
        auto col = cellBounds (0, playStep).withHeight ((float) getHeight());
        g.setColour (Palette::accent2.withAlpha (0.12f));
        g.fillRect (col);
    }

    for (int lane = 0; lane < fgd::numLanes; ++lane)
    {
        const auto colour = laneColour (lane);
        const bool isMuted = muted[(size_t) lane];

        // Header: light, name, MIDI note, die.
        auto h = headerBounds (lane).reduced (2.0f, 1.0f);
        g.setColour (lane == selectedLane ? Palette::control.brighter (0.16f)
                     : lane == hoverLane && hoverStep < 0 ? Palette::control.brighter (0.06f) : Palette::control);
        g.fillRoundedRectangle (h, 4.0f);
        if (lane == selectedLane)
        {
            g.setColour (colour.withAlpha (0.6f));
            g.drawRoundedRectangle (h.reduced (0.5f), 4.0f, 1.0f);
        }
        g.setColour (colour.withAlpha (isMuted ? 0.25f : 0.9f));
        g.fillRoundedRectangle (h.withWidth (3.0f), 1.5f);

        const auto light = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ h.getX() + 12.0f, h.getCentreY() });
        g.setColour (Palette::field);
        g.fillEllipse (light);
        g.setColour (colour.withAlpha (juce::jlimit (0.0f, 1.0f, lights[(size_t) lane])));
        g.fillEllipse (light);

        auto textArea = h.withTrimmedLeft (20.0f).withTrimmedRight (h.getHeight() + 2.0f);
        g.setColour (isMuted ? Palette::dim.withAlpha (0.6f) : Palette::text);
        g.setFont (font (11.0f, true));
        g.drawText (fgd::laneShortName (lane), textArea.removeFromTop (textArea.getHeight() * 0.58f), juce::Justification::bottomLeft);
        g.setColour (Palette::dim);
        g.setFont (font (9.5f));
        const auto sample = processor.samplePath (lane);
        if (sample.isNotEmpty() && ! isMuted)
            g.setColour (laneColour (lane));
        g.drawText (isMuted ? juce::String ("MUTE")
                    : sample.isNotEmpty() ? juce::File (sample).getFileNameWithoutExtension()
                                          : "note " + juce::String (processor.laneNote (lane)),
                    textArea, juce::Justification::topLeft);

        IconButton::drawIcon (g, IconButton::Icon::dice, diceBounds (lane),
                              (lane == hoverLane && hoverDice) ? Palette::accent : Palette::accent.withAlpha (0.55f));

        // Steps.
        const auto& line = pattern.lanes[(size_t) lane];
        for (int step = 0; step < steps; ++step)
        {
            auto c = cellBounds (lane, step).reduced (1.5f, 1.5f);
            const bool beat = (step / 4) % 2 == 0;
            g.setColour (beat ? Palette::panelEdge.withAlpha (0.55f) : Palette::panelEdge.withAlpha (0.32f));
            g.fillRoundedRectangle (c, 3.0f);
            if (lane == hoverLane && step == hoverStep)
            {
                g.setColour (Palette::text.withAlpha (0.12f));
                g.fillRoundedRectangle (c, 3.0f);
            }

            const char symbol = step < (int) line.size() ? line[(size_t) step] : '.';
            if (! fgd::isHit (symbol))
                continue;

            const float alpha = isMuted ? 0.3f : 1.0f;
            switch (symbol)
            {
                case 'X':
                    g.setColour (colour.withAlpha (alpha));
                    g.fillRoundedRectangle (c, 3.0f);
                    g.setColour (juce::Colours::white.withAlpha (0.35f * alpha));
                    g.fillRoundedRectangle (c.withHeight (3.0f).reduced (2.0f, 0.0f).translated (0.0f, 2.0f), 1.5f);
                    break;
                case 'x':
                    g.setColour (colour.withAlpha (0.68f * alpha));
                    g.fillRoundedRectangle (c, 3.0f);
                    break;
                case 'g':
                    g.setColour (colour.withAlpha (0.55f * alpha));
                    g.fillRoundedRectangle (c.withSizeKeepingCentre (c.getWidth() * 0.45f, c.getHeight() * 0.4f), 2.0f);
                    break;
                case 'f':
                    g.setColour (colour.withAlpha (0.8f * alpha));
                    g.fillRoundedRectangle (c.withTrimmedLeft (c.getWidth() * 0.3f), 3.0f);
                    g.fillRoundedRectangle (c.withWidth (c.getWidth() * 0.2f).reduced (0.0f, c.getHeight() * 0.25f), 1.5f);
                    break;
                default:
                    break;
            }

            // A velocity set by hand: a small level mark at the bottom of the cell.
            if (const int v = processor.stepVelocity (lane, step); v > 0)
            {
                g.setColour (Palette::background.withAlpha (0.7f));
                g.fillRect (c.removeFromBottom (3.0f).withTrimmedRight (c.getWidth() * (1.0f - (float) v / 127.0f)));
            }
        }
    }

    // Lines between the half bars (A B C D) and the two bars.
    for (int k = 1; k < fgd::numSegments; ++k)
    {
        const float x = cellBounds (0, k * 8).getX();
        g.setColour (k == 2 ? Palette::dim.withAlpha (0.6f) : Palette::panelEdge.brighter (0.2f));
        g.drawLine (x, 0.0f, x, (float) getHeight(), k == 2 ? 1.2f : 1.0f);
    }
}

void DrumGrid::paintCell (int lane, int step, char symbol)
{
    if (lane == lastPaintLane && step == lastPaintStep)
        return;
    lastPaintLane = lane;
    lastPaintStep = step;
    processor.editStep (lane, step, symbol);
    pattern = processor.displayedPattern();
    repaint();
}

void DrumGrid::mouseDown (const juce::MouseEvent& e)
{
    int lane = 0, step = 0;
    lastPaintLane = lastPaintStep = -1;
    paintSymbol = 0;

    if (! cellAt (e.position, lane, step))
    {
        // The header: the die rolls a new line for the drum, a click mutes, a right-click opens
        // the drum's menu, a double-click shows its velocities.
        if (juce::isPositiveAndBelow (lane, fgd::numLanes) && e.position.x < (float) headerWidth)
        {
            if (e.mods.isPopupMenu())
                showLaneMenu (lane);
            else if (diceBounds (lane).expanded (3.0f).contains (e.position))
            {
                processor.randomizeLane (lane);
                refresh();
            }
            else if (e.getNumberOfClicks() == 2 && onShowVelocity != nullptr)
                onShowVelocity (lane);
            else if (onSelectLane != nullptr)
                onSelectLane (lane);
        }
        return;
    }

    if (e.mods.isPopupMenu())
    {
        showCellMenu (lane, step);
        return;
    }

    const char current = pattern.lanes[(size_t) lane][(size_t) step];
    if (e.mods.isShiftDown())                  paintSymbol = 'X';
    else if (e.mods.isAltDown())               paintSymbol = 'g';
    else if (e.mods.isCommandDown())           paintSymbol = 'f';
    else                                       paintSymbol = fgd::isHit (current) ? '.' : 'x';
    paintCell (lane, step, paintSymbol);
}

void DrumGrid::mouseDrag (const juce::MouseEvent& e)
{
    int lane = 0, step = 0;
    if (paintSymbol != 0 && cellAt (e.position, lane, step))
        paintCell (lane, step, paintSymbol);
}

void DrumGrid::mouseUp (const juce::MouseEvent&)
{
    paintSymbol = 0;
}

void DrumGrid::mouseMove (const juce::MouseEvent& e)
{
    int lane = 0, step = 0;
    const bool onCell = cellAt (e.position, lane, step);
    const int newLane = juce::isPositiveAndBelow (lane, fgd::numLanes) ? lane : -1;
    const int newStep = onCell ? step : -1;
    const bool overDice = newLane >= 0 && ! onCell && diceBounds (newLane).expanded (3.0f).contains (e.position);
    if (newLane != hoverLane || newStep != hoverStep || overDice != hoverDice)
    {
        hoverLane = newLane;
        hoverStep = newStep;
        hoverDice = overDice;
        setTooltip (overDice ? "A new line for this drum only (in the NEW BEAT genre; locked parts stay)"
                    : newLane >= 0 && newStep < 0 ? "Click: pick it (kit panel). Double-click: its velocities. Right-click: mute, sample, MIDI note, level"
                    : juce::String());
        repaint();
    }
}

void DrumGrid::mouseExit (const juce::MouseEvent&)
{
    hoverLane = hoverStep = -1;
    hoverDice = false;
    repaint();
}

void DrumGrid::showCellMenu (int lane, int step)
{
    juce::PopupMenu menu;
    menu.addSectionHeader (juce::String (fgd::laneName (lane)) + " - step " + juce::String (step + 1));
    menu.addItem (1, "Accent (X)");
    menu.addItem (2, "Hit (x)");
    menu.addItem (3, "Ghost note (g)");
    menu.addItem (4, "Flam (f)");
    menu.addItem (5, "Rest");

    juce::Component::SafePointer<DrumGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [safe, lane, step] (int result)
    {
        static const char symbols[] { 'X', 'x', 'g', 'f', '.' };
        if (safe == nullptr || result < 1 || result > 5)
            return;
        safe->processor.editStep (lane, step, symbols[result - 1]);
        safe->refresh();
    });
}

void DrumGrid::showLaneMenu (int lane)
{
    juce::PopupMenu menu;
    menu.addSectionHeader (fgd::laneName (lane));
    menu.addItem (1, "Mute", true, processor.isLaneMuted (lane));
    menu.addItem (3, "New random line (die)");
    menu.addItem (4, "Edit its velocities");
    menu.addItem (2, "Clear this row");
    menu.addSeparator();
    const auto sample = processor.samplePath (lane);
    menu.addItem (5, sample.isEmpty() ? juce::String ("Load your own sample...") : "Sample: " + juce::File (sample).getFileName() + " (change...)");
    menu.addItem (6, "Back to the built-in sound", sample.isNotEmpty());

    // The MIDI note the drum plays: the General MIDI drum notes by name, any other by number.
    juce::PopupMenu notes, others;
    const int current = processor.laneNote (lane);
    for (int n = 35; n <= 82; ++n)
        notes.addItem (1000 + n, juce::String (n) + "  " + fgd::generalMidiDrumName (n), true, n == current);
    for (int octave = 0; octave < 11; ++octave)
    {
        juce::PopupMenu sub;
        for (int n = octave * 12; n < juce::jmin (128, octave * 12 + 12); ++n)
            sub.addItem (1000 + n, juce::MidiMessage::getMidiNoteName (n, true, true, 3) + "  (" + juce::String (n) + ")", true, n == current);
        others.addSubMenu ("Octave " + juce::String (octave - 2), sub);
    }
    notes.addSeparator();
    notes.addSubMenu ("Any note", others);
    notes.addItem (999, "Back to General MIDI (" + juce::String (fgd::defaultNote (lane)) + ")");
    menu.addSubMenu ("MIDI note: " + juce::String (current), notes);

    juce::PopupMenu levels;
    const float level = processor.raw (ParamIDs::level (lane));
    for (int pct : { 25, 50, 75, 100, 125, 150 })
        levels.addItem (2000 + pct, juce::String (pct) + "%", true, juce::roundToInt (level * 100.0f) == pct);
    menu.addSubMenu ("Level: " + juce::String (juce::roundToInt (level * 100.0f)) + "%", levels);

    juce::Component::SafePointer<DrumGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [safe, lane] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        auto& p = safe->processor;
        if (result == 1)                          p.setLaneMuted (lane, ! p.isLaneMuted (lane));
        else if (result == 2)                     p.clearLane (lane);
        else if (result == 3)                     p.randomizeLane (lane);
        else if (result == 4)                     { if (safe->onShowVelocity != nullptr) safe->onShowVelocity (lane); }
        else if (result == 5)                     safe->chooseSample (lane);
        else if (result == 6)                     p.clearSample (lane);
        else if (result == 999)                   p.setParameterFromUI (ParamIDs::note (lane), (float) fgd::defaultNote (lane));
        else if (result >= 1000 && result < 1128) p.setParameterFromUI (ParamIDs::note (lane), (float) (result - 1000));
        else if (result >= 2000)                  p.setParameterFromUI (ParamIDs::level (lane), (float) (result - 2000) / 100.0f);
        safe->refresh();
    });
}

void DrumGrid::chooseSample (int lane)
{
    chooser = std::make_unique<juce::FileChooser> ("Your own sound for the " + juce::String (fgd::laneName (lane)).toLowerCase(),
                                                   juce::File (processor.samplePath (lane)), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
    juce::Component::SafePointer<DrumGrid> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe, lane] (const juce::FileChooser& fc)
    {
        if (safe != nullptr && fc.getResult().existsAsFile())
        {
            safe->processor.loadSample (lane, fc.getResult());
            safe->refresh();
        }
    });
}

int DrumGrid::laneAt (juce::Point<int> p) const
{
    const int lane = (int) std::floor ((float) p.y / ((float) getHeight() / (float) fgd::numLanes));
    return getLocalBounds().contains (p) && juce::isPositiveAndBelow (lane, fgd::numLanes) ? lane : -1;
}

//==============================================================================
VelocityView::VelocityView (FaseGrooveDrumsProcessor& p) : processor (p)
{
    for (int l = 0; l < fgd::numLanes; ++l)
        drumBox.addItem (fgd::laneName (l), l + 1);
    drumBox.setSelectedId (lane + 1, juce::dontSendNotification);
    drumBox.setTooltip ("The drum whose velocities you see and edit");
    drumBox.onChange = [this] { setLane (drumBox.getSelectedId() - 1); };
    addAndMakeVisible (drumBox);

    diceButton.setButtonText ("RANDOM");
    diceButton.iconColour = Palette::accent;
    diceButton.setTooltip ("Random velocities for this drum: a groove shape with a little life on every hit");
    diceButton.onClick = [this] { processor.randomizeVelocities (lane); refresh(); };
    addAndMakeVisible (diceButton);

    shapeBox.setTextWhenNothingSelected ("SHAPE...");
    const auto shapes = FaseGrooveDrumsProcessor::velocityShapeNames();
    for (int i = 0; i < shapes.size(); ++i)
        shapeBox.addItem (shapes[i], i + 1);
    shapeBox.setTooltip ("A velocity shape for this drum's hits (Offbeat push: the classic house hats)");
    shapeBox.onChange = [this]
    {
        if (const int id = shapeBox.getSelectedId(); id > 0)
            processor.applyVelocityShape (lane, id - 1);
        shapeBox.setSelectedId (0, juce::dontSendNotification);
        refresh();
    };
    addAndMakeVisible (shapeBox);

    autoButton.setTooltip ("This drum back to the automatic velocities (from accents and ghost notes)");
    autoButton.onClick = [this] { processor.resetStepVelocities (lane); refresh(); };
    addAndMakeVisible (autoButton);
    autoAllButton.setTooltip ("Every drum back to the automatic velocities");
    autoAllButton.onClick = [this] { processor.resetStepVelocities(); refresh(); };
    addAndMakeVisible (autoAllButton);
    refresh();
}

void VelocityView::setLane (int newLane)
{
    lane = juce::jlimit (0, fgd::numLanes - 1, newLane);
    drumBox.setSelectedId (lane + 1, juce::dontSendNotification);
    refresh();
}

void VelocityView::refresh()
{
    const auto p = processor.displayedPattern();
    for (int i = 0; i < FaseGrooveDrumsProcessor::gridSteps; ++i)
    {
        symbols[(size_t) i] = p.lanes[(size_t) lane][(size_t) i];
        setByHand[(size_t) i] = processor.stepVelocity (lane, i);
        levels[(size_t) i] = processor.effectiveVelocity (lane, i);
    }
    playStep = processor.uiStep.load();
    repaint();
}

void VelocityView::resized()
{
    auto bar = getLocalBounds().removeFromTop (28);
    drumBox.setBounds (bar.removeFromLeft (DrumGrid::headerWidth + 60));
    bar.removeFromLeft (8);
    diceButton.setBounds (bar.removeFromLeft (100));
    bar.removeFromLeft (6);
    shapeBox.setBounds (bar.removeFromLeft (150));
    bar.removeFromLeft (6);
    autoButton.setBounds (bar.removeFromLeft (64));
    bar.removeFromLeft (6);
    autoAllButton.setBounds (bar.removeFromLeft (86));
}

juce::Rectangle<float> VelocityView::stripArea() const
{
    return getLocalBounds().toFloat().withTrimmedTop (38.0f).withHeight (46.0f).withTrimmedLeft ((float) DrumGrid::headerWidth);
}

juce::Rectangle<float> VelocityView::laneArea() const
{
    return getLocalBounds().toFloat().withTrimmedTop (96.0f).withTrimmedBottom (4.0f).withTrimmedLeft ((float) DrumGrid::headerWidth);
}

juce::Rectangle<float> VelocityView::column (int step, juce::Rectangle<float> area) const
{
    const float w = area.getWidth() / (float) FaseGrooveDrumsProcessor::gridSteps;
    return { area.getX() + w * (float) step, area.getY(), w, area.getHeight() };
}

int VelocityView::stepAt (float x) const
{
    const auto area = laneArea();
    const int step = (int) std::floor ((x - area.getX()) / (area.getWidth() / (float) FaseGrooveDrumsProcessor::gridSteps));
    return juce::isPositiveAndBelow (step, FaseGrooveDrumsProcessor::gridSteps) ? step : -1;
}

void VelocityView::paint (juce::Graphics& g)
{
    const auto colour = laneColour (lane);
    const auto strip = stripArea(), area = laneArea();
    const int steps = FaseGrooveDrumsProcessor::gridSteps;

    // Captions on the left.
    g.setColour (Palette::dim);
    g.setFont (font (11.0f, true));
    g.drawText ("STEPS", juce::Rectangle<float> (0.0f, strip.getY(), (float) DrumGrid::headerWidth - 8.0f, strip.getHeight()), juce::Justification::centredLeft);
    g.drawText ("VELOCITY", juce::Rectangle<float> (0.0f, area.getY(), (float) DrumGrid::headerWidth - 8.0f, 20.0f), juce::Justification::centredLeft);
    g.setFont (font (10.0f));
    for (int v : { 127, 96, 64, 32 })
    {
        const float y = area.getBottom() - area.getHeight() * (float) v / 127.0f;
        g.setColour (Palette::dim);
        g.drawText (juce::String (v), juce::Rectangle<float> (0.0f, y - 7.0f, (float) DrumGrid::headerWidth - 8.0f, 14.0f), juce::Justification::centredRight);
        g.setColour (Palette::panelEdge.withAlpha (0.6f));
        g.drawHorizontalLine (juce::roundToInt (y), area.getX(), area.getRight());
    }

    // Playhead.
    if (juce::isPositiveAndBelow (playStep, steps))
    {
        g.setColour (Palette::accent2.withAlpha (0.12f));
        g.fillRect (column (playStep, strip).getUnion (column (playStep, area)));
    }

    for (int i = 0; i < steps; ++i)
    {
        // The drum's steps (click to add / remove a hit).
        auto c = column (i, strip).reduced (1.5f, 2.0f);
        g.setColour ((i / 4) % 2 == 0 ? Palette::panelEdge.withAlpha (0.55f) : Palette::panelEdge.withAlpha (0.32f));
        g.fillRoundedRectangle (c, 3.0f);
        const char symbol = symbols[(size_t) i];
        if (fgd::isHit (symbol))
        {
            g.setColour (colour.withAlpha (symbol == 'X' ? 1.0f : symbol == 'g' ? 0.45f : 0.72f));
            g.fillRoundedRectangle (symbol == 'g' ? c.withSizeKeepingCentre (c.getWidth() * 0.5f, c.getHeight() * 0.4f) : c, 3.0f);
        }

        // Its velocity bar.
        auto col = column (i, area);
        if (i == hoverStep)
        {
            g.setColour (Palette::text.withAlpha (0.05f));
            g.fillRect (col);
        }
        if (! fgd::isHit (symbol))
            continue;
        const float h = col.getHeight() * (float) levels[(size_t) i] / 127.0f;
        auto bar = col.reduced (col.getWidth() * 0.18f, 0.0f).withTop (col.getBottom() - h);
        const bool hand = setByHand[(size_t) i] > 0;
        g.setColour (colour.withAlpha (hand ? 0.95f : 0.38f));
        g.fillRoundedRectangle (bar, 2.0f);
        g.setColour (hand ? Palette::text : colour);
        g.fillRect (bar.withHeight (2.5f));
        if (i == hoverStep)
        {
            g.setColour (Palette::text);
            g.setFont (font (10.5f, true));
            g.drawText (juce::String (levels[(size_t) i]), bar.withY (bar.getY() - 16.0f).withHeight (14.0f).expanded (8.0f, 0.0f),
                        juce::Justification::centred);
        }
    }

    // Half-bar lines.
    for (int k = 1; k < fgd::numSegments; ++k)
    {
        const float x = column (k * 8, area).getX();
        g.setColour (k == 2 ? Palette::dim.withAlpha (0.6f) : Palette::panelEdge.brighter (0.2f));
        g.drawLine (x, strip.getY(), x, area.getBottom(), 1.0f);
    }

    g.setColour (Palette::dim);
    g.setFont (font (10.5f));
    g.drawFittedText ("drag the bars - double-click: automatic\nfaint bars: automatic levels",
                      juce::Rectangle<int> (autoAllButton.getRight() + 12, 0, getWidth() - autoAllButton.getRight() - 12, 28),
                      juce::Justification::centredLeft, 2, 0.8f);
}

void VelocityView::setFromMouse (juce::Point<float> pos)
{
    const int step = stepAt (pos.x);
    if (step < 0 || ! fgd::isHit (symbols[(size_t) step]))
        return;
    const auto area = laneArea();
    const int v = juce::jlimit (1, 127, juce::roundToInt ((area.getBottom() - pos.y) / area.getHeight() * 127.0f));
    processor.setStepVelocity (lane, step, v);
    setByHand[(size_t) step] = v;
    levels[(size_t) step] = v;
    repaint();
}

void VelocityView::mouseDown (const juce::MouseEvent& e)
{
    painting = false;
    if (stripArea().contains (e.position))
    {
        if (const int step = stepAt (e.position.x); step >= 0)
        {
            processor.editStep (lane, step, fgd::isHit (symbols[(size_t) step]) ? '.' : 'x');
            refresh();
        }
        return;
    }
    if (laneArea().expanded (0.0f, 6.0f).contains (e.position))
    {
        painting = true;
        setFromMouse (e.position);
    }
}

void VelocityView::mouseDrag (const juce::MouseEvent& e)
{
    if (painting)
        setFromMouse (e.position);
}

void VelocityView::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int step = stepAt (e.position.x);
    if (step >= 0 && laneArea().contains (e.position))
    {
        processor.setStepVelocity (lane, step, 0);
        refresh();
    }
}

void VelocityView::mouseMove (const juce::MouseEvent& e)
{
    const int step = laneArea().contains (e.position) || stripArea().contains (e.position) ? stepAt (e.position.x) : -1;
    if (step != hoverStep)
    {
        hoverStep = step;
        repaint();
    }
}

void VelocityView::mouseExit (const juce::MouseEvent&)
{
    hoverStep = -1;
    repaint();
}

//==============================================================================
void DragTile::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    const float alpha = isEnabled() ? 1.0f : 0.4f;
    if (filled)
    {
        // The piano's big orange tile: a MIDI clip drawn on it, the title, what it drags.
        g.setColour (isMouseOver() || dragging ? Palette::accent.brighter (0.08f) : Palette::accent);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (Palette::accent.darker (0.3f));
        g.drawRoundedRectangle (r.reduced (4.0f), 6.0f, 1.5f);
        auto clip = juce::Rectangle<float> (64.0f, 22.0f).withCentre ({ r.getCentreX(), r.getY() + r.getHeight() * 0.3f });
        g.setColour (Palette::background.withAlpha (0.85f));
        g.fillRoundedRectangle (clip, 4.0f);
        g.setColour (Palette::accent);
        for (auto bar : { juce::Rectangle<float> (6, 6, 10, 3), { 20, 11, 8, 3 }, { 32, 6, 12, 3 }, { 48, 14, 10, 3 } })
            g.fillRect (bar.translated (clip.getX(), clip.getY()));
        g.setColour (Palette::background);
        g.setFont (font (16.0f, true));
        g.drawFittedText (title, r.withTrimmedTop (r.getHeight() * 0.5f).withHeight (24.0f).toNearestInt(), juce::Justification::centred, 1, 0.7f);
        g.setColour (Palette::background.withAlpha (0.7f));
        g.setFont (font (11.5f));
        g.drawFittedText (subtitle, r.withTrimmedTop (r.getHeight() * 0.5f + 26.0f).withHeight (18.0f).toNearestInt(), juce::Justification::centred, 1, 0.7f);
        return;
    }
    g.setColour (Palette::accent.withAlpha ((isMouseOver() ? 0.22f : 0.12f) * alpha));
    g.fillRoundedRectangle (r, 8.0f);
    const float dashes[] { 5.0f, 4.0f };
    juce::Path outline;
    outline.addRoundedRectangle (r, 8.0f);
    juce::Path dashed;
    juce::PathStrokeType (1.5f).createDashedStroke (dashed, outline, dashes, 2);
    g.setColour (Palette::accent.withAlpha ((dragging ? 1.0f : 0.8f) * alpha));
    g.fillPath (dashed);

    g.setColour (Palette::text.withAlpha (alpha));
    g.setFont (font (juce::jlimit (11.0f, 15.0f, r.getHeight() * 0.22f), true));
    g.drawFittedText (title, r.withTrimmedBottom (r.getHeight() * 0.45f).reduced (6.0f, 0.0f).toNearestInt(), juce::Justification::centredBottom, 1, 0.7f);
    g.setColour (Palette::dim.withAlpha (alpha));
    g.setFont (font (11.0f));
    g.drawFittedText (subtitle, r.withTrimmedTop (r.getHeight() * 0.6f).reduced (6.0f, 0.0f).toNearestInt(), juce::Justification::centredTop, 1, 0.7f);
}

void DragTile::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging || ! isEnabled() || e.getDistanceFromDragStart() < 4 || createFiles == nullptr)
        return;

    const auto files = createFiles();
    if (files.isEmpty())
        return;

    dragging = true;
    repaint();
    juce::Component::SafePointer<DragTile> safe (this);
    juce::DragAndDropContainer::performExternalDragDropOfFiles (files, false, this, [safe]
    {
        if (safe != nullptr)
        {
            safe->dragging = false;
            safe->repaint();
        }
    });
}

//==============================================================================
SongOverlay::SongOverlay (FaseGrooveDrumsProcessor& p) : processor (p)
{
    for (int i = 0; i < FaseGrooveDrumsProcessor::numSongParts; ++i)
    {
        auto* box = barBoxes.add (new juce::ComboBox());
        for (int bars : songBarChoices)
            box->addItem (bars == 0 ? "-" : juce::String (bars), bars + 1);
        box->setTooltip ("Bars of this section (- leaves it out)");
        box->onChange = [this, i, box] { processor.setSongBars (i, box->getSelectedId() - 1); refresh(); };
        addAndMakeVisible (box);

        auto* fresh = newButtons.add (new IconButton (IconButton::Icon::dice));
        fresh->setButtonText ("NEW VARIATION");
        fresh->iconColour = Palette::accent;
        fresh->setTooltip ("This section plays its own version of the groove (hats, percussion and ghost notes move)");
        fresh->onClick = [this, i] { processor.regenerateSongPart (i); refresh(); };
        addAndMakeVisible (fresh);

        auto* original = originalButtons.add (new juce::TextButton ("ORIGINAL"));
        original->setTooltip ("Back to the groove as it is");
        original->onClick = [this, i] { processor.resetSongPart (i); refresh(); };
        addAndMakeVisible (original);

        auto* fill = fillBoxes.add (new juce::ComboBox());
        fill->addItem ("Fill: as set", 1);
        fill->addItem ("No fill", 2);
        for (int f = 0; f < fgd::numFillStyles; ++f)
            fill->addItem ("Fill: " + juce::String (fgd::fillStyleName (f)), f + 3);
        fill->setTooltip ("The fill on the last bar of this section (as set: the Song panel's fills)");
        fill->onChange = [this, i, fill]
        {
            const int id = fill->getSelectedId();
            processor.setSongFill (i, id == 1 ? -1 : id == 2 ? -2 : id - 3);
        };
        addAndMakeVisible (fill);
    }

    for (auto* b : { &allButton, &tracksButton })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (4321);
        addAndMakeVisible (b);
    }
    allButton.setToggleState (true, juce::dontSendNotification);
    allButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    tracksButton.setConnectedEdges (juce::Button::ConnectedOnLeft);

    songTile.setTooltip ("Drag the whole song (every section, with markers) onto a track in your DAW");
    songTile.createFiles = [this]
    {
        const auto file = processor.writeMidiClip (FaseGrooveDrumsProcessor::songArrangement, clipPart());
        return file.existsAsFile() ? juce::StringArray (file.getFullPathName()) : juce::StringArray();
    };
    sectionTile.setTooltip ("Drag only the section picked on the timeline (click a section to pick it)");
    sectionTile.createFiles = [this]
    {
        const auto file = processor.writeSongSectionClip (selected, clipPart());
        return file.existsAsFile() ? juce::StringArray (file.getFullPathName()) : juce::StringArray();
    };
    sectionsTile.setTooltip ("Drag every section at once, each as its own clip: the DAW puts them one after another");
    sectionsTile.createFiles = [this]
    {
        juce::StringArray files;
        for (int i = 0; i < FaseGrooveDrumsProcessor::numSongParts; ++i)
            if (const auto file = processor.writeSongSectionClip (i, clipPart()); file.existsAsFile())
                files.add (file.getFullPathName());
        return files;
    };
    for (auto* t : { &sectionTile, &sectionsTile, &songTile })
        addAndMakeVisible (t);

    saveSongButton.onClick = [this] { saveSong(); };
    saveSectionsButton.onClick = [this] { saveSections(); };
    saveSectionsButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    resetButton.setTooltip ("Default bars, no own variations, fills as set");
    resetButton.onClick = [this] { processor.resetSong(); refresh(); };
    closeButton.onClick = [this] { setVisible (false); };
    for (auto* b : { &saveSongButton, &saveSectionsButton, &resetButton, &closeButton })
        addAndMakeVisible (b);
}

FaseGrooveDrumsProcessor::ClipPart SongOverlay::clipPart() const
{
    return tracksButton.getToggleState() ? FaseGrooveDrumsProcessor::ClipPart::tracks : FaseGrooveDrumsProcessor::ClipPart::all;
}

juce::Rectangle<float> SongOverlay::card() const
{
    return getLocalBounds().toFloat().withSizeKeepingCentre (920.0f, 600.0f);
}

juce::Rectangle<float> SongOverlay::timelineArea() const
{
    return card().reduced (28.0f, 20.0f).withTrimmedTop (64.0f).withHeight (46.0f);
}

juce::Rectangle<int> SongOverlay::rowArea (int part) const
{
    auto inner = card().toNearestInt().reduced (28, 20);
    inner.removeFromTop (64 + 46 + 14 + 18);   // title, timeline, gap, column names
    return inner.removeFromTop (44 * (part + 1)).removeFromBottom (44).reduced (0, 4);
}

void SongOverlay::show()
{
    refresh();
    setVisible (true);
    toFront (true);
}

void SongOverlay::refresh()
{
    const auto parts = processor.songArrangementParts();
    if (! juce::isPositiveAndBelow (selected, (int) parts.size()) || parts[(size_t) selected].bars <= 0)
    {
        selected = -1;
        for (int i = 0; i < (int) parts.size() && selected < 0; ++i)
            if (parts[(size_t) i].bars > 0)
                selected = i;
    }
    sectionTile.setSubtitle (selected >= 0 ? "Section: " + FaseGrooveDrumsProcessor::sectionClipName (selected) : juce::String ("No section"));
    sectionTile.setEnabled (selected >= 0);

    for (int i = 0; i < (int) parts.size() && i < barBoxes.size(); ++i)
    {
        int id = 1;
        for (int bars : songBarChoices)
            if (bars <= parts[(size_t) i].bars) id = bars + 1;
        barBoxes[i]->setSelectedId (id, juce::dontSendNotification);
        originalButtons[i]->setEnabled (parts[(size_t) i].variation != 0);
        newButtons[i]->setEnabled (parts[(size_t) i].bars > 0);
        const int fill = parts[(size_t) i].endFill;
        fillBoxes[i]->setSelectedId (fill == -1 ? 1 : fill == -2 ? 2 : fill + 3, juce::dontSendNotification);
    }
    repaint();
}

void SongOverlay::mouseDown (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
    {
        setVisible (false);
        return;
    }

    // Pick a section: click its block on the timeline.
    const auto line = timelineArea();
    if (! line.contains (e.position))
        return;
    const auto parts = processor.songArrangementParts();
    int total = 0;
    for (const auto& part : parts)
        total += juce::jmax (0, part.bars);
    if (total == 0)
        return;
    float x = line.getX() + 3.0f;
    const float width = line.getWidth() - 6.0f;
    for (int i = 0; i < (int) parts.size(); ++i)
    {
        if (parts[(size_t) i].bars <= 0)
            continue;
        const float w = width * (float) parts[(size_t) i].bars / (float) total;
        if (e.position.x >= x && e.position.x < x + w)
        {
            selected = i;
            refresh();
            return;
        }
        x += w;
    }
}

void SongOverlay::resized()
{
    for (int i = 0; i < barBoxes.size(); ++i)
    {
        auto row = rowArea (i);
        row.removeFromLeft (190);   // section name
        barBoxes[i]->setBounds (row.removeFromLeft (76));
        row.removeFromLeft (14);
        newButtons[i]->setBounds (row.removeFromLeft (160));
        row.removeFromLeft (6);
        originalButtons[i]->setBounds (row.removeFromLeft (96));
        row.removeFromLeft (14);
        fillBoxes[i]->setBounds (row.removeFromLeft (220));
    }

    auto inner = card().toNearestInt().reduced (28, 20);
    auto bottom = inner.removeFromBottom (40);
    closeButton.setBounds (bottom.removeFromRight (80));
    bottom.removeFromRight (8);
    saveSectionsButton.setBounds (bottom.removeFromRight (190));
    bottom.removeFromRight (8);
    saveSongButton.setBounds (bottom.removeFromRight (150));
    bottom.removeFromRight (8);
    resetButton.setBounds (bottom.removeFromRight (100));
    bottom.removeFromRight (8);
    allButton.setBounds (bottom.removeFromLeft (96));
    tracksButton.setBounds (bottom.removeFromLeft (176));

    inner.removeFromBottom (12);
    auto tiles = inner.removeFromBottom (68);
    const int w = (tiles.getWidth() - 2 * 12) / 3;
    sectionTile.setBounds (tiles.removeFromLeft (w));
    tiles.removeFromLeft (12);
    sectionsTile.setBounds (tiles.removeFromLeft (w));
    tiles.removeFromLeft (12);
    songTile.setBounds (tiles);
}

void SongOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));
    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (28.0f, 20.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText ("SONG ARRANGEMENT", inner.removeFromTop (28.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawFittedText ("Bars of each section (- leaves it out), its own variation of the groove and the fill that leads out of it. "
                      "Drag the whole song, or click a section on the timeline and drag just that one.",
                      inner.removeFromTop (30.0f).toNearestInt(), juce::Justification::topLeft, 2);

    const auto parts = processor.songArrangementParts();
    int total = 0;
    for (const auto& part : parts)
        total += juce::jmax (0, part.bars);

    // The timeline: each section as long as its bars.
    {
        auto line = timelineArea();
        g.setColour (Palette::background);
        g.fillRoundedRectangle (line, 8.0f);
        const juce::Colour colours[] { Palette::accent, juce::Colour (0xff6f8fd6), juce::Colour (0xff9b7fd6), juce::Colour (0xffd6b25f),
                                       Palette::accent2, juce::Colour (0xff6fc8a8) };
        float x = line.getX() + 3.0f;
        const float width = line.getWidth() - 6.0f;
        for (int i = 0; i < (int) parts.size(); ++i)
        {
            const auto& part = parts[(size_t) i];
            if (part.bars <= 0 || total == 0)
                continue;
            const float w = width * (float) part.bars / (float) total;
            auto block = juce::Rectangle<float> (x, line.getY() + 3.0f, w, line.getHeight() - 6.0f).reduced (1.5f, 0.0f);
            const auto colour = colours[(size_t) juce::jlimit (0, 5, (int) part.section)];
            g.setColour (colour.withAlpha (part.variation != 0 ? 1.0f : 0.8f));
            g.fillRoundedRectangle (block, 6.0f);
            if (i == selected)
            {
                g.setColour (Palette::text);
                g.drawRoundedRectangle (block.reduced (1.0f), 6.0f, 2.5f);
            }
            g.setColour (Palette::background);
            g.setFont (font (12.0f, true));
            auto text = block.reduced (6.0f, 3.0f);
            g.drawFittedText (fgd::sectionName ((int) part.section), text.removeFromTop (text.getHeight() * 0.55f).toNearestInt(),
                              juce::Justification::centredLeft, 1, 0.6f);
            g.setFont (font (10.5f));
            g.drawFittedText (juce::String (part.bars) + " bars", text.toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);
            x += w;
        }
    }

    // Column names.
    {
        auto header = rowArea (0).toFloat().translated (0.0f, -22.0f).withHeight (16.0f);
        g.setColour (Palette::dim);
        g.setFont (font (10.5f, true));
        auto col = [&] (float w, const juce::String& text) { g.drawText (text, header.removeFromLeft (w), juce::Justification::centredLeft); };
        col (190.0f, "SECTION");
        col (90.0f, "BARS");
        col (276.0f, "GROOVE");
        col (220.0f, "FILL OUT");
    }

    for (int i = 0; i < (int) parts.size(); ++i)
    {
        const auto row = rowArea (i).toFloat();
        g.setColour (Palette::background.withAlpha (i % 2 == 0 ? 1.0f : 0.6f));
        g.fillRoundedRectangle (row.expanded (8.0f, 2.0f), 6.0f);
        auto label = row.withWidth (180.0f);
        g.setColour (parts[(size_t) i].bars > 0 ? Palette::text : Palette::dim);
        g.setFont (font (14.0f, true));
        g.drawText (FaseGrooveDrumsProcessor::sectionClipName (i), label.removeFromTop (label.getHeight() * 0.56f), juce::Justification::bottomLeft);
        g.setFont (font (10.5f));
        g.setColour (parts[(size_t) i].variation != 0 ? Palette::accent : Palette::dim);
        g.drawText (parts[(size_t) i].bars <= 0 ? "left out" : parts[(size_t) i].variation != 0 ? "own variation" : "the groove as it is",
                    label, juce::Justification::topLeft);
    }

    // Length of the whole song.
    const double bpm = processor.exportBpm();
    const int seconds = bpm > 0.0 ? juce::roundToInt (total * 4 * 60.0 / bpm) : 0;
    auto info = card().reduced (28.0f, 20.0f).withTrimmedBottom (40.0f + 12.0f + 68.0f + 6.0f).removeFromBottom (20.0f);
    g.setColour (Palette::text);
    g.setFont (font (13.0f, true));
    g.drawText ("Total: " + juce::String (total) + " bars   " + juce::String (seconds / 60) + ":" + juce::String (seconds % 60).paddedLeft ('0', 2)
                    + "  @ " + juce::String (bpm, 1) + " BPM",
                info, juce::Justification::centredLeft);
}

void SongOverlay::saveSong()
{
    auto folder = FaseGrooveDrumsProcessor::clipFolder();
    folder.createDirectory();
    const auto part = clipPart();
    chooser = std::make_unique<juce::FileChooser> ("Save the whole song",
                                                   folder.getChildFile (processor.clipFileName (FaseGrooveDrumsProcessor::songArrangement, part)), "*.mid");
    juce::Component::SafePointer<SongOverlay> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe, part] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (safe == nullptr || file == juce::File())
            return;
        file = file.withFileExtension ("mid");
        file.deleteFile();
        juce::FileOutputStream stream (file);
        if (stream.openedOk())
            safe->processor.createMidiClip (FaseGrooveDrumsProcessor::songArrangement, part).writeTo (stream);
    });
}

void SongOverlay::saveSections()
{
    const auto part = clipPart();
    chooser = std::make_unique<juce::FileChooser> ("Choose a folder for the sections", FaseGrooveDrumsProcessor::clipFolder());
    juce::Component::SafePointer<SongOverlay> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [safe, part] (const juce::FileChooser& fc)
    {
        const auto folder = fc.getResult();
        if (safe == nullptr || folder == juce::File())
            return;
        const int written = safe->processor.saveSongSections (folder, part);
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Song sections",
                                                juce::String (written) + " .mid files saved in\n" + folder.getFullPathName());
    });
}

//==============================================================================
AboutPanel::AboutPanel()
{
    artistLogo = Branding::artistLogo();

    auto link = [this] (juce::TextButton& b, const char* url, juce::Colour colour)
    {
        addChildComponent (b);
        b.setVisible (juce::String (url).isNotEmpty());
        b.setColour (juce::TextButton::buttonColourId, colour);
        b.onClick = [url] { juce::URL (url).launchInDefaultBrowser(); };
    };
    link (sourceButton, Branding::sourceUrl, Palette::control);
    link (kofiButton, Branding::kofiUrl, Palette::accent);
    link (paypalButton, Branding::paypalUrl, Palette::accent2);
    link (spotifyButton, Branding::spotifyUrl, juce::Colour (0xff1db954));

    addAndMakeVisible (closeButton);
    closeButton.onClick = [this] { setVisible (false); };
}

juce::Rectangle<int> AboutPanel::card() const
{
    return getLocalBounds().withSizeKeepingCentre (600, 560);
}

void AboutPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.7f));
    auto r = card();
    drawPanel (g, r, {});
    r.reduce (28, 24);

    Branding::drawFullLogo (g, r.removeFromTop (64).toFloat());
    r.removeFromTop (10);

    g.setColour (Palette::dim);
    g.setFont (font (14.0f));
    g.drawText (Branding::tagline, r.removeFromTop (22), juce::Justification::centredLeft);
    g.drawText ("Version " JucePlugin_VersionString, r.removeFromTop (20), juce::Justification::centredLeft);
    r.removeFromTop (10);

    auto row = [&] (const juce::String& what, const juce::String& who)
    {
        auto line = r.removeFromTop (22);
        g.setColour (Palette::dim);
        g.setFont (font (13.0f));
        g.drawText (what, line.removeFromLeft (230), juce::Justification::centredLeft);
        g.setColour (Palette::text);
        g.setFont (font (13.5f, true));
        g.drawText (who, line, juce::Justification::centredLeft);
    };
    row ("Idea, music direction and sound", Branding::artist);
    row ("Code", "Claude (Anthropic)");
    row ("Framework", "JUCE");
    row ("Licence", Branding::licence);

    r.removeFromTop (10);
    g.setColour (Palette::accent);
    g.setFont (font (12.0f, true));
    g.drawText ("SHORTCUTS", r.removeFromTop (18), juce::Justification::centredLeft);
    g.setColour (Palette::text.withAlpha (0.85f));
    g.setFont (font (12.0f));
    for (const auto* line : { "Space: play / pause      Ctrl+Z: undo      Ctrl+Shift+Z / Ctrl+Y: redo",
                              "Grid: click hit, Shift accent, Alt ghost, Ctrl/Cmd flam, drag to paint",
                              "Drum name: click mute, double-click velocities, right-click note / level",
                              "Drop a .mid file on the window: its drums go into the grid",
                              "Drop a sample on a drum's row or pad: your own sound for it",
                              "Drop an audio drum loop anywhere else: it becomes the grid" })
        g.drawText (line, r.removeFromTop (18), juce::Justification::centredLeft);

    r.removeFromTop (8);
    if (artistLogo.isValid())
        g.drawImageWithin (artistLogo, r.getX(), r.getY(), r.getWidth(), 60, juce::RectanglePlacement::centred);
}

void AboutPanel::resized()
{
    auto r = card().reduced (28, 24);
    auto buttons = r.removeFromBottom (34);
    const int w = (buttons.getWidth() - 4 * 8) / 5;
    for (auto* b : { &sourceButton, &kofiButton, &paypalButton, &spotifyButton, &closeButton })
    {
        b->setBounds (buttons.removeFromLeft (w));
        buttons.removeFromLeft (8);
    }
}

void AboutPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! card().contains (e.getPosition()))
        setVisible (false);
}

//==============================================================================
PartCard::PartCard (FaseGrooveDrumsProcessor& p, int pos)
    : processor (p), position (pos), combo (p.apvts, ParamIDs::segment (pos), juce::String())
{
    combo.setTip ("Which part (A-D) plays in this half bar: e.g. A A A B");
    addAndMakeVisible (combo);
    dice.iconColour = Palette::accent;
    dice.setTooltip ("New random content for this part (in the beat genre)");
    dice.onClick = [this] { processor.randomizeSegment (processor.segmentAt (position)); };
    addAndMakeVisible (dice);
    lock.setTooltip ("Lock this part: the dice leave it alone");
    lock.onClick = [this]
    {
        const int part = processor.segmentAt (position);
        processor.setSegmentLocked (part, ! processor.isSegmentLocked (part));
    };
    addAndMakeVisible (lock);
}

void PartCard::refresh (const fgd::DrumPattern& shown, int playStep)
{
    pattern = shown;
    playing = playStep >= 0 && playStep / 8 == position;
    locked = processor.isSegmentLocked (processor.segmentAt (position));
    lock.setIcon (locked ? IconButton::Icon::lock : IconButton::Icon::unlock);
    lock.setToggleState (locked, juce::dontSendNotification);
    dice.setEnabled (! locked);
    combo.refresh();
    repaint();
}

void PartCard::resized()
{
    auto r = getLocalBounds().reduced (6);
    auto top = r.removeFromTop (22);
    dice.setBounds (top.removeFromLeft (26));
    lock.setBounds (top.removeFromRight (26));
    combo.setBounds (r.removeFromBottom (26));
}

void PartCard::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (playing ? Palette::accent.withAlpha (0.1f) : Palette::field);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (playing ? Palette::accent : locked ? Palette::dim : Palette::panelEdge);
    g.drawRoundedRectangle (r, 6.0f, playing ? 1.5f : 1.0f);
    // The bar on top, as on the chord slots (green when it plays).
    g.setColour (playing ? Palette::cymbals : Palette::panelEdge.brighter (0.2f));
    g.fillRoundedRectangle (r.reduced (10.0f, 0.0f).withHeight (3.0f).translated (0.0f, 2.0f), 1.5f);

    const int part = processor.segmentAt (position);
    g.setColour (Palette::dim);
    g.setFont (font (11.0f, true));
    g.drawText (juce::String (position + 1) + "   " + (position < 2 ? "BAR 1" : "BAR 2") + (position % 2 == 0 ? " a" : " b"),
                getLocalBounds().reduced (34, 6).removeFromTop (22), juce::Justification::centred);

    // The part's letter and a picture of its hits: kick, snare + clap, hats, percussion.
    auto body = getLocalBounds().reduced (8).withTrimmedTop (24).withTrimmedBottom (30).toFloat();
    g.setColour (locked ? Palette::dim : Palette::text);
    g.setFont (font (24.0f, true));
    g.drawText (juce::String::charToString ((juce::juce_wchar) ('A' + part)), body.removeFromLeft (30.0f), juce::Justification::centred);

    if (pattern.length() < FaseGrooveDrumsProcessor::gridSteps)
        return;
    struct Row { std::initializer_list<fgd::Lane> lanes; int colourLane; };
    const Row rows[] { { { fgd::Lane::closedHat, fgd::Lane::openHat, fgd::Lane::ride }, (int) fgd::Lane::closedHat },
                       { { fgd::Lane::shaker, fgd::Lane::tambourine, fgd::Lane::conga, fgd::Lane::cowbell, fgd::Lane::rim, fgd::Lane::tom }, (int) fgd::Lane::shaker },
                       { { fgd::Lane::snare, fgd::Lane::clap }, (int) fgd::Lane::snare },
                       { { fgd::Lane::kick }, (int) fgd::Lane::kick } };
    const float rowH = body.getHeight() / 4.0f, cellW = body.getWidth() / 8.0f;
    for (int row = 0; row < 4; ++row)
        for (int i = 0; i < 8; ++i)
        {
            char strongest = '.';
            for (auto lane : rows[row].lanes)
            {
                const char c = pattern[lane][(size_t) (position * 8 + i)];
                if (c == 'X' || (fgd::isHit (c) && strongest == '.') || (c != 'g' && fgd::isHit (c) && strongest == 'g'))
                    strongest = c;
            }
            auto cell = juce::Rectangle<float> (body.getX() + cellW * (float) i, body.getY() + rowH * (float) row, cellW, rowH).reduced (1.5f);
            g.setColour (Palette::panelEdge.withAlpha (0.45f));
            g.fillRoundedRectangle (cell, 1.5f);
            if (fgd::isHit (strongest))
            {
                g.setColour (laneColour (rows[row].colourLane).withAlpha (strongest == 'X' ? 1.0f : strongest == 'g' ? 0.4f : 0.75f));
                g.fillRoundedRectangle (cell, 1.5f);
            }
        }
}

//==============================================================================
juce::Rectangle<float> DrumPads::padBounds (int lane) const
{
    // 4 x 4: the kit in its usual order, the three cymbals... and percussion around them.
    const float w = (float) getWidth() / 4.0f, h = (float) getHeight() / 4.0f;
    const int row = lane / 4, col = lane % 4;
    auto r = juce::Rectangle<float> (w * (float) col, h * (float) row, w, h).reduced (4.0f);
    if (lane == fgd::numLanes - 1)   // the crash: the whole last row
        r = juce::Rectangle<float> (0.0f, h * 3.0f, (float) getWidth(), h).reduced (4.0f);
    return r;
}

void DrumPads::paint (juce::Graphics& g)
{
    for (int lane = 0; lane < fgd::numLanes; ++lane)
    {
        const auto r = padBounds (lane);
        const auto colour = laneColour (lane);
        const bool muted = processor.isLaneMuted (lane);
        const float light = juce::jlimit (0.0f, 1.0f, lights[(size_t) lane]);

        g.setColour (Palette::control.interpolatedWith (colour, muted ? 0.0f : 0.12f + 0.75f * light));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (lane == selectedLane ? Palette::text : colour.withAlpha (muted ? 0.2f : 0.55f));
        g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, lane == selectedLane ? 2.0f : 1.0f);

        g.setColour (light > 0.5f ? Palette::background : (muted ? Palette::dim.withAlpha (0.6f) : Palette::text));
        g.setFont (font (11.0f, true));
        auto text = r.reduced (6.0f, 4.0f);
        g.drawText (fgd::laneShortName (lane), text.removeFromTop (text.getHeight() * 0.6f), juce::Justification::bottomLeft);
        g.setFont (font (9.5f));
        g.setColour (light > 0.5f ? Palette::background.withAlpha (0.7f) : Palette::dim);
        g.drawText (muted ? juce::String ("MUTE") : processor.samplePath (lane).isNotEmpty() ? juce::String ("SAMPLE")
                                                                                           : juce::String (processor.laneNote (lane)),
                    text, juce::Justification::topLeft);
    }
}

void DrumPads::mouseDown (const juce::MouseEvent& e)
{
    for (int lane = 0; lane < fgd::numLanes; ++lane)
        if (padBounds (lane).contains (e.position))
        {
            if (e.mods.isPopupMenu())
                processor.setLaneMuted (lane, ! processor.isLaneMuted (lane));
            else
            {
                processor.audition (lane, 70 + juce::jlimit (0, 57, juce::roundToInt ((e.position.y - padBounds (lane).getY()) / padBounds (lane).getHeight() * 57.0f)));
                if (onSelectLane != nullptr)
                    onSelectLane (lane);
            }
            repaint();
            return;
        }
}

//==============================================================================
FaseGrooveDrumsEditor::FaseGrooveDrumsEditor (FaseGrooveDrumsProcessor& p)
    : AudioProcessorEditor (&p), processor (p), tooltips (this, 600),
      grooveCombo (p.apvts, ParamIDs::pattern, "Groove"),
      genreCombo (p.apvts, ParamIDs::genre, "Beat genre"),
      pads (p),
      sectionCombo (p.apvts, ParamIDs::section, "Song section"),
      fillsCombo (p.apvts, ParamIDs::fills, "Fills"),
      variationCombo (p.apvts, ParamIDs::variation, "Variation"),
      fillStyleCombo (p.apvts, ParamIDs::fillStyle, "Fill style"),
      swing (p.apvts, ParamIDs::swing, "Swing"),
      humanize (p.apvts, ParamIDs::humanize, "Humanize"),
      velocity (p.apvts, ParamIDs::velocity, "Velocity"),
      accents (p.apvts, ParamIDs::accents, "Accents"),
      kitCombo (p.apvts, ParamIDs::kit, "Kit"),
      channelCombo (p.apvts, ParamIDs::midiChannel, "MIDI ch."),
      tone (p.apvts, ParamIDs::tone, "Tone"),
      reverb (p.apvts, ParamIDs::reverb, "Reverb"),
      volume (p.apvts, ParamIDs::volume, "Volume"),
      grid (p), velocityView (p)
{
    setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (content);
    content.setWantsKeyboardFocus (true);

    auto show = [this] (juce::Component& c) { content.addAndMakeVisible (c); };

    // ---- Header: presets, total random, undo / redo, full screen; style, reset, options ----
    presetsButton.setTooltip ("Your presets: save everything, load it in any project");
    presetsButton.onClick = [this] { showPresetsMenu(); };
    totalRandomButton.setButtonText ("TOTAL");
    totalRandomButton.iconColour = Palette::accent;
    totalRandomButton.setTooltip ("Total random: style, groove or new beat, swing, feel, fills and kit all at once (undo with the arrow)");
    totalRandomButton.onClick = [this] { processor.totalRandom(); };
    undoButton.setTooltip ("Undo (Ctrl+Z)");
    undoButton.onClick = [this] { undo(); };
    undoButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    redoButton.setTooltip ("Redo (Ctrl+Shift+Z)");
    redoButton.onClick = [this] { redo(); };
    redoButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    fullScreenButton.setTooltip ("Full screen");
    fullScreenButton.onClick = [this] { toggleFullScreen(); };
    styleButton.setTooltip ("A groove, its swing, feel, fills and kit for a genre, in one click");
    styleButton.onClick = [this]
    {
        juce::PopupMenu menu;
        menu.addSectionHeader ("One-click setups by genre");
        const auto names = FaseGrooveDrumsProcessor::styleNames();
        for (int i = 0; i < names.size(); ++i)
            menu.addItem (i + 1, names[i]);
        juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&styleButton), [safe] (int result)
        {
            if (safe != nullptr && result > 0)
                safe->processor.applyStyle (result - 1);
        });
    };
    resetButton.setTooltip ("Every setting back to its default (undo with the arrow)");
    resetButton.onClick = [this] { processor.resetToDefaults(); };
    optionsButton.setButtonText ("OPTIONS");
    optionsButton.setTooltip ("About, credits, shortcuts and links");
    optionsButton.onClick = [this] { about.setVisible (true); about.toFront (false); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &presetsButton, &totalRandomButton, &undoButton, &redoButton,
                                                                        &fullScreenButton, &styleButton, &resetButton, &optionsButton })
        show (*c);

    // ---- Transport & tempo ----
    rewindButton.setTooltip ("Back to the start");
    rewindButton.onClick = [this] { processor.previewToStart(); };
    rewindButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    playButton.setTooltip ("Play / pause the groove without the DAW (space bar)");
    playButton.onClick = [this] { processor.previewToggle(); };
    playButton.onColour = Palette::accent2;
    playButton.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    stopButton.setTooltip ("Stop and go back to the start");
    stopButton.onClick = [this] { processor.previewStop(); };
    stopButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    sequencerToggle.setTooltip ("Off: the groove doesn't play with the DAW (e.g. while the dragged clip plays on this track). The play button still previews");
    sequencerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sequencer, sequencerToggle);
    syncButton.setClickingTogglesState (true);
    syncButton.setTooltip ("On: the DAW's tempo and position. Off: your own tempo (still started by the DAW's play)");
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::tempoSync, syncButton);
    tempoSlider.setSliderStyle (juce::Slider::LinearBar);
    tempoSlider.setTooltip ("Tempo when SYNC DAW is off (drag or double-click to type)");
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, ParamIDs::tempo, tempoSlider);
    for (juce::Component* c : std::initializer_list<juce::Component*> { &rewindButton, &playButton, &stopButton, &sequencerToggle, &syncButton, &tempoSlider })
        show (*c);

    // ---- Groove row: presets, random, genre, density and the parts A-D ----
    {
        auto& box = grooveCombo.comboBox();
        box.clear (juce::dontSendNotification);
        const auto& presets = fgd::drumPatterns();
        for (const auto* category : fgd::grooveCategories())
        {
            box.addSectionHeading (category);
            for (size_t i = 0; i < presets.size(); ++i)
                if (presets[i].category == category)
                    box.addItem (presets[i].name, (int) i + 1);
        }
        box.addSectionHeading ("Your beats");
        box.addItem ("Random beat", fgd::randomPatternIndex() + 1);
        box.addItem ("User pattern (edited on the grid)", fgd::userPatternIndex() + 1);
        grooveCombo.refresh();
    }
    grooveCombo.setTip ("The groove, by genre (the same genres as Fase Groove Piano). Edit any groove on the grid");
    prevGroove.setTooltip ("Previous groove");
    prevGroove.onClick = [this] { stepGroove (-1); };
    prevGroove.setConnectedEdges (juce::Button::ConnectedOnRight);
    nextGroove.setTooltip ("Next groove");
    nextGroove.onClick = [this] { stepGroove (1); };
    nextGroove.setConnectedEdges (juce::Button::ConnectedOnLeft);
    genreCombo.setTip ("The genre the dice write in");
    newBeatButton.setButtonText ("RANDOM");
    newBeatButton.iconColour = Palette::accent;
    newBeatButton.setTooltip ("A brand new random beat in the chosen genre (locked parts stay)");
    newBeatButton.onClick = [this] { processor.rollNewBeat(); };
    densitySlider.setSliderStyle (juce::Slider::LinearBar);
    densitySlider.setTooltip ("How busy the dice are: from kick, clap and hats to 16ths and percussion");
    densityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, ParamIDs::density, densitySlider);
    for (juce::Component* c : std::initializer_list<juce::Component*> { &grooveCombo, &prevGroove, &nextGroove, &genreCombo, &newBeatButton, &densitySlider })
        show (*c);
    for (int i = 0; i < fgd::numSegments; ++i)
    {
        partCards[i] = std::make_unique<PartCard> (p, i);
        show (*partCards[i]);
    }

    // ---- Kit & sounds: the drum picked ----
    for (int l = 0; l < fgd::numLanes; ++l)
        drumBox.addItem (fgd::laneName (l), l + 1);
    drumBox.setTooltip ("The drum you edit here (or click a pad, or its name on the grid)");
    drumBox.onChange = [this] { selectLane (drumBox.getSelectedId() - 1); };
    for (int n = 0; n < 128; ++n)
    {
        if (n == 35) noteBox.addSectionHeading ("General MIDI drums");
        if (n == 83) noteBox.addSectionHeading ("Other notes");
        if (n == 0)  noteBox.addSectionHeading ("Low notes");
        const juce::String gm (fgd::generalMidiDrumName (n));
        noteBox.addItem (juce::String (n) + "  " + (gm.isNotEmpty() ? gm : juce::MidiMessage::getMidiNoteName (n, true, true, 3)), n + 1);
    }
    noteBox.setTooltip ("The MIDI note this drum plays (to match your drum rack or sampler)");
    noteBox.onChange = [this]
    {
        if (const int id = noteBox.getSelectedId(); id > 0)
            processor.setParameterFromUI (ParamIDs::note (selectedLane), (float) (id - 1));
    };
    laneDiceButton.iconColour = Palette::accent;
    laneDiceButton.setTooltip ("A new line for this drum only (locked parts stay)");
    laneDiceButton.onClick = [this] { processor.randomizeLane (selectedLane); };
    muteButton.setClickingTogglesState (false);
    muteButton.setTooltip ("Mute this drum (MIDI out, guide kit and clips)");
    muteButton.onClick = [this] { processor.setLaneMuted (selectedLane, ! processor.isLaneMuted (selectedLane)); };
    loadSampleButton.setTooltip ("Your own sound for this drum: a WAV, AIFF, FLAC or MP3 (or drop the file on its row)");
    loadSampleButton.onClick = [this]
    {
        const int lane = selectedLane;
        sampleChooser = std::make_unique<juce::FileChooser> ("Your own sound for the " + juce::String (fgd::laneName (lane)).toLowerCase(),
                                                             juce::File (processor.samplePath (lane)), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
        juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
        sampleChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe, lane] (const juce::FileChooser& fc)
        {
            if (safe != nullptr && fc.getResult().existsAsFile())
                safe->showMessage (safe->processor.loadSample (lane, fc.getResult()) ? juce::String (fgd::laneName (lane)) + ": " + fc.getResult().getFileName()
                                                                                    : "Could not read " + fc.getResult().getFileName());
        });
    };
    builtInButton.setTooltip ("Back to the built-in sound");
    builtInButton.onClick = [this] { processor.clearSample (selectedLane); };
    levelKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    levelKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 15);
    levelKnob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    levelKnob.setTooltip ("This drum's level: its velocities (and so the clips) go up or down");
    for (juce::Component* c : std::initializer_list<juce::Component*> { &drumBox, &noteBox, &laneDiceButton, &muteButton, &loadSampleButton, &builtInButton, &levelKnob })
        show (*c);

    // ---- Pads ----
    pads.onSelectLane = [this] (int lane) { selectLane (lane); };
    show (pads);

    // ---- Groove panel ----
    sectionCombo.setTip ("Song section: Intro / Outro (kick and hats), Break (no kick), Build-up (snare roll), Drop (crash every 8 bars)");
    fillsCombo.setTip ("A fill at the end of every N bars");
    fillStyleCombo.setTip ("What the fill plays. Mix: a different one now and then");
    variationCombo.setTip ("Every 4 or 8 bars the last bar plays a variation (a hat lift, a pick-up...)");
    swing.setTip ("Pushes every other 16th later: 0% straight, 100% full shuffle");
    humanize.setTip ("Small timing and velocity changes, like a drummer (the kick stays tight)");
    velocity.setTip ("Overall velocity of the hits");
    accents.setTip ("The accent groove of hats, shaker, ride and tambourine: the offbeat sings, the 16ths are softer (0%: flat)");
    crashButton.setClickingTogglesState (true);
    crashButton.setTooltip ("A crash on the bar after every fill");
    crashAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::crash, crashButton);
    newFeelButton.setTooltip ("Another humanization (the same every time you play it)");
    newFeelButton.onClick = [this] { processor.newFeel(); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &sectionCombo, &fillsCombo, &variationCombo, &fillStyleCombo,
                                                                        &swing, &humanize, &velocity, &accents, &crashButton, &newFeelButton })
        show (*c);

    // ---- Guide sound & MIDI out ----
    soundToggle.setTooltip ("The guide kit: hear the groove here. Off: MIDI only, for your own drums");
    soundAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sound, soundToggle);
    channelCombo.setTip ("The MIDI channel of the notes (10 is the drum channel of General MIDI)");
    for (juce::Component* c : std::initializer_list<juce::Component*> { &soundToggle, &kitCombo, &channelCombo, &tone, &reverb, &volume })
        show (*c);

    // ---- Edit: step grid / velocity ----
    gridTab.setConnectedEdges (juce::Button::ConnectedOnRight);
    velocityTab.setConnectedEdges (juce::Button::ConnectedOnLeft);
    gridTab.onClick = [this] { setView (false); };
    velocityTab.setTooltip ("The velocity of every hit, one drum at a time (what makes hats groove)");
    velocityTab.onClick = [this] { setView (true); };
    newRhythmButton.setButtonText ("BEAT");
    newRhythmButton.iconColour = Palette::accent;
    newRhythmButton.setTooltip ("A brand new random beat (locked parts stay)");
    newRhythmButton.onClick = [this] { processor.rollNewBeat(); };
    myGroovesButton.setTooltip ("Save the grid as your own groove, or load one");
    myGroovesButton.onClick = [this] { showGroovesMenu(); };
    importButton.setButtonText ("IMPORT...");
    importButton.setTooltip ("A MIDI file's drums, or an audio drum loop, into the grid (or drop the file on the window)");
    importButton.onClick = [this]
    {
        juce::PopupMenu menu;
        menu.addItem (1, "Import the drums of a MIDI file...");
        menu.addItem (2, "Import an audio drum loop (WAV, AIFF...)...");
        juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&importButton), [safe] (int result)
        {
            if (safe == nullptr) return;
            if (result == 1) safe->importMidi();
            if (result == 2) safe->importAudio();
        });
    };
    clearButton.setTooltip ("Empty the grid and start your own beat");
    clearButton.onClick = [this] { processor.clearGrid(); grid.refresh(); velocityView.refresh(); };
    grid.onShowVelocity = [this] (int lane) { selectLane (lane); setView (true); };
    grid.onSelectLane = [this] (int lane) { selectLane (lane); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &gridTab, &velocityTab, &newRhythmButton, &myGroovesButton, &importButton, &clearButton, &grid })
        show (*c);
    content.addChildComponent (velocityView);

    // ---- Export MIDI ----
    notesBox.addItem ("All drums", 1);
    notesBox.addItem ("One track per drum", 2);
    notesBox.setSelectedId (1, juce::dontSendNotification);
    notesBox.setTooltip ("One drum track, or one MIDI track per drum (kick, snare, hats...)");
    const char* lengths[] { "Full loop", "1 bar", "2 bars", "4 bars", "8 bars", "16 bars", "Whole song" };
    for (int i = 0; i < (int) std::size (lengths); ++i)
        lengthBox.addItem (lengths[i], i + 1);
    lengthBox.setSelectedId (1, juce::dontSendNotification);
    lengthBox.setTooltip ("Length of the clip. Full loop: until groove, fills and variations come round. "
                          "Whole song: the sections of the song window, with markers");
    dragTile.filled = true;
    dragTile.createFiles = [this]
    {
        const auto file = processor.writeMidiClip (selectedClipBars(), selectedClipPart());
        return file.existsAsFile() ? juce::StringArray (file.getFullPathName()) : juce::StringArray();
    };
    saveButton.setTooltip ("Save the clip as a .mid file");
    saveButton.onClick = [this] { saveMidiFile(); };
    songButton.setTooltip ("The song window: bars, variation and fill of each section; drag or save the whole song or each section");
    songButton.onClick = [this] { songOverlay.show(); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &notesBox, &lengthBox, &dragTile, &saveButton, &songButton })
        show (*c);

    content.addChildComponent (songOverlay);
    content.addChildComponent (about);

    combos = { &grooveCombo, &genreCombo, &sectionCombo, &fillsCombo, &variationCombo, &fillStyleCombo, &kitCombo, &channelCombo };

    lastSnapshot = processor.snapshot();
    selectLane ((int) fgd::Lane::closedHat);
    setView (false);

    const float scale = juce::jlimit (0.6f, 2.0f, processor.editorScale);
    setResizable (true, true);
    setResizeLimits ((int) (designWidth * 0.55f), (int) (designHeight * 0.55f), designWidth * 2, designHeight * 2);
    getConstrainer()->setFixedAspectRatio ((double) designWidth / (double) designHeight);
    setSize (juce::roundToInt (designWidth * scale), juce::roundToInt (designHeight * scale));

    startTimerHz (30);
}

FaseGrooveDrumsEditor::~FaseGrooveDrumsEditor()
{
    setLookAndFeel (nullptr);
}

void FaseGrooveDrumsEditor::resized()
{
    // Full screen can have another shape: the interface keeps its proportions, centred.
    const float scale = juce::jmax (0.3f, juce::jmin ((float) getWidth() / (float) designWidth, (float) getHeight() / (float) designHeight));
    if (! isFullScreen())
        processor.editorScale = scale;
    content.setBounds (0, 0, designWidth, designHeight);
    content.setTransform (juce::AffineTransform::scale (scale).translated (((float) getWidth() - designWidth * scale) * 0.5f,
                                                                         ((float) getHeight() - designHeight * scale) * 0.5f));
}

bool FaseGrooveDrumsEditor::isFullScreen() const
{
    if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
        if (window->isFullScreen())
            return true;
    return ! sizeBeforeFullScreen.isEmpty();
}

void FaseGrooveDrumsEditor::toggleFullScreen()
{
    // Full screen = the screen's usable area (the taskbar stays visible). Standalone app: its
    // window fills that area. In a DAW (the DAW owns the window) the plug-in grows to fit in it.
    auto* window = findParentComponentOfClass<juce::ResizableWindow>();
    if (! sizeBeforeFullScreen.isEmpty())
    {
        getConstrainer()->setFixedAspectRatio ((double) designWidth / (double) designHeight);
        if (window != nullptr)
            window->setBounds (sizeBeforeFullScreen);
        else
            setSize (sizeBeforeFullScreen.getWidth(), sizeBeforeFullScreen.getHeight());
        sizeBeforeFullScreen = {};
        return;
    }
    if (const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds()))
    {
        if (window != nullptr)
        {
            sizeBeforeFullScreen = window->getBounds();
            getConstrainer()->setFixedAspectRatio (0.0);
            window->setBounds (display->userArea);
            return;
        }
        sizeBeforeFullScreen = getBounds();
        const auto area = display->userArea.reduced (8).withTrimmedTop (30);
        const float scale = juce::jmin ((float) area.getWidth() / (float) designWidth, (float) area.getHeight() / (float) designHeight, 2.0f);
        setSize (juce::roundToInt (designWidth * scale), juce::roundToInt (designHeight * scale));
    }
}

void FaseGrooveDrumsEditor::Content::resized()
{
    editor.layout();
}

void FaseGrooveDrumsEditor::Content::paint (juce::Graphics& g)
{
    editor.paintContent (g);
}

void FaseGrooveDrumsEditor::Content::mouseDown (const juce::MouseEvent& e)
{
    if (editor.logoArea.contains (e.getPosition()))
    {
        editor.about.setVisible (true);
        editor.about.toFront (false);
    }
    grabKeyboardFocus();
}

bool FaseGrooveDrumsEditor::Content::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    if (key == juce::KeyPress::spaceKey)
    {
        editor.processor.previewToggle();
        return true;
    }
    if (mods.isCommandDown() && (key.getKeyCode() == 'Z' || key.getKeyCode() == 'z'))
    {
        mods.isShiftDown() ? editor.redo() : editor.undo();
        return true;
    }
    if (mods.isCommandDown() && (key.getKeyCode() == 'Y' || key.getKeyCode() == 'y'))
    {
        editor.redo();
        return true;
    }
    if (key == juce::KeyPress::escapeKey)
    {
        editor.about.setVisible (false);
        editor.songOverlay.setVisible (false);
        return true;
    }
    return false;
}

void FaseGrooveDrumsEditor::layout()
{
    // Header: logo | presets, total, undo / redo, full screen / style, reset, options | transport & tempo | display
    logoArea = { 20, 12, 238, 54 };
    presetsButton.setBounds (276, 18, 76, 28);
    totalRandomButton.setBounds (356, 18, 88, 28);
    undoButton.setBounds (452, 18, 30, 28);
    redoButton.setBounds (482, 18, 30, 28);
    fullScreenButton.setBounds (518, 18, 30, 28);
    styleButton.setBounds (276, 56, 76, 30);
    resetButton.setBounds (356, 56, 88, 30);
    optionsButton.setBounds (452, 56, 96, 30);

    sequencerToggle.setBounds (744, 18, 108, 22);
    rewindButton.setBounds (572, 50, 36, 32);
    playButton.setBounds (608, 50, 44, 32);
    stopButton.setBounds (652, 50, 36, 32);
    syncButton.setBounds (698, 50, 76, 32);
    tempoSlider.setBounds (778, 50, 84, 32);
    displayArea = { 874, 12, 288, 78 };

    // Groove row (20, 100, 1142, 132)
    grooveCombo.setBounds (34, 128, 214, 46);
    prevGroove.setBounds (252, 143, 24, 30);
    nextGroove.setBounds (276, 143, 24, 30);
    genreCombo.setBounds (34, 178, 150, 46);
    newBeatButton.setBounds (190, 193, 110, 30);
    densitySlider.setBounds (310, 193, 104, 30);
    for (int i = 0; i < fgd::numSegments; ++i)
        partCards[i]->setBounds (430 + i * 182, 122, 174, 104);

    // Kit & sounds (20, 242, 290, 256)
    drumBox.setBounds (34, 290, 164, 28);
    laneDiceButton.setBounds (202, 290, 34, 28);
    muteButton.setBounds (240, 290, 56, 28);
    noteBox.setBounds (34, 344, 176, 28);
    levelKnob.setBounds (218, 326, 82, 88);
    loadSampleButton.setBounds (34, 456, 166, 30);
    builtInButton.setBounds (204, 456, 92, 30);

    // Pads (320, 242, 260, 256)
    pads.setBounds (326, 276, 248, 216);

    // Groove panel (590, 242, 300, 256)
    newFeelButton.setBounds (742, 248, 72, 22);
    crashButton.setBounds (818, 248, 62, 22);
    sectionCombo.setBounds (604, 278, 134, 46);
    fillsCombo.setBounds (744, 278, 134, 46);
    variationCombo.setBounds (604, 328, 134, 46);
    fillStyleCombo.setBounds (744, 328, 134, 46);
    swing.setBounds (600, 390, 70, 102);
    humanize.setBounds (670, 390, 70, 102);
    velocity.setBounds (740, 390, 70, 102);
    accents.setBounds (810, 390, 70, 102);

    // Guide sound & MIDI out (900, 242, 262, 256)
    soundToggle.setBounds (914, 294, 116, 24);
    kitCombo.setBounds (1034, 278, 116, 46);
    channelCombo.setBounds (914, 328, 100, 46);
    tone.setBounds (914, 384, 76, 108);
    reverb.setBounds (994, 384, 76, 108);
    volume.setBounds (1074, 384, 76, 108);

    // Edit (20, 508, 830, 342)
    gridTab.setBounds (34, 534, 100, 26);
    velocityTab.setBounds (134, 534, 96, 26);
    newRhythmButton.setBounds (246, 534, 90, 26);
    myGroovesButton.setBounds (352, 534, 112, 26);
    importButton.setBounds (468, 534, 112, 26);
    clearButton.setBounds (762, 534, 76, 26);
    grid.setBounds (28, 568, 814, 274);
    velocityView.setBounds (28, 568, 814, 274);

    // Export MIDI (860, 508, 302, 342)
    notesBox.setBounds (874, 556, 134, 28);
    lengthBox.setBounds (1014, 556, 134, 28);
    dragTile.setBounds (874, 596, 274, 152);
    saveButton.setBounds (874, 756, 134, 32);
    songButton.setBounds (1014, 756, 134, 32);

    songOverlay.setBounds (content.getLocalBounds());
    about.setBounds (content.getLocalBounds());
}

void FaseGrooveDrumsEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (Palette::background);

    // ---- Header ----
    Branding::drawFullLogo (g, logoArea.toFloat().withHeight (46.0f));
    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.drawText ("by " + juce::String (Branding::artist), logoArea.getX() + 58, logoArea.getBottom() + 2, 200, 14, juce::Justification::centredLeft);

    auto box = [&g] (juce::Rectangle<int> r)
    {
        g.setColour (Palette::panel);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (Palette::panelEdge);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 8.0f, 1.0f);
    };
    box ({ 560, 12, 306, 78 });
    g.setColour (Palette::accent);
    g.fillRoundedRectangle (572.0f, 23.0f, 3.0f, 12.0f, 1.0f);
    g.setColour (Palette::text.withAlpha (0.92f));
    g.setFont (font (11.5f, true));
    g.drawText ("TRANSPORT & TEMPO", 581, 19, 160, 20, juce::Justification::centredLeft);

    // The display: the groove playing, its genre, the section and where the playhead is.
    box (displayArea);
    {
        const auto settings = processor.currentSettings();
        const auto base = fgd::GrooveEngine::basePattern (settings);
        auto r = displayArea.reduced (16, 10);
        auto top = r.removeFromTop (36);
        g.setColour (Palette::accent);
        g.setFont (font (13.0f, true));
        const juce::String section (fgd::sectionName ((int) settings.section));
        g.drawText (section.toUpperCase(), top.removeFromRight (70), juce::Justification::topRight);
        g.setColour (Palette::text);
        g.setFont (font (22.0f, true));
        g.drawFittedText (base.name, top, juce::Justification::centredLeft, 1, 0.6f);

        const double ppq = processor.uiPpq.load();
        const auto where = ppq >= 0.0 ? "Bar " + juce::String ((int) std::floor (ppq / 4.0) + 1) + "  Beat " + juce::String ((int) std::floor (std::fmod (ppq, 4.0)) + 1)
                                      : juce::String ("Stopped");
        const auto genre = settings.pattern == fgd::userPatternIndex() ? juce::String ("Your pattern")
                         : base.category.empty() ? juce::String (fgd::grooveCategories()[(size_t) juce::jlimit (0, 12, settings.randomGenre)])
                                                 : juce::String (base.category);
        g.setColour (Palette::dim);
        g.setFont (font (12.0f));
        g.drawText (genre, r, juce::Justification::centredLeft);
        g.drawText (where + "   " + juce::String (processor.lastBpm.load(), 1) + " BPM", r, juce::Justification::centredRight);
    }

    // ---- Panels ----
    drawPanel (g, { 20, 100, 1142, 132 }, "GROOVE");
    drawPanel (g, { 20, 242, 290, 256 }, "KIT & SOUNDS");
    drawPanel (g, { 320, 242, 260, 256 }, "DRUM PADS");
    drawPanel (g, { 590, 242, 300, 256 }, "GROOVE FEEL & SONG");
    drawPanel (g, { 900, 242, 262, 256 }, "GUIDE SOUND & MIDI OUT");
    drawPanel (g, { 20, 508, 830, 342 }, {});
    drawPanel (g, { 860, 508, 302, 342 }, "EXPORT MIDI");

    auto caption = [&g] (const juce::String& text, int x, int y, int w = 160)
    {
        g.setColour (Palette::dim);
        g.setFont (font (11.0f, true));
        g.drawText (text, x, y, w, 15, juce::Justification::centredLeft);
    };
    caption ("DENSITY", 310, 178);
    caption ("PARTS A-D  (each half a bar: play them in any order)", 430, 104, 400);
    caption ("DRUM", 34, 274);
    caption ("MIDI NOTE", 34, 328);
    caption ("LEVEL", 220, 274);
    caption ("SAMPLE", 34, 390);
    caption ("VIEW", 34, 516);
    caption ("NEW RHYTHM", 246, 516);
    caption ("GROOVE", 352, 516);
    caption ("NOTES", 874, 540);
    caption ("CLIP LENGTH", 1014, 540);

    // The picked drum's sound.
    {
        const auto path = processor.samplePath (selectedLane);
        g.setColour (path.isNotEmpty() ? laneColour (selectedLane) : Palette::text.withAlpha (0.8f));
        g.setFont (font (13.0f, true));
        g.drawFittedText (path.isNotEmpty() ? juce::File (path).getFileName()
                                            : "Built-in: " + juce::String (GuideDrums::kitName (juce::roundToInt (processor.raw (ParamIDs::kit)))),
                          juce::Rectangle<int> (34, 408, 262, 20), juce::Justification::centredLeft, 1, 0.7f);
        g.setColour (Palette::dim);
        g.setFont (font (11.0f));
        g.drawText ("or drop a WAV / AIFF on the drum's row", 34, 428, 262, 18, juce::Justification::centredLeft);
    }

    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.setFont (font (10.5f));
    g.drawFittedText (velocityShown ? "one drum at a time:\ndrag its bars" : "shift accent - alt ghost\nctrl flam - drag paints",
                      juce::Rectangle<int> (590, 532, 168, 30), juce::Justification::centredLeft, 2, 0.8f);
    g.drawText ("Drop it on a drum track (your own kit / drum rack)", 874, 806, 280, 16, juce::Justification::centredLeft);

    // A short message (a saved preset, an imported file...)
    if (messageTicks > 0)
    {
        auto r = juce::Rectangle<float> (0.0f, 0.0f, 440.0f, 34.0f).withCentre ({ designWidth * 0.5f, 520.0f });
        g.setColour (Palette::accent);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (Palette::background);
        g.setFont (font (13.0f, true));
        g.drawFittedText (message, r.reduced (10.0f, 0.0f).toNearestInt(), juce::Justification::centred, 1, 0.8f);
    }
}

void FaseGrooveDrumsEditor::setView (bool showVelocity)
{
    velocityShown = showVelocity;
    gridTab.setToggleState (! showVelocity, juce::dontSendNotification);
    velocityTab.setToggleState (showVelocity, juce::dontSendNotification);
    grid.setVisible (! showVelocity);
    velocityView.setVisible (showVelocity);
    velocityView.setLane (selectedLane);
    content.repaint();
}

void FaseGrooveDrumsEditor::selectLane (int lane)
{
    selectedLane = juce::jlimit (0, fgd::numLanes - 1, lane);
    levelAttachment.reset();
    levelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::level (selectedLane), levelKnob);
    levelKnob.setColour (juce::Slider::textBoxTextColourId, Palette::text);
    if (velocityView.getLane() != selectedLane)
        velocityView.setLane (selectedLane);
    refreshKitPanel();
    content.repaint();
}

void FaseGrooveDrumsEditor::refreshKitPanel()
{
    drumBox.setSelectedId (selectedLane + 1, juce::dontSendNotification);
    noteBox.setSelectedId (processor.laneNote (selectedLane) + 1, juce::dontSendNotification);
    const bool muted = processor.isLaneMuted (selectedLane);
    muteButton.setToggleState (muted, juce::dontSendNotification);
    muteButton.setColour (juce::TextButton::buttonOnColourId, Palette::toms);
    builtInButton.setEnabled (processor.samplePath (selectedLane).isNotEmpty());
}

void FaseGrooveDrumsEditor::stepGroove (int delta)
{
    // Through the menu's order (by genre).
    std::vector<int> order;
    const auto& presets = fgd::drumPatterns();
    for (const auto* category : fgd::grooveCategories())
        for (size_t i = 0; i < presets.size(); ++i)
            if (presets[i].category == category)
                order.push_back ((int) i);
    const int current = juce::roundToInt (processor.raw (ParamIDs::pattern));
    auto it = std::find (order.begin(), order.end(), current);
    int index = it == order.end() ? 0 : (int) (it - order.begin()) + delta;
    index = (index % (int) order.size() + (int) order.size()) % (int) order.size();
    processor.setParameterFromUI (ParamIDs::pattern, (float) order[(size_t) index]);
}

void FaseGrooveDrumsEditor::timerCallback()
{
    for (auto* c : combos)
        c->refresh();

    // The lights follow the hits and fade out.
    for (int l = 0; l < fgd::numLanes; ++l)
        lights[(size_t) l] = juce::jmax (processor.laneFlash[(size_t) l].exchange (0.0f), lights[(size_t) l] * 0.8f);
    grid.setLights (lights, selectedLane);
    pads.setLights (lights, selectedLane);

    if (velocityShown)
        velocityView.refresh();
    else
        grid.refresh();

    const auto shown = processor.displayedPattern();
    const int step = processor.uiStep.load();
    for (auto& card : partCards)
        card->refresh (shown, step);
    refreshKitPanel();

    const bool playing = processor.previewOn.load();
    playButton.setIcon (playing ? IconButton::Icon::pause : IconButton::Icon::play);
    playButton.setToggleState (playing || step >= 0, juce::dontSendNotification);
    tempoSlider.setEnabled (processor.raw (ParamIDs::tempoSync) < 0.5f);
    fullScreenButton.setIcon (isFullScreen() ? IconButton::Icon::exitFullScreen : IconButton::Icon::fullScreen);
    fullScreenButton.setTooltip (isFullScreen() ? "Exit full screen" : "Full screen");

    // Undo history: a snapshot twice a second; a change pushes the previous one.
    if (++snapshotTicks >= 15)
    {
        snapshotTicks = 0;
        const auto now = processor.snapshot();
        if (now != lastSnapshot)
        {
            undoStack.add (lastSnapshot);
            if (undoStack.size() > 80)
                undoStack.remove (0);
            redoStack.clear();
            lastSnapshot = now;
        }
    }
    undoButton.setEnabled (! undoStack.isEmpty());
    redoButton.setEnabled (! redoStack.isEmpty());

    if (messageTicks > 0 && --messageTicks == 0)
        content.repaint();

    content.repaint (displayArea);
    content.repaint (20, 400, 290, 50);
}

void FaseGrooveDrumsEditor::undo()
{
    const auto now = processor.snapshot();
    if (now != lastSnapshot)   // a change not yet in the history: undo that first
    {
        undoStack.add (lastSnapshot);
        lastSnapshot = now;
    }
    if (undoStack.isEmpty())
        return;
    redoStack.add (lastSnapshot);
    lastSnapshot = undoStack[undoStack.size() - 1];
    undoStack.remove (undoStack.size() - 1);
    processor.restoreSnapshot (lastSnapshot);
    snapshotTicks = 0;
}

void FaseGrooveDrumsEditor::redo()
{
    if (redoStack.isEmpty())
        return;
    undoStack.add (processor.snapshot());
    lastSnapshot = redoStack[redoStack.size() - 1];
    redoStack.remove (redoStack.size() - 1);
    processor.restoreSnapshot (lastSnapshot);
    snapshotTicks = 0;
}

void FaseGrooveDrumsEditor::showMessage (const juce::String& text)
{
    message = text;
    messageTicks = 75;
    content.repaint();
}

void FaseGrooveDrumsEditor::askName (const juce::String& title, const juce::String& suggestion, std::function<void (const juce::String&)> onName)
{
    auto* window = new juce::AlertWindow (title, "Name:", juce::MessageBoxIconType::NoIcon, this);
    window->addTextEditor ("name", suggestion);
    window->addButton ("SAVE", 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    window->enterModalState (true, juce::ModalCallbackFunction::create ([window, onName] (int result)
    {
        if (result == 1 && onName != nullptr)
            onName (window->getTextEditorContents ("name").trim());
    }), true);
}

void FaseGrooveDrumsEditor::showPresetsMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Save preset...");
    const auto files = FaseGrooveDrumsProcessor::presetFiles();
    if (! files.isEmpty())
    {
        menu.addSectionHeader ("Your presets");
        for (int i = 0; i < files.size(); ++i)
            menu.addItem (100 + i, files[i].getFileNameWithoutExtension());
    }
    menu.addSeparator();
    menu.addItem (2, "Open the presets folder");

    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetsButton), [safe, files] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        auto& p = safe->processor;
        if (result == 1)
            safe->askName ("Save preset", "My preset", [safe] (const juce::String& name)
            {
                if (safe != nullptr && name.isNotEmpty())
                    safe->showMessage (safe->processor.savePreset (name) ? "Preset saved: " + name : juce::String ("Could not save the preset"));
            });
        else if (result == 2)
        {
            FaseGrooveDrumsProcessor::presetFolder().createDirectory();
            FaseGrooveDrumsProcessor::presetFolder().startAsProcess();
        }
        else if (juce::isPositiveAndBelow (result - 100, files.size()))
            safe->showMessage (p.loadPreset (files[result - 100]) ? "Preset: " + files[result - 100].getFileNameWithoutExtension()
                                                                  : juce::String ("Could not read the preset"));
    });
}

void FaseGrooveDrumsEditor::showGroovesMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Save this groove...");
    menu.addItem (3, "Import the drums of a MIDI file...");
    const auto files = FaseGrooveDrumsProcessor::grooveFiles();
    if (! files.isEmpty())
    {
        menu.addSectionHeader ("My grooves");
        for (int i = 0; i < files.size(); ++i)
            menu.addItem (100 + i, files[i].getFileNameWithoutExtension());
    }
    menu.addSeparator();
    menu.addItem (2, "Open the grooves folder");

    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&myGroovesButton), [safe, files] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        if (result == 1)
            safe->askName ("Save groove", "My groove", [safe] (const juce::String& name)
            {
                if (safe != nullptr && name.isNotEmpty())
                    safe->showMessage (safe->processor.saveGroove (name) ? "Groove saved: " + name : juce::String ("Could not save the groove"));
            });
        else if (result == 2)
        {
            FaseGrooveDrumsProcessor::grooveFolder().createDirectory();
            FaseGrooveDrumsProcessor::grooveFolder().startAsProcess();
        }
        else if (result == 3)
            safe->importMidi();
        else if (juce::isPositiveAndBelow (result - 100, files.size()))
            safe->showMessage (safe->processor.loadGroove (files[result - 100]) ? "Groove: " + files[result - 100].getFileNameWithoutExtension()
                                                                                 : juce::String ("Could not read the groove"));
    });
}

void FaseGrooveDrumsEditor::importMidi()
{
    chooser = std::make_unique<juce::FileChooser> ("Import the drums of a MIDI file", juce::File(), "*.mid;*.midi");
    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe] (const juce::FileChooser& fc)
    {
        if (safe != nullptr && fc.getResult().existsAsFile())
            safe->importFile (fc.getResult());
    });
}

void FaseGrooveDrumsEditor::importAudio()
{
    chooser = std::make_unique<juce::FileChooser> ("Import an audio drum loop", juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe] (const juce::FileChooser& fc)
    {
        if (safe != nullptr && fc.getResult().existsAsFile())
            safe->importFile (fc.getResult());
    });
}

void FaseGrooveDrumsEditor::importFile (const juce::File& file)
{
    if (FaseGrooveDrumsProcessor::isAudioFile (file))
    {
        const auto report = processor.importAudioGroove (file);
        showMessage (report.isNotEmpty() ? file.getFileNameWithoutExtension() + ": " + report : "No drums found in " + file.getFileName());
    }
    else
        showMessage (processor.importMidiGroove (file) ? "Drums imported: " + file.getFileNameWithoutExtension()
                                                       : juce::String ("No drums found in ") + file.getFileName());
    setView (false);
}

bool FaseGrooveDrumsEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
        if (f.endsWithIgnoreCase (".mid") || f.endsWithIgnoreCase (".midi") || FaseGrooveDrumsProcessor::isAudioFile (juce::File (f)))
            return true;
    return false;
}

void FaseGrooveDrumsEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    // An audio file on a drum's row (or on its pad, or the kit panel): that drum's own sample.
    // Anywhere else: an audio loop, turned into the grid.
    for (const auto& f : files)
        if (const juce::File file (f); FaseGrooveDrumsProcessor::isAudioFile (file))
        {
            int lane = grid.isVisible() ? grid.laneAt (grid.getLocalPoint (this, juce::Point<int> (x, y))) : -1;
            const auto inContent = content.getLocalPoint (this, juce::Point<int> (x, y));
            if (lane < 0 && (pads.getBounds().contains (inContent) || juce::Rectangle<int> (20, 242, 290, 256).contains (inContent)))
                lane = selectedLane;
            if (lane < 0)
                importFile (file);
            else
                showMessage (processor.loadSample (lane, file) ? juce::String (fgd::laneName (lane)) + ": " + file.getFileName()
                                                               : "Could not read " + file.getFileName());
            return;
        }

    for (const auto& f : files)
        if (f.endsWithIgnoreCase (".mid") || f.endsWithIgnoreCase (".midi"))
        {
            importFile (juce::File (f));
            return;
        }
}

int FaseGrooveDrumsEditor::selectedClipBars() const
{
    return clipBarChoices[juce::jlimit (0, (int) std::size (clipBarChoices) - 1, lengthBox.getSelectedId() - 1)];
}

FaseGrooveDrumsProcessor::ClipPart FaseGrooveDrumsEditor::selectedClipPart() const
{
    return notesBox.getSelectedId() == 2 ? FaseGrooveDrumsProcessor::ClipPart::tracks : FaseGrooveDrumsProcessor::ClipPart::all;
}

void FaseGrooveDrumsEditor::saveMidiFile()
{
    const auto bars = selectedClipBars();
    const auto part = selectedClipPart();
    auto folder = FaseGrooveDrumsProcessor::clipFolder();
    folder.createDirectory();

    chooser = std::make_unique<juce::FileChooser> ("Save the MIDI clip", folder.getChildFile (processor.clipFileName (bars, part)), "*.mid");
    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe, bars, part] (const juce::FileChooser& fc)
    {
        if (safe == nullptr)
            return;
        auto file = fc.getResult();
        if (file == juce::File())
            return;
        file = file.withFileExtension ("mid");
        file.deleteFile();
        juce::FileOutputStream stream (file);
        if (stream.openedOk())
            safe->processor.createMidiClip (bars, part).writeTo (stream);
    });
}
