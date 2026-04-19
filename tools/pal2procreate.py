#!/usr/bin/env python3
"""
pal2procreate.py — Generate Procreate .swatches palette files from NES .pal definitions.
========================================================================================

Creates importable Procreate palette files so the artist can use exact NES colors.

Usage:
  # Generate a .swatches file for a specific sub-palette:
  python3 tools/pal2procreate.py assets/palettes/overworld.pal -o assets/palettes/overworld.procreate.swatches

  # Generate individual sub-palette swatches (4 colors each):
  python3 tools/pal2procreate.py assets/palettes/overworld.pal --split -o assets/palettes/

  # Generate the full NES master palette:
  python3 tools/pal2procreate.py --master -o assets/palettes/nes_master.procreate.swatches

Procreate .swatches format:
  A .swatches file is actually a ZIP archive containing a JSON file (Swatches.json).
  This script creates that archive directly — just AirDrop to iPad and open.

Requirements:
  pip install Pillow  (only needed if you want color previews; otherwise stdlib only)
"""

from __future__ import annotations

import argparse
import json
import io
import sys
import zipfile
from pathlib import Path

# ─── NES master palette (2C02G) ─────────────────────────────────────────────
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


def rgb_to_hsb(r: int, g: int, b: int) -> tuple[float, float, float]:
    """Convert RGB (0–255) to HSB (0–1) for Procreate swatch format."""
    r_, g_, b_ = r / 255.0, g / 255.0, b / 255.0
    mx = max(r_, g_, b_)
    mn = min(r_, g_, b_)
    diff = mx - mn

    # Brightness
    v = mx

    # Saturation
    s = 0.0 if mx == 0 else diff / mx

    # Hue
    h = 0.0
    if diff != 0:
        if mx == r_:
            h = (g_ - b_) / diff % 6
        elif mx == g_:
            h = (b_ - r_) / diff + 2
        else:
            h = (r_ - g_) / diff + 4
        h /= 6.0
        if h < 0:
            h += 1.0

    return h, s, v


def make_swatch_entry(r: int, g: int, b: int) -> dict:
    """Create a single Procreate swatch color entry."""
    h, s, v = rgb_to_hsb(r, g, b)
    return {
        "hue": h,
        "saturation": s,
        "brightness": v,
        "alpha": 1.0,
        "colorSpace": 0,  # 0 = HSB
    }


def make_swatches_json(name: str, colors: list[tuple[int, int, int]]) -> str:
    """Create the Swatches.json content for a Procreate palette."""
    swatches = [make_swatch_entry(r, g, b) for r, g, b in colors]
    data = [{"name": name, "swatches": swatches}]
    return json.dumps(data, indent=2)


def write_swatches_file(path: Path, name: str, colors: list[tuple[int, int, int]]) -> None:
    """Write a Procreate .swatches ZIP file."""
    json_data = make_swatches_json(name, colors)
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("Swatches.json", json_data)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(buf.getvalue())
    print(f"Wrote {path} ({len(colors)} colors)")


def load_pal_file(path: Path) -> list[int]:
    """Load .pal file → list of NES hex IDs."""
    ids: list[int] = []
    for line in path.read_text().splitlines():
        line = line.split("#")[0].strip()
        if not line:
            continue
        for token in line.split():
            ids.append(int(token, 16))
    return ids


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate Procreate .swatches from NES palette definitions.")
    parser.add_argument("pal_file", nargs="?", help=".pal file with NES hex IDs")
    parser.add_argument("-o", "--output", default="palette.swatches", help="Output path")
    parser.add_argument("--master", action="store_true", help="Generate full NES 64-color master palette")
    parser.add_argument("--split", action="store_true", help="Split into 4-color sub-palette files")
    parser.add_argument("--name", default=None, help="Palette name (default: filename)")

    args = parser.parse_args()

    if args.master:
        # Full NES master palette
        colors = [NES_MASTER_PALETTE[i] for i in range(0x40)]
        out = Path(args.output)
        if out.is_dir():
            out = out / "NES_Master_Palette.swatches"
        write_swatches_file(out, "NES Master (2C02G)", colors)
        return

    if not args.pal_file:
        parser.print_help()
        sys.exit(1)

    pal_path = Path(args.pal_file)
    nes_ids = load_pal_file(pal_path)
    name = args.name or pal_path.stem

    if args.split:
        # Split into sub-palettes of 4
        out_dir = Path(args.output)
        out_dir.mkdir(parents=True, exist_ok=True)
        for i in range(0, len(nes_ids), 4):
            chunk = nes_ids[i:i + 4]
            colors = [NES_MASTER_PALETTE.get(c, (0, 0, 0)) for c in chunk]
            sub_name = f"{name}_sub{i // 4}"
            write_swatches_file(out_dir / f"{sub_name}.swatches", sub_name, colors)
    else:
        colors = [NES_MASTER_PALETTE.get(c, (0, 0, 0)) for c in nes_ids]
        out = Path(args.output)
        if out.is_dir():
            out = out / f"{name}.swatches"
        write_swatches_file(out, name, colors)


if __name__ == "__main__":
    main()
