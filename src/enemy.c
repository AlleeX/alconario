/*
 * enemy.c — dino-style enemies: walk from right edge to left edge
 */

#include "neslib.h"
#include "enemy.h"
#include "player.h"

/* Tile 'O' = 0x4F in the ASCII CHR layout */
#define ENEMY_TILE  0x4F
#define ENEMY_ATTR  0x01   /* sprite palette 1 */

/* Ground Y must match player.c */
#define GROUND_Y    200

/* Spawn cooldown in frames (enemies appear every ~2 seconds) */
#define SPAWN_INTERVAL  120

enemy_t enemies[MAX_ENEMIES];

unsigned char enemy_base_speed;

/* ------------------------------------------------------------------ */
void enemies_init(void)
{
    unsigned char i;
    for (i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = 0;
    }
    if (enemy_base_speed == 0) enemy_base_speed = 1;
}

/* ------------------------------------------------------------------ */
void enemies_spawn(void)
{
    unsigned char i;
    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            enemies[i].x      = 248;
            enemies[i].y      = GROUND_Y - 8;
            enemies[i].active  = 1;
            enemies[i].speed   = enemy_base_speed + (i & 1);
            return;
        }
    }
}

/* ------------------------------------------------------------------ */
unsigned char enemies_update(void)
{
    unsigned char i;
    unsigned char escaped = 0;

    /* Move all active enemies left */
    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (enemies[i].x < enemies[i].speed) {
            /* Went off the left edge — deactivate */
            enemies[i].active = 0;
            ++escaped;
        } else {
            enemies[i].x -= enemies[i].speed;
        }
    }
    return escaped;
}

/* ------------------------------------------------------------------ */
void enemies_draw(void)
{
    unsigned char i;
    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].active) {
            oam_spr(enemies[i].x, enemies[i].y, ENEMY_TILE, ENEMY_ATTR,
                    (i + 1) * 4);  /* OAM slots 4,8,12,16 — slot 0 is player */
        }
    }
}

/* ------------------------------------------------------------------ */
unsigned char enemies_check_collision(void)
{
    unsigned char i;
    unsigned char px, py;
    unsigned char ex, ey;
    unsigned char dx, dy;

    px = player.x;
    py = player.y;

    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        ex = enemies[i].x;
        ey = enemies[i].y;

        /* Unsigned distance — handles wrap correctly */
        dx = (px > ex) ? (px - ex) : (ex - px);
        dy = (py > ey) ? (py - ey) : (ey - py);

        if (dx < 6 && dy < 6) {
            enemies[i].active = 0;  /* consume the enemy */
            return 1;
        }
    }
    return 0;
}
