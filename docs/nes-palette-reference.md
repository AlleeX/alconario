# NES Palette Reference 🎨

Complete reference for the NES color system, hardware limitations, and the full 64-color master palette with HEX values.

---

## Table of Contents

1. [How NES Colors Work](#how-nes-colors-work)
2. [Limitations Summary](#limitations-summary)
3. [Full NES Master Palette (64 Colors)](#full-nes-master-palette-64-colors)
4. [Palette Memory Map](#palette-memory-map)
5. [Emphasis Bits](#emphasis-bits)
6. [Choosing Palettes for a Platformer](#choosing-palettes-for-a-platformer)

---

## How NES Colors Work

The NES does **not** use RGB. The PPU generates an analog NTSC composite signal directly from a 6-bit color index (`$00`–`$3F`). Different emulators and capture devices decode this signal slightly differently, which is why NES palettes vary across sources.

### Color index layout

The 6-bit index is organized as:

```
  Bits:  5 4 3 2 1 0
         └─┘ └───┘
        Luma  Hue
```

- **Hue** (bits 3–0): 0 = grey, 1–C = color wheel, D = black, E–F = black (mirrors)
- **Luma** (bits 5–4): 0 = darkest, 1 = dark, 2 = medium, 3 = lightest

---

## Limitations Summary

| Constraint | Detail |
| --- | --- |
| Master palette | 64 entries (really ~54 unique; some are duplicate blacks) |
| On-screen at once | **25 unique colors** max (1 universal BG + 4×3 BG + 4×3 sprite) |
| BG sub-palettes | 4 palettes × 4 colors (color 0 = shared universal BG) |
| Sprite sub-palettes | 4 palettes × 4 colors (color 0 = transparent) |
| Palette assignment (BG) | Per **16×16 pixel area** (2×2 tiles) via attribute table |
| Palette assignment (Sprites) | Per **sprite** (each sprite picks one of 4 sub-palettes) |
| Colors per tile | **4** (2 bits per pixel) — but one is always BG/transparent |
| Sprite color 0 | Always transparent (shows BG behind) |
| BG color 0 | Universal background color (same across all 4 BG sub-palettes) |
| Total palette RAM | 32 bytes at PPU `$3F00`–`$3F1F` |
| Emphasis bits | PPU register `$2001` bits 5–7 can tint the entire screen (R/G/B) |

### The "3+1" rule

Each sub-palette gives you **3 unique colors + 1 shared** (BG or transparent). Plan art around groups of 3 colors that work together.

---

## Full NES Master Palette (64 Colors)

The table below uses the **"2C02G" canonical palette** — the most widely used reference (matches FCEUX, Mesen defaults). HEX values are **sRGB approximations** of the NTSC signal.

### Row 0 — Darkest (Luma 0)

| Index | Hex | NES ID | Color Name | Swatch |
| --- | --- | --- | --- | --- |
| `$00` | `#585858` | 00 | Dark Grey | 🟫 |
| `$01` | `#00238C` | 01 | Dark Blue | 🔵 |
| `$02` | `#00139B` | 02 | Dark Indigo | 🔵 |
| `$03` | `#2D0585` | 03 | Dark Violet | 🟣 |
| `$04` | `#5D0052` | 04 | Dark Magenta | 🟣 |
| `$05` | `#7A0017` | 05 | Dark Red | 🔴 |
| `$06` | `#7A0800` | 06 | Dark Scarlet | 🔴 |
| `$07` | `#611800` | 07 | Dark Brown | 🟤 |
| `$08` | `#3B2400` | 08 | Dark Olive | 🟤 |
| `$09` | `#013A00` | 09 | Dark Green | 🟢 |
| `$0A` | `#004000` | 0A | Dark Forest | 🟢 |
| `$0B` | `#003C14` | 0B | Dark Teal | 🟢 |
| `$0C` | `#003650` | 0C | Dark Cyan | 🔵 |
| `$0D` | `#000000` | 0D | Black | ⬛ |
| `$0E` | `#000000` | 0E | Black (mirror) | ⬛ |
| `$0F` | `#000000` | 0F | Black (mirror) | ⬛ |

### Row 1 — Dark (Luma 1)

| Index | Hex | NES ID | Color Name | Swatch |
| --- | --- | --- | --- | --- |
| `$10` | `#A0A0A0` | 10 | Medium Grey | ⬜ |
| `$11` | `#0063D1` | 11 | Blue | 🔵 |
| `$12` | `#1B4AFF` | 12 | Royal Blue | 🔵 |
| `$13` | `#6E29FF` | 13 | Violet | 🟣 |
| `$14` | `#B31AA8` | 14 | Magenta | 🟣 |
| `$15` | `#D81853` | 15 | Red-Magenta | 🔴 |
| `$16` | `#D82800` | 16 | Red | 🔴 |
| `$17` | `#BE4A10` | 17 | Orange-Brown | 🟠 |
| `$18` | `#8A6C00` | 18 | Olive | 🟤 |
| `$19` | `#378600` | 19 | Green | 🟢 |
| `$1A` | `#009400` | 1A | Bright Green | 🟢 |
| `$1B` | `#008C3C` | 1B | Sea Green | 🟢 |
| `$1C` | `#007E8C` | 1C | Teal | 🔵 |
| `$1D` | `#000000` | 1D | Black (true) | ⬛ |
| `$1E` | `#000000` | 1E | Black (mirror) | ⬛ |
| `$1F` | `#000000` | 1F | Black (mirror) | ⬛ |

### Row 2 — Medium (Luma 2)

| Index | Hex | NES ID | Color Name | Swatch |
| --- | --- | --- | --- | --- |
| `$20` | `#F8F8F8` | 20 | White | ⬜ |
| `$21` | `#3CBCFC` | 21 | Light Blue | 🔵 |
| `$22` | `#6888FC` | 22 | Cornflower | 🔵 |
| `$23` | `#9878F8` | 23 | Light Violet | 🟣 |
| `$24` | `#F878F8` | 24 | Pink | 🩷 |
| `$25` | `#F85898` | 25 | Hot Pink | 🩷 |
| `$26` | `#F87858` | 26 | Salmon | 🟠 |
| `$27` | `#FCA044` | 27 | Orange | 🟠 |
| `$28` | `#F8B800` | 28 | Gold | 🟡 |
| `$29` | `#B8F818` | 29 | Yellow-Green | 🟢 |
| `$2A` | `#58D854` | 2A | Light Green | 🟢 |
| `$2B` | `#58F898` | 2B | Mint | 🟢 |
| `$2C` | `#00E8D8` | 2C | Aqua | 🔵 |
| `$2D` | `#787878` | 2D | Grey | ⬜ |
| `$2E` | `#000000` | 2E | Black (mirror) | ⬛ |
| `$2F` | `#000000` | 2F | Black (mirror) | ⬛ |

### Row 3 — Lightest (Luma 3)

| Index | Hex | NES ID | Color Name | Swatch |
| --- | --- | --- | --- | --- |
| `$30` | `#FCFCFC` | 30 | Bright White | ⬜ |
| `$31` | `#A4E4FC` | 31 | Pale Blue | 🔵 |
| `$32` | `#B8B8F8` | 32 | Lavender | 🟣 |
| `$33` | `#D8B8F8` | 33 | Light Lavender | 🟣 |
| `$34` | `#F8B8F8` | 34 | Pale Pink | 🩷 |
| `$35` | `#F8A4C0` | 35 | Light Rose | 🩷 |
| `$36` | `#F0D0B0` | 36 | Peach | 🟠 |
| `$37` | `#FCE0A8` | 37 | Pale Orange | 🟡 |
| `$38` | `#F8D878` | 38 | Light Gold | 🟡 |
| `$39` | `#D8F878` | 39 | Pale Yellow-Green | 🟢 |
| `$3A` | `#B8F8B8` | 3A | Pale Green | 🟢 |
| `$3B` | `#B8F8D8` | 3B | Pale Mint | 🟢 |
| `$3C` | `#00FCFC` | 3C | Bright Cyan | 🔵 |
| `$3D` | `#D8D8D8` | 3D | Light Grey | ⬜ |
| `$3E` | `#000000` | 3E | Black (mirror) | ⬛ |
| `$3F` | `#000000` | 3F | Black (mirror) | ⬛ |

### Quick-Reference Grid

```
       Hue →  0(grey)  1       2       3       4       5       6       7       8       9       A       B       C       D(blk)
Luma 0 (dark)  585858  00238C  00139B  2D0585  5D0052  7A0017  7A0800  611800  3B2400  013A00  004000  003C14  003650  000000
Luma 1         A0A0A0  0063D1  1B4AFF  6E29FF  B31AA8  D81853  D82800  BE4A10  8A6C00  378600  009400  008C3C  007E8C  000000
Luma 2         F8F8F8  3CBCFC  6888FC  9878F8  F878F8  F85898  F87858  FCA044  F8B800  B8F818  58D854  58F898  00E8D8  787878
Luma 3         FCFCFC  A4E4FC  B8B8F8  D8B8F8  F8B8F8  F8A4C0  F0D0B0  FCE0A8  F8D878  D8F878  B8F8B8  B8F8D8  00FCFC  D8D8D8
```

> ⚠️ **Avoid `$0D`** — on real hardware, some TVs interpret this as "blacker than black" and it can cause sync issues. Use `$0F` for black instead.

---

## Palette Memory Map

```
PPU Address     Contents
──────────      ────────────────────────────
$3F00           Universal background color
$3F01–$3F03     BG sub-palette 0 (colors 1–3)
$3F04           Mirror of $3F00 (writes ignored for color selection)
$3F05–$3F07     BG sub-palette 1
$3F08           Mirror of $3F00
$3F09–$3F0B     BG sub-palette 2
$3F0C           Mirror of $3F00
$3F0D–$3F0F     BG sub-palette 3
$3F10           Mirror of $3F00 (sprite color 0 = transparent)
$3F11–$3F13     Sprite sub-palette 0
$3F14           Mirror of $3F00
$3F15–$3F17     Sprite sub-palette 1
$3F18           Mirror of $3F00
$3F19–$3F1B     Sprite sub-palette 2
$3F1C           Mirror of $3F00
$3F1D–$3F1F     Sprite sub-palette 3
```

In code (neslib):

```c
// Set all 16 BG palette bytes
const unsigned char bg_pal[] = {
    0x0F, 0x30, 0x10, 0x00,   // sub-palette 0
    0x0F, 0x27, 0x17, 0x07,   // sub-palette 1
    0x0F, 0x29, 0x19, 0x09,   // sub-palette 2
    0x0F, 0x12, 0x22, 0x32    // sub-palette 3
};
pal_bg(bg_pal);

// Set all 16 sprite palette bytes
const unsigned char spr_pal[] = {
    0x0F, 0x16, 0x26, 0x36,   // sub-palette 0 (player)
    0x0F, 0x11, 0x21, 0x31,   // sub-palette 1 (enemies)
    0x0F, 0x05, 0x15, 0x25,   // sub-palette 2 (items)
    0x0F, 0x09, 0x19, 0x29    // sub-palette 3 (HUD)
};
pal_spr(spr_pal);
```

---

## Emphasis Bits

PPU register `$2001` bits 5–7 apply a color tint to the **entire screen**:

| Bit | NTSC effect | Use case |
| --- | --- | --- |
| 5 | Emphasize Red (darken green+blue) | Sunset / lava levels |
| 6 | Emphasize Green (darken red+blue) | Underwater tint |
| 7 | Emphasize Blue (darken red+green) | Night / cave |
| 5+6+7 | All emphasized = dim everything | Fade-to-dark effect |

> Can be toggled mid-frame for scanline-level tricks (e.g., sky vs ground tint).

---

## Choosing Palettes for a Platformer

### Workflow

1. **Pick universal BG** — usually sky color (`$21` light blue for overworld, `$0F` black for underground).
2. **Build BG sub-palettes** around terrain types:
   - Ground/dirt ramp (3 shades from same hue column)
   - Brick/block ramp
   - Vegetation ramp
   - Special (coins, lava, water)
3. **Build sprite sub-palettes**:
   - Player (skin + 2 outfit colors)
   - Enemies (2–3 brown/red tones)
   - Items/projectiles (bright, high-contrast)
   - HUD/text (white + grey)
4. **Swap palettes per level theme** — only costs 32 bytes during VBlank.

### Example: Alconario Overworld

```
BG Palette (universal BG = $21 sky blue):
  Sub 0: $21  $30  $10  $00    Sky → white, grey, dark grey (clouds, text)
  Sub 1: $21  $27  $17  $07    Sky → orange, brown, dark brown (ground)
  Sub 2: $21  $2A  $1A  $09    Sky → lt green, green, dk green (bushes, pipes)
  Sub 3: $21  $28  $18  $08    Sky → gold, olive, dk olive (coins, ? blocks)

Sprite Palette (color 0 = transparent):
  Sub 0: $0F  $36  $16  $06    Peach skin, red shirt, dark red hat (Alconario)
  Sub 1: $0F  $27  $17  $07    Orange, brown, dk brown (Goombas)
  Sub 2: $0F  $28  $16  $30    Gold, red, white (coins, fire, stars)
  Sub 3: $0F  $30  $10  $00    White, grey, dk grey (HUD, text)
```

---

## Tools for Palette Design

| Tool | Link | Notes |
| --- | --- | --- |
| **NES Lightbox** | [nesdev.org/NESLightbox](https://www.nesdev.org/NESLightbox/) | Web-based, drag & drop palette builder |
| **NEXXT** | [frankengraphics](https://frankengraphics.itch.io/nexxt) | Full NES graphics editor (tiles + palettes) |
| **YY-CHR** | [Various mirrors](https://www.romhacking.net/utilities/119/) | Classic CHR editor with palette support |
| **Aseprite** | [aseprite.org](https://www.aseprite.org/) | Pixel art editor; use indexed mode with 4-color palette |

---

## Further Reading

- [NESDev Wiki — PPU Palettes](https://www.nesdev.org/wiki/PPU_palettes)
- [NESDev Wiki — Full Palette Table](https://www.nesdev.org/wiki/PPU_palettes#2C02)
- [Bisqwit's NES Palette Generator](https://bisqwit.iki.fi/utils/nespalette.php)
