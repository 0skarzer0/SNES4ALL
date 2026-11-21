#if !defined(_VIDEOGL_H_) && defined(USE_GL)
#define _VIDEOGL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#ifndef WIN32
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#define GL_UNSIGNED_SHORT_1_5_5_5_REV           0x8366
#endif
#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5                 0x8363
#endif
#endif

#include "snes9x.h"
#include "ppu.h"

#if defined(CYGWIN) && defined(__WIN32__)
#undef __WIN32__
#endif

#define VIDEO_GL_WIDTH 640
#define VIDEO_GL_HEIGHT 480
#define VIDEO_GL_MAX_TILES 0x8000

#ifndef DREAMCAST
#define SNES4ALL_FILTER_NONE     GL_NEAREST
#define SNES4ALL_FILTER_BILINEAR GL_LINEAR
#else
#define SNES4ALL_FILTER_NONE     GL_FILTER_NONE
#define SNES4ALL_FILTER_BILINEAR GL_FILTER_BILINEAR
#endif

#ifndef VIDEO_GL_MAX_TEX
//#define VIDEO_GL_MAX_TEX (8*(MAX_2BIT_TILES + MAX_4BIT_TILES + MAX_8BIT_TILES))
#define VIDEO_GL_MAX_TEX (8*(2048 + 2048))
#endif

#define VIDEO_GL_TILE_Z_INIT 0.1
#define VIDEO_GL_TILE_Z_INC 0.000000069f

#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
#define VIDEO_GL_SELECTIVE_MAX_CACHE (VIDEO_GL_MAX_TEX/16)
extern unsigned short videogl_selective[256];
extern unsigned short videogl_selective_cache[VIDEO_GL_SELECTIVE_MAX_CACHE*256];
#endif

#ifndef VIDEO_GL_RASTER
#define VIDEO_GL_RASTER 64
#endif

extern SDL_Surface *videogl_screen;
extern void *videogl_screen_buffer;
extern GLint videogl_screen_texture;
extern unsigned videogl_screen_endY;
extern float videogl_z;
extern GLint videogl_tex[VIDEO_GL_MAX_TEX];
extern unsigned short *videogl_cache;
extern unsigned short videogl_cached[VIDEO_GL_MAX_TEX];

typedef struct {
	unsigned char z,x,y,flip,clipx,clipy;
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
	unsigned char pal;
#endif
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	unsigned char add;
#if !defined(USE_VGL_LARGE_TABLE) || !defined(USE_VGL_TABLE)
       	unsigned char y1, mosaic;
#endif
#endif
	unsigned short tile;
}videogl_tile_list_t;

extern videogl_tile_list_t videogl_tile[VIDEO_GL_MAX_TEX];
extern videogl_tile_list_t *videogl_tile_p[VIDEO_GL_MAX_TEX];
extern unsigned short videogl_tiles_z[256];

extern unsigned videogl_tiles;
extern unsigned char videogl_min_z, videogl_max_z;

extern int videogl_used_addfix;
extern unsigned short videogl_addfix_color[VIDEO_GL_RASTER];
extern int videogl_addfix_x0, videogl_addfix_y0, videogl_addfix_x1, videogl_addfix_y1;

extern unsigned char videogl_first_brightness;
extern unsigned char videogl_brightness;

extern int videogl_used_sub;
extern unsigned short videogl_sub_color;
extern int videogl_sub_x0, videogl_sub_y0, videogl_sub_x1, videogl_sub_y1;

extern unsigned short videogl_background_color[VIDEO_GL_RASTER];
extern int videogl_used_screen;

extern int videogl_fps_enabled;
extern void *videogl_fps_buffer;

extern struct ClipData *videogl_clip;
extern int videogl_clip_enabled;

extern unsigned short videogl_cache_slot[VIDEO_GL_MAX_TEX];
extern unsigned short videogl_cache_tile_slot[VIDEO_GL_MAX_TEX];
extern unsigned videogl_cache_slot_last;

extern int videogl_hw_render;
extern int videogl_8bit_tiles;

#ifdef DREAMCAST
extern void *videogl_dc_buffer;
#endif

extern int videogl_cache_to_reseted;

void videogl_flip(SDL_bool);
SDL_bool videogl_init(void);
void videogl_change_render(int);


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
