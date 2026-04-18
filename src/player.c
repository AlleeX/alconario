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

/* ---- Tuneable constants ------------------------------------------- */

/*
 * PLAYER_SPEED — pixels moved per frame when a direction is held.
 * At 60 fps, speed=1 → 60 px/s ≈ crosses the screen in ~4 seconds.
 * Increase to 2 for a snappier feel.
 */
#define PLAYER_SPEED   1

/*
 * PLAYER_TILE — index of the player's sprite tile in the CHR pattern table.
 * Tile $00 is the very first 8×8 tile in the sprite bank (CHR bank 1).
 * Change this once you have real artwork in your .chr file.
 */
#define PLAYER_TILE    0x00

/*
 * PLAYER_ATTR — sprite attribute byte:
 *   bits 0-1 : palette (0 = sprite palette 0, defined in gfx.c)
 *   bit  5   : priority (0 = in front of background)
 *   bit  6   : horizontal flip
 *   bit  7   : vertical flip
 * 0x00 = palette 0, no flip, in front of BG.
 */
#define PLAYER_ATTR    0x00

/* ---- Global player instance (one per game) ----------------------- */
player_t player;

/* ------------------------------------------------------------------
 * player_init()
 * Reset the player to the default starting state.
 * Called by enter_play() in game.c each time a new game starts.
 * ------------------------------------------------------------------ */
void player_init(void)
{
    player.x     = 120;   /* start near horizontal centre (screen = 256px) */
    player.y     = 112;   /* start near vertical centre   (screen = 240px) */
    player.vx    = 0;     /* no initial horizontal velocity */
    player.vy    = 0;     /* no initial vertical velocity   */
    player.frame = 0;     /* first animation frame          */
    player.alive = 1;     /* player is alive                */
}

/* ------------------------------------------------------------------
 * player_update()
 * Read input and move the player.  Called once per frame during
 * STATE_PLAY in game_tick().
 * ------------------------------------------------------------------ */
void player_update(void)
{
    /* Don't move if dead (e.g. death animation is playing). */
    if (!player.alive) return;

    /*
     * D-pad movement — pad1 is the HELD bitmask (see input.c).
     * We use += / -= directly on uint8_t; wrapping is safe here
     * because we clamp immediately below.
     */
    if (pad1 & BTN_LEFT)  player.x -= PLAYER_SPEED;
    if (pad1 & BTN_RIGHT) player.x += PLAYER_SPEED;
    if (pad1 & BTN_UP)    player.y -= PLAYER_SPEED;
    if (pad1 & BTN_DOWN)  player.y += PLAYER_SPEED;

    /*
     * Screen boundary clamp.
     * Left/Right: keep the sprite fully on-screen (8px margin = sprite width).
     * Top/Bottom: 8px top margin; 216 = 240 - 24 (sprite + a little padding).
     *
     * NOTE: x and y are uint8_t (0-255), so we check against 8 before
     * subtracting to avoid underflow wrap-around.
     */
    if (player.x < 8)   player.x = 8;
    if (player.x > 240) player.x = 240;
    if (player.y < 8)   player.y = 8;
    if (player.y > 216) player.y = 216;

    /* TODO: animate player.frame based on movement direction */
    /* TODO: check collisions with enemies / tiles            */
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
    oam_spr(player.x, player.y, PLAYER_TILE, PLAYER_ATTR, 0);

    /*
     * For a 16×16 player made of 4 tiles you would call oam_spr() four
     * times with offsets:
     *   oam_spr(player.x,   player.y,   TILE_TL, PLAYER_ATTR);
     *   oam_spr(player.x+8, player.y,   TILE_TR, PLAYER_ATTR);
     *   oam_spr(player.x,   player.y+8, TILE_BL, PLAYER_ATTR);
     *   oam_spr(player.x+8, player.y+8, TILE_BR, PLAYER_ATTR);
     */
}
