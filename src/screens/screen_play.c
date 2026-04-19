/*
 * screen_play.c -- active gameplay screen
 * ============================================================
 */

#include "neslib.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "screen.h"
#include "player.h"
#include "enemy.h"
#include "pickup.h"
#include "sfx.h"

/* VRAM update buffer — two horizontal runs:
 *   "LIVES:X"   at row 2, col 24  (NAMETABLE_A + 2*32 + 24 = $2058)
 *   "SCORE:000" at row 4, col 22  (NAMETABLE_A + 4*32 + 22 = $20B6)
 *   Both right-aligned to column 31.
 */
static unsigned char hud_buf[] = {
    /* Run 1: LIVES:XX  (8 chars, col 23-30) */
    0x20|NT_UPD_HORZ, 0x57, 8,
    'L','I','V','E','S',':','0','3',
    /* Run 2: SCORE:000 (9 chars, col 22-30) */
    0x20|NT_UPD_HORZ, 0xB6, 9,
    'S','C','O','R','E',':','0','0','0',
    NT_UPD_EOF
};

/* Empty update buffer — disables VRAM writes without passing NULL */
static unsigned char empty_buf[] = { NT_UPD_EOF };

/* Shared spawn queue — one object at a time, random enemy or pickup */
#define SPAWN_INTERVAL  90   /* frames between spawns (~1.5s) */
static unsigned char spawn_timer;
static unsigned char rng_state;  /* simple PRNG */

static unsigned char next_random(void)
{
    /* LFSR-style 8-bit PRNG */
    rng_state ^= (rng_state << 3);
    rng_state ^= (rng_state >> 5);
    rng_state ^= (rng_state << 4);
    return rng_state;
}

static void spawn_next(void)
{
    /* ~30% chance pickup, ~70% chance enemy */
    if ((next_random() % 10) < 3) {
        pickups_spawn();
    } else {
        enemies_spawn();
    }
}

static void hud_update(void)
{
    unsigned int s;
    /* Update lives digits (indices 9, 10) — 2-digit decimal */
    hud_buf[9]  = '0' + (g_lives / 10);
    hud_buf[10] = '0' + (g_lives % 10);
    /* Update score digits (indices 20, 21, 22) — 3-digit decimal */
    s = g_score;
    if (s > 999) s = 999;
    hud_buf[22] = '0' + (unsigned char)(s % 10); s /= 10;
    hud_buf[21] = '0' + (unsigned char)(s % 10); s /= 10;
    hud_buf[20] = '0' + (unsigned char)(s);
    set_vram_update(hud_buf);
}

/* ------------------------------------------------------------------
 * screen_play_enter()
 * Set up the level and start gameplay.
 * ------------------------------------------------------------------ */
static void draw_play_screen(void)
{
    unsigned char i;
    char world_str[12];

    ppu_off();

    gfx_clear_nametable();
    gfx_load_palettes();

    /* HUD — "STAGE X" or "STAGE XX" */
    world_str[0] = 'S'; world_str[1] = 'T'; world_str[2] = 'A';
    world_str[3] = 'G'; world_str[4] = 'E'; world_str[5] = ' ';
    if (g_stage >= 10) {
        world_str[6] = '0' + (g_stage / 10);
        world_str[7] = '0' + (g_stage % 10);
        world_str[8] = '\0';
    } else {
        world_str[6] = '0' + g_stage;
        world_str[7] = '\0';
    }
    gfx_draw_text(2, 2, world_str);

    /* Surface row (tile row 25) */
    vram_adr(NAMETABLE_A + 25 * 32);
    for (i = 0; i < 32; ++i) vram_put(0x42);

    /* Underground rows (tile rows 26-29) */
    vram_adr(NAMETABLE_A + 26 * 32);
    vram_fill(0x58, 32 * 4);

    player_init();
    enemies_init();
    pickups_init();
    spawn_timer = 60;     /* first spawn after 1 second */
    if (rng_state == 0) rng_state = 0xAD;  /* seed PRNG */

    ppu_on_all();

    hud_update();
    g_state = STATE_PLAY;
}

/* ------------------------------------------------------------------
 * screen_play_enter()
 * Fresh game start — reset everything.
 * ------------------------------------------------------------------ */
void screen_play_enter(void)
{
    /* Cheat: hold A (jump) when starting → 20 lives */
    if (pad1 & BTN_A) {
        g_lives = 20;
    } else {
        g_lives = 3;
    }
    g_score = 0;
    g_stage = 1;
    g_next_stage_score = 10;
    enemy_base_speed = 1;
    draw_play_screen();
}

/* ------------------------------------------------------------------
 * screen_play_resume()
 * Resume after stage card — keep score, lives, speed.
 * ------------------------------------------------------------------ */
void screen_play_resume(void)
{
    draw_play_screen();
}

/* ------------------------------------------------------------------
 * screen_play_tick()
 * Per-frame gameplay logic.
 * ------------------------------------------------------------------ */
void screen_play_tick(void)
{
    unsigned char escaped;
    unsigned char missed;
    unsigned char collected;

    /* Re-enable HUD VRAM update (may have been cleared by pause) */
    set_vram_update(hud_buf);

    player_update();
    escaped = enemies_update();
    missed  = pickups_update();

    /* Shared spawn queue */
    if (spawn_timer == 0) {
        spawn_next();
        spawn_timer = SPAWN_INTERVAL;
    } else {
        --spawn_timer;
    }

    /* Score +1 for each enemy that left the screen */
    if (escaped) {
        g_score += escaped;
        hud_update();

        /* Check stage advance */
        if (g_score >= g_next_stage_score) {
            set_vram_update(empty_buf);
            ++g_stage;
            g_next_stage_score += 10;
            ++enemy_base_speed;
            screen_stage_enter(g_stage);
            return;
        }
    }

    /* Missed pickup — lose a life */
    if (missed) {
        sfx_play(SFX_MISS, SFX_CH2);
        if (g_lives > 0) --g_lives;
        hud_update();

        if (g_lives == 0) {
            set_vram_update(empty_buf);
            screen_gameover_enter();
            return;
        }
    }

    /* Collected pickup — score +1 */
    collected = pickups_check_collision();
    if (collected) {
        sfx_play(SFX_COLLECT, SFX_CH1);
        g_score += collected;
        hud_update();

        if (g_score >= g_next_stage_score) {
            set_vram_update(empty_buf);
            ++g_stage;
            g_next_stage_score += 10;
            ++enemy_base_speed;
            screen_stage_enter(g_stage);
            return;
        }
    }

    /* Check enemy collision */
    if (enemies_check_collision()) {
        sfx_play(SFX_HIT, SFX_CH2);
        if (g_lives > 0) --g_lives;
        hud_update();

        if (g_lives == 0) {
            set_vram_update(empty_buf);
            screen_gameover_enter();
            return;
        }
    }

    player_draw();
    enemies_draw();
    pickups_draw();

    if (pad1_new & BTN_START) {
        set_vram_update(empty_buf);
        screen_pause_enter();
    }
}
