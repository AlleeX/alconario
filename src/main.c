/*
 * main.c — entry point for Alconario
 * ============================================================
 *
 * HOW THE NES STARTS UP
 * ---------------------
 * When the NES powers on, the CPU jumps to the RESET vector (stored at
 * address $FFFC-$FFFD in PRG-ROM).  neslib's crt0.s handles the low-level
 * setup (stack pointer, zero-page clear, PPU warm-up) and then calls
 * main() here.
 *
 * THE GAME LOOP
 * -------------
 * The NES PPU (Picture Processing Unit) draws 60 frames per second (NTSC).
 * At the end of each frame it fires an NMI (Non-Maskable Interrupt).
 * neslib's ppu_wait_nmi() blocks until that NMI arrives, so everything
 * inside the loop runs exactly once per frame — a rock-solid 60 Hz heartbeat.
 *
 * Frame budget (very rough, 6502 @ 1.79 MHz):
 *   ~29,780 CPU cycles per frame.
 *   A simple C function call costs ~10-20 cycles — you have thousands to
 *   spend, but keep tight loops in mind on the 6502!
 *
 * CALL ORDER MATTERS:
 *   1. ppu_wait_nmi()  — wait for vblank; neslib DMAs the OAM sprite buffer
 *                        to the PPU automatically during NMI.
 *   2. input_poll()    — sample the controller *after* vblank so we always
 *                        work with a fresh snapshot of button states.
 *   3. game_tick()     — run all game logic (state machine, physics, AI…).
 *                        Any PPU writes (nametable updates, palette changes)
 *                        should happen early in the tick while we're still
 *                        in the vblank window.
 */

#include "neslib.h"   /* ppu_wait_nmi and all other NES helpers */
#include "game.h"     /* game_init() / game_tick() declarations */
#include "input.h"    /* input_poll() declaration */

/*
 * main() — called by crt0.s after hardware initialisation.
 *
 * Note: cc65 uses void main(void) for the NES target; the standard
 * int main() signature is not used here.
 */
void main(void)
{
    /* One-time setup: palettes, PPU banks, initial game state. */
    game_init();

    /*
     * Infinite loop — the NES never "exits" a program.
     * The only way to stop is to power off the console.
     */
    for (;;) {
        /*
         * Block until the PPU fires its NMI (start of vblank).
         * neslib uses this interrupt to DMA the 256-byte OAM shadow buffer
         * (sprite positions/tiles) directly into PPU memory — essentially
         * "uploading" all sprites to the screen in one shot.
         */
        ppu_wait_nmi();

        /* Read controller #1 (and store edge-detect for button presses). */
        input_poll();

        /* Advance the game by one frame. */
        game_tick();
    }
}
