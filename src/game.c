/*
 * game.c -- top-level game state machine (dispatcher)
 *
 * Owns the global game state and dispatches each frame to the active
 * screen's tick function.  All screen-specific logic lives in
 * src/screens/screen_*.c.
 *
 * STATE DIAGRAM
 *   BOOT -> TITLE -[START]-> STAGE -[START/timeout]-> PLAY -[START]-> PAUSE
 *             ^                                         |                |
 *             |                                       [die]          [START]
 *             +-----------------------------------------+                |
 *                  GAMEOVER <-----------------------------------------+
 */

#include "neslib.h"
#include "game.h"
#include "screen.h"

/* The current active state -- readable from any module via game.h */
game_state_t g_state;

void game_init(void)
{
    pal_bright(4);
    bank_bg(0);
    bank_spr(1);
    screen_title_enter();
}

void game_tick(void)
{
    oam_clear();
    switch (g_state) {
    case STATE_TITLE:    screen_title_tick();    break;
    case STATE_STAGE:    screen_stage_tick();    break;
    case STATE_PLAY:     screen_play_tick();     break;
    case STATE_PAUSE:    screen_pause_tick();    break;
    case STATE_GAMEOVER: screen_gameover_tick(); break;
    default: break;
    }
}
