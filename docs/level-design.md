# Level Design Strategy 🗺️

How levels are structured, stored, and rendered in Alconario — a horizontal-scrolling platformer in the style of Super Mario Bros.

---

## Core Mechanics (SMB-Style)

### Scrolling

- **Horizontal only**, left-to-right. Camera follows the player; no scroll-back (like SMB1).
- Implemented via PPU nametable swapping + scroll register updates.
- Vertical mirroring (two horizontal nametables) — one is visible, the other is updated off-screen.
- New column of metatiles drawn into the off-screen nametable as the player advances.

### World / Level structure

```
World 1 ──► Level 1-1 (overworld, 8 screens)
         ──► Level 1-2 (underground, 6 screens)
         ──► Level 1-3 (overworld with platforms, 6 screens)
         ──► Level 1-4 (castle / boss, 4 screens)
```

Planned: **4 worlds × 4 levels** = 16 levels (expandable with mapper upgrade).

---

## Level Data Format

### Level header (8 bytes)

| Byte | Field | Description |
| --- | --- | --- |
| 0 | `width` | Number of screens (1–16) |
| 1 | `music` | Music track ID |
| 2 | `time` | Time limit (×10 seconds, so 40 = 400 seconds) |
| 3 | `bg_palette` | Index into palette table |
| 4 | `start_x` | Player start X (in metatile units) |
| 5 | `start_y` | Player start Y (in metatile units) |
| 6 | `flags` | Bit flags: auto-scroll, underwater, dark, etc. |
| 7 | `reserved` | — |

### Metatile map

Each screen is **16 columns × 13 rows** of metatiles (16×16 px each = 256×208 visible area, with top 32px reserved for HUD).

Storage: `width × 16 × 13` bytes (uncompressed) or RLE-compressed columns.

#### Column-based RLE encoding

Levels tend to have lots of vertical repetition (sky above, ground below). Encode each column as RLE:

```
Column data: [count, metatile_id, count, metatile_id, ...]
Terminator: 0 (count=0 means end of column)
```

Example — a column with 10 sky + 3 ground:

```
0A 00 03 01 00
```

This typically compresses a screen from 208 bytes to ~30–60 bytes.

### Object list

Enemies and interactive objects are stored separately:

```c
typedef struct {
    unsigned char screen;   // which screen (0-based)
    unsigned char x;        // X position in metatile units (0–15)
    unsigned char y;        // Y position in metatile units (0–12)
    unsigned char type;     // object type enum
    unsigned char param;    // type-specific parameter
} LevelObject;
```

Object types:

| Type ID | Name | Param meaning |
| --- | --- | --- |
| 0 | Goomba | Direction (0=right, 1=left) |
| 1 | Koopa (green) | Direction |
| 2 | Koopa (red) | Direction (patrols platform) |
| 3 | Piranha Plant | Pipe metatile position |
| 4 | Bullet Bill cannon | Fire interval (frames) |
| 5 | Hammer Bro | — |
| 6 | Coin block | Item type (0=coin, 1=multi-coin) |
| 7 | ? Block | Item type (0=coin, 1=mushroom, 2=fire flower, 3=star, 4=1up) |
| 8 | Hidden block | Item type |
| 9 | Coin (free) | — |
| 10 | Springboard | Jump height |
| 11 | Moving platform | Direction + range |
| 12 | Warp pipe | Destination (world, level, screen) |
| 13 | Flagpole | — |
| 14 | Checkpoint | — |

---

## Level Design Guidelines

### Difficulty curve (SMB philosophy)

1. **World 1-1**: Teach core mechanics organically.
   - First screen: open space to experiment with controls.
   - First enemy: Goomba in open area (easy to stomp or avoid).
   - First ? Block: placed so player naturally bumps it → discovers mushroom.
   - First pipe: curiosity reward (coins or shortcut).

2. **Progressive complexity**:
   - Introduce ONE new element per level.
   - Test the element in a safe context first, then in a dangerous context.
   - Combine previously learned elements.

3. **Rhythm**: Alternate between action and rest sections.

### Screen composition rules

| Rule | Reason |
| --- | --- |
| Start with safe ground | Player needs orientation time |
| Place enemies after a gap or elevation change | Creates decision moments |
| Max 3 enemies visible at once | Sprite limit + fairness |
| Always provide a visible "way out" | Avoid feeling trapped |
| Reward exploration (hidden blocks, coin rooms) | Replay value |
| End screens with ground (not a gap) | Safe stopping point |

### Attribute table planning

Remember: BG palette changes on a **16×16 grid** (metatile boundary).

```
Good ✓                        Bad ✗
┌──────┬──────┐              ┌──────┬──────┐
│ Sky  │ Sky  │              │ Sky  │SKY+  │
│ pal0 │ pal0 │              │ pal0 │BRICK │ ← Two palettes needed
├──────┼──────┤              │      │pal1  │   in one attribute cell!
│Ground│Ground│              ├──────┼──────┤
│ pal1 │ pal1 │              │      │      │
└──────┴──────┘              └──────┴──────┘
```

Design tile art so color transitions align to 16×16 boundaries.

---

## Level Editing Workflow

### Phase 1 (Current) — JSON + Python

1. Design level in a spreadsheet or text grid.
2. Encode as `assets/levels/world_X_Y.json`.
3. Run `tools/lvl_pack.py` to generate `.s` assembly data.
4. Build and test in emulator.

### Phase 2 (Future) — Visual editor

- Use **Tiled** (mapeditor.org) with custom NES tileset.
- Export as JSON/CSV → same `lvl_pack.py` pipeline.
- Or build a custom NEXXT-based workflow.

---

## Memory Budget (NROM)

| Resource | Available | Notes |
| --- | --- | --- |
| PRG-ROM | 32 KiB | Code + level data + music + SFX |
| CHR-ROM | 8 KiB | All tiles |
| CPU RAM | 2 KiB | Runtime state (player, enemies, scroll) |

### Level data budget estimate

| Component | Per level | 16 levels total |
| --- | --- | --- |
| Header | 8 bytes | 128 bytes |
| Metatile map (RLE) | ~300 bytes (8 screens) | ~4.8 KiB |
| Object list | ~60 bytes (15 objects) | ~960 bytes |
| **Total** | **~370 bytes** | **~5.9 KiB** |

This leaves ~26 KiB for code, metatile definitions, music, palettes, and string data — tight but feasible for NROM.

---

## Runtime: Scrolling Engine

```
Per frame:
  1. Read player X → compute camera scroll position
  2. If new metatile column entered:
     a. Decode next column from level data
     b. Write 13 metatiles (= 26 tiles) to off-screen nametable column
     c. Update corresponding attribute table bytes
  3. Set PPU scroll registers ($2005)
  4. Spawn any objects whose screen X is now on-screen
  5. Despawn objects that scrolled off the left edge
```

Column rendering must complete within VBlank (~2,273 CPU cycles). Writing 26 tiles = 52 PPU writes ≈ ~400 cycles — well within budget.

---

## File Organization

```
assets/levels/
├── metatiles.json          # Shared metatile definitions
│                            (tile IDs, palette, collision, behavior)
├── world_1_1.json          # Level data files
├── world_1_2.json
├── world_1_3.json
├── world_1_4.json
├── world_2_1.json
├── ...
└── README.md               # Level design notes and conventions
```

---

## Related Docs

- [Tiles & CHR Strategy](tiles-and-chr.md) — tile allocation and metatile format
- [Asset Pipeline](asset-pipeline.md) — build flow from JSON to ROM
- [NES Palette Reference](nes-palette-reference.md) — attribute table color constraints
