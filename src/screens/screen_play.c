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
    ppu_off();

    gfx_clear_nametable();
    gfx_load_palettes();

    /* Mock level screen -- HUD placeholder */
    gfx_draw_text(2, 2, "WORLD 1-1");
    gfx_draw_text(2, 4, "ALCONARIO x 3");

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
