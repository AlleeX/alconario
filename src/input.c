/*
 * input.c — NES controller polling
 * ============================================================
 *
 * THE NES CONTROLLER PROTOCOL (brief)
 * ------------------------------------
 * The NES reads 8 buttons serially via a shift register inside the pad.
 * The CPU writes $01 then $00 to the controller port ($4016) to latch
 * the current button states, then reads the port 8 times to get each
 * bit: A, B, Select, Start, Up, Down, Left, Right.
 *
 * neslib wraps all of this into two handy functions:
 *
 *   pad_poll(n)    — reads controller n (0 = player 1, 1 = player 2).
 *                    Returns an 8-bit mask of HELD buttons this frame.
 *
 *   pad_trigger(n) — returns only the buttons that changed from
 *                    "not held" to "held" this frame (rising edge).
 *                    Great for actions that should fire exactly once
 *                    per press (jump, pause, menu select…).
 *
 * WHY TWO VARIABLES?
 * ------------------
 *   pad1     — "is the button held RIGHT NOW?"
 *              Use for continuous actions: walking, running.
 *
 *   pad1_new — "was the button just pressed THIS frame?"
 *              Use for single-fire actions: jump, start, shoot.
 *
 * EXAMPLE
 * -------
 *   if (pad1     & BTN_RIGHT) player.x++;   // walks while held
 *   if (pad1_new & BTN_A)     player_jump(); // jumps only on press
 */

#include "neslib.h"   /* pad_poll(), pad_trigger() */
#include "input.h"    /* extern declarations + BTN_* constants */

/* Buttons held this frame (bitmask). */
uint8_t pad1;

/* Buttons newly pressed this frame — NOT held last frame (bitmask). */
uint8_t pad1_new;

/*
 * input_poll()
 * Called once per frame from main(), right after ppu_wait_nmi().
 * Refreshes both pad1 and pad1_new.
 */
void input_poll(void)
{
    /*
     * pad_poll(0) latches and reads controller #1 (zero-indexed).
     * Returns 0 if no buttons are pressed.
     */
    pad1     = pad_poll(0);

    /*
     * pad_trigger(0) must be called AFTER pad_poll() for the same
     * controller — it uses the internal previous-frame state that
     * pad_poll() just updated.
     */
    pad1_new = pad_trigger(0);
}
