# Music & Sound Effects Strategy 🎵

Audio design plan for Alconario — a Super Mario Bros–style NES platformer.

---

## NES Audio Hardware

| Channel | Type | Range | Notes |
| --- | --- | --- | --- |
| Pulse 1 | Square wave (4 duty cycles) | ~54 Hz – 12.4 kHz | Melody, leads |
| Pulse 2 | Square wave (4 duty cycles) | ~54 Hz – 12.4 kHz | Harmony, counter-melody |
| Triangle | Triangle wave (no volume control) | ~27 Hz – 55.9 kHz | Bass, smooth leads |
| Noise | Pseudo-random noise (short/long mode) | 16 frequency presets | Percussion, hi-hats, snares |
| DPCM | 1-bit delta-encoded samples | 16 sample rates (4.2–33.1 kHz) | Sampled drums, voice clips |

### Constraints

- **5 channels total** — shared between music and SFX.
- SFX typically steal Pulse 2 and/or Noise during playback.
- DPCM samples live in **PRG-ROM** (`$C000`–`$FFFF` on NROM) — each sample costs ROM space.
- No hardware mixing — you get what the 5 channels give you.

---

## Toolchain

We use **FamiStudio** for composition and its built-in FamiTone2 exporter for conversion.

```
FamiStudio (.fms)
       │
       ▼  File → Export → FamiTone2 Music (CA65)
assets/music/*.s (raw export)
       │
       ▼  Strip .export/.global lines
src/asm/music.sinc (clean include)
       │
       ▼  .include in crt0.s (via -I src/asm)
Linked into ROM via ld65
```

> See [Music Export Guide](music-export-guide.md) for step-by-step instructions.

---

## Music Tracks Plan

| Track | Game State | Tempo | Feel | Status |
| --- | --- | --- | --- | --- |
| `title` (Kasiu Yas) | Title screen | 112 BPM | Bouncy, inviting | ✅ Done |
| `overworld` | Main levels (outdoor) | ~140 BPM | Energetic, adventurous | ⬜ TODO |
| `underground` | Underground / pipe areas | ~100 BPM | Mysterious, echoey | ⬜ TODO |
| `water` | Water levels | ~90 BPM | Flowing, waltz-like (3/4) | ⬜ TODO |
| `castle` | Castle / boss approach | ~130 BPM | Tense, dark | ⬜ TODO |
| `boss` | Boss fight | ~160 BPM | Intense, driving | ⬜ TODO |
| `star` | Invincibility | ~180 BPM | Fast, triumphant | ⬜ TODO |
| `hurry` | Timer low (<100) | Faster variant of current | Speed up current track | ⬜ TODO |
| `gameover` | Game over | ~80 BPM | Short, melancholic | ⬜ TODO |
| `victory` | Level clear | — | Fanfare, 3–4 seconds | ⬜ TODO |
| `world_clear` | World clear / castle | — | Longer fanfare | ⬜ TODO |

### Music design guidelines

- Keep melodies **simple and memorable** — 4–8 bar loops work great for NES.
- Use **echo effects** by delaying Pulse 2 by 1–2 rows behind Pulse 1.
- Bass on Triangle — it has no volume control, so use note length for dynamics.
- Reserve DPCM for **kick drums** only (saves ROM space).

---

## Sound Effects Plan

| SFX ID | Name | Priority | Channel(s) | Duration | Description |
| --- | --- | --- | --- | --- | --- |
| 0 | `sfx_jump` | High | Pulse 2 | ~0.1s | Rising pitch sweep |
| 1 | `sfx_coin` | High | Pulse 2 | ~0.15s | Two quick high notes |
| 2 | `sfx_stomp` | High | Noise | ~0.1s | Short thud |
| 3 | `sfx_bump` | Medium | Noise | ~0.1s | Head bump on block |
| 4 | `sfx_powerup` | High | Pulse 2 | ~0.5s | Ascending arpeggio |
| 5 | `sfx_powerdown` | High | Pulse 2 | ~0.3s | Descending wah |
| 6 | `sfx_fireball` | Medium | Noise | ~0.15s | Quick burst |
| 7 | `sfx_kick` | Medium | Noise | ~0.1s | Shell kick |
| 8 | `sfx_break` | Medium | Noise | ~0.2s | Brick breaking |
| 9 | `sfx_1up` | High | Pulse 2 | ~0.3s | Cheerful jingle |
| 10 | `sfx_pipe` | Low | Noise | ~0.3s | Descending noise |
| 11 | `sfx_death` | Critical | Pulse 1+2 | ~1.5s | Stops music, plays death jingle |
| 12 | `sfx_flag` | High | Pulse 2 | ~0.2s | Flag slide sound |
| 13 | `sfx_timer_tick` | Low | Pulse 2 | ~0.05s | Timer counting at level end |

### SFX priority system

When multiple SFX fire simultaneously, higher priority wins. `sfx_death` is special — it **stops the music engine** and plays a standalone jingle.

---

## Runtime API (neslib)

These functions are declared in `neslib.h` and backed by FamiTone2:

```c
/* Music — initialized automatically by crt0.s at boot */
void __fastcall__ music_play(unsigned char song);   /* play by index */
void __fastcall__ music_stop(void);                 /* stop */
void __fastcall__ music_pause(unsigned char pause);  /* 1=pause, 0=resume */

/* SFX — requires sounds.sinc to be included in crt0.s */
void __fastcall__ sfx_play(unsigned char sfx, unsigned char channel);
/* channel: 0=Pulse1, 1=Pulse2, 2=Triangle, 3=Noise */
```

### Current usage in game.c

```c
music_play(0);   /* enter_title() — play "Kasiu Yas Title" */
music_stop();    /* enter_play()  — silence when gameplay starts */
```

---

## File Organization

```
assets/music/
├── title.fms              # FamiStudio project (original title theme)
├── title2.s               # FamiTone2 export — active title music
├── fami-title.txt         # FamiStudio text format (human-readable)
├── nes_theme.txt          # Earlier theme (reference)
├── overworld.fms          # TODO
├── underground.fms        # TODO
├── castle.fms             # TODO
├── boss.fms               # TODO
├── gameover.fms           # TODO
├── sfx.fms                # TODO: all SFX in one project
└── README.md              # Composition notes

src/asm/
├── music.sinc             # Active music data (included in ROM by crt0.s)
├── sounds.sinc            # TODO: SFX data (included in ROM by crt0.s)
├── chr.s                  # CHR-ROM tile embedding
└── nes_header.s           # iNES header
```

---

## Related Docs

- [Music Export Guide](music-export-guide.md) — step-by-step export & integration workflow
- [Asset Pipeline](asset-pipeline.md) — full build flow
- [NES Glossary](nes-glossary.md) — APU channel definitions
