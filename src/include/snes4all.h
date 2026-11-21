#ifndef _SNES4ALL_H_
#define _SNES4ALL_H_

#include <SDL.h>

#ifndef SNES4ALL_MAX_ROM_SIZE
#define SNES4ALL_MAX_ROM_SIZE 0x600000
#endif

extern int snes4all_frameskip;
extern int snes4all_sound_enable;
extern int snes4all_transparency;
extern int snes4all_throttle;
extern char snes4all_image_file[];
extern int snes4all_emulating;
extern unsigned snes4all_timeslice;
extern int snes4all_sound_throttle;

extern unsigned snes4all_max_vcounter;
extern short snes4all_vcounter[];

extern unsigned snes4all_apu_hmax;
extern int snes4all_displayframerate;

#ifdef NORES256
#define SNES4ALL_SCREEN_WIDTH 320
#else
#define SNES4ALL_SCREEN_WIDTH 256
#endif

#define SNES4ALL_SCREEN_HEIGHT 240
#define SNES4ALL_SCREEN_DEPTH_BYTES 2
#define SNES4ALL_SCREEN_DEPTH (8 * SNES4ALL_SCREEN_DEPTH_BYTES)
#ifndef USE_GL
#define SNES4ALL_SCREEN_PITCH (SNES4ALL_SCREEN_WIDTH * SNES4ALL_SCREEN_DEPTH_BYTES)
#else
#ifdef NORES256
#define SNES4ALL_SCREEN_PITCH (512 * SNES4ALL_SCREEN_DEPTH_BYTES)
#else
#define SNES4ALL_SCREEN_PITCH (256 * SNES4ALL_SCREEN_DEPTH_BYTES)
#endif
#endif
#define SNES4ALL_SCREEN_PITCH2 (SNES4ALL_SCREEN_PITCH >> 1)

#ifdef NORES256
#define getY(Y) (SNES4ALL_SCREEN_WIDTH * SNES4ALL_SCREEN_DEPTH_BYTES * (Y) )
#define getY2(Y) (((SNES4ALL_SCREEN_WIDTH * SNES4ALL_SCREEN_DEPTH_BYTES)>>1) * (Y) )
#else
#define getY(Y) ( (Y) << 9 )
#define getY2(Y) ( (Y) << 8 )
#endif


#ifndef PROFILER_SNES4ALL

#define snes4all_prof_start(A)
#define snes4all_prof_end(A)

#else

#define SNES4ALL_PROFILER_MAX 256

extern unsigned long long snes4all_prof_initial[SNES4ALL_PROFILER_MAX];
extern unsigned long long snes4all_prof_sum[SNES4ALL_PROFILER_MAX];
extern unsigned long long snes4all_prof_executed[SNES4ALL_PROFILER_MAX];
extern int snes4all_prof_started[SNES4ALL_PROFILER_MAX];

static __inline__ void snes4all_prof_start(unsigned a)
{
	if (snes4all_prof_started[a])
		return;
	snes4all_prof_executed[a]++;
#ifndef DREAMCAST
	snes4all_prof_initial[a]=SDL_GetTicks();
#else
	snes4all_prof_initial[a]=timer_us_gettime64();
#endif
	snes4all_prof_started[a]=1;
}


static __inline__ void snes4all_prof_end(unsigned a)
{
	if (!snes4all_prof_started[a])
		return;
#ifndef DREAMCAST
	snes4all_prof_sum[a]+=SDL_GetTicks()-snes4all_prof_initial[a];
#else
	extern unsigned snes4all_prof_total;
	int i;
	for(i=0;i<snes4all_prof_total;i++)
		snes4all_prof_initial[i]+=6;
	snes4all_prof_sum[a]+=timer_us_gettime64()-snes4all_prof_initial[a]+2;
#endif
	snes4all_prof_started[a]=0;
}

void snes4all_prof_init(void);
void snes4all_prof_add(char *msg);
void snes4all_prof_show(void);


#endif





#endif
