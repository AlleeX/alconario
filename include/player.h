/*
 * player.h — player entity definition
 * ============================================================
 *
 * Declares the player_t struct and the global player instance.
 * Include this in any module that needs to read or write player state
 * (e.g. a collision module, HUD renderer, enemy AI…).
 */
#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

/*
 * player_t — all state for the player character.
 *
 * Fields:
 *   x, y    — screen position (pixels, top-left of sprite).
 *              uint8_t gives a 0-255 range, sufficient for one screen.
 *              For scrolling games you would switch to uint16_t.
 *
 *   vx, vy  — signed velocity in pixels per frame.
 *              int8_t gives -128 to +127 — plenty for basic physics.
 *              Not actively used yet; reserved for momentum / gravity.
 *
 *   frame   — current animation frame index.
 *              Indexes into the tile sheet in the CHR ROM.
 *              Increment / modulate in player_update() to animate.
 *
 *   alive   — 1 = player is active, 0 = player is dead.
 *              When 0, player_update() skips all movement / input.
 *              Set to 0 when hit by an enemy; trigger STATE_GAMEOVER.
 */
typedef struct {
    uint8_t x;
    uint8_t y;
    int8_t  vx;
    int8_t  vy;
    uint8_t frame;
    uint8_t alive;
} player_t;

/* The one and only player instance — defined in player.c. */
extern player_t player;

/* player_init()   — reset to starting position & clear all fields. */
void player_init(void);

/* player_update() — read input, apply movement, check boundaries.  */
void player_update(void);

/* player_draw()   — write player sprite(s) into the OAM buffer.    */
void player_draw(void);

#endif /* PLAYER_H */
