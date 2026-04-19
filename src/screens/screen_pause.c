/*
 * screen_pause.c -- pause overlay
 * ============================================================
 *
 * Freezes gameplay.  The player sprite is still drawn so the
 * screen doesn't go blank, but nothing moves.
 */

#include "neslib.h"
#include "game.h"
#include "input.h"
#include "screen.h"
#include "player.h"
#include "sfx.h"

/*
 * VRAM update buffer for drawing "PAUSED" on screen while PPU is on.
 * Format: MSB (hi|$00), LSB (lo), LEN, data..., NT_UPD_EOF
 * "PAUSED" at tile row 14, col 13 → center of 32-column screen.
 * NAMETABLE_A ($2000) + 14*32 + 13 = $2000 + $01CD = $21CD
 */
static unsigned char pause_vram_buf[] = {
    0x21|NT_UPD_HORZ, 0xCD, 6,    /* addr hi|HORZ, addr lo, length=6 */
    'P','A','U','S','E','D',
    NT_UPD_EOF
};

/* Empty update buffer to clear the overlay on resume */
static unsigned char empty_vram_buf[] = {
    NT_UPD_EOF
};

/* ------------------------------------------------------------------
 * screen_pause_enter()
 * Instant pause -- no PPU transition needed.
 * ------------------------------------------------------------------ */
void screen_pause_enter(void)
{
    set_vram_update(pause_vram_buf);
    sfx_play(SFX_PAUSE, SFX_CH0);
    g_state = STATE_PAUSE;
}

/* ------------------------------------------------------------------
 * screen_pause_tick()
 * ------------------------------------------------------------------ */
void screen_pause_tick(void)
{
    player_draw();   /* keep player visible */

    if (pad1_new & BTN_START) {
        /* Clear "PAUSED" by overwriting with spaces */
        pause_vram_buf[3] = ' ';
        pause_vram_buf[4] = ' ';
        pause_vram_buf[5] = ' ';
        pause_vram_buf[6] = ' ';
        pause_vram_buf[7] = ' ';
        pause_vram_buf[8] = ' ';
        /* The buffer will be flushed this frame, then disable updates */
        ppu_wait_nmi();
        /* Restore letters for next pause */
        pause_vram_buf[3] = 'P';
        pause_vram_buf[4] = 'A';
        pause_vram_buf[5] = 'U';
        pause_vram_buf[6] = 'S';
        pause_vram_buf[7] = 'E';
        pause_vram_buf[8] = 'D';
        set_vram_update(empty_vram_buf);
        sfx_play(SFX_PAUSE, SFX_CH0);
        g_state = STATE_PLAY;
    }
}
