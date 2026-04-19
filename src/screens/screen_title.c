/*
 * screen_title.c -- title / attract screen
 * ============================================================
 */

#include "neslib.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "screen.h"
#include "sfx.h"

/* ------------------------------------------------------------------
 * screen_title_enter()
 * Draw the title screen artwork and start music.
 * ------------------------------------------------------------------ */
void screen_title_enter(void)
{
    ppu_off();

    gfx_clear_nametable();
    gfx_load_palettes();

    /* Vodka bottle image (4x7 tiles, tiles $01-$1C) */
    gfx_draw_tile_rect(14, 2, 4, 7, 0x01);

    /* Title logo (22x3 tiles, SMB-style) */
    gfx_draw_tile_rect(5, 11, 22, 3, 0x80);
    gfx_draw_text(10, 18, "PRESS START");
    gfx_draw_text(8,  26, "(C) ALLEEX 2026");

    ppu_on_all();
    g_state = STATE_TITLE;

    music_play(0);
}

/* ------------------------------------------------------------------
 * screen_title_tick()
 * Wait for START -> transition to stage screen.
 * ------------------------------------------------------------------ */
void screen_title_tick(void)
{
    if (pad1_new & BTN_START) {
        sfx_play(SFX_MENU_SELECT, SFX_CH0);
        screen_stage_enter(1);
    }
}
