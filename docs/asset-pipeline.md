# Asset Pipeline 🛠️

How raw art, music, and level data flow from creation tools into the final `.nes` ROM.

---

## Overview

```
 Artist / Designer                    Build System
 ─────────────────                    ────────────
 .png / .bmp (indexed)  ──► gen_chr.py ──► tiles.chr ──► chr.s ──► chr.o ─┐
 .pal (hex list)        ──► palette_gen ──► palette.h ──────────────────── │
 .ftm (FamiTracker)     ──► text2data  ──► music.s   ──► music.o ──────── ├──► ld65 ──► alconario.nes
 .json / .csv (levels)  ──► lvl_pack   ──► levels.s  ──► levels.o ─────── │
 C sources (.c)         ──► cc65 ──► ca65 ──► .o ──────────────────────── ┘
```

---

## 1. Tiles (CHR)

### Source format
- **Indexed PNG**, 128×128 px (= 16×16 tiles = 256 tiles, one pattern table).
- Exactly **4 colors** per tile (indices 0–3); color 0 = transparent/background.
- Two PNGs: one for **background** tiles (`bg_tiles.png`), one for **sprite** tiles (`spr_tiles.png`).

### Conversion
```bash
python3 tools/gen_chr.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png -o assets/chr/tiles.chr
```
`gen_chr.py` reads pixels, packs each 8×8 tile into the NES 2-bit-planar format (16 bytes/tile), and concatenates both pattern tables into a single 8 KiB `.chr` file.

### Embedding
`src/asm/chr.s` includes the binary:
```asm
.segment "CHARS"
.incbin "../assets/chr/tiles.chr"
```
The linker places `CHARS` at the CHR-ROM address.

### Guidelines
| Rule | Why |
|---|---|
| Max **256 BG tiles + 256 sprite tiles** | Two pattern tables, 256 tiles each, is all the PPU can address (NROM). |
| Each tile uses **2 bits per pixel** (4 colors) | Hardware limit. The actual colors come from the palette, not the tile data. |
| Plan tile reuse — SMB uses <200 unique tiles | Reuse via flipping (sprites) and clever layout saves space. |
| Leave tile index `$00` as blank/transparent | Convention; simplifies nametable clearing. |

---

## 2. Palettes

### NES palette basics
- 32 bytes total: **4 BG sub-palettes** + **4 sprite sub-palettes**.
- Each sub-palette = 4 colors (but color 0 is the shared universal background).
- Effective unique colors: 1 shared BG + 4×3 BG + 4×3 sprite = **25 on-screen colors**.

### Source format
Plain text `.pal` files in `assets/palettes/`:
```
# bg_palette.pal — 16 entries (4 sub-palettes × 4 colors)
0F 30 10 00
0F 27 17 07
0F 29 19 09
0F 12 22 32

# spr_palette.pal — 16 entries
0F 16 26 36
0F 11 21 31
0F 05 15 25
0F 09 19 29
```

### Loading
At runtime, palettes are written to PPU `$3F00`–`$3F1F` during VBlank via neslib's `pal_bg()` / `pal_spr()`.

### Strategy for a Super Mario Bros–style game

| Sub-palette | Purpose | Example colors |
|---|---|---|
| BG 0 | Sky + ground | Sky blue, brown, dark brown, white |
| BG 1 | Bricks / blocks | Sky blue, orange-brown, dark red, cream |
| BG 2 | Pipes / bushes | Sky blue, green, dark green, light green |
| BG 3 | Coins / special | Sky blue, gold, dark gold, white |
| SPR 0 | Player (Alconario) | Transparent, skin, shirt, hat |
| SPR 1 | Enemies (Goombas) | Transparent, brown, dark brown, cream |
| SPR 2 | Fire / items | Transparent, red, orange, white |
| SPR 3 | HUD / extra | Transparent, white, grey, black |

> **Attribute table constraint**: BG palette assignment is per 16×16 area (2×2 tiles). Plan level art so color transitions align to this grid.

---

## 3. Music & SFX

### Toolchain
1. Compose in **FamiTracker** (`.ftm` files stored in `assets/music/`).
2. Export as **FamiTracker text** (`.txt`).
3. Convert with **text2data** (famitone2 tool):
   ```bash
   text2data assets/music/overworld.txt -ca65 -o src/asm/music_overworld.s
   ```
4. Include the `.s` file; famitone2 runtime plays it.

### Channel budget
| Channel | Typical use |
|---|---|
| Pulse 1 | Melody |
| Pulse 2 | Harmony / counter-melody |
| Triangle | Bass line |
| Noise | Drums / percussion |
| DPCM | Sampled kicks, speech (uses PRG-ROM space!) |

### SFX
- Composed as short FamiTracker instruments or single-channel patterns.
- Exported with `nsf2data` or `text2data --sfx`.
- Played via `famitone2_sfx_play()`.

### Strategy
- **One music track per game state**: title, overworld, underground, boss, game-over, victory.
- **~8–12 SFX**: jump, coin, stomp, powerup, pipe, bump, fireball, death, 1-up, flag.
- Keep DPCM usage minimal on NROM (eats PRG-ROM space).

---

## 4. Levels

### Design philosophy (SMB-style)
Levels scroll **horizontally** (left-to-right). Each level is a series of **screens** (256 px wide = 32 tiles). A typical world has 4–8 screens.

### Data format
Each level is a compressed/packed data blob containing:

| Data | Encoding | Size estimate |
|---|---|---|
| **Metatile map** | RLE or column-based (tile indices per 16×16 metatile) | ~200–500 bytes/screen |
| **Object list** | `(type, x, y, param)` tuples — enemies, coins, blocks | ~4–8 bytes/object |
| **Attribute hints** | Palette assignment per metatile (can be implicit from metatile ID) | Included in metatile defs |
| **Level header** | Width (screens), music track, time limit, start position, background palette set | ~8 bytes |

### Metatile system
Instead of placing individual 8×8 tiles, define **metatiles** (16×16 or 32×32 blocks):

```
Metatile #01 "Brick":
  ┌────┬────┐
  │ $45│ $46│   ← tile indices into pattern table
  ├────┼────┤
  │ $55│ $56│
  └────┴────┘
  Palette: BG sub-palette 1
  Collision: solid
  Behavior: breakable
```

Benefits:
- **Saves ROM**: level stored as metatile indices (1 byte per 16×16 area vs 4 bytes).
- **Built-in collision**: each metatile has a collision flag (empty, solid, platform, hazard).
- **Built-in palette**: attribute table values derived automatically.

### Level file convention
`assets/levels/world_X_Y.json`:
```json
{
  "world": 1, "level": 1,
  "screens": 8,
  "music": "overworld",
  "time": 400,
  "palette_bg": "overworld.pal",
  "metatile_map": [ ... ],
  "objects": [
    { "type": "goomba", "screen": 2, "x": 5, "y": 10 },
    { "type": "coin_block", "screen": 1, "x": 12, "y": 6 }
  ]
}
```

### Build step
```bash
python3 tools/lvl_pack.py assets/levels/world_1_1.json -o src/asm/level_1_1.s
```
Packs JSON into ca65-compatible `.byte` tables with optional RLE compression.

---

## 5. Tile Organization Strategy

### Pattern Table 0 — Background (`$0000–$0FFF`)

| Index range | Content |
|---|---|
| `$00` | Empty / sky |
| `$01–$0F` | Font (digits 0–9, some punctuation) |
| `$10–$3F` | Ground, dirt, underground tiles |
| `$40–$6F` | Bricks, blocks (? block, used block, breakable) |
| `$70–$8F` | Pipes (top-left, top-right, body-left, body-right) |
| `$90–$AF` | Scenery (clouds, bushes, hills, fences) |
| `$B0–$CF` | Special (flag, castle, water, lava) |
| `$D0–$FF` | Title screen / reserved |

### Pattern Table 1 — Sprites (`$1000–$1FFF`)

| Index range | Content |
|---|---|
| `$00–$1F` | Player (walk cycle ×2, jump, skid, climb, crouch, death, swim) |
| `$20–$3F` | Player big / power-up variant |
| `$40–$5F` | Goomba, Koopa (walk cycles, squished, shell) |
| `$60–$7F` | Other enemies (Piranha Plant, Bullet Bill, Hammer Bro) |
| `$80–$9F` | Items (coin spin, mushroom, star, fire flower, 1-up) |
| `$A0–$BF` | Projectiles (fireball, hammer) + particles |
| `$C0–$DF` | HUD elements (small coin icon, ×, digits) |
| `$E0–$FF` | Reserved / title screen sprites |

---

## 6. Directory Summary

```
assets/
├── chr/
│   ├── bg_tiles.png        # Source art (indexed PNG)
│   ├── spr_tiles.png       # Source art (indexed PNG)
│   └── tiles.chr           # Compiled 8 KiB CHR-ROM (generated)
├── palettes/
│   ├── overworld.pal       # BG palette set for overworld levels
│   ├── underground.pal     # BG palette set for underground
│   ├── castle.pal          # BG palette set for castle
│   └── sprites.pal         # Sprite palettes (shared across levels)
├── music/
│   ├── overworld.ftm       # FamiTracker source
│   ├── underground.ftm
│   ├── boss.ftm
│   ├── title.ftm
│   ├── gameover.ftm
│   └── sfx.ftm             # Sound effects
└── levels/
    ├── metatiles.json       # Metatile definitions (shared)
    ├── world_1_1.json       # Level data
    ├── world_1_2.json
    └── ...
```

---

## Quick Reference: SMB-Style Mechanics Checklist

- [ ] Horizontal scrolling (camera follows player, no scroll-back)
- [ ] Metatile-based level rendering
- [ ] Gravity + variable-height jump (hold A = higher)
- [ ] Collision: solid, platform (one-way), breakable, hazard
- [ ] ? Blocks → items (coin, mushroom, star, fire flower)
- [ ] Enemies: walk, stomp-to-kill, shell physics
- [ ] Power-up states: small → big → fire
- [ ] Pipes (enter/exit, warp zones)
- [ ] Flagpole / level end trigger
- [ ] Coins + score + timer
- [ ] Lives system + game over
- [ ] Status bar (Sprite 0 hit for split scroll)
