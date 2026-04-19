# Graphics Developer Workflow 🎨

Step-by-step guide for creating NES tile graphics using **Procreate** (iPad) and converting them into the game ROM.

---

## Roles

| Role | Responsibility | Tools |
| --- | --- | --- |
| **Pixel Artist** | Draws tiles in Procreate with NES palette constraints | Procreate (iPad) |
| **Asset Integrator** | Runs conversion scripts, verifies CHR output in emulator | Python 3, terminal, emulator |
| **Game Developer** | Assigns tiles to metatiles, updates level data, codes rendering | cc65, text editor, emulator |

In a solo project, one person fills all three roles. The pipeline is designed so the artist only needs Procreate + a shared folder.

---

## Canvas Setup in Procreate

### Background tiles canvas

| Setting | Value |
| --- | --- |
| Width | **128 px** |
| Height | **128 px** |
| DPI | 72 (irrelevant for pixel art) |
| Color profile | sRGB |
| Grid | 8×8 px (use Drawing Guide → 2D Grid, grid size = 8) |

This gives you a **16×16 grid of 8×8 tiles** = 256 tiles (one full NES pattern table).

### Sprite tiles canvas

Same as above: 128×128 px, 8×8 grid.

### Canvas settings tips

- Enable **Drawing Guide** → 2D Grid → grid size **8 px**, opacity ~25%.
- Use **Assisted Drawing** off (it snaps to symmetry, not what we want).
- Set brush to **Monoline** or **Hard Round** at **1 px** size.
- Zoom in to max — you're working pixel-by-pixel.
- Turn off **StreamLine** (set to 0%) for precise pixel placement.

---

## Color Rules (CRITICAL)

Each tile can use **exactly 4 colors** (indices 0–3). Color 0 is always background/transparent.

### Setting up the palette in Procreate

1. Create a **custom palette** named `NES_BG_Sub0` (or per sub-palette).
2. Add exactly **4 colors** using HEX values from the [NES Palette Reference](nes-palette-reference.md).
3. Use **only these 4 colors** when drawing tiles assigned to that sub-palette.

Example — overworld ground sub-palette:

| Index | Role | NES ID | HEX |
| --- | --- | --- | --- |
| 0 | Background (sky) | `$21` | `#3CBCFC` |
| 1 | Light brown | `$27` | `#FCA044` |
| 2 | Medium brown | `$17` | `#BE4A10` |
| 3 | Dark brown | `$07` | `#611800` |

### Important constraints

- **Do NOT use anti-aliasing** — every pixel must be one of the 4 exact colors.
- **Do NOT use layers with transparency blending** — flatten before export.
- If a tile belongs to BG sub-palette 2, it uses that sub-palette's 4 colors only.
- Sprite tiles: color 0 = **transparent** (pick a distinct "key" color like `#FF00FF` magenta, the converter will map it to index 0).

---

## Drawing Workflow

### Step 1: Plan on paper

Sketch your metatiles (16×16) first. Refer to the [metatile catalog](tiles-and-chr.md) for what's needed.

### Step 2: Draw in Procreate

1. Open the 128×128 canvas.
2. Select the correct 4-color palette for the sub-palette.
3. Draw tiles in their assigned grid position (see [tile allocation map](tiles-and-chr.md#pattern-table-0--background-0000-0fff)).
4. Use the grid overlay to stay within 8×8 boundaries.

**Tip**: Draw metatiles as 16×16 blocks first, then verify each 8×8 quarter looks correct independently.

### Step 3: Export as PNG

1. **Actions** → **Share** → **PNG**.
2. Ensure the export is **128×128 px** (do NOT let Procreate upscale).
3. Save to `assets/chr/`:
   - `bg_tiles.png` — background pattern table
   - `spr_tiles.png` — sprite pattern table

### Step 4: Validate colors

Run the validation tool before converting:

```bash
python3 tools/png2chr.py --validate assets/chr/bg_tiles.png --palette assets/palettes/overworld.pal
```

This checks that every pixel matches one of the expected NES colors.

### Step 5: Convert to CHR

```bash
python3 tools/png2chr.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png -o assets/chr/tiles.chr
```

### Step 6: Build & verify

```bash
make
make run
```

Open the PPU viewer in FCEUX (Debug → PPU Viewer) or Mesen to verify tiles look correct.

---

## File Flow Diagram

```
Procreate (iPad)
    │
    ▼
bg_tiles.png  +  spr_tiles.png     (128×128, indexed 4-color)
    │               │
    ▼               ▼
    └───── png2chr.py ──────┐
                            ▼
                      tiles.chr (8 KiB)
                            │
                            ▼
                      src/asm/chr.s (.incbin)
                            │
                            ▼
                      ca65 → chr.o → ld65 → alconario.nes
```

---

## Procreate Color Palettes (Ready to Import)

Create these as Procreate swatches. Each has 4 colors.

### Background palettes — Overworld

**BG Sub-palette 0** (Sky / Clouds):

| Swatch | HEX | NES ID | Use |
| --- | --- | --- | --- |
| 1 | `#3CBCFC` | `$21` | Sky (universal BG) |
| 2 | `#FCFCFC` | `$30` | White (cloud) |
| 3 | `#A0A0A0` | `$10` | Grey |
| 4 | `#585858` | `$00` | Dark grey |

**BG Sub-palette 1** (Ground / Bricks):

| Swatch | HEX | NES ID | Use |
| --- | --- | --- | --- |
| 1 | `#3CBCFC` | `$21` | Sky (shared BG) |
| 2 | `#FCA044` | `$27` | Orange-brown |
| 3 | `#BE4A10` | `$17` | Brown |
| 4 | `#611800` | `$07` | Dark brown |

**BG Sub-palette 2** (Pipes / Vegetation):

| Swatch | HEX | NES ID | Use |
| --- | --- | --- | --- |
| 1 | `#3CBCFC` | `$21` | Sky (shared BG) |
| 2 | `#58D854` | `$2A` | Light green |
| 3 | `#009400` | `$1A` | Green |
| 4 | `#013A00` | `$09` | Dark green |

**BG Sub-palette 3** (Coins / Special):

| Swatch | HEX | NES ID | Use |
| --- | --- | --- | --- |
| 1 | `#3CBCFC` | `$21` | Sky (shared BG) |
| 2 | `#F8B800` | `$28` | Gold |
| 3 | `#8A6C00` | `$18` | Dark gold |
| 4 | `#3B2400` | `$08` | Dark olive |

### Sprite palettes

**SPR Sub-palette 0** (Player — Alconario):

| Swatch | HEX | NES ID | Use |
| --- | --- | --- | --- |
| 1 | `#FF00FF` | — | Transparent (key color) |
| 2 | `#F0D0B0` | `$36` | Skin / peach |
| 3 | `#D82800` | `$16` | Red (shirt) |
| 4 | `#7A0800` | `$06` | Dark red (hat, shoes) |

**SPR Sub-palette 1** (Enemies):

| Swatch | HEX | NES ID | Use |
| --- | --- | --- | --- |
| 1 | `#FF00FF` | — | Transparent |
| 2 | `#FCA044` | `$27` | Orange |
| 3 | `#BE4A10` | `$17` | Brown |
| 4 | `#611800` | `$07` | Dark brown |

---

## Iteration Checklist

- [ ] Canvas is 128×128 px
- [ ] Grid overlay is 8×8
- [ ] Only 4 exact HEX colors used per tile group
- [ ] No anti-aliasing, no transparency blending
- [ ] Exported as PNG (not JPEG, not PSD)
- [ ] `png2chr.py --validate` passes with no errors
- [ ] Tiles visible in emulator PPU viewer
- [ ] Metatile assignments updated in level data

---

## Related Docs

- [NES Palette Reference](nes-palette-reference.md) — all 64 NES colors with HEX
- [Tiles & CHR Strategy](tiles-and-chr.md) — tile allocation and metatile system
- [Asset Pipeline](asset-pipeline.md) — full build flow
