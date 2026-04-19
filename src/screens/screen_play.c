/*
 * screen_play.c -- active gameplay screen
 * ============================================================
 */

#include "neslib.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "screen.h"
#include "player.h"

/* ------------------------------------------------------------------
 * screen_play_enter()
 * Set up the level and start gameplay.
 * ------------------------------------------------------------------ */
void screen_play_enter(void)
{
    unsigned char i;

    ppu_off();

    gfx_clear_nametable();
    gfx_load_palettes();

    /* HUD */
    gfx_draw_text(2, 2, "WORLD 1-1");
    gfx_draw_text(2, 4, "ALCONARIO x 3");

    /*
     * Draw ground — two rows of tiles at the bottom of the screen.
     * Ground Y = 200px → tile row 25 (200/8 = 25).
     * Row 25: "surface" tiles (e.g. grass tops).
     * Row 26-29: "underground" dirt tiles.
     * We use tile $B2 for surface and $B3 for sub-ground; change
     * these to real art tile indices once you have them in the CHR.
     * For now any non-zero tile makes a visible ground strip.
     */
    /* Surface row (tile row 25) */
    vram_adr(NAMETABLE_A + 25 * 32);
    for (i = 0; i < 32; ++i) vram_put(0x42);  /* 'B' char = visible ground */

    /* Underground rows (tile rows 26-29) */
    vram_adr(NAMETABLE_A + 26 * 32);
    vram_fill(0x58, 32 * 4);  /* 'X' char = underground fill */

    player_init();
    ppu_on_all();
    g_state = STATE_PLAY;
}

/* ------------------------------------------------------------------
 * screen_play_tick()
 * Per-frame gameplay logic.
 * ------------------------------------------------------------------ */
void screen_play_tick(void)
{
    player_update();
    player_draw();

    /* TODO: update enemies, scroll, collisions, score... */

    if (pad1_new & BTN_START) {
        screen_pause_enter();
    }
}
