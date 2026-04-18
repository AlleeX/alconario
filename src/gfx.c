/*
 * gfx.c — palette loading and background (nametable) helpers
 * ============================================================
 *
 * THE NES COLOUR SYSTEM
 * ---------------------
 * The NES can display 64 distinct colours (the "NES palette").
 * At runtime only 32 of those can be "loaded" at once:
 *   - 16 bytes for BACKGROUND palettes (4 palettes × 4 colours each)
 *   - 16 bytes for SPRITE palettes     (4 palettes × 4 colours each)
 *
 * Each group is organised as 4 sub-palettes of 4 entries:
 *   index 0 of every sub-palette = the universal background colour
 *                                  (shared by ALL sub-palettes).
 *   indices 1-3 = the 3 usable colours for that sub-palette.
 *
 * Colour values are NES palette indexes (NOT RGB).
 * Reference: https://www.nesdev.org/wiki/PPU_palettes
 * Interactive picker: https://www.nesdev.org/palgen/
 *
 * COMMON COLOUR VALUES
 * --------------------
 *   $0F = black (darkest)    $30 = white
 *   $16 = red                $2A = green
 *   $11 = dark blue          $21 = medium blue
 *   $27 = brown              $29 = dark green
 *   $17 = dark brown         $12 = dark red
 *
 * THE NAMETABLE (background tiles)
 * ---------------------------------
 * The NES background is made of 8×8-pixel tiles arranged on a 32×30 grid
 * (= 256×240 pixels = one full screen).
 * Each cell stores one byte: the tile index to draw from the CHR ROM.
 * After the 32×30 tile grid comes a 64-byte "attribute table" that assigns
 * a 2-bit sub-palette index to each 16×16-pixel region of the screen.
 *
 * NAMETABLE_A ($2000) is the standard first nametable address (neslib macro).
 *
 * vram_adr(addr) — set the PPU VRAM write pointer.
 * vram_fill(v,n) — write byte v into VRAM n times (fast bulk fill).
 */

#include "neslib.h"   /* pal_bg, pal_spr, vram_adr, vram_fill, NAMETABLE_A */
#include "gfx.h"      /* palette array declarations */

/* ------------------------------------------------------------------
 * Background palettes — 4 sub-palettes × 4 colours = 16 bytes total.
 *
 * Layout:  { bg_universal, colour1, colour2, colour3,   <- sub-palette 0
 *             bg_universal, colour1, colour2, colour3,   <- sub-palette 1
 *             …                                        }
 *
 * The first byte of EACH sub-palette ($0F here) is the universal
 * background colour shared across all four sub-palettes.
 * ------------------------------------------------------------------ */
const uint8_t palette_bg[16] = {
    /* Sub-palette 0 — dark/sky (used for sky tiles) */
    0x0F,  /* universal BG = black          */
    0x30,  /* white  — clouds / text        */
    0x16,  /* red    — accent               */
    0x11,  /* blue   — sky details          */

    /* Sub-palette 1 — greens (grass, trees) */
    0x0F,
    0x29,  /* dark green                    */
    0x1A,  /* medium green                  */
    0x09,  /* yellowish green               */

    /* Sub-palette 2 — earth tones (ground, bricks) */
    0x0F,
    0x27,  /* brown                         */
    0x17,  /* dark brown                    */
    0x07,  /* very dark brown               */

    /* Sub-palette 3 — neutral / UI */
    0x0F,
    0x30,  /* white                         */
    0x10,  /* grey                          */
    0x00   /* dark grey                     */
};

/* ------------------------------------------------------------------
 * Sprite palettes — same structure as background palettes.
 * NOTE: index 0 of each sprite sub-palette is TRANSPARENT (not drawn),
 * regardless of the colour value written there.  Convention is to write
 * the same universal BG colour ($0F) so palette dumps look consistent.
 * ------------------------------------------------------------------ */
const uint8_t palette_sp[16] = {
    /* Sub-palette 0 — player sprite */
    0x0F,  /* transparent                   */
    0x16,  /* red   — main body colour      */
    0x27,  /* brown — skin / hair           */
    0x30,  /* white — highlights            */

    /* Sub-palette 1 — player alternate / powerup */
    0x0F,
    0x11,  /* blue                          */
    0x21,  /* lighter blue                  */
    0x30,  /* white                         */

    /* Sub-palette 2 — enemies */
    0x0F,
    0x2A,  /* green                         */
    0x1A,  /* darker green                  */
    0x30,  /* white                         */

    /* Sub-palette 3 — items / effects */
    0x0F,
    0x12,  /* dark red                      */
    0x22,  /* medium red                    */
    0x30   /* white                         */
};

/* ------------------------------------------------------------------
 * gfx_load_palettes()
 * Upload both palette arrays to the PPU palette RAM.
 * Must be called while the PPU is off (ppu_off) OR during vblank.
 * ------------------------------------------------------------------ */
void gfx_load_palettes(void)
{
    pal_bg(palette_bg);   /* copies 16 bytes to PPU $3F00-$3F0F */
    pal_spr(palette_sp);  /* copies 16 bytes to PPU $3F10-$3F1F */
}

/* ------------------------------------------------------------------
 * gfx_clear_nametable()
 * Wipe nametable 0 (the visible screen) to a blank state:
 *   - All 32×30 = 960 tile cells filled with tile index 0.
 *   - All 64 attribute bytes set to 0 (sub-palette 0 everywhere).
 *
 * Call this when entering a new screen so no stale tiles linger.
 * Must be called while the PPU is off (inside a ppu_off / ppu_on_all
 * bracket) to avoid visual corruption.
 * ------------------------------------------------------------------ */
void gfx_clear_nametable(void)
{
    /*
     * Point the PPU write address to the start of nametable 0.
     * NAMETABLE_A is a neslib constant equal to $2000.
     */
    vram_adr(NAMETABLE_A);

    /*
     * Fill the 32×30 tile grid with tile index 0 (blank / sky tile).
     * 32 columns × 30 rows = 960 bytes.
     */
    vram_fill(0, 32 * 30);

    /*
     * Advance to the attribute table that follows the tile grid.
     * $2000 + 960 ($3C0) = $23C0.
     * Fill all 64 attribute bytes with 0 → sub-palette 0 everywhere.
     */
    vram_adr(NAMETABLE_A + 32 * 30);
    vram_fill(0, 64);
}
