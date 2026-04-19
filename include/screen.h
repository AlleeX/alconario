#ifndef SCREEN_H
#define SCREEN_H
extern void screen_title_enter(void);
extern void screen_title_tick(void);
extern void screen_stage_enter(unsigned char stage_num);
extern void screen_stage_tick(void);
extern void screen_play_enter(void);
extern void screen_play_resume(void);  /* resume after stage card (keep score/lives) */
extern void screen_play_tick(void);
extern void screen_pause_enter(void);
extern void screen_pause_tick(void);
extern void screen_gameover_enter(void);
extern void screen_gameover_tick(void);
#endif
