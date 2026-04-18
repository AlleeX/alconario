/*
 * input.h — NES controller abstraction
 * ============================================================
 *
 * Provides two variables updated every frame by input_poll():
 *
 *   pad1     — bitmask of buttons HELD this frame
 *   pad1_new — bitmask of buttons just PRESSED this frame (edge detect)
 *
 * Usage:
 *   if (pad1     & BTN_RIGHT)  player.x++;     // continuous movement
 *   if (pad1_new & BTN_START)  toggle_pause();  // single-fire on press
 *
 * BTN_* constants match the bit positions neslib uses internally
 * (same as the hardware shift-register read order).
 */
#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

/* Individual button bit flags. */
#define BTN_A       0x01   /* A button       */
#define BTN_B       0x02   /* B button       */
#define BTN_SELECT  0x04   /* Select button  */
#define BTN_START   0x08   /* Start button   */
#define BTN_UP      0x10   /* D-pad Up       */
#define BTN_DOWN    0x20   /* D-pad Down     */
#define BTN_LEFT    0x40   /* D-pad Left     */
#define BTN_RIGHT   0x80   /* D-pad Right    */

/* Buttons held this frame (set by input_poll). */
extern uint8_t pad1;

/* Buttons newly pressed this frame — not held last frame (edge detect). */
extern uint8_t pad1_new;

/*
 * input_poll() — read controller #1.
 * Call once per frame, right after ppu_wait_nmi().
 */
void input_poll(void);

#endif /* INPUT_H */
