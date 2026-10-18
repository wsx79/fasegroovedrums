# Fase Groove Drums – Guida

*🇬🇧 [English guide](GUIDE.md)*

## 1. Installazione

Scarica lo zip per il tuo sistema dalla pagina
[Releases](https://github.com/wsx79/fasegroovedrums/releases/latest) ed estrailo.

| Sistema | Copia | In |
|---|---|---|
| Windows | `Fase Groove Drums.vst3` | `C:\Program Files\Common Files\VST3\` |
| macOS | `Fase Groove Drums.vst3` / `Fase Groove Drums.component` | `~/Library/Audio/Plug-Ins/VST3/` / `~/Library/Audio/Plug-Ins/Components/` |
| Linux | `Fase Groove Drums.vst3` | `~/.vst3/` |

Poi fai ripetere alla DAW la ricerca dei plug-in. Su macOS, se il plug-in viene bloccato, esegui
una volta nel Terminale: `xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Fase Groove Drums.vst3"` (e lo
stesso per il `.component`). L'app **Standalone** funziona senza DAW.

Caricalo come **strumento** su una traccia MIDI. Premi play nella DAW (o **PLAY** nel plug-in) e
il groove suona con il kit guida.

## 2. Il groove

- **GROOVE:** 53 groove in 13 generi (gli stessi di Fase Groove Piano). **Random beat** è il beat
  di NEW BEAT, **User pattern** quello modificato sulla griglia.
- **NEW BEAT:** un nuovo beat casuale nel genere scelto in **NEW BEAT GENRE**. **DENSITY** va da
  cassa-clap-hat essenziali a 16esimi pieni con percussioni.
- **STYLE** (in alto): groove, swing, feel, fill e kit di un genere in un clic. **RESET** riporta
  tutto ai valori iniziali.

## 3. La griglia

13 strumenti (cassa, rullante, clap, rim, hat chiuso, hat aperto, ride, shaker, tamburello,
conga, campanaccio, tom, crash) × 32 step (due battute di 16esimi).

| Azione | Risultato |
|---|---|
| Clic | colpo (di nuovo: pausa) |
| Shift + clic | accento |
| Alt + clic | ghost note (molto piano) |
| Cmd/Ctrl + clic | flam (un colpetto appena prima) |
| Trascina | disegna lo stesso simbolo sugli altri step |
| Tasto destro su uno step | menu: accento, colpo, ghost, flam, pausa |
| Clic sul nome di uno strumento | muto / attivo |
| Tasto destro sul nome | muto, svuota la riga, **nota MIDI** (nomi General MIDI o qualsiasi nota), **livello** |

La prima modifica copia il groove mostrato nello **User pattern**, che si salva con il progetto.
**CLEAR** svuota la griglia. La luce accanto a ogni nome lampeggia quando lo strumento suona.

## 4. Feel

- **SWING:** sposta in avanti un 16esimo sì e uno no (0% dritto, 100% shuffle pieno).
- **HUMANIZE:** piccole variazioni di tempo e velocity, come un batterista; la cassa resta precisa.
  **NEW FEEL** dà un'altra umanizzazione (sempre la stessa quando la riascolti).
- **VELOCITY:** velocity generale.

## 5. Brano

- **SECTION:** Main, Intro / Outro (cassa e hat), Break (senza cassa), Build-up (rullata che
  raddoppia, in crescendo), Drop (il groove pieno con un crash ogni 8 battute). Automatizzabile.
- **FILLS / FILL STYLE:** un fill alla fine di ogni 1, 2, 4 o 8 battute: rullata, giro di tom,
  clap in crescendo, hat aperto, stop, giro di conga o mix. **CRASH AFTER FILLS** aggiunge un crash
  sulla battuta dopo.
- **VARIATION:** ogni 4 o 8 battute l'ultima battuta varia (un hat aperto, una cassa in levare, una ghost...).

## 6. Suono e MIDI

- **GUIDE KIT:** House 909, Deep 808, Disco Live, Lo-Fi Soul, tutti sintetizzati; **TONE**,
  **REVERB**, **VOLUME**. **SOUND** (in alto) spegne il kit guida: solo MIDI, per la tua batteria.
- **MIDI CH.:** il canale delle note (10 di default). Le note sono General MIDI: cassa 36,
  rullante 38, clap 39, rim 37, hat chiuso 42, hat aperto 46, ride 51, shaker 82, tamburello 54,
  conga 63, campanaccio 56, tom 45, crash 49. Puoi cambiarle sulla griglia (tasto destro sul nome).
- **SEQUENCER:** spento, il groove non suona con la DAW (ad esempio mentre su questa traccia suona
  una clip trascinata); il MIDI che arriva sulla traccia suona comunque il kit guida.
- **SYNC DAW:** acceso, tempo e posizione della DAW; spento, il tempo impostato accanto.

## 7. Export

- **LENGTH:** Loop (finché groove, fill e variazioni ricominciano), 1–16 battute o **Whole song**
  (Intro 8, Main 16, Break 8, Build-up 4, Drop 16, Outro 8 battute, con i marker).
- **ONE TRACK PER DRUM:** un file MIDI con una traccia per ogni strumento.
- **DRAG MIDI TO YOUR DAW:** trascina il riquadro su una traccia. Le clip restano anche in
  `Documenti/Fase Groove Drums/MIDI clips/`.
- **SAVE .MID:** salva la clip dove vuoi.

## 8. Scorciatoie

- **Barra spaziatrice:** play / pausa senza DAW.
- Clicca il **logo** o **ABOUT** per crediti, link e donazioni.
