# Fase Groove Drums – User guide

*🇮🇹 [Guida in italiano](GUIDA.md)*

## 1. Installation

Download the zip for your system from the
[Releases](https://github.com/wsx79/fasegroovedrums/releases/latest) page and unzip it.

| System | Copy | To |
|---|---|---|
| Windows | `Fase Groove Drums.vst3` | `C:\Program Files\Common Files\VST3\` |
| macOS | `Fase Groove Drums.vst3` / `Fase Groove Drums.component` | `~/Library/Audio/Plug-Ins/VST3/` / `~/Library/Audio/Plug-Ins/Components/` |
| Linux | `Fase Groove Drums.vst3` | `~/.vst3/` |

Then rescan the plug-ins in your DAW. On macOS, if the plug-in is blocked, run once in Terminal:
`xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Fase Groove Drums.vst3"` (and the same for the `.component`).
The **Standalone** app works without a DAW.

Load it as an **instrument** on a MIDI track. Press play in the DAW (or **PLAY** in the plug-in)
and the groove plays with the guide kit.

## 2. The groove

- **GROOVE:** 53 grooves in 13 genres (the same as Fase Groove Piano). **Random beat** is the
  beat of NEW BEAT, **User pattern** the one edited on the grid.
- **NEW BEAT:** a new random beat in the genre chosen in **NEW BEAT GENRE**. **DENSITY** goes from
  a bare kick-clap-hat to busy 16ths with percussion.
- **STYLE** (top): a genre's groove, swing, feel, fills and kit in one click. **RESET** puts
  everything back to the default.

## 3. The grid

13 drums (kick, snare, clap, rim, closed hat, open hat, ride, shaker, tambourine, conga, cowbell,
tom, crash) × 32 steps (two bars of 16ths).

| Action | Result |
|---|---|
| Click | hit (click again: rest) |
| Shift + click | accent |
| Alt + click | ghost note (very soft) |
| Cmd/Ctrl + click | flam (a soft grace note just before) |
| Drag | paints the same symbol on the other steps |
| Right-click a step | menu: accent, hit, ghost, flam, rest |
| Click a drum's name | pick it (KIT & SOUNDS, VELOCITY) |
| Right-click a drum's name | mute, its die, clear the row, **your own sample**, **MIDI note**, **level** |

The first edit copies the groove showing into the **User pattern**, which is saved with your
project. **CLEAR** empties the grid. The light next to each name flashes when the drum plays.

## 4. Feel

- **SWING:** pushes every other 16th later (0% straight, 100% full shuffle).
- **HUMANIZE:** small timing and velocity changes, like a drummer; the kick stays tight.
  **NEW FEEL** gives another humanization (always the same one when you play it back).
- **VELOCITY:** overall velocity.

## 5. Song

- **SECTION:** Main, Intro / Outro (kick and hats), Break (no kick), Build-up (a snare roll that
  doubles up, with a crescendo), Drop (the full groove with a crash every 8 bars). Automatable.
- **FILLS / FILL STYLE:** a fill at the end of every 1, 2, 4 or 8 bars: snare roll, tom run, clap
  build, hat lift, drop out, conga run, or mix. **CRASH AFTER FILLS** adds a crash on the next bar.
- **VARIATION:** every 4 or 8 bars the last bar varies (a hat lift, a kick pick-up, a ghost...).

## 6. Sound & MIDI

- **GUIDE KIT:** House 909, Deep 808, Disco Live, Lo-Fi Soul, all synthesized; **TONE**,
  **REVERB**, **VOLUME**. **SOUND** (top) switches the guide kit off: MIDI only, for your own drums.
- **MIDI CH.:** the channel of the notes (10 by default). The notes are General MIDI: kick 36,
  snare 38, clap 39, rim 37, closed hat 42, open hat 46, ride 51, shaker 82, tambourine 54,
  conga 63, cowbell 56, tom 45, crash 49. Change any of them on the grid (right-click the name).
- **SEQUENCER:** off, the groove doesn't play with the DAW (e.g. while a dragged clip plays on
  this track); MIDI arriving on the track still plays the guide kit.
- **SYNC DAW:** on, the DAW's tempo and position; off, the tempo set next to it.

## 7. Export

- **LENGTH:** Loop (until groove, fills and variations come round), 1–16 bars, or **Whole song**
  (Intro 8, Main 16, Break 8, Build-up 4, Drop 16, Outro 8 bars, with markers).
- **ONE TRACK PER DRUM:** one MIDI file with a track for each drum.
- **DRAG MIDI TO YOUR DAW:** drag the tile onto a track. The clips are also kept in
  `Documents/Fase Groove Drums/MIDI clips/`.
- **SAVE .MID:** save the clip where you want.

## 8. Shortcuts

- **Space bar:** play / pause without the DAW.
- Click the **logo** or **ABOUT** for credits, links and donations.

## 9. New in 0.2 and 0.3

- **The layout of Fase Groove Piano:** GROOVE row with the part cards A-D, KIT & SOUNDS (the drum picked: MIDI note, level, mute, die, sample), DRUM PADS (click: hear and pick, right-click: mute), GROOVE FEEL & SONG, GUIDE SOUND & MIDI OUT, the grid at the bottom and EXPORT MIDI on the right (FULL SONG... opens the song window).

- **Dice:** the die in the header is TOTAL RANDOM; every drum's row has its own die (a new line for that
  drum only); each part A-D has a die and a lock (locked parts are never changed by the dice).
- **PARTS A-D** (over the grid): which half bar plays where, e.g. A A A B.
- **VELOCITY** (next to GRID): pick a drum from the menu, drag its bars; SHAPE applies a groove
  (Offbeat push is the classic house hat), RANDOM rolls one, AUTO goes back to the automatic levels.
  Double-click a drum's name on the grid to open it there.
- **Your own sample:** right-click a drum's name → *Load your own sample...*, or drop a WAV / AIFF /
  FLAC / MP3 on its row. *Back to the built-in sound* removes it. The file stays where it is: the
  project remembers its path.
- **SONG:** the song window (bars, own variation and fill per section; drag or save the song or its sections).
- **PRESETS / MY GROOVES:** save and load; MY GROOVES also imports the drums of a MIDI file (or drop
  the .mid on the window).
- ⏮ ▶ ■, undo / redo (arrows, Ctrl+Z), full screen (top right).

## 10. New in 0.4

- **GROOVE** (top left) opens the **browser**: a genre on the left, a groove on the right (click
  to load it, arrows for the next one).
- **IMPORT...**: the drums of a MIDI file, or an **audio drum loop** turned into the grid. You can
  also drop the file on the window (on a drum's row or pad an audio file becomes that drum's sample).
- **ACCENTS** (GROOVE FEEL): the hats' accent groove. **VOLUME** (KIT & SOUNDS) and **MIXER** (next to
  VELOCITY): the volume of each drum in the guide sound.
- The kick never leaves the grid.

## 11. Language

**OPTIONS → Language**: English, Italiano, Español, Français, Deutsch or Português. The interface changes at once; the choice is kept for every project.

## 12. Part length

**PART LENGTH** (GROOVE row): *1/2 bar* = parts A-D of half a bar, a 2-bar groove; *1 bar* = parts of one bar, a 4-bar groove (the grid shows 4 bars).

## 13. New in 0.8

- **Swing** goes up to the heaviest shuffle and reads as on a drum machine: 50% straight, 67% a
  triplet shuffle, 75% the heaviest. Every genre has its own swing (Jackin' 70%, Garage about 63%...).
- **MIXER**: every drum has **PUSH** (milliseconds ahead of the beat, −, or laid back, +) and
  **SWING** (how much of the swing it takes: 0% straight, 100% as the Swing knob, 200% double).
  **TIMING...** sets ready-made feels (laid-back snare & clap, pushed hats, jackin'...). The kick
  always stays exactly on the grid.
- **TEMPLATE...**: a groove template takes the timing and the accents of every 16th from an audio
  loop or a MIDI file and puts them on any groove (instead of the swing and the accent groove).
  Switch it on / off or remove it from the same menu.
- **Chance and roll** (right-click a step): a hit can play always or 90%...10% of the time (the
  same way each time round; NEW FEEL changes it), and play as a quick roll of 2, 3 or 4 hits.
  Never on the kick. A cut corner marks a chance, small ticks a roll.
- **FILL** view: draw your own fill (one bar). It takes over from its first written step; the
  drums you leave empty keep playing the groove. **START FROM...** copies a fill style to begin
  with. Drawing sets the fill style to **Custom**. New fill styles: Kick build, Rim shuffle, Open
  hat run, Clap stutter, Percussion break.
- **KIT FOLDER...** (Guide sound): pick a folder of samples, or drop it on the window: each drum
  gets the sample whose name says it (kick / bd, snare / sd, clap, rim, closed hat / ch, open hat /
  oh, ride, crash, shaker, tambourine, conga / perc, cowbell, tom).
- **SAVE .WAV...** (Export): the guide sound (built-in kit or your samples) as a WAV: this clip
  (it loops seamlessly: the tails ring into the start), the whole song, or one WAV per drum (stems).
- **LINK PIANO** (Groove row): the genre and the swing follow Fase Groove Piano (2.18 or later,
  with its **LINK DRUMS** on) and the other linked Fase Groove instances: change them in either
  one. A groove of another genre brings the first groove of that genre in the other plug-in.
