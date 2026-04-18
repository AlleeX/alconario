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

#endif /* GFX_H */
