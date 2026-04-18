/*
 * gfx.h — palette tables and rendering helpers
 * ============================================================
 *
 * Exposes the two 16-byte palette arrays (background + sprites) and
 * two helper functions used when entering a new screen.
 *
 * All PPU writes must happen while the PPU is off (ppu_off / ppu_on_all)
 * OR inside the vblank window — otherwise the image will glitch.
 */
#ifndef GFX_H
#define GFX_H

#include <stdint.h>

/*
 * palette_bg[16] — 4 background sub-palettes × 4 NES colour indexes.
 * palette_sp[16] — 4 sprite      sub-palettes × 4 NES colour indexes.
 *
 * Edit the values in gfx.c to change the game's colour scheme.
 * Full NES colour reference: https://www.nesdev.org/wiki/PPU_palettes
 */
extern const uint8_t palette_bg[16];
extern const uint8_t palette_sp[16];

/*
 * gfx_load_palettes() — upload both palettes to the PPU.
 * Call once per screen transition (inside a ppu_off / ppu_on_all block).
 */
void gfx_load_palettes(void);

/*
 * gfx_clear_nametable() — fill the visible nametable (screen) with
 * blank tiles and reset the attribute table to sub-palette 0.
 * Call before drawing a new screen to avoid leftover tile garbage.
 */
void gfx_clear_nametable(void);

/*
 * gfx_draw_text() — write an ASCII string into the nametable.
 * x, y are tile coordinates (0-31, 0-29).
 * Assumes CHR ROM maps ASCII codes directly as tile indices.
 * Must be called while the PPU is off.
 */
void gfx_draw_text(unsigned char x, unsigned char y, const char *str);

/*
 * gfx_draw_tile_rect() — blit a rectangular block of sequential tiles.
 * x, y       : top-left tile coordinates in the nametable.
 * w, h       : size in tiles.
 * first_tile : index of the first tile; tiles are stored row-major.
 * Must be called while the PPU is off.
 */
void gfx_draw_tile_rect(unsigned char x, unsigned char y,
                         unsigned char w, unsigned char h,
                         unsigned char first_tile);

#endif /* GFX_H */
