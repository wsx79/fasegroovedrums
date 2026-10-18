# Novità

*🇬🇧 [English version](CHANGELOG.md)*

Tutte le modifiche importanti di **Fase Groove Drums**.

## [0.5.0] - 2026-10-18

- **Interfaccia in 6 lingue**, come in Fase Groove Piano: English, Italiano, Español, Français,
  Deutsch, Português. Si sceglie in **OPZIONI → Lingua**: cambia tutto subito e la scelta vale per
  ogni progetto (la prima volta si usa la lingua del computer).
- Tradotti pulsanti, didascalie, menu, suggerimenti, finestra del brano, browser, mixer, messaggi e
  le impostazioni audio dell'app Standalone; i nomi dei groove e dei generi restano com'erano.

## [0.4.0] - 2026-10-18

- **Browser dei groove**: niente più menu a scorrimento. Generi a sinistra, i loro groove a destra,
  ognuno con il disegno del ritmo (hat, rullante/clap, cassa); un clic lo carica, le frecce li
  scorrono, un dado crea un nuovo beat nel genere.
- **Loop audio → griglia** (IMPORT... → audio, o trascina un loop sulla finestra): cassa, clap, hat
  chiusi e aperti, con le velocity e lo swing del loop.
- **La cassa è sempre sulla griglia**: niente swing, humanize o flam sulla cassa, mai.
- **ACCENTS**: hat, shaker, ride e tamburello più forti in levare e più leggeri sui 16esimi, come
  un vero batterista (0% = piatto).
- **VOLUME** per strumento (pannello kit) e un **MIXER** (un fader e un muto per ogni strumento del
  suono guida e dei tuoi campioni). LEVEL cambia ancora le velocity MIDI.
- Le velocity impostate a mano ora restano anche nella battuta di variazione e sotto i fill
  (prima cambiavano a fine battuta).
- **Beat casuali rifatti** per ruoli, come lavora un produttore: cassa, backbeat, una linea di hat
  principale, al massimo un secondo hat, uno shaker o un tamburello, al massimo una percussione;
  la battuta 2 ripete la 1.
- Swing per genere e forma "Offbeat push" più vicini ai veri loop house.

## [0.3.0] - 2026-10-18

La stessa interfaccia di Fase Groove Piano, adattata alla batteria.

- **Intestazione** come nel piano: PRESETS, TOTAL (casuale), annulla / ripeti, schermo intero;
  STYLE, RESET, OPTIONS; un riquadro TRANSPORT & TEMPO (⏮ ▶ ■, interruttore Sequencer, SYNC DAW,
  BPM) e un grande display con il groove, il genere, la sezione e la posizione.
- **Riga GROOVE** (la progressione del piano): preset dei groove, RANDOM, genere, densità e le quattro
  **schede delle parti A-D** (al posto degli slot degli accordi), con il disegno dei colpi, dado e lucchetto.
- **KIT & SOUNDS**: lo strumento scelto, la sua nota MIDI, livello, muto, il suo dado e il tuo campione.
- **DRUM PADS** (al posto del circolo delle quinte): 13 pad che si accendono sui colpi; clic per
  sentirlo e sceglierlo, tasto destro per il muto.
- Pannelli **GROOVE FEEL & SONG** e **GUIDE SOUND & MIDI OUT**.
- **STEP GRID / VELOCITY** in basso ed **EXPORT MIDI** con il grande riquadro arancione DRAG MIDI
  TO DAW, SAVE .MID... e FULL SONG...
- Nella griglia, il clic sul nome di uno strumento ora lo sceglie (muto: pannello kit, pad o tasto destro).

## [0.2.0] - 2026-10-18

L'interfaccia di Fase Groove Piano, per la batteria.

- **Dadi ovunque:** TOTAL RANDOM in alto, NEW BEAT, un dado su **ogni strumento** (una nuova linea solo
  per quello strumento, nel genere scelto), un dado e un lucchetto su ogni parte A-D.
- **Parti A-D:** le quattro mezze battute del groove in qualsiasi ordine (es. A A A B), ognuna con dado e lucchetto.
- **Vista VELOCITY:** un piano roll per le velocity, uno strumento alla volta (scelto da un menu): i
  colpi sopra, una barra per colpo da trascinare; forme (offbeat push, on the beat, shuffle 16ths,
  crescendo, humanize, flat), un dado per le velocity, ritorno all'automatico. Doppio clic sul nome di
  uno strumento per aprirla.
- **I tuoi campioni:** carica un WAV / AIFF / FLAC / MP3 per qualsiasi strumento (tasto destro sul nome,
  o trascina il file sulla sua riga); salvato con il progetto.
- **Finestra del brano (SONG):** battute di ogni sezione, sua variazione, suo fill; trascina il brano
  intero, una sezione o tutte le sezioni come clip separate; salva il brano o un .mid per sezione.
- **Trasporto** ⏮ ▶/❚❚ ■, **annulla / ripeti** (Ctrl+Z / Ctrl+Shift+Z), **schermo intero**.
- **PRESETS** (salva / carica tutto) e **MY GROOVES** (salva / carica la griglia, con le velocity).
- **Importa la batteria di un file MIDI** (MY GROOVES, o trascina il .mid sulla finestra): due battute
  sulla griglia, con le loro velocity.
- ◀ ▶ per scorrere i groove; un segno su ogni cella con velocity impostata a mano.

## [0.1.0] - 2026-10-18

La prima versione: il compagno di batteria di Fase Groove Piano.

- 53 groove di batteria nei 13 generi di Fase Groove Piano (Piano House, Soulful, Deep, Gospel,
  Jackin', Jackin' Piano, Garage & UK, Afro & Latin, Funky & Disco, '90s Piano House,
  Neo Soul & Broken, Chill & Pads, Dance Stabs).
- NEW BEAT: beat casuali per genere, con Density.
- STYLE: impostazioni per genere in un clic; RESET.
- Griglia modificabile: 13 strumenti × 32 step con accenti, ghost note e flam; muto, nota MIDI e
  livello per strumento.
- Swing, humanize e NEW FEEL.
- Fill (7 stili) ogni 1, 2, 4 o 8 battute, crash dopo i fill, variazioni ogni 4 o 8 battute.
- Sezioni del brano (Intro, Main, Break, Build-up, Drop, Outro) ed export del brano intero con i marker.
- Quattro kit guida sintetizzati (House 909, Deep 808, Disco Live, Lo-Fi Soul) con tono,
  riverbero e volume.
- Clip MIDI con drag and drop o file `.mid` (una traccia o una traccia per strumento), uscita MIDI
  dal vivo sul canale 10 (o qualsiasi canale), sync con la DAW o tempo manuale, anteprima senza DAW.
