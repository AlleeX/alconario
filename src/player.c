/*
 * player.c — Alconario player entity
 * ============================================================
 *
 * NES SPRITES — QUICK PRIMER
 * --------------------------
 * The NES PPU supports up to 64 hardware sprites (called "OAM objects").
 * Each sprite is 8×8 (or 8×16) pixels and is described by 4 bytes:
 *
 *   Byte 0 : Y position (top of sprite, 0-255)
 *   Byte 1 : Tile index into the sprite pattern table (CHR bank)
 *   Byte 2 : Attributes — palette (bits 0-1), priority (bit 5),
 *             horizontal flip (bit 6), vertical flip (bit 7)
 *   Byte 3 : X position (left of sprite, 0-255)
 *
 * neslib's oam_spr(x, y, tile, attr) writes one 8×8 sprite into the
 * next free slot of the 256-byte OAM shadow buffer.  The buffer is
 * DMA'd to the PPU once per frame during NMI.
 *
 * COORDINATE SYSTEM
 * -----------------
 *   (0,0) is the top-left of the screen.
 *   X increases to the right  (0-255).
 *   Y increases downward      (0-239).
 *   Sprites are clipped at Y=240 (off-screen bottom).
 *
 * THE player_t STRUCT (see include/player.h)
 * ------------------------------------------
 *   x, y    — on-screen pixel position of the sprite's top-left corner
 *   vx, vy  — velocity (pixels per frame) — not used yet, reserved for
 *             physics / inertia later
 *   frame   — animation frame counter — index into tile sheet
 *   alive   — 0 means dead / game-over, 1 means active
 */

#include "neslib.h"   /* oam_spr() */
#include "game.h"     /* STATE_GAMEOVER, g_state */
#include "input.h"    /* pad1, BTN_* */
#include "player.h"   /* player_t, extern player */
#include "sfx.h"      /* SFX_JUMP, SFX_CH1 */

/* ---- Tuneable constants ------------------------------------------- */

#define PLAYER_SPEED    2       /* horizontal walk speed (px/frame)    */
#define PLAYER_RUN_SPEED 3     /* horizontal run speed (hold B)       */
#define PLAYER_TILE     0x01   /* first non-empty tile in CHR bank 0 */
#define PLAYER_ATTR     0x00

#define GRAVITY         1       /* downward accel per frame            */
#define MAX_FALL        4       /* terminal fall velocity              */
#define JUMP_FORCE      (-10)    /* initial upward velocity on jump     */
#define GROUND_Y        200     /* Y pixel where the ground is         */

/* ---- Global player instance (one per game) ----------------------- */
player_t player;

/* ------------------------------------------------------------------
 * player_init()
 * Reset the player to the default starting state.
 * Called by enter_play() in game.c each time a new game starts.
 * ------------------------------------------------------------------ */
void player_init(void)
{
    player.x      = 40;    /* start near left side                    */
    player.y      = GROUND_Y - 8; /* stand on ground (8px sprite)     */
    player.vx     = 0;
    player.vy     = 0;
    player.frame  = 0;
    player.alive  = 1;
    player.on_ground = 1;
    player.facing = 0;     /* facing right */
}

/* ------------------------------------------------------------------
 * player_update()
 * Read input and move the player.  Called once per frame during
 * STATE_PLAY in game_tick().
 * ------------------------------------------------------------------ */
void player_update(void)
{
    unsigned char speed;

    if (!player.alive) return;

    /* -- Horizontal movement ---------------------------------------- */
    speed = (pad1 & BTN_B) ? PLAYER_RUN_SPEED : PLAYER_SPEED;

    if (pad1 & BTN_LEFT) {
        player.x -= speed;
        player.facing = 1;
    }
    if (pad1 & BTN_RIGHT) {
        player.x += speed;
        player.facing = 0;
    }

    /* -- Jumping (press A while on ground) -------------------------- */
    if ((pad1_new & BTN_A) && player.on_ground) {
        player.vy = JUMP_FORCE;
        player.on_ground = 0;
        sfx_play(SFX_JUMP, SFX_CH1);
    }

    /* -- Gravity ---------------------------------------------------- */
    if (!player.on_ground) {
        player.vy += GRAVITY;
        if (player.vy > MAX_FALL) player.vy = MAX_FALL;
    }

    /* -- Apply vertical velocity ------------------------------------ */
    player.y += player.vy;

    /* -- Ground collision ------------------------------------------- */
    if (player.y >= GROUND_Y - 8) {
        player.y = GROUND_Y - 8;
        player.vy = 0;
        player.on_ground = 1;
    }

    /* -- Screen boundary clamp (horizontal) ------------------------- */
    if (player.x < 4)   player.x = 4;
    if (player.x > 244) player.x = 244;
    if (player.y < 4)   player.y = 4;
}

/* ------------------------------------------------------------------
 * player_draw()
 * Push the player sprite into the OAM shadow buffer.
 * Called every frame (even during pause so the sprite stays visible).
 *
 * oam_spr(x, y, tile, attr) — neslib function.
 * Each call uses the next available OAM slot (auto-incremented).
 * oam_clear() in game_tick() resets the slot counter each frame.
 * ------------------------------------------------------------------ */
void player_draw(void)
{
    unsigned char attr = player.facing ? 0x40 : 0x00; /* bit 6 = H-flip */
    oam_spr(player.x, player.y, PLAYER_TILE, attr, 0);

    /*
     * For a 16×16 player made of 4 tiles you would call oam_spr() four
     * times with offsets:
     *   oam_spr(player.x,   player.y,   TILE_TL, PLAYER_ATTR);
     *   oam_spr(player.x+8, player.y,   TILE_TR, PLAYER_ATTR);
     *   oam_spr(player.x,   player.y+8, TILE_BL, PLAYER_ATTR);
     *   oam_spr(player.x+8, player.y+8, TILE_BR, PLAYER_ATTR);
     */
}
