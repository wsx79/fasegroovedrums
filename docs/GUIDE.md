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
| Click a drum's name | mute / unmute |
| Right-click a drum's name | mute, clear the row, **MIDI note** (General MIDI names or any note), **level** |

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
