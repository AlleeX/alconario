#!/usr/bin/env python3
"""
png2chr.py — Convert indexed PNG tile sheets to NES CHR-ROM binary.
====================================================================

Reads one or two 128×128 PNG images (background + sprites) drawn in
Procreate (or any editor) using exact NES palette colors, and outputs
an 8 KiB CHR-ROM file ready to embed in the cartridge.

Usage:
  # Convert both pattern tables:
  python3 tools/png2chr.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png -o assets/chr/tiles.chr

  # Convert background only (sprites filled with zeros):
  python3 tools/png2chr.py assets/chr/bg_tiles.png -o assets/chr/tiles.chr

  # Validate colors without converting:
  python3 tools/png2chr.py --validate assets/chr/bg_tiles.png --palette assets/palettes/overworld.pal

Requirements:
  pip install Pillow
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("ERROR: Pillow is required.  Install with:  pip install Pillow", file=sys.stderr)
    sys.exit(1)


# ─── NES master palette (2C02G canonical) ────────────────────────────────────
# Maps NES color index ($00–$3F) → (R, G, B)
NES_MASTER_PALETTE: dict[int, tuple[int, int, int]] = {
    0x00: (0x58, 0x58, 0x58), 0x01: (0x00, 0x23, 0x8C), 0x02: (0x00, 0x13, 0x9B), 0x03: (0x2D, 0x05, 0x85),
    0x04: (0x5D, 0x00, 0x52), 0x05: (0x7A, 0x00, 0x17), 0x06: (0x7A, 0x08, 0x00), 0x07: (0x61, 0x18, 0x00),
    0x08: (0x3B, 0x24, 0x00), 0x09: (0x01, 0x3A, 0x00), 0x0A: (0x00, 0x40, 0x00), 0x0B: (0x00, 0x3C, 0x14),
    0x0C: (0x00, 0x36, 0x50), 0x0D: (0x00, 0x00, 0x00), 0x0E: (0x00, 0x00, 0x00), 0x0F: (0x00, 0x00, 0x00),
    0x10: (0xA0, 0xA0, 0xA0), 0x11: (0x00, 0x63, 0xD1), 0x12: (0x1B, 0x4A, 0xFF), 0x13: (0x6E, 0x29, 0xFF),
    0x14: (0xB3, 0x1A, 0xA8), 0x15: (0xD8, 0x18, 0x53), 0x16: (0xD8, 0x28, 0x00), 0x17: (0xBE, 0x4A, 0x10),
    0x18: (0x8A, 0x6C, 0x00), 0x19: (0x37, 0x86, 0x00), 0x1A: (0x00, 0x94, 0x00), 0x1B: (0x00, 0x8C, 0x3C),
    0x1C: (0x00, 0x7E, 0x8C), 0x1D: (0x00, 0x00, 0x00), 0x1E: (0x00, 0x00, 0x00), 0x1F: (0x00, 0x00, 0x00),
    0x20: (0xF8, 0xF8, 0xF8), 0x21: (0x3C, 0xBC, 0xFC), 0x22: (0x68, 0x88, 0xFC), 0x23: (0x98, 0x78, 0xF8),
    0x24: (0xF8, 0x78, 0xF8), 0x25: (0xF8, 0x58, 0x98), 0x26: (0xF8, 0x78, 0x58), 0x27: (0xFC, 0xA0, 0x44),
    0x28: (0xF8, 0xB8, 0x00), 0x29: (0xB8, 0xF8, 0x18), 0x2A: (0x58, 0xD8, 0x54), 0x2B: (0x58, 0xF8, 0x98),
    0x2C: (0x00, 0xE8, 0xD8), 0x2D: (0x78, 0x78, 0x78), 0x2E: (0x00, 0x00, 0x00), 0x2F: (0x00, 0x00, 0x00),
    0x30: (0xFC, 0xFC, 0xFC), 0x31: (0xA4, 0xE4, 0xFC), 0x32: (0xB8, 0xB8, 0xF8), 0x33: (0xD8, 0xB8, 0xF8),
    0x34: (0xF8, 0xB8, 0xF8), 0x35: (0xF8, 0xA4, 0xC0), 0x36: (0xF0, 0xD0, 0xB0), 0x37: (0xFC, 0xE0, 0xA8),
    0x38: (0xF8, 0xD8, 0x78), 0x39: (0xD8, 0xF8, 0x78), 0x3A: (0xB8, 0xF8, 0xB8), 0x3B: (0xB8, 0xF8, 0xD8),
    0x3C: (0x00, 0xFC, 0xFC), 0x3D: (0xD8, 0xD8, 0xD8), 0x3E: (0x00, 0x00, 0x00), 0x3F: (0x00, 0x00, 0x00),
}

# Transparency key color (magenta) — used by sprite tiles
TRANSPARENT_KEY = (0xFF, 0x00, 0xFF)

# Build a reverse lookup: RGB → NES index
_RGB_TO_NES: dict[tuple[int, int, int], int] = {rgb: idx for idx, rgb in NES_MASTER_PALETTE.items()}


# ─── Helpers ─────────────────────────────────────────────────────────────────

def rgb_distance(a: tuple[int, int, int], b: tuple[int, int, int]) -> int:
    """Manhattan distance between two RGB colors."""
    return abs(a[0] - b[0]) + abs(a[1] - b[1]) + abs(a[2] - b[2])


def nearest_nes_color(rgb: tuple[int, int, int]) -> tuple[int, int]:
    """Find the closest NES palette entry.  Returns (nes_index, distance)."""
    best_idx = 0x0F
    best_dist = 999
    for idx, pal_rgb in NES_MASTER_PALETTE.items():
        d = rgb_distance(rgb, pal_rgb)
        if d < best_dist:
            best_dist = d
            best_idx = idx
    return best_idx, best_dist


def load_pal_file(path: Path) -> list[tuple[int, int, int]]:
    """Load a .pal file (space-separated NES hex IDs) and return RGB list."""
    colors: list[tuple[int, int, int]] = []
    for line in path.read_text().splitlines():
        line = line.split("#")[0].strip()
        if not line:
            continue
        for token in line.split():
            nes_id = int(token, 16)
            if nes_id in NES_MASTER_PALETTE:
                colors.append(NES_MASTER_PALETTE[nes_id])
            else:
                print(f"WARNING: NES ID ${nes_id:02X} not in master palette", file=sys.stderr)
                colors.append((0, 0, 0))
    return colors


def get_unique_colors(img: Image.Image) -> set[tuple[int, int, int]]:
    """Get unique RGB colors in an image."""
    pixels = img.convert("RGB").getdata()
    return set(pixels)


def map_pixel_to_index(
    rgb: tuple[int, int, int],
    sub_palette: list[tuple[int, int, int]],
    tolerance: int = 8,
) -> int:
    """Map an RGB pixel to a 2-bit index (0–3) within a sub-palette.

    Color matching uses Manhattan distance with a tolerance threshold.
    Magenta (#FF00FF) always maps to index 0 (transparent).
    """
    if rgb == TRANSPARENT_KEY:
        return 0

    for idx, pal_color in enumerate(sub_palette):
        if rgb_distance(rgb, pal_color) <= tolerance:
            return idx

    # Fallback: nearest
    best_i = 0
    best_d = 999
    for i, pc in enumerate(sub_palette):
        d = rgb_distance(rgb, pc)
        if d < best_d:
            best_d = d
            best_i = i
    return best_i


def pixels_to_chr_tile(pixels_8x8: list[list[int]]) -> bytes:
    """Convert 8×8 grid of 2-bit indices to 16-byte NES tile (2 bitplanes)."""
    plane0 = []
    plane1 = []
    for row in pixels_8x8:
        b0 = 0
        b1 = 0
        for col in range(8):
            px = row[col] if col < len(row) else 0
            bit = 7 - col
            if px & 1:
                b0 |= 1 << bit
            if px & 2:
                b1 |= 1 << bit
        plane0.append(b0)
        plane1.append(b1)
    return bytes(plane0) + bytes(plane1)


def png_to_pattern_table(
    img: Image.Image,
    sub_palette: list[tuple[int, int, int]],
    tolerance: int = 8,
) -> bytearray:
    """Convert a 128×128 PNG to a 4 KiB NES pattern table (256 tiles).

    Tiles are arranged in reading order: left-to-right, top-to-bottom,
    in 8×8 blocks across the 128-px-wide image.
    """
    if img.size != (128, 128):
        print(f"WARNING: Image is {img.size}, expected (128, 128). Resizing.", file=sys.stderr)
        img = img.resize((128, 128), Image.NEAREST)

    rgb_img = img.convert("RGB")
    table = bytearray(4096)  # 256 tiles × 16 bytes

    tile_idx = 0
    for tile_row in range(16):      # 16 rows of tiles
        for tile_col in range(16):   # 16 columns of tiles
            pixels_8x8: list[list[int]] = []
            for py in range(8):
                row_indices: list[int] = []
                for px in range(8):
                    x = tile_col * 8 + px
                    y = tile_row * 8 + py
                    rgb = rgb_img.getpixel((x, y))
                    row_indices.append(map_pixel_to_index(rgb, sub_palette, tolerance))
                pixels_8x8.append(row_indices)

            tile_bytes = pixels_to_chr_tile(pixels_8x8)
            offset = tile_idx * 16
            table[offset:offset + 16] = tile_bytes
            tile_idx += 1

    return table


# ─── Validation ──────────────────────────────────────────────────────────────

def validate_image(img_path: Path, allowed_colors: list[tuple[int, int, int]] | None = None, tolerance: int = 8) -> bool:
    """Check that every pixel in the image uses an allowed NES color.

    If allowed_colors is None, validates against the full NES master palette.
    Returns True if valid.
    """
    img = Image.open(img_path).convert("RGB")
    w, h = img.size
    errors = 0
    unique_bad: set[tuple[int, int, int]] = set()

    check_against = allowed_colors if allowed_colors else list(NES_MASTER_PALETTE.values())

    for y in range(h):
        for x in range(w):
            rgb = img.getpixel((x, y))
            if rgb == TRANSPARENT_KEY:
                continue
            found = False
            for c in check_against:
                if rgb_distance(rgb, c) <= tolerance:
                    found = True
                    break
            if not found:
                errors += 1
                unique_bad.add(rgb)

    if errors:
        print(f"VALIDATION FAILED: {img_path.name}")
        print(f"  {errors} pixel(s) with unrecognized colors:")
        for c in sorted(unique_bad):
            nes_idx, dist = nearest_nes_color(c)
            nes_rgb = NES_MASTER_PALETTE[nes_idx]
            print(f"    #{c[0]:02X}{c[1]:02X}{c[2]:02X} → nearest NES ${nes_idx:02X} (#{nes_rgb[0]:02X}{nes_rgb[1]:02X}{nes_rgb[2]:02X}, distance={dist})")
        return False

    print(f"VALIDATION OK: {img_path.name} — all pixels match NES palette (tolerance={tolerance})")
    unique = get_unique_colors(img)
    print(f"  Unique colors: {len(unique)}")
    for c in sorted(unique):
        nes_idx, dist = nearest_nes_color(c)
        print(f"    #{c[0]:02X}{c[1]:02X}{c[2]:02X} → NES ${nes_idx:02X} (distance={dist})")
    return True


# ─── Main ────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert Procreate PNG tile sheets to NES CHR-ROM binary.",
        epilog="See docs/graphics-workflow.md for the full artist guide.",
    )
    parser.add_argument("images", nargs="*", help="PNG file(s): [bg_tiles.png] [spr_tiles.png]")
    parser.add_argument("-o", "--output", default="assets/chr/tiles.chr", help="Output CHR file (default: assets/chr/tiles.chr)")
    parser.add_argument("--validate", action="store_true", help="Validate colors only (no conversion)")
    parser.add_argument("--palette", type=str, default=None, help=".pal file for validation (NES hex IDs)")
    parser.add_argument("--tolerance", type=int, default=8, help="Color matching tolerance (Manhattan distance, default: 8)")
    parser.add_argument(
        "--bg-palette", type=str, default=None,
        help="NES hex IDs for BG sub-palette, e.g. '21,30,10,00'. Used for index mapping.",
    )
    parser.add_argument(
        "--spr-palette", type=str, default=None,
        help="NES hex IDs for sprite sub-palette, e.g. '0F,36,16,06'. Used for index mapping.",
    )

    args = parser.parse_args()

    if not args.images:
        parser.print_help()
        sys.exit(1)

    # --- Validation mode ---
    if args.validate:
        allowed = None
        if args.palette:
            allowed = load_pal_file(Path(args.palette))
        ok = True
        for img_path in args.images:
            if not validate_image(Path(img_path), allowed, args.tolerance):
                ok = False
        sys.exit(0 if ok else 1)

    # --- Parse sub-palettes ---
    def parse_pal_arg(s: str | None, default_ids: list[int]) -> list[tuple[int, int, int]]:
        if s:
            ids = [int(x.strip(), 16) for x in s.split(",")]
        else:
            ids = default_ids
        return [NES_MASTER_PALETTE.get(i, (0, 0, 0)) for i in ids]

    bg_sub = parse_pal_arg(args.bg_palette, [0x21, 0x30, 0x10, 0x00])
    spr_sub = parse_pal_arg(args.spr_palette, [0x0F, 0x36, 0x16, 0x06])

    # --- Convert ---
    chr_data = bytearray(8192)  # 8 KiB

    # Pattern table 0 — background
    bg_path = Path(args.images[0])
    print(f"Converting BG tiles: {bg_path}")
    bg_img = Image.open(bg_path)
    bg_table = png_to_pattern_table(bg_img, bg_sub, args.tolerance)
    chr_data[0:4096] = bg_table

    # Pattern table 1 — sprites
    if len(args.images) >= 2:
        spr_path = Path(args.images[1])
        print(f"Converting sprite tiles: {spr_path}")
        spr_img = Image.open(spr_path)
        spr_table = png_to_pattern_table(spr_img, spr_sub, args.tolerance)
        chr_data[4096:8192] = spr_table
    else:
        print("No sprite sheet provided — sprite pattern table will be empty.")

    # --- Write output ---
    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(chr_data)
    print(f"Wrote {len(chr_data)} bytes to {out_path}")


if __name__ == "__main__":
    main()
