#if !defined(_VIDEOGL_H_) && defined(USE_GL)
#define _VIDEOGL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_opengl.h>

#if defined(CYGWIN) && defined(__WIN32__)
#undef __WIN32__
#endif

#define VIDEO_GL_WIDTH 640
#define VIDEO_GL_HEIGHT 480

#ifndef DREAMCAST
#define PSX4ALL_FILTER_NONE     GL_NEAREST
#define PSX4ALL_FILTER_BILINEAR GL_LINEAR
#else
#define PSX4ALL_FILTER_NONE     GL_FILTER_NONE
#define PSX4ALL_FILTER_BILINEAR GL_FILTER_BILINEAR
#endif

#ifdef DREAMCAST
#define TILE_Z_INIT 1.0
#define TILE_Z_INC 0.69
#else
#define TILE_Z_INIT 0.1
#define TILE_Z_INC 0.000069
#endif

extern SDL_Surface *screen_gl;
extern void *texture_buffer;
extern GLint screen_texture;
extern double videogl_z;
extern GLint videogl_tex[];

void videogl_flip(SDL_bool);
void videogl_fill(unsigned,unsigned,unsigned);
void videogl_fill_box(unsigned,unsigned,unsigned,unsigned,unsigned);


static __inline__ void loadTextureParams(int filter)
{
//	glPixelStorei(GL_UNPACK_ROW_LENGTH, 16);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


#endif
