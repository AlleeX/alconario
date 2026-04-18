/*
 * game.h — global game definitions & state machine
 * ============================================================
 *
 * Include this header in any module that needs to read the current
 * game state (g_state) or call game_init() / game_tick().
 */
#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* NTSC NES runs at 60 frames/s.  PAL runs at 50. */
#define FPS                60u

/* NES screen dimensions in pixels. */
#define SCREEN_W           256u
#define SCREEN_H           240u

/*
 * game_state_t — all possible high-level states the game can be in.
 *
 * STATE_BOOT     : very first frame after power-on (reserved for future use)
 * STATE_TITLE    : title / attract screen — waiting for START
 * STATE_PLAY     : active gameplay
 * STATE_PAUSE    : gameplay frozen — waiting for START to resume
 * STATE_GAMEOVER : player has died — show score, wait for START
 *
 * Only ONE state is active at a time.  Transitions are handled by the
 * enter_*() functions in game.c.
 */
typedef enum {
    STATE_BOOT = 0,
    STATE_TITLE,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_GAMEOVER
} game_state_t;

/*
 * g_state — the currently active game state.
 * Defined in game.c; declared extern here so other modules can read it.
 * Only game.c should WRITE to g_state.
 */
extern game_state_t g_state;

/*
 * game_init() — one-time setup: palettes, CHR banks, initial state.
 * Called once from main() before the game loop starts.
 */
void game_init(void);

/*
 * game_tick() — advance the game by one frame.
 * Called once per frame from main(), after input_poll().
 * Dispatches to per-state logic via a switch on g_state.
 */
void game_tick(void);

#endif /* GAME_H */
