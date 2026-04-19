#!/usr/bin/env python3
"""
validate_tiles.py — Quick validation of tile PNGs before conversion.
====================================================================

Checks:
  1. Image dimensions (must be 128×128)
  2. Color count per 8×8 tile (max 4)
  3. All colors are valid NES palette entries
  4. Reports which tiles violate constraints

Usage:
  python3 tools/validate_tiles.py assets/chr/bg_tiles.png
  python3 tools/validate_tiles.py assets/chr/bg_tiles.png assets/chr/spr_tiles.png
"""

from __future__ import annotations

import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("ERROR: Pillow is required.  Install with:  pip install Pillow", file=sys.stderr)
    sys.exit(1)

# Import the master palette from png2chr
sys.path.insert(0, str(Path(__file__).parent))
from png2chr import NES_MASTER_PALETTE, TRANSPARENT_KEY, rgb_distance


def validate(img_path: Path) -> bool:
    img = Image.open(img_path).convert("RGB")
    w, h = img.size
    errors = 0

    # Check dimensions
    if (w, h) != (128, 128):
        print(f"  ⚠ Dimensions: {w}×{h} (expected 128×128)")
        errors += 1

    # Check each 8×8 tile
    tiles_w = w // 8
    tiles_h = h // 8
    all_palette = list(NES_MASTER_PALETTE.values()) + [TRANSPARENT_KEY]

    for ty in range(tiles_h):
        for tx in range(tiles_w):
            tile_colors: set[tuple[int, int, int]] = set()
            bad_pixels: list[tuple[int, int, tuple[int, int, int]]] = []

            for py in range(8):
                for px in range(8):
                    x, y = tx * 8 + px, ty * 8 + py
                    rgb = img.getpixel((x, y))

                    tile_colors.add(rgb)

                    # Check if valid NES color
                    is_valid = False
                    for c in all_palette:
                        if rgb_distance(rgb, c) <= 8:
                            is_valid = True
                            break
                    if not is_valid:
                        bad_pixels.append((x, y, rgb))

            tile_idx = ty * tiles_w + tx

            if len(tile_colors) > 4:
                print(f"  ✗ Tile ${tile_idx:02X} ({tx},{ty}): {len(tile_colors)} colors (max 4)")
                errors += 1

            if bad_pixels:
                print(f"  ✗ Tile ${tile_idx:02X} ({tx},{ty}): {len(bad_pixels)} pixel(s) not in NES palette")
                for x, y, rgb in bad_pixels[:3]:
                    print(f"      ({x},{y}) = #{rgb[0]:02X}{rgb[1]:02X}{rgb[2]:02X}")
                if len(bad_pixels) > 3:
                    print(f"      ... and {len(bad_pixels) - 3} more")
                errors += 1

    if errors == 0:
        unique = set()
        for y in range(h):
            for x in range(w):
                unique.add(img.getpixel((x, y)))
        print(f"  ✓ All checks passed — {tiles_w * tiles_h} tiles, {len(unique)} unique colors")
        return True

    print(f"  {errors} issue(s) found")
    return False


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: validate_tiles.py <image.png> [image2.png ...]")
        sys.exit(1)

    ok = True
    for path in sys.argv[1:]:
        p = Path(path)
        print(f"\nValidating: {p.name}")
        if not validate(p):
            ok = False

    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
