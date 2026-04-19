# Music & Sound Export Guide 🎵

How to create, export, and integrate music and sound effects into Alconario.

---

## Current Setup

| File | Purpose |
| --- | --- |
| `assets/music/title.fms` | FamiStudio project — title screen music (original) |
| `assets/music/title2.s` | FamiStudio FamiTone2 export — **active** title music ("Kasiu Yas Title") |
| `assets/music/fami-title.txt` | FamiStudio text format (human-readable reference) |
| `assets/music/nes_theme.txt` | FamiStudio text format (earlier theme, reference) |
| `src/asm/music.sinc` | **FamiTone2 data included in ROM** — stripped from `title2.s` |
| `lib/neslib/famitone2.sinc` | FamiTone2 engine (auto-updated every NMI) |
| `lib/neslib/crt0.s` | Startup code — `.include "music.sinc"` pulls data from `src/asm/` |

### Key convention

- **Source exports** go in `assets/music/` (`.fms`, `.ftm`, `.txt`, `.s`)
- **ROM-ready include** goes in `src/asm/music.sinc` (stripped of `.export`/`.global`)
- `lib/neslib/` is **vendored and should not contain music data files**
- `crt0.s` finds `music.sinc` via the `-I src/asm` assembler flag in the Makefile

---

## End-to-End Workflow

### Step 1 — Compose in FamiStudio

1. Download [FamiStudio](https://famistudio.org/) (free, runs on Mac/Win/Linux).
2. Create a new project:
   - **Tempo Mode**: FamiTracker
   - **Machine**: NTSC
3. Compose your track using up to 5 channels:

   | Channel | Typical use | Notes |
   | --- | --- | --- |
   | Square 1 | Melody / lead | 4 duty cycle options |
   | Square 2 | Harmony / echo | Delay Square 1 by 1–2 rows for echo |
   | Triangle | Bass line | No volume control — use note length |
   | Noise | Percussion | Short bursts for drums |
   | DPCM | Sampled drums | Uses PRG-ROM space — use sparingly |

4. Save as `.fms` in `assets/music/`.

### Step 2 — Export to FamiTone2

1. In FamiStudio: **File → Export Other → FamiTone2 Music**
2. Settings:
   - **Format**: `CA65 assembly`
   - **Separate files**: unchecked
   - **Generate DPCM**: unchecked (unless using samples)
3. Save the `.s` file in `assets/music/` (e.g. `title2.s`, `overworld.s`)

### Step 3 — Convert to music.sinc

The exported `.s` file contains `.export` and `.global` directives that conflict with `crt0.s`. Strip them:

**Manual method** — copy the file and remove these lines:
```
.export _music_data_...:=music_data_...
music_data_...:            ← remove this label (crt0.s provides music_data:)
.export music_data_...
.global FAMISTUDIO_DPCM_PTR
```

Keep everything from the first `.byte` line onwards.

**Quick method** — use sed:

```bash
sed -E '/^\.(export|global)/d; /^music_data_/d' assets/music/title2.s > src/asm/music.sinc
```

Then add a header comment to `src/asm/music.sinc`:

```ca65
; music.sinc — FamiTone2 music data
; Source: assets/music/title2.s
; Auto-included by crt0.s via -I src/asm
```

### Step 4 — Build and test

```bash
make clean && make
make run
```

The title screen calls `music_play(0)` in `enter_title()` — you should hear the music immediately.

---

## Adding Multiple Songs

When you export a FamiStudio project with multiple songs, they get sequential indices (0, 1, 2…).

### Option A — Multiple songs in one project

1. Add songs in FamiStudio (Song → Add Song).
2. Export — all songs end up in one `.s` file.
3. Convert to `music.sinc` as above.
4. Reference by index in C:

```c
/* Define song indices (match export order in FamiStudio) */
#define MUSIC_TITLE       0
#define MUSIC_OVERWORLD   1
#define MUSIC_UNDERGROUND 2
#define MUSIC_CASTLE      3
#define MUSIC_BOSS        4
#define MUSIC_GAMEOVER    5
#define MUSIC_VICTORY     6
#define MUSIC_STAR        7

/* Play in game.c */
music_play(MUSIC_TITLE);      /* title screen */
music_play(MUSIC_OVERWORLD);  /* level start */
music_stop();                 /* silence */
```

### Option B — Separate projects per track

Export each `.fms` separately, then manually concatenate the data in `music.sinc`. This is harder — **Option A is recommended**.

---

## Planned Music Tracks

| Index | Song | BPM | Game state | Status |
| --- | --- | --- | --- | --- |
| 0 | Kasiu Yas Title | 112 | Title screen | ✅ Done |
| 1 | Overworld | ~140 | Outdoor levels | ⬜ TODO |
| 2 | Underground | ~100 | Pipe / cave areas | ⬜ TODO |
| 3 | Castle | ~130 | Boss approach | ⬜ TODO |
| 4 | Boss | ~160 | Boss fight | ⬜ TODO |
| 5 | Game Over | ~80 | Death screen | ⬜ TODO |
| 6 | Victory | — | Level clear fanfare | ⬜ TODO |
| 7 | Star | ~180 | Invincibility | ⬜ TODO |

---

## Sound Effects (SFX)

### Creating SFX

1. In FamiStudio, create a new project (or add SFX as instruments in the music project).
2. SFX are typically short single-channel patterns.
3. Export: **File → Export Other → FamiTone2 SFX**
   - Format: CA65
   - Save as: `assets/music/sfx.s`

### Integrating SFX

1. Strip `.export`/`.global` from `sfx.s` and save as `src/asm/sounds.sinc`.
2. Uncomment in `lib/neslib/crt0.s`:

   ```ca65
   sounds_data:
    .include "sounds.sinc"    ; ← uncomment this line
   ```

3. Call from C:

   ```c
   /* SFX channels (which APU channel to steal) */
   #define SFX_CH0  0  /* Pulse 1 */
   #define SFX_CH1  1  /* Pulse 2 */
   #define SFX_CH2  2  /* Triangle */
   #define SFX_CH3  3  /* Noise */

   /* Play sound effect */
   sfx_play(SFX_JUMP, SFX_CH1);   /* jump on Pulse 2 */
   sfx_play(SFX_COIN, SFX_CH1);   /* coin on Pulse 2 */
   sfx_play(SFX_STOMP, SFX_CH3);  /* stomp on Noise */
   ```

### Planned SFX

| ID | Name | Channel | Duration | Description |
| --- | --- | --- | --- | --- |
| 0 | `SFX_JUMP` | Pulse 2 | ~0.1s | Rising pitch sweep |
| 1 | `SFX_COIN` | Pulse 2 | ~0.15s | Two quick high notes |
| 2 | `SFX_STOMP` | Noise | ~0.1s | Short thud |
| 3 | `SFX_BUMP` | Noise | ~0.1s | Head bump on block |
| 4 | `SFX_POWERUP` | Pulse 2 | ~0.5s | Ascending arpeggio |
| 5 | `SFX_POWERDOWN` | Pulse 2 | ~0.3s | Descending wah |
| 6 | `SFX_FIREBALL` | Noise | ~0.15s | Quick burst |
| 7 | `SFX_1UP` | Pulse 2 | ~0.3s | Cheerful jingle |
| 8 | `SFX_DEATH` | Pulse 1+2 | ~1.5s | Stops music, plays jingle |
| 9 | `SFX_PIPE` | Noise | ~0.3s | Descending noise |
| 10 | `SFX_FLAG` | Pulse 2 | ~0.2s | Flag slide |
| 11 | `SFX_BREAK` | Noise | ~0.2s | Brick breaking |

---

## C API Reference

Declared in `neslib.h`:

```c
void __fastcall__ music_play(unsigned char song);   /* play song by index */
void __fastcall__ music_stop(void);                 /* stop music */
void __fastcall__ music_pause(unsigned char pause);  /* 1=pause, 0=resume */
void __fastcall__ sfx_play(unsigned char sfx, unsigned char channel); /* play SFX */
```

### Current usage in game.c

| Call | Location | Purpose |
| --- | --- | --- |
| `music_play(0)` | `enter_title()` | Start title music |
| `music_stop()` | `enter_play()` | Stop music when gameplay starts |

---

## Architecture

```
FamiStudio (.fms)
    │
    ▼  File → Export → FamiTone2 Music (CA65)
assets/music/title2.s (raw export)
    │
    ▼  Strip .export/.global, remove label
src/asm/music.sinc (clean include)
    │
    ▼  .include "music.sinc" in crt0.s (via -I src/asm)
lib/neslib/crt0.s → crt0.o
    │
    ▼  ld65 links into ROM
build/alconario.nes
    │
    ▼  At runtime:
    FamiToneInit()    — called once at startup (crt0.s)
    FamiToneUpdate()  — called every NMI (neslib.sinc)
    music_play(0)     — called from game.c enter_title()
    music_stop()      — called from game.c enter_play()
```

---

## File Organization

```
assets/music/
├── title.fms              # FamiStudio project (original)
├── title2.s               # FamiTone2 export (active title music)
├── fami-title.txt         # FamiStudio text format (reference)
├── nes_theme.txt          # Earlier theme (reference)
├── overworld.fms          # TODO: overworld music
├── sfx.fms                # TODO: sound effects
└── README.md              # Composition notes

src/asm/
├── music.sinc             # Active music data (included in ROM)
├── sounds.sinc            # TODO: SFX data (included in ROM)
├── chr.s                  # CHR-ROM tile embedding
└── nes_header.s           # iNES header
```

---

## Troubleshooting

| Problem | Cause | Fix |
| --- | --- | --- |
| No sound at all | `music.sinc` not included | Check `crt0.s` has `.include "music.sinc"` uncommented |
| Old music plays | `lib/neslib/music.sinc` exists | Delete it — it shadows `src/asm/music.sinc` |
| CPU crash on boot | Invalid FamiTone2 data | Re-export from FamiStudio; ensure `.export`/`.global` lines are stripped |
| Music doesn't loop | Missing `$fd` + loop address | Check export — FamiStudio adds this automatically |
| SFX cuts music | SFX and music on same channel | Use different channels for SFX vs melody |

---

## Tips for Composing NES Music

- **Keep it simple** — 4–8 bar loops sound great on NES
- **Echo trick** — delay Square 2 by 1–2 rows behind Square 1 at lower volume
- **Bass** — Triangle channel has no volume control; use note duration for dynamics
- **Percussion** — Noise channel: high frequencies for hi-hat, low for kick
- **Test often** — export and build after every major change
- **ROM budget** — each song is roughly 200–500 bytes; NROM has ~26 KiB total for everything
