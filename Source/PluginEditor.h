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
/** A flat button with a drawn icon (and optional text): transport, dice, undo, locks... */
class IconButton : public juce::Button
{
public:
    enum class Icon { none, play, pause, stop, rewind, dice, undo, redo, gear, close, fullScreen, exitFullScreen,
                      lock, unlock, left, right, song, folder };

    explicit IconButton (Icon i = Icon::none) : juce::Button ({}), icon (i) {}
    void setIcon (Icon i) { if (i != icon) { icon = i; repaint(); } }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    static void drawIcon (juce::Graphics&, Icon, juce::Rectangle<float>, juce::Colour);

    juce::Colour onColour { Palette::accent };
    juce::Colour iconColour;   // when set: the icon's own colour while off (the dice are amber)
    bool primary = false;      // filled with onColour even when off (the main action of a panel)

private:
    Icon icon;
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

/** A drop-down menu for a parameter, with a caption above it (or none). The items can be grouped
    under headings in any order: an item's id is always "parameter index + 1". */
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
    bool hasCaption = true;
};

//==============================================================================
/** The rhythm grid: one row per drum (with its die), 32 steps (two bars). */
class DrumGrid : public juce::Component, public juce::SettableTooltipClient
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
    /** Called when a drum's name is double-clicked: show it in the velocity view. */
    std::function<void (int lane)> onShowVelocity;
    /** The drum row at a point (-1 outside), e.g. where a sample is dropped. */
    int laneAt (juce::Point<int>) const;
    void chooseSample (int lane);

private:
    juce::Rectangle<float> cellBounds (int lane, int step) const;
    juce::Rectangle<float> headerBounds (int lane) const;
    juce::Rectangle<float> diceBounds (int lane) const;
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
    bool hoverDice = false;
    char paintSymbol = 0;
    int lastPaintLane = -1, lastPaintStep = -1;
    std::unique_ptr<juce::FileChooser> chooser;
};

//==============================================================================
/** The velocity piano roll: one drum at a time (chosen from a menu), its hits on top and a
    velocity bar under each one, to drag up and down. */
class VelocityView : public juce::Component
{
public:
    explicit VelocityView (FaseGrooveDrumsProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    void refresh();
    void setLane (int lane);
    int getLane() const noexcept { return lane; }

private:
    juce::Rectangle<float> stripArea() const;
    juce::Rectangle<float> laneArea() const;
    juce::Rectangle<float> column (int step, juce::Rectangle<float> area) const;
    int stepAt (float x) const;
    void setFromMouse (juce::Point<float>);

    FaseGrooveDrumsProcessor& processor;
    int lane = (int) fgd::Lane::closedHat;
    juce::ComboBox drumBox, shapeBox;
    IconButton diceButton { IconButton::Icon::dice };
    juce::TextButton autoButton { "AUTO" }, autoAllButton { "ALL AUTO" };
    std::array<char, FaseGrooveDrumsProcessor::gridSteps> symbols {};
    std::array<int, FaseGrooveDrumsProcessor::gridSteps> levels {}, setByHand {};
    int playStep = -1, hoverStep = -1;
    bool painting = false;
};

//==============================================================================
/** Drag me into the DAW: writes the MIDI clip(s) and starts an external file drag. */
class DragTile : public juce::Component, public juce::SettableTooltipClient
{
public:
    DragTile (juce::String titleText = "DRAG MIDI TO YOUR DAW", juce::String subtitleText = "drag this tile onto a track")
        : title (std::move (titleText)), subtitle (std::move (subtitleText)) {}
    std::function<juce::StringArray()> createFiles;
    void setSubtitle (const juce::String& s) { if (s != subtitle) { subtitle = s; repaint(); } }
    void paint (juce::Graphics&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { dragging = false; repaint(); }
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }

private:
    juce::String title, subtitle;
    bool dragging = false;
};

//==============================================================================
/** The song window: bars, own variation and fill of each section; drag or save the whole song,
    one section, or every section as its own clip. */
class SongOverlay : public juce::Component
{
public:
    explicit SongOverlay (FaseGrooveDrumsProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void show();
    void refresh();

    FaseGrooveDrumsProcessor::ClipPart clipPart() const;

private:
    juce::Rectangle<float> card() const;
    juce::Rectangle<float> timelineArea() const;
    juce::Rectangle<int> rowArea (int part) const;
    void saveSong();
    void saveSections();

    FaseGrooveDrumsProcessor& processor;
    juce::OwnedArray<juce::ComboBox> barBoxes, fillBoxes;
    juce::OwnedArray<IconButton> newButtons;
    juce::OwnedArray<juce::TextButton> originalButtons;
    juce::TextButton allButton { "ONE TRACK" }, tracksButton { "ONE TRACK PER DRUM" };
    juce::TextButton saveSongButton { "SAVE SONG .MID..." }, saveSectionsButton { "ONE .MID PER SECTION..." }, closeButton { "CLOSE" };
    juce::TextButton resetButton { "RESET SONG" };
    DragTile sectionTile { "DRAG THIS SECTION" }, sectionsTile { "DRAG ALL SECTIONS", "one clip per section" },
             songTile { "DRAG THE WHOLE SONG", "every section, with markers" };
    std::unique_ptr<juce::FileChooser> chooser;
    int selected = 1;   // the section the tile drags (part index)
};

//==============================================================================
/** Credits, links, shortcuts and licence: the gear, or a click on the logo. */
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
class FaseGrooveDrumsEditor : public juce::AudioProcessorEditor, public juce::FileDragAndDropTarget, private juce::Timer
{
public:
    explicit FaseGrooveDrumsEditor (FaseGrooveDrumsProcessor&);
    ~FaseGrooveDrumsEditor() override;

    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray&, int, int) override;

    static constexpr int designWidth = 1180, designHeight = 760;

private:
    void timerCallback() override;

    /** Everything is laid out at the design size, then scaled (and centred) in the window. */
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
    void showPresetsMenu();
    void showGroovesMenu();
    void askName (const juce::String& title, const juce::String& suggestion, std::function<void (const juce::String&)> onName);
    void importMidi();
    void setView (bool velocity);
    void stepGroove (int delta);
    void undo();
    void redo();
    bool isFullScreen() const;
    void toggleFullScreen();
    void showMessage (const juce::String& text);

    FaseGrooveDrumsProcessor& processor;
    GrooveLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips;
    Content content { *this };

    // Header
    IconButton optionsButton { IconButton::Icon::gear }, undoButton { IconButton::Icon::undo }, redoButton { IconButton::Icon::redo },
               totalRandomButton { IconButton::Icon::dice };
    juce::ComboBox styleBox;
    juce::TextButton presetsButton { "PRESETS" }, resetButton { "RESET" };
    IconButton rewindButton { IconButton::Icon::rewind }, playButton { IconButton::Icon::play }, stopButton { IconButton::Icon::stop };
    juce::TextButton syncButton { "SYNC DAW" };
    juce::Slider tempoSlider;
    IconButton songButton { IconButton::Icon::song }, fullScreenButton { IconButton::Icon::fullScreen };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment, soundAttachment, crashAttachment, sequencerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;

    // Groove
    ParamCombo grooveCombo, genreCombo;
    IconButton prevGroove { IconButton::Icon::left }, nextGroove { IconButton::Icon::right };
    IconButton newBeatButton { IconButton::Icon::dice };
    juce::TextButton myGroovesButton { "MY GROOVES" };
    Knob density;

    // Feel
    Knob swing, humanize, velocity;
    juce::TextButton newFeelButton { "NEW FEEL" };

    // Song
    ParamCombo sectionCombo, fillsCombo, fillStyleCombo, variationCombo;
    juce::TextButton crashButton { "CRASH AFTER FILLS" };

    // Grid / velocity
    juce::TextButton gridTab { "GRID" }, velocityTab { "VELOCITY" }, clearButton { "CLEAR" };
    std::unique_ptr<ParamCombo> segmentCombos[fgd::numSegments];
    IconButton segmentDice[fgd::numSegments], segmentLocks[fgd::numSegments];
    DrumGrid grid;
    VelocityView velocityView;
    bool velocityShown = false;

    // Sound
    ParamCombo kitCombo, channelCombo;
    Knob tone, reverb, volume;
    juce::TextButton sequencerButton { "SEQUENCER" }, soundButton { "GUIDE SOUND" };

    // Export
    juce::ComboBox lengthBox;
    juce::TextButton tracksButton { "ONE TRACK PER DRUM" }, saveButton { "SAVE .MID" };
    DragTile dragTile;
    std::unique_ptr<juce::FileChooser> chooser;

    SongOverlay songOverlay { processor };
    AboutPanel about;
    std::vector<ParamCombo*> combos;
    juce::Rectangle<int> logoArea;

    // Undo history: a snapshot twice a second; a change pushes the previous one.
    juce::StringArray undoStack, redoStack;
    juce::String lastSnapshot;
    int snapshotTicks = 0;

    juce::Rectangle<int> sizeBeforeFullScreen;
    juce::String message;
    int messageTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGrooveDrumsEditor)
};
