/*
 * pickup.h — positive pickups (V symbol, move right to left)
 */
#ifndef PICKUP_H
#define PICKUP_H

#include <stdint.h>

#define MAX_PICKUPS  2

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t active;
    uint8_t speed;
} pickup_t;

extern pickup_t pickups[MAX_PICKUPS];

/* Reset all pickups. */
void pickups_init(void);

/* Try to spawn one pickup at the right edge. */
void pickups_spawn(void);

/* Move pickups. Returns number that escaped off-screen. */
unsigned char pickups_update(void);

/* Draw all active pickups. */
void pickups_draw(void);

/* Check player collision. Returns number collected. */
unsigned char pickups_check_collision(void);

#endif /* PICKUP_H */
