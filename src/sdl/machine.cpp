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
extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#ifdef USE_MENU_MUSIC
#include <SDL_mixer.h>
#endif
#ifdef DREAMCAST
#include <SDL_dreamcast.h>
extern int sdcard_exists;
void reinit_sdcard(void);
int save_to_vmu(char *);
int load_from_vmu(char *);
#endif
#include <signal.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include "keydef.h"

#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"
#include "sa1.h"
#include "dsp1.h"
#include "missing.h"

#include "menu.h"

#include "sdlinterface.h"

#define AUTO_FRAMESKIP_DELAY 10
#define TH_VCOUNTER_SPLIT 11
#define SNES_MAX_MID_VCOUNTER (SNES_MAX_NTSC_VCOUNTER+((SNES_MAX_PAL_VCOUNTER-SNES_MAX_NTSC_VCOUNTER)/2))
#define TH_VCOUNTER0 ((TH_VCOUNTER_SPLIT*SNES_MAX_PAL_VCOUNTER)/10)
#define TH_VCOUNTER1 TH_VCOUNTER0
#define TH_VCOUNTER2 ((TH_VCOUNTER_SPLIT*SNES_MAX_MID_VCOUNTER)/10)
#define TH_VCOUNTER3 ((TH_VCOUNTER_SPLIT*SNES_MAX_NTSC_VCOUNTER)/10)
#define TH_VCOUNTER4 ((TH_VCOUNTER_SPLIT*240)/10)


#if !defined(DREAMCAST) || !defined(USE_OPC_ASM)
unsigned snes4all_max_vcounter=TH_VCOUNTER0;
unsigned snes4all_apu_hmax=1;
short snes4all_vcounter[512];
#endif

#if defined(DEBUG_TILECACHE) || defined(PROFILER_SNES4ALL)
int snes4all_frameskip=0;
int snes4all_sound_enable=0;
static int _snes4all_sound_enable_=0;
int snes4all_transparency=0;
int snes4all_throttle=0;
int snes4all_raster=2;
int snes4all_autosave=0;
int snes4all_raster_diff=16;
#if !defined(DREAMCAST) || !defined(USE_OPC_ASM)
int snes4all_sound_throttle=0;
unsigned snes4all_timeslice=2;
#else
extern  unsigned snes4all_timeslice;
#endif

#ifdef DEBUG_TILECACHE
unsigned snes4all_debug_tilecache_tiles=0;
unsigned snes4all_debug_tilecache_fails=0;

static void print_tileche_eficience(void)
{
	double d=snes4all_debug_tilecache_fails;
	d*=100.0;
	d/=(double)snes4all_debug_tilecache_tiles;
	d=100.0-d;
	printf("%i total, %i fallados, EFICIENCIA %.2f%%\n",snes4all_debug_tilecache_tiles,snes4all_debug_tilecache_fails,d);
}
#endif

#else
int snes4all_frameskip=-1;
int snes4all_sound_enable=1;
static int _snes4all_sound_enable_=0;
int snes4all_transparency=0;
int snes4all_raster=2;
int snes4all_raster_diff=16;
int snes4all_autosave=-1;
int snes4all_throttle=2;
#if !defined(DREAMCAST) || !defined(USE_OPC_ASM)
int snes4all_sound_throttle=((1<<2)-1);
unsigned snes4all_timeslice=2+(2<<1);
#else
extern int snes4all_sound_throttle;
extern  unsigned snes4all_timeslice;
#endif
#endif

int OldSkipFrame;

extern SDL_Surface *screen;

long reportf=0;

#define MUSIC_VOLUME 64

#define MAX_FRAMESKIP 10
 
char snes4all_image_file[1024];
int snes4all_emulating=0;
char *rom_filename = (char *)&snes4all_image_file[0];
char *snapshot_filename = NULL;


static uint32 joypad_val[8]={0x80000000,0x80000000,0x80000000,0x80000000,0x80000000,0x80000000,0x80000000,0x80000000};

static int savestate_tostate=0;
#if !defined(DREAMCAST) || !defined(USE_OPC_ASM)
int savestate_state=0;
#else
extern int savestate_state;
#endif
void show_please_wait(char *title, SDL_Surface *scr);

void OutOfMemory ()
{
    fprintf (stderr, "\
Snes9X: Memory allocation failure - not enough RAM/virtual memory available.\n\
        S9xExiting...\n");
    CMemory_Deinit ();
    S9xDeinitAPU ();
    exit (1);
}


static void getChanges(void)
{
	if (snes4all_transparency)
		Settings.Transparency=TRUE;
	else
		Settings.Transparency=FALSE;
	if (snes4all_frameskip<0)
		Settings.SkipFrames=AUTO_FRAMERATE;
	else
		Settings.SkipFrames=snes4all_frameskip;
#ifdef NOSOUND
	S9xSetSoundMute(!snes4all_sound_enable);
#endif

	long i;
	switch(snes4all_raster) {
		case 0: snes4all_raster_diff=1024; break;
		case 1: snes4all_raster_diff=32; break;
		case 2: snes4all_raster_diff=16; break;
		case 3: snes4all_raster_diff=8; break;
		default: snes4all_raster_diff=1; 
	}
	switch(snes4all_throttle)
	{
		case 4:
			Settings.CyclesPercentage = (100 * SNES_MAX_PAL_VCOUNTER) / TH_VCOUNTER4;
			for(i=0;i<240;i++)
				snes4all_vcounter[i]=i+1;
			for(i=240;i<512;i++)
				snes4all_vcounter[i]=511;
			break;
		case 3:
			Settings.CyclesPercentage = (100 * SNES_MAX_PAL_VCOUNTER) / TH_VCOUNTER3;
			for(i=0;i<SNES_MAX_NTSC_VCOUNTER;i++)
				snes4all_vcounter[i]=i+1;
			for(i=SNES_MAX_NTSC_VCOUNTER;i<512;i++)
				snes4all_vcounter[i]=511;
			break;
		case 2:
			Settings.CyclesPercentage = (100 * SNES_MAX_PAL_VCOUNTER) / TH_VCOUNTER2;
			for(i=0;i<SNES_MAX_MID_VCOUNTER;i++)
				snes4all_vcounter[i]=i+1;
			for(i=SNES_MAX_MID_VCOUNTER;i<512;i++)
				snes4all_vcounter[i]=511;
			break;
		case 1:
			Settings.CyclesPercentage = (100 * SNES_MAX_PAL_VCOUNTER) / TH_VCOUNTER1;
			for(i=0;i<SNES_MAX_PAL_VCOUNTER;i++)
				snes4all_vcounter[i]=i+1;
			for(i=SNES_MAX_PAL_VCOUNTER;i<512;i++)
				snes4all_vcounter[i]=511;
			break;
		default:
			Settings.CyclesPercentage = (100 * SNES_MAX_PAL_VCOUNTER) / TH_VCOUNTER0;
			for(i=0;i<512;i++)
				snes4all_vcounter[i]=i+1;
	}
	if (snes4all_throttle)
		snes4all_sound_throttle=(1<<snes4all_throttle)-1;
	else
		snes4all_sound_throttle=0;
	snes4all_vcounter[511]=511;
	snes4all_timeslice=2+(snes4all_throttle<<1);
	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * Settings.CyclesPercentage)/100;
	snes4all_apu_hmax=Settings.H_Max*10000L;

	SDL_Event e;
	SDL_Delay(100);
	while (SDL_PollEvent(&e) > 0)
		SDL_Delay(10);
}

#define dCPU(A) ((unsigned)(A))-((unsigned)&CPU)
#define dSA1(A) ((unsigned)(A))-((unsigned)&SA1)
#define dIAPU(A) ((unsigned)(A))-((unsigned)&IAPU)

const char *S9xGetFilenameBase(const char *ex, int  based);

/*#include "cheats.h"*/
extern "C"
int main (int argc, char **argv)
{
    ZeroMemory (&Settings, sizeof (Settings));

    Settings.C4=FALSE;
    Settings.ForceC4=FALSE;
    Settings.SoundPlaybackRate = 3; //4;
#ifdef SNES4ALL_STEREO
    Settings.Stereo = TRUE;
#else
    Settings.Stereo = FALSE;
#endif
    Settings.SoundBufferSize = 1024;
    Settings.JoystickEnabled = TRUE;
    Settings.APUEnabled = Settings.NextAPUEnabled = TRUE;
    Settings.CyclesPercentage = 100;
    Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
    snes4all_apu_hmax=Settings.H_Max*10000L;
    Settings.SkipFrames = AUTO_FRAMERATE;
    Settings.ShutdownMaster = TRUE;
    Settings.FrameTimePAL = 20;
    Settings.FrameTimeNTSC = 17;
    Settings.FrameTime = Settings.FrameTimePAL;
    Settings.Mouse = FALSE;
    Settings.SuperScope = FALSE;
    Settings.MultiPlayer5 = FALSE;
//    Settings.ControllerOption = SNES_MULTIPLAYER5;
    Settings.ControllerOption = 0;
    Settings.Transparency = FALSE; //TRUE;
#ifdef NETPLAY_SUPPORT
    Settings.NetPlay = FALSE;
    Settings.ServerName [0] = 0;
#endif
    Settings.AutoSaveDelay = 2;
    Settings.ApplyCheats = TRUE;
    Settings.TurboMode = FALSE;
    Settings.TurboSkipFrames = 15;


    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

    if (!CMemory_Init () || !S9xInitAPU())
	OutOfMemory ();

    uint32 saved_flags = CPU.Flags;

#ifdef GFX_MULTI_FORMAT
    S9xSetRenderPixelFormat (RGB565);
#endif

puts("MAIN!!!!");
#ifdef USE_OPC_ASM
    if (sizeof(Registers)!=16) {
	    printf("ERROR: Registers size = %u\n",sizeof(Registers));
	    exit(0);
    }
/*
    if (sizeof(ICPU)!=36) {

	    printf("ERROR: ICPU size = %u\n",sizeof(ICPU));
	    exit(0);
    }
*/
/*
    if (sizeof(CPU)!=76) {
	    printf("ERROR: CPU size = %u\n",sizeof(CPU));
	    exit(0);
    }
*/
    if (sizeof(APURegisters)!=8) {
	    printf("ERROR: APURegisters size = %u\n",sizeof(APURegisters));
	    exit(0);
    }
/*
    if (sizeof(IAPU)!=68) {
	    printf("ERROR: IAPU size = %u\n",sizeof(IAPU));
	    exit(0);
    }
*/
/*
    if (sizeof(APU)!=224) {
	    printf("ERROR: APU size = %u\n",sizeof(APU));
	    exit(0);
    }
*/
    if (sizeof(SA1Registers)!=16) {
	    printf("ERROR: SA1Registers size = %u\n",sizeof(SA1Registers));
	    exit(0);
    }
/*
    if (sizeof(SA1)!=32848) {
	    printf("ERROR: SA1 size = %u\n",sizeof(SA1));
	    exit(0);
    }
*/
#endif
    S9xInitDisplay (0, NULL);
    if (!S9xGraphicsInit ())
	OutOfMemory ();

   (void) S9xInitSound (Settings.SoundPlaybackRate, Settings.Stereo,
			 Settings.SoundBufferSize);

#ifdef NOSOUND
    Settings.APUEnabled=FALSE;
#else
    if (!Settings.APUEnabled)
#endif
	S9xSetSoundMute (TRUE);

    S9xInitInputDevices ();

    snes4all_image_file[0]=0;
    init_text(1);
#ifndef AUTOLOAD
    run_mainMenu();
#else
    strcpy(rom_filename,AUTOLOAD);
#endif
    SDL_FillRect(screen,NULL,0);
    sound_disable_music();
    quit_text();
    snes4all_emulating=1;

    getChanges();

    if (rom_filename)
    {
	if (!CMemory_LoadROM (rom_filename))
	{
	    char dir [_MAX_DIR + 1];
	    char drive [_MAX_DRIVE + 1];
	    char name [_MAX_FNAME + 1];
	    char ext [_MAX_EXT + 1];
	    char fname [_MAX_PATH + 1];

	    _splitpath (rom_filename, drive, dir, name, ext);
	    _makepath (fname, drive, dir, name, ext);

	    strcpy (fname, S9xGetROMDirectory ());
	    strcat (fname, SLASH_STR);
	    strcat (fname, name);
	    if (ext [0])
	    {
		strcat (fname, ".");
		strcat (fname, ext);
	    }
	    _splitpath (fname, drive, dir, name, ext);
	    _makepath (fname, drive, dir, name, ext);
	    if (!CMemory_LoadROM (fname))
	    {
		printf ("Error opening: %s\n", rom_filename);
		exit (1);
	    }
	}
#ifdef DREAMCAST
	reinit_sdcard();
	if (!sdcard_exists) 
		load_from_vmu((char *)S9xGetFilenameBase(".srm",1));
#endif
	CMemory_LoadSRAM (S9xGetFilename (".srm"));
//	S9xLoadCheatFile (S9xGetFilename (".cht"));
    }
    else
    {
	S9xReset ();
	Settings.Paused |= 2;
    }
    CPU.Flags = saved_flags;

    if (snapshot_filename)
    {
	int Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	if (!S9xLoadSnapshot (snapshot_filename))
	    exit (1);
	CPU.Flags |= Flags;
    }
#ifndef _ZAURUS
    S9xGraphicsMode ();
    sprintf (String, "\"%s\" %s: %s", CMemory_ROMName, TITLE, VERSION);
    S9xSetTitle (String);
#endif

   S9xSetSoundMute (TRUE);
   if (!Settings.APUEnabled)
	S9xSetSoundMute (FALSE);

#ifdef PROFILER_SNES4ALL
   snes4all_prof_init();
   snes4all_prof_add("S9xMainLoop");			// 0
   snes4all_prof_add("S9xDoHBlankProcessing");		// 1
   snes4all_prof_add("RenderLine");			// 2
   snes4all_prof_add("S9xEndScreenRefresh");		// 3
#endif

   snes4all_prof_start(0);
#ifdef AUTOSAVESTATE
   savestate_state=-302;
#endif
   _snes4all_sound_enable_=snes4all_sound_enable;
#ifndef DEBUGGER
   S9xMainLoop ();
#else
    while (1)
    {
	if (!Settings.Paused
	    || (CPU.Flags & (DEBUG_MODE_FLAG | SINGLE_STEP_FLAG))
           )

	    S9xMainLoop ();

	if (Settings.Paused
	    || (CPU.Flags & DEBUG_MODE_FLAG)
           )
	{
	    S9xSetSoundMute (TRUE);
	}

	if (CPU.Flags & DEBUG_MODE_FLAG)
	{
	    S9xDoDebug ();
	}
	else
	if (Settings.Paused)
	    S9xProcessEvents (TRUE);

	if (!Settings.Paused
	    && !(CPU.Flags & DEBUG_MODE_FLAG)
           )
	{
	    S9xSetSoundMute (FALSE);
	}
    }
#endif
    return (0);
}

void S9xAutoSaveSRAM ()
{
//puts("S9xAutoSaveSRAM");
    CMemory_SaveSRAM (S9xGetFilename (".srm"));
#ifdef DREAMCAST
    reinit_sdcard();
    if (!sdcard_exists)
	    save_to_vmu((char *)S9xGetFilenameBase(".srm",1));
#endif
}

void S9xExit ()
{
    S9xSetSoundMute (TRUE);
    S9xDeinitDisplay ();
    S9xAutoSaveSRAM ();
//    S9xSaveCheatFile (S9xGetFilename (".cht"));
    CMemory_Deinit ();
    S9xDeinitAPU ();

    exit (0);
}

void S9xInitInputDevices ()
{
}

#ifndef DREAMCAST
const char *GetHomeDirectory ()
{
    return (getenv ("HOME"));
}
#endif

const char *S9xGetSnapshotDirectory ()
{
#ifndef DREAMCAST
    static char filename [PATH_MAX];
    const char *snapshot;
    if (!(snapshot = getenv ("SNES9X_SNAPSHOT_DIR")) &&
	!(snapshot = getenv ("SNES96_SNAPSHOT_DIR")))
#else
    const char *snapshot=fs_getwd();
    if (strncmp(fs_getwd(),"/sd",3))
#endif
    {
#ifdef WIN32
	return("/tmp");
#else
#ifdef DREAMCAST
	reinit_sdcard();
	if (sdcard_exists) return("/sd/snes4all");
	return("/ram");
#else
	const char *home = GetHomeDirectory ();
	strcpy (filename, home);
	strcat (filename, SLASH_STR);
	strcat (filename, ".snes96_snapshots");
	mkdir (filename, 0777);
	chown (filename, getuid (), getgid ());
#endif
#endif
    }
#ifndef DREAMCAST
    else
	return (snapshot);

    return (filename);
#else
    return (snapshot);
#endif
}

const char *S9xGetFilenameBase(const char *ex, int  based) {
    static char filename [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];

    _splitpath (CMemory_ROMFilename, drive, dir, fname, ext);
    if (!based) {
 	   strcpy (filename, S9xGetSnapshotDirectory ());
 	   strcat (filename, SLASH_STR);
 	   strcat (filename, fname);
    } else {
 	   strcpy (filename, fname);
    }
    strcat (filename, ex);

//printf("S9xGetFilename(%s)=%s\n",ex,filename);
    return (filename);
}

const char *S9xGetFilename (const char *ex) {
	return S9xGetFilenameBase(ex,0);
}

const char *S9xGetROMDirectory ()
{
    const char *roms;
    
    if (!(roms = getenv ("SNES9X_ROM_DIR")) &&
	!(roms = getenv ("SNES96_ROM_DIR")))
	return ("." SLASH_STR "roms");
    else
	return (roms);
}

const char *S9xBasename (const char *f)
{
    const char *p;
    if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
	return (p + 1);

    return (f);
}

#if 0
const char *S9xChooseFilename (bool8 read_only)
{
    char def [PATH_MAX + 1];
    char title [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char ext [_MAX_EXT + 1];

    _splitpath (CMemory_ROMFilename, drive, dir, def, ext);
    strcat (def, ".s96");
    sprintf (title, "%s snapshot filename",
	    read_only ? "Select load" : "Choose save");
    const char *filename;

    S9xSetSoundMute (TRUE);
    filename = S9xSelectFilename (def, S9xGetSnapshotDirectory (), "s96", title);
    S9xSetSoundMute (FALSE);
    return (filename);
}
#endif

bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file)
{
    char filename [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char ext [_MAX_EXT + 1];

    _splitpath (fname, drive, dir, filename, ext);

    if (*drive || *dir == '/' ||
	(*dir == '.' && (*(dir + 1) == '/'
        )))
    {
	strcpy (filename, fname);
	if (!*ext)
	    strcat (filename, ".s96");
    }
    else
    {
	strcpy (filename, S9xGetSnapshotDirectory ());
	strcat (filename, SLASH_STR);
	strcat (filename, fname);
	if (!*ext)
	    strcat (filename, ".s96");
    }
    
#ifdef ZLIB
    if (read_only)
    {
	if ((*file = OPEN_STREAM (filename, "rb")))
	    return (TRUE);
    }
    else
    {
	if ((*file = OPEN_STREAM (filename, "wb")))
	    return (TRUE);
    }
#else
    char command [PATH_MAX];
    
    if (read_only)
    {
	sprintf (command, "gzip -d <\"%s\"", filename);
	if (*file = popen (command, "r"))
	    return (TRUE);
    }
    else
    {
	sprintf (command, "gzip --best >\"%s\"", filename);
	if (*file = popen (command, "wb"))
	    return (TRUE);
    }
#endif
    return (FALSE);
}

void S9xCloseSnapshotFile (STREAM file)
{
#ifdef ZLIB
    CLOSE_STREAM (file);
#else
    pclose (file);
#endif
}

#if 0
bool8 S9xInitUpdate ()
{
    return (TRUE);
}
#endif

bool8 S9xDeinitUpdate (int Width, int Height,bool8 sixnocount)
{
        SDL_Flip(screen);
	return(TRUE);
}


// #define now() SDL_GetTicks()

void _makepath (char *path, const char *, const char *dir,
		const char *fname, const char *ext)
{
    if (dir && *dir)
    {
	strcpy (path, dir);
	strcat (path, "/");
    }
    else
	*path = 0;
    strcat (path, fname);
    if (ext && *ext)
    {
        strcat (path, ".");
        strcat (path, ext);
    }
}

void _splitpath (const char *path, char *drive, char *dir, char *fname,
		 char *ext)
{
    *drive = 0;

    char *slash = (char *)strrchr (path, '/');
    if (!slash)
	slash = (char *)strrchr (path, '\\');

    char *dot = (char *)strrchr (path, '.');

    if (dot && slash && dot < slash)
	dot = NULL;

    if (!slash)
    {
	strcpy (dir, "");
	strcpy (fname, path);
        if (dot)
        {
	    *(fname + (dot - path)) = 0;
	    strcpy (ext, dot + 1);
        }
	else
	    strcpy (ext, "");
    }
    else
    {
	strcpy (dir, path);
	*(dir + (slash - path)) = 0;
	strcpy (fname, slash + 1);
        if (dot)
	{
	    *(fname + (dot - slash) - 1) = 0;
    	    strcpy (ext, dot + 1);
	}
	else
	    strcpy (ext, "");
    }
}

/*
#ifndef _ZAURUS
void S9xToggleSoundChannel (int c)
{
    if (c == 8)
	so.sound_switch = 255;
    else
	so.sound_switch ^= 1 << c;
    S9xSetSoundControl (so.sound_switch);
}
#endif
*/

#ifdef DEBUG_FRAMESKIP
static unsigned snes4all_fskip_nframes=0;
static unsigned snes4all_fskip_skipped=0;
static double snes4all_fskip_framerate=0.0;
static unsigned snes4all_fskip_start_time=0;

static void print_frameskip(void)
{
	double d=(((double)snes4all_fskip_skipped)*100.0)/((double)snes4all_fskip_nframes);
	printf("%i frames, skipped %i (%.2f%%)\n",snes4all_fskip_nframes,snes4all_fskip_skipped,d);
	printf("Framerate %.2f/%.2f\n", ((100.0-d)*snes4all_fskip_framerate)/100.0,snes4all_fskip_framerate);
}

static void debug_reset_frameskip(void){
	snes4all_fskip_start_time=0;
	snes4all_fskip_nframes=0;
	snes4all_fskip_skipped=0;
}

static void debug_frameskip(bool isframe)
{
	static Uint32 start_numframes=0;
	static Uint32 nveces=0;

	if (!isframe)
		snes4all_fskip_skipped++;
	snes4all_fskip_nframes++;
	if (!snes4all_fskip_start_time)
	{
		snes4all_fskip_start_time=SDL_GetTicks();
		start_numframes=snes4all_fskip_nframes;
		snes4all_fskip_framerate=0.0;
	}
	else
	{
		Uint32 now=SDL_GetTicks();
		if (now-snes4all_fskip_start_time>=1000)
		{
			if (snes4all_fskip_framerate!=0.0)
				snes4all_fskip_framerate=((snes4all_fskip_framerate*((double)(nveces-1)))+((double)(snes4all_fskip_nframes-start_numframes)))/((double)nveces);
			else {
				snes4all_fskip_framerate=(double)(snes4all_fskip_nframes-start_numframes);
				nveces=1;
			}
			snes4all_fskip_start_time=now;
			start_numframes=snes4all_fskip_nframes;
			nveces++;
		}
	}
#ifdef AUTO_DEBUG_FRAMESKIP
	if (snes4all_fskip_nframes>=AUTO_DEBUG_FRAMESKIP)
	{
		print_frameskip();
		exit(0);
	}
#endif

}

#else
#define debug_reset_frameskip()
#endif

#ifdef USE_RENDER_BACK
bool8 backRenderFrame=FALSE;
#endif

void S9xSyncSpeed ()
{
    S9xProcessEvents (FALSE);
#if defined(AUTO_DEBUG_TILECACHE) && defined(DEBUG_TILECACHE)
    {
	    static unsigned cu=0;
	    if (cu>=AUTO_DEBUG_TILECACHE)
	    {
		    print_tileche_eficience();
		    exit(0);
	    }
	    cu++;
    }
#endif
    static Uint32 next1=0;
#ifdef USE_RENDER_BACK
    static bool8 nextRenderFrame=FALSE;
    backRenderFrame=nextRenderFrame;
#endif
//    if (!Settings.TurboMode && Settings.SkipFrames == AUTO_FRAMERATE)
    if (Settings.SkipFrames == AUTO_FRAMERATE)
    {
	Uint32 now=SDL_GetTicks();

	if (!next1)
	    next1 = now;

	next1 += Settings.FrameTime;

	if ((now-AUTO_FRAMESKIP_DELAY)>next1)
	{
	    IPPU.SkippedFrames++;
	    if (IPPU.SkippedFrames > MAX_FRAMESKIP)
	    {
		IPPU.RenderThisFrame = TRUE;
		IPPU.SkippedFrames = 0;
		next1 = now + 2;
	    }
	    else
		IPPU.RenderThisFrame = FALSE;
	}
	else
	{
	    CHECK_SOUND ();
	    if ((now+AUTO_FRAMESKIP_DELAY)<next1)
	    {
		if (IPPU.RenderThisFrame)
		{
#ifdef DREAMCAST
			SDL_Delay(1);
#endif
			do
			{
			    CHECK_SOUND ();
#ifndef DREAMCAST
			    SDL_Delay(1);
#endif
//			    S9xProcessEvents (FALSE);
			    now=SDL_GetTicks();
			} while (next1 > now);
		}
		next1=now;
	    }
	    IPPU.RenderThisFrame = TRUE;
	    IPPU.SkippedFrames = 0;
	}
    }
    else
    {
	next1 = 0;
	if (++IPPU.FrameSkip >= (Settings.TurboMode ? Settings.TurboSkipFrames
						    : Settings.SkipFrames))
	{
	    IPPU.FrameSkip = 0;
	    IPPU.SkippedFrames = 0;
	    IPPU.RenderThisFrame = TRUE;
	}
	else
	{
	    IPPU.SkippedFrames++;
	    IPPU.RenderThisFrame = FALSE;
	}
    }
#ifdef USE_RENDER_BACK
    nextRenderFrame=IPPU.RenderThisFrame;
#endif
#ifdef DEBUG_FRAMESKIP
    debug_frameskip(IPPU.RenderThisFrame);
#endif
}


#ifndef AUTO_EVENTS

static void goMenu(void)
{
	int eret;
	snes4all_emulating=1;
	_snes4all_sound_enable_=0;
	while(!_snes4all_sound_enable_)
		SDL_Delay(60);
#ifdef DEBUG_TILECACHE
	print_tileche_eficience();
#endif
#ifdef DEBUG_FRAMESKIP
	print_frameskip();
#endif
	rom_filename[0]=0;
    	sound_enable_music();
	init_text(0);
    	eret=run_mainMenu();
	quit_text();
    	sound_disable_music();
	if(eret==2)
		S9xReset();
	getChanges();
	if (rom_filename[0]) {
		CMemory_LoadROM (rom_filename);
		eret=2;
	}
	if(eret==2) {
#ifdef DREAMCAST
		fs_ramdisk_shutdown();
		fs_ramdisk_init();
		reinit_sdcard();
		if (!sdcard_exists) 
			load_from_vmu((char *)S9xGetFilenameBase(".srm",1));
#endif
		CMemory_LoadSRAM (S9xGetFilename (".srm"));
	}
	SDL_FillRect(screen,NULL,0);
	_snes4all_sound_enable_=snes4all_sound_enable;
}

static void keyprocess(SDLKey key, SDL_bool pressed)
{
	uint32 val=0,which=0;

	switch(key)
	{
		case SDLK_RETURN:
			which=0;
			val=SNES_START_MASK;
			break;
		case SDLK_LCTRL:
			which=0;
			val=SNES_B_MASK;
			break;
		case SDLK_LALT:
			which=0;
			val=SNES_A_MASK;
			break;
		case SDLK_SPACE:
			which=0;
			val=SNES_X_MASK;
			break;
		case SDLK_LSHIFT:
			which=0;
			val=SNES_Y_MASK;
			break;
		case SDLK_TAB:
			which=0;
			val=SNES_TR_MASK;
			break;
		case SDLK_BACKSPACE:
			which=0;
			val=SNES_TL_MASK;
			break;
		case SDLK_LEFT:
			which=0;
			val=SNES_LEFT_MASK;
			break;
		case SDLK_RIGHT:
			which=0;
			val=SNES_RIGHT_MASK;
			break;
		case SDLK_UP:
			which=0;
			val=SNES_UP_MASK;
			break;
		case SDLK_DOWN:
			which=0;
			val=SNES_DOWN_MASK;
			break;
#ifndef DREAMCAST
		case SDLK_BACKSLASH:
			which=0;
			val=SNES_SELECT_MASK;
			break;
#endif


		case SDLK_z:
			which=1;
			val=SNES_START_MASK;
			break;
		case SDLK_e:
			which=1;
			val=SNES_B_MASK;
			break;
		case SDLK_q:
			which=1;
			val=SNES_A_MASK;
			break;
		case SDLK_x:
			which=1;
			val=SNES_X_MASK;
			break;
		case SDLK_c:
			which=1;
			val=SNES_Y_MASK;
			break;
		case SDLK_1:
			which=1;
			val=SNES_TR_MASK;
			break;
		case SDLK_2:
			which=1;
			val=SNES_TL_MASK;
			break;
		case SDLK_a:
			which=1;
			val=SNES_LEFT_MASK;
			break;
		case SDLK_d:
			which=1;
			val=SNES_RIGHT_MASK;
			break;
		case SDLK_w:
			which=1;
			val=SNES_UP_MASK;
			break;
		case SDLK_s:
			which=1;
			val=SNES_DOWN_MASK;
			break;
#ifndef DREAMCAST
		case SDLK_3:
			which=1;
			val=SNES_SELECT_MASK;
			break;
#endif



		case SDLK_v:
			which=2;
			val=SNES_START_MASK;
			break;
		case SDLK_y:
			which=2;
			val=SNES_B_MASK;
			break;
		case SDLK_r:
			which=2;
			val=SNES_A_MASK;
			break;
		case SDLK_b:
			which=2;
			val=SNES_X_MASK;
			break;
		case SDLK_n:
			which=2;
			val=SNES_Y_MASK;
			break;
		case SDLK_4:
			which=2;
			val=SNES_TR_MASK;
			break;
		case SDLK_5:
			which=2;
			val=SNES_TL_MASK;
			break;
		case SDLK_f:
			which=2;
			val=SNES_LEFT_MASK;
			break;
		case SDLK_h:
			which=2;
			val=SNES_RIGHT_MASK;
			break;
		case SDLK_t:
			which=2;
			val=SNES_UP_MASK;
			break;
		case SDLK_g:
			which=2;
			val=SNES_DOWN_MASK;
			break;
#ifndef DREAMCAST
		case SDLK_7:
			which=2;
			val=SNES_SELECT_MASK;
			break;
#endif



		case SDLK_m:
			which=3;
			val=SNES_START_MASK;
			break;
		case SDLK_o:
			which=3;
			val=SNES_B_MASK;
			break;
		case SDLK_u:
			which=3;
			val=SNES_A_MASK;
			break;
		case SDLK_COMMA:
			which=3;
			val=SNES_X_MASK;
			break;
		case SDLK_PERIOD:
			which=3;
			val=SNES_Y_MASK;
			break;
		case SDLK_8:
			which=3;
			val=SNES_TR_MASK;
			break;
		case SDLK_9:
			which=3;
			val=SNES_TL_MASK;
			break;
		case SDLK_j:
			which=3;
			val=SNES_LEFT_MASK;
			break;
		case SDLK_l:
			which=3;
			val=SNES_RIGHT_MASK;
			break;
		case SDLK_i:
			which=3;
			val=SNES_UP_MASK;
			break;
		case SDLK_k:
			which=3;
			val=SNES_DOWN_MASK;
			break;
#ifndef DREAMCAST
		case SDLK_0:
			which=3;
			val=SNES_SELECT_MASK;
			break;
#endif

		default:
			return;
	}

	if (pressed)
		joypad_val[which]|=val;
	else
		joypad_val[which]^=val;

//	printf("%s %s -> JOYPAD[%i]=0x%X\n",pressed?"Press":"Unpress", SDL_GetKeyName(key),which,joypad_val[which]);
}
#endif

extern "C"
void S9xSaveStateProcess(void) {
	extern int saveMenu_n_savestate;
	char fname[256];
	char fnamebase[256];
	switch (saveMenu_n_savestate) {
		case 1: strcpy(fname, S9xGetFilename(".1ssn")); strcpy(fnamebase, S9xGetFilenameBase(".1ssn",1)); break;
		case 2: strcpy(fname, S9xGetFilename(".2ssn")); strcpy(fnamebase, S9xGetFilenameBase(".2ssn",1));break;
		case 3: strcpy(fname, S9xGetFilename(".3ssn")); strcpy(fnamebase, S9xGetFilenameBase(".3ssn",1));break;
		default: strcpy(fname, S9xGetFilename(".ssn")); strcpy(fnamebase, S9xGetFilenameBase(".ssn",1));
	}
	if (savestate_state<0) {
		if (savestate_state<-10) {
			savestate_state+=10;
		} else {
			savestate_state=-savestate_state;
		}
		return;
	}
	_snes4all_sound_enable_=0;
	while(!_snes4all_sound_enable_)
		SDL_Delay(60);
	if  (savestate_state==1){
		show_please_wait(" Saving state",screen);
    		S9xFreezeGame (fname);
#ifdef DREAMCAST
		if (!sdcard_exists) 
			save_to_vmu(fnamebase);
#endif
	} else if  (savestate_state==2) {
		show_please_wait(" Loading state",screen);
#ifdef DREAMCAST
		if (!sdcard_exists) 
			load_from_vmu(fnamebase);
#endif
		S9xLoadSnapshot (fname);
	}
	debug_reset_frameskip();
	_snes4all_sound_enable_=snes4all_sound_enable;
	savestate_state=0;
}

void S9xProcessEvents (bool8 block)
{
#ifdef AUTO_EVENTS
	static unsigned cuenta=0;

	switch(cuenta&127)
	{
		case 3:
			joypad_val[0]|=SNES_START_MASK;
			break;
		case 10:
			joypad_val[0]^=SNES_START_MASK;
			break;
	}
#ifdef MAX_AUTO_EVENTS
	if (cuenta>MAX_AUTO_EVENTS)
	{
#ifdef PROFILER_SNES4ALL
		snes4all_prof_show();
#endif
#ifdef DEBUG_FRAMESKIP
		print_frameskip();
#endif
#ifdef DEBUG_TILECACHE
		print_tileche_eficience();
#endif
		exit(0);
	}
#endif
	cuenta++;
#else
#ifndef DREAMCAST
	uint32 num = 0;
	static bool8 TURBO = FALSE;
#else
	static bool8 start_holded = FALSE;
#endif

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			S9xExit();
			break;
#ifdef DREAMCAST
		case SDL_JOYAXISMOTION:
			if (savestate_state || savestate_tostate) break;
			if (event.jaxis.axis==0)
			{
				if (start_holded) {
					if (event.jaxis.value<-120) {
						savestate_tostate=1;
						keyprocess(SDLK_RETURN,SDL_FALSE);
						start_holded=FALSE;
					} else if (event.jaxis.value>120) {
						savestate_tostate=2;
						keyprocess(SDLK_RETURN,SDL_FALSE);
						start_holded=FALSE;
					}
				} else  {
					if (event.jaxis.value<-120)
						goMenu();
				}
			}
			else
			if (event.jaxis.axis==1)
			{
				if (event.jaxis.value>100)
					joypad_val[event.jaxis.which]|=SNES_SELECT_MASK;
				else 
					joypad_val[event.jaxis.which]^=SNES_SELECT_MASK;
			}
			break;
#endif
		case SDL_KEYDOWN:
			keyprocess(event.key.keysym.sym,SDL_TRUE);

#ifdef DREAMCAST
			if (event.key.keysym.sym==SDLK_RETURN)  {
				start_holded=TRUE;
				savestate_tostate=savestate_state=0;
			}
#else
			if (event.key.keysym.sym == SDLK_ESCAPE)
				S9xExit();
			else
			if (event.key.keysym.sym == SDLK_F11)
				goMenu();
			else
			if (event.key.keysym.sym == SDLK_F12)
				SDL_WM_ToggleFullScreen(screen);
			else
			if (event.key.keysym.sym == SDLK_PAGEUP)
			{
				if (!TURBO) {
					TURBO = TRUE;
					OldSkipFrame = Settings.SkipFrames;
					Settings.SkipFrames = 10;
				}
			}
			else
		if (event.key.keysym.sym == SDLK_F1)	PPU.BG_Forced ^= 1;
		    else if (event.key.keysym.sym == SDLK_F2)	PPU.BG_Forced ^= 2;
		    else if (event.key.keysym.sym == SDLK_F3)	PPU.BG_Forced ^= 4;
		    else if (event.key.keysym.sym == SDLK_F4)	PPU.BG_Forced ^= 8;
		    else if (event.key.keysym.sym == SDLK_F5)	PPU.BG_Forced ^= 16;
			else if (event.key.keysym.sym == SDLK_F6)	num = 1;
			else if (event.key.keysym.sym == SDLK_F7)	num = 2;
			else if (event.key.keysym.sym == SDLK_F8)	num = 3;
			else if (event.key.keysym.sym == SDLK_F9)	num = 4;
			else if (event.key.keysym.sym == SDLK_F10) {
					S9xReset();
			}
			if (num) {
				char fname[256], ext[8];
				sprintf(ext, ".00%d", num - 1);
				strcpy(fname, S9xGetFilename (ext));
			    if (event.key.keysym.mod & KMOD_SHIFT)
				    S9xFreezeGame (fname);
				else
					S9xLoadSnapshot (fname);
			}
#endif
			break;
		case SDL_KEYUP:
			keyprocess(event.key.keysym.sym,SDL_FALSE);
#ifdef DREAMCAST
			if (event.key.keysym.sym==SDLK_RETURN) {
				start_holded=FALSE;
				savestate_state=savestate_tostate;
				savestate_tostate=0;
			}
#else
			if (event.key.keysym.sym == SDLK_PAGEUP)
				if (TURBO) {
					TURBO = FALSE;
					Settings.SkipFrames = OldSkipFrame;
				}
#endif
			break;
		}
	}
#endif
}

#ifdef _ZAURUS
static long log2 (long num)
{
    long n = 0;

    while (num >>= 1)
	n++;

    return (n);
}
#endif

#ifdef SNES4ALL_PLAYBACK_RATE
static int Rates[8] =
{
    0, 8192, 11025, 16500, 22050, 29300, 36600, 44000
};
#endif

static int BufferSizes [8] =
{
    0, 256, 256, 256, 512, 512, 1024, 1024
};

#ifdef USE_MENU_MUSIC
static Mix_Chunk *sample_beep=NULL;
static Mix_Music *menu_music=NULL;
#endif

void update_sdl_audio(void *userdata, Uint8 * stream, int len);
void sound_play_menu_music(void)
{
#ifdef USE_MENU_MUSIC
    if (menu_music)
    {
    	Mix_PlayMusic(menu_music,-1);
    	Mix_VolumeMusic(MUSIC_VOLUME);
    }
#endif
}

void sound_play_beep(void) 
{
#ifdef USE_MENU_MUSIC
	if (sample_beep)
		Mix_PlayChannel(0,sample_beep,0);
#endif
}

void sound_enable_music(void)
{
#ifdef USE_MENU_MUSIC
	SDL_PauseAudio(1);
#ifdef DREAMCAST
	SDL_DC_RestoreSoundBuffer();
#endif
	Mix_HookMusic(NULL,NULL);
	SDL_PauseAudio(0);
	Mix_VolumeMusic(MUSIC_VOLUME);
#endif
}

void sound_disable_music(void)
{
#ifdef USE_MENU_MUSIC
	SDL_PauseAudio(1);
	Mix_HookMusic(&update_sdl_audio,NULL);
	SDL_PauseAudio(0);
#endif
}


bool8 S9xOpenSoundDevice (int mode, bool8 stereo, int buffer_size)
{
#ifdef SNES4ALL_STEREO
    so.stereo = stereo;
#endif
#ifdef SNES4ALL_PLAYBACK_RATE
    so.playback_rate = Rates[mode & 0x07];
#endif
    S9xSetPlaybackRate (PLAYBACK_RATE);

    if (buffer_size == 0)
	buffer_size = BufferSizes [mode & 7];
    if (buffer_size > MAX_BUFFER_SIZE / 4)
	buffer_size = MAX_BUFFER_SIZE / 4;
    so.buffer_size=buffer_size;
#ifdef USE_MENU_MUSIC
    Mix_OpenAudio(PLAYBACK_RATE,AUDIO_S16, SOUND_STEREO?2:1,so.buffer_size);
    sample_beep=Mix_LoadWAV(DATA_PREFIX "beep.wav");
    menu_music=Mix_LoadMUS(DATA_PREFIX "music.mod");
#else
    SDL_AudioSpec fmt;
    fmt.freq = PLAYBACK_RATE;
    fmt.format = AUDIO_S16;
    fmt.channels = SOUND_STEREO?2:1;
    fmt.samples = so.buffer_size;
    fmt.callback = update_sdl_audio;
    if ( SDL_OpenAudio(&fmt, NULL) < 0 )
	    return (FALSE);
    _snes4all_sound_enable_=0;
    SDL_PauseAudio(0);
#endif
    return (TRUE);
}

static uint8 Buf[MAX_BUFFER_SIZE*2];

#define FIXED_POINT 0x10000
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_REMAINDER 0xffff

void S9xGenerateSound ()
{
#ifndef NOSOUND
    if (_snes4all_sound_enable_<2)
	    return;
    int buffer_size = so.buffer_size;
    int bytes_so_far = (so.samples_mixed_so_far << 1);
    if (bytes_so_far >= buffer_size)
	return;


    so.err_counter += ERR_RATE;
    if (so.err_counter >= FIXED_POINT)
    {
        int sample_count = so.err_counter >> FIXED_POINT_SHIFT;
	int byte_offset;

        so.err_counter &= FIXED_POINT_REMAINDER;
#ifdef SNES4ALL_STEREO
	if (so.stereo)
	    sample_count <<= 1;
#endif
	byte_offset = bytes_so_far + so.play_position;
	    
	do
	{
	    int sc = sample_count;
	    int byte_count = sample_count;
	    byte_count <<= 1;
	    
	    if ((byte_offset & SOUND_BUFFER_SIZE_MASK) + byte_count > SOUND_BUFFER_SIZE)
	    {
		sc = SOUND_BUFFER_SIZE - (byte_offset & SOUND_BUFFER_SIZE_MASK);
		byte_count = sc;
		sc >>= 1;
	    }
	    if (bytes_so_far + byte_count > buffer_size)
	    {
		byte_count = buffer_size - bytes_so_far;
		if (byte_count == 0)
		    break;
		sc = byte_count;
		sc >>= 1;
	    }
#ifdef USE_MIXSMAPLES0
	    S9xMixSamplesO (Buf, sc, byte_offset & SOUND_BUFFER_SIZE_MASK);
#else
	    S9xMixSamples (Buf + (byte_offset & SOUND_BUFFER_SIZE_MASK), sc);
#endif
	    so.samples_mixed_so_far += sc;
	    sample_count -= sc;
	    bytes_so_far = (so.samples_mixed_so_far << 1);
	    byte_offset += byte_count;
	} while (sample_count > 0);
    }
#endif
}


void update_sdl_audio(void *userdata, Uint8 * stream, int len)
{
#ifndef NOSOUND
    if (_snes4all_sound_enable_<=0) {
#ifdef DREAMCAST
	    SDL_DC_RestoreSoundBuffer();
#endif
	    bzero(stream,len);
	    _snes4all_sound_enable_=-1;
	    return;
    }
    int sample_count = len>>1;
    if (so.samples_mixed_so_far < sample_count)
    {
	int byte_offset = so.play_position + (so.samples_mixed_so_far << 1);
#ifdef USE_MIXSMAPLES0
	S9xMixSamplesO (Buf, sample_count - so.samples_mixed_so_far, byte_offset & SOUND_BUFFER_SIZE_MASK);
#else
    	S9xMixSamples (Buf+(byte_offset & SOUND_BUFFER_SIZE_MASK), sample_count - so.samples_mixed_so_far);
#endif
	so.samples_mixed_so_far = 0;
    }
    else
	so.samples_mixed_so_far -= sample_count;

 
//    if (!so.mute_sound)
#ifndef DREAMCAST
    memcpy(stream,Buf+so.play_position,len);
#else
    SDL_DC_SetSoundBuffer(Buf+so.play_position);
#endif
    so.play_position = (so.play_position + len) & SOUND_BUFFER_SIZE_MASK;
#endif
}


uint32 S9xReadJoypad (int which1)
{
	return joypad_val[which1];
}

#if 0
	ndef _ZAURUS
static int S9xCompareSDD1IndexEntries (const void *p1, const void *p2)
{
    return (*(uint32 *) p1 - *(uint32 *) p2);
}

void S9xLoadSDD1Data ()
{
    char filename [_MAX_PATH + 1];
    char index [_MAX_PATH + 1];
    char data [_MAX_PATH + 1];
    char patch [_MAX_PATH + 1];

    CMemory_FreeSDD1Data ();

    strcpy (filename, S9xGetSnapshotDirectory ());

    if (strncmp (CMemory_ROMName, "Star Ocean", 10) == 0)
	strcat (filename, "/socnsdd1");
    else
	strcat (filename, "/sfa2sdd1");

    DIR *dir = opendir (filename);

    index [0] = 0;
    data [0] = 0;
    patch [0] = 0;

    if (dir)
    {
	struct dirent *d;
	
	while ((d = readdir (dir)))
	{
	    if (strcasecmp (d->d_name, "SDD1GFX.IDX") == 0)
	    {
		strcpy (index, filename);
		strcat (index, "/");
		strcat (index, d->d_name);
	    }
	    else
	    if (strcasecmp (d->d_name, "SDD1GFX.DAT") == 0)
	    {
		strcpy (data, filename);
		strcat (data, "/");
		strcat (data, d->d_name);
	    }
	    if (strcasecmp (d->d_name, "SDD1GFX.PAT") == 0)
	    {
		strcpy (patch, filename);
		strcat (patch, "/");
		strcat (patch, d->d_name);
	    }
	}
	closedir (dir);

	if (strlen (index) && strlen (data))
	{
	    FILE *fs = fopen (index, "rb");
	    int len = 0;

	    if (fs)
	    {
		// Index is stored as a sequence of entries, each entry being
		// 12 bytes consisting of:
		// 4 byte key: (24bit address & 0xfffff * 16) | translated block
		// 4 byte ROM offset
		// 4 byte length
		fseek (fs, 0, SEEK_END);
		len = ftell (fs);
		rewind (fs);
		CMemory_SDD1Index = (uint8 *) malloc (len);
		fread (CMemory_SDD1Index, 1, len, fs);
		fclose (fs);
		CMemory_SDD1Entries = len / 12;

		if (!(fs = fopen (data, "rb")))
		{
		    free ((char *) CMemory_SDD1Index);
		    CMemory_SDD1Index = NULL;
		    CMemory_SDD1Entries = 0;
		}
		else
		{
		    fseek (fs, 0, SEEK_END);
		    len = ftell (fs);
		    rewind (fs);
		    CMemory_SDD1Data = (uint8 *) malloc (len);
		    fread (CMemory_SDD1Data, 1, len, fs);
		    fclose (fs);

		    if (strlen (patch) > 0 &&
			(fs = fopen (patch, "rb")))
		    {
			fclose (fs);
		    }
#ifdef MSB_FIRST
		    // Swap the byte order of the 32-bit value triplets on
		    // MSBFirst machines.
		    uint8 *ptr = CMemory_SDD1Index;
		    for (int i = 0; i < CMemory_SDD1Entries; i++, ptr += 12)
		    {
			SWAP_DWORD ((*(uint32 *) (ptr + 0)));
			SWAP_DWORD ((*(uint32 *) (ptr + 4)));
			SWAP_DWORD ((*(uint32 *) (ptr + 8)));
		    }
#endif
		    qsort (CMemory_SDD1Index, CMemory_SDD1Entries, 12,
			   S9xCompareSDD1IndexEntries);
		}
	    }
	}
	else
	{
	    printf ("Decompressed data pack not found in '%s'.\n", filename);
	}
    }
}
#endif

/*
extern "C"
void salida(unsigned a, unsigned b, unsigned c, unsigned d) {
	printf("\n\n\n\nSALIMOS %p, %p, %p, %p\n",a,b,c,d);
	exit(0);
}
*/
