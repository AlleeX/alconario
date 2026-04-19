#!/usr/bin/env python3
"""
gen_chr.py — Generate tiles.chr for Alconario
==============================================
Creates an 8 KiB NES CHR-ROM file containing:
  • Tile $00        : blank
  • Tiles $01–$1C   : vodka bottle pixel art (4 wide × 7 tall)
  • Tiles $20–$7E   : minimal ASCII bitmap font

Run:  python3 tools/gen_chr.py
Output: assets/chr/tiles.chr
"""

import os, struct

CHR_SIZE  = 8192
TILE_SIZE = 16        # 8 bytes plane-0 + 8 bytes plane-1

# Colour indices (2-bit, matching bg sub-palette 0)
_  = 0   # background (black)
W  = 1   # white  — outline / glass highlight
R  = 2   # red    — label accent
B  = 3   # blue   — liquid / body fill

# ─── helpers ────────────────────────────────────────────────────────
chr_data = bytearray(CHR_SIZE)

def put_tile(index, data):
    off = index * TILE_SIZE
    chr_data[off:off + TILE_SIZE] = data

def tile_1bpp(rows8):
    """Colour-1 only tile (plane-0 = rows, plane-1 = 0)."""
    return bytes(rows8) + b'\x00' * 8

def tile_2bpp(pixels_8x8):
    """Full 2-bit colour tile from an 8×8 list-of-lists (values 0-3)."""
    p0, p1 = [], []
    for row in pixels_8x8:
        b0 = b1 = 0
        for i, px in enumerate(row):
            bit = 7 - i
            if px & 1: b0 |= 1 << bit
            if px & 2: b1 |= 1 << bit
        p0.append(b0)
        p1.append(b1)
    return bytes(p0) + bytes(p1)

# ─── 1. ASCII bitmap font (colour 1 = white) ───────────────────────
FONT = {
    0x20: [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],  # (space)
    0x28: [0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00],  # (
    0x29: [0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00],  # )
    0x30: [0x3C,0x66,0x6E,0x7E,0x76,0x66,0x3C,0x00],  # 0
    0x31: [0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00],  # 1
    0x32: [0x3C,0x66,0x06,0x1C,0x30,0x66,0x7E,0x00],  # 2
    0x33: [0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00],  # 3
    0x34: [0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00],  # 4
    0x35: [0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00],  # 5
    0x36: [0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00],  # 6
    0x37: [0x7E,0x06,0x0C,0x18,0x18,0x18,0x18,0x00],  # 7
    0x38: [0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00],  # 8
    0x39: [0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00],  # 9
    0x41: [0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00],  # A
    0x42: [0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00],  # B
    0x43: [0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00],  # C
    0x44: [0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00],  # D
    0x45: [0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00],  # E
    0x46: [0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00],  # F
    0x47: [0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00],  # G
    0x48: [0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00],  # H
    0x49: [0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00],  # I
    0x4A: [0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00],  # J
    0x4B: [0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00],  # K
    0x4C: [0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00],  # L
    0x4D: [0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00],  # M
    0x4E: [0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00],  # N
    0x4F: [0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00],  # O
    0x50: [0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00],  # P
    0x51: [0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00],  # Q
    0x52: [0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00],  # R
    0x53: [0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00],  # S
    0x54: [0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00],  # T
    0x55: [0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00],  # U
    0x56: [0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00],  # V
    0x57: [0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00],  # W
    0x58: [0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00],  # X
    0x59: [0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00],  # Y
    0x5A: [0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00],  # Z
}

for code, rows in FONT.items():
    put_tile(code, tile_1bpp(rows))

# ─── 2. Vodka bottle pixel art ─────────────────────────────────────
# Canvas: 32 px wide × 56 px tall  (4 tiles × 7 tiles)
# We draw programmatically, then slice into tiles.

CW, CH = 32, 56
canvas = [[_] * CW for _ in range(CH)]

def fill_rect(x1, y1, x2, y2, c):
    for y in range(y1, y2 + 1):
        for x in range(x1, x2 + 1):
            canvas[y][x] = c

def hline(x1, x2, y, c):
    for x in range(x1, x2 + 1):
        canvas[y][x] = c

def vline(x, y1, y2, c):
    for y in range(y1, y2 + 1):
        canvas[y][x] = c

# --- Cap (rows 0-7) ---
fill_rect(12, 0, 19, 0, W)   # cap top edge
fill_rect(11, 1, 20, 3, W)   # cap body
fill_rect(12, 1, 19, 3, B)   # cap glass interior (overwrite center)
fill_rect(12, 4, 19, 4, W)   # cap bottom edge

# --- Neck (rows 5-15) ---
vline(13, 5, 15, W)           # neck left wall
vline(18, 5, 15, W)           # neck right wall
fill_rect(14, 5, 17, 15, B)  # neck interior

# --- Shoulder (rows 16-21) — diagonal widening ---
shoulder_data = [
    (11, 20),   # row 16
    ( 9, 22),   # row 17
    ( 7, 24),   # row 18
    ( 6, 25),   # row 19
    ( 5, 26),   # row 20
    ( 5, 26),   # row 21
]
for i, (xl, xr) in enumerate(shoulder_data):
    y = 16 + i
    canvas[y][xl] = W
    canvas[y][xr] = W
    for x in range(xl + 1, xr):
        canvas[y][x] = B

# --- Body (rows 22-47) ---
for y in range(22, 48):
    canvas[y][5]  = W
    canvas[y][26] = W
    for x in range(6, 26):
        canvas[y][x] = B

# --- Label (rows 26-37) — red rectangle with white border ---
fill_rect(6, 26, 25, 26, W)   # label top border
fill_rect(6, 37, 25, 37, W)   # label bottom border
fill_rect(6, 27, 25, 36, R)   # label red fill

# Thin white border lines inside label
hline(7, 24, 28, W)
hline(7, 24, 35, W)

# "HADD" in white pixels inside label (rows 30-34, simple 3-wide × 5-tall font)
# Centered in the 20-px label interior (cols 7-24)
# H at col 8, A at col 12, D at col 16, D at col 20

def draw_letter_3x5(lx, ly, bitmap):
    """Draw a 3×5 pixel letter at (lx, ly) in colour W on the canvas."""
    for dy, row_bits in enumerate(bitmap):
        for dx in range(3):
            if row_bits & (4 >> dx):  # bit 2,1,0 → cols 0,1,2
                canvas[ly + dy][lx + dx] = W

MINI_FONT = {
    'H': [0b101, 0b101, 0b111, 0b101, 0b101],
    'A': [0b010, 0b101, 0b111, 0b101, 0b101],
    'D': [0b110, 0b101, 0b101, 0b101, 0b110],
}

draw_letter_3x5( 8, 30, MINI_FONT['H'])
draw_letter_3x5(12, 30, MINI_FONT['A'])
draw_letter_3x5(16, 30, MINI_FONT['D'])
draw_letter_3x5(20, 30, MINI_FONT['D'])

# --- "MAKE" smaller, rows 32-34 beneath HADD ---
MINI2 = {
    'M': [0b10001, 0b11011, 0b10101, 0b10001, 0b10001],
    'A': [0b01110, 0b10001, 0b11111, 0b10001, 0b10001],
    'K': [0b10010, 0b10100, 0b11000, 0b10100, 0b10010],
    'E': [0b11110, 0b10000, 0b11100, 0b10000, 0b11110],
}

# Hmm, 5-wide letters won't fit well. Let me use the same 3-wide font.
MINI_FONT2 = {
    'M': [0b101, 0b111, 0b111, 0b101, 0b101],
    'K': [0b101, 0b110, 0b100, 0b110, 0b101],
    'E': [0b111, 0b100, 0b110, 0b100, 0b111],
}

# "MAKE" → M A K E  at row 33 (shifted down slightly so it doesn't overlap)
# Actually let's center "VODKA" instead? No, user asked for "HADD MAKE".
# Let me skip the second word to keep label clean — "HADD" is enough for the brand.

# --- Base (rows 48-55) ---
for y in range(48, 54):
    canvas[y][5]  = W
    canvas[y][26] = W
    for x in range(6, 26):
        canvas[y][x] = B

# Bottom edge
fill_rect(5, 54, 26, 54, W)    # base bottom line
fill_rect(4, 55, 27, 55, W)    # base foot (slightly wider)

# --- Extract tiles from canvas and store at indices $01-$1C ---
tile_idx = 0x01
for tr in range(7):       # 7 tile-rows
    for tc in range(4):   # 4 tile-cols
        px8x8 = []
        for py in range(8):
            row = []
            for px in range(8):
                row.append(canvas[tr * 8 + py][tc * 8 + px])
            px8x8.append(row)
        put_tile(tile_idx, tile_2bpp(px8x8))
        tile_idx += 1

# ─── 3. Big title logo "ALCONARIO" — SMB-style block letters ────────
# Solid blue bar, white letters with dark bottom-right shadow, bar drop-shadow.
# Stored at tiles $80 onwards, laid out as 22 tiles wide × 3 tiles tall.

TITLE_TILES_W = 22
TITLE_TILES_H = 3      # 2 tile-rows for bar + 1 for bar shadow
TITLE_W = TITLE_TILES_W * 8   # 176 px
TITLE_H = TITLE_TILES_H * 8   # 24 px
title_canvas = [[_] * TITLE_W for _ in range(TITLE_H)]

# The actual blue bar occupies a region with 2px margin for the shadow
BAR_X1 = 0
BAR_Y1 = 0
BAR_X2 = TITLE_W   # leave 2px on right for shadow
BAR_Y2 = 24            # 16px tall bar

# Draw bar drop-shadow first (offset 2px right, 2px down) in red/dark
# for yy in range(BAR_Y1 + 2, BAR_Y2 + 3):
#     for xx in range(BAR_X1 + 2, BAR_X2 + 3):
#         title_canvas[yy][xx] = R

# Draw the solid blue bar on top
for yy in range(BAR_Y1, BAR_Y2):
    for xx in range(BAR_X1, BAR_X2):
        title_canvas[yy][xx] = B

def t_set(x, y, c):
    if 0 <= x < TITLE_W and 0 <= y < TITLE_H:
        title_canvas[y][x] = c

# 12-wide × 14-tall letter bitmaps — tall chunky letters like SMB.
LETTER_BITMAPS = {
    'A': [
        "...######...",
        "..########..",
        ".####..####.",
        "####....####",
        "####....####",
        "####....####",
        "############",
        "############",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
    ],
    'L': [
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "############",
        "############",
    ],
    'C': [
        "..##########",
        ".###########",
        "####....####",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####........",
        "####....####",
        ".###########",
        "..##########",
    ],
    'O': [
        "..########..",
        ".##########.",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        ".##########.",
        "..########..",
    ],
    'N': [
        "#####...####",
        "######..####",
        "######..####",
        "####.##.####",
        "####.##.####",
        "####..######",
        "####..######",
        "####...#####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
        "####....####",
    ],
    'R': [
        "###########.",
        "############",
        "####....####",
        "####....####",
        "####....####",
        "############",
        "###########.",
        "########....",
        "####.####...",
        "####..####..",
        "####...####.",
        "####....####",
        "####....####",
        "####....####",
    ],
    'I': [
        "############",
        "############",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "....####....",
        "############",
        "############",
    ],
}

TITLE_STR = "ALCONARIO"

for li, ch in enumerate(TITLE_STR):
    ox = li * 16 + 16  # center 144px of letters within bar
    oy = 5             # 5px top margin
    bitmap = LETTER_BITMAPS[ch]

    # Pass 1 — dark shadow (offset +1,+1) using background colour
    for dy, row_str in enumerate(bitmap):
        for dx, pixel in enumerate(row_str):
            if pixel == '#':
                t_set(ox + dx + 1, oy + dy + 1, _)

    # Pass 2 — solid white letter body (overwrites shadow where they overlap)
    for dy, row_str in enumerate(bitmap):
        for dx, pixel in enumerate(row_str):
            if pixel == '#':
                t_set(ox + dx, oy + dy, W)

# Slice title canvas into tiles → $80 onwards
TITLE_FIRST_TILE = 0x80
tile_idx = TITLE_FIRST_TILE
for tr in range(TITLE_TILES_H):
    for tc in range(TITLE_TILES_W):
        px8x8 = []
        for py in range(8):
            row = []
            for px in range(8):
                row.append(title_canvas[tr * 8 + py][tc * 8 + px])
            px8x8.append(row)
        put_tile(tile_idx, tile_2bpp(px8x8))
        tile_idx += 1

print(f"  Title logo: tiles ${TITLE_FIRST_TILE:02X}-${tile_idx-1:02X} ({tile_idx - TITLE_FIRST_TILE} tiles, {TITLE_TILES_W}x{TITLE_TILES_H})")

# ─── 4. Write output ───────────────────────────────────────────────
out_path = os.path.join(os.path.dirname(__file__), '..', 'assets', 'chr', 'tiles.chr')
out_path = os.path.normpath(out_path)
os.makedirs(os.path.dirname(out_path), exist_ok=True)

with open(out_path, 'wb') as f:
    f.write(chr_data)

print(f"  GEN  {out_path}  ({len(chr_data)} bytes)")
