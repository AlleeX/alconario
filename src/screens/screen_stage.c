/*
 * screen_stage.c -- "STAGE X" title card
 * ============================================================
 *
 * Shown briefly before gameplay begins.  Auto-advances after a
 * timeout or when the player presses START.
 */

#include "neslib.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "screen.h"
#include "sfx.h"

/* How many frames to show the stage card (~3 seconds at 60 fps). */
#define STAGE_TIMEOUT  180

/* Module-local state */
static unsigned char stage_timer;
static unsigned char current_stage;

/* ------------------------------------------------------------------
 * screen_stage_enter()
 * ------------------------------------------------------------------ */
void screen_stage_enter(unsigned char stage_num)
{
    static const char prefix[] = "STAGE ";
    char buf[9];
    unsigned char i;

    current_stage = stage_num;

    music_stop();
    ppu_off();

    gfx_clear_nametable();
    gfx_load_palettes();

    /* Build "STAGE X" string */
    for (i = 0; i < 6; ++i) buf[i] = prefix[i];
    buf[6] = '0' + current_stage;
    buf[7] = '\0';
    gfx_draw_text(13, 13, buf);

    ppu_on_all();
    stage_timer = 0;
    g_state = STATE_STAGE;
}

/* ------------------------------------------------------------------
 * screen_stage_tick()
 * ------------------------------------------------------------------ */
void screen_stage_tick(void)
{
    ++stage_timer;

    if ((pad1_new & BTN_START) || stage_timer >= STAGE_TIMEOUT) {
        sfx_play(SFX_MENU_SELECT, SFX_CH0);
        screen_play_enter();
    }
}
