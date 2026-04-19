/*
 * screen_pause.c -- pause overlay
 * ============================================================
 *
 * Freezes gameplay.  The player sprite is still drawn so the
 * screen doesn't go blank, but nothing moves.
 */

#include "neslib.h"
#include "game.h"
#include "input.h"
#include "screen.h"
#include "player.h"

/* ------------------------------------------------------------------
 * screen_pause_enter()
 * Instant pause -- no PPU transition needed.
 * ------------------------------------------------------------------ */
void screen_pause_enter(void)
{
    /* TODO: draw "PAUSED" overlay text via vram update buffer */
    g_state = STATE_PAUSE;
}

/* ------------------------------------------------------------------
 * screen_pause_tick()
 * ------------------------------------------------------------------ */
void screen_pause_tick(void)
{
    player_draw();   /* keep player visible */

    if (pad1_new & BTN_START) {
        g_state = STATE_PLAY;   /* resume */
    }
}
