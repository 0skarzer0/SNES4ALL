/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifdef DREAMCAST
#include <kos.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>

#include <SDL.h>
#ifdef DREAMCAST
#include <SDL_dreamcast.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "ppu.h"
#include "snapshot.h"
#include "gfx.h"
#include "display.h"
#include "apu.h"
#include "keydef.h"

#include "snes4all.h"

#define COUNT(a) (sizeof(a) / sizeof(a[0]))

SDL_Surface *screen, *gfxscreen;
uint16 *RGBconvert;
extern uint32 cl, cs;

#if 0
int S9xMinCommandLineArgs ()
{
    return (2);
}

void S9xGraphicsMode ()
{
}

void S9xTextMode ()
{
}
#endif

#ifdef DREAMCAST
#include<dirent.h>
extern "C" { void fs_sdcard_shutdown(void); void fs_sdcard_init(void); int fs_sdcard_unmount(void); int fs_sdcard_mount(void); void sci_init(void); }

int sdcard_exists=0;
void reinit_sdcard(void)
{
	static uint32 last=(uint32)-5000;
	uint32 now=(((unsigned long long)timer_us_gettime64())>>10);
	if (now-last>5000) {
		char *dir="/sd/snes4all";
		DIR *d=NULL;
		fs_sdcard_shutdown();
		timer_spin_sleep(111);
		fs_sdcard_init();
		timer_spin_sleep(111);
		fs_mkdir(dir);
		d=opendir(dir);
		sdcard_exists=(d!=NULL);
		if (d)
			closedir(d);
		last=now;
	}
}
#else
#define reinit_sdcard()
#endif

void S9xInitDisplay (int /*argc*/, char ** /*argv*/)
{
	Uint32 flags=SDL_HWSURFACE|SDL_DOUBLEBUF;
#if defined(DREAMCAST) && defined(NORES256)
	flags|=SDL_FULLSCREEN;
#endif
#if defined(DREAMCAST) && ( defined(DEBUG_FRAMESKIP) || defined(AUTO_EVENTS) || defined(AUTOLOAD) || defined(DEBUG_TILECACHE) || defined(PROFILER_SNES4ALL) )
	SDL_DC_ShowAskHz(SDL_FALSE);
#endif
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK) < 0 ) 
	{
		printf("Could not initialize SDL(%s)\n", SDL_GetError());
		S9xExit();
	}
#if defined(DREAMCAST) && defined(DEBUG_FRAMESKIP)
	SDL_DC_ShowAskHz(SDL_FALSE);
#endif
#if defined(DREAMCAST) && !defined(NORES256)
	SDL_DC_SetVideoDriver(SDL_DC_TEXTURED_VIDEO);
	screen = SDL_SetVideoMode(SNES4ALL_SCREEN_WIDTH, SNES4ALL_SCREEN_WIDTH, SNES4ALL_SCREEN_DEPTH, flags);
	SDL_DC_SetWindow(SNES4ALL_SCREEN_WIDTH, SNES4ALL_SCREEN_HEIGHT);
#else
	screen = SDL_SetVideoMode(SNES4ALL_SCREEN_WIDTH, SNES4ALL_SCREEN_HEIGHT, SNES4ALL_SCREEN_DEPTH, flags);
#endif
	if (screen == NULL)
	{
		printf("Couldn't set video mode: %s\n", SDL_GetError());
		S9xExit();
	}
#ifndef DREAMCAST
	atexit(SDL_Quit);
#else
        SDL_JoystickEventState(SDL_ENABLE);   
        SDL_JoystickOpen(0);
        SDL_JoystickOpen(1);
        SDL_JoystickOpen(2);
        SDL_JoystickOpen(3);
	SDL_DC_EmulateKeyboard(SDL_TRUE);
#endif
	GFX.Screen = (uint8 *)screen->pixels
#ifdef NORES256
		+ 64
#endif
	;
	GFX.Pitch = SNES4ALL_SCREEN_PITCH;
	GFX.SubScreen = (uint8 *)malloc(512 * 480 * SNES4ALL_SCREEN_DEPTH_BYTES);
	GFX.ZBuffer = (uint8 *)malloc(512 * 480 * SNES4ALL_SCREEN_DEPTH_BYTES);
	GFX.SubZBuffer = (uint8 *)malloc(512 * 480 * SNES4ALL_SCREEN_DEPTH_BYTES);

	RGBconvert = (uint16 *)malloc(65536 * SNES4ALL_SCREEN_DEPTH_BYTES);
	if (!RGBconvert)
	{
//		OutOfMemory();
		S9xExit();
	}
	for (uint32 i = 0; i < 65536; i++) 
		((uint16 *)(RGBconvert))[i] = ((i >> 11) << 10) | ((((i >> 5) & 63) >> 1) << 5) | (i & 31);
	reinit_sdcard();
}

void S9xDeinitDisplay ()
{
//	SDL_FreeSurface(gfxscreen);
	SDL_FreeSurface(screen);
	free(GFX.SubScreen);
	free(GFX.ZBuffer);
	free(GFX.SubZBuffer);
}

#if 0
void S9xSetPalette ()
{
}

void S9xSetTitle (const char * /*title*/)
{
}

const char *S9xSelectFilename (const char *def, const char *dir1,
			    const char *ext1, const char *title)
{
    static char path [PATH_MAX];
    char buffer [PATH_MAX];
    
    S9xTextMode ();
    printf ("\n%s (default: %s): ", title, def);
    fflush (stdout);
    if (fgets (buffer, sizeof (buffer) - 1, stdin))
    {
	char *p = buffer;
	while (isspace (*p) || *p == '\n')
	    p++;
	if (!*p)
	{
	    strcpy (buffer, def);
	    p = buffer;
	}

	char *q = strrchr (p, '\n');
	if (q)
	    *q = 0;

	char fname [PATH_MAX];
	char drive [_MAX_DRIVE];
	char dir [_MAX_DIR];
	char ext [_MAX_EXT];

	_splitpath (p, drive, dir, fname, ext);
	_makepath (path, drive, *dir ? dir : dir1, fname, *ext ? ext : ext1);
	S9xGraphicsMode ();
	return (path);
    }
    S9xGraphicsMode ();
    return (NULL);
}

void S9xParseDisplayArg (char **argv, int &ind, int)
{
}

void S9xExtraUsage ()
{
}

bool8 S9xReadMousePosition (int /* which1 */, int &/* x */, int & /* y */,
			    uint32 & /* buttons */)
{
    return (FALSE);
}

bool8 S9xReadSuperScopePosition (int & /* x */, int & /* y */, 
				 uint32 & /* buttons */)
{
    return (FALSE);
}

bool JustifierOffscreen()
{
	return false;
}

void JustifierButtons(uint32 justifiers)
{
}
#endif

void S9xMessage (int /* type */, int /* number */, const char *message)
{
    fprintf (stderr, "%s\n", message);
}


START_EXTERN_C
uint8 snes9x_clear_change_log = 0;
END_EXTERN_C

const char *S9xGetFilenameInc (const char *e)
{
    static char filename [_MAX_PATH + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
    char *ptr;
    struct stat buf;

    if (strlen (S9xGetSnapshotDirectory()))
    {
        _splitpath (CMemory_ROMFilename, drive, dir, fname, ext);
        strcpy (filename, S9xGetSnapshotDirectory());
        strcat (filename, "/");
        strcat (filename, fname);
        ptr = filename + strlen (filename);
        strcat (filename, "00/");
        strcat (filename, e);
    }
    else
    {
        _splitpath (CMemory_ROMFilename, drive, dir, fname, ext);
        strcat (fname, "00/");
        _makepath (filename, drive, dir, fname, e);
        ptr = strstr (filename, "00/");
    }

    do
    {
        if (++*(ptr + 2) > '9')
        {
            *(ptr + 2) = '0';
            if (++*(ptr + 1) > '9')
            {
                *(ptr + 1) = '0';
                if (++*ptr > '9')
                    break;
            }
        }
    } while( stat(filename, &buf) == 0 );

    return (filename);
}

#if 0
START_EXTERN_C
char* osd_GetPackDir()
{
  static char filename[_MAX_PATH];
  memset(filename, 0, _MAX_PATH);
  
  if(strlen(S9xGetSnapshotDirectory())!=0)
    strcpy (filename, S9xGetSnapshotDirectory());
  else
  {
    char dir [_MAX_DIR + 1];
    char drive [_MAX_DRIVE + 1];
    char name [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
    _splitpath(CMemory_ROMFilename, drive, dir, name, ext);
    _makepath(filename, drive, dir, NULL, NULL);
  }
  
  if(!strncmp((char*)&CMemory_ROM [0xffc0], "SUPER POWER LEAG 4   ", 21))
  {
    if (getenv("SPL4PACK"))
      return getenv("SPL4PACK");
    else 
      strcat(filename, "/SPL4-SP7");
  }
  else if(!strncmp((char*)&CMemory_ROM [0xffc0], "MOMOTETSU HAPPY      ",21))
  {
    if (getenv("MDHPACK"))
      return getenv("MDHPACK");
    else 
      strcat(filename, "/SMHT-SP7");
  }
  else if(!strncmp((char*)&CMemory_ROM [0xffc0], "HU TENGAI MAKYO ZERO ", 21))
  {
    if (getenv("FEOEZPACK"))
      return getenv("FEOEZPACK");
    else 
      strcat(filename, "/FEOEZSP7");
  }
  else if(!strncmp((char*)&CMemory_ROM [0xffc0], "JUMP TENGAIMAKYO ZERO",21))
  {
    if (getenv("SJNSPACK"))
      return getenv("SJNSPACK");
    else 
      strcat(filename, "/SJUMPSP7");
  } else strcat(filename, "/MISC-SP7");
  return filename;
}
END_EXTERN_C
#endif



#ifdef PROFILER_SNES4ALL

static unsigned snes4all_prof_total_initial=0;
unsigned snes4all_prof_total=0;
static char *snes4all_prof_msg[SNES4ALL_PROFILER_MAX];
unsigned long long snes4all_prof_initial[SNES4ALL_PROFILER_MAX];
unsigned long long snes4all_prof_sum[SNES4ALL_PROFILER_MAX];
unsigned long long snes4all_prof_executed[SNES4ALL_PROFILER_MAX];
int snes4all_prof_started[SNES4ALL_PROFILER_MAX];


void snes4all_prof_init(void)
{
	unsigned i;
#ifndef DREAMCAST
	unsigned long long s=SDL_GetTicks();
#else
	unsigned long long s=timer_us_gettime64();
#endif
	for(i=0;i<SNES4ALL_PROFILER_MAX;i++)
	{
		snes4all_prof_initial[i]=s;
		snes4all_prof_started[i]=0;
		snes4all_prof_sum[i]=0;
		snes4all_prof_executed[i]=0;
		if (!snes4all_prof_total)
			snes4all_prof_msg[i]=NULL;
	}
	snes4all_prof_total_initial=s;
}

void snes4all_prof_add(char *msg)
{
	if (snes4all_prof_total<SNES4ALL_PROFILER_MAX)
	{
		snes4all_prof_msg[snes4all_prof_total]=msg;	
		snes4all_prof_total++;
	}
}

void snes4all_prof_show(void)
{
	int i;

	for(i=snes4all_prof_total-1;i>=0;i--)
		snes4all_prof_end(i);

	double toper=0;
#ifndef DREAMCAST
	unsigned long long to=SDL_GetTicks()-snes4all_prof_total_initial;
#else
	unsigned long long to=snes4all_prof_sum[0]+snes4all_prof_sum[1];
	for(i=0;i<snes4all_prof_total;i++)
		if (snes4all_prof_sum[i]>to)
			snes4all_prof_sum[i]=0;
#endif

	puts("\n\n\n\n");
	puts("--------------------------------------------");
	for(i=0;i<snes4all_prof_total;i++)
	{
		unsigned long long t0=snes4all_prof_sum[i];
		double percent=(double)t0;
		percent*=100.0;
		percent/=(double)to;
		toper+=percent;
#ifdef DREAMCAST
		t0/=1000;
#endif
		printf("%s: %.2f%% -> Ticks=%i -> %iK veces\n",snes4all_prof_msg[i],percent,((unsigned)t0),(unsigned)(snes4all_prof_executed[i]>>10));
	}
//	printf("TOTAL: %.2f%% -> Ticks=%i\n",toper,to);
	puts("--------------------------------------------"); fflush(stdout);
}
#endif

/*
extern "C" {
void mirame(void)
{
	IAPU.NextAPUTimerPos -= snes4all_apu_hmax;
}
}
*/
