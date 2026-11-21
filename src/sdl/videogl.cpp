#if 0
def DREAMCAST
#include"dc/pvr.h"
#include<kos.h>
#endif
#include "snes4all.h"
#include "videogl.h"

#if defined(DREAMCAST) && defined(USE_OPC_ASM)
extern unsigned videogl_changed_colors_back;
extern int videogl_info_enabled;
extern int videogl_back_no_tiles;
extern void *videogl_info_buffer;
extern void *videogl_black_buffer;
extern void *videogl_background_buffer;
extern void *videogl_addfix_buffer;
extern void *videogl_sub_buffer;
extern unsigned char videogl_changed_color[256];
extern unsigned short videogl_actual_color[256];
#else
int videogl_back_no_tiles=0;
int videogl_cache_reseted=1;
int videogl_cache_to_reseted=0;
unsigned char videogl_first_brightness=0;
unsigned char videogl_brightness=0;
unsigned short *videogl_cache=NULL;
unsigned videogl_tiles=0;
unsigned char videogl_min_z=255;
unsigned char videogl_max_z=0;
int videogl_hw_render=-1;
unsigned videogl_changed_colors=0;
unsigned videogl_changed_colors_back=0;
unsigned videogl_screen_endY=0;
struct ClipData *videogl_clip=NULL;
int videogl_clip_enabled=0;
int videogl_used_screen=0;
int videogl_used_addfix=0;
int videogl_addfix_x0=0;
int videogl_addfix_y0=256;
int videogl_addfix_x1=256;
int videogl_addfix_y1=0;
int videogl_used_sub=0;
unsigned short videogl_sub_color=0x8000;
int videogl_sub_x0=0;
int videogl_sub_y0=256;
int videogl_sub_x1=256;
int videogl_sub_y1=0;
int videogl_8bit_tiles=0;
void *videogl_info_buffer=NULL;
void *videogl_black_buffer=NULL;
void *videogl_background_buffer=NULL;
void *videogl_addfix_buffer=NULL;
void *videogl_sub_buffer=NULL;
void *videogl_screen_buffer=NULL;
GLint videogl_screen_texture=0;
SDL_Surface *videogl_screen=NULL;
float videogl_z=0.0;
int videogl_fps_enabled=0;
void *videogl_fps_buffer=NULL;
int videogl_info_enabled=0;
unsigned char videogl_changed_color[256];
unsigned short videogl_actual_color[256];
unsigned short videogl_tiles_z[256];
unsigned short videogl_cached[VIDEO_GL_MAX_TEX];
unsigned short videogl_background_color[VIDEO_GL_RASTER];
unsigned short videogl_addfix_color[VIDEO_GL_RASTER];
#ifndef USE_VIDEOGL_DIRECT_SLOT
unsigned videogl_cache_slot_last=0;
unsigned short videogl_cache_slot[VIDEO_GL_MAX_TEX];
unsigned short videogl_cache_tile_slot[VIDEO_GL_MAX_TEX];
#endif
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
unsigned short videogl_selective[256];
unsigned short videogl_selective_cache[VIDEO_GL_SELECTIVE_MAX_CACHE*256];
#endif
videogl_tile_list_t videogl_tile[VIDEO_GL_MAX_TEX];
#endif

#ifndef DREAMCAST
GLint videogl_info_texture=0;
GLint videogl_fps_texture=0;
GLint videogl_black_texture=0;
GLint videogl_background_texture=0;
GLint videogl_addfix_texture=0;
GLint videogl_sub_texture=0;
GLint videogl_tex[VIDEO_GL_MAX_TEX];
#else
static pvr_poly_hdr_t videogl_polyhdr;
extern pvr_poly_cxt_t gl_poly_cxt;
#define DREAMCAST_FAST_TILE_PER_TEXTURE
#define DREAMCAST_USE_DMA_FOR_SCREEN
#endif

#define PREPARE_TILE(WIDTH,HEIGHT,BLEND_SRC,BLEND_DST) \
		gl_poly_cxt.txr.filter= PVR_FILTER_TRILINEAR2 /*SNES4ALL_FILTER_NONE*/; \
		gl_poly_cxt.gen.alpha = PVR_ALPHA_DISABLE; \
		gl_poly_cxt.txr.alpha = PVR_TXRALPHA_ENABLE; \
		gl_poly_cxt.blend.src = (BLEND_SRC); \
		gl_poly_cxt.blend.dst = (BLEND_DST); \
		gl_poly_cxt.gen.culling = PVR_CULLING_NONE; \
		gl_poly_cxt.txr.width = WIDTH; \
		gl_poly_cxt.txr.height = HEIGHT; \
		gl_poly_cxt.txr.format = GL_ARGB1555
#define PUT_VERTEX(X,Y,TX,TY,Z,CMD) \
	{ \
		pvr_vertex_t *vert=pvr_dr_target(dr_state); \
		vert->argb = 0xffffffff; \
		vert->oargb = 0xff000000; \
		vert->flags = CMD; \
		vert->x = X; \
		vert->y = Y; \
		vert->z = Z; \
		vert->u = TX; \
		vert->v = TY; \
		pvr_dr_commit(vert); \
	}
#define PUT_TILE(X0,Y0,X1,Y1,TX0,TY0,TX1,TY1,Z) \
	{ \
		static pvr_dr_state_t  dr_state; \
		pvr_dr_init(dr_state); \
		PUT_VERTEX(X0,Y0,TX0,TY0,Z,PVR_CMD_VERTEX) \
		PUT_VERTEX(X1,Y0,TX1,TY0,Z,PVR_CMD_VERTEX) \
		PUT_VERTEX(X0,Y1,TX0,TY1,Z,PVR_CMD_VERTEX) \
		PUT_VERTEX(X1,Y1,TX1,TY1,Z,PVR_CMD_VERTEX_EOL) \
	}

#ifdef USE_VGL_TABLE
#ifdef USE_VGL_LARGE_TABLE
#include "vgl.h"
#else
static float _vgl_tx0[]={
	0.000, 1.000, 0.000, 1.000,
	0.000, 0.875, 0.000, 0.875,
	0.000, 0.750, 0.000, 0.750,
	0.000, 0.625, 0.000, 0.625,
	0.000, 0.500, 0.000, 0.500,
	0.000, 0.375, 0.000, 0.375,
	0.000, 0.250, 0.000, 0.250,
	0.000, 0.125, 0.000, 0.125,
};

static float _vgl_tx1[]={
	1.000, 0.000, 1.000, 0.000,
	0.875, 0.000, 0.875, 0.000,
	0.750, 0.000, 0.750, 0.000,
	0.625, 0.000, 0.625, 0.000,
	0.500, 0.000, 0.500, 0.000,
	0.375, 0.000, 0.375, 0.000,
	0.250, 0.000, 0.250, 0.000,
	0.125, 0.000, 0.125, 0.000,
};
static float _vgl_ty0[]={
	0.000, 0.000, 1.000, 1.000,
	0.000, 0.000, 0.875, 0.875,
	0.000, 0.000, 0.750, 0.750,
	0.000, 0.000, 0.625, 0.625,
	0.000, 0.000, 0.500, 0.500,
	0.000, 0.000, 0.375, 0.375,
	0.000, 0.000, 0.250, 0.250,
	0.000, 0.000, 0.125, 0.125,
};
static float _vgl_ty1[]={
	1.000, 1.000, 0.000, 0.000,
	0.875, 0.875, 0.000, 0.000,
	0.750, 0.750, 0.000, 0.000,
	0.625, 0.625, 0.000, 0.000,
	0.500, 0.500, 0.000, 0.000,
	0.375, 0.375, 0.000, 0.000,
	0.250, 0.250, 0.000, 0.000,
	0.125, 0.125, 0.000, 0.000,
};
#endif
#endif

#if defined(DISPLAY_FRAMERATE) && !defined(DEBUG_FRAMESKIP)
static void videogl_draw_fps(void) {
	if (videogl_fps_enabled) {
		videogl_fps_enabled=0;
#ifndef DREAMCAST
		glEnable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, videogl_fps_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 64, 8, 0,
			GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_fps_buffer);
		loadTextureParams(SNES4ALL_FILTER_NONE);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0,0.0);
			glVertex3f(0.0,0.0,videogl_z);
			glTexCoord2f(1.0,0.0);
			glVertex3f(64.0,0,videogl_z);
			glTexCoord2f(1.0,1.0);
			glVertex3f(64.0,8.0,videogl_z);
			glTexCoord2f(0.0,1.0);
			glVertex3f(0.0,8.0,videogl_z);
		glEnd();
#else
	float x0 = 0.0;
	float y0 = 0.0;
	float x1 = 64.0*(640.0f/256.0f);
	float y1 = 8.0*(480.0f/240.0f);
	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;
	PREPARE_TILE(64,8 ,PVR_BLEND_SRCALPHA, PVR_BLEND_INVSRCALPHA);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_fps_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
	}
}
#else
#define videogl_draw_fps()
#endif

static void videogl_draw_info(void) {
#ifndef DREAMCAST
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, videogl_info_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 8, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_info_buffer);
	loadTextureParams(SNES4ALL_FILTER_NONE);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(0.0,232.0,videogl_z);
		glTexCoord2f(1.0,0.0);
		glVertex3f(256.0,232.0,videogl_z);
		glTexCoord2f(1.0,1.0);
		glVertex3f(256.0,240.0,videogl_z);
		glTexCoord2f(0.0,1.0);
		glVertex3f(0.0,240.0,videogl_z);
	glEnd();
#else
	float x0 = 0.0f;
	float y0 = 228.0f*(480.0f/240.0f);
	float x1 = 256.0f*(640.0f/256.0f);
	float y1 = 236.0f*(480.0f/240.0f);
	float tx0 = 0.0f;
	float ty0 = 0.0f;
	float tx1 = 1.0f;
	float ty1 = 1.0f;
	PREPARE_TILE(256,8 ,PVR_BLEND_SRCALPHA, PVR_BLEND_INVSRCALPHA);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_info_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
}

static void videogl_draw_screen(void) {
#ifndef DREAMCAST
	if (videogl_hw_render)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D,videogl_screen_texture);
	loadTextureParams(SNES4ALL_FILTER_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_screen_buffer);
	float x1=0.0,y1=0.0,x2=256.0,y2=256.0;//274.0;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x1,y1,videogl_z);
		glTexCoord2f(1.0,0.0);
		glVertex3f(x2,y1,videogl_z);
		glTexCoord2f(1.0,1.0);
		glVertex3f(x2,y2,videogl_z);
		glTexCoord2f(0.0,1.0);
		glVertex3f(x1,y2,videogl_z);
	glEnd();
	if (!videogl_hw_render)
		glEnable(GL_BLEND);
#else
	float x0 = 0.0;
	float y0 = 0.0;
	float x1 = 256.0*(640.0f/256.0f);
	float y1 = /*274.0*/256.0*(480.0f/240.0f);
	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;
#ifndef DREAMCAST_USE_DMA_FOR_SCREEN
	if (!videogl_screen_endY)
		sq_cpy(videogl_screen_buffer,GFX.Screen,240*256*2);
	else
		sq_cpy(videogl_screen_buffer,GFX.Screen,videogl_screen_endY*256*2);
#else
//	while (!pvr_dma_ready());
	if (!videogl_screen_endY) {
		dcache_flush_range(((uint32)GFX.Screen)+180*256*2,60*256*2);
		pvr_txr_load_dma(GFX.Screen,videogl_screen_buffer,240*256*2,-1,NULL,NULL);
	}else if (videogl_screen_endY>60) {
		dcache_flush_range(((uint32)GFX.Screen)+(videogl_screen_endY-60)*256*2,60*256*2);
		pvr_txr_load_dma(GFX.Screen,videogl_screen_buffer,videogl_screen_endY*256*2,-1,NULL,NULL);
	}else {
		dcache_flush_range(((uint32)GFX.Screen),(videogl_screen_endY)*256*2);
		pvr_txr_load_dma(GFX.Screen,videogl_screen_buffer,videogl_screen_endY*256*2,-1,NULL,NULL);
	}
	videogl_screen_endY=0;
#endif
	PREPARE_TILE(256,256 ,videogl_hw_render?PVR_BLEND_SRCALPHA:PVR_BLEND_ONE, videogl_hw_render?PVR_BLEND_INVSRCALPHA:PVR_BLEND_ZERO);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_screen_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
}


static void videogl_draw_black(int _x0, int _y0, int _x1, int _y1){
#ifndef DREAMCAST
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D,videogl_black_texture);
	loadTextureParams(SNES4ALL_FILTER_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_black_buffer);
	float x0=_x0,y0=_y0,x1=_x1,y1=_y1;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x0,y0,videogl_z);
		glTexCoord2f(1.0,0.0);
		glVertex3f(x1,y0,videogl_z);
		glTexCoord2f(1.0,1.0);
		glVertex3f(x1,y1,videogl_z);
		glTexCoord2f(0.0,1.0);
		glVertex3f(x0,y1,videogl_z);
	glEnd();
#else
	float x0 = ((float)_x0)*(640.0f/256.0f);
	float y0 = ((float)_y0)*(480.0f/240.0f);
	float x1 = ((float)_x1)*(640.0f/256.0f);
	float y1 = ((float)_y1)*(480.0f/240.0f);
	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;
	PREPARE_TILE(8,8 ,PVR_BLEND_ONE, PVR_BLEND_ZERO); //SRCALPHA, PVR_BLEND_INVSRCALPHA);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_black_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
}

#if 0
static void videogl_draw_sub(void){
	static unsigned short back_col=0x8000;
	if (videogl_sub_color!=back_col) {
		back_col=videogl_sub_color;
		for(int i=0;i<8*8;i++)
			((unsigned short*)(videogl_sub_buffer))[i]=back_col;
	}
//printf("draw_sub %.4X (%i %i %i %i)\n",videogl_sub_color,videogl_sub_x0,videogl_sub_y0, videogl_sub_x1,videogl_sub_y1);
#ifndef DREAMCAST
//	glDisable(GL_BLEND);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ONE);
	glBindTexture(GL_TEXTURE_2D,videogl_sub_texture);
	loadTextureParams(SNES4ALL_FILTER_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_sub_buffer);
	float x1=videogl_sub_x0, y1=videogl_sub_y0, x2=videogl_sub_x1, y2=videogl_sub_y1;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x1,y1,videogl_z);
		glTexCoord2f(1.0,0.0);
		glVertex3f(x2,y1,videogl_z);
		glTexCoord2f(1.0,1.0);
		glVertex3f(x2,y2,videogl_z);
		glTexCoord2f(0.0,1.0);
		glVertex3f(x1,y2,videogl_z);
	glEnd();
#else
	float x0 = ((float)videogl_sub_x0)*(640.0f/256.0f);
	float y0 = ((float)videogl_sub_y0)*(480.0f/240.0f);
	float x1 = ((float)videogl_sub_x1)*(640.0f/256.0f);
	float y1 = ((float)videogl_sub_y1)*(480.0f/240.0f);
	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;
	PREPARE_TILE(8,8 ,PVR_BLEND_ONE, PVR_BLEND_ZERO);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_sub_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif

	videogl_z+=VIDEO_GL_TILE_Z_INC;
}
#endif

static void videogl_draw_background(void){
#ifndef DREAMCAST
	for(unsigned i=0;i<VIDEO_GL_RASTER;i++)
		for(int j=0;j<8;j++) 
			((unsigned short*)(videogl_background_buffer))[j+(i*8)]=videogl_background_color[i];
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D,videogl_background_texture);
	loadTextureParams(SNES4ALL_FILTER_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, VIDEO_GL_RASTER, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_background_buffer);
	float x1=0.0, y1=0.0, x2=256.0, y2=240.0;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x1,y1,videogl_z);
		glTexCoord2f(1.0,0.0);
		glVertex3f(x2,y1,videogl_z);
		glTexCoord2f(1.0,1.0);
		glVertex3f(x2,y2,videogl_z);
		glTexCoord2f(0.0,1.0);
		glVertex3f(x1,y2,videogl_z);
	glEnd();
	glEnable(GL_BLEND);
#else
	{
#ifndef USE_MMU
		unsigned int *d = (unsigned int *)(void *)
			(0xe0000000 | (((unsigned long)videogl_background_buffer) & 0x03ffffe0));
		QACR0 = ((((unsigned int)videogl_background_buffer)>>26)<<2)&0x1c;
		QACR1 = ((((unsigned int)videogl_background_buffer)>>26)<<2)&0x1c;
#else
		unsigned int *d = (unsigned int *)(void *)
			((0xe0000000 | (((unsigned long)videogl_background_buffer) & 0x03ffffe0))|((((unsigned)videogl_background_buffer)>>3)&0x3800000));
#endif
		for(unsigned m=0;m<VIDEO_GL_RASTER;m+=2){
			register unsigned dat=videogl_background_color[m]|((videogl_background_color[m])<<16);
			d[0]=dat; d[1]=dat; d[2]=dat; d[3]=dat;
			dat=videogl_background_color[m+1]|((videogl_background_color[m+1])<<16);
			d[4]=dat; d[5]=dat; d[6]=dat; d[7]=dat;
			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		d = (unsigned int *)0xe0000000;
		d[0] = d[8] = 0;
	}
	float x0 = 0.0;//*(640.0f/256.0f);
	float y0 = 0.0;//*(480.0f/240.0f);
	float x1 = 256.0*(640.0f/256.0f);
	float y1 = 240.0*(480.0f/240.0f);
	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;
	PREPARE_TILE(8,VIDEO_GL_RASTER ,PVR_BLEND_ONE, PVR_BLEND_ZERO); //SRCALPHA, PVR_BLEND_INVSRCALPHA);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_background_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
}

static void videogl_draw_addfix(void){
#ifndef DREAMCAST
	for(unsigned i=0;i<VIDEO_GL_RASTER;i++)
		for(int j=0;j<8;j++) 
			((unsigned short*)(videogl_addfix_buffer))[j+(i*8)]=videogl_addfix_color[i];
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBindTexture(GL_TEXTURE_2D,videogl_addfix_texture);
	loadTextureParams(SNES4ALL_FILTER_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, VIDEO_GL_RASTER, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, videogl_addfix_buffer);
	float x1=videogl_addfix_x0, y1=videogl_addfix_y0, x2=videogl_addfix_x1, y2=videogl_addfix_y1;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x1,y1,videogl_z);
		glTexCoord2f(1.0,0.0);
		glVertex3f(x2,y1,videogl_z);
		glTexCoord2f(1.0,1.0);
		glVertex3f(x2,y2,videogl_z);
		glTexCoord2f(0.0,1.0);
		glVertex3f(x1,y2,videogl_z);
	glEnd();
#else
	{
#ifndef USE_MMU
		unsigned int *d = (unsigned int *)(void *)
			(0xe0000000 | (((unsigned long)videogl_addfix_buffer) & 0x03ffffe0));
		QACR0 = ((((unsigned int)videogl_addfix_buffer)>>26)<<2)&0x1c;
		QACR1 = ((((unsigned int)videogl_addfix_buffer)>>26)<<2)&0x1c;
#else
		unsigned int *d = (unsigned int *)(void *)
			((0xe0000000 | (((unsigned long)videogl_addfix_buffer) & 0x03ffffe0))|((((unsigned)videogl_background_buffer)>>3)&0x3800000));
#endif
		for(unsigned m=0;m<VIDEO_GL_RASTER;m+=2){
			register unsigned dat=videogl_addfix_color[m]|((videogl_addfix_color[m])<<16);
			d[0]=dat; d[1]=dat; d[2]=dat; d[3]=dat;
			dat=videogl_addfix_color[m+1]|((videogl_addfix_color[m+1])<<16);
			d[4]=dat; d[5]=dat; d[6]=dat; d[7]=dat;
			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		d = (unsigned int *)0xe0000000;
		d[0] = d[8] = 0;
	}
	float x0 = ((float)videogl_addfix_x0)*(640.0f/256.0f);
	float y0 = ((float)videogl_addfix_y0)*(480.0f/240.0f);
	float x1 = ((float)videogl_addfix_x1)*(640.0f/256.0f);
	float y1 = ((float)videogl_addfix_y1)*(480.0f/240.0f);
	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;
	PREPARE_TILE(8,VIDEO_GL_RASTER ,PVR_BLEND_ONE, PVR_BLEND_ONE);
	static pvr_poly_hdr_t polyhdr;
	gl_poly_cxt.txr.base =(unsigned short *)videogl_addfix_buffer;
	pvr_poly_compile(&polyhdr, &gl_poly_cxt);
	pvr_prim(&polyhdr, sizeof(pvr_poly_hdr_t));
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
}

static __inline__ void videogl_draw_tile(videogl_tile_list_t *tile) {
//if (tile->tile>VIDEO_GL_MAX_TEX) printf("TILE %i> %i\n",tile->tile,VIDEO_GL_MAX_TEX);
#ifdef USE_VGL_TABLE
#ifndef USE_VGL_LARGE_TABLE
	const float tx0=_vgl_tx0[tile->flip];
	const float tx1=_vgl_tx1[tile->flip];
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
	const float ty0=_vgl_ty0[tile->flip]+((1.0/8.0)*(tile->clipy));
	const float ty1=((_vgl_ty1[tile->flip]/8.0)*(tile->y1))+((1.0/8.0)*(tile->clipy));
#else
	const float ty0=_vgl_ty0[tile->flip]+((1.0/8.0)*(tile->clipy&0xf));
	const float ty1=((_vgl_ty1[tile->flip]/8.0)*(tile->clipy>>4))+((1.0/8.0)*(tile->clipy&0xf));
#endif
#else
	const float tx0=vgl_tx0[tile->flip];
	const float tx1=vgl_tx1[tile->flip];
	const float ty0=vgl_ty0[((unsigned)tile->flip)+(((unsigned)tile->clipy)<<5)];
	const float ty1=vgl_ty1[((unsigned)tile->flip)+(((unsigned)tile->clipy)<<5)];
#endif
#else
	float tx0, tx1, ty0, ty1;
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
	switch(tile->flip) {
#else
	switch(tile->flip&3) {
#endif
		case 0: 
			tx0=0.000;
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
			tx1=1.000-(0.125*(tile->mosaic));
			ty1=(((1.000-(0.125*(tile->mosaic)))/8.0)*(tile->y1))+(0.125*(tile->clipy));
			ty0=0.000+(0.125*(tile->clipy));
#else
			tx1=1.000-(0.125*(tile->flip>>2));
			ty1=(((1.000-(0.125*(tile->flip>>2)))/8.0)*(tile->clipy>>4))+(0.125*(tile->clipy&0xf));
			ty0=0.000+(0.125*(tile->clipy&0xf));
#endif
			break;
		case 1:
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
			tx0=1.000-(0.125*(tile->mosaic));
			ty1=(((1.000-(0.125*(tile->mosaic)))/8.0)*(tile->y1))+(0.125*(tile->clipy));
			ty0=0.000+(0.125*(tile->clipy));
#else
			tx0=1.000-(0.125*(tile->flip>>2));
			ty1=(((1.000-(0.125*(tile->flip>>2)))/8.0)*(tile->clipy>>4))+(0.125*(tile->clipy&0xf));
			ty0=0.000+(0.125*(tile->clipy&0xf));
#endif
		       	tx1=0.000;
			break;
		case 2:
			tx0=0.000;
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
			tx1=1.000-(0.125*(tile->mosaic));
			ty0=1.000-(0.125*(tile->mosaic))+(0.125*(tile->clipy));
			ty1=0.000+(0.125*(tile->clipy));
#else
			tx1=1.000-(0.125*(tile->flip>>2));
			ty0=1.000-(0.125*(tile->flip>>2))+(0.125*(tile->clipy&0xf));
			ty1=0.000+(0.125*(tile->clipy&0xf));
#endif
			break;
		default:
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
			tx0=1.000-(0.125*(tile->mosaic));
			ty0=1.000-(0.125*(tile->mosaic))+(0.125*(tile->clipy));
			ty1=0.000+(0.125*(tile->clipy));
#else
			tx0=1.000-(0.125*(tile->flip>>2));
			ty0=1.000-(0.125*(tile->flip>>2))+(0.125*(tile->clipy&0xf));
			ty1=0.000+(0.125*(tile->clipy&0xf));
#endif
			tx1=0.000;
	}
#endif
#ifdef DREAMCAST
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	const float x0=((float)((tile->x)-(tile->clipx)))*(640.0f/256.0f);
#else
	const float x0=((float)((tile->x)-(tile->clipx&0x7)))*(640.0f/256.0f);
#endif
	const float y0=((float)(tile->y))*(480.0f/240.0f);
	const float x1=x0+(8.0*(640.0f/256.0f));
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
	const float y1=(y0+((tile->y1))*(480.0f/240.0f));
#else
	const float y1=(y0+((tile->clipy>>4))*(480.0f/240.0f));
#endif
#else
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	float x0 = ((tile->x)-(tile->clipx));
#else
	float x0 = ((tile->x)-(tile->clipx&0x7));
#endif
	float y0 = (tile->y); //-(tile->clipy&0xf));
	float x1 = x0 + 8.0;
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
	float y1 = y0 + (tile->y1);
#else
	float y1 = y0 + (tile->clipy>>4);
#endif
#endif
#ifdef USE_VIDEOGL_DIRECT_SLOT
	const void *txr_buf=(unsigned short *)(((unsigned)videogl_cache)+(tile->tile*8*8*2));
#else
	const void *txr_buf=(unsigned short *)(((unsigned)videogl_cache)+(videogl_cache_tile_slot[tile->tile]*8*8*2));
#endif
//printf("draw x0=%f, y0=%f, x1=%f, y1=%f, ty0=%f, ty1=%f\n",(double)x0,(double)y0,(double)x1,(double)y1,(double)ty0,(double)ty1);
//printf("draw x=%i, y=%i, flip=%X, tile=%i, z=%f (%i)\n",tile->x,tile->y,tile->flip,tile->tile,videogl_z,tile->z);

#ifndef DREAMCAST
	glBindTexture(GL_TEXTURE_2D, videogl_tex[tile->tile]);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	if (tile->add) glBlendFunc(GL_ONE,GL_ONE);
#else
	if (tile->clipx&8) glBlendFunc(GL_ONE,GL_ONE);
#endif
	loadTextureParams(SNES4ALL_FILTER_NONE);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0,
			GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, txr_buf);
	glBegin(GL_QUADS);
		glTexCoord2f(tx0,ty0);
		glVertex3f(x0,y0,videogl_z);
		glTexCoord2f(tx1,ty0);
		glVertex3f(x1,y0,videogl_z);
		glTexCoord2f(tx1,ty1);
		glVertex3f(x1,y1,videogl_z);
	glTexCoord2f(tx0,ty1);
		glVertex3f(x0,y1,videogl_z);
	glEnd();
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	if (tile->add) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
	if (tile->clipx&8) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
#else
#ifndef DREAMCAST_FAST_TILE_PER_TEXTURE
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	if (tile->add) {gl_poly_cxt.blend.src = PVR_BLEND_ONE;/*SRCALPHA;*/ gl_poly_cxt.blend.dst = PVR_BLEND_ONE; }
#else
	if (tile->clipx&8) {gl_poly_cxt.blend.src = PVR_BLEND_ONE;/*SRCALPHA;*/ gl_poly_cxt.blend.dst = PVR_BLEND_ONE; }
#endif
	gl_poly_cxt.txr.base = txr_buf;
	pvr_poly_compile(&videogl_polyhdr, &gl_poly_cxt);
	pvr_prim(&videogl_polyhdr, sizeof(pvr_poly_hdr_t));
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	if (tile->add) { gl_poly_cxt.blend.src = PVR_BLEND_SRCALPHA; gl_poly_cxt.blend.dst = PVR_BLEND_INVSRCALPHA; }
#else
	if (tile->clipx&8) { gl_poly_cxt.blend.src = PVR_BLEND_SRCALPHA; gl_poly_cxt.blend.dst = PVR_BLEND_INVSRCALPHA; }
#endif
#else
	unsigned txr_base=(unsigned)txr_buf;
	uint32 mode2=videogl_polyhdr.mode2;
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	if (tile->add) videogl_polyhdr.mode2 = (mode2 & (~(PVR_TA_PM2_SRCBLEND_MASK|PVR_TA_PM2_DSTBLEND_MASK))) | (PVR_BLEND_ONE << PVR_TA_PM2_SRCBLEND_SHIFT) | (PVR_BLEND_ONE << PVR_TA_PM2_DSTBLEND_SHIFT);
#else
	if (tile->clipx&8) videogl_polyhdr.mode2 = (mode2 & (~(PVR_TA_PM2_SRCBLEND_MASK|PVR_TA_PM2_DSTBLEND_MASK))) | (PVR_BLEND_ONE << PVR_TA_PM2_SRCBLEND_SHIFT) | (PVR_BLEND_ONE << PVR_TA_PM2_DSTBLEND_SHIFT);
#endif
	txr_base = (txr_base & 0x00fffff8) >> 3;
	videogl_polyhdr.mode3=(videogl_polyhdr.mode3&0xffe00000)|txr_base;
	{
		unsigned int *s = (unsigned int *)&videogl_polyhdr;
#ifndef USE_MMU
		unsigned int *d = (unsigned int *)(void *)
			(0xe0000000 | (((unsigned long)PVR_TA_INPUT) & 0x03ffffe0));
		QACR0 = ((((unsigned int)PVR_TA_INPUT)>>26)<<2)&0x1c;
		QACR1 = ((((unsigned int)PVR_TA_INPUT)>>26)<<2)&0x1c;
#else
		unsigned int *d = (unsigned int *)(void *)
			((0xe0000000 | (((unsigned long)PVR_TA_INPUT) & 0x03ffffe0))|((((unsigned)PVR_TA_INPUT)>>3)&0x3800000));
#endif
		unsigned n=sizeof(pvr_poly_hdr_t)>>5;
		while(n--) {
			asm("pref @%0" : : "r" (s + 8));
			d[0] = *(s++); d[1] = *(s++); d[2] = *(s++); d[3] = *(s++);
			d[4] = *(s++); d[5] = *(s++); d[6] = *(s++); d[7] = *(s++);
			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		d = (unsigned int *)0xe0000000;
		d[0] = d[8] = 0;
	}
#endif
	PUT_TILE(x0,y0,x1,y1,tx0,ty0,tx1,ty1,videogl_z)
#ifdef DREAMCAST_FAST_TILE_PER_TEXTURE
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	if (tile->add) videogl_polyhdr.mode2=mode2;
#else
	if (tile->clipx&8) videogl_polyhdr.mode2=mode2;
#endif
#endif
#endif
	videogl_z+=VIDEO_GL_TILE_Z_INC;
}

static void videogl_draw_tiles(void) {
//printf("DRAW_TILES %i\n",videogl_tiles);
#ifdef DREAMCAST
	PREPARE_TILE(8,8 ,PVR_BLEND_SRCALPHA, PVR_BLEND_INVSRCALPHA);
#ifdef DREAMCAST_FAST_TILE_PER_TEXTURE
	pvr_poly_compile(&videogl_polyhdr, &gl_poly_cxt);
#endif
#else
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	if (videogl_used_screen) {
		if (videogl_min_z>videogl_used_screen)
			videogl_min_z=videogl_used_screen;
		if (videogl_max_z<videogl_used_screen)
			videogl_max_z=videogl_used_screen;
	}
	register const unsigned char nm=videogl_max_z;
	for(unsigned char n=videogl_min_z;n<=nm;n++){
		int z=videogl_tiles_z[n];
		if (z) {
//			for(int i=videogl_tiles-1;i>=0;--i) {
			register const int imax=videogl_tiles;
			for(int i=0;i<imax;i++) {
				if (videogl_tile[i].z==n) {
					videogl_draw_tile(&videogl_tile[i]);
					z--;
					if (!z) break;
				}
			}
		}
		videogl_z+=VIDEO_GL_TILE_Z_INC;
		if (videogl_used_screen==n)
			videogl_draw_screen();
	}
	ZeroMemory(&videogl_tiles_z[0],sizeof(videogl_tiles_z));
#ifndef USE_VIDEOGL_SELECTIVE_CACHE_RESET
	videogl_tiles=0;
#endif
}

void videogl_flip(SDL_bool ingame)
{
	if (!ingame)
	{
#ifdef DREAMCAST
		glKosBeginFrame();
#endif
		glClearColor( 0.0,0.0,0.0, 255.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D,videogl_screen_texture);
		loadTextureParams(SNES4ALL_FILTER_BILINEAR);

#ifndef DREAMCAST
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
			GL_RGB, GL_UNSIGNED_SHORT_5_6_5, videogl_screen_buffer);
#else
		glKosTex2D(GL_RGB565,256,256, videogl_screen_buffer);
#endif

		double t_x1=0.0,t_y1=0.0,t_x2=256.0,t_y2=256.0;

#ifdef DREAMCAST
		t_y1+=4.0; t_y2+=4.0;
#endif
		glBegin(GL_QUADS);
			glTexCoord2f(0.0,0.0);
			glVertex3f(t_x1,t_y1,0.1);
			glTexCoord2f(1.0,0.0);
			glVertex3f(t_x2,t_y1,0.1);
			glTexCoord2f(1.0,1.0);
			glVertex3f(t_x2,t_y2,0.1);
			glTexCoord2f(0.0,1.0);
			glVertex3f(t_x1,t_y2,0.1);
		glEnd();

#ifndef DREAMCAST
		SDL_GL_SwapBuffers();
#else
		glKosFinishFrame();
#endif
	}
	else
	{
		videogl_z=VIDEO_GL_TILE_Z_INIT;
		if (!videogl_tiles)  {
			videogl_back_no_tiles=1;
			videogl_first_brightness=PPU.Brightness;
		} else if (videogl_back_no_tiles) {
			if (videogl_brightness!=PPU.Brightness && !videogl_8bit_tiles) {
				videogl_first_brightness=videogl_brightness=PPU.Brightness;
				S9xFixColourBrightness();
				videogl_cache_reseted=videogl_cache_to_reseted=1;
				GFX.FixedColour = BUILD_PIXEL (IPPU.XB [PPU.FixedColourRed], IPPU.XB [PPU.FixedColourGreen],IPPU.XB [PPU.FixedColourBlue]);
				for(unsigned i=0;i<VIDEO_GL_RASTER;i++){
					videogl_background_color[i]=IPPU.ScreenColors [0];
					videogl_addfix_color[i]=GFX.FixedColour;
				}
			}
		}
#ifdef DREAMCAST
		glKosBeginFrame();
		pvr_set_bg_color( ((videogl_background_color[0]>>10)&0x1f)/31.0,
				  ((videogl_background_color[0]>>5)&0x1f)/31.0,
				  ((videogl_background_color[0]>>0)&0x1f)/31.0);

#else
		glClearColor(	((videogl_background_color[0]>>0)&0x1f)/31.0,
				((videogl_background_color[0]>>5)&0x1f)/31.0,
				((videogl_background_color[0]>>10)&0x1f)/31.0,
			      1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
		videogl_draw_background();
		if (!videogl_hw_render || (!videogl_tiles && videogl_used_screen))
			videogl_draw_screen();
#if 0
		if (videogl_used_sub)
			videogl_draw_sub();
#endif
		if (videogl_used_addfix)
			videogl_draw_addfix();
#ifdef DREAMCAST
//		glKosFinishList();
#endif
		if (videogl_hw_render && videogl_tiles && !videogl_back_no_tiles)
			videogl_draw_tiles();
		else
			if (videogl_tiles)
				videogl_back_no_tiles=0;
		if (PPU.ScreenHeight<240)
			videogl_draw_black(0,PPU.ScreenHeight-(videogl_used_screen?1:0),256,240);
		if (videogl_clip) {
			for (unsigned i=0;i<videogl_clip->Count[videogl_clip_enabled];i++) {
				unsigned Left=videogl_clip->Left[i][videogl_clip_enabled];
				unsigned Right=videogl_clip->Right[i][videogl_clip_enabled];
//printf("CLIP %i %i\n",videogl_clip->Left[i][videogl_clip_enabled],videogl_clip->Right[i][videogl_clip_enabled]);
				if (Right<=Left) continue;
				if (Left) 
					videogl_draw_black(0,0,Left,240);
				if (Right<256)
					videogl_draw_black(Right,0,256,240);
			}
		}
		videogl_draw_fps();
		if (videogl_info_enabled) {
			videogl_draw_info();
			videogl_info_enabled=0;
		}
#ifndef DREAMCAST
		SDL_GL_SwapBuffers();
#else
#ifdef DREAMCAST_USE_DMA_FOR_SCREEN
//		while (!pvr_dma_ready());
#endif
		glKosFinishFrame();
#endif
		if (videogl_hw_render) {
		if (videogl_changed_colors && !videogl_cache_to_reseted) {
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
//unsigned destr=0, iter=0;
			if (videogl_8bit_tiles /*|| videogl_tiles<900*/) {
				videogl_changed_colors=0;
				videogl_cache_to_reseted=4;
			} else
			for(unsigned j=0;(j<256)&&(videogl_changed_colors);j+=USE_VIDEOGL_SELECTIVE_CACHE_RESET) {
			unsigned changed=0;
			const unsigned imax=j+16;
			for(unsigned i=j;i<imax;i++) {
#else
			unsigned changed=0;
			for(unsigned i=0;(i<256)&&(videogl_changed_colors);i++) {
#endif
				const unsigned c=videogl_changed_color[i];
				if (c) {
					if (videogl_actual_color[i]!=IPPU.ScreenColors[i]) {
//printf("CAMBIADO %i (%i)\n",i,j);
						changed++;
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
						videogl_actual_color[i]=IPPU.ScreenColors[i];
#endif
					}
					videogl_changed_colors--;
					videogl_changed_color[i]=0;
				}
			}
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
			if (changed)
				for(unsigned i=j;i<imax;i++) {
					unsigned short *sel=&videogl_selective_cache[i*VIDEO_GL_SELECTIVE_MAX_CACHE];
					const unsigned kmax=videogl_selective[i];
					for(unsigned k=0;k<kmax;k++) {
//iter++;
						unsigned short tile=*sel++;
						if (videogl_cached[tile]) {
//printf("DESTRUYE Tile %i\n",tile);
//destr++;
							videogl_cached[tile]=0;
							videogl_cache_slot[videogl_cache_tile_slot[tile]]=0;
							videogl_cache_tile_slot[tile]=0xffff;
						}
					}
					videogl_selective[i]=0;
				}
			}
			videogl_changed_colors=0;
//if (destr) printf("Selective %i/%i (%.2f%%) inter=%i\n",destr,videogl_tiles,((double)destr)*100.0/((double)videogl_tiles),iter);
#else
			if (changed>40) {
				videogl_cache_reseted=1;
				videogl_changed_colors_back=0;
			}else
			if (changed>1) {
//puts("RESET por changed>");
				videogl_cache_to_reseted=8;
				videogl_changed_colors_back=0;
			} else {
				if (videogl_changed_colors_back){
					if (changed>videogl_changed_colors_back) {
//puts("RESET por changed!=");
						videogl_changed_colors_back=0;
						videogl_cache_to_reseted=8;
					}
				} else {
					videogl_changed_colors_back=changed;
				}
			}
		} else {
			if (videogl_changed_colors_back>1 && !videogl_cache_to_reseted)
				videogl_cache_to_reseted=8;
			videogl_changed_colors_back=0;
#endif
//if (cambiados) printf("SELECTIVE %i\n",cambiados);
		}
		if (videogl_brightness!=videogl_first_brightness) {
//printf("FIX: videogl_brightness=%.2X, first=%.2X, PPU=%.2X\n",videogl_brightness,videogl_first_brightness,PPU.Brightness);
			PPU.Brightness=videogl_first_brightness;
			S9xFixColourBrightness();
			videogl_brightness=videogl_first_brightness;
			videogl_cache_reseted=1;
		}
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
		videogl_tiles=0;
#endif
//static unsigned nframe=0;
//nframe++;
		if (videogl_cache_reseted) {
//static unsigned nreset=0;
//printf("RESET %i/%i!\n",nreset++,nframe);
			videogl_cache_reseted=0;
			videogl_cache_to_reseted=0;
			videogl_changed_colors_back=0;
			videogl_changed_colors=0;
			ZeroMemory(&videogl_changed_color[0],sizeof(videogl_changed_color));
			memcpy((void *)&videogl_actual_color,(void *)&IPPU.ScreenColors[0],256*sizeof(unsigned short));
			ZeroMemory(&videogl_cached[0],sizeof(videogl_cached));
#ifndef USE_VIDEOGL_DIRECT_SLOT
			ZeroMemory(&videogl_cache_slot[0],sizeof(videogl_cache_slot));
			memset(&videogl_cache_tile_slot[0],-1,sizeof(videogl_cache_tile_slot));
#endif
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
			ZeroMemory(&videogl_selective[0],sizeof(videogl_selective));
#endif
		}
		} // videogl_hw_render
		if (videogl_used_screen) {
			videogl_used_screen=0;
//			ZeroMemory(videogl_screen_buffer,sizeof(256*256*2));
		}
		if (videogl_used_addfix) {
			videogl_used_addfix=0;
			videogl_addfix_y0=256;
			videogl_addfix_y1=0;
		}
#if 0
		if (videogl_used_sub) {
			videogl_used_sub=0;
			videogl_sub_y0=256;
			videogl_sub_y1=0;
		}
#endif
		videogl_8bit_tiles=0;
		videogl_clip=NULL;
		videogl_clip_enabled=0;
		videogl_max_z=0;
		videogl_min_z=255;
	}
//puts("SWAP");
//SDL_Delay(333);
//static int pintado=0; if (pintado>1) {SDL_Event e; while(SDL_PollEvent(&e)) SDL_Delay(333); do{ while(SDL_PollEvent(&e)) if (e.type==SDL_QUIT || e.type==SDL_KEYDOWN) exit(0); SDL_Delay(333); }while(1);} else printf("---- FRAME %i ----\n",pintado); pintado++;
}

static void videogl_init_cache(void) {
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

#ifndef DREAMCAST
    videogl_screen_buffer=malloc(SNES4ALL_SCREEN_PITCH*256);
    videogl_cache=(unsigned short *)malloc(VIDEO_GL_MAX_TEX *8*8*2);
    for(int i=0;i<VIDEO_GL_MAX_TEX;i++)
    	glGenTextures(1,(GLuint *)&videogl_tex[i]);
#else
    unsigned dcpvrmem=(unsigned)pvr_mem_malloc((SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2)+(64*8*2)+(8*8*2)+(8*8*2)+(VIDEO_GL_RASTER*8*2)+(VIDEO_GL_RASTER*8*2)+(256*8*2));
    if (!dcpvrmem) {
	    puts("Unable to allow video memory");
	    exit(0);
    }
    videogl_screen_buffer=(void *)dcpvrmem;
    videogl_cache=(unsigned short *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256));
#endif

#ifndef USE_VIDEOGL_DIRECT_SLOT
    ZeroMemory(&videogl_cache_slot[0],sizeof(videogl_cache_slot));
    memset(&videogl_cache_tile_slot[0],-1,sizeof(videogl_cache_tile_slot));
    videogl_cache_slot_last=0;
#endif
    videogl_changed_colors=0;
    videogl_changed_colors_back=0;
    ZeroMemory(&videogl_changed_color[0],sizeof(videogl_changed_color));
    ZeroMemory(&videogl_actual_color[0],sizeof(videogl_actual_color));

#if defined(DISPLAY_FRAMERATE) && !defined(DEBUG_FRAMESKIP)
#ifndef DREAMCAST
    videogl_fps_buffer=malloc(64*8*2);
    glGenTextures(1,(GLuint *)&videogl_fps_texture);
#else
    videogl_fps_buffer=(void *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2));
#endif
#endif

#ifndef DREAMCAST
    videogl_black_buffer=malloc(8*8*2);
    glGenTextures(1,(GLuint *)&videogl_black_texture);
    videogl_background_buffer=malloc(VIDEO_GL_RASTER*8*2);
    glGenTextures(1,(GLuint *)&videogl_background_texture);
    videogl_addfix_buffer=malloc(VIDEO_GL_RASTER*8*2);
    glGenTextures(1,(GLuint *)&videogl_addfix_texture);
    videogl_sub_buffer=malloc(8*8*2);
    glGenTextures(1,(GLuint *)&videogl_sub_texture);
    videogl_info_buffer=malloc(256*8*2);
    glGenTextures(1,(GLuint *)&videogl_info_texture);
#else
    videogl_black_buffer=(void *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2)+(64*8*2));
    videogl_addfix_buffer=(void *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2)+(64*8*2)+(8*8*2));
    videogl_sub_buffer=(void *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2)+(64*8*2)+(8*8*2)+(VIDEO_GL_RASTER*8*2));
    videogl_background_buffer=(void *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2)+(64*8*2)+(8*8*2)+(VIDEO_GL_RASTER*8*2)+(8*8*2));
    videogl_info_buffer=(void *)(dcpvrmem+(SNES4ALL_SCREEN_PITCH*256)+(VIDEO_GL_MAX_TEX*8*8*2)+(64*8*2)+(8*8*2)+(VIDEO_GL_RASTER*8*2)+(8*8*2)+(VIDEO_GL_RASTER*8*2));
#endif
    for(int i=0;i<8*8;i++)
	    ((unsigned short *)(videogl_black_buffer))[i]= ((unsigned short *)(videogl_sub_buffer))[i]= 0x8000;
    for(int i=0;i<VIDEO_GL_RASTER*8;i++)
	    ((unsigned short *)(videogl_addfix_buffer))[i]= ((unsigned short *)(videogl_background_buffer))[i]= 0x8000;
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
    ZeroMemory(&videogl_selective[0],sizeof(videogl_selective));
#endif
}

SDL_bool videogl_init(void) {
    extern SDL_Surface *screen;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);
    videogl_screen = SDL_SetVideoMode(VIDEO_GL_WIDTH, VIDEO_GL_HEIGHT, 16,
		    SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_OPENGL);

#ifdef DREAMCAST
	pvr_init_params_t mis_ps = { { PVR_BINSIZE_8, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 }, (VIDEO_GL_MAX_TILES+24) * 0x50,0,0 };
	pvr_mem_reset();
	pvr_init(&mis_ps);
	glKosInit();
	glKosBeginFrame();
#endif
	
    if ( videogl_screen == NULL)
	return SDL_FALSE;

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(1);
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, VIDEO_GL_WIDTH, VIDEO_GL_HEIGHT);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0.0, SNES4ALL_SCREEN_WIDTH, SNES4ALL_SCREEN_HEIGHT, 0.0, -50.0, 50.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    videogl_init_cache();

    screen = SDL_CreateRGBSurfaceFrom(videogl_screen_buffer, SNES4ALL_SCREEN_WIDTH, SNES4ALL_SCREEN_HEIGHT, SNES4ALL_SCREEN_DEPTH, SNES4ALL_SCREEN_PITCH , 0xF800, 0x7E0, 0x1F, 0);

    glGenTextures(1,(GLuint *)&videogl_screen_texture);
    glBindTexture(GL_TEXTURE_2D,videogl_screen_texture);
    loadTextureParams(SNES4ALL_FILTER_NONE);
#ifdef DREAMCAST
    glKosFinishFrame();
#endif
    return SDL_TRUE;
}

void videogl_change_render(int hw_render) {
#ifndef USE_VIDEOGL_DIRECT_SLOT
    ZeroMemory(&videogl_cache_slot[0],sizeof(videogl_cache_slot));
    memset(&videogl_cache_tile_slot[0],-1,sizeof(videogl_cache_tile_slot));
    videogl_cache_slot_last=0;
#endif
    videogl_changed_colors=0;
    videogl_changed_colors_back=0;
    ZeroMemory(&videogl_changed_color[0],sizeof(videogl_changed_color));
    ZeroMemory(&videogl_actual_color[0],sizeof(videogl_actual_color));

    videogl_cache_reseted=0;
    memcpy((void *)&videogl_actual_color,(void *)&IPPU.ScreenColors[0],256*sizeof(unsigned short));
    ZeroMemory(&videogl_cached[0],sizeof(videogl_cached));
//    ZeroMemory(videogl_screen_buffer,SNES4ALL_SCREEN_PITCH*256);
    ZeroMemory(GFX.Screen,SNES4ALL_SCREEN_PITCH*240);
    videogl_cache_to_reseted=0;
    videogl_brightness=~videogl_first_brightness;
    videogl_tiles=0;
    videogl_clip=NULL;
    videogl_clip_enabled=0;
    videogl_used_screen=0;
    videogl_used_addfix=0;
    videogl_addfix_x0=0;
    videogl_addfix_y0=256;
    videogl_addfix_x1=256;
    videogl_addfix_y1=0;
#if 0
    videogl_used_sub=0;
    videogl_sub_color=0x8000;
    videogl_sub_x0=0;
    videogl_sub_y0=256;
    videogl_sub_x1=256;
    videogl_sub_y1=0;
#endif

    IPPU.OBJChanged = TRUE;
#ifdef DIRECT_COLOR_MAP
    IPPU.DirectColourMapsNeedRebuild = TRUE;
#endif

#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
    ZeroMemory(&videogl_selective[0],sizeof(videogl_selective));
#endif
#if defined(DISPLAY_FRAMERATE) && !defined(DEBUG_FRAMESKIP)
    videogl_fps_enabled=0;
#endif
    videogl_info_enabled=0;
    videogl_back_no_tiles=0;
    videogl_8bit_tiles=0;
    videogl_screen_endY=0;
    videogl_hw_render=hw_render;
}
