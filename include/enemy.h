/*
 * enemy.h — simple dino-style enemies (move right to left)
 */
#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>

#define MAX_ENEMIES  4

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t active;   /* 1 = on screen, 0 = inactive */
    uint8_t speed;    /* pixels per frame */
} enemy_t;

extern enemy_t enemies[MAX_ENEMIES];

/* Base speed for enemies — increased each stage */
extern unsigned char enemy_base_speed;

/* Reset all enemies to inactive. */
void enemies_init(void);

/* Try to spawn one enemy at the right edge. */
void enemies_spawn(void);

/* Spawn, move, and recycle enemies. Returns number that left the screen. */
unsigned char enemies_update(void);

/* Draw all active enemies into OAM. Call once per frame. */
void enemies_draw(void);

/*
 * Check collision between player and all enemies.
 * Returns 1 if any enemy overlaps the player, 0 otherwise.
 */
unsigned char enemies_check_collision(void);

#endif /* ENEMY_H */
