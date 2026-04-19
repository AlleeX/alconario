/*
 * pickup.c — positive pickups: V symbols moving right to left
 */

#include "neslib.h"
#include "pickup.h"
#include "player.h"
#include "enemy.h"   /* enemy_base_speed */

/* Tile 'V' = 0x56 in ASCII CHR layout */
#define PICKUP_TILE  0x56
#define PICKUP_ATTR  0x02   /* sprite palette 2 (green) */

/* Ground Y must match player.c */
#define GROUND_Y     200

/* Spawn interval — slightly longer than enemies */
#define PICKUP_SPAWN_INTERVAL  200

pickup_t pickups[MAX_PICKUPS];

/* ------------------------------------------------------------------ */
void pickups_init(void)
{
    unsigned char i;
    for (i = 0; i < MAX_PICKUPS; ++i) {
        pickups[i].active = 0;
    }
}

/* ------------------------------------------------------------------ */
void pickups_spawn(void)
{
    unsigned char i;
    for (i = 0; i < MAX_PICKUPS; ++i) {
        if (!pickups[i].active) {
            pickups[i].x      = 248;
            pickups[i].y      = GROUND_Y - 8;
            pickups[i].active  = 1;
            pickups[i].speed   = enemy_base_speed;
            return;
        }
    }
}

/* ------------------------------------------------------------------ */
unsigned char pickups_update(void)
{
    unsigned char i;
    unsigned char missed = 0;

    for (i = 0; i < MAX_PICKUPS; ++i) {
        if (!pickups[i].active) continue;

        if (pickups[i].x < pickups[i].speed) {
            pickups[i].active = 0;
            ++missed;
        } else {
            pickups[i].x -= pickups[i].speed;
        }
    }
    return missed;
}

/* ------------------------------------------------------------------ */
void pickups_draw(void)
{
    unsigned char i;
    for (i = 0; i < MAX_PICKUPS; ++i) {
        if (pickups[i].active) {
            /* OAM slots 20,24 — after enemies (4,8,12,16) and player (0) */
            oam_spr(pickups[i].x, pickups[i].y, PICKUP_TILE, PICKUP_ATTR,
                    20 + i * 4);
        }
    }
}

/* ------------------------------------------------------------------ */
unsigned char pickups_check_collision(void)
{
    unsigned char i;
    unsigned char px, py;
    unsigned char cx, cy;
    unsigned char dx, dy;
    unsigned char collected = 0;

    px = player.x;
    py = player.y;

    for (i = 0; i < MAX_PICKUPS; ++i) {
        if (!pickups[i].active) continue;

        cx = pickups[i].x;
        cy = pickups[i].y;

        dx = (px > cx) ? (px - cx) : (cx - px);
        dy = (py > cy) ? (py - cy) : (cy - py);

        if (dx < 6 && dy < 6) {
            pickups[i].active = 0;
            ++collected;
        }
    }
    return collected;
}
