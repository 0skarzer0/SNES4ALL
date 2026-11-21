#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<SDL.h>
#ifdef USE_MENU_MUSIC
#include<SDL_mixer.h>
#endif

#ifdef DREAMCAST
#include <SDL_dreamcast.h>
#endif

#include "snes9x.h"
#include "snes4all.h"
#include "menu.h"

#include "msg.h"
#include "fade.h"

#include "sdlinterface.h"
#include "videogl.h"

#define TRANS_COLOR 0xFEEEEE;

SDL_Surface *text_screen=NULL, *text_image=NULL, *text_image_inv=NULL, *text_background=NULL; //, *text_window_background=NULL;

static Uint32 menu_inv_color=0, menu_win0_color=0, menu_win1_color=0;
static Uint32 menu_barra0_color=0, menu_barra1_color=0;
static Uint32 menu_win0_color_base=0, menu_win1_color_base=0;

void write_text_pos(int x, int y, char * str);
void write_num(int x, int y, int v);
int menu_msg_pos=330;
Uint32 menu_msg_time;

int menu_moving=1;

static int DelayAndEvents(Uint32 msec)
{
	static int exitnow=0;
	int i;
	for(i=msec/10;(i)&&(!exitnow);i--)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e)) exitnow=1;
		SDL_Delay(10);
	}
	return exitnow;
}

void menu_raise(void)
{
	int i;
	for(i=64;i>=0;i-=4)
	{
#ifdef USE_MENU_MUSIC
		Mix_VolumeMusic(96-(i<<1));
#endif
#ifdef DREAMCAST
		vid_waitvbl();
#else
		SDL_Delay(50);
#endif
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
	}
}

void menu_unraise(void)
{
	int i;
	for(i=0;i<=64;i+=4)
	{
#ifdef USE_MENU_MUSIC
		Mix_VolumeMusic(96-(i<<1));
#endif
#ifdef DREAMCAST
		vid_waitvbl();
#else
		SDL_Delay(50);
#endif
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
	}
}


static __inline__ void update_window_color(void)
{
	static int cambio=0;
	static int spin=0;

	Uint8 r,g,b;
	int cambio2=cambio>>3;
	SDL_GetRGB(menu_win0_color_base,text_screen->format,&r,&g,&b);
	if (((int)r)-cambio2>0) r-=cambio2;
	else r=0;
	if (((int)g)-cambio2>0) g-=cambio2;
	else g=0;
	if (((int)b)-cambio2>0) b-=cambio2;
	else b=0;
	menu_win0_color=SDL_MapRGB(text_screen->format,r,g,b);
	SDL_GetRGB(menu_win1_color_base,text_screen->format,&r,&g,&b);
	if (((int)r)-cambio>0) r-=cambio;
	else r=0;
	if (((int)g)-cambio>0) g-=cambio;
	else g=0;
	if (((int)b)-cambio>0) b-=cambio;
	else b=0;
	menu_win1_color=SDL_MapRGB(text_screen->format,r,g,b);
	if (spin)
	{
		if (cambio<=0) spin=0;
		else cambio-=4;
	}
	else
	{
		if (cambio>=32) spin=1;
		else cambio+=4;
	}
}

void text_draw_menu_msg()
{
/*
	Uint32 now=SDL_GetTicks();
	menu_msg_pos=330-((now-menu_msg_time)&0x7);
*/
	if (menu_msg_pos<MAX_SCROLL_MSG)
	{
//		menu_msg_time=now;
		menu_msg_pos=330;
	}
	else
		menu_msg_pos-=2;
	write_text_pos_inv(menu_msg_pos,240-8,menu_msg);
}

static __inline__ void text_draw_real_background()
{
	static int pos_x=12345678;
	static int pos_y=12345678;
	SDL_Rect r;
	int i,j;
	int w=text_screen->w+text_background->w-1;
	int h=text_screen->h+text_background->h-1;

	if (menu_moving)
	{
		if (pos_x>=0) pos_x=-screen->w;
		else pos_x++;
		if (pos_y>=0) pos_y=-screen->h;
		else pos_y++;
	}

	for(i=pos_x;i<w;i+=text_background->w)
		for(j=pos_y;j<h;j+=text_background->h)
		{
			r.x=i;
			r.y=j;
			r.w=text_background->w;
			r.h=text_background->h;
			SDL_BlitSurface(text_background,NULL,text_screen,&r);
		}
	if (menu_moving)
		text_draw_menu_msg();
}

void text_draw_background()
{
	text_draw_real_background();
	if (menu_moving)
		update_window_color();
}

void text_flip(void)
{
	static Uint32 t=0;
	Uint32 now=SDL_GetTicks();
#ifdef DREAMCAST
	if ((now-t)<10)
#else
	if ((now-t)<30)
#endif
		SDL_Delay(now-t);
	t=now;
#ifdef USE_GL
//	texture_buffer=text_screen->pixels;
SDL_BlitSurface(text_screen,NULL,screen,NULL);
	videogl_flip(SDL_FALSE);
#else
	SDL_BlitSurface(text_screen,NULL,screen,NULL);
	SDL_Flip(screen);
#endif
}


static void obten_colores(void)
{
	FILE *f=fopen(DATA_PREFIX "colors.txt", "rt");
	if (f)
	{
		Uint32 r,g,b;
		fscanf(f,"menu_inv_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_inv_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_win0_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_win0_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_win1_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_win1_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_barra0_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_barra0_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_barra1_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_barra1_color=SDL_MapRGB(text_screen->format,r,g,b);
		fclose(f);
	}
	else
	{
		menu_inv_color=SDL_MapRGB(text_screen->format, 0x20, 0x20, 0x40);
		menu_win0_color=SDL_MapRGB(text_screen->format, 0x10, 0x08, 0x08);
		menu_win1_color=SDL_MapRGB(text_screen->format, 0x20, 0x10, 0x10);
		menu_barra0_color=SDL_MapRGB(text_screen->format, 0x30, 0x20, 0x20);
		menu_barra1_color=SDL_MapRGB(text_screen->format, 0x50, 0x40, 0x40);
	}
	menu_win0_color_base=menu_win0_color;
	menu_win1_color_base=menu_win1_color;
}

void init_text(int splash)
{

#ifdef DREAMCAST
	SDL_DC_EmulateKeyboard(SDL_TRUE);
	SDL_DC_EmulateMouse(SDL_FALSE);
#endif
	if (!text_screen)
	{
#ifdef USE_GL
//		text_screen=SDL_CreateRGBSurface(0,256,256,16,0xf800,0x07e0,0x001f,0);
		void *buf_=calloc((256*256*2)+64,1);
		void *buf=((void *)(((((unsigned)buf_)/32)+1)*32));
		text_screen=SDL_CreateRGBSurfaceFrom(buf,256,256,16,SNES4ALL_SCREEN_PITCH,0xf800,0x07e0,0x001f,0);
#else
		text_screen=SDL_DisplayFormat(screen);
#endif
		if (text_screen==NULL)
		{
			puts("No se pudo crear el SDL_Surface text_screen");
			exit(-1);
		}
	}
	if (!text_image)
	{
		SDL_Surface *tmp=SDL_LoadBMP(MENU_FILE_TEXT);
		if (tmp==NULL)
		{
			puts("No se pudo cargar " MENU_FILE_TEXT);
			exit(-1);
		}
		text_image=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		SDL_SetColorKey(text_image,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_image -> format, 0, 0, 0));
	}
	if (!text_image_inv)
	{
		SDL_Surface *tmp=SDL_LoadBMP(MENU_FILE_TEXT_INV);
		if (tmp==NULL)
		{
			puts("No se pudo crear el SDL_Surface text_screen o cargar " MENU_FILE_TEXT_INV);
			exit(-1);
		}
		text_image_inv=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		SDL_SetColorKey(text_image_inv,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_image -> format, 0, 0, 0));
	}
	if (!text_background)
	{
		SDL_Surface *tmp=SDL_LoadBMP(MENU_FILE_BACKGROUND);
		if (tmp==NULL)
		{
			puts("No se pudo cargar " MENU_FILE_BACKGROUND);
			exit(-3);
		}
		text_background=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
	}
/*
	if (!text_window_background)
	{
		SDL_Surface *tmp=SDL_LoadBMP(MENU_FILE_WINDOW);
		if (tmp==NULL)
		{
			puts("No se pudo cargar " MENU_FILE_WINDOW);
			exit(-4);
		}
		text_window_background=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
	}
*/
#if !defined(DEBUG_FRAMESKIP) && !defined(AUTO_EVENTS) && !defined(AUTOLOAD) && !defined(DEBUG_TILECACHE) && !defined(PROFILER_SNES4ALL)
	if (splash)
	{
		SDL_Event ev;
		int toexit=0;
		SDL_Surface *sur;
		SDL_Rect r;
		int i,j;
		SDL_Surface *tmp=SDL_LoadBMP(MENU_FILE_SPLASH);
		if (tmp==NULL)
		{
			puts("No se pudo cargar " MENU_FILE_SPLASH);
			exit(-6);
		}
		sur = SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		sound_play_menu_music();
		r.x=(text_screen->w - sur->w)/2;
		r.y=(text_screen->h - sur->h)/2;
		r.h=sur->w;
		r.w=sur->h;
		SDL_FillRect(text_screen,NULL,0xFFFFFFFF);
		while(SDL_PollEvent(&ev)) SDL_Delay(50);
		for (i=128;(i>-8)&&(!toexit);i-=8)
		{
#ifdef DREAMCAST
			vid_waitvbl();
			while(SDL_PollEvent(&ev)) toexit=1;
#else
			toexit=DelayAndEvents(50);
#endif
			SDL_BlitSurface(sur,NULL,text_screen,&r);
			fade16(text_screen,i);
			text_flip();
		}
		if (!toexit) {
#ifdef DREAMCAST
		toexit=DelayAndEvents(4100);
#else
		toexit=DelayAndEvents(2300);
#endif
		}
		for(i=0;(i<128)&&(!toexit);i+=16)
		{
#ifdef DREAMCAST
			vid_waitvbl();
			while(SDL_PollEvent(&ev)) toexit=1;
#else
			toexit=DelayAndEvents(50);
#endif
			SDL_BlitSurface(sur,NULL,text_screen,&r);
			fade16(text_screen,i);
			text_flip();
		}
		SDL_FreeSurface(sur);
		for(i=128;(i>-8)&&(!toexit);i-=8)
		{
#ifdef DREAMCAST
			vid_waitvbl();
			while(SDL_PollEvent(&ev)) toexit=1;
#else
			toexit=DelayAndEvents(50);
#endif
			text_draw_background();
			fade16(text_screen,i);
			text_flip();
		}

	}
	else
#endif
	{
		text_draw_background();
		text_flip();
	}
	if (splash)
	{
		obten_colores();
		menu_msg_time=SDL_GetTicks();
	}
}


void quit_text(void)
{
/*
	SDL_FreeSurface(text_image);
	SDL_FreeSurface(text_background);
//	SDL_FreeSurface(text_window_background);
	SDL_FreeSurface(text_screen);
*/
}

static void write_text_pos_common(int x, int y, char * str, SDL_Surface *txt_scr)
{
  int i, c;
  SDL_Rect src, dest;

#ifndef NORES256
  x-=4;
#endif
  
  for (i = 0; i < strlen(str); i++)
    {
      c = -1;
      
      if (str[i] >= '0' && str[i] <= '9')
	c = str[i] - '0';
      else if (str[i] >= 'A' && str[i] <= 'Z')
	c = str[i] - 'A' + 10;
      else if (str[i] >= 'a' && str[i] <= 'z')
	c = str[i] - 'a' + 36;
      else if (str[i] == '#')
	c = 62;
      else if (str[i] == '=')
	c = 63;
      else if (str[i] == '.')
	c = 64;
      else if (str[i] == '_')
	c = 68;
      else if (str[i] == '-')
	c = 68;
      else if (str[i] == '/')
	c = 65;
      else if (str[i] == ',')
	c = 66;
      else if (str[i] == ':')
	c = 67;
      
      if (c >= 0)
	{
	  src.x = c * 8;
	  src.y = 0;
	  src.w = 8;
	  src.h = 8;
	  
	  dest.x = x + (i * 8);
	  dest.y = y;
	  dest.w = 8;
	  dest.h = 8;
	  
	  SDL_BlitSurface(txt_scr, &src,
			  text_screen, &dest);
	}
    }
}

void write_text_pos(int x, int y, char * str)
{
	write_text_pos_common(x,y,str,text_image);
}

void write_text_pos_inv(int x, int y, char * str)
{
	write_text_pos_common(x,y,str,text_image_inv);
}

static void write_text_common(int x, int y, int offs, char * str, SDL_Surface *txt_scr)
{
  int i, c;
  SDL_Rect src, dest;
  
#ifndef NORES256
  x-=4;
#endif
  for (i = 0; i < strlen(str); i++)
    {
      c = -1;
      
      if (str[i] >= '0' && str[i] <= '9')
	c = str[i] - '0';
      else if (str[i] >= 'A' && str[i] <= 'Z')
	c = str[i] - 'A' + 10;
      else if (str[i] >= 'a' && str[i] <= 'z')
	c = str[i] - 'a' + 36;
      else if (str[i] == '#')
	c = 62;
      else if (str[i] == '=')
	c = 63;
      else if (str[i] == '.')
	c = 64;
      else if (str[i] == '_')
	c = 68;
      else if (str[i] == '-')
	c = 68;
      else if (str[i] == '/')
	c = 65;
      else if (str[i] == ',')
	c = 66;
      else if (str[i] == ':')
	c = 67;
      else if (str[i] == '(')
	c = 69;
      else if (str[i] == ')')
	c = 70;
      
      if (c >= 0)
	{
	  src.x = c * 8;
	  src.y = 0;
	  src.w = 8;
	  src.h = 8;
	  
	  dest.x = offs +((x + i) * 8);
	  dest.y = offs + (y * 8); //10;
	  dest.w = 8;
	  dest.h = 8;
	  
	  SDL_BlitSurface(txt_scr, &src,
			  text_screen, &dest);
	}
    }
}

void write_text(int x, int y, char * str)
{
	write_text_common(x,y,0,str,text_image);
}

/* Write text, inverted: */

void write_text_inv(int x, int y, char *str)
{
	write_text_common(x, y, 0, str,text_image_inv);
}

void write_text_offs(int offs,int x, int y, char *str)
{
	write_text_common(x, y, offs, str,text_image_inv);
}

static void draw_box(SDL_Rect *src) {
  SDL_Rect dest;
  dest.x = src->x;
  dest.y = src->y;
  dest.w = src->w;
  dest.h = 1;
  SDL_FillRect(text_screen, &dest, menu_inv_color);
  dest.x = src->x;
  dest.y = src->y+src->h-1;
  dest.w = src->w;
  dest.h = 1;
  SDL_FillRect(text_screen, &dest, menu_inv_color);
  dest.x = src->x;
  dest.y = src->y;
  dest.w = 1;
  dest.h = src->h;
  SDL_FillRect(text_screen, &dest, menu_inv_color);
  dest.x = src->x+src->w-1;
  dest.y = src->y;
  dest.w = 1;
  dest.h = src->h;
  SDL_FillRect(text_screen, &dest, menu_inv_color);
}

void write_text_sel(int x, int y, char * str)
{
  SDL_Rect dest;
  
#ifndef NORES256
  x-=4;
#endif
  dest.x = (x * 8) -2 ;
  dest.y = (y * 8) /*10*/ - 2;
  dest.w = (strlen(str) * 8) + 4;
  dest.h = 12;

#if 0
  SDL_FillRect(text_screen, &dest, menu_inv_color);
#else
  draw_box(&dest);
#endif
 
#ifndef NORES256
  x+=4;
#endif
  write_text_common(x, y, 0, str,text_image_inv);
}

void write_text_sel3(int x, int y, char * str)
{
  SDL_Rect dest;
  
#ifndef NORES256
  x-=4;
#endif
  dest.x = (x * 8) -2 ;
  dest.y = (y * 8) /*10*/ - 2;
  dest.w = (strlen(str) * 8) + 4;
  dest.h = 12;

#if 0
  SDL_FillRect(text_screen, &dest, menu_inv_color);
#else
  draw_box(&dest);
#endif
 
#ifndef NORES256
  x+=4;
#endif
#if 0
  write_text_common(x, y, 0, str,text_image_inv);
#else
  write_text_common(x, y, 0,  str,text_image);
#endif
}


void write_text_sel2(int x, int y, char * str)
{
  SDL_Rect dest;
  
#ifndef NORES256
  x-=4;
#endif
  dest.x = (x * 8) -2 ;
  dest.y = (y * 8) /*10*/ - 2;
  dest.w = (strlen(str) * 8);
  dest.h = 12;

#if 0
  SDL_FillRect(text_screen, &dest, menu_inv_color);
#else
  draw_box(&dest);
#endif
 
#ifndef NORES256
  x+=4;
#endif
#if 0
  write_text_common(x, y, str,text_image_inv);
#else
  write_text_common(x, y, 0, str,text_image);
#endif
}


/* Write text, horizontally centered... */

void write_centered_text(int y, char * str)
{
  write_text(20 - (strlen(str) / 2), y/2, str);
}

void write_centered_text_inv(int y, char * str)
{
  write_text_inv(20 - (strlen(str) / 2), y/2, str);
}


/* Write numbers on the option screen: */

void write_num(int x, int y, int v)
{
  char str[24];
  
  sprintf(str, "%d", v);
  write_text(x, y, str);
}

void write_num_inv(int x, int y, int v)
{
  char str[24];
  
  sprintf(str, "%d", v);
  write_text_inv(x, y, str);
}

void text_draw_barra(int x, int y, int w, int h, int per, int max)
{
        SDL_Rect dest;
        dest.x=x-1;
        dest.y=y-1;
        dest.w=w+2;
        dest.h=h+2;
        SDL_FillRect(text_screen, &dest, menu_barra0_color);
        dest.x=x;
        dest.y=y;
        dest.h=h;
        dest.w=(w*per)/max;
        SDL_FillRect(text_screen, &dest, menu_barra1_color);
}

void text_draw_window(int x, int y, int w, int h, char *title)
{
#ifndef NORES256
  x-=4*8;
#endif
	if (x>=SNES4ALL_SCREEN_WIDTH || y>=SNES4ALL_SCREEN_HEIGHT)
		return;
	if (w+x>SNES4ALL_SCREEN_WIDTH)
		w=SNES4ALL_SCREEN_WIDTH-x;
	if (h+y>SNES4ALL_SCREEN_HEIGHT)
		h=SNES4ALL_SCREEN_HEIGHT-h;

	int i,j;
	int r8x = x / 8;
	int r8y = y / 8;
	int rx = r8x * 8;
	int ry = r8y * 8;
#ifdef NORES256
	int r32w =  w / 32;
	int rw = r32w * 32;
	int r8w = rw / 8;
#else
	int r32w =  w / 24;
	int rw = r32w * 24;
	int r8w = (rw / 8)+4;
	rw+=4;
#endif
	int r24h =  h / 24;
	int rh = r24h * 24;


	SDL_Rect dest;
/*
	dest.x = rx + 6;
	dest.y = ry - 4;
	dest.w = rw + 6;
	dest.h = rh + 18;
	SDL_FillRect(text_screen, &dest, menu_win0_color);
*/

	dest.x = rx - 2;
	dest.y = ry - 10; //12;
	dest.w = rw + 4;
	dest.h = 12; //rh + 14; //16;
	SDL_FillRect(text_screen, &dest, menu_win1_color);


	dest.x = rx - 2;
	dest.y = ry - 10;
	dest.w = 2;
	dest.h = rh + 14 + 8;
	SDL_FillRect(text_screen, &dest, menu_win1_color);

	dest.x = rx + rw;
	dest.y = ry - 10;
	dest.w = 2;
	dest.h = rh + 14 + 8;
	SDL_FillRect(text_screen, &dest, menu_win1_color);

	dest.x = rx - 2; 
	dest.y = ry + rh + 2 + 8;
	dest.w = rw + 4;
	dest.h = 2;
	SDL_FillRect(text_screen, &dest, menu_win1_color);


/*
	for(i=0;i<r32w;i++)
		for(j=0;j<r24h;j++)
		{
			dest.x=rx+i*32;
			dest.y=ry+j*24;
			dest.w=32;
			dest.h=24;
			SDL_BlitSurface(text_window_background,NULL,text_screen,&dest);

		}
*/
#ifndef DREAMCAST
	SDL_LockSurface(text_screen);
#endif
	ry+=2;
	rh+=8;
	if ((ry+rh)>SNES4ALL_SCREEN_HEIGHT)
		rh=SNES4ALL_SCREEN_HEIGHT-ry;
	if (ry<SNES4ALL_SCREEN_HEIGHT)
	{
		register Uint16 *buf=(Uint16 *)text_screen->pixels;
		buf=(Uint16 *)&buf[rx+(ry*SNES4ALL_SCREEN_WIDTH)];
		register unsigned dx=SNES4ALL_SCREEN_WIDTH-rw;
		for(j=0;j<rh;j++,buf+=dx)
		{
			for(i=0;i<rw;i+=2,buf++)
				*buf++=TRANS_COLOR;
			if (j&1)
				buf++;
			else
				buf--;
		}
	}
#ifndef DREAMCAST
	SDL_UnlockSurface(text_screen);
#endif

#ifdef USE_GL
	write_text(r8x + ((r8w-strlen(title)) / 2), r8y - 1, title);
	write_text_offs(1,r8x + ((r8w-strlen(title)) / 2), r8y - 1, title);
#else
#ifdef NORES256
	write_text_inv(r8x, r8y - 1, "o -");
#else
	write_text_inv(r8x+4, r8y - 1, "o -");
#endif
	write_text_inv(r8x + r8w - 6, r8y - 1, "-- =ox");
	write_text_inv(r8x + ((r8w-strlen(title)) / 2), r8y - 1, title);
#endif

}


void text_draw_loading(int per, int max)
{
	text_draw_background();
	text_draw_window(80,64,172,32,"Loading");
	text_draw_barra(84, 78, 150, 6, per, max);
	write_text(14,9,"Please wait");
	text_flip();
}

void text_draw_saving(int per, int max)
{
	text_draw_background();
	text_draw_window(80,64,172,32,"Saving");
	text_draw_barra(84, 78, 150, 6, per, max);
	write_text(14,9,"Please wait");
	text_flip();
}

