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
            g.setColour (Palette::text.withAlpha (0.92f));
            g.setFont (font (12.0f, true));
            g.drawText (title, area.withHeight (24).reduced (12, 0), juce::Justification::centredLeft);
        }
    }

    const int clipBarChoices[] { 0, 1, 2, 4, 8, 16, FaseGrooveDrumsProcessor::songArrangement };
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

    styleCaption (label, caption);
    addAndMakeVisible (label);
    refresh();
}

void ParamCombo::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (15));
    box.setBounds (r.removeFromTop (30));
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
DrumGrid::DrumGrid (FaseGrooveDrumsProcessor& p) : processor (p)
{
    lights.fill (0.0f);
    refresh();
}

void DrumGrid::refresh()
{
    pattern = processor.displayedPattern();
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        muted[(size_t) l] = processor.isLaneMuted (l);
        // The lights follow the hits and fade out.
        const float hit = processor.laneFlash[(size_t) l].exchange (0.0f);
        lights[(size_t) l] = juce::jmax (hit, lights[(size_t) l] * 0.8f);
    }
    playStep = processor.uiStep.load();
    repaint();
}

juce::Rectangle<float> DrumGrid::headerBounds (int lane) const
{
    const float rowH = (float) getHeight() / (float) fgd::numLanes;
    return { 0.0f, rowH * (float) lane, (float) headerWidth, rowH };
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

        // Header: light, name, MIDI note.
        auto h = headerBounds (lane).reduced (2.0f, 1.5f);
        g.setColour (lane == hoverLane && hoverStep < 0 ? Palette::control.brighter (0.06f) : Palette::control);
        g.fillRoundedRectangle (h, 4.0f);
        g.setColour (colour.withAlpha (isMuted ? 0.25f : 0.9f));
        g.fillRoundedRectangle (h.withWidth (3.0f), 1.5f);

        const auto light = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ h.getX() + 13.0f, h.getCentreY() });
        g.setColour (Palette::field);
        g.fillEllipse (light);
        g.setColour (colour.withAlpha (juce::jlimit (0.0f, 1.0f, lights[(size_t) lane])));
        g.fillEllipse (light);

        g.setColour (isMuted ? Palette::dim.withAlpha (0.6f) : Palette::text);
        g.setFont (font (11.5f, true));
        g.drawText (fgd::laneShortName (lane), h.withTrimmedLeft (22.0f).withTrimmedRight (26.0f), juce::Justification::centredLeft);
        g.setColour (Palette::dim);
        g.setFont (font (10.0f));
        g.drawText (isMuted ? juce::String ("MUTE") : juce::String (processor.laneNote (lane)),
                    h.withTrimmedRight (6.0f), juce::Justification::centredRight);

        // Steps.
        const auto& line = pattern.lanes[(size_t) lane];
        for (int step = 0; step < steps; ++step)
        {
            auto c = cellBounds (lane, step).reduced (1.5f, 2.0f);
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
        }
    }

    // Bar line between the two bars.
    g.setColour (Palette::dim.withAlpha (0.5f));
    const float x = cellBounds (0, 16).getX();
    g.drawLine (x, 0.0f, x, (float) getHeight(), 1.0f);
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
        // The header: click mutes, right-click opens the drum's menu.
        if (juce::isPositiveAndBelow (lane, fgd::numLanes) && e.position.x < (float) headerWidth)
        {
            if (e.mods.isPopupMenu())
                showLaneMenu (lane);
            else
                processor.setLaneMuted (lane, ! processor.isLaneMuted (lane));
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
    if (newLane != hoverLane || newStep != hoverStep)
    {
        hoverLane = newLane;
        hoverStep = newStep;
        repaint();
    }
}

void DrumGrid::mouseExit (const juce::MouseEvent&)
{
    hoverLane = hoverStep = -1;
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
    menu.addItem (2, "Clear this row");

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
        if (result == 1)                        p.setLaneMuted (lane, ! p.isLaneMuted (lane));
        else if (result == 2)                   p.clearLane (lane);
        else if (result == 999)                 p.setParameterFromUI (ParamIDs::note (lane), (float) fgd::defaultNote (lane));
        else if (result >= 1000 && result < 1128) p.setParameterFromUI (ParamIDs::note (lane), (float) (result - 1000));
        else if (result >= 2000)                p.setParameterFromUI (ParamIDs::level (lane), (float) (result - 2000) / 100.0f);
        safe->refresh();
    });
}

//==============================================================================
void DragTile::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (isMouseOver() ? Palette::accent.withAlpha (0.22f) : Palette::accent.withAlpha (0.12f));
    g.fillRoundedRectangle (r, 8.0f);
    const float dashes[] { 5.0f, 4.0f };
    juce::Path outline;
    outline.addRoundedRectangle (r, 8.0f);
    juce::Path dashed;
    juce::PathStrokeType (1.5f).createDashedStroke (dashed, outline, dashes, 2);
    g.setColour (Palette::accent.withAlpha (dragging ? 1.0f : 0.8f));
    g.fillPath (dashed);

    g.setColour (Palette::text);
    g.setFont (font (15.0f, true));
    g.drawText ("DRAG MIDI TO YOUR DAW", r.withTrimmedBottom (r.getHeight() * 0.4f), juce::Justification::centredBottom);
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    g.drawText ("drag this tile onto a track", r.withTrimmedTop (r.getHeight() * 0.62f), juce::Justification::centredTop);
}

void DragTile::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging || e.getDistanceFromDragStart() < 4 || createFile == nullptr)
        return;

    const auto file = createFile();
    if (! file.existsAsFile())
        return;

    dragging = true;
    repaint();
    juce::Component::SafePointer<DragTile> safe (this);
    juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this, [safe]
    {
        if (safe != nullptr)
        {
            safe->dragging = false;
            safe->repaint();
        }
    });
}

//==============================================================================
AboutPanel::AboutPanel()
{
    artistLogo = Branding::artistLogo();

    auto link = [this] (juce::TextButton& b, const char* url, juce::Colour colour)
    {
        b.setVisible (juce::String (url).isNotEmpty());
        addChildComponent (b);
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
    return getLocalBounds().withSizeKeepingCentre (560, 470);
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
    r.removeFromTop (12);

    auto row = [&] (const juce::String& what, const juce::String& who)
    {
        auto line = r.removeFromTop (24);
        g.setColour (Palette::dim);
        g.setFont (font (13.0f));
        g.drawText (what, line.removeFromLeft (230), juce::Justification::centredLeft);
        g.setColour (Palette::text);
        g.setFont (font (14.0f, true));
        g.drawText (who, line, juce::Justification::centredLeft);
    };
    row ("Idea, music direction and sound", Branding::artist);
    row ("Code", "Claude (Anthropic)");
    row ("Framework", "JUCE");
    row ("Licence", Branding::licence);

    r.removeFromTop (8);
    if (artistLogo.isValid())
        g.drawImageWithin (artistLogo, r.getX(), r.getY(), r.getWidth(), 70, juce::RectanglePlacement::centred);
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
FaseGrooveDrumsEditor::FaseGrooveDrumsEditor (FaseGrooveDrumsProcessor& p)
    : AudioProcessorEditor (&p), processor (p), tooltips (this, 600),
      grooveCombo (p.apvts, ParamIDs::pattern, "Groove"),
      genreCombo (p.apvts, ParamIDs::genre, "New beat genre"),
      density (p.apvts, ParamIDs::density, "Density"),
      swing (p.apvts, ParamIDs::swing, "Swing"),
      humanize (p.apvts, ParamIDs::humanize, "Humanize"),
      velocity (p.apvts, ParamIDs::velocity, "Velocity"),
      sectionCombo (p.apvts, ParamIDs::section, "Section"),
      fillsCombo (p.apvts, ParamIDs::fills, "Fills"),
      fillStyleCombo (p.apvts, ParamIDs::fillStyle, "Fill style"),
      variationCombo (p.apvts, ParamIDs::variation, "Variation"),
      grid (p),
      kitCombo (p.apvts, ParamIDs::kit, "Guide kit"),
      channelCombo (p.apvts, ParamIDs::midiChannel, "MIDI ch."),
      tone (p.apvts, ParamIDs::tone, "Tone"),
      reverb (p.apvts, ParamIDs::reverb, "Reverb"),
      volume (p.apvts, ParamIDs::volume, "Volume")
{
    setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (content);
    content.setWantsKeyboardFocus (true);

    auto show = [this] (juce::Component& c) { content.addAndMakeVisible (c); };

    // ---- Header ----
    styleBox.setTextWhenNothingSelected ("STYLE...");
    styleBox.addSectionHeading ("One-click setups by genre");
    const auto styleNames = FaseGrooveDrumsProcessor::styleNames();
    for (int i = 0; i < styleNames.size(); ++i)
        styleBox.addItem (styleNames[i], i + 1);
    styleBox.onChange = [this]
    {
        if (const int id = styleBox.getSelectedId(); id > 0)
            processor.applyStyle (id - 1);
        styleBox.setSelectedId (0, juce::dontSendNotification);
    };
    styleBox.setTooltip ("A groove, its swing, feel, fills and kit for a genre, in one click.");
    show (styleBox);

    resetButton.setTooltip ("Every setting back to its default.");
    resetButton.onClick = [this] { processor.resetToDefaults(); };
    show (resetButton);

    playButton.setTooltip ("Play / pause the groove without the DAW (space bar).");
    playButton.onClick = [this] { processor.previewToggle(); };
    show (playButton);
    stopButton.setButtonText ("STOP");
    stopButton.setTooltip ("Stop and go back to the start.");
    stopButton.onClick = [this] { processor.previewStop(); };
    show (stopButton);

    syncButton.setClickingTogglesState (true);
    syncButton.setTooltip ("On: the DAW's tempo and position. Off: your own tempo (still started by the DAW's play).");
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::tempoSync, syncButton);
    show (syncButton);

    tempoSlider.setSliderStyle (juce::Slider::LinearBar);
    tempoSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 110, 30);
    tempoSlider.setTooltip ("Tempo when SYNC DAW is off (drag or double-click to type).");
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, ParamIDs::tempo, tempoSlider);
    show (tempoSlider);

    soundButton.setClickingTogglesState (true);
    soundButton.setTooltip ("The guide kit: hear the groove here. Off: MIDI only, for your own drums.");
    soundAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sound, soundButton);
    show (soundButton);

    aboutButton.onClick = [this] { about.setVisible (true); about.toFront (false); };
    show (aboutButton);

    // ---- Groove ----
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
    grooveCombo.setTip ("The groove, by genre (the same genres as Fase Groove Piano). Edit any groove on the grid.");
    show (grooveCombo);
    genreCombo.setTip ("The genre NEW BEAT writes in.");
    show (genreCombo);
    newBeatButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    newBeatButton.setTooltip ("A brand new random beat in the chosen genre.");
    newBeatButton.onClick = [this] { processor.rollNewBeat(); };
    show (newBeatButton);
    density.setTip ("How busy NEW BEAT is: from kick, clap and hats to 16ths and percussion.");
    show (density);

    // ---- Feel ----
    swing.setTip ("Pushes every other 16th later: 0% straight, 100% full shuffle.");
    humanize.setTip ("Small timing and velocity changes, like a drummer (the kick stays tight).");
    velocity.setTip ("Overall velocity of the hits.");
    for (auto* k : { &swing, &humanize, &velocity })
        show (*k);
    newFeelButton.setTooltip ("Another humanization (the same every time you play it).");
    newFeelButton.onClick = [this] { processor.newFeel(); };
    show (newFeelButton);

    // ---- Song ----
    sectionCombo.setTip ("Song section: Intro / Outro (kick and hats), Break (no kick), Build-up (snare roll), Drop (crash every 8 bars).");
    fillsCombo.setTip ("A fill at the end of every N bars.");
    fillStyleCombo.setTip ("What the fill plays. Mix: a different one now and then.");
    variationCombo.setTip ("Every 4 or 8 bars the last bar plays a variation (a hat lift, a pick-up...).");
    for (auto* c : { &sectionCombo, &variationCombo, &fillsCombo, &fillStyleCombo })
        show (*c);
    crashButton.setClickingTogglesState (true);
    crashButton.setTooltip ("A crash on the bar after every fill.");
    crashAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::crash, crashButton);
    show (crashButton);

    // ---- Grid ----
    show (grid);
    clearButton.setTooltip ("Empty the grid and start your own beat.");
    clearButton.onClick = [this] { processor.clearGrid(); grid.refresh(); };
    show (clearButton);

    // ---- Sound ----
    show (kitCombo);
    show (channelCombo);
    channelCombo.setTip ("The MIDI channel of the notes (10 is the drum channel of General MIDI).");
    for (auto* k : { &tone, &reverb, &volume })
        show (*k);
    sequencerButton.setClickingTogglesState (true);
    sequencerButton.setTooltip ("Off: the groove doesn't play with the DAW (e.g. while a dragged clip plays on this track).");
    sequencerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sequencer, sequencerButton);
    show (sequencerButton);

    // ---- Export ----
    const char* lengths[] { "Loop", "1 bar", "2 bars", "4 bars", "8 bars", "16 bars", "Whole song" };
    for (int i = 0; i < (int) std::size (lengths); ++i)
        lengthBox.addItem (lengths[i], i + 1);
    lengthBox.setSelectedId (4, juce::dontSendNotification);
    lengthBox.setTooltip ("Length of the clip. Loop: until groove, fills and variations come round. "
                          "Whole song: intro, main, break, build-up, drop and outro with markers.");
    show (lengthBox);
    tracksButton.setClickingTogglesState (true);
    tracksButton.setTooltip ("One MIDI track per drum (kick, snare, hats...) instead of one drum track.");
    show (tracksButton);
    dragTile.createFile = [this] { return processor.writeMidiClip (selectedClipBars(), selectedClipPart()); };
    show (dragTile);
    saveButton.setTooltip ("Save the clip as a .mid file.");
    saveButton.onClick = [this] { saveMidiFile(); };
    show (saveButton);

    content.addChildComponent (about);

    combos = { &grooveCombo, &genreCombo, &sectionCombo, &fillsCombo, &fillStyleCombo, &variationCombo, &kitCombo, &channelCombo };

    const float scale = juce::jlimit (0.6f, 2.0f, processor.editorScale);
    setResizable (true, true);
    setResizeLimits ((int) (designWidth * 0.6f), (int) (designHeight * 0.6f), designWidth * 2, designHeight * 2);
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
    const float scale = (float) getWidth() / (float) designWidth;
    processor.editorScale = scale;
    content.setBounds (0, 0, designWidth, designHeight);
    content.setTransform (juce::AffineTransform::scale (scale));
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
    if (key == juce::KeyPress::spaceKey)
    {
        editor.processor.previewToggle();
        return true;
    }
    if (key == juce::KeyPress::escapeKey && editor.about.isVisible())
    {
        editor.about.setVisible (false);
        return true;
    }
    return false;
}

void FaseGrooveDrumsEditor::layout()
{
    // Header
    logoArea = { 16, 10, 250, 44 };
    styleBox.setBounds (290, 16, 170, 32);
    resetButton.setBounds (466, 16, 70, 32);
    playButton.setBounds (560, 14, 60, 36);
    stopButton.setBounds (624, 14, 60, 36);
    syncButton.setBounds (704, 16, 92, 32);
    tempoSlider.setBounds (800, 16, 110, 32);
    soundButton.setBounds (1000, 16, 80, 32);
    aboutButton.setBounds (1088, 16, 76, 32);

    // Groove panel (12, 76, 290, 214)
    grooveCombo.setBounds (24, 102, 266, 46);
    genreCombo.setBounds (24, 156, 176, 46);
    newBeatButton.setBounds (24, 214, 176, 36);
    density.setBounds (206, 160, 84, 96);

    // Feel panel (12, 298, 290, 130)
    swing.setBounds (24, 322, 82, 98);
    humanize.setBounds (110, 322, 82, 98);
    velocity.setBounds (196, 322, 82, 98);
    newFeelButton.setBounds (214, 302, 80, 20);

    // Song panel (12, 436, 290, 160)
    sectionCombo.setBounds (24, 460, 130, 46);
    variationCombo.setBounds (160, 460, 130, 46);
    fillsCombo.setBounds (24, 512, 130, 46);
    fillStyleCombo.setBounds (160, 512, 130, 46);
    crashButton.setBounds (24, 564, 266, 26);

    // Grid panel (312, 76, 856, 520)
    clearButton.setBounds (1090, 80, 70, 20);
    grid.setBounds (320, 106, 840, 482);

    // Sound panel (12, 604, 600, 104)
    kitCombo.setBounds (24, 630, 150, 46);
    tone.setBounds (184, 626, 76, 78);
    reverb.setBounds (264, 626, 76, 78);
    volume.setBounds (344, 626, 76, 78);
    channelCombo.setBounds (430, 630, 80, 46);
    sequencerButton.setBounds (520, 646, 84, 30);

    // Export panel (620, 604, 548, 104)
    lengthBox.setBounds (632, 640, 130, 28);
    tracksButton.setBounds (632, 674, 130, 24);
    dragTile.setBounds (772, 630, 250, 70);
    saveButton.setBounds (1032, 642, 124, 44);

    about.setBounds (content.getLocalBounds());
}

void FaseGrooveDrumsEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (Palette::background);

    // Header band
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff15181e), 0.0f, 0.0f, juce::Colour (0xff101317), 0.0f, 64.0f, false));
    g.fillRect (0, 0, designWidth, 64);
    g.setColour (Palette::panelEdge);
    g.drawHorizontalLine (64, 0.0f, (float) designWidth);
    Branding::drawFullLogo (g, logoArea.toFloat());

    // Tempo in use
    g.setColour (Palette::dim);
    g.setFont (font (11.0f, true));
    g.drawText ("BPM", 914, 14, 70, 14, juce::Justification::centredLeft);
    g.setColour (Palette::text);
    g.setFont (font (16.0f, true));
    g.drawText (juce::String (processor.lastBpm.load(), 1), 914, 28, 80, 22, juce::Justification::centredLeft);

    drawPanel (g, { 12, 76, 290, 214 }, "GROOVE");
    drawPanel (g, { 12, 298, 290, 130 }, "FEEL");
    drawPanel (g, { 12, 436, 290, 160 }, "SONG");
    drawPanel (g, { 312, 76, 856, 520 }, "GRID");
    drawPanel (g, { 12, 604, 600, 104 }, "SOUND & MIDI");
    drawPanel (g, { 620, 604, 548, 104 }, "EXPORT");

    // The grid's name and hint
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    const auto groove = juce::String (fgd::GrooveEngine::basePattern (processor.currentSettings()).name);
    g.drawText (groove.toUpperCase() + "   -   click: hit   shift: accent   alt: ghost   cmd/ctrl: flam   right-click: menu   drag: paint",
                370, 80, 710, 20, juce::Justification::centredLeft);

    g.setColour (Palette::dim);
    g.setFont (font (11.0f, true));
    g.drawText ("LENGTH", 632, 624, 130, 14, juce::Justification::centredLeft);
}

void FaseGrooveDrumsEditor::timerCallback()
{
    for (auto* c : combos)
        c->refresh();
    grid.refresh();

    const bool playing = processor.previewOn.load();
    playButton.setButtonText (playing ? "PAUSE" : "PLAY");
    playButton.setToggleState (playing, juce::dontSendNotification);
    tempoSlider.setEnabled (processor.raw (ParamIDs::tempoSync) < 0.5f);
    content.repaint (900, 10, 100, 44);
    content.repaint (360, 78, 730, 24);
}

int FaseGrooveDrumsEditor::selectedClipBars() const
{
    return clipBarChoices[juce::jlimit (0, (int) std::size (clipBarChoices) - 1, lengthBox.getSelectedId() - 1)];
}

FaseGrooveDrumsProcessor::ClipPart FaseGrooveDrumsEditor::selectedClipPart() const
{
    return tracksButton.getToggleState() ? FaseGrooveDrumsProcessor::ClipPart::tracks : FaseGrooveDrumsProcessor::ClipPart::all;
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
