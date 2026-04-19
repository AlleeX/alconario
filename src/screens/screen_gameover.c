/*
 * screen_gameover.c -- game-over screen
 * ============================================================
 */

#include "neslib.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "screen.h"
#include "sfx.h"

static unsigned int gameover_timer;
static char score_str[12];  /* "SCORE: 000" */

static void build_score_str(void)
{
    unsigned int s = g_score;
    if (s > 999) s = 999;
    score_str[0] = 'S';
    score_str[1] = 'C';
    score_str[2] = 'O';
    score_str[3] = 'R';
    score_str[4] = 'E';
    score_str[5] = ':';
    score_str[6] = ' ';
    score_str[7] = '0' + (unsigned char)(s / 100);
    score_str[8] = '0' + (unsigned char)((s / 10) % 10);
    score_str[9] = '0' + (unsigned char)(s % 10);
    score_str[10] = '\0';
}

/* ------------------------------------------------------------------
 * screen_gameover_enter()
 * ------------------------------------------------------------------ */
void screen_gameover_enter(void)
{
    ppu_off();

    gfx_clear_nametable();
    gfx_load_palettes();

    gfx_draw_text(12, 13, "GAME OVER");

    build_score_str();
    gfx_draw_text(11, 16, score_str);

    ppu_on_all();

    sfx_play(SFX_GAMEOVER, SFX_CH0);
    gameover_timer = 120;   /* 2 seconds at 60fps */
    g_state = STATE_GAMEOVER;
}

/* ------------------------------------------------------------------
 * screen_gameover_tick()
 * Wait 2 seconds then return to title.
 * ------------------------------------------------------------------ */
void screen_gameover_tick(void)
{
    if (gameover_timer > 0) {
        --gameover_timer;
    } else {
        screen_title_enter();
    }
}
