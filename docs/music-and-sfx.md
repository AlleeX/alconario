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

```
FamiTracker (.ftm)
       │
       ▼
FamiTracker Text Export (.txt)
       │
       ▼
text2data (famitone2)
       │
       ▼
ca65-compatible .s file
       │
       ▼
Linked into ROM via ld65
```

### Commands

```bash
# Music
text2data assets/music/overworld.txt -ca65 -o src/asm/music_overworld.s

# SFX (single file with all effects)
nsf2data assets/music/sfx.nsf -ca65 -o src/asm/sfx_data.s
```

---

## Music Tracks Plan

| Track | Game State | Tempo | Feel | Channels |
| --- | --- | --- | --- | --- |
| `title` | Title screen | ~120 BPM | Bouncy, inviting | All 5 |
| `overworld` | Main levels (outdoor) | ~140 BPM | Energetic, adventurous | All 5 |
| `underground` | Underground / pipe areas | ~100 BPM | Mysterious, echoey | P1, P2, Tri, Noise |
| `water` | Water levels | ~90 BPM | Flowing, waltz-like (3/4) | P1, P2, Tri |
| `castle` | Castle / boss approach | ~130 BPM | Tense, dark | All 5 |
| `boss` | Boss fight | ~160 BPM | Intense, driving | All 5 |
| `star` | Invincibility | ~180 BPM | Fast, triumphant | P1, P2, Tri |
| `hurry` | Timer low (<100) | Faster variant of current | Speed up current track | — |
| `gameover` | Game over | ~80 BPM | Short, melancholic | P1, Tri |
| `victory` | Level clear | — | Fanfare, 3–4 seconds | All 5 |
| `world_clear` | World clear / castle | — | Longer fanfare | All 5 |

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

## Runtime API (famitone2 + neslib)

```c
// Initialize music engine (call once at startup)
famitone2_init(music_data);
sfx_init(sfx_data);

// Play music track
famitone2_music_play(MUSIC_OVERWORLD);

// Stop music
famitone2_music_stop();

// Play SFX (channel: 0=Pulse1, 1=Pulse2, 2=Triangle, 3=Noise)
famitone2_sfx_play(SFX_JUMP, SFX_CH1);  // Play jump on Pulse 2

// Call every frame in NMI/update
famitone2_update();
```

---

## File Organization

```
assets/music/
├── overworld.ftm          # FamiTracker project files
├── underground.ftm
├── castle.ftm
├── boss.ftm
├── title.ftm
├── gameover.ftm
├── victory.ftm
├── star.ftm
├── sfx.ftm                # All SFX in one file (separate instruments)
└── README.md              # Notes on tempo, key, design choices
```

Generated files (in `src/asm/`):

```
src/asm/
├── music_overworld.s
├── music_underground.s
├── music_castle.s
├── ...
└── sfx_data.s
```

---

## Related Docs

- [Asset Pipeline](asset-pipeline.md) — full build flow
- [NES Glossary](nes-glossary.md) — APU channel definitions
