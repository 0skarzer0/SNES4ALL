#ifndef SDLINTERFACE_H
#define SDLINTERFACE_H
#include<SDL.h>

extern SDL_Surface *screen, *gfxscreen;


/*
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
*/
void sound_play_menu_music(void);
void sound_play_beep(void);
void sound_enable_music(void);
void sound_disable_music(void);
void sound_play_beep(void);
/*
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
*/

#endif
