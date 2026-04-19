/*
 * screen_gameover.c -- game-over screen
 * ============================================================
 */

#include "neslib.h"
#include "game.h"
#include "input.h"
#include "screen.h"

/* ------------------------------------------------------------------
 * screen_gameover_enter()
 * ------------------------------------------------------------------ */
void screen_gameover_enter(void)
{
    /* TODO: show score, play game-over jingle */
    g_state = STATE_GAMEOVER;
}

/* ------------------------------------------------------------------
 * screen_gameover_tick()
 * ------------------------------------------------------------------ */
void screen_gameover_tick(void)
{
    if (pad1_new & BTN_START) {
        screen_title_enter();
    }
}
