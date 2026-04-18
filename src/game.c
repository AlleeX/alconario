/*
 * game.c — top-level game state machine
 * ============================================================
 *
 * WHAT IS A STATE MACHINE?
 * ------------------------
 * Instead of a tangle of if/else flags, we model the game as a set of
 * named *states* (title screen, playing, paused, game-over).  At any moment
 * only ONE state is active.  Each state knows:
 *   - what to draw / update every frame
 *   - which inputs trigger a *transition* to another state
 *
 * Transitions happen through dedicated enter_*() functions that always:
 *   1. Turn the PPU off  (ppu_off)  — safe to write VRAM only when PPU is off
 *                                     or during vblank.
 *   2. Clear / rebuild the nametable (background tiles).
 *   3. Load the correct palettes.
 *   4. Initialise any entities needed in the new state.
 *   5. Turn the PPU back on (ppu_on_all) — screen becomes visible again.
 *   6. Update g_state so game_tick() dispatches to the right branch.
 *
 * STATE DIAGRAM
 * -------------
 *   BOOT ──► TITLE ──[START]──► PLAY ──[START]──► PAUSE
 *              ▲                  │                  │
 *              │                  │[die]         [START]
 *              └──────────────────┘                  │
 *                   GAMEOVER ◄───────────────────────┘
 *   (GAMEOVER ──[START]──► TITLE)
 */

#include "neslib.h"   /* ppu_off, ppu_on_all, pal_bright, bank_bg, bank_spr,
                         oam_clear — all PPU/hardware helpers from neslib */
#include "game.h"     /* game_state_t enum, g_state, prototypes */
#include "input.h"    /* pad1_new — buttons pressed THIS frame only */
#include "player.h"   /* player_init, player_update, player_draw */
#include "gfx.h"      /* gfx_load_palettes, gfx_clear_nametable */

/* The current active state — readable from any module via game.h */
game_state_t g_state;

/* ------------------------------------------------------------------
 * enter_title()
 * Show the title / attract screen.
 * Called once when transitioning INTO the title state.
 * ------------------------------------------------------------------ */
static void enter_title(void)
{
    /*
     * ppu_off() disables rendering.  We MUST do this before writing large
     * amounts of data to VRAM (nametables, palettes) outside of vblank —
     * otherwise the PPU may read garbled data mid-scanline.
     */
    ppu_off();

    gfx_clear_nametable();  /* wipe all background tiles to tile #0   */
    gfx_load_palettes();    /* upload colour palettes to the PPU       */

    /* TODO: draw the title logo and "PRESS START" text into nametable */

    /*
     * ppu_on_all() re-enables both background AND sprite rendering.
     * After this call the screen is live again.
     */
    ppu_on_all();
    g_state = STATE_TITLE;
}

/* ------------------------------------------------------------------
 * enter_play()
 * Transition into active gameplay.
 * ------------------------------------------------------------------ */
static void enter_play(void)
{
    ppu_off();
    gfx_clear_nametable();  /* clear old title-screen tiles            */
    gfx_load_palettes();    /* reload palettes (may differ per level)  */
    player_init();          /* place the player at the starting position */
    ppu_on_all();
    g_state = STATE_PLAY;
}

/* ------------------------------------------------------------------
 * game_init()
 * One-time setup called from main() before the game loop starts.
 * ------------------------------------------------------------------ */
void game_init(void)
{
    /*
     * pal_bright(4) sets the master brightness to normal (range 0-8,
     * 4 = no change).  Call with 0 to fade to black, 8 to fade to white.
     */
    pal_bright(4);

    /*
     * bank_bg(0)  — select CHR bank 0 for background tiles (pattern table 0).
     * bank_spr(1) — select CHR bank 1 for sprite tiles    (pattern table 1).
     * On NROM these are fixed; on MMC1/MMC3 you can swap banks dynamically.
     */
    bank_bg(0);
    bank_spr(1);

    /* Start on the title screen. */
    enter_title();
}

/* ------------------------------------------------------------------
 * game_tick()
 * Called ONCE PER FRAME from the main loop.
 * All gameplay logic lives here (or is called from here).
 * ------------------------------------------------------------------ */
void game_tick(void)
{
    /*
     * oam_clear() fills the 256-byte OAM shadow buffer with $FF.
     * Any sprite slot not written this frame will be pushed off-screen
     * (Y >= 240), which is the correct way to "hide" unused sprites on NES.
     * neslib DMAs this buffer to the PPU automatically during NMI.
     */
    oam_clear();

    /*
     * Dispatch to the logic for whichever state is currently active.
     * pad1_new contains buttons that were just PRESSED this frame
     * (edge-detect — true only on the first frame the button is held).
     */
    switch (g_state) {

    /* ---- Title screen ---- */
    case STATE_TITLE:
        /* Wait for the player to press START, then begin the game. */
        if (pad1_new & BTN_START) {
            enter_play();
        }
        break;

    /* ---- Active gameplay ---- */
    case STATE_PLAY:
        player_update();    /* move player based on D-pad input */
        player_draw();      /* write player sprite into OAM buffer */
        /* TODO: update enemies, scroll, collisions, score… */
        if (pad1_new & BTN_START) {
            g_state = STATE_PAUSE;  /* instant pause — no transition needed */
        }
        break;

    /* ---- Paused ---- */
    case STATE_PAUSE:
        /*
         * We still draw the player so the screen doesn't go blank,
         * but we don't call player_update() so nothing moves.
         */
        player_draw();
        /* TODO: draw "PAUSED" overlay text */
        if (pad1_new & BTN_START) {
            g_state = STATE_PLAY;   /* resume */
        }
        break;

    /* ---- Game over ---- */
    case STATE_GAMEOVER:
        /* TODO: show score, play jingle */
        if (pad1_new & BTN_START) {
            enter_title();          /* back to the title screen */
        }
        break;

    default:
        break;
    }
}
