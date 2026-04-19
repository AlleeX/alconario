/*
 * sfx.h -- sound effect IDs
 * ============================================================
 *
 * Indices must match the order in src/asm/sounds.sinc pointer table.
 */
#ifndef SFX_H
#define SFX_H

#define SFX_MENU_SELECT  0
#define SFX_MENU_CURSOR  1
#define SFX_JUMP         2
#define SFX_PAUSE        3
#define SFX_HIT          4
#define SFX_GAMEOVER     5
#define SFX_WIN          6
#define SFX_COLLECT      7
#define SFX_MISS         8

/* FamiTone2 SFX channel slots (0-3). */
#define SFX_CH0  0
#define SFX_CH1  1
#define SFX_CH2  2
#define SFX_CH3  3

#endif /* SFX_H */
