// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "Branding.h"
#include "Localisation.h"
#include "Updater.h"
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
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

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
    void setTip (const juce::String& tip) { Localisation::bindTip (slider, tip); }

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
    void setTip (const juce::String& tip) { Localisation::bindTip (box, tip); }

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
    /** What a click on an empty step draws: 'X' accent, 'x' hit, 'g' ghost, 'f' flam (the editor's brush). */
    char brush = 'x';
    explicit DrumGrid (FaseGrooveDrumsProcessor&);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    /** Re-reads the groove and the playhead (editor timer). */
    void refresh();
    /** The drums' lights (they flash on the hits) and the drum picked in the kit panel. */
    void setLights (const std::array<float, fgd::numLanes>& l, int selected) { lights = l; selectedLane = selected; }
    std::function<void (int lane)> onSelectLane;

    static constexpr int headerWidth = 112;
    /** Called when a drum's name is double-clicked: show it in the velocity view. */
    std::function<void (int lane)> onShowVelocity;
    /** Called to choose a drum's sample in the sample browser. */
    std::function<void (int lane)> onBrowseSamples;
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
    int playStep = -1, selectedLane = -1;
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
/** The fill drawn by hand (Fill style: Custom): one bar, every drum. It takes over from its first
    written step; the drums left empty keep playing the groove. */
class FillView : public juce::Component, public juce::SettableTooltipClient
{
public:
    char brush = 'x';   // what a click on an empty step draws (the editor's brush)
    explicit FillView (FaseGrooveDrumsProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { paintSymbol = 0; }

private:
    juce::Rectangle<float> gridArea() const;
    bool cellAt (juce::Point<float>, int& lane, int& step) const;
    void paintCell (int lane, int step, char symbol);

    FaseGrooveDrumsProcessor& processor;
    juce::TextButton startButton { "START FROM..." }, clearButton { "CLEAR FILL" };
    char paintSymbol = 0;
    int lastLane = -1, lastStep = -1;
};

//==============================================================================
/** Drag me into the DAW: writes the MIDI clip(s) and starts an external file drag. */
class DragTile : public juce::Component, public juce::SettableTooltipClient
{
public:
    DragTile (juce::String titleText = "DRAG MIDI TO DAW", juce::String subtitleText = "drag this tile onto a track")
        : title (std::move (titleText)), subtitle (std::move (subtitleText)) {}
    std::function<juce::StringArray()> createFiles;
    bool filled = false;   // the big orange tile of the export panel
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
    juce::OwnedArray<juce::TextButton> groupButtons;
    std::vector<juce::TextButton*> partButtonsInOrder()
    {
        std::vector<juce::TextButton*> all { &allButton };
        for (auto* b : groupButtons)
            all.push_back (b);
        all.push_back (&tracksButton);
        return all;
    }   // the instrument groups, between ONE TRACK and ONE TRACK PER ...
    juce::TextButton saveSongButton { "SAVE SONG .MID..." }, saveSectionsButton { "ONE .MID PER SECTION..." }, closeButton { "CLOSE" };
    juce::TextButton resetButton { "RESET SONG" };
    DragTile sectionTile { "DRAG THIS SECTION" }, sectionsTile { "DRAG ALL SECTIONS", "one clip per section" },
             songTile { "DRAG THE WHOLE SONG", "every section, with markers" };
    std::unique_ptr<juce::FileChooser> chooser;
    int selected = 1;   // the section the tile drags (part index)
};

//==============================================================================
/** OPTIONS, as in Fase Groove Piano: a SETTINGS tab (language, interface size, tips, updates,
    shortcuts) and an ABOUT tab (credits, licence, links). OPTIONS, or a click on the logo. */
class OptionsOverlay : public juce::Component
{
public:
    OptionsOverlay();

    std::function<void (float)> onScaleChosen;
    std::function<void (bool)> onTipsChanged;
    std::function<void()> onCheckUpdates;

    juce::TextButton checkUpdatesButton;

    void show (bool aboutTab, float currentScale);
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (620.0f, 620.0f); }
    void selectTab (bool about);

    bool showingAbout = false;
    juce::Image artist;
    juce::TextButton settingsTab, aboutTab, closeButton;
    juce::TextButton sourceButton, kofiButton, paypalButton, spotifyButton;
    juce::Label languageLabel, sizeLabel;
    juce::ComboBox languageBox, sizeBox;
    juce::ToggleButton tipsToggle, updatesToggle;
};

//==============================================================================
/** "A new version is ready": what's new (the changelog) and the user confirms the install. */
class UpdateOverlay : public juce::Component
{
public:
    UpdateOverlay();
    void show (const Updater::Result&);
    void refreshText();
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (640.0f, 560.0f); }
    Updater::Result result;
    juce::TextEditor notes;
    juce::TextButton installButton, laterButton, skipButton;
};

//==============================================================================
/** One of the four parts of the groove (a half bar), shown like the piano's chord slots: its
    position, the part playing there (A-D), a picture of its hits, its die and its lock. */
class PartCard : public juce::Component
{
public:
    PartCard (FaseGrooveDrumsProcessor&, int position);
    void paint (juce::Graphics&) override;
    void resized() override;
    void refresh (const fgd::DrumPattern& shown, int playStep);

private:
    FaseGrooveDrumsProcessor& processor;
    const int position;
    ParamCombo combo;
    IconButton dice { IconButton::Icon::dice }, lock { IconButton::Icon::unlock };
    fgd::DrumPattern pattern;
    bool playing = false, locked = false;
};

//==============================================================================
/** The groove browser: the genres on the left, their grooves on the right, each with a picture
    of its rhythm. A click loads it (you hear it at once if it plays). */
class GrooveBrowser : public juce::Component
{
public:
    explicit GrooveBrowser (FaseGrooveDrumsProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { hover = -1; repaint(); }
    bool keyPressed (const juce::KeyPress&) override;
    void show();

private:
    juce::Rectangle<float> card() const;
    juce::Rectangle<float> genreBounds (int genre) const;
    juce::Rectangle<float> tileBounds (int index) const;
    std::vector<int> groovesOf (int genre) const;
    void pick (int pattern);

    FaseGrooveDrumsProcessor& processor;
    int genre = 0, hover = -1;
    IconButton randomButton { IconButton::Icon::dice };
    juce::TextButton closeButton { "CLOSE" };
};

//==============================================================================
/** The sample browser: a folder of samples, each recognised as a drum (by its name, its folder's
    name or its sound). Pick a drum on the left, click a sample to hear it, USE it (or double-click).
    Wrongly recognised? Drag the samples (several with shift / ctrl) onto the right drum. */
class SampleBrowser : public juce::Component, public juce::DragAndDropContainer, public juce::DragAndDropTarget,
                      private juce::ListBoxModel, private juce::Timer
{
public:
    explicit SampleBrowser (FaseGrooveDrumsProcessor&);
    ~SampleBrowser() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

    /** Opens the browser on a folder (the last one if none), for a drum. */
    void show (const juce::File& folder, int lane);
    std::function<void (const juce::String&)> onMessage;

    bool isInterestedInDragSource (const SourceDetails&) override;
    void itemDragMove (const SourceDetails&) override;
    void itemDragExit (const SourceDetails&) override;
    void itemDropped (const SourceDetails&) override;

private:
    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
    void selectedRowsChanged (int row) override;
    juce::var getDragSourceDescription (const juce::SparseSet<int>& rows) override;

    juce::Rectangle<float> card() const;
    juce::Rectangle<float> drumBounds (int lane) const;
    int drumAt (juce::Point<float>) const;
    void chooseFolder();
    void scan (const juce::File& folder);   // adds the folder (or reads it again)
    void showFoldersMenu();                 // the folders read, each one can be left out
    void removeFolder (const juce::File& folder);
    void scanFinished();
    void timerCallback() override { repaint(); }
    juce::String relativePath (const juce::File&) const;
    void rebuildList();
    void use (int row);
    void moveSelectedTo (int newLane);
    void showRowMenu (int row);
    int countFor (int lane) const;

    FaseGrooveDrumsProcessor& processor;
    std::vector<FaseGrooveDrumsProcessor::SampleInfo> found;
    std::vector<int> rows;   // indexes into found, for the drum shown
    juce::File folder;                      // the last one added
    juce::Array<juce::File> folders;        // every folder in the browser
    std::shared_ptr<std::atomic<float>> progress = std::make_shared<std::atomic<float>> (0.0f);
    int pendingScans = 0;
    int lane = 0, dropLane = -1;
    bool scanning = false, showAll = false;
    juce::ListBox list;
    juce::TextButton folderButton { "ADD FOLDER..." }, foldersButton, clearListButton { "CLEAR LIST" }, autoButton { "AUTO KIT" }, allButton { "ALL FILES" }, closeButton { "CLOSE" },
                     chooseButton { "CHOOSE A FOLDER OF SAMPLES..." }, builtInButton { "BUILT-IN" }, fileButton { "ONE FILE..." },
                     allBuiltInButton { "ALL BUILT-IN" };
    std::unique_ptr<juce::FileChooser> chooser;
};

//==============================================================================
/** The mixer: a volume fader and a mute for each drum of the guide kit (and your samples). */
class MixerView : public juce::Component
{
public:
    explicit MixerView (FaseGrooveDrumsProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void setLights (const std::array<float, fgd::numLanes>& l) { lights = l; repaint(); }

private:
    juce::Rectangle<int> strip (int lane) const;
    FaseGrooveDrumsProcessor& processor;
    juce::OwnedArray<juce::Slider> faders, pushKnobs, swingKnobs;
    juce::OwnedArray<juce::TextButton> mutes;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> faderAttachments, knobAttachments;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachments;
    std::array<float, fgd::numLanes> lights {};
};

//==============================================================================
/** The kit as 13 pads: they light up on the hits; click one to hear it and pick it. */
class DrumPads : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit DrumPads (FaseGrooveDrumsProcessor& p) : processor (p) { setTooltip ("Click a pad: hear the drum and pick it. Right-click: mute"); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void setLights (const std::array<float, fgd::numLanes>& l, int selected) { lights = l; selectedLane = selected; repaint(); }
    std::function<void (int lane)> onSelectLane;

private:
    juce::Rectangle<float> padBounds (int lane) const;
    FaseGrooveDrumsProcessor& processor;
    std::array<float, fgd::numLanes> lights {};
    int selectedLane = 0;
};

//==============================================================================
//==============================================================================
/** The groove at a glance, on the main window: every drum's hits and the playhead, only to
    look at. A click opens the editor. */
class GrooveOverview : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit GrooveOverview (FaseGrooveDrumsProcessor& p) : processor (p) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { if (onOpen != nullptr) onOpen(); }
    void refresh();
    std::function<void()> onOpen;

private:
    FaseGrooveDrumsProcessor& processor;
    fgd::DrumPattern pattern;
    std::array<bool, fgd::numLanes> muted {};
    int playStep = -1;
};

/** The volume of each drum in the guide sound, small: the only thing changed from the main
    window (everything else in the editor). */
class MiniMixer : public juce::Component
{
public:
    explicit MiniMixer (FaseGrooveDrumsProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Rectangle<int> strip (int lane) const;
    FaseGrooveDrumsProcessor& processor;
    juce::OwnedArray<juce::Slider> faders;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;
};

/** The editor, big, over the whole window (as Fase Groove Piano's piano roll): the transport, the
    brush (what a click draws), the views (step grid, velocity, mixer, fill) and the groove's tools. */
class EditorOverlay : public juce::Component
{
public:
    EditorOverlay();
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override {}   // nothing under it is clicked
    void refreshText();
    void setBrush (char symbol);
    std::function<void (char)> onBrush;
    std::function<int()> viewMode;           // the view shown (its caption above the tools)

    IconButton rewindButton { IconButton::Icon::rewind }, playButton { IconButton::Icon::play }, stopButton { IconButton::Icon::stop },
               closeButton { IconButton::Icon::close };
    juce::TextButton accentBrush, hitBrush, ghostBrush, flamBrush;
    char brush = 'x';
};

class FaseGrooveDrumsEditor : public juce::AudioProcessorEditor, public juce::FileDragAndDropTarget, public juce::Timer
{
public:
    explicit FaseGrooveDrumsEditor (FaseGrooveDrumsProcessor&);
    ~FaseGrooveDrumsEditor() override;

    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray&, int, int) override;

    static constexpr int designWidth = 1180, designHeight = 716;

    void timerCallback() override;

private:

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

    float currentScale() const;
    void applyTips();
    void translateHostWindow();
    void parentHierarchyChanged() override;
    void layout();
    void paintContent (juce::Graphics&);
    int selectedClipBars() const;
    FaseGrooveDrumsProcessor::ClipPart selectedClipPart() const;
    void saveMidiFile();
    void showPresetsMenu();
    void showGroovesMenu();
    void askName (const juce::String& title, const juce::String& suggestion, std::function<void (const juce::String&)> onName);
    void importMidi();
    void importAudio();
    void importFile (const juce::File&);
    void setView (int mode);   // 0 step grid, 1 velocity, 2 mixer, 3 fill
    void showTemplateMenu();
    void showKitMenu();
    void showAudioExportMenu();
    void loadKitFolder (const juce::File&);
    void stepGroove (int delta);
    void selectLane (int lane);
    void refreshKitPanel();
    void undo();
    void redo();
    bool isFullScreen() const;
    void toggleFullScreen();
    void showMessage (const juce::String& text);

    FaseGrooveDrumsProcessor& processor;
    GrooveLookAndFeel lookAndFeel;
    std::unique_ptr<juce::TooltipWindow> tooltips;
    Content content { *this };

    // Header
    juce::TextButton presetsButton { "PRESETS" }, styleButton { "STYLE" }, resetButton { "RESET" };
    IconButton totalRandomButton { IconButton::Icon::dice }, undoButton { IconButton::Icon::undo }, redoButton { IconButton::Icon::redo },
               fullScreenButton { IconButton::Icon::fullScreen }, optionsButton { IconButton::Icon::gear };
    IconButton rewindButton { IconButton::Icon::rewind }, playButton { IconButton::Icon::play }, stopButton { IconButton::Icon::stop };
    juce::ToggleButton sequencerToggle { "Sequencer" };
    juce::TextButton syncButton { "SYNC DAW" };
    juce::Slider tempoSlider;
    juce::Label dawTempo;   // the DAW's tempo, in place of the slider while SYNC DAW is on
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment, soundAttachment, crashAttachment, sequencerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;

    // Groove row
    ParamCombo grooveCombo, genreCombo, partLengthCombo;
    juce::TextButton grooveButton;
    IconButton prevGroove { IconButton::Icon::left }, nextGroove { IconButton::Icon::right };
    IconButton newBeatButton { IconButton::Icon::dice };
    juce::Slider densitySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<PartCard> partCards[fgd::numSegments];

    // Kit & sounds (the drum picked)
    int selectedLane = 0;
    juce::ComboBox drumBox, noteBox;
    juce::TextButton loadSampleButton { "YOUR SAMPLES..." }, builtInButton { "BUILT-IN" }, muteButton { "MUTE" };
    IconButton laneDiceButton { IconButton::Icon::dice };
    juce::Slider levelKnob, volumeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment, volumeAttachment;
    std::unique_ptr<juce::FileChooser> sampleChooser;

    // Pads
    DrumPads pads;

    // Groove panel
    ParamCombo sectionCombo, fillsCombo, variationCombo, fillStyleCombo;
    juce::TextButton crashButton { "CRASH" }, newFeelButton { "NEW FEEL" };
    Knob swing, humanize, velocity, accents;

    // Guide sound & MIDI out
    juce::ToggleButton soundToggle { "Guide sound" };
    ParamCombo kitCombo, channelCombo;
    Knob tone, reverb, volume;

    // Edit
    juce::TextButton gridTab { "STEP GRID" }, velocityTab { "VELOCITY" }, mixerTab { "MIXER" }, fillTab { "FILL" }, clearButton { "CLEAR" },
                     myGroovesButton { "MY GROOVES" }, importButton { "IMPORT MIDI" }, templateButton { "TEMPLATE..." }, timingButton { "TIMING..." };
    IconButton newRhythmButton { IconButton::Icon::dice };
    DrumGrid grid;
    VelocityView velocityView;
    MixerView mixerView;
    FillView fillView;
    int viewMode = 0;
    GrooveOverview overview { processor };
    MiniMixer miniMixer { processor };
    EditorOverlay editOverlay;
    juce::TextButton openEditorButton;
    // The same tools on the main window (the editor has its own): tabs groove / volumes, the dice,
    // MY GROOVES, IMPORT, TEMPLATE, CLEAR.
    juce::TextButton mainGrooveTab, mainVolumeTab, mainMyGrooves, mainImport, mainTemplate, mainClear;
    IconButton mainNewRhythm { IconButton::Icon::dice };
    bool showVolumes = false;
    void setMainTab (bool volumes);
    /** The button a menu opens from: the editor's while it is open, else the main window's. */
    juce::Component* menuTarget (juce::Component& inEditor, juce::Component& onMain) { return editOverlay.isVisible() ? &inEditor : &onMain; }
    void openEditor (int view = -1);

    // Export
    juce::ComboBox notesBox, lengthBox;
    DragTile dragTile;
    juce::TextButton saveButton { "SAVE .MID..." }, songButton { "FULL SONG..." }, wavButton { "SAVE .WAV..." };
    juce::TextButton kitFolderButton { "YOUR SAMPLES..." }, linkButton { "LINK" };
    std::unique_ptr<juce::FileChooser> chooser;

    SongOverlay songOverlay { processor };
    GrooveBrowser browser { processor };
    SampleBrowser sampleBrowser { processor };
    OptionsOverlay options;
    UpdateOverlay updateOverlay;
    Updater::Checker updateChecker;
    std::vector<ParamCombo*> combos;
    juce::Rectangle<int> tempoArea, logoArea, displayArea;
    std::array<float, fgd::numLanes> lights {};

    // Undo history: a snapshot twice a second; a change pushes the previous one.
    juce::StringArray undoStack, redoStack;
    juce::String lastSnapshot;
    int snapshotTicks = 0;
    int uiTicks = 0;

public:
    /** As in Fase Groove Piano: while a clip is dragged the window shrinks to a small strip, so
        the DAW's tracks are in view; it comes back when the clip is dropped. */
    void enterMiniMode();
    void exitMiniMode();
private:
    struct MiniStrip : public juce::Component
    {
        void paint (juce::Graphics&) override;
    };
    MiniStrip miniStrip;
    bool miniMode = false;
    juce::uint32 miniStartedAt = 0;
    juce::Point<int> fullSize;
    // Undo history: a snapshot only after the mouse has changed something (not polled).
    int pendingSnapshot = 0;
    void mouseUp (const juce::MouseEvent&) override { pendingSnapshot = 8; }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override { pendingSnapshot = 8; }

    juce::Rectangle<int> sizeBeforeFullScreen;
    juce::String message;
    int messageTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGrooveDrumsEditor)
};
