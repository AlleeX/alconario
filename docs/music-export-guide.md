# Music Export Guide 🎵

How to get music from FamiStudio (`.fms`) into the Alconario ROM.

---

## Current Setup

| File | Purpose |
| --- | --- |
| `assets/music/title.fms` | FamiStudio project — title screen music |
| `assets/music/nes_theme.txt` | FamiStudio text export (reference) |
| `lib/neslib/music.sinc` | **FamiTone2 assembly data** — included in ROM |
| `lib/neslib/famitone2.sinc` | FamiTone2 engine (auto-updated every NMI) |

---

## Export Steps

### 1. Open in FamiStudio

Open `assets/music/title.fms` in [FamiStudio](https://famistudio.org/).

### 2. Export to FamiTone2

1. **File** → **Export** → **FamiTone2 Music**
2. Settings:
   - **Format**: `CA65 assembly`
   - **Separate files**: unchecked (single file)
   - **Generate DPCM**: unchecked (unless using samples)
3. **Save as**: `lib/neslib/music.sinc`

> ⚠️ The file **must** be named `music.sinc` and placed in `lib/neslib/` — `crt0.s` includes it directly.

### 3. Rebuild

```bash
make clean && make
```

### 4. Test

```bash
make run
```

You should hear the title music when the game starts. It plays in `enter_title()` via `music_play(0)`.

---

## C API Reference

These functions are declared in `neslib.h`:

```c
// Play song by index (0-based, order from FamiStudio export)
void __fastcall__ music_play(unsigned char song);

// Stop all music
void __fastcall__ music_stop(void);

// Pause (1) or unpause (0) music
void __fastcall__ music_pause(unsigned char pause);
```

### Where they're called in game.c

| Function | Called in | Purpose |
| --- | --- | --- |
| `music_play(0)` | `enter_title()` | Start title music |
| `music_stop()` | `enter_play()` | Stop music when gameplay starts |

### Adding more songs

When you export multiple songs from FamiStudio, they get sequential indices:

```c
#define MUSIC_TITLE      0
#define MUSIC_OVERWORLD  1
#define MUSIC_UNDERGROUND 2
// etc.

music_play(MUSIC_TITLE);
music_play(MUSIC_OVERWORLD);
```

---

## FamiStudio CLI Export (Alternative)

If FamiStudio CLI is installed:

```bash
famistudio assets/music/title.fms famitone2-export lib/neslib/music.sinc -famitone2-format:ca65
```

---

## SFX (Future)

Sound effects follow a similar flow:

1. Create SFX in FamiStudio (separate project or same `.fms`)
2. Export → FamiTone2 SFX → `lib/neslib/sounds.sinc`
3. Uncomment `; .include "sounds.sinc"` in `crt0.s`
4. Call `sfx_play(SFX_ID, channel)` from C

---

## Architecture

```
title.fms (FamiStudio project)
    │
    ▼  (Export → FamiTone2 Music → CA65)
lib/neslib/music.sinc (assembly data)
    │
    ▼  (.include in crt0.s → RODATA segment)
crt0.s → crt0.o
    │
    ▼  (ld65 links into ROM)
alconario.nes
    │
    ▼  (at runtime)
FamiToneInit (called once in crt0.s startup)
FamiToneUpdate (called every NMI automatically)
music_play(0) (called from game.c enter_title)
```
