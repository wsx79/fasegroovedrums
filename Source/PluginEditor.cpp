// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginEditor.h"

#include "Localisation.h"

#include <BinaryData.h>

namespace
{
    juce::Font font (float height, bool bold = false)
    {
        return juce::Font (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    }

    /** Shows a value in a menu that the timer keeps up to date. Only a new value is shown: a pick
        still on its way is left alone (a ComboBox tells onChange a moment later, and showing the
        old value in between would undo the pick). onChange calls pickedFromMenu(). */
    void showInMenu (juce::ComboBox& box, int id)
    {
        auto& props = box.getProperties();
        if ((int) props.getWithDefault ("fgdShown", -1) == id)
            return;
        props.set ("fgdShown", id);
        if (box.getSelectedId() != id)
            box.setSelectedId (id, juce::dontSendNotification);
    }

    void pickedFromMenu (juce::ComboBox& box)
    {
        box.getProperties().set ("fgdShown", -1);   // shows the value again, whatever it is now
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
            g.drawText (Localisation::upper (tr (title)), area.withHeight (34).withTrimmedLeft (21).withTrimmedRight (12), juce::Justification::centredLeft);
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
    Localisation::bindLabel (label, caption);
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
        Localisation::bindItems (box, c->choices);
    }
    else if (auto* n = dynamic_cast<juce::AudioParameterInt*> (parameter))
    {
        for (int v = n->getRange().getStart(); v <= n->getRange().getEnd(); ++v)
            box.addItem (juce::String (v), v - n->getRange().getStart() + 1);
    }

    box.onChange = [this]
    {
        pickedFromMenu (box);
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
    if (hasCaption)
        Localisation::bindLabel (label, caption);
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
    showInMenu (box, juce::roundToInt (plain - base) + 1);
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
    const float cellW = (float) (getWidth() - headerWidth) / (float) processor.steps();
    return { (float) headerWidth + cellW * (float) step, rowH * (float) lane, cellW, rowH };
}

bool DrumGrid::cellAt (juce::Point<float> p, int& lane, int& step) const
{
    const float rowH = (float) getHeight() / (float) fgd::numLanes;
    const float cellW = (float) (getWidth() - headerWidth) / (float) processor.steps();
    lane = (int) std::floor (p.y / rowH);
    step = (int) std::floor ((p.x - (float) headerWidth) / cellW);
    return p.x >= (float) headerWidth && juce::isPositiveAndBelow (lane, fgd::numLanes)
        && juce::isPositiveAndBelow (step, processor.steps());
}

void DrumGrid::paint (juce::Graphics& g)
{
    const int steps = processor.steps();

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
        g.drawText (tr (fgd::laneShortName (lane)), textArea.removeFromTop (textArea.getHeight() * 0.58f), juce::Justification::bottomLeft);
        g.setColour (Palette::dim);
        g.setFont (font (9.5f));
        const auto sample = processor.samplePath (lane);
        if (sample.isNotEmpty() && ! isMuted)
            g.setColour (laneColour (lane));
        g.drawText (isMuted ? tr ("MUTE")
                    : sample.isNotEmpty() ? juce::File (sample).getFileNameWithoutExtension()
                                          : tr ("note") + " " + juce::String (processor.laneNote (lane)),
                    textArea, juce::Justification::topLeft);

        IconButton::drawIcon (g, IconButton::Icon::dice, diceBounds (lane),
                              (lane == hoverLane && hoverDice) ? Palette::accent : Palette::accent.withAlpha (0.55f));

        // Steps.
        const auto& line = pattern.lanes[(size_t) lane];
        for (int step = 0; step < steps; ++step)
        {
            auto c = cellBounds (lane, step).reduced (steps > 32 ? 0.8f : 1.5f, 1.5f);
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

            // A roll: small ticks; a chance: the cell's corner cut and its percentage.
            if (const int n = processor.stepRoll (lane, step); n > 1)
            {
                g.setColour (Palette::background.withAlpha (0.75f));
                for (int k = 1; k < n; ++k)
                    g.fillRect (juce::Rectangle<float> (1.2f, c.getHeight() * 0.5f).withCentre ({ c.getX() + c.getWidth() * (float) k / (float) n, c.getCentreY() }));
            }
            if (const int pct = processor.stepChance (lane, step); pct < 100)
            {
                juce::Path corner;
                corner.addTriangle (c.getRight() - c.getWidth() * 0.55f, c.getY(), c.getRight(), c.getY(), c.getRight(), c.getY() + c.getHeight() * 0.55f);
                g.setColour (Palette::background.withAlpha (0.8f));
                g.fillPath (corner);
                g.setColour (Palette::text.withAlpha (0.9f));
                g.setFont (font (7.5f, true));
                g.drawText ("?", c.withTrimmedLeft (c.getWidth() * 0.5f).withHeight (c.getHeight() * 0.5f), juce::Justification::centred);
            }

            // A velocity set by hand: a small level mark at the bottom of the cell.
            if (const int v = processor.stepVelocity (lane, step); v > 0)
            {
                g.setColour (Palette::background.withAlpha (0.7f));
                g.fillRect (c.removeFromBottom (3.0f).withTrimmedRight (c.getWidth() * (1.0f - (float) v / 127.0f)));
            }
        }
    }

    // Lines between the parts (A B C D) and the bars.
    const int part = steps / fgd::numSegments;
    for (int k = 1; k < fgd::numSegments; ++k)
    {
        const float x = cellBounds (0, k * part).getX();
        const bool strong = k == 2 || part == 16;
        g.setColour (strong ? Palette::dim.withAlpha (0.6f) : Palette::panelEdge.brighter (0.2f));
        g.drawLine (x, 0.0f, x, (float) getHeight(), strong ? 1.2f : 1.0f);
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
    else                                       paintSymbol = fgd::isHit (current) ? '.' : brush;
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
        setTooltip (overDice ? tr ("A new line for this drum only (in the NEW BEAT genre; locked parts stay)")
                    : newLane >= 0 && newStep < 0 ? tr ("Click: pick it (kit panel). Double-click: its velocities. Right-click: mute, sample, MIDI note, level")
                    : newStep >= 0 ? tr ("Click: hit. Shift: accent, alt: ghost, ctrl: flam. Drag paints. Right-click: chance and roll")
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
    menu.addSectionHeader (tr (fgd::laneName (lane)) + " - " + tr ("step") + " " + juce::String (step + 1));
    menu.addItem (1, tr ("Accent (X)"));
    menu.addItem (2, tr ("Hit (x)"));
    menu.addItem (3, tr ("Ghost note (g)"));
    menu.addItem (4, tr ("Flam (f)"));
    menu.addItem (5, tr ("Delete"));
    menu.addItem (6, tr ("Move left"), processor.stepSymbol (lane, step) != '.');
    menu.addItem (7, tr ("Move right"), processor.stepSymbol (lane, step) != '.');

    // How often it plays, and a quick roll of 2-4 hits in the 16th (hats, snares...).
    const bool hit = fgd::isHit (processor.stepSymbol (lane, step));
    juce::PopupMenu chance, roll;
    const int currentChance = processor.stepChance (lane, step), currentRoll = processor.stepRoll (lane, step);
    for (int pct : { 100, 90, 75, 50, 33, 25, 10 })
        chance.addItem (100 + pct, pct == 100 ? tr ("Always") : juce::String (pct) + "%", true, pct == currentChance);
    for (int n = 1; n <= 4; ++n)
        roll.addItem (200 + n, n == 1 ? tr ("One hit") : juce::String (n) + " " + tr ("hits"), true, n == currentRoll);
    menu.addSeparator();
    menu.addSubMenu (tr ("Chance") + ": " + (currentChance >= 100 ? tr ("Always") : juce::String (currentChance) + "%"), chance, hit);
    menu.addSubMenu (tr ("Roll") + ": " + juce::String (currentRoll), roll, hit && lane != (int) fgd::Lane::kick);

    juce::Component::SafePointer<DrumGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [safe, lane, step] (int result)
    {
        static const char symbols[] { 'X', 'x', 'g', 'f', '.' };
        if (safe == nullptr || result < 1)
            return;
        auto& p = safe->processor;
        if (result <= 5)
            p.editStep (lane, step, symbols[result - 1]);
        else if (result == 6 || result == 7)
        {
            // The hit moves one 16th, with its velocity, chance and roll.
            const int to = (step + (result == 6 ? -1 : 1) + p.steps()) % p.steps();
            const char symbol = p.stepSymbol (lane, step);
            const int velocity = p.stepVelocity (lane, step), chance = p.stepChance (lane, step), roll = p.stepRoll (lane, step);
            p.editStep (lane, step, '.');
            p.editStep (lane, to, symbol);
            p.setStepVelocity (lane, to, velocity);
            p.setStepChance (lane, to, chance);
            p.setStepRoll (lane, to, roll);
        }
        else if (result >= 200)
            safe->processor.setStepRoll (lane, step, result - 200);
        else if (result >= 100)
            safe->processor.setStepChance (lane, step, result - 100);
        safe->refresh();
    });
}

void DrumGrid::showLaneMenu (int lane)
{
    juce::PopupMenu menu;
    menu.addSectionHeader (tr (fgd::laneName (lane)));
    menu.addItem (1, tr ("Mute"), true, processor.isLaneMuted (lane));
    menu.addItem (3, tr ("New random line (die)"));
    menu.addItem (4, tr ("Edit its velocities"));
    menu.addItem (2, tr ("Clear this row"));
    menu.addSeparator();
    const auto sample = processor.samplePath (lane);
    menu.addItem (5, sample.isEmpty() ? tr ("Load your own sample...") : tr ("Sample") + ": " + juce::File (sample).getFileName() + " (" + tr ("change...") + ")");
    menu.addItem (7, tr ("Choose it in the sample browser..."));
    menu.addItem (6, tr ("Back to the built-in sound"), sample.isNotEmpty());

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
        others.addSubMenu (tr ("Octave") + " " + juce::String (octave - 2), sub);
    }
    notes.addSeparator();
    notes.addSubMenu (tr ("Any note"), others);
    notes.addItem (999, tr ("Back to General MIDI") + " (" + juce::String (fgd::defaultNote (lane)) + ")");
    menu.addSubMenu (tr ("MIDI note") + ": " + juce::String (current), notes);

    juce::PopupMenu levels;
    const float level = processor.raw (ParamIDs::level (lane));
    for (int pct : { 25, 50, 75, 100, 125, 150 })
        levels.addItem (2000 + pct, juce::String (pct) + "%", true, juce::roundToInt (level * 100.0f) == pct);
    menu.addSubMenu (tr ("Level") + ": " + juce::String (juce::roundToInt (level * 100.0f)) + "%", levels);

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
        else if (result == 7)                     { if (safe->onBrowseSamples != nullptr) safe->onBrowseSamples (lane); }
        else if (result == 999)                   p.setParameterFromUI (ParamIDs::note (lane), (float) fgd::defaultNote (lane));
        else if (result >= 1000 && result < 1128) p.setParameterFromUI (ParamIDs::note (lane), (float) (result - 1000));
        else if (result >= 2000)                  p.setParameterFromUI (ParamIDs::level (lane), (float) (result - 2000) / 100.0f);
        safe->refresh();
    });
}

void DrumGrid::chooseSample (int lane)
{
    chooser = std::make_unique<juce::FileChooser> (tr ("Your own sound for this drum") + ": " + tr (fgd::laneName (lane)),
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
    juce::StringArray laneNames;
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        drumBox.addItem (fgd::laneName (l), l + 1);
        laneNames.add (fgd::laneName (l));
    }
    Localisation::bindItems (drumBox, laneNames);
    drumBox.setSelectedId (lane + 1, juce::dontSendNotification);
    Localisation::bindTip (drumBox, "The drum whose velocities you see and edit");
    drumBox.onChange = [this] { setLane (drumBox.getSelectedId() - 1); };
    addAndMakeVisible (drumBox);

    Localisation::bindText (diceButton, "RANDOM");
    diceButton.iconColour = Palette::accent;
    Localisation::bindTip (diceButton, "Random velocities for this drum: a groove shape with a little life on every hit");
    diceButton.onClick = [this] { processor.randomizeVelocities (lane); refresh(); };
    addAndMakeVisible (diceButton);

    const auto shapes = FaseGrooveDrumsProcessor::velocityShapeNames();
    for (int i = 0; i < shapes.size(); ++i)
        shapeBox.addItem (shapes[i], i + 1);
    Localisation::bindItems (shapeBox, shapes);
    Localisation::bind (shapeBox, [this] { shapeBox.setTextWhenNothingSelected (tr ("SHAPE...")); });
    Localisation::bindTip (shapeBox, "A velocity shape for this drum's hits (Offbeat push: the classic house hats)");
    shapeBox.onChange = [this]
    {
        if (const int id = shapeBox.getSelectedId(); id > 0)
            processor.applyVelocityShape (lane, id - 1);
        shapeBox.setSelectedId (0, juce::dontSendNotification);
        refresh();
    };
    addAndMakeVisible (shapeBox);

    Localisation::bindTip (autoButton, "This drum back to the automatic velocities (from accents and ghost notes)");
    autoButton.onClick = [this] { processor.resetStepVelocities (lane); refresh(); };
    Localisation::bindText (autoButton, "AUTO");
    addAndMakeVisible (autoButton);
    Localisation::bindTip (autoAllButton, "Every drum back to the automatic velocities");
    autoAllButton.onClick = [this] { processor.resetStepVelocities(); refresh(); };
    Localisation::bindText (autoAllButton, "ALL AUTO");
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
    for (int i = 0; i < processor.steps() && i < p.length(); ++i)
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
    const float w = area.getWidth() / (float) processor.steps();
    return { area.getX() + w * (float) step, area.getY(), w, area.getHeight() };
}

int VelocityView::stepAt (float x) const
{
    const auto area = laneArea();
    const int step = (int) std::floor ((x - area.getX()) / (area.getWidth() / (float) processor.steps()));
    return juce::isPositiveAndBelow (step, processor.steps()) ? step : -1;
}

void VelocityView::paint (juce::Graphics& g)
{
    const auto colour = laneColour (lane);
    const auto strip = stripArea(), area = laneArea();
    const int steps = processor.steps();

    // Captions on the left.
    g.setColour (Palette::dim);
    g.setFont (font (11.0f, true));
    g.drawText (tr ("STEPS"), juce::Rectangle<float> (0.0f, strip.getY(), (float) DrumGrid::headerWidth - 8.0f, strip.getHeight()), juce::Justification::centredLeft);
    g.drawText (tr ("VELOCITY"), juce::Rectangle<float> (0.0f, area.getY(), (float) DrumGrid::headerWidth - 8.0f, 20.0f), juce::Justification::centredLeft);
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

    // Lines between the parts.
    for (int k = 1; k < fgd::numSegments; ++k)
    {
        const float x = column (k * steps / fgd::numSegments, area).getX();
        g.setColour (k == 2 ? Palette::dim.withAlpha (0.6f) : Palette::panelEdge.brighter (0.2f));
        g.drawLine (x, strip.getY(), x, area.getBottom(), 1.0f);
    }

    g.setColour (Palette::dim);
    g.setFont (font (10.5f));
    g.drawFittedText (tr ("drag the bars - double-click: automatic\nfaint bars: automatic levels"),
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
FillView::FillView (FaseGrooveDrumsProcessor& p) : processor (p)
{
    Localisation::bindText (startButton, "START FROM...");
    Localisation::bindTip (startButton, "Start the custom fill from what a fill style plays");
    startButton.onClick = [this]
    {
        juce::PopupMenu menu;
        menu.addSectionHeader (tr ("Start from"));
        for (int i = 0; i < fgd::numFillStyles; ++i)
            if (i != (int) fgd::FillStyle::custom)
                menu.addItem (i + 1, tr (fgd::fillStyleName (i)));
        juce::Component::SafePointer<FillView> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&startButton), [safe] (int result)
        {
            if (safe != nullptr && result > 0)
            {
                safe->processor.customFillFromStyle (result - 1);
                safe->repaint();
            }
        });
    };
    Localisation::bindText (clearButton, "CLEAR FILL");
    Localisation::bindTip (clearButton, "Empty the custom fill");
    clearButton.onClick = [this] { processor.clearCustomFill(); repaint(); };
    addAndMakeVisible (startButton);
    addAndMakeVisible (clearButton);
    Localisation::bind (*this, [this] { setTooltip (tr ("Click: hit. Shift: accent, alt: ghost, ctrl: flam. Drag paints. Fill style Custom plays it")); });
}

void FillView::resized()
{
    auto top = getLocalBounds().removeFromTop (24);
    clearButton.setBounds (top.removeFromRight (96));
    top.removeFromRight (6);
    startButton.setBounds (top.removeFromRight (124));
}

juce::Rectangle<float> FillView::gridArea() const
{
    return getLocalBounds().toFloat().withTrimmedTop (30.0f);
}

bool FillView::cellAt (juce::Point<float> p, int& lane, int& step) const
{
    const auto area = gridArea();
    const float rowH = area.getHeight() / (float) fgd::numLanes;
    const float cellW = (area.getWidth() - (float) DrumGrid::headerWidth) / 16.0f;
    lane = (int) std::floor ((p.y - area.getY()) / rowH);
    step = (int) std::floor ((p.x - area.getX() - (float) DrumGrid::headerWidth) / cellW);
    return p.x >= area.getX() + (float) DrumGrid::headerWidth && juce::isPositiveAndBelow (lane, fgd::numLanes) && juce::isPositiveAndBelow (step, 16);
}

void FillView::paint (juce::Graphics& g)
{
    const bool active = juce::roundToInt (processor.raw (ParamIDs::fillStyle)) == (int) fgd::FillStyle::custom;
    g.setColour (active ? Palette::accent : Palette::dim);
    g.setFont (font (11.5f, true));
    g.drawFittedText (active ? tr ("Custom fill: plays at the end of every fill bar (from its first step)")
                             : tr ("Draw your own fill: Fill style becomes Custom"),
                      getLocalBounds().removeFromTop (24).withTrimmedRight (236), juce::Justification::centredLeft, 1, 0.7f);

    const auto area = gridArea();
    const float rowH = area.getHeight() / (float) fgd::numLanes;
    const float cellW = (area.getWidth() - (float) DrumGrid::headerWidth) / 16.0f;
    for (int lane = 0; lane < fgd::numLanes; ++lane)
    {
        const auto colour = laneColour (lane);
        auto h = juce::Rectangle<float> (area.getX(), area.getY() + rowH * (float) lane, (float) DrumGrid::headerWidth, rowH).reduced (2.0f, 1.0f);
        g.setColour (Palette::control);
        g.fillRoundedRectangle (h, 4.0f);
        g.setColour (colour.withAlpha (0.9f));
        g.fillRoundedRectangle (h.withWidth (3.0f), 1.5f);
        g.setColour (Palette::text);
        g.setFont (font (11.0f, true));
        g.drawText (tr (fgd::laneShortName (lane)), h.withTrimmedLeft (10.0f), juce::Justification::centredLeft);

        for (int step = 0; step < 16; ++step)
        {
            auto c = juce::Rectangle<float> (area.getX() + (float) DrumGrid::headerWidth + cellW * (float) step, area.getY() + rowH * (float) lane, cellW, rowH).reduced (1.5f);
            g.setColour (Palette::panelEdge.withAlpha ((step / 4) % 2 == 0 ? 0.55f : 0.32f));
            g.fillRoundedRectangle (c, 3.0f);
            const char symbol = processor.customFillSymbol (lane, step);
            if (! fgd::isHit (symbol))
                continue;
            if (symbol == 'g')
            {
                g.setColour (colour.withAlpha (0.55f));
                g.fillRoundedRectangle (c.withSizeKeepingCentre (c.getWidth() * 0.45f, c.getHeight() * 0.4f), 2.0f);
            }
            else
            {
                g.setColour (colour.withAlpha (symbol == 'X' ? 1.0f : symbol == 'f' ? 0.8f : 0.68f));
                g.fillRoundedRectangle (symbol == 'f' ? c.withTrimmedLeft (c.getWidth() * 0.3f) : c, 3.0f);
            }
        }
    }
}

void FillView::paintCell (int lane, int step, char symbol)
{
    if (lane == lastLane && step == lastStep)
        return;
    lastLane = lane;
    lastStep = step;
    processor.editCustomFill (lane, step, symbol);
    repaint();
}

void FillView::mouseDown (const juce::MouseEvent& e)
{
    int lane = 0, step = 0;
    lastLane = lastStep = -1;
    paintSymbol = 0;
    if (! cellAt (e.position, lane, step))
    {
        if (juce::isPositiveAndBelow (lane, fgd::numLanes) && e.position.y > gridArea().getY())
            processor.audition (lane);
        return;
    }
    const char current = processor.customFillSymbol (lane, step);
    if (e.mods.isShiftDown())          paintSymbol = 'X';
    else if (e.mods.isAltDown())       paintSymbol = 'g';
    else if (e.mods.isCommandDown())   paintSymbol = 'f';
    else                               paintSymbol = fgd::isHit (current) ? '.' : brush;
    paintCell (lane, step, paintSymbol);
}

void FillView::mouseDrag (const juce::MouseEvent& e)
{
    int lane = 0, step = 0;
    if (paintSymbol != 0 && cellAt (e.position, lane, step))
        paintCell (lane, step, paintSymbol);
}

//==============================================================================
void DragTile::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    const float alpha = isEnabled() ? 1.0f : 0.4f;
    if (filled)
    {
        // Fase Groove Piano's orange tile: a small MIDI clip, the title, what it drags.
        g.setColour (isMouseOver() || dragging ? Palette::accent.brighter (0.1f) : Palette::accent);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (Palette::background.withAlpha (0.25f));
        g.drawRoundedRectangle (r.reduced (4.0f), 6.0f, 1.0f);

        auto content = r.reduced (12.0f, 8.0f);
        if (content.getHeight() > 54.0f)
        {
            auto icon = content.removeFromTop (content.getHeight() * 0.38f).withSizeKeepingCentre (56.0f, juce::jmin (26.0f, content.getHeight() * 0.38f));
            g.setColour (Palette::background.withAlpha (0.85f));
            g.fillRoundedRectangle (icon, 4.0f);
            g.setColour (Palette::accent);
            const float w = icon.getWidth() / 6.0f, h = icon.getHeight() / 5.0f;
            const int rows[] { 3, 1, 2, 1 };
            for (int k = 0; k < 4; ++k)
                g.fillRoundedRectangle (icon.getX() + w * (0.6f + 1.2f * k), icon.getY() + h * (float) rows[k], w, h, 1.5f);
        }

        g.setColour (Palette::background);
        g.setFont (font (15.0f, true));
        auto titleArea = subtitle.isEmpty() ? content : content.removeFromTop (content.getHeight() * 0.55f);
        g.drawFittedText (tr (title), titleArea.toNearestInt(), subtitle.isEmpty() ? juce::Justification::centred : juce::Justification::centredBottom, 1, 0.8f);
        if (subtitle.isNotEmpty())
        {
            g.setColour (Palette::background.withAlpha (0.75f));
            g.setFont (font (11.5f));
            g.drawFittedText (tr (subtitle), content.toNearestInt().withTrimmedTop (3), juce::Justification::centredTop, 1, 0.8f);
        }
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
    g.drawFittedText (tr (title), r.withTrimmedBottom (r.getHeight() * 0.45f).reduced (6.0f, 0.0f).toNearestInt(), juce::Justification::centredBottom, 1, 0.7f);
    g.setColour (Palette::dim.withAlpha (alpha));
    g.setFont (font (11.0f));
    g.drawFittedText (tr (subtitle), r.withTrimmedTop (r.getHeight() * 0.6f).reduced (6.0f, 0.0f).toNearestInt(), juce::Justification::centredTop, 1, 0.7f);
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
        Localisation::bindTip (*box, "Bars of this section (- leaves it out)");
        box->onChange = [this, i, box] { processor.setSongBars (i, box->getSelectedId() - 1); refresh(); };
        addAndMakeVisible (box);

        auto* fresh = newButtons.add (new IconButton (IconButton::Icon::dice));
        Localisation::bindText (*fresh, "NEW VARIATION");
        fresh->iconColour = Palette::accent;
        Localisation::bindTip (*fresh, "This section plays its own version of the groove (hats, percussion and ghost notes move)");
        fresh->onClick = [this, i] { processor.regenerateSongPart (i); refresh(); };
        addAndMakeVisible (fresh);

        auto* original = originalButtons.add (new juce::TextButton ("ORIGINAL"));
        Localisation::bindText (*original, "ORIGINAL");
        Localisation::bindTip (*original, "Back to the groove as it is");
        original->onClick = [this, i] { processor.resetSongPart (i); refresh(); };
        addAndMakeVisible (original);

        auto* fill = fillBoxes.add (new juce::ComboBox());
        fill->addItem ("Fill: as set", 1);
        fill->addItem ("No fill", 2);
        for (int f = 0; f < fgd::numFillStyles; ++f)
            fill->addItem (fgd::fillStyleName (f), f + 3);
        Localisation::bind (*fill, [fill]
        {
            fill->changeItemText (1, tr ("Fill: as set"));
            fill->changeItemText (2, tr ("No fill"));
            for (int f = 0; f < fgd::numFillStyles; ++f)
                fill->changeItemText (f + 3, tr ("Fill") + ": " + tr (fgd::fillStyleName (f)));
            fill->setText (fill->getItemText (fill->getSelectedItemIndex()), juce::dontSendNotification);
        });
        Localisation::bindTip (*fill, "The fill on the last bar of this section (as set: the Song panel's fills)");
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
    Localisation::bindText (allButton, "ONE TRACK");
    Localisation::bindText (tracksButton, "ONE TRACK PER DRUM");
    Localisation::bindText (saveSongButton, "SAVE SONG .MID...");
    Localisation::bindText (saveSectionsButton, "ONE .MID PER SECTION...");
    Localisation::bindText (resetButton, "RESET SONG");
    Localisation::bindText (closeButton, "CLOSE");
    allButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    tracksButton.setConnectedEdges (juce::Button::ConnectedOnLeft);

    Localisation::bindTip (songTile, "Drag the whole song (every section, with markers) onto a track in your DAW");
    songTile.createFiles = [this]
    {
        const auto file = processor.writeMidiClip (FaseGrooveDrumsProcessor::songArrangement, clipPart());
        return file.existsAsFile() ? juce::StringArray (file.getFullPathName()) : juce::StringArray();
    };
    Localisation::bindTip (sectionTile, "Drag only the section picked on the timeline (click a section to pick it)");
    sectionTile.createFiles = [this]
    {
        const auto file = processor.writeSongSectionClip (selected, clipPart());
        return file.existsAsFile() ? juce::StringArray (file.getFullPathName()) : juce::StringArray();
    };
    Localisation::bindTip (sectionsTile, "Drag every section at once, each as its own clip: the DAW puts them one after another");
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
    Localisation::bindTip (resetButton, "Default bars, no own variations, fills as set");
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
    sectionTile.setSubtitle (selected >= 0 ? tr ("Section") + ": " + tr (FaseGrooveDrumsProcessor::sectionClipName (selected)) : tr ("No section"));
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
    g.drawText (Localisation::upper (tr ("Song arrangement")), inner.removeFromTop (28.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawFittedText (tr ("Bars of each section (- leaves it out), its own variation of the groove and the fill that leads out of it. "
                          "Drag the whole song, or click a section on the timeline and drag just that one."),
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
            g.drawFittedText (tr (fgd::sectionName ((int) part.section)), text.removeFromTop (text.getHeight() * 0.55f).toNearestInt(),
                              juce::Justification::centredLeft, 1, 0.6f);
            g.setFont (font (10.5f));
            g.drawFittedText (juce::String (part.bars) + " " + tr ("bars"), text.toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);
            x += w;
        }
    }

    // Column names.
    {
        auto header = rowArea (0).toFloat().translated (0.0f, -22.0f).withHeight (16.0f);
        g.setColour (Palette::dim);
        g.setFont (font (10.5f, true));
        auto col = [&] (float w, const juce::String& text) { g.drawText (Localisation::upper (tr (text)), header.removeFromLeft (w), juce::Justification::centredLeft); };
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
        g.drawText (tr (FaseGrooveDrumsProcessor::sectionClipName (i)), label.removeFromTop (label.getHeight() * 0.56f), juce::Justification::bottomLeft);
        g.setFont (font (10.5f));
        g.setColour (parts[(size_t) i].variation != 0 ? Palette::accent : Palette::dim);
        g.drawText (tr (parts[(size_t) i].bars <= 0 ? "left out" : parts[(size_t) i].variation != 0 ? "own variation" : "the groove as it is"),
                    label, juce::Justification::topLeft);
    }

    // Length of the whole song.
    const double bpm = processor.exportBpm();
    const int seconds = bpm > 0.0 ? juce::roundToInt (total * 4 * 60.0 / bpm) : 0;
    auto info = card().reduced (28.0f, 20.0f).withTrimmedBottom (40.0f + 12.0f + 68.0f + 6.0f).removeFromBottom (20.0f);
    g.setColour (Palette::text);
    g.setFont (font (13.0f, true));
    g.drawText (tr ("Total") + ": " + juce::String (total) + " " + tr ("bars") + "   " + juce::String (seconds / 60) + ":" + juce::String (seconds % 60).paddedLeft ('0', 2)
                    + "  @ " + juce::String (bpm, 1) + " BPM",
                info, juce::Justification::centredLeft);
}

void SongOverlay::saveSong()
{
    auto folder = FaseGrooveDrumsProcessor::clipFolder();
    folder.createDirectory();
    const auto part = clipPart();
    chooser = std::make_unique<juce::FileChooser> (tr ("Save the whole song"),
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
    chooser = std::make_unique<juce::FileChooser> (tr ("Choose a folder for the sections"), FaseGrooveDrumsProcessor::clipFolder());
    juce::Component::SafePointer<SongOverlay> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [safe, part] (const juce::FileChooser& fc)
    {
        const auto folder = fc.getResult();
        if (safe == nullptr || folder == juce::File())
            return;
        const int written = safe->processor.saveSongSections (folder, part);
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, tr ("Song sections"),
                                                juce::String (written) + " " + tr (".mid files saved in") + "\n" + folder.getFullPathName());
    });
}

//==============================================================================
OptionsOverlay::OptionsOverlay() : artist (Branding::artistLogo())
{
    setInterceptsMouseClicks (true, true);

    auto styleTab = [this] (juce::TextButton& b)
    {
        b.setClickingTogglesState (false);
        b.setColour (juce::TextButton::buttonColourId, Palette::background);
        b.setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        b.setColour (juce::TextButton::textColourOffId, Palette::text);
        b.setColour (juce::TextButton::textColourOnId, Palette::background);
        addAndMakeVisible (b);
    };
    styleTab (settingsTab);
    styleTab (aboutTab);
    styleTab (closeButton);
    settingsTab.onClick = [this] { selectTab (false); };
    aboutTab.onClick    = [this] { selectTab (true); };
    closeButton.onClick = [this] { setVisible (false); };

    auto setUpLink = [this] (juce::TextButton& b, const char* url, juce::Colour colour, juce::Colour text)
    {
        b.setColour (juce::TextButton::buttonColourId, colour);
        b.setColour (juce::TextButton::textColourOffId, text);
        b.onClick = [url] { juce::URL (url).launchInDefaultBrowser(); };
        addChildComponent (b);
    };
    setUpLink (sourceButton, Branding::sourceUrl, Palette::background, Palette::text);
    setUpLink (kofiButton, Branding::kofiUrl, Palette::accent, Palette::background);
    setUpLink (paypalButton, Branding::paypalUrl, Palette::accent2, Palette::background);
    setUpLink (spotifyButton, Branding::spotifyUrl, juce::Colour (0xff1db954), Palette::background);

    // Settings tab
    for (auto* l : { &languageLabel, &sizeLabel })
    {
        l->setFont (font (11.0f, true));
        l->setColour (juce::Label::textColourId, Palette::dim);
        addChildComponent (l);
    }

    for (int i = 0; i < Localisation::numLanguages; ++i)
        languageBox.addItem (juce::String::fromUTF8 (Localisation::languageName (i)), i + 1);
    languageBox.onChange = [this]
    {
        Localisation::setCurrent ((Localisation::Language) (languageBox.getSelectedItemIndex()));
        refreshText();
        if (auto* top = getParentComponent())
            top->repaint();
    };
    addChildComponent (languageBox);

    for (int percent : { 60, 75, 90, 100, 110, 125, 150, 175, 200 })
        sizeBox.addItem (juce::String (percent) + " %", percent);
    sizeBox.onChange = [this]
    {
        if (onScaleChosen != nullptr && sizeBox.getSelectedId() > 0)
            onScaleChosen (sizeBox.getSelectedId() / 100.0f);
    };
    addChildComponent (sizeBox);

    tipsToggle.onClick = [this] { if (onTipsChanged != nullptr) onTipsChanged (tipsToggle.getToggleState()); };
    addChildComponent (tipsToggle);
    updatesToggle.onClick = [this] { Updater::setAutoCheckEnabled (updatesToggle.getToggleState()); };
    addChildComponent (updatesToggle);
    checkUpdatesButton.onClick = [this] { if (onCheckUpdates != nullptr) onCheckUpdates(); };
    addChildComponent (checkUpdatesButton);

    refreshText();
    selectTab (false);
}

void OptionsOverlay::show (bool about, float currentScale)
{
    languageBox.setSelectedItemIndex ((int) Localisation::current(), juce::dontSendNotification);

    int closest = 100;
    for (int i = 0; i < sizeBox.getNumItems(); ++i)
        if (std::abs (sizeBox.getItemId (i) - currentScale * 100.0f) < std::abs (closest - currentScale * 100.0f))
            closest = sizeBox.getItemId (i);
    sizeBox.setSelectedId (closest, juce::dontSendNotification);

    tipsToggle.setToggleState (Localisation::tipsEnabled(), juce::dontSendNotification);
    updatesToggle.setToggleState (Updater::autoCheckEnabled(), juce::dontSendNotification);
    refreshText();
    selectTab (about);
    setVisible (true);
    toFront (false);
}

void OptionsOverlay::refreshText()
{
    settingsTab.setButtonText (tr ("SETTINGS"));
    aboutTab.setButtonText (tr ("ABOUT"));
    closeButton.setButtonText (tr ("CLOSE"));
    sourceButton.setButtonText (tr ("SOURCE CODE"));
    kofiButton.setButtonText (tr ("DONATE: Ko-fi"));
    paypalButton.setButtonText (tr ("DONATE: PayPal"));
    spotifyButton.setButtonText (tr ("SPOTIFY"));
    spotifyButton.setTooltip (tr ("Listen to my productions on Spotify"));
    languageLabel.setText (Localisation::upper (tr ("Language")), juce::dontSendNotification);
    sizeLabel.setText (Localisation::upper (tr ("Interface size")), juce::dontSendNotification);
    tipsToggle.setButtonText (tr ("Show tips when the mouse rests on a control"));
    updatesToggle.setButtonText (tr ("Check for updates automatically (GitHub)"));
    if (checkUpdatesButton.isEnabled())
        checkUpdatesButton.setButtonText (tr ("CHECK NOW"));
    repaint();
}

void OptionsOverlay::selectTab (bool about)
{
    showingAbout = about;
    settingsTab.setToggleState (! about, juce::dontSendNotification);
    aboutTab.setToggleState (about, juce::dontSendNotification);

    for (auto* c : std::initializer_list<juce::Component*> { &languageLabel, &languageBox, &sizeLabel, &sizeBox, &tipsToggle,
                                                             &updatesToggle, &checkUpdatesButton })
        c->setVisible (! about);

    auto showLink = [about] (juce::Component& b, const char* url) { b.setVisible (about && juce::String (url).isNotEmpty()); };
    showLink (sourceButton, Branding::sourceUrl);
    showLink (kofiButton, Branding::kofiUrl);
    showLink (paypalButton, Branding::paypalUrl);
    showLink (spotifyButton, Branding::spotifyUrl);

    resized();
    repaint();
}

void OptionsOverlay::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);
}

void OptionsOverlay::resized()
{
    auto inner = card().toNearestInt().reduced (24, 18);

    auto tabs = inner.removeFromTop (30);
    closeButton.setBounds (tabs.removeFromRight (100));
    settingsTab.setBounds (tabs.removeFromLeft (150));
    tabs.removeFromLeft (8);
    aboutTab.setBounds (tabs.removeFromLeft (150));

    // Settings tab
    auto settings = inner.withTrimmedTop (40).reduced (40, 0);
    languageLabel.setBounds (settings.removeFromTop (18));
    languageBox.setBounds (settings.removeFromTop (30).withWidth (260));
    settings.removeFromTop (22);
    sizeLabel.setBounds (settings.removeFromTop (18));
    sizeBox.setBounds (settings.removeFromTop (30).withWidth (260));
    settings.removeFromTop (22);
    tipsToggle.setBounds (settings.removeFromTop (26).withWidth (460));
    settings.removeFromTop (10);
    {
        auto row = settings.removeFromTop (26);
        updatesToggle.setBounds (row.removeFromLeft (380));
        checkUpdatesButton.setBounds (row.removeFromLeft (130).reduced (0, 1));
    }

    // About tab: link buttons above the footer
    auto row = inner.withTrimmedBottom (26).removeFromBottom (30);
    juce::Array<juce::Component*> buttons;
    for (auto* b : { &sourceButton, &kofiButton, &paypalButton, &spotifyButton })
        if (b->isVisible())
            buttons.add (b);

    const int w = 126, gap = 10;
    row = row.withSizeKeepingCentre (juce::jmax (1, buttons.size() * w + (buttons.size() - 1) * gap), row.getHeight());
    for (auto* b : buttons)
    {
        b->setBounds (row.removeFromLeft (w));
        row.removeFromLeft (gap);
    }
}

void OptionsOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto card = this->card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (card, 16.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (card, 10.0f, 1.0f);

    auto r = card.reduced (36.0f, 18.0f);
    r.removeFromTop (46.0f);   // tabs
    const auto dot = juce::String::fromUTF8 ("  \xc2\xb7  ");

    if (! showingAbout)
    {
        // Under the settings: the shortcuts of the drums.
        auto keys = juce::Rectangle<float> (r.getX() + 28.0f, r.getY() + 266.0f, r.getWidth() - 28.0f, 220.0f);
        g.setColour (Palette::dim);
        g.setFont (font (11.0f, true));
        g.drawText (Localisation::upper (tr ("Shortcuts")), keys.removeFromTop (18.0f), juce::Justification::centredLeft);
        g.setColour (Palette::text.withAlpha (0.85f));
        g.setFont (font (12.0f));
        for (const auto* line : { "Space: play / pause      Ctrl+Z: undo      Ctrl+Shift+Z / Ctrl+Y: redo",
                                  "Grid: click hit, Shift accent, Alt ghost, Ctrl/Cmd flam, drag to paint",
                                  "Drum name: click picks it, double-click velocities, right-click mute / note / level",
                                  "Drop a .mid file on the window: its drums go into the grid",
                                  "Drop a sample on a drum's row or pad: your own sound for it",
                                  "Drop an audio drum loop anywhere else: it becomes the grid" })
            g.drawFittedText (tr (line), keys.removeFromTop (20.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);

        g.setColour (Palette::dim);
        g.setFont (font (12.0f));
        g.drawText (tr ("Settings are saved for all your projects."),
                    juce::Rectangle<float> (r.getX() + 28.0f, r.getY() + 500.0f, r.getWidth() - 28.0f, 18.0f), juce::Justification::centredLeft);
        g.setFont (font (11.0f));
        g.drawText ("Fase Groove Drums " + juce::String (JucePlugin_VersionString) + dot + tr ("Built with JUCE"),
                    r.removeFromBottom (16.0f), juce::Justification::centred);
        return;
    }

    Branding::drawFullLogo (g, r.removeFromTop (70.0f).withSizeKeepingCentre (340.0f, 70.0f));

    g.setColour (Palette::dim);
    g.setFont (font (13.0f));
    g.drawText (tr ("Version") + " " + juce::String (JucePlugin_VersionString) + dot + tr (Branding::tagline),
                r.removeFromTop (26.0f), juce::Justification::centred);

    r.removeFromTop (8.0f);
    g.setColour (Palette::panelEdge);
    g.fillRect (r.removeFromTop (1.0f));
    r.removeFromTop (12.0f);

    auto caption = [&g] (const juce::String& text, juce::Rectangle<float> area)
    {
        g.setColour (Palette::accent);
        g.setFont (font (11.0f, true));
        g.drawText (text, area, juce::Justification::centred);
    };

    caption (tr ("CREATED BY"), r.removeFromTop (16.0f));
    r.removeFromTop (6.0f);
    if (artist.isValid())
        g.drawImageWithin (artist, (int) r.getX(), (int) r.getY(), (int) r.getWidth(), 110,
                           juce::RectanglePlacement::centred);
    r.removeFromTop (114.0f);

    g.setColour (Palette::text);
    g.setFont (font (14.0f));
    g.drawText (tr ("DJ & Producer"), r.removeFromTop (20.0f), juce::Justification::centred);
    g.setColour (juce::Colour (0xff1db954));
    g.setFont (font (12.0f, true));
    g.drawText (tr ("Listen to my productions on Spotify"), r.removeFromTop (18.0f), juce::Justification::centred);

    r.removeFromTop (12.0f);
    caption (tr ("CODE"), r.removeFromTop (16.0f));
    r.removeFromTop (4.0f);
    g.setColour (Palette::text);
    g.setFont (font (16.0f, true));
    g.drawText ("Claude  (Anthropic)", r.removeFromTop (22.0f), juce::Justification::centred);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("AI coding assistant"), r.removeFromTop (16.0f), juce::Justification::centred);

    g.setFont (font (11.0f));
    g.drawText (tr ("Built with JUCE") + dot + "VST is a registered trademark of Steinberg Media Technologies GmbH",
                r.removeFromBottom (16.0f), juce::Justification::centred);

    r.removeFromBottom (48.0f);   // link buttons
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (juce::String (Branding::sourceUrl).fromFirstOccurrenceOf ("://", false, false),
                r.removeFromBottom (18.0f), juce::Justification::centred);
    g.setColour (Palette::text);
    g.setFont (font (12.0f, true));
    g.drawText (tr (Branding::licence), r.removeFromBottom (18.0f), juce::Justification::centred);
}

//==============================================================================
UpdateOverlay::UpdateOverlay()
{
    notes.setMultiLine (true, true);
    notes.setReadOnly (true);
    notes.setScrollbarsShown (true);
    notes.setIndents (10, 8);
    notes.setCaretVisible (false);
    notes.setFont (font (14.0f));
    notes.setColour (juce::TextEditor::backgroundColourId, Palette::background);
    notes.setColour (juce::TextEditor::textColourId, Palette::text);
    notes.setColour (juce::TextEditor::outlineColourId, Palette::accent.withAlpha (0.3f));
    addAndMakeVisible (notes);

    installButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    installButton.setColour (juce::TextButton::textColourOffId, Palette::background);
    for (auto* b : { &installButton, &laterButton, &skipButton })
        addAndMakeVisible (b);

    installButton.onClick = [this]
    {
        const auto report = Updater::install (result);
        setVisible (false);

        if (report.relaunching)
        {
            // Standalone app: close now; the new version opens by itself.
            juce::MessageManager::callAsync ([] { if (auto* app = juce::JUCEApplicationBase::getInstance()) app->systemRequestedQuit(); });
            return;
        }

        if (! report.updated.isEmpty())
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Fase Groove Drums",
                                                    tr ("Updated to version") + " " + result.version + ":\n\n"
                                                        + report.updated.joinIntoString ("\n") + "\n\n"
                                                        + tr ("Restart your DAW (or the app) to use the new version."));
        else if (! report.done && ! report.installerStarted)
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Fase Groove Drums",
                                                    tr ("The files could not be replaced automatically: the download is shown, copy it as the first time."));
    };
    laterButton.onClick = [this] { setVisible (false); };
    skipButton.onClick = [this]
    {
        Updater::skipVersion (result.version);
        setVisible (false);
    };
    refreshText();
}

void UpdateOverlay::show (const Updater::Result& r)
{
    result = r;
    notes.setText (r.notes, false);
    notes.moveCaretToTop (false);
    refreshText();
    setVisible (true);
    toFront (false);
}

void UpdateOverlay::refreshText()
{
    installButton.setButtonText (tr ("INSTALL NOW"));
    laterButton.setButtonText (tr ("LATER"));
    skipButton.setButtonText (tr ("SKIP THIS VERSION"));
    repaint();
}

void UpdateOverlay::resized()
{
    auto inner = card().toNearestInt().reduced (28, 22);
    inner.removeFromTop (92);
    auto buttons = inner.removeFromBottom (34);
    inner.removeFromBottom (44);
    notes.setBounds (inner);

    const int w = 170, gap = 12;
    buttons = buttons.withSizeKeepingCentre (3 * w + 2 * gap, buttons.getHeight());
    for (auto* b : { &installButton, &laterButton, &skipButton })
    {
        b->setBounds (buttons.removeFromLeft (w));
        buttons.removeFromLeft (gap);
    }
}

void UpdateOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto r = c.reduced (28.0f, 22.0f);
    g.setColour (Palette::accent);
    g.setFont (font (22.0f, true));
    g.drawText (tr ("New version") + " " + result.version + " " + tr ("downloaded"), r.removeFromTop (30.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (13.0f));
    g.drawText (tr ("You have") + " " + juce::String (JucePlugin_VersionString) + ". " + tr ("What's new:"),
                r.removeFromTop (26.0f), juce::Justification::centredLeft);

    const char* how = result.kind == Updater::InstallKind::installer
                        ? "INSTALL NOW updates it with the setup, without the wizard (same folders and parts). Close your DAW if it asks."
                    : result.kind == Updater::InstallKind::portable
                        ? "INSTALL NOW replaces the files of this copy where they are. Then restart your DAW."
                       #if JUCE_MAC
                        : "INSTALL NOW opens the zip: copy the plug-ins to the same folders as the first time, then restart your DAW.";
                       #else
                        : "INSTALL NOW shows the downloaded zip: copy the plug-ins to the same folders as the first time.";
                       #endif
    auto bottom = c.reduced (28.0f, 22.0f).removeFromBottom (34.0f + 44.0f).removeFromTop (44.0f);
    g.drawFittedText (tr (how), bottom.toNearestInt(), juce::Justification::centredLeft, 2);
    g.setFont (font (11.0f));
    g.drawText (result.file.getFullPathName(), r.removeFromTop (20.0f), juce::Justification::centredLeft);
}

//==============================================================================
PartCard::PartCard (FaseGrooveDrumsProcessor& p, int pos)
    : processor (p), position (pos), combo (p.apvts, ParamIDs::segment (pos), juce::String())
{
    combo.setTip ("Which part (A-D) plays in this half bar: e.g. A A A B");
    addAndMakeVisible (combo);
    dice.iconColour = Palette::accent;
    Localisation::bindTip (dice, "New random content for this part (in the beat genre)");
    dice.onClick = [this] { processor.randomizeSegment (processor.segmentAt (position)); };
    addAndMakeVisible (dice);
    Localisation::bindTip (lock, "Lock this part: the dice leave it alone");
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
    playing = playStep >= 0 && playStep / juce::jmax (1, shown.length() / fgd::numSegments) == position;
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
    const bool wholeBars = pattern.length() > 32;
    g.drawText (juce::String (position + 1) + "   " + Localisation::upper (tr ("Bar")) + " "
                    + (wholeBars ? juce::String (position + 1) : juce::String (position < 2 ? "1" : "2") + (position % 2 == 0 ? " a" : " b")),
                getLocalBounds().reduced (34, 6).removeFromTop (22), juce::Justification::centred);

    // The part's letter and a picture of its hits: kick, snare + clap, hats, percussion.
    auto body = getLocalBounds().reduced (8).withTrimmedTop (24).withTrimmedBottom (30).toFloat();
    g.setColour (locked ? Palette::dim : Palette::text);
    g.setFont (font (24.0f, true));
    g.drawText (juce::String::charToString ((juce::juce_wchar) ('A' + part)), body.removeFromLeft (30.0f), juce::Justification::centred);

    if (pattern.length() < 32)
        return;
    const int partSteps = pattern.length() / fgd::numSegments;
    struct Row { std::initializer_list<fgd::Lane> lanes; int colourLane; };
    const Row rows[] { { { fgd::Lane::closedHat, fgd::Lane::openHat, fgd::Lane::ride }, (int) fgd::Lane::closedHat },
                       { { fgd::Lane::shaker, fgd::Lane::tambourine, fgd::Lane::conga, fgd::Lane::cowbell, fgd::Lane::rim, fgd::Lane::tom }, (int) fgd::Lane::shaker },
                       { { fgd::Lane::snare, fgd::Lane::clap }, (int) fgd::Lane::snare },
                       { { fgd::Lane::kick }, (int) fgd::Lane::kick } };
    const float rowH = body.getHeight() / 4.0f, cellW = body.getWidth() / (float) partSteps;
    for (int row = 0; row < 4; ++row)
        for (int i = 0; i < partSteps; ++i)
        {
            char strongest = '.';
            for (auto lane : rows[row].lanes)
            {
                const char c = pattern[lane][(size_t) (position * partSteps + i)];
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
GrooveBrowser::GrooveBrowser (FaseGrooveDrumsProcessor& p) : processor (p)
{
    setWantsKeyboardFocus (true);
    Localisation::bindText (randomButton, "NEW RANDOM BEAT IN THIS GENRE");
    randomButton.iconColour = Palette::accent;
    randomButton.onClick = [this]
    {
        if (genre < (int) fgd::grooveCategories().size())
            processor.setParameterFromUI (ParamIDs::genre, (float) genre);
        processor.rollNewBeat();
        repaint();
    };
    addAndMakeVisible (randomButton);
    closeButton.onClick = [this] { setVisible (false); };
    Localisation::bindText (closeButton, "CLOSE");
    addAndMakeVisible (closeButton);
}

juce::Rectangle<float> GrooveBrowser::card() const
{
    return getLocalBounds().toFloat().withSizeKeepingCentre (1040.0f, 680.0f);
}

juce::Rectangle<float> GrooveBrowser::genreBounds (int g) const
{
    auto c = card().reduced (22.0f);
    return { c.getX(), c.getY() + 56.0f + 38.0f * (float) g, 210.0f, 34.0f };
}

juce::Rectangle<float> GrooveBrowser::tileBounds (int index) const
{
    auto c = card().reduced (22.0f);
    auto area = c.withTrimmedLeft (230.0f).withTrimmedTop (56.0f).withTrimmedBottom (56.0f);
    const float w = (area.getWidth() - 2 * 12.0f) / 3.0f, h = 76.0f;
    return { area.getX() + (w + 12.0f) * (float) (index % 3), area.getY() + (h + 10.0f) * (float) (index / 3), w, h };
}

std::vector<int> GrooveBrowser::groovesOf (int g) const
{
    std::vector<int> list;
    const auto& presets = fgd::drumPatterns();
    if (g >= (int) fgd::grooveCategories().size())
        return { fgd::randomPatternIndex(), fgd::userPatternIndex() };
    for (size_t i = 0; i < presets.size(); ++i)
        if (presets[i].category == fgd::grooveCategories()[(size_t) g])
            list.push_back ((int) i);
    return list;
}

void GrooveBrowser::show()
{
    const int current = juce::roundToInt (processor.raw (ParamIDs::pattern));
    genre = current < (int) fgd::drumPatterns().size() ? juce::jmax (0, fgd::categoryIndex (fgd::drumPatterns()[(size_t) current].category))
                                                        : (int) fgd::grooveCategories().size();
    setVisible (true);
    toFront (true);
    grabKeyboardFocus();
    repaint();
}

void GrooveBrowser::pick (int pattern)
{
    processor.selectGroove (pattern);
    repaint();
}

void GrooveBrowser::resized()
{
    auto c = card().reduced (22.0f).toNearestInt();
    auto bottom = c.removeFromBottom (36);
    closeButton.setBounds (bottom.removeFromRight (90));
    bottom.removeFromRight (10);
    randomButton.setBounds (bottom.removeFromRight (300));
}

void GrooveBrowser::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));
    const auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (22.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText (Localisation::upper (tr ("Grooves")), inner.removeFromTop (30.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("Pick a genre, click a groove: it loads at once (press play to hear it).  Arrows: previous / next."),
                c.reduced (22.0f).withTrimmedTop (30.0f).withHeight (18.0f).withTrimmedLeft (0.0f), juce::Justification::centredLeft);

    // Genres
    const int numGenres = (int) fgd::grooveCategories().size();
    for (int i = 0; i <= numGenres; ++i)
    {
        auto r = genreBounds (i);
        const bool on = i == genre;
        g.setColour (on ? Palette::accent : Palette::control);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (on ? Palette::background : Palette::text);
        g.setFont (font (13.0f, on));
        const auto name = i < numGenres ? juce::String (fgd::grooveCategories()[(size_t) i]) : tr ("Your beats");
        g.drawText (name, r.reduced (12.0f, 0.0f), juce::Justification::centredLeft);
        g.setColour (on ? Palette::background.withAlpha (0.7f) : Palette::dim);
        g.setFont (font (11.0f));
        g.drawText (juce::String ((int) groovesOf (i).size()), r.reduced (12.0f, 0.0f), juce::Justification::centredRight);
    }

    // Grooves of the genre, with a picture of their rhythm.
    const int current = juce::roundToInt (processor.raw (ParamIDs::pattern));
    const auto list = groovesOf (genre);
    for (int k = 0; k < (int) list.size(); ++k)
    {
        const int index = list[(size_t) k];
        auto r = tileBounds (k);
        const bool on = index == current;
        g.setColour (k == hover ? Palette::control.brighter (0.08f) : Palette::field);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (on ? Palette::accent : Palette::panelEdge);
        g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, on ? 2.0f : 1.0f);

        fgd::DrumSettings s = processor.currentSettings();
        s.pattern = index;
        auto pattern = fgd::GrooveEngine::basePattern (s);
        auto inner2 = r.reduced (10.0f, 7.0f);
        g.setColour (on ? Palette::accent : Palette::text);
        g.setFont (font (13.0f, true));
        g.drawFittedText (index == fgd::userPatternIndex() ? tr ("User pattern") : index == fgd::randomPatternIndex() ? tr ("Random beat") : juce::String (pattern.name),
                          inner2.removeFromTop (18.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);
        inner2.removeFromTop (5.0f);

        struct Row { std::initializer_list<fgd::Lane> lanes; int colourLane; };
        const Row rows[] { { { fgd::Lane::closedHat, fgd::Lane::openHat, fgd::Lane::ride, fgd::Lane::shaker }, (int) fgd::Lane::closedHat },
                           { { fgd::Lane::snare, fgd::Lane::clap, fgd::Lane::rim }, (int) fgd::Lane::snare },
                           { { fgd::Lane::kick }, (int) fgd::Lane::kick } };
        const float rowH = inner2.getHeight() / 3.0f, cellW = inner2.getWidth() / 32.0f;
        for (int row = 0; row < 3; ++row)
            for (int i = 0; i < 32 && i < pattern.length(); ++i)
            {
                char strongest = '.';
                for (auto lane : rows[row].lanes)
                {
                    const char ch = pattern[lane][(size_t) i];
                    if (ch == 'X' || (fgd::isHit (ch) && (strongest == '.' || strongest == 'g')))
                        strongest = ch;
                }
                auto cell = juce::Rectangle<float> (inner2.getX() + cellW * (float) i, inner2.getY() + rowH * (float) row, cellW, rowH).reduced (0.8f);
                g.setColour (Palette::panelEdge.withAlpha (i / 4 % 2 == 0 ? 0.5f : 0.3f));
                g.fillRect (cell);
                if (fgd::isHit (strongest))
                {
                    g.setColour (laneColour (rows[row].colourLane).withAlpha (strongest == 'X' ? 1.0f : strongest == 'g' ? 0.4f : 0.75f));
                    g.fillRect (cell);
                }
            }
    }
}

void GrooveBrowser::mouseDown (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
    {
        setVisible (false);
        return;
    }
    for (int i = 0; i <= (int) fgd::grooveCategories().size(); ++i)
        if (genreBounds (i).contains (e.position))
        {
            genre = i;
            hover = -1;
            repaint();
            return;
        }
    const auto list = groovesOf (genre);
    for (int k = 0; k < (int) list.size(); ++k)
        if (tileBounds (k).contains (e.position))
        {
            pick (list[(size_t) k]);
            if (e.getNumberOfClicks() > 1)
                setVisible (false);
            return;
        }
}

void GrooveBrowser::mouseMove (const juce::MouseEvent& e)
{
    int now = -1;
    const auto list = groovesOf (genre);
    for (int k = 0; k < (int) list.size(); ++k)
        if (tileBounds (k).contains (e.position))
            now = k;
    if (now != hover)
    {
        hover = now;
        repaint();
    }
}

bool GrooveBrowser::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        setVisible (false);
        return true;
    }
    const auto list = groovesOf (genre);
    if (list.empty())
        return false;
    const int current = juce::roundToInt (processor.raw (ParamIDs::pattern));
    auto it = std::find (list.begin(), list.end(), current);
    const int at = it == list.end() ? -1 : (int) (it - list.begin());
    if (key == juce::KeyPress::rightKey || key == juce::KeyPress::downKey)
    {
        pick (list[(size_t) ((at + 1) % (int) list.size())]);
        return true;
    }
    if (key == juce::KeyPress::leftKey || key == juce::KeyPress::upKey)
    {
        pick (list[(size_t) ((at - 1 + (int) list.size()) % (int) list.size())]);
        return true;
    }
    return false;
}

//==============================================================================
SampleBrowser::SampleBrowser (FaseGrooveDrumsProcessor& p) : processor (p)
{
    setWantsKeyboardFocus (true);
    list.setModel (this);
    list.setRowHeight (34);
    list.setMultipleSelectionEnabled (true);
    list.setColour (juce::ListBox::backgroundColourId, Palette::field);
    list.setColour (juce::ListBox::outlineColourId, Palette::panelEdge);
    list.setOutlineThickness (1);
    addAndMakeVisible (list);

    Localisation::bindText (folderButton, "ADD FOLDER...");
    Localisation::bindTip (folderButton, "Add a folder of samples (its subfolders too): the folders already here stay");
    folderButton.onClick = [this] { chooseFolder(); };
    Localisation::bindText (clearListButton, "CLEAR LIST");
    Localisation::bindTip (clearListButton, "Empty the browser's list of folders (the drums keep their samples)");
    clearListButton.onClick = [this]
    {
        folders.clear();
        found.clear();
        processor.sampleFolders.clear();
        rebuildList();
        resized();
        repaint();
    };
    Localisation::bindTip (foldersButton, "The folders your samples are read from: open one, read it again, or leave it out");
    foldersButton.onClick = [this] { showFoldersMenu(); };
    Localisation::bindText (chooseButton, "CHOOSE A FOLDER OF SAMPLES...");
    chooseButton.onClick = [this] { chooseFolder(); };
    Localisation::bindText (autoButton, "AUTO KIT");
    Localisation::bindTip (autoButton, "Every drum gets the first sample that suits it (then change any of them here)");
    autoButton.onClick = [this]
    {
        if (folders.isEmpty())
            return;
        // As recognised here (your corrections included).
        int loaded = 0;
        for (int l = 0; l < fgd::numLanes; ++l)
        {
            const FaseGrooveDrumsProcessor::SampleInfo* best = nullptr;
            for (const auto& info : found)
                if (info.lane == l && (best == nullptr || (int) info.source < (int) best->source))
                    best = &info;
            if (best != nullptr && processor.loadSample (l, best->file))
                ++loaded;
        }
        if (onMessage != nullptr)
            onMessage (loaded == 0 ? tr ("No drum samples found in") + " " + tr ("Your samples")
                                   : tr ("Your samples") + ": " + juce::String (loaded) + " " + tr ("drums"));
        list.repaint();
        repaint();
    };
    Localisation::bindText (allButton, "ALL FILES");
    Localisation::bindTip (allButton, "Also the other samples of the folder (any of them can play this drum)");
    allButton.setClickingTogglesState (true);
    allButton.onClick = [this] { showAll = allButton.getToggleState(); rebuildList(); };
    Localisation::bindText (builtInButton, "BUILT-IN");
    Localisation::bindTip (builtInButton, "This drum back to the built-in sound");
    builtInButton.onClick = [this] { processor.clearSample (lane); list.repaint(); repaint(); };
    Localisation::bindText (allBuiltInButton, "ALL BUILT-IN");
    Localisation::bindTip (allBuiltInButton, "Every drum back to the built-in kit");
    allBuiltInButton.onClick = [this] { processor.clearAllSamples(); list.repaint(); repaint(); };
    Localisation::bindText (fileButton, "ONE FILE...");
    Localisation::bindTip (fileButton, "A single sample file for this drum, from anywhere");
    fileButton.onClick = [this]
    {
        const int forLane = lane;
        chooser = std::make_unique<juce::FileChooser> (tr ("Your own sound for this drum") + ": " + tr (fgd::laneName (forLane)),
                                                       folder, "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
        juce::Component::SafePointer<SampleBrowser> safe (this);
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe, forLane] (const juce::FileChooser& fc)
        {
            if (safe != nullptr && fc.getResult().existsAsFile() && safe->processor.loadSample (forLane, fc.getResult()))
            {
                if (safe->onMessage != nullptr)
                    safe->onMessage (tr (fgd::laneName (forLane)) + ": " + fc.getResult().getFileName());
                safe->repaint();
            }
        });
    };
    Localisation::bindText (closeButton, "CLOSE");
    closeButton.onClick = [this] { setVisible (false); };
    for (auto* b : { &folderButton, &foldersButton, &clearListButton, &autoButton, &allButton, &closeButton, &chooseButton, &builtInButton, &fileButton, &allBuiltInButton })
        addAndMakeVisible (b);
    chooseButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    chooseButton.setColour (juce::TextButton::textColourOffId, Palette::background);
}

SampleBrowser::~SampleBrowser() = default;

juce::Rectangle<float> SampleBrowser::card() const
{
    return getLocalBounds().toFloat().withSizeKeepingCentre (1060.0f, 760.0f);
}

juce::Rectangle<float> SampleBrowser::drumBounds (int l) const
{
    auto c = card().reduced (22.0f);
    return { c.getX(), c.getY() + 100.0f + 44.0f * (float) l, 250.0f, 40.0f };
}

int SampleBrowser::drumAt (juce::Point<float> p) const
{
    for (int l = 0; l < fgd::numLanes; ++l)
        if (drumBounds (l).contains (p))
            return l;
    return -1;
}

void SampleBrowser::resized()
{
    auto c = card().reduced (22.0f).toNearestInt();
    auto top = c.removeFromTop (30);
    closeButton.setBounds (top.removeFromRight (90));
    top.removeFromRight (8);
    allBuiltInButton.setBounds (top.removeFromRight (120));
    top.removeFromRight (8);
    autoButton.setBounds (top.removeFromRight (100));
    top.removeFromRight (8);
    clearListButton.setBounds (top.removeFromRight (110));
    top.removeFromRight (8);
    folderButton.setBounds (top.removeFromRight (130));
    top.removeFromRight (8);
    foldersButton.setBounds (top.removeFromRight (190));

    auto listArea = juce::Rectangle<int> (c.getX() + 266, c.getY() + 70, c.getWidth() - 266, c.getHeight() - 96);
    auto tools = listArea.removeFromTop (30);
    allButton.setBounds (tools.removeFromRight (110).withTrimmedBottom (4));
    tools.removeFromRight (6);
    fileButton.setBounds (tools.removeFromRight (110).withTrimmedBottom (4));
    tools.removeFromRight (6);
    builtInButton.setBounds (tools.removeFromRight (100).withTrimmedBottom (4));
    list.setBounds (listArea);
    chooseButton.setBounds (listArea.withSizeKeepingCentre (340, 46));

    const bool haveFolder = ! folders.isEmpty();
    chooseButton.setVisible (! haveFolder);
    list.setVisible (haveFolder);
    for (auto* b : { &autoButton, &allButton, &clearListButton })
        b->setEnabled (haveFolder);
    foldersButton.setVisible (haveFolder);
    foldersButton.setButtonText (tr ("FOLDERS READ") + " (" + juce::String (folders.size()) + ")  " + juce::String::fromUTF8 ("\xe2\x96\xbe"));
}

void SampleBrowser::show (const juce::File& f, int forLane)
{
    lane = juce::jlimit (0, fgd::numLanes - 1, forLane);
    setVisible (true);
    toFront (true);
    grabKeyboardFocus();
    if (f.isDirectory())
    {
        if (! folders.contains (f))
            scan (f);
    }
    else if (folders.isEmpty())
    {
        // The folders of last time.
        auto saved = processor.sampleFolders;
        if (saved.isEmpty() && processor.sampleFolder.isDirectory())
            saved.add (processor.sampleFolder.getFullPathName());
        for (const auto& path : saved)
            if (juce::File (path).isDirectory())
                scan (juce::File (path));
    }
    rebuildList();
    resized();
    repaint();
}

void SampleBrowser::chooseFolder()
{
    chooser = std::make_unique<juce::FileChooser> (tr ("Choose a folder of samples"), folder, "");
    juce::Component::SafePointer<SampleBrowser> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [safe] (const juce::FileChooser& fc)
    {
        if (safe != nullptr && fc.getResult().isDirectory())
            safe->scan (fc.getResult());
    });
}

void SampleBrowser::scan (const juce::File& f)
{
    folder = f;
    if (! folders.contains (f))
        folders.add (f);
    processor.sampleFolder = f;
    processor.sampleFolders.clear();
    for (const auto& d : folders)
        processor.sampleFolders.add (d.getFullPathName());

    // Read again: what this folder gave before goes.
    found.erase (std::remove_if (found.begin(), found.end(), [&f] (const auto& i) { return i.file.isAChildOf (f); }), found.end());
    ++pendingScans;
    scanning = true;
    progress->store (0.0f);
    startTimerHz (15);
    rebuildList();
    resized();
    repaint();

    // Reading the samples (and listening to the unnamed ones) takes a moment: in the background
    // (with a progress bar), unless the folder is small.
    if (f.findChildFiles (juce::File::findFiles, true, "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg").size() < 40)
    {
        auto result = FaseGrooveDrumsProcessor::scanSampleFolder (f);
        found.insert (found.end(), result.begin(), result.end());
        scanFinished();
        return;
    }
    juce::Component::SafePointer<SampleBrowser> safe (this);
    auto shared = progress;
    juce::Thread::launch ([f, safe, shared]
    {
        auto result = FaseGrooveDrumsProcessor::scanSampleFolder (f, 4000, [shared] (float x) { shared->store (x); });
        juce::MessageManager::callAsync ([result = std::move (result), safe, f]() mutable
        {
            if (safe == nullptr)
                return;
            if (safe->folders.contains (f))
                safe->found.insert (safe->found.end(), result.begin(), result.end());
            safe->scanFinished();
        });
    });
}

void SampleBrowser::scanFinished()
{
    if (--pendingScans <= 0)
    {
        pendingScans = 0;
        scanning = false;
        stopTimer();
    }
    rebuildList();
    repaint();
}

void SampleBrowser::showFoldersMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader (tr ("Your samples are read from these folders (and their subfolders)"));
    for (int i = 0; i < folders.size(); ++i)
    {
        const auto& d = folders.getReference (i);
        int count = 0;
        for (const auto& info : found)
            count += info.file.isAChildOf (d) ? 1 : 0;
        juce::PopupMenu one;
        one.addItem (1000 + i, tr ("Leave it out (the drums keep their samples)"));
        one.addItem (2000 + i, tr ("Read it again"));
        one.addItem (3000 + i, tr ("Open the folder"));
        menu.addSubMenu (d.getFullPathName() + "   (" + juce::String (count) + " " + tr ("samples") + ")", one);
    }
    menu.addSeparator();
    menu.addItem (1, tr ("ADD FOLDER..."));

    juce::Component::SafePointer<SampleBrowser> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&foldersButton), [safe] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        if (result == 1)
            return safe->chooseFolder();
        const int index = result % 1000;
        if (index >= safe->folders.size())
            return;
        const auto d = safe->folders[index];
        if (result >= 3000)
            d.revealToUser();
        else if (result >= 2000)
            safe->scan (d);
        else
            safe->removeFolder (d);
    });
}

void SampleBrowser::removeFolder (const juce::File& d)
{
    folders.removeFirstMatchingValue (d);
    // Its samples go from the list, unless another folder of the list has them too.
    found.erase (std::remove_if (found.begin(), found.end(), [this, &d] (const auto& info)
    {
        if (! info.file.isAChildOf (d))
            return false;
        for (const auto& other : folders)
            if (info.file.isAChildOf (other))
                return false;
        return true;
    }), found.end());
    processor.sampleFolders.clear();
    for (const auto& f : folders)
        processor.sampleFolders.add (f.getFullPathName());
    folder = folders.isEmpty() ? juce::File() : folders.getLast();
    processor.sampleFolder = folder;
    rebuildList();
    resized();
    repaint();
}

juce::String SampleBrowser::relativePath (const juce::File& file) const
{
    for (const auto& d : folders)
        if (file.isAChildOf (d))
            return file.getParentDirectory().getRelativePathFrom (d.getParentDirectory());
    return file.getParentDirectory().getFullPathName();
}

int SampleBrowser::countFor (int l) const
{
    return (int) std::count_if (found.begin(), found.end(), [l] (const auto& i) { return i.lane == l; });
}

void SampleBrowser::rebuildList()
{
    rows.clear();
    std::vector<int> others;
    for (int i = 0; i < (int) found.size(); ++i)
        (found[(size_t) i].lane == lane ? rows : others).push_back (i);
    std::stable_sort (rows.begin(), rows.end(), [this] (int a, int b) { return (int) found[(size_t) a].source < (int) found[(size_t) b].source; });
    if (showAll)
    {
        // The samples nothing names that sound like this drum first, then all the others.
        std::stable_partition (others.begin(), others.end(), [this] (int i) { return found[(size_t) i].lane < 0 && found[(size_t) i].guess == lane; });
        rows.insert (rows.end(), others.begin(), others.end());
    }
    list.updateContent();
    list.deselectAllRows();
    list.repaint();
}

int SampleBrowser::getNumRows()
{
    return (int) rows.size();
}

juce::var SampleBrowser::getDragSourceDescription (const juce::SparseSet<int>&)
{
    return "fgd-samples";
}

void SampleBrowser::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;
    const auto& info = found[(size_t) rows[(size_t) row]];
    const bool inUse = processor.samplePath (lane) == info.file.getFullPathName();
    const bool other = info.lane != lane;
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (3.0f, 2.0f);

    g.setColour (selected ? Palette::accent2.withAlpha (0.22f) : row % 2 == 0 ? Palette::control.withAlpha (0.5f) : Palette::field);
    g.fillRoundedRectangle (r, 4.0f);
    if (inUse)
    {
        g.setColour (laneColour (lane));
        g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.5f);
    }

    IconButton::drawIcon (g, IconButton::Icon::play, r.withWidth (26.0f).reduced (7.0f), selected ? Palette::accent2 : Palette::dim);
    auto text = r.withTrimmedLeft (30.0f);

    // USE on the right, then the drum it was recognised as, and how.
    auto useArea = text.removeFromRight (90.0f).reduced (4.0f, 5.0f);
    g.setColour (inUse ? laneColour (lane) : Palette::panelEdge.brighter (0.15f));
    g.fillRoundedRectangle (useArea, 4.0f);
    g.setColour (inUse ? Palette::background : Palette::text);
    g.setFont (font (11.0f, true));
    g.drawFittedText (Localisation::upper (tr (inUse ? "in use" : "USE")), useArea.toNearestInt(), juce::Justification::centred, 1, 0.7f);

    using Source = FaseGrooveDrumsProcessor::SampleInfo::Source;
    const char* how = info.source == Source::manual ? "set by you" : info.source == Source::name ? "by name"
                    : info.source == Source::folder ? "by folder" : info.source == Source::sound ? "by sound" : "unknown";
    auto tag = text.removeFromRight (170.0f).reduced (4.0f, 8.0f);
    const int shownLane = info.lane >= 0 ? info.lane : info.guess;
    const auto tagColour = shownLane >= 0 ? laneColour (shownLane) : Palette::dim;
    g.setColour (tagColour.withAlpha (other ? 0.3f : 0.15f));
    g.fillRoundedRectangle (tag, 4.0f);
    g.setColour (other ? tagColour : Palette::dim);
    g.setFont (font (10.0f, true));
    const auto drum = info.lane >= 0 ? tr (fgd::laneShortName (info.lane))
                    : info.guess >= 0 ? tr ("maybe") + " " + tr (fgd::laneShortName (info.guess)) : tr ("unknown");
    g.drawFittedText (drum + (info.lane >= 0 || info.guess >= 0 ? " - " + tr (how) : juce::String()), tag.toNearestInt(),
                      juce::Justification::centred, 1, 0.7f);

    g.setColour (Palette::text);
    g.setFont (font (13.0f, true));
    g.drawFittedText (info.file.getFileNameWithoutExtension(), text.removeFromTop (text.getHeight() * 0.58f).toNearestInt(),
                      juce::Justification::bottomLeft, 1, 0.7f);
    g.setColour (Palette::dim);
    g.setFont (font (10.0f));
    g.drawFittedText (relativePath (info.file), text.toNearestInt(),
                      juce::Justification::topLeft, 1, 0.7f);
}

void SampleBrowser::selectedRowsChanged (int row)
{
    if (juce::isPositiveAndBelow (row, (int) rows.size()) && list.getNumSelectedRows() == 1)
        processor.previewSampleFile (found[(size_t) rows[(size_t) row]].file);
}

void SampleBrowser::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
        showRowMenu (row);
    else if (e.x > list.getVisibleRowWidth() - 96)
        use (row);
    else if (! e.mods.isShiftDown() && ! e.mods.isCommandDown() && juce::isPositiveAndBelow (row, (int) rows.size()))
        processor.previewSampleFile (found[(size_t) rows[(size_t) row]].file);   // again
}

void SampleBrowser::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    use (row);
}

void SampleBrowser::use (int row)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;
    const auto& file = found[(size_t) rows[(size_t) row]].file;
    if (processor.loadSample (lane, file) && onMessage != nullptr)
        onMessage (tr (fgd::laneName (lane)) + ": " + file.getFileName());
    list.repaint();
    repaint();
}

void SampleBrowser::moveSelectedTo (int newLane)
{
    // The samples picked are that drum's from now on (remembered for this folder next time too).
    if (! juce::isPositiveAndBelow (newLane, fgd::numLanes))
        return;
    const auto selected = list.getSelectedRows();
    int moved = 0;
    for (int k = 0; k < selected.size(); ++k)
        if (const int row = selected[k]; juce::isPositiveAndBelow (row, (int) rows.size()))
        {
            auto& info = found[(size_t) rows[(size_t) row]];
            info.lane = newLane;
            info.source = FaseGrooveDrumsProcessor::SampleInfo::Source::manual;
            FaseGrooveDrumsProcessor::tagSample (info.file, newLane);
            ++moved;
        }
    if (moved > 0 && onMessage != nullptr)
        onMessage (juce::String (moved) + " -> " + tr (fgd::laneName (newLane)));
    rebuildList();
    repaint();
}

void SampleBrowser::showRowMenu (int row)
{
    if (! list.isRowSelected (row))
        list.selectRow (row);
    juce::PopupMenu menu, move;
    menu.addItem (1, tr ("USE") + ": " + tr (fgd::laneName (lane)));
    for (int l = 0; l < fgd::numLanes; ++l)
        move.addItem (100 + l, tr (fgd::laneName (l)), l != lane || showAll);
    menu.addSubMenu (tr ("It is a..."), move);
    juce::Component::SafePointer<SampleBrowser> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&list).withMousePosition(), [safe, row] (int result)
    {
        if (safe == nullptr)
            return;
        if (result == 1)
            safe->use (row);
        else if (result >= 100)
            safe->moveSelectedTo (result - 100);
    });
}

bool SampleBrowser::isInterestedInDragSource (const SourceDetails& details)
{
    return details.description == "fgd-samples";
}

void SampleBrowser::itemDragMove (const SourceDetails& details)
{
    if (const int l = drumAt (details.localPosition.toFloat()); l != dropLane)
    {
        dropLane = l;
        repaint();
    }
}

void SampleBrowser::itemDragExit (const SourceDetails&)
{
    dropLane = -1;
    repaint();
}

void SampleBrowser::itemDropped (const SourceDetails& details)
{
    dropLane = -1;
    moveSelectedTo (drumAt (details.localPosition.toFloat()));
}

void SampleBrowser::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));
    const auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (22.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText (Localisation::upper (tr ("Your samples")), inner.removeFromTop (30.0f), juce::Justification::centredLeft);

    // The three steps, always in sight.
    g.setColour (Palette::text.withAlpha (0.9f));
    g.setFont (font (12.5f, true));
    g.drawFittedText (tr ("1. Pick a drum on the left   2. Click a sample: you hear it   3. USE: the drum plays it"),
                      inner.removeFromTop (22.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    g.drawFittedText (tr ("Wrong drum? Drag the samples (shift / ctrl: several) onto the right drum, or right-click: It is a..."),
                      inner.removeFromTop (18.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);

    // The folder, and the drum whose samples are listed.
    const auto base = c.reduced (22.0f);
    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.setColour (laneColour (lane));
    g.setFont (font (15.0f, true));
    g.drawFittedText (Localisation::upper (tr (fgd::laneName (lane))) + "  -  " + juce::String ((int) rows.size()) + " " + tr ("samples"),
                      juce::Rectangle<float> (base.getX() + 266.0f, base.getY() + 100.0f, 330.0f, 26.0f).toNearestInt(),
                      juce::Justification::centredLeft, 1, 0.7f);

    // The drums: their sample now and how many the folder has for them (a drop target too).
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        auto r = drumBounds (l);
        const bool on = l == lane, drop = l == dropLane;
        g.setColour (drop ? laneColour (l).withAlpha (0.35f) : on ? Palette::control.brighter (0.2f) : Palette::control);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (laneColour (l));
        g.fillRoundedRectangle (r.withWidth (4.0f), 2.0f);
        if (on || drop)
            g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, drop ? 2.0f : 1.2f);
        auto t = r.reduced (12.0f, 3.0f);
        g.setColour (Palette::dim);
        g.setFont (font (11.0f));
        g.drawText (juce::String (countFor (l)), t.removeFromRight (30.0f), juce::Justification::centredRight);
        g.setColour (Palette::text);
        g.setFont (font (13.0f, true));
        g.drawText (tr (fgd::laneName (l)), t.removeFromTop (t.getHeight() * 0.55f), juce::Justification::bottomLeft);
        const auto path = processor.samplePath (l);
        g.setColour (path.isNotEmpty() ? laneColour (l) : Palette::dim);
        g.setFont (font (10.0f));
        g.drawFittedText (path.isNotEmpty() ? juce::File (path).getFileNameWithoutExtension() : tr ("Built-in"), t.toNearestInt(),
                          juce::Justification::topLeft, 1, 0.7f);
    }

    // Under the list, what is going on.
    auto hint = c.reduced (22.0f).removeFromBottom (22.0f).withTrimmedLeft (266.0f);
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    if (scanning)
    {
        // How far the reading is.
        const float done = juce::jlimit (0.0f, 1.0f, progress->load());
        auto bar = hint.withTrimmedLeft (230.0f).withSizeKeepingCentre (hint.getWidth() - 230.0f, 10.0f);
        g.setColour (Palette::panelEdge);
        g.fillRoundedRectangle (bar, 5.0f);
        g.setColour (Palette::accent);
        g.fillRoundedRectangle (bar.withWidth (juce::jmax (10.0f, bar.getWidth() * done)), 5.0f);
        g.setColour (Palette::text);
        g.drawFittedText (tr ("Scanning the folder...") + " " + juce::String (juce::roundToInt (done * 100.0f)) + "%", hint.withWidth (224.0f).toNearestInt(),
                          juce::Justification::centredLeft, 1, 0.7f);
    }
    else if (folders.isEmpty())
        g.drawFittedText (tr ("Choose a folder of samples: kicks, claps, hats... (or drop a folder on the window)"), hint.toNearestInt(),
                          juce::Justification::centredLeft, 1, 0.7f);
    else
        g.drawFittedText (rows.empty() ? tr ("No samples for this drum in the folder (try ALL FILES)")
                          : tr ("Click: listen (arrows: the next one). USE or double-click: this drum plays it."),
                          hint.toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);
}

void SampleBrowser::mouseDown (const juce::MouseEvent& e)
{
    if (const int l = drumAt (e.position); l >= 0)
    {
        lane = l;
        processor.audition (l);
        rebuildList();
        repaint();
        return;
    }
    if (! card().contains (e.position))
        setVisible (false);
}

bool SampleBrowser::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        setVisible (false);
        return true;
    }
    return false;
}

//==============================================================================
MixerView::MixerView (FaseGrooveDrumsProcessor& p) : processor (p)
{
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        auto* f = faders.add (new juce::Slider (juce::Slider::LinearVertical, juce::Slider::TextBoxBelow));
        f->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
        f->setColour (juce::Slider::trackColourId, laneColour (l));
        f->setColour (juce::Slider::thumbColourId, Palette::text);
        f->setColour (juce::Slider::backgroundColourId, Palette::field);
        f->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        Localisation::bind (*f, [f, l] { f->setTooltip (tr (fgd::laneName (l)) + ": " + tr ("volume of the guide sound (your sample too). The MIDI velocities don't change: LEVEL does that")); });
        f->setDoubleClickReturnValue (true, 0.0);
        addAndMakeVisible (f);
        faderAttachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (processor.apvts, ParamIDs::laneVolume (l), *f));

        auto* m = mutes.add (new juce::TextButton ("M"));
        m->setClickingTogglesState (true);
        m->setColour (juce::TextButton::buttonOnColourId, Palette::toms);
        Localisation::bind (*m, [m, l] { m->setTooltip (tr ("Mute") + ": " + tr (fgd::laneName (l))); });
        addAndMakeVisible (m);
        muteAttachments.add (new juce::AudioProcessorValueTreeState::ButtonAttachment (processor.apvts, ParamIDs::mute (l), *m));

        // The drum's feel: ahead of / behind the beat, and its share of the swing.
        const bool kick = l == (int) fgd::Lane::kick;
        for (int k = 0; k < 2; ++k)
        {
            auto* knob = (k == 0 ? pushKnobs : swingKnobs).add (new juce::Slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
            knob->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 13);
            knob->setColour (juce::Slider::rotarySliderFillColourId, laneColour (l));
            knob->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            knob->setDoubleClickReturnValue (true, k == 0 ? 0.0 : 1.0);
            knob->setEnabled (! kick);
            if (k == 0)
                Localisation::bind (*knob, [knob, l, kick] { knob->setTooltip (tr (fgd::laneName (l)) + ": " + tr (kick ? "the kick always stays on the grid"
                                                                                                                   : "PUSH: ahead of the beat (-) or laid back (+), in milliseconds")); });
            else
                Localisation::bind (*knob, [knob, l, kick] { knob->setTooltip (tr (fgd::laneName (l)) + ": " + tr (kick ? "the kick always stays on the grid"
                                                                                                                   : "SWING: how much of the swing this drum takes (100% = as the Swing knob)")); });
            addAndMakeVisible (knob);
            knobAttachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (processor.apvts, k == 0 ? ParamIDs::push (l) : ParamIDs::laneSwing (l), *knob));
        }
    }
}

juce::Rectangle<int> MixerView::strip (int lane) const
{
    const int w = getWidth() / fgd::numLanes;
    return { lane * w, 0, w, getHeight() };
}

void MixerView::resized()
{
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        auto r = strip (l).reduced (4, 2);
        r.removeFromTop (30);
        r.removeFromTop (10);
        pushKnobs[l]->setBounds (r.removeFromTop (40));
        r.removeFromTop (10);
        swingKnobs[l]->setBounds (r.removeFromTop (40));
        mutes[l]->setBounds (r.removeFromBottom (22).reduced (6, 0));
        r.removeFromBottom (4);
        faders[l]->setBounds (r);
    }
}

void MixerView::paint (juce::Graphics& g)
{
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        auto r = strip (l).reduced (2, 0).toFloat();
        g.setColour (Palette::field);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (laneColour (l).withAlpha (0.9f));
        g.fillRoundedRectangle (r.withHeight (3.0f).reduced (6.0f, 0.0f), 1.5f);
        g.setColour (Palette::text);
        g.setFont (font (10.5f, true));
        g.drawText (tr (fgd::laneShortName (l)), r.withTrimmedTop (5.0f).withHeight (14.0f), juce::Justification::centred);
        const float light = juce::jlimit (0.0f, 1.0f, lights[(size_t) l]);
        auto led = juce::Rectangle<float> (r.getWidth() - 20.0f, 4.0f).withCentre ({ r.getCentreX(), r.getY() + 24.0f });
        g.setColour (Palette::panelEdge);
        g.fillRoundedRectangle (led, 2.0f);
        g.setColour (laneColour (l));
        g.fillRoundedRectangle (led.withWidth (led.getWidth() * light), 2.0f);

        g.setColour (Palette::dim);
        g.setFont (font (8.5f, true));
        g.drawText (Localisation::upper (tr ("Push")), r.withTrimmedTop (32.0f).withHeight (10.0f), juce::Justification::centred);
        g.drawText (Localisation::upper (tr ("Swing")), r.withTrimmedTop (82.0f).withHeight (10.0f), juce::Justification::centred);
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
        g.drawText (tr (fgd::laneShortName (lane)), text.removeFromTop (text.getHeight() * 0.6f), juce::Justification::bottomLeft);
        g.setFont (font (9.5f));
        g.setColour (light > 0.5f ? Palette::background.withAlpha (0.7f) : Palette::dim);
        g.drawText (muted ? tr ("MUTE") : processor.samplePath (lane).isNotEmpty() ? tr ("SAMPLE")
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
void GrooveOverview::refresh()
{
    pattern = processor.displayedPattern();
    for (int l = 0; l < fgd::numLanes; ++l)
        muted[(size_t) l] = processor.isLaneMuted (l);
    playStep = processor.uiStep.load();
    repaint();
}

void GrooveOverview::paint (juce::Graphics& g)
{
    const int steps = juce::jmax (1, pattern.length());
    const float label = 64.0f, rowH = (float) getHeight() / (float) fgd::numLanes;
    const float cellW = ((float) getWidth() - label) / (float) steps;
    if (juce::isPositiveAndBelow (playStep, steps))
    {
        g.setColour (Palette::accent2.withAlpha (0.14f));
        g.fillRect (label + cellW * (float) playStep, 0.0f, cellW, (float) getHeight());
    }
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const float y = rowH * (float) l;
        const auto colour = laneColour (l);
        g.setColour (muted[(size_t) l] ? Palette::dim.withAlpha (0.5f) : Palette::text.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (juce::jmin (10.5f, rowH * 0.8f)).withStyle ("Bold")));
        g.drawText (tr (fgd::laneShortName (l)), juce::Rectangle<float> (4.0f, y, label - 8.0f, rowH), juce::Justification::centredLeft);
        const auto& line = pattern.lanes[(size_t) l];
        for (int s = 0; s < steps; ++s)
        {
            auto cell = juce::Rectangle<float> (label + cellW * (float) s, y, cellW, rowH).reduced (0.8f, 0.8f);
            g.setColour (Palette::panelEdge.withAlpha ((s / 4) % 2 == 0 ? 0.5f : 0.3f));
            g.fillRect (cell);
            const char c = s < (int) line.size() ? line[(size_t) s] : '.';
            if (fgd::isHit (c))
            {
                g.setColour (colour.withAlpha ((muted[(size_t) l] ? 0.3f : 1.0f) * (c == 'X' ? 1.0f : c == 'g' ? 0.4f : 0.72f)));
                g.fillRect (c == 'g' ? cell.withSizeKeepingCentre (cell.getWidth() * 0.5f, cell.getHeight() * 0.5f) : cell);
            }
        }
    }
    // The parts A-D.
    g.setColour (Palette::dim.withAlpha (0.6f));
    for (int k = 1; k < 4; ++k)
        g.drawVerticalLine (juce::roundToInt (label + cellW * (float) (k * steps / 4)), 0.0f, (float) getHeight());
}

MiniMixer::MiniMixer (FaseGrooveDrumsProcessor& p) : processor (p)
{
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        auto* f = faders.add (new juce::Slider (juce::Slider::LinearVertical, juce::Slider::NoTextBox));
        f->setColour (juce::Slider::trackColourId, laneColour (l));
        f->setColour (juce::Slider::thumbColourId, Palette::text);
        f->setColour (juce::Slider::backgroundColourId, Palette::field);
        f->setDoubleClickReturnValue (true, 0.0);
        f->setPopupDisplayEnabled (true, true, nullptr);
        Localisation::bind (*f, [f, l] { f->setTooltip (tr (fgd::laneName (l)) + ": " + tr ("volume in the guide sound (double-click: 0 dB)")); });
        addAndMakeVisible (f);
        attachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (processor.apvts, ParamIDs::laneVolume (l), *f));
    }
}

juce::Rectangle<int> MiniMixer::strip (int lane) const
{
    const int w = getWidth() / fgd::numLanes;
    return { lane * w, 0, w, getHeight() };
}

void MiniMixer::resized()
{
    for (int l = 0; l < fgd::numLanes; ++l)
        faders[l]->setBounds (strip (l).reduced (6, 0).withTrimmedBottom (15));
}

void MiniMixer::paint (juce::Graphics& g)
{
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        auto r = strip (l);
        g.setColour (processor.isLaneMuted (l) ? Palette::dim.withAlpha (0.5f) : Palette::dim);
        g.setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        g.drawFittedText (tr (fgd::laneShortName (l)), r.removeFromBottom (14), juce::Justification::centred, 1, 0.6f);
    }
}

EditorOverlay::EditorOverlay()
{
    for (juce::Button* b : std::initializer_list<juce::Button*> { &rewindButton, &playButton, &stopButton, &closeButton,
                                                                  &accentBrush, &hitBrush, &ghostBrush, &flamBrush })
    {
        b->setWantsKeyboardFocus (false);
        addAndMakeVisible (b);
    }
    rewindButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    playButton.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    stopButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    playButton.onColour = Palette::accent2;
    accentBrush.setConnectedEdges (juce::Button::ConnectedOnRight);
    hitBrush.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    ghostBrush.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    flamBrush.setConnectedEdges (juce::Button::ConnectedOnLeft);
    accentBrush.onClick = [this] { setBrush ('X'); };
    hitBrush.onClick    = [this] { setBrush ('x'); };
    ghostBrush.onClick  = [this] { setBrush ('g'); };
    flamBrush.onClick   = [this] { setBrush ('f'); };
    closeButton.onClick = [this] { setVisible (false); };
    Localisation::bind (*this, [this] { refreshText(); });
    setBrush ('x');
}

void EditorOverlay::setBrush (char symbol)
{
    brush = symbol;
    accentBrush.setToggleState (brush == 'X', juce::dontSendNotification);
    hitBrush.setToggleState (brush == 'x', juce::dontSendNotification);
    ghostBrush.setToggleState (brush == 'g', juce::dontSendNotification);
    flamBrush.setToggleState (brush == 'f', juce::dontSendNotification);
    if (onBrush != nullptr)
        onBrush (brush);
}

void EditorOverlay::refreshText()
{
    accentBrush.setButtonText (tr ("ACCENT"));
    hitBrush.setButtonText (tr ("HIT"));
    ghostBrush.setButtonText (tr ("GHOST"));
    flamBrush.setButtonText (tr ("FLAM"));
    const auto tip = tr ("What a click on an empty step draws (Shift: accent, Alt: ghost, Ctrl: flam work too)");
    for (auto* b : { &accentBrush, &hitBrush, &ghostBrush, &flamBrush })
        b->setTooltip (tip);
    rewindButton.setTooltip (tr ("Back to the start"));
    playButton.setTooltip (tr ("Play / pause the groove without the DAW (space bar)"));
    stopButton.setTooltip (tr ("Stop and go back to the start"));
    closeButton.setTooltip (tr ("CLOSE"));
    repaint();
}

void EditorOverlay::paint (juce::Graphics& g)
{
    g.fillAll (Palette::background.withAlpha (0.97f));
    const auto card = getLocalBounds().toFloat().reduced (10.0f);
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (card, 10.0f);
    g.setColour (Palette::panelEdge);
    g.drawRoundedRectangle (card.reduced (0.5f), 10.0f, 1.0f);
    g.setColour (Palette::accent);
    g.fillRoundedRectangle (28.0f, 30.0f, 3.0f, 14.0f, 1.0f);
    g.setColour (Palette::text);
    g.setFont (juce::Font (juce::FontOptions (15.0f).withStyle ("Bold")));
    g.drawText (Localisation::upper (tr ("Edit groove")), 38, 22, 260, 30, juce::Justification::centredLeft);

    g.setColour (Palette::dim);
    g.setFont (juce::Font (juce::FontOptions (10.5f).withStyle ("Bold")));
    auto caption = [&g] (const juce::String& text, int x, int w)
    {
        g.drawFittedText (Localisation::upper (tr (text)), { x, 58, w, 14 }, juce::Justification::centredLeft, 1, 0.7f);
    };
    caption ("Transport", 28, 120);
    caption ("Brush", 174, 120);
    caption ("View", 476, 120);
    caption ("New rhythm", 784, 90);
    caption ("Groove", 874, 90);
    caption (viewMode != nullptr && viewMode() == 2 ? "Feel" : "Template", 1062, 96);
}

//==============================================================================
FaseGrooveDrumsEditor::FaseGrooveDrumsEditor (FaseGrooveDrumsProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      grooveCombo (p.apvts, ParamIDs::pattern, "Groove"),
      genreCombo (p.apvts, ParamIDs::genre, "Beat genre"),
      partLengthCombo (p.apvts, ParamIDs::partLength, "Part length"),
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
      grid (p), velocityView (p), mixerView (p), fillView (p)
{
    Localisation::loadSaved();
    setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (content);
    content.setWantsKeyboardFocus (true);

    auto show = [this] (juce::Component& c) { content.addAndMakeVisible (c); };

    // ---- Header: presets, total random, undo / redo, full screen; style, reset, options ----
    Localisation::bindTip (presetsButton, "Your presets: save everything, load it in any project");
    presetsButton.onClick = [this] { showPresetsMenu(); };
    Localisation::bindText (totalRandomButton, "TOTAL");
    totalRandomButton.iconColour = Palette::accent;
    Localisation::bindTip (totalRandomButton, "Total random: style, groove or new beat, swing, feel, fills and kit all at once (undo with the arrow)");
    totalRandomButton.onClick = [this] { processor.totalRandom(); };
    Localisation::bindTip (undoButton, "Undo (Ctrl+Z)");
    undoButton.onClick = [this] { undo(); };
    undoButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    Localisation::bindTip (redoButton, "Redo (Ctrl+Shift+Z)");
    redoButton.onClick = [this] { redo(); };
    redoButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    Localisation::bindTip (fullScreenButton, "Full screen");
    fullScreenButton.onClick = [this] { toggleFullScreen(); };
    Localisation::bindTip (styleButton, "A groove, its swing, feel, fills and kit for a genre, in one click");
    styleButton.onClick = [this]
    {
        juce::PopupMenu menu;
        menu.addSectionHeader (tr ("One-click setups by genre"));
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
    Localisation::bindTip (resetButton, "Every setting back to its default (undo with the arrow)");
    resetButton.onClick = [this] { processor.resetToDefaults(); };
    Localisation::bindText (optionsButton, "OPTIONS");
    Localisation::bindTip (optionsButton, "Language, interface size, tips and credits");
    optionsButton.onClick = [this] { options.show (false, currentScale()); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &presetsButton, &totalRandomButton, &undoButton, &redoButton,
                                                                        &fullScreenButton, &styleButton, &resetButton, &optionsButton })
        show (*c);

    // ---- Transport & tempo ----
    Localisation::bindTip (rewindButton, "Back to the start");
    rewindButton.onClick = [this] { processor.previewToStart(); };
    rewindButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    Localisation::bindTip (playButton, "Play / pause the groove without the DAW (space bar)");
    playButton.onClick = [this] { processor.previewToggle(); };
    playButton.onColour = Palette::accent2;
    playButton.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    Localisation::bindTip (stopButton, "Stop and go back to the start");
    stopButton.onClick = [this] { processor.previewStop(); };
    stopButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    Localisation::bindTip (sequencerToggle, "Off: the groove doesn't play with the DAW (e.g. while the dragged clip plays on this track). The play button still previews");
    sequencerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sequencer, sequencerToggle);
    syncButton.setClickingTogglesState (true);
    Localisation::bindTip (syncButton, "On: the DAW's tempo and position. Off: your own tempo (still started by the DAW's play)");
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::tempoSync, syncButton);
    tempoSlider.setSliderStyle (juce::Slider::LinearBar);
    Localisation::bindTip (tempoSlider, "Tempo when SYNC DAW is off (drag or double-click to type)");
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, ParamIDs::tempo, tempoSlider);
    dawTempo.setColour (juce::Label::backgroundColourId, Palette::background);
    dawTempo.setColour (juce::Label::textColourId, Palette::text);
    dawTempo.setJustificationType (juce::Justification::centred);
    dawTempo.setFont (font (15.0f, true));
    for (juce::Component* c : std::initializer_list<juce::Component*> { &rewindButton, &playButton, &stopButton, &sequencerToggle, &syncButton, &tempoSlider, &dawTempo })
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
        box.addSectionHeading (tr ("Your beats"));
        box.addItem (tr ("Random beat"), fgd::randomPatternIndex() + 1);
        box.addItem (tr ("User pattern (edited on the grid)"), fgd::userPatternIndex() + 1);
        grooveCombo.refresh();
    }
    grooveCombo.setTip ("The groove, by genre (the same genres as Fase Groove Piano). Edit any groove on the grid");
    Localisation::bindTip (prevGroove, "Previous groove");
    prevGroove.onClick = [this] { stepGroove (-1); };
    prevGroove.setConnectedEdges (juce::Button::ConnectedOnRight);
    Localisation::bindTip (nextGroove, "Next groove");
    nextGroove.onClick = [this] { stepGroove (1); };
    nextGroove.setConnectedEdges (juce::Button::ConnectedOnLeft);
    genreCombo.setTip ("The genre the dice write in");
    Localisation::bindText (newBeatButton, "RANDOM");
    newBeatButton.iconColour = Palette::accent;
    Localisation::bindTip (newBeatButton, "A brand new random beat in the chosen genre (locked parts stay)");
    newBeatButton.onClick = [this] { processor.rollNewBeat(); };
    partLengthCombo.setTip ("How long each part A-D is: half a bar (a 2-bar groove) or one bar (a 4-bar groove)");
    densitySlider.setSliderStyle (juce::Slider::LinearBar);
    Localisation::bindTip (densitySlider, "How busy the dice are: from kick, clap and hats to 16ths and percussion");
    densityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, ParamIDs::density, densitySlider);
    Localisation::bindTip (grooveButton, "The groove: click to open the browser (genres, grooves and the picture of their rhythm)");
    grooveButton.onClick = [this] { browser.show(); };
    linkButton.setClickingTogglesState (true);
    linkButton.setColour (juce::TextButton::buttonOnColourId, Palette::accent2.darker (0.3f));
    Localisation::bindTip (linkButton, "Link with Fase Groove Piano and Guitar: the same genre, swing, tempo and play button in all of them, on the same beat (change it in any one)");
    linkButton.onClick = [this] { processor.setLinked (linkButton.getToggleState()); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &grooveButton, &prevGroove, &nextGroove, &genreCombo, &newBeatButton, &densitySlider,
                                                                        &partLengthCombo, &linkButton })
        show (*c);
    for (int i = 0; i < fgd::numSegments; ++i)
    {
        partCards[i] = std::make_unique<PartCard> (p, i);
        show (*partCards[i]);
    }

    // ---- Kit & sounds: the drum picked ----
    juce::StringArray laneNames;
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        drumBox.addItem (fgd::laneName (l), l + 1);
        laneNames.add (fgd::laneName (l));
    }
    Localisation::bindItems (drumBox, laneNames);
    Localisation::bindTip (drumBox, "The drum you edit here (or click a pad, or its name on the grid)");
    drumBox.onChange = [this] { pickedFromMenu (drumBox); selectLane (drumBox.getSelectedId() - 1); };
    for (int n = 0; n < 128; ++n)
    {
        if (n == 35) noteBox.addSectionHeading (tr ("General MIDI drums"));
        if (n == 83) noteBox.addSectionHeading (tr ("Other notes"));
        if (n == 0)  noteBox.addSectionHeading (tr ("Low notes"));
        const juce::String gm (fgd::generalMidiDrumName (n));
        noteBox.addItem (juce::String (n) + "  " + (gm.isNotEmpty() ? gm : juce::MidiMessage::getMidiNoteName (n, true, true, 3)), n + 1);
    }
    Localisation::bindTip (noteBox, "The MIDI note this drum plays (to match your drum rack or sampler)");
    noteBox.onChange = [this]
    {
        pickedFromMenu (noteBox);
        if (const int id = noteBox.getSelectedId(); id > 0)
            processor.setParameterFromUI (ParamIDs::note (selectedLane), (float) (id - 1));
    };
    laneDiceButton.iconColour = Palette::accent;
    Localisation::bindTip (laneDiceButton, "A new line for this drum only (locked parts stay)");
    laneDiceButton.onClick = [this] { processor.randomizeLane (selectedLane); };
    muteButton.setClickingTogglesState (false);
    Localisation::bindTip (muteButton, "Mute this drum (MIDI out, guide kit and clips)");
    muteButton.onClick = [this] { processor.setLaneMuted (selectedLane, ! processor.isLaneMuted (selectedLane)); };
    Localisation::bindTip (loadSampleButton, "Your own samples: pick a folder, every sample is recognised as a drum; listen and choose");
    loadSampleButton.onClick = [this] { sampleBrowser.show ({}, selectedLane); };
    Localisation::bindTip (builtInButton, "Back to the built-in sound");
    builtInButton.onClick = [this] { processor.clearSample (selectedLane); };
    levelKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    levelKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 15);
    levelKnob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    Localisation::bindTip (levelKnob, "This drum's LEVEL: its MIDI velocities (and so the clips) go up or down");
    volumeKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    volumeKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 15);
    volumeKnob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    volumeKnob.setDoubleClickReturnValue (true, 0.0);
    Localisation::bindTip (volumeKnob, "This drum's VOLUME in the guide sound (your sample too); the MIDI doesn't change. All of them: MIXER");
    for (juce::Component* c : std::initializer_list<juce::Component*> { &drumBox, &noteBox, &laneDiceButton, &muteButton, &loadSampleButton, &builtInButton,
                                                                        &levelKnob, &volumeKnob })
        show (*c);

    // ---- Pads ----
    pads.onSelectLane = [this] (int lane) { selectLane (lane); };
    show (pads);

    // ---- Groove panel ----
    sectionCombo.setTip ("Song section: Intro / Outro (kick and hats), Break (no kick), Build-up (snare roll), Drop (crash every 8 bars)");
    fillsCombo.setTip ("A fill at the end of every N bars");
    fillStyleCombo.setTip ("What the fill plays. Mix: a different one now and then");
    variationCombo.setTip ("Every 4 or 8 bars the last bar plays a variation (a hat lift, a pick-up...)");
    swing.setTip ("Pushes every other 16th later: 50% straight, 67% triplet shuffle, 75% the heaviest. Each drum's share: MIXER");
    humanize.setTip ("Like a drummer: small timing and velocity changes, and the soft notes come and go over the four bars (the kick stays tight). 0%: exactly as written");
    velocity.setTip ("Overall velocity of the hits");
    accents.setTip ("The accent groove of hats, shaker, ride and tambourine: the offbeat sings, the 16ths are softer (0%: flat)");
    crashButton.setClickingTogglesState (true);
    Localisation::bindTip (crashButton, "A crash on the bar after every fill");
    crashAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::crash, crashButton);
    Localisation::bindTip (newFeelButton, "Another humanization (the same every time you play it)");
    newFeelButton.onClick = [this] { processor.newFeel(); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &sectionCombo, &fillsCombo, &variationCombo, &fillStyleCombo,
                                                                        &swing, &humanize, &velocity, &accents, &crashButton, &newFeelButton })
        show (*c);

    // ---- Guide sound & MIDI out ----
    Localisation::bindTip (soundToggle, "The guide kit: hear the groove here. Off: MIDI only, for your own drums");
    soundAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sound, soundToggle);
    channelCombo.setTip ("The MIDI channel of the notes (10 is the drum channel of General MIDI)");
    Localisation::bindTip (kitFolderButton, "Your own samples: pick a folder, every sample is recognised as a drum; listen and choose");
    kitFolderButton.onClick = [this] { showKitMenu(); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &soundToggle, &kitCombo, &channelCombo, &kitFolderButton, &tone, &reverb, &volume })
        show (*c);

    // ---- Edit: step grid / velocity ----
    gridTab.setConnectedEdges (juce::Button::ConnectedOnRight);
    velocityTab.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    mixerTab.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    fillTab.setConnectedEdges (juce::Button::ConnectedOnLeft);
    Localisation::bindTip (mixerTab, "Every drum: volume, mute, push / pull and its share of the swing");
    Localisation::bindTip (fillTab, "Draw your own fill (Fill style: Custom)");
    fillTab.onClick = [this] { setView (3); };
    Localisation::bindTip (templateButton, "Groove template: the timing and accents of an audio loop or a MIDI file, on any groove");
    templateButton.onClick = [this] { showTemplateMenu(); };
    Localisation::bindTip (timingButton, "Ready-made feels: which drums sit ahead of or behind the beat, which ones swing");
    timingButton.onClick = [this]
    {
        juce::PopupMenu menu;
        menu.addSectionHeader (tr ("Timing of the drums (the kick stays on the grid)"));
        const auto names = FaseGrooveDrumsProcessor::timingFeelNames();
        for (int i = 0; i < names.size(); ++i)
            menu.addItem (i + 1, tr (names[i]));
        juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&timingButton), [safe] (int result)
        {
            if (safe != nullptr && result > 0)
                safe->processor.applyTimingFeel (result - 1);
        });
    };
    mixerTab.onClick = [this] { setView (2); };
    gridTab.onClick = [this] { setView (0); };
    Localisation::bindTip (velocityTab, "The velocity of every hit, one drum at a time (what makes hats groove)");
    velocityTab.onClick = [this] { setView (1); };
    Localisation::bindText (newRhythmButton, "BEAT");
    newRhythmButton.iconColour = Palette::accent;
    Localisation::bindTip (newRhythmButton, "A brand new random beat (locked parts stay)");
    newRhythmButton.onClick = [this] { processor.rollNewBeat(); };
    Localisation::bindTip (myGroovesButton, "Save the grid as your own groove, or load one");
    myGroovesButton.onClick = [this] { showGroovesMenu(); };
    Localisation::bindText (importButton, "IMPORT...");
    Localisation::bindTip (importButton, "A MIDI file's drums, or an audio drum loop, into the grid (or drop the file on the window)");
    importButton.onClick = [this]
    {
        juce::PopupMenu menu;
        menu.addItem (1, tr ("Import the drums of a MIDI file..."));
        menu.addItem (2, tr ("Import an audio drum loop (WAV, AIFF...)..."));
        juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (menuTarget (importButton, mainImport)), [safe] (int result)
        {
            if (safe == nullptr) return;
            if (result == 1) safe->importMidi();
            if (result == 2) safe->importAudio();
        });
    };
    Localisation::bindTip (clearButton, "Empty the grid and start your own beat");
    clearButton.onClick = [this] { processor.clearGrid(); grid.refresh(); velocityView.refresh(); };
    grid.onShowVelocity = [this] (int lane) { selectLane (lane); setView (1); };
    grid.onSelectLane = [this] (int lane) { selectLane (lane); };
    grid.onBrowseSamples = [this] (int lane) { selectLane (lane); sampleBrowser.show ({}, lane); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &gridTab, &velocityTab, &mixerTab, &fillTab, &newRhythmButton, &myGroovesButton,
                                                                        &importButton, &templateButton, &clearButton, &grid })
        editOverlay.addAndMakeVisible (*c);
    editOverlay.addChildComponent (velocityView);
    editOverlay.addChildComponent (mixerView);
    editOverlay.addChildComponent (fillView);
    editOverlay.addChildComponent (timingButton);

    // The main window: the groove at a glance (a click opens the editor) and the volume faders.
    overview.onOpen = [this] { openEditor(); };
    Localisation::bind (overview, [this] { overview.setTooltip (tr ("Click: open the editor")); });
    Localisation::bind (openEditorButton, [this]
    {
        openEditorButton.setButtonText (tr ("EDITOR..."));
        openEditorButton.setTooltip (tr ("The big editor: step grid, velocity, mixer and fill, to edit everything (or click the groove)"));
    });
    openEditorButton.onClick = [this] { openEditor(); };
    show (overview);
    content.addChildComponent (miniMixer);
    show (openEditorButton);
    mainGrooveTab.setConnectedEdges (juce::Button::ConnectedOnRight);
    mainVolumeTab.setConnectedEdges (juce::Button::ConnectedOnLeft);
    mainGrooveTab.onClick = [this] { setMainTab (false); };
    mainVolumeTab.onClick = [this] { setMainTab (true); };
    mainNewRhythm.iconColour = Palette::accent;
    mainNewRhythm.onClick = [this] { if (newRhythmButton.onClick) newRhythmButton.onClick(); };
    mainMyGrooves.onClick = [this] { if (myGroovesButton.onClick) myGroovesButton.onClick(); };
    mainImport.onClick    = [this] { if (importButton.onClick) importButton.onClick(); };
    mainTemplate.onClick  = [this] { if (templateButton.onClick) templateButton.onClick(); };
    mainClear.onClick     = [this] { if (clearButton.onClick) clearButton.onClick(); };
    Localisation::bind (mainGrooveTab, [this]
    {
        mainGrooveTab.setButtonText (tr ("GROOVE"));
        mainVolumeTab.setButtonText (tr ("VOLUMES"));
        mainVolumeTab.setTooltip (tr ("The volume of each one in the guide sound"));
        mainNewRhythm.setButtonText (tr ("BEAT"));
        mainNewRhythm.setTooltip (newRhythmButton.getTooltip());
        mainMyGrooves.setButtonText (tr ("MY GROOVES"));
        mainMyGrooves.setTooltip (myGroovesButton.getTooltip());
        mainImport.setButtonText (tr ("IMPORT..."));
        mainImport.setTooltip (importButton.getTooltip());
        mainTemplate.setButtonText (tr ("TEMPLATE..."));
        mainTemplate.setTooltip (templateButton.getTooltip());
        mainClear.setButtonText (tr ("CLEAR"));
        mainClear.setTooltip (clearButton.getTooltip());
    });
    for (juce::Component* m : std::initializer_list<juce::Component*> { &mainGrooveTab, &mainVolumeTab, &mainNewRhythm, &mainMyGrooves, &mainImport,
                                                                        &mainTemplate, &mainClear })
        show (*m);
    setMainTab (false);
    editOverlay.viewMode = [this] { return viewMode; };
    editOverlay.onBrush = [this] (char b) { grid.brush = b; fillView.brush = b; };
    editOverlay.rewindButton.onClick = [this] { processor.previewToStart(); };
    editOverlay.playButton.onClick = [this] { processor.previewToggle(); };
    editOverlay.stopButton.onClick = [this] { processor.previewStop(); };

    // ---- Export MIDI ----
    notesBox.addItem ("All drums", 1);
    notesBox.addItem ("One track per drum", 2);
    Localisation::bindItems (notesBox, { "All drums", "One track per drum" });
    notesBox.setSelectedId (1, juce::dontSendNotification);
    Localisation::bindTip (notesBox, "One drum track, or one MIDI track per drum (kick, snare, hats...)");
    const char* lengths[] { "Full loop", "1 bar", "2 bars", "4 bars", "8 bars", "16 bars", "Whole song" };
    juce::StringArray lengthNames;
    for (int i = 0; i < (int) std::size (lengths); ++i)
    {
        lengthBox.addItem (lengths[i], i + 1);
        lengthNames.add (lengths[i]);
    }
    Localisation::bindItems (lengthBox, lengthNames);
    lengthBox.setSelectedId (1, juce::dontSendNotification);
    Localisation::bindTip (lengthBox, "Length of the clip. Full loop: until groove, fills and variations come round. "
                                      "Whole song: the sections of the song window, with markers");
    dragTile.filled = true;   // the orange tile of Fase Groove Piano, at the same size
    dragTile.createFiles = [this]
    {
        const auto file = processor.writeMidiClip (selectedClipBars(), selectedClipPart());
        return file.existsAsFile() ? juce::StringArray (file.getFullPathName()) : juce::StringArray();
    };
    Localisation::bindTip (saveButton, "Save the clip as a .mid file");
    saveButton.onClick = [this] { saveMidiFile(); };
    Localisation::bindTip (songButton, "The song window: bars, variation and fill of each section; drag or save the whole song or each section");
    songButton.onClick = [this] { songOverlay.show(); };
    Localisation::bindTip (wavButton, "The guide sound as audio (WAV): this clip, the whole song, or one file per drum");
    wavButton.onClick = [this] { showAudioExportMenu(); };
    for (juce::Component* c : std::initializer_list<juce::Component*> { &notesBox, &lengthBox, &dragTile, &saveButton, &wavButton, &songButton })
        show (*c);

    content.addChildComponent (editOverlay);
    content.addChildComponent (songOverlay);
    content.addChildComponent (browser);
    content.addChildComponent (sampleBrowser);
    sampleBrowser.onMessage = [this] (const juce::String& text) { showMessage (text); };
    content.addChildComponent (options);
    content.addChildComponent (updateOverlay);

    // OPTIONS, as in Fase Groove Piano.
    options.onScaleChosen = [this] (float newScale)
    {
        setSize (juce::roundToInt (designWidth * newScale), juce::roundToInt (designHeight * newScale));
    };
    options.onTipsChanged = [this] (bool on)
    {
        Localisation::setTipsEnabled (on);
        applyTips();
    };
    applyTips();

    // Automatic updates, as in Fase Groove Piano.
    options.onCheckUpdates = [this]
    {
        options.checkUpdatesButton.setEnabled (false);
        options.checkUpdatesButton.setButtonText (tr ("CHECKING..."));
        updateChecker.start (true);
    };
    updateChecker.onResult = [this] (const Updater::Result& result, bool manual)
    {
        options.checkUpdatesButton.setEnabled (true);
        options.refreshText();
        if (result.status == Updater::Result::Status::ready)
        {
            options.setVisible (false);
            updateOverlay.show (result);
        }
        else if (manual)
        {
            const bool upToDate = result.status == Updater::Result::Status::upToDate;
            juce::AlertWindow::showMessageBoxAsync (upToDate ? juce::MessageBoxIconType::InfoIcon : juce::MessageBoxIconType::WarningIcon,
                                                    "Fase Groove Drums",
                                                    upToDate ? tr ("You have the latest version.") + " (" + juce::String (JucePlugin_VersionString) + ")"
                                                             : tr ("Could not check for updates. Check your internet connection and try again."));
        }
    };
    Updater::cleanupAfterUpdate();
    if (Updater::autoCheckEnabled() && Updater::firstAutoCheckThisSession())
        updateChecker.start (false);

    combos = { &partLengthCombo, &genreCombo, &sectionCombo, &fillsCombo, &variationCombo, &fillStyleCombo, &kitCombo, &channelCombo };

    lastSnapshot = processor.snapshot();
    // Texts that follow the interface language.
    for (auto [b, english] : std::initializer_list<std::pair<juce::Button*, const char*>> {
             { &presetsButton, "PRESETS" }, { &styleButton, "STYLE" }, { &resetButton, "RESET" }, { &syncButton, "SYNC DAW" },
             { &sequencerToggle, "Sequencer" }, { &soundToggle, "Guide sound" }, { &loadSampleButton, "YOUR SAMPLES..." },
             { &builtInButton, "BUILT-IN" }, { &muteButton, "MUTE" }, { &crashButton, "CRASH" }, { &newFeelButton, "NEW FEEL" },
             { &gridTab, "STEP GRID" }, { &velocityTab, "VELOCITY" }, { &mixerTab, "MIXER" }, { &clearButton, "CLEAR" },
             { &myGroovesButton, "MY GROOVES" }, { &saveButton, "SAVE .MID..." }, { &songButton, "FULL SONG..." },
             { &fillTab, "FILL" }, { &templateButton, "TEMPLATE..." }, { &timingButton, "TIMING..." }, { &wavButton, "SAVE .WAV..." },
             { &kitFolderButton, "YOUR SAMPLES..." }, { &linkButton, "LINK" } })
        Localisation::bindText (*b, english);
    Localisation::bind (content, [this] { content.repaint(); options.refreshText(); translateHostWindow(); });

    selectLane ((int) fgd::Lane::closedHat);
    setView (0);
    timerCallback();   // the groove's name, the parts and the lights from the start

    // The first time: as big as the screen comfortably allows (bigger buttons and text).
    float scale = processor.editorScale;
    if (scale <= 0.0f)
    {
        scale = 1.0f;
        if (const auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
            scale = juce::jmin ((float) display->userArea.getWidth() * 0.9f / (float) designWidth,
                                (float) display->userArea.getHeight() * 0.88f / (float) designHeight);
    }
    scale = juce::jlimit (0.6f, 1.25f, scale);
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

float FaseGrooveDrumsEditor::currentScale() const
{
    return juce::jmax (0.3f, juce::jmin ((float) getWidth() / (float) designWidth, (float) getHeight() / (float) designHeight));
}

void FaseGrooveDrumsEditor::applyTips()
{
    // Tips appear when the mouse rests on a control; they can be switched off in OPTIONS.
    if (Localisation::tipsEnabled())
    {
        if (tooltips == nullptr)
            tooltips = std::make_unique<juce::TooltipWindow> (this, 600);
    }
    else
    {
        tooltips.reset();
    }
}

void FaseGrooveDrumsEditor::parentHierarchyChanged()
{
    translateHostWindow();
}

void FaseGrooveDrumsEditor::translateHostWindow()
{
    // The Standalone app's window (Options button, feedback warning) has a few fixed English
    // texts; its menus and the audio settings follow the language through JUCE's TRANS().
    if (processor.wrapperType != juce::AudioProcessor::wrapperType_Standalone)
        return;

    auto* top = getTopLevelComponent();
    if (top == nullptr || top == this)
        return;

    static const char* const fixedTexts[] { "Options", "Mute audio input", "Feedback Loop:", "Audio input is muted to avoid feedback loop" };

    std::function<void (juce::Component&)> visit = [&] (juce::Component& c)
    {
        if (&c == this)
            return;

        auto& props = c.getProperties();
        auto translate = [&] (const juce::String& current, auto&& apply)
        {
            if (! props.contains ("fgdEnglish"))
                for (auto* t : fixedTexts)
                    if (current == t)
                        props.set ("fgdEnglish", t);

            if (props.contains ("fgdEnglish"))
                apply (tr (props["fgdEnglish"].toString()));
        };

        if (auto* b = dynamic_cast<juce::Button*> (&c))
            translate (b->getButtonText(), [b] (const juce::String& t) { b->setButtonText (t); });
        else if (auto* l = dynamic_cast<juce::Label*> (&c))
            translate (l->getText(), [l] (const juce::String& t) { l->setText (t, juce::dontSendNotification); });

        for (auto* child : c.getChildren())
            visit (*child);
    };

    visit (*top);
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
        editor.options.show (true, editor.currentScale());
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
        editor.options.setVisible (false);
        editor.songOverlay.setVisible (false);
        editor.browser.setVisible (false);
        editor.editOverlay.setVisible (false);
        return true;
    }
    return false;
}

void FaseGrooveDrumsEditor::layout()
{
    // Header: logo | presets, total, undo / redo, full screen / style, reset, options | transport & tempo | display
    // The same places and sizes as in Fase Groove Piano.
    logoArea = { 20, 18, 238, 54 };
    presetsButton.setBounds (282, 22, 46, 26);
    totalRandomButton.setBounds (332, 22, 84, 26);
    undoButton.setBounds (422, 22, 30, 26);
    redoButton.setBounds (452, 22, 30, 26);
    fullScreenButton.setBounds (486, 22, 30, 26);
    styleButton.setBounds (282, 60, 66, 26);
    resetButton.setBounds (352, 60, 60, 26);
    optionsButton.setBounds (416, 60, 100, 26);

    tempoArea = { 520, 18, 330, 68 };
    sequencerToggle.setBounds (734, 24, 104, 18);
    linkButton.setBounds (664, 24, 64, 18);
    rewindButton.setBounds (532, 48, 34, 30);
    playButton.setBounds (566, 48, 40, 30);
    stopButton.setBounds (606, 48, 34, 30);
    syncButton.setBounds (652, 48, 86, 30);
    tempoSlider.setBounds (744, 48, 94, 30);
    dawTempo.setBounds (tempoSlider.getBounds());
    displayArea = { 860, 18, 300, 68 };

    // Groove row (20, 100, 1142, 120)
    grooveButton.setBounds (34, 135, 204, 30);
    prevGroove.setBounds (240, 135, 30, 30);
    nextGroove.setBounds (270, 135, 30, 30);
    genreCombo.setBounds (34, 170, 150, 44);
    newBeatButton.setBounds (190, 184, 110, 30);
    densitySlider.setBounds (310, 184, 104, 30);
    partLengthCombo.setBounds (310, 120, 104, 44);
    for (int i = 0; i < fgd::numSegments; ++i)
        partCards[i]->setBounds (430 + i * 182, 118, 174, 96);

    // Kit & sounds (20, 230, 290, 186)
    drumBox.setBounds (34, 268, 164, 28);
    laneDiceButton.setBounds (202, 268, 34, 28);
    muteButton.setBounds (240, 268, 56, 28);
    noteBox.setBounds (34, 318, 146, 26);
    levelKnob.setBounds (184, 314, 56, 58);
    volumeKnob.setBounds (242, 314, 58, 58);
    loadSampleButton.setBounds (34, 382, 166, 26);
    builtInButton.setBounds (204, 382, 92, 26);

    // Pads (320, 230, 260, 186)
    pads.setBounds (326, 254, 248, 156);

    // Groove panel (590, 230, 300, 186)
    newFeelButton.setBounds (730, 234, 84, 24);
    crashButton.setBounds (818, 234, 62, 24);
    sectionCombo.setBounds (604, 256, 134, 40);
    fillsCombo.setBounds (744, 256, 134, 40);
    variationCombo.setBounds (604, 298, 134, 40);
    fillStyleCombo.setBounds (744, 298, 134, 40);
    swing.setBounds (600, 340, 70, 72);
    humanize.setBounds (670, 340, 70, 72);
    velocity.setBounds (740, 340, 70, 72);
    accents.setBounds (810, 340, 70, 72);

    // Guide sound & MIDI out (900, 230, 262, 186)
    soundToggle.setBounds (914, 272, 116, 24);
    kitCombo.setBounds (1034, 256, 116, 40);
    channelCombo.setBounds (914, 298, 100, 40);
    kitFolderButton.setBounds (1034, 304, 116, 28);
    tone.setBounds (914, 340, 76, 72);
    reverb.setBounds (994, 340, 76, 72);
    volume.setBounds (1074, 340, 76, 72);

    // Edit (20, 426, 830, 424): the biggest part of the window
    // The groove at a glance and the faders (20, 426, 830, 276); the editor over the window.
    openEditorButton.setBounds (34, 450, 112, 30);
    mainGrooveTab.setBounds (156, 450, 84, 30);
    mainVolumeTab.setBounds (240, 450, 84, 30);
    mainNewRhythm.setBounds (338, 450, 86, 30);
    mainMyGrooves.setBounds (432, 450, 100, 30);
    mainImport.setBounds (536, 450, 86, 30);
    mainTemplate.setBounds (626, 450, 96, 30);
    mainClear.setBounds (762, 450, 76, 30);
    overview.setBounds (30, 488, 810, 206);
    miniMixer.setBounds (30, 490, 810, 204);

    editOverlay.setBounds (content.getLocalBounds());
    editOverlay.closeButton.setBounds (1134, 22, 28, 28);
    editOverlay.rewindButton.setBounds (28, 74, 38, 30);
    editOverlay.playButton.setBounds (66, 74, 44, 30);
    editOverlay.stopButton.setBounds (110, 74, 38, 30);
    editOverlay.accentBrush.setBounds (174, 74, 74, 30);
    editOverlay.hitBrush.setBounds (248, 74, 60, 30);
    editOverlay.ghostBrush.setBounds (308, 74, 70, 30);
    editOverlay.flamBrush.setBounds (378, 74, 70, 30);
    gridTab.setBounds (476, 74, 92, 30);
    velocityTab.setBounds (568, 74, 84, 30);
    mixerTab.setBounds (652, 74, 64, 30);
    fillTab.setBounds (716, 74, 54, 30);
    newRhythmButton.setBounds (784, 74, 82, 30);
    myGroovesButton.setBounds (874, 74, 96, 30);
    importButton.setBounds (970, 74, 84, 30);
    templateButton.setBounds (1062, 74, 96, 30);
    timingButton.setBounds (1062, 74, 96, 30);
    clearButton.setBounds (1068, 22, 60, 28);
    grid.setBounds (24, 116, 1132, 576);
    velocityView.setBounds (24, 116, 1132, 576);
    mixerView.setBounds (24, 116, 1132, 576);
    fillView.setBounds (24, 116, 1132, 576);

    // Export MIDI & audio (860, 426, 302, 424)
    notesBox.setBounds (874, 472, 134, 28);
    lengthBox.setBounds (1012, 472, 134, 28);
    dragTile.setBounds (874, 512, 272, 114);
    saveButton.setBounds (874, 636, 88, 30);
    wavButton.setBounds (966, 636, 88, 30);
    songButton.setBounds (1058, 636, 88, 30);

    songOverlay.setBounds (content.getLocalBounds());
    browser.setBounds (content.getLocalBounds());
    sampleBrowser.setBounds (content.getLocalBounds());
    options.setBounds (content.getLocalBounds());
    updateOverlay.setBounds (content.getLocalBounds());
}

void FaseGrooveDrumsEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (Palette::background);

    // ---- Header ----
    Branding::drawFullLogo (g, logoArea.toFloat().withHeight (46.0f));
    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.drawText (tr ("by") + " " + juce::String (Branding::artist), logoArea.getX() + 58, logoArea.getBottom() + 2, 200, 14, juce::Justification::centredLeft);

    auto box = [&g] (juce::Rectangle<int> r)
    {
        g.setColour (Palette::panel);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (Palette::panelEdge);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 8.0f, 1.0f);
    };
    drawPanel (g, tempoArea, {});
    g.setColour (Palette::accent);
    g.fillRoundedRectangle ((float) tempoArea.getX() + 12.0f, (float) tempoArea.getY() + 9.0f, 3.0f, 12.0f, 1.0f);
    g.setColour (Palette::text.withAlpha (0.92f));
    g.setFont (font (11.5f, true));
    g.drawFittedText (Localisation::upper (tr ("TRANSPORT & TEMPO")), { tempoArea.getX() + 21, tempoArea.getY() + 5, 122, 20 }, juce::Justification::centredLeft, 1, 0.7f);

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
        g.drawText (Localisation::upper (tr (section)), top.removeFromRight (90), juce::Justification::topRight);
        g.setColour (Palette::text);
        g.setFont (font (22.0f, true));
        g.drawFittedText (settings.pattern == fgd::userPatternIndex() ? tr ("User pattern") : settings.pattern == fgd::randomPatternIndex() ? tr ("Random beat")
                                                                                                                       : juce::String (base.name),
                          top, juce::Justification::centredLeft, 1, 0.6f);

        const double ppq = processor.uiPpq.load();
        const auto where = ppq >= 0.0 ? tr ("Bar") + " " + juce::String ((int) std::floor (ppq / 4.0) + 1) + "  " + tr ("Beat") + " " + juce::String ((int) std::floor (std::fmod (ppq, 4.0)) + 1)
                                      : tr ("Stopped");
        const auto genre = settings.pattern == fgd::userPatternIndex() ? tr ("Your pattern")
                         : base.category.empty() ? juce::String (fgd::grooveCategories()[(size_t) juce::jlimit (0, 12, settings.randomGenre)])
                                                 : juce::String (base.category);
        g.setColour (Palette::dim);
        g.setFont (font (12.0f));
        g.drawText (genre, r, juce::Justification::centredLeft);
        g.drawText (where + "   " + juce::String (processor.lastBpm.load(), 1) + " BPM", r, juce::Justification::centredRight);
    }

    // ---- Panels ----
    drawPanel (g, { 20, 100, 1140, 120 }, "Groove");
    drawPanel (g, { 20, 230, 290, 186 }, "Kit & sounds");
    drawPanel (g, { 320, 230, 260, 186 }, "Drum pads");
    drawPanel (g, { 590, 230, 300, 186 }, "Groove feel & song");
    drawPanel (g, { 900, 230, 260, 186 }, "Guide sound & MIDI out");
    drawPanel (g, { 20, 426, 830, 276 }, {});
    drawPanel (g, { 860, 426, 300, 276 }, "Export MIDI & audio");

    auto caption = [&g] (const juce::String& text, int x, int y, int w = 160)
    {
        g.setColour (Palette::dim);
        g.setFont (font (11.0f, true));
        g.drawFittedText (Localisation::upper (tr (text)), { x, y, w, 15 }, juce::Justification::centredLeft, 1, 0.7f);
    };
    caption ("Groove (click: browser)", 34, 119, 214);
    caption ("Density", 310, 169);
    caption (processor.steps() > 32 ? "Parts A-D (one bar each: play them in any order)" : "Parts A-D (each half a bar: play them in any order)", 430, 104, 400);
    caption ("Drum", 34, 252);
    caption ("MIDI note", 34, 302);
    caption ("Level", 186, 300, 54);
    caption ("Volume", 242, 300, 58);
    caption ("Edit", 34, 434, 112);
    caption ("View", 156, 434);
    caption ("New rhythm", 338, 434);
    caption ("Groove", 432, 434);
    caption ("Template", 626, 434, 96);
    caption ("Notes", 874, 456);
    caption ("Clip length", 1012, 456);

    // The picked drum's sound.
    {
        const auto path = processor.samplePath (selectedLane);
        g.setColour (path.isNotEmpty() ? laneColour (selectedLane) : Palette::text.withAlpha (0.8f));
        g.setFont (font (13.0f, true));
        g.drawFittedText (path.isNotEmpty() ? juce::File (path).getFileName()
                                            : tr ("Built-in") + ": " + juce::String (GuideDrums::kitName (juce::roundToInt (processor.raw (ParamIDs::kit)))),
                          juce::Rectangle<int> (34, 348, 146, 16), juce::Justification::centredLeft, 1, 0.6f);
        g.setColour (Palette::dim);
        g.setFont (font (10.5f));
        g.drawFittedText (tr ("or drop a WAV / AIFF on its row or pad"), { 34, 364, 146, 14 }, juce::Justification::centredLeft, 1, 0.5f);
    }

    g.setColour (Palette::dim);
    g.setFont (font (10.5f));
    g.drawFittedText (tr ("Drop it on a drum track (your own kit / drum rack)"), { 874, 672, 272, 16 }, juce::Justification::centredLeft, 1, 0.7f);

    // A short message (a saved preset, an imported file...)
    if (messageTicks > 0)
    {
        auto r = juce::Rectangle<float> (0.0f, 0.0f, 440.0f, 34.0f).withCentre ({ designWidth * 0.5f, 560.0f });
        g.setColour (Palette::accent);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (Palette::background);
        g.setFont (font (13.0f, true));
        g.drawFittedText (message, r.reduced (10.0f, 0.0f).toNearestInt(), juce::Justification::centred, 1, 0.8f);
    }
}

void FaseGrooveDrumsEditor::setMainTab (bool volumes)
{
    showVolumes = volumes;
    mainGrooveTab.setToggleState (! volumes, juce::dontSendNotification);
    mainVolumeTab.setToggleState (volumes, juce::dontSendNotification);
    overview.setVisible (! volumes);
    miniMixer.setVisible (volumes);
}

void FaseGrooveDrumsEditor::openEditor (int view)
{
    if (view >= 0)
        setView (view);
    editOverlay.setVisible (true);
    editOverlay.toFront (false);
    grid.refresh();
}

void FaseGrooveDrumsEditor::setView (int mode)
{
    viewMode = juce::jlimit (0, 3, mode);
    gridTab.setToggleState (viewMode == 0, juce::dontSendNotification);
    velocityTab.setToggleState (viewMode == 1, juce::dontSendNotification);
    mixerTab.setToggleState (viewMode == 2, juce::dontSendNotification);
    fillTab.setToggleState (viewMode == 3, juce::dontSendNotification);
    grid.setVisible (viewMode == 0);
    velocityView.setVisible (viewMode == 1);
    mixerView.setVisible (viewMode == 2);
    fillView.setVisible (viewMode == 3);
    timingButton.setVisible (viewMode == 2);
    templateButton.setVisible (viewMode != 2);
    velocityView.setLane (selectedLane);
    content.repaint();
    editOverlay.repaint();
}

void FaseGrooveDrumsEditor::selectLane (int lane)
{
    selectedLane = juce::jlimit (0, fgd::numLanes - 1, lane);
    levelAttachment.reset();
    levelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::level (selectedLane), levelKnob);
    volumeAttachment.reset();
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::laneVolume (selectedLane), volumeKnob);
    volumeKnob.setColour (juce::Slider::textBoxTextColourId, Palette::text);
    levelKnob.setColour (juce::Slider::textBoxTextColourId, Palette::text);
    if (velocityView.getLane() != selectedLane)
        velocityView.setLane (selectedLane);
    refreshKitPanel();
    content.repaint();
}

void FaseGrooveDrumsEditor::refreshKitPanel()
{
    showInMenu (drumBox, selectedLane + 1);
    showInMenu (noteBox, processor.laneNote (selectedLane) + 1);
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
    processor.selectGroove (order[(size_t) index]);
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

    overview.refresh();
    miniMixer.repaint();
    const bool playingNow = processor.previewOn.load();
    editOverlay.playButton.setIcon (playingNow ? IconButton::Icon::pause : IconButton::Icon::play);
    editOverlay.playButton.setToggleState (playingNow, juce::dontSendNotification);
    if (editOverlay.isVisible())   // the views only while the editor is open
    {
        if (viewMode == 1)
            velocityView.refresh();
        else if (viewMode == 2)
            mixerView.setLights (lights);
        else if (viewMode == 3)
            fillView.repaint();
        else
            grid.refresh();
    }
    linkButton.setToggleState (processor.isLinked(), juce::dontSendNotification);
    templateButton.setToggleState (processor.hasGrooveTemplate(), juce::dontSendNotification);
    mainTemplate.setToggleState (processor.hasGrooveTemplate(), juce::dontSendNotification);
    mainTemplate.setColour (juce::TextButton::buttonOnColourId, Palette::accent.darker (0.35f));
    templateButton.setColour (juce::TextButton::buttonOnColourId, Palette::accent.darker (0.35f));

    const auto shown = processor.displayedPattern();
    const int step = processor.uiStep.load();
    for (auto& card : partCards)
        card->refresh (shown, step);
    refreshKitPanel();

    // The groove's name on its button.
    {
        const auto settings = processor.currentSettings();
        const auto name = settings.pattern == fgd::userPatternIndex() ? tr ("User pattern")
                        : settings.pattern == fgd::randomPatternIndex() ? tr ("Random beat")
                                                                      : juce::String (fgd::GrooveEngine::basePattern (settings).name);
        if (grooveButton.getButtonText() != name)
            grooveButton.setButtonText (name);
        if (browser.isVisible())
            browser.repaint();
    }

    const bool playing = processor.previewOn.load();
    playButton.setIcon (playing ? IconButton::Icon::pause : IconButton::Icon::play);
    playButton.setToggleState (playing || step >= 0, juce::dontSendNotification);
    {
        // As in Fase Groove Piano: while synced, the DAW's tempo in place of the slider.
        const bool standalone = processor.wrapperType == juce::AudioProcessor::wrapperType_Standalone;
        const bool synced = processor.raw (ParamIDs::tempoSync) >= 0.5f && ! standalone;
        tempoSlider.setVisible (! synced);
        dawTempo.setVisible (synced);
        if (synced)
            dawTempo.setText (juce::String (processor.hostBpm.load(), 1) + " BPM  " + tr ("(DAW)"), juce::dontSendNotification);
    }
    fullScreenButton.setIcon (isFullScreen() ? IconButton::Icon::exitFullScreen : IconButton::Icon::fullScreen);
    dragTile.setSubtitle (notesBox.getText() + juce::String::fromUTF8 ("  \xc2\xb7  ") + lengthBox.getText());
    fullScreenButton.setTooltip (tr (isFullScreen() ? "Exit full screen" : "Full screen"));

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
    content.repaint (20, 340, 290, 50);
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
    auto* window = new juce::AlertWindow (tr (title), tr ("Name") + ":", juce::MessageBoxIconType::NoIcon, this);
    window->addTextEditor ("name", tr (suggestion));
    window->addButton (tr ("SAVE"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton (tr ("CANCEL"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    window->enterModalState (true, juce::ModalCallbackFunction::create ([window, onName] (int result)
    {
        if (result == 1 && onName != nullptr)
            onName (window->getTextEditorContents ("name").trim());
    }), true);
}

void FaseGrooveDrumsEditor::showPresetsMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, tr ("Save preset..."));
    const auto files = FaseGrooveDrumsProcessor::presetFiles();
    if (! files.isEmpty())
    {
        menu.addSectionHeader (tr ("Your presets"));
        for (int i = 0; i < files.size(); ++i)
            menu.addItem (100 + i, files[i].getFileNameWithoutExtension());
    }
    menu.addSeparator();
    menu.addItem (2, tr ("Open the presets folder"));

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
                    safe->showMessage (safe->processor.savePreset (name) ? tr ("Preset saved") + ": " + name : tr ("Could not save the preset"));
            });
        else if (result == 2)
        {
            FaseGrooveDrumsProcessor::presetFolder().createDirectory();
            FaseGrooveDrumsProcessor::presetFolder().startAsProcess();
        }
        else if (juce::isPositiveAndBelow (result - 100, files.size()))
            safe->showMessage (p.loadPreset (files[result - 100]) ? tr ("Preset") + ": " + files[result - 100].getFileNameWithoutExtension()
                                                                  : tr ("Could not read the preset"));
    });
}

void FaseGrooveDrumsEditor::showGroovesMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, tr ("Save this groove..."));
    menu.addItem (3, tr ("Import the drums of a MIDI file..."));
    const auto files = FaseGrooveDrumsProcessor::grooveFiles();
    if (! files.isEmpty())
    {
        menu.addSectionHeader (tr ("My grooves"));
        for (int i = 0; i < files.size(); ++i)
            menu.addItem (100 + i, files[i].getFileNameWithoutExtension());
    }
    menu.addSeparator();
    menu.addItem (2, tr ("Open the grooves folder"));

    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (menuTarget (myGroovesButton, mainMyGrooves)), [safe, files] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        if (result == 1)
            safe->askName ("Save groove", "My groove", [safe] (const juce::String& name)
            {
                if (safe != nullptr && name.isNotEmpty())
                    safe->showMessage (safe->processor.saveGroove (name) ? tr ("Groove saved") + ": " + name : tr ("Could not save the groove"));
            });
        else if (result == 2)
        {
            FaseGrooveDrumsProcessor::grooveFolder().createDirectory();
            FaseGrooveDrumsProcessor::grooveFolder().startAsProcess();
        }
        else if (result == 3)
            safe->importMidi();
        else if (juce::isPositiveAndBelow (result - 100, files.size()))
            safe->showMessage (safe->processor.loadGroove (files[result - 100]) ? tr ("Groove") + ": " + files[result - 100].getFileNameWithoutExtension()
                                                                                 : tr ("Could not read the groove"));
    });
}

void FaseGrooveDrumsEditor::importMidi()
{
    chooser = std::make_unique<juce::FileChooser> (tr ("Import the drums of a MIDI file"), juce::File(), "*.mid;*.midi");
    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe] (const juce::FileChooser& fc)
    {
        if (safe != nullptr && fc.getResult().existsAsFile())
            safe->importFile (fc.getResult());
    });
}

void FaseGrooveDrumsEditor::importAudio()
{
    chooser = std::make_unique<juce::FileChooser> (tr ("Import an audio drum loop"), juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
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
        showMessage (report.isNotEmpty() ? file.getFileNameWithoutExtension() + ": " + report : tr ("No drums found in") + " " + file.getFileName());
    }
    else
        showMessage (processor.importMidiGroove (file) ? tr ("Drums imported") + ": " + file.getFileNameWithoutExtension()
                                                       : tr ("No drums found in") + " " + file.getFileName());
    setView (0);
}

void FaseGrooveDrumsEditor::showTemplateMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader (tr ("Groove template: timing and accents of every 16th"));
    menu.addItem (1, tr ("From an audio loop (WAV, AIFF...)..."));
    menu.addItem (2, tr ("From a MIDI file..."));
    if (processor.grooveTemplateName().isNotEmpty())
    {
        menu.addSeparator();
        menu.addItem (3, tr ("Template on") + ": " + processor.grooveTemplateName(), true, processor.hasGrooveTemplate());
        menu.addItem (4, tr ("Remove the template"));
    }
    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (menuTarget (templateButton, mainTemplate)), [safe] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        auto& p = safe->processor;
        if (result == 3) { p.setGrooveTemplateActive (! p.hasGrooveTemplate()); return; }
        if (result == 4) { p.clearGrooveTemplate(); return; }
        const bool audio = result == 1;
        safe->chooser = std::make_unique<juce::FileChooser> (tr ("Groove template"), juce::File(), audio ? "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg" : "*.mid;*.midi");
        safe->chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [safe, audio] (const juce::FileChooser& fc)
        {
            if (safe == nullptr || ! fc.getResult().existsAsFile())
                return;
            const auto report = audio ? safe->processor.grooveTemplateFromAudio (fc.getResult()) : safe->processor.grooveTemplateFromMidi (fc.getResult());
            safe->showMessage (report.isNotEmpty() ? tr ("Groove template") + ": " + report : tr ("No groove found in") + " " + fc.getResult().getFileName());
        });
    });
}

void FaseGrooveDrumsEditor::showKitMenu()
{
    sampleBrowser.show ({}, selectedLane);
}

void FaseGrooveDrumsEditor::loadKitFolder (const juce::File& folder)
{
    const auto loaded = processor.loadKitFolder (folder);
    juce::StringArray names;
    for (const auto& n : loaded)
        names.add (tr (n));
    showMessage (loaded.isEmpty() ? tr ("No drum samples found in") + " " + folder.getFileName()
                                  : folder.getFileName() + ": " + names.joinIntoString (", "));
}

void FaseGrooveDrumsEditor::showAudioExportMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader (tr ("The guide sound as audio (WAV)"));
    menu.addItem (1, tr ("This clip (loops seamlessly)..."));
    menu.addItem (2, tr ("The whole song..."));
    menu.addItem (3, tr ("One WAV per drum (stems)..."));
    juce::Component::SafePointer<FaseGrooveDrumsEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&wavButton), [safe] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;
        const auto mode = (FaseGrooveDrumsProcessor::AudioExport) (result - 1);
        const int bars = safe->selectedClipBars();
        auto folder = FaseGrooveDrumsProcessor::userFolder().getChildFile ("Audio");
        folder.createDirectory();
        const auto suggested = folder.getChildFile (safe->processor.audioFileName (mode, bars));
        const bool stems = mode == FaseGrooveDrumsProcessor::AudioExport::stems;
        safe->chooser = std::make_unique<juce::FileChooser> (tr ("Save the audio"), suggested, stems ? "" : "*.wav");
        safe->chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                        | (stems ? 0 : juce::FileBrowserComponent::warnAboutOverwriting),
                                    [safe, mode, bars, stems] (const juce::FileChooser& fc)
        {
            if (safe == nullptr || fc.getResult() == juce::File())
                return;
            const auto target = stems ? fc.getResult() : fc.getResult().withFileExtension ("wav");
            const auto files = safe->processor.writeAudio (mode, bars, target);
            safe->showMessage (files.isEmpty() ? tr ("Could not write") + " " + target.getFileName()
                                               : tr ("Saved") + ": " + (stems ? juce::String (files.size()) + " WAV" : target.getFileName()));
        });
    });
}

bool FaseGrooveDrumsEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
        if (f.endsWithIgnoreCase (".mid") || f.endsWithIgnoreCase (".midi") || FaseGrooveDrumsProcessor::isAudioFile (juce::File (f))
            || juce::File (f).isDirectory())
            return true;
    return false;
}

void FaseGrooveDrumsEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    // A folder: a whole kit.
    for (const auto& f : files)
        if (juce::File (f).isDirectory())
        {
            sampleBrowser.show (juce::File (f), selectedLane);
            return;
        }

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
                showMessage (processor.loadSample (lane, file) ? tr (fgd::laneName (lane)) + ": " + file.getFileName()
                                                               : tr ("Could not read") + " " + file.getFileName());
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

    chooser = std::make_unique<juce::FileChooser> (tr ("Save the MIDI clip"), folder.getChildFile (processor.clipFileName (bars, part)), "*.mid");
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
