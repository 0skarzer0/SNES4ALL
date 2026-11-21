#include "videogl.h"

void *texture_buffer;
GLint screen_texture;
SDL_Surface *screen_gl;
double videogl_z=0.0;

void videogl_flip(SDL_bool ingame)
{
extern unsigned char marca[];
bzero(marca,0x1000);
	if (!ingame)
	{
#ifdef DREAMCAST
		glKosBeginFrame();
#endif
		glClearColor( 0.0,0.0,0.0, 255.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D,screen_texture);
		loadTextureParams(PSX4ALL_FILTER_BILINEAR);

#ifndef DREAMCAST
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
			GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture_buffer);
#else
		glKosTex2D(GL_RGB565,256,256, texture_buffer);
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
extern SDL_Surface *screen;
screen->pixels=NULL;
#ifndef DREAMCAST
		SDL_GL_SwapBuffers();
#else
		glKosFinishFrame();
		glKosBeginFrame();
#endif
		glClearColor( 0.0,0.0,0.0, 255.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		videogl_z=TILE_Z_INIT;
	}
}


void videogl_fill(unsigned starty, unsigned endy, unsigned col)
{
/*
//printf("videogl_fill %i %i %.4X\n",starty,endy,col);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4ub(((col&0xF800)>>8),((col&0x07E0)>>3),((col&0x001F)<<3),0);
		glVertex3f(0,(float)starty,videogl_z);
		glVertex3f(256,(float)starty,videogl_z);
		glVertex3f(0,(float)endy,videogl_z);
		glVertex3f(256,(float)endy,videogl_z);
	glEnd();
	videogl_z+=TILE_Z_INC;
*/
}

void videogl_fill_box(unsigned x0,unsigned y0,unsigned x1,unsigned y1,unsigned col)
{
/*
//printf("videogl_fill_box %i,%i %i,%i %.4X\n",x0,y0,x1,y1,col);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4ub(((col&0xF800)>>8),((col&0x07E0)>>3),((col&0x001F)<<3),0);
		glVertex3f((float)x0,(float)y0,videogl_z);
		glVertex3f((float)x1,(float)y0,videogl_z);
		glVertex3f((float)x0,(float)y1,videogl_z);
		glVertex3f((float)x1,(float)y1,videogl_z);
	glEnd();
	videogl_z+=TILE_Z_INC;
*/
}
