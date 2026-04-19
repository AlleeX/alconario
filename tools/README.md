# Tools

Helper scripts for converting and packing game assets.

## Setup

```bash
pip3 install -r tools/requirements.txt
```

### Dependencies

| Package | Version | Purpose |
| --- | --- | --- |
| **Pillow** | ≥ 10.0 | PNG reading/writing for `png2chr.py`, `validate_tiles.py` |

## Scripts

### `png2chr.py` — PNG to NES CHR-ROM converter

Converts 128×128 indexed PNG tile sheets (drawn in Procreate) to 8 KiB NES CHR-ROM binary.

```bash
# Convert both BG + sprite sheets:
python3 tools/png2chr.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png -o assets/chr/tiles.chr

# Specify sub-palettes (NES hex IDs):
python3 tools/png2chr.py assets/chr/bg_tiles.png --bg-palette 21,30,10,00 -o assets/chr/tiles.chr

# Validate colors only (no conversion):
python3 tools/png2chr.py --validate assets/chr/bg_tiles.png --palette assets/palettes/overworld.pal
```

### `validate_tiles.py` — Quick tile sheet validator

Checks image dimensions, colors-per-tile (max 4), and NES palette compliance.

```bash
python3 tools/validate_tiles.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png
```

### `pal2procreate.py` — NES palette to Procreate swatches

Generates `.swatches` files you can AirDrop to iPad and import into Procreate.

```bash
# Full NES 64-color master palette:
python3 tools/pal2procreate.py --master -o assets/palettes/NES_Master.swatches

# Specific game palette (split into 4-color sub-palettes):
python3 tools/pal2procreate.py assets/palettes/overworld.pal --split -o assets/palettes/
```

### `gen_chr.py` — Procedural CHR generator (legacy)

Generates tiles programmatically (bottle art, font). Used before the Procreate art pipeline was set up.

```bash
python3 tools/gen_chr.py
```

## Typical Workflow

```
1. python3 tools/pal2procreate.py --master -o assets/palettes/   # Get palettes into Procreate
2. (Draw tiles in Procreate, export as 128×128 PNG)
3. python3 tools/validate_tiles.py assets/chr/bg_tiles.png       # Check for errors
4. python3 tools/png2chr.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png -o assets/chr/tiles.chr
5. make                                                           # Build ROM
6. make run                                                       # Test in emulator
```

Or simply: `make chr && make` (if using the Makefile `chr` target).
