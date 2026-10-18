// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "Branding.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

// The "studio" theme of Fase Groove Piano: graphite surfaces, one warm and one cool accent.
namespace Palette
{
    const juce::Colour background { 0xff0d0f13 };   // window
    const juce::Colour panel      { 0xff161920 };   // panels
    const juce::Colour panelEdge  { 0xff272c35 };   // borders, tracks, empty cells
    const juce::Colour control    { 0xff1d2129 };   // buttons
    const juce::Colour field      { 0xff0f1216 };   // menus, displays
    const juce::Colour accent     { 0xffffa94d };   // amber: the kick, the main actions
    const juce::Colour accent2    { 0xff4cc3f0 };   // cyan: snare, clap, rim, playing
    const juce::Colour cymbals    { 0xff5fd68a };   // green: hats, ride, crash
    const juce::Colour percussion { 0xffb69cff };   // violet: shaker, tambourine, conga, bell
    const juce::Colour toms       { 0xffff5a5f };   // red: toms
    const juce::Colour text       { 0xffe7e9ec };
    const juce::Colour dim        { 0xff7f8794 };
}

/** Each drum's colour, everywhere: kick amber, snares cyan, cymbals green, percussion violet, toms red. */
juce::Colour laneColour (int lane);

//==============================================================================
class GrooveLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrooveLookAndFeel();

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& background, bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isDown, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    juce::Font getPopupMenuFont() override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& text, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    juce::Font getLabelFont (juce::Label&) override;

private:
    juce::Typeface::Ptr regular, semiBold;
};

//==============================================================================
/** Rotary knob with a caption under it. */
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& caption);
    void resized() override;
    void setTip (const juce::String& tip) { slider.setTooltip (tip); }

private:
    juce::Slider slider;
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

/** A drop-down menu for a parameter, with a caption above it. The items can be grouped under
    headings in any order: an item's id is always "parameter index + 1". */
class ParamCombo : public juce::Component
{
public:
    ParamCombo (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& caption);
    void resized() override;

    /** Re-reads the parameter (call it a few times a second). */
    void refresh();
    juce::ComboBox& comboBox() noexcept { return box; }
    void setTip (const juce::String& tip) { box.setTooltip (tip); }

private:
    juce::ComboBox box;
    juce::Label label;
    juce::RangedAudioParameter* parameter = nullptr;
};

//==============================================================================
/** The rhythm grid: one row per drum, 32 steps (two bars). */
class DrumGrid : public juce::Component
{
public:
    explicit DrumGrid (FaseGrooveDrumsProcessor&);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    /** Re-reads the groove and the playhead (editor timer). */
    void refresh();

    static constexpr int headerWidth = 112;

private:
    juce::Rectangle<float> cellBounds (int lane, int step) const;
    juce::Rectangle<float> headerBounds (int lane) const;
    bool cellAt (juce::Point<float>, int& lane, int& step) const;
    void showCellMenu (int lane, int step);
    void showLaneMenu (int lane);
    void paintCell (int lane, int step, char symbol);

    FaseGrooveDrumsProcessor& processor;
    fgd::DrumPattern pattern;
    std::array<bool, fgd::numLanes> muted {};
    std::array<float, fgd::numLanes> lights {};
    int playStep = -1;
    int hoverLane = -1, hoverStep = -1;
    char paintSymbol = 0;
    int lastPaintLane = -1, lastPaintStep = -1;
};

//==============================================================================
/** Drag me into the DAW: writes the MIDI clip and starts an external file drag. */
class DragTile : public juce::Component
{
public:
    std::function<juce::File()> createFile;
    void paint (juce::Graphics&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { dragging = false; repaint(); }

private:
    bool dragging = false;
};

//==============================================================================
/** Credits, links and licence: OPTIONS -> ABOUT, or a click on the logo. */
class AboutPanel : public juce::Component
{
public:
    AboutPanel();
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    juce::Rectangle<int> card() const;
    juce::TextButton sourceButton { "SOURCE CODE" }, kofiButton { "DONATE: KO-FI" }, paypalButton { "DONATE: PAYPAL" },
                     spotifyButton { "SPOTIFY" }, closeButton { "CLOSE" };
    juce::Image artistLogo;
};

//==============================================================================
class FaseGrooveDrumsEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit FaseGrooveDrumsEditor (FaseGrooveDrumsProcessor&);
    ~FaseGrooveDrumsEditor() override;

    void resized() override;

    static constexpr int designWidth = 1180, designHeight = 720;

private:
    void timerCallback() override;

    /** Everything is laid out at the design size, then scaled to the window. */
    struct Content : public juce::Component
    {
        explicit Content (FaseGrooveDrumsEditor& e) : editor (e) {}
        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent&) override;
        bool keyPressed (const juce::KeyPress&) override;
        FaseGrooveDrumsEditor& editor;
    };

    void layout();
    void paintContent (juce::Graphics&);
    int selectedClipBars() const;
    FaseGrooveDrumsProcessor::ClipPart selectedClipPart() const;
    void saveMidiFile();

    FaseGrooveDrumsProcessor& processor;
    GrooveLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips;
    Content content { *this };

    // Header
    juce::ComboBox styleBox;
    juce::TextButton resetButton { "RESET" }, playButton, stopButton, aboutButton { "ABOUT" };
    juce::TextButton syncButton { "SYNC DAW" }, soundButton { "SOUND" };
    juce::Slider tempoSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment, soundAttachment, crashAttachment,
                                                                         sequencerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;

    // Groove
    ParamCombo grooveCombo, genreCombo;
    juce::TextButton newBeatButton { "NEW BEAT" }, clearButton { "CLEAR" };
    Knob density;

    // Feel
    Knob swing, humanize, velocity;
    juce::TextButton newFeelButton { "NEW FEEL" };

    // Song
    ParamCombo sectionCombo, fillsCombo, fillStyleCombo, variationCombo;
    juce::TextButton crashButton { "CRASH AFTER FILLS" };

    // Grid
    DrumGrid grid;

    // Sound
    ParamCombo kitCombo, channelCombo;
    Knob tone, reverb, volume;
    juce::TextButton sequencerButton { "SEQUENCER" };

    // Export
    juce::ComboBox lengthBox;
    juce::TextButton tracksButton { "ONE TRACK PER DRUM" }, saveButton { "SAVE .MID" };
    DragTile dragTile;
    std::unique_ptr<juce::FileChooser> chooser;

    AboutPanel about;
    std::vector<ParamCombo*> combos;
    juce::Rectangle<int> logoArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGrooveDrumsEditor)
};
