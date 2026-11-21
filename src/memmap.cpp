/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman
 
  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se) 

 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/

#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>

#ifdef __linux
#include <unistd.h>
#endif

#ifdef JMA_SUPPORT
#include "jma/s9x-jma.h"
#endif

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "ppu.h"
#include "display.h"
#include "cheats.h"
#include "apu.h"
#include "sa1.h"
#include "dsp1.h"
#include "srtc.h"
#include "sdd1.h"
#include "spc7110.h"
#include "seta.h"
#include "unzip.h"
#include "snes4all.h"

#ifdef __W32_HEAP
#include <malloc.h>
#endif

#ifndef ZSNES_FX
#include "fxemu.h"
extern struct FxInit_s SuperFX;
#else
START_EXTERN_C
extern uint8 *SFXPlotTable;
END_EXTERN_C
#endif


#ifndef SET_UI_COLOR
#define SET_UI_COLOR(r,g,b) ;
#endif

//you would think everyone would have these
//since they're so useful.
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef USE_MMU
#define MMU_ALIGN __attribute__ ((__aligned__ (0x1000)))
#else
#define MMU_ALIGN
#endif

static int retry_count=0;
static uint8 bytes0x2000 [0x2000] MMU_ALIGN;
static int is_bsx(unsigned char *);
static int bs_name(unsigned char *);
static int check_char(unsigned);
static void S9xDeinterleaveType2 (bool8 reset);
static __inline__ uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32);

extern char *rom_filename;

const uint32 crc32Table[256] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};



void S9xDeinterleaveType1(int TotalFileSize, uint8 * base)
{
	if(Settings.DisplayColor==0xffff)
	{
		Settings.DisplayColor=BUILD_PIXEL(0,31,0);
		SET_UI_COLOR(0,255,0);
	}

	int i;
	int nblocks = TotalFileSize >> 16;
	uint8 blocks [256];
	for (i = 0; i < nblocks; i++)
	{
		blocks [i * 2] = i + nblocks;
		blocks [i * 2 + 1] = i;
	}
	uint8 *tmp = (uint8 *) malloc (0x8000);
	if (tmp)
	{
		for (i = 0; i < nblocks * 2; i++)
		{
			int j;
			for (j = i; j < nblocks * 2; j++)
			{
				if (blocks [j] == i)
				{
					memmove (tmp, &base [blocks [j] * 0x8000], 0x8000);
					memmove (&base [blocks [j] * 0x8000], 
						&base [blocks [i] * 0x8000], 0x8000);
					memmove (&base [blocks [i] * 0x8000], tmp, 0x8000);
					uint8 b = blocks [j];
					blocks [j] = blocks [i];
					blocks [i] = b;
					break;
				}
			}
		}
		free ((char *) tmp);
	}
}

void S9xDeinterleaveGD24(int TotalFileSize, uint8 * base)
{

	if(TotalFileSize!=0x300000)
		return;

	if(Settings.DisplayColor==0xffff)
	{
		Settings.DisplayColor=BUILD_PIXEL(0,31,31);
		SET_UI_COLOR(0,255,255);
	}

	uint8 *tmp = (uint8 *) malloc (0x80000);
	if (tmp)
	{
		memmove(tmp, &base[0x180000], 0x80000);
		memmove(&base[0x180000], &base[0x200000], 0x80000);
		memmove(&base[0x200000], &base[0x280000], 0x80000);
		memmove(&base[0x280000], tmp, 0x80000);
		free ((char *) tmp);

		S9xDeinterleaveType1(TotalFileSize, base);
	}
}

bool8 CMemory_AllASCII (uint8 *b, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
		if (b[i] < 32 || b[i] > 126)
			return (FALSE);
    }
    return (TRUE);
}

int CMemory_ScoreHiROM (bool8 skip_header, int32 romoff)
{
    int score = 0;
    int o = skip_header ? 0xff00 + 0x200 : 0xff00;
	
	o+=romoff;

	if(CMemory_ROM [o + 0xd5] & 0x1)
		score+=2;

	//Mode23 is SA-1
	if(CMemory_ROM [o + 0xd5] == 0x23)
		score-=2;

	if(CMemory_ROM [o+0xd4] == 0x20)
		score +=2;

    if ((CMemory_ROM [o + 0xdc] + (CMemory_ROM [o + 0xdd] << 8) +
		CMemory_ROM [o + 0xde] + (CMemory_ROM [o + 0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if(0!=(CMemory_ROM [o + 0xde] + (CMemory_ROM [o + 0xdf] << 8)))
			score++;
	}
	
    if (CMemory_ROM [o + 0xda] == 0x33)
		score += 2;
    if ((CMemory_ROM [o + 0xd5] & 0xf) < 4)
		score += 2;
    if (!(CMemory_ROM [o + 0xfd] & 0x80))
		score -= 6;
    if ((CMemory_ROM [o + 0xfc]|(CMemory_ROM [o + 0xfd]<<8))>0xFFB0)
		score -= 2; //reduced after looking at a scan by Cowering
    if (CMemory_CalculatedSize > 1024 * 1024 * 3)
		score += 4;
    if ((1 << (CMemory_ROM [o + 0xd7] - 7)) > 48)
		score -= 1;
    if (!CMemory_AllASCII (&CMemory_ROM [o + 0xb0], 6))
		score -= 1;
    if (!CMemory_AllASCII (&CMemory_ROM [o + 0xc0], ROM_NAME_LEN - 1))
		score -= 1;
	
    return (score);
}	

int CMemory_ScoreLoROM (bool8 skip_header, int32 romoff)
{
    int score = 0;
    int o = skip_header ? 0x7f00 + 0x200 : 0x7f00;
	
	o+=romoff;

	if(!(CMemory_ROM [o + 0xd5] & 0x1))
		score+=3;

	//Mode23 is SA-1
	if(CMemory_ROM [o + 0xd5] == 0x23)
		score+=2;

    if ((CMemory_ROM [o + 0xdc] + (CMemory_ROM [o + 0xdd] << 8) +
		CMemory_ROM [o + 0xde] + (CMemory_ROM [o + 0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if(0!=(CMemory_ROM [o + 0xde] + (CMemory_ROM [o + 0xdf] << 8)))
			score++;
	}
	
    if (CMemory_ROM [o + 0xda] == 0x33)
		score += 2;
    if ((CMemory_ROM [o + 0xd5] & 0xf) < 4)
		score += 2;
    if (CMemory_CalculatedSize <= 1024 * 1024 * 16)
		score += 2;
    if (!(CMemory_ROM [o + 0xfd] & 0x80))
		score -= 6;
	if ((CMemory_ROM [o + 0xfc]|(CMemory_ROM [o + 0xfd]<<8))>0xFFB0)
		score -= 2;//reduced per Cowering suggestion
    if ((1 << (CMemory_ROM [o + 0xd7] - 7)) > 48)
		score -= 1;
    if (!CMemory_AllASCII (&CMemory_ROM [o + 0xb0], 6))
		score -= 1;
    if (!CMemory_AllASCII (&CMemory_ROM [o + 0xc0], ROM_NAME_LEN - 1))
		score -= 1;
	
    return (score);
}

char *CMemory_Safe (const char *s)
{
    static char *safe;
    static int safe_len = 0;

	if(s==NULL)
	{
		if(safe!=NULL)
		{
			free((char*)safe);
			safe = NULL;
		}
		return NULL;
	}
    int len = strlen (s);
    if (!safe || len + 1 > safe_len)
    {
		if (safe)
			free ((char *) safe);
		safe = (char *) malloc (safe_len = len + 1);
    }
    {int i;
    for (i = 0; i < len; i++)
    {
		if (s [i] >= 32 && s [i] < 127)
			safe [i] = s[i];
		else
			safe [i] = '?';
    }
    }
    safe [len] = 0;
    return (safe);
}

/**********************************************************************************************/
/* Init()                                                                                     */
/* This function allocates all the memory needed by the emulator                              */
/**********************************************************************************************/

#if SNES4ALL_MAX_ROM_SIZE != 0x600000
#error SNES4ALL_MAX_ROM_SIZE_INCORRECT
#endif
#if MAX_2BIT_TILES != 4096
#error MAX_2BIT_TILES_INCORRECT
#endif
#if MAX_4BIT_TILES != 2048
#error MAX_4BIT_TILES_INCORRECT
#endif
#if MAX_8BIT_TILES != 1024
#error MAX_4BIT_TILES_INCORRECT
#endif

#if !defined(DREAMCAST) || !defined(USE_OPC_ASM)
static char _IPPU_TileCache_2_[MAX_2BIT_TILES * 128];
static char _IPPU_TileCache_4_[MAX_4BIT_TILES * 128];
static char _IPPU_TileCache_8_[MAX_8BIT_TILES * 128];
static char _IPPU_TileCached_2_[MAX_2BIT_TILES];
static char _IPPU_TileCached_4_[MAX_4BIT_TILES];
static char _IPPU_TileCached_8_[MAX_8BIT_TILES];
uint8 CMemory_RAM[0x20000] MMU_ALIGN;
uint8 CMemory_SRAM[0x20000] MMU_ALIGN;
uint8 CMemory_VRAM[0x10000] MMU_ALIGN;
uint8 CMemory_BSRAM[0x80000] MMU_ALIGN;
uint8 CMemory_FillRAM[SNES4ALL_MAX_ROM_SIZE + 0x200 + 0x8000] MMU_ALIGN;
#else
extern char _IPPU_TileCache_2_[MAX_2BIT_TILES * 128];
extern char _IPPU_TileCache_4_[MAX_4BIT_TILES * 128];
extern char _IPPU_TileCache_8_[MAX_8BIT_TILES * 128];
extern char _IPPU_TileCached_2_[MAX_2BIT_TILES];
extern char _IPPU_TileCached_4_[MAX_4BIT_TILES];
extern char _IPPU_TileCached_8_[MAX_8BIT_TILES];
#endif

bool8 CMemory_Init ()
{
	memset (CMemory_RAM, 0, 0x20000);
	memset (CMemory_SRAM, 0, 0x20000);
	memset (CMemory_VRAM, 0, 0x10000);
	memset (CMemory_FillRAM, 0, SNES4ALL_MAX_ROM_SIZE + 0x200 + 0x8000);
	memset (CMemory_BSRAM, 0, 0x80000);
	
    IPPU.TileCache [TILE_2BIT] = (uint8 *) &_IPPU_TileCache_2_;
    IPPU.TileCache [TILE_4BIT] = (uint8 *) &_IPPU_TileCache_4_;
    IPPU.TileCache [TILE_8BIT] = (uint8 *) &_IPPU_TileCache_8_;
    
    IPPU.TileCached [TILE_2BIT] = (uint8 *) &_IPPU_TileCached_2_;
    IPPU.TileCached [TILE_4BIT] = (uint8 *) &_IPPU_TileCached_4_;
    IPPU.TileCached [TILE_8BIT] = (uint8 *) &_IPPU_TileCached_8_;
   
#if 0 
    if (!CMemory_RAM || !CMemory_SRAM || !CMemory_VRAM || !CMemory_ROM || !CMemory_BSRAM ||
        !IPPU.TileCache [TILE_2BIT] || !IPPU.TileCache [TILE_4BIT] ||
		!IPPU.TileCache [TILE_8BIT] || !IPPU.TileCached [TILE_2BIT] ||
		!IPPU.TileCached [TILE_4BIT] ||	!IPPU.TileCached [TILE_8BIT])
    {
		CMemory_Deinit ();
		return (FALSE);
    }
#endif
	
#ifdef ZSNES_FX
    SFXPlotTable = CMemory_ROM + 0x400000;
#else
    SuperFX.pvRegisters = &CMemory_FillRAM [0x3000];
    SuperFX.nRamBanks = 2;	// Most only use 1.  1=64KB, 2=128KB=1024Mb
    SuperFX.pvRam = CMemory_SRAM;
    SuperFX.nRomBanks = (2 * 1024 * 1024) / (32 * 1024);
    SuperFX.pvRom = (uint8 *) CMemory_ROM;
#endif
	
    ZeroMemory (IPPU.TileCache [TILE_2BIT], MAX_2BIT_TILES * 128);
    ZeroMemory (IPPU.TileCache [TILE_4BIT], MAX_4BIT_TILES * 128);
    ZeroMemory (IPPU.TileCache [TILE_8BIT], MAX_8BIT_TILES * 128);

    ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES);
    ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES);
    ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES);
    
    CMemory_SDD1Data = NULL;
    CMemory_SDD1Index = NULL;
	
    return (TRUE);
}

void CMemory_Deinit ()
{
#ifdef __W32_HEAP
	if(_HEAPOK!=_heapchk())
		MessageBox(GUI.hWnd, "CMemory_Deinit", "Heap Corrupt", MB_OK);
#endif

	IPPU.TileCache [TILE_2BIT] = NULL;
	IPPU.TileCache [TILE_4BIT] = NULL;
	IPPU.TileCache [TILE_8BIT] = NULL;
	IPPU.TileCached [TILE_2BIT] = NULL;
	IPPU.TileCached [TILE_4BIT] = NULL;
	IPPU.TileCached [TILE_8BIT] = NULL;
    CMemory_FreeSDD1Data ();
	CMemory_Safe(NULL);
}

void CMemory_FreeSDD1Data ()
{
    if (CMemory_SDD1Index)
    {
		free ((char *) CMemory_SDD1Index);
		CMemory_SDD1Index = NULL;
    }
    if (CMemory_SDD1Data)
    {
		free ((char *) CMemory_SDD1Data);
		CMemory_SDD1Data = NULL;
    }
}

/**********************************************************************************************/
/* LoadROM()                                                                                  */
/* This function loads a Snes-Backup image                                                    */
/**********************************************************************************************/

bool8 CMemory_LoadROM (const char *filename)
{
    int32 TotalFileSize = 0;
    bool8 Interleaved = FALSE;
    bool8 Tales = FALSE;
 
	uint8* RomHeader=CMemory_ROM;
	
	CMemory_ExtendedFormat=NOPE;

#if 0
 	if(CleanUp7110!=NULL)
		(*CleanUp7110)();
#endif
	
    memset (&SNESGameFixes, 0, sizeof(SNESGameFixes));
    SNESGameFixes.SRAMInitialValue = 0x60;
	
    memset (bytes0x2000, 0, 0x2000);
    CPU.TriedInterleavedMode2 = FALSE;
	
    CMemory_CalculatedSize = 0;
	retry_count =0;

again:
	Settings.DisplayColor=0xffff;
	SET_UI_COLOR(255,255,255);

	TotalFileSize = CMemory_FileLoader(CMemory_ROM, filename, CMemory_MAX_ROM_SIZE);

	if (!TotalFileSize)
		return FALSE;		// it ends here
	else if(!Settings.NoPatch)
		CMemory_CheckForIPSPatch (filename, CMemory_HeaderCount != 0, &TotalFileSize);

	//fix hacked games here.
	if((strncmp("HONKAKUHA IGO GOSEI", (char*)&CMemory_ROM[0x7FC0],19)==0)&&(CMemory_ROM[0x7FD5]!=0x31))
	{
		CMemory_ROM[0x7FD5]=0x31;
		CMemory_ROM[0x7FD6]=0x02;
		Settings.DisplayColor=BUILD_PIXEL(31,0,0);
		SET_UI_COLOR(255,0,0);
		S9xMessage(S9X_ERROR,S9X_ROM_CONFUSING_FORMAT_INFO, "Warning! Hacked Dump!");
	}

	if((strncmp("HONKAKUHA IGO GOSEI", (char*)&CMemory_ROM[0xFFC0],19)==0)&&(CMemory_ROM[0xFFD5]!=0x31))
	{
		CMemory_ROM[0xFFD5]=0x31;
		CMemory_ROM[0xFFD6]=0x02;
		Settings.DisplayColor=BUILD_PIXEL(31,0,0);
		SET_UI_COLOR(255,0,0);
		S9xMessage(S9X_ERROR,S9X_ROM_CONFUSING_FORMAT_INFO, "Warning! Hacked Dump!");
	}

	if((CMemory_ROM[0x7FD5]==0x42)&&(CMemory_ROM[0x7FD6]==0x13)&&(strncmp("METAL COMBAT",(char*)&CMemory_ROM[0x7FC0],12)==0))
	{
		Settings.DisplayColor=BUILD_PIXEL(31,0,0);
		SET_UI_COLOR(255,0,0);
		S9xMessage(S9X_ERROR,S9X_ROM_CONFUSING_FORMAT_INFO, "Warning! Hacked Dump!");
	}

    int orig_hi_score, orig_lo_score;
    int hi_score, lo_score;
	
    orig_hi_score = hi_score = CMemory_ScoreHiROM (FALSE,0);
    orig_lo_score = lo_score = CMemory_ScoreLoROM (FALSE,0);
    
    if (CMemory_HeaderCount == 0 && !Settings.ForceNoHeader &&
		((hi_score > lo_score && CMemory_ScoreHiROM (TRUE,0) > hi_score) ||
		(hi_score <= lo_score && CMemory_ScoreLoROM (TRUE,0) > lo_score)))
    {
		memmove (CMemory_ROM, CMemory_ROM + 512, TotalFileSize - 512);
		TotalFileSize -= 512;
		S9xMessage (S9X_INFO, S9X_HEADER_WARNING, 
			"Try specifying the -nhd command line option if the game doesn't work\n");
		//modifying ROM, so we need to rescore
		orig_hi_score = hi_score = CMemory_ScoreHiROM (FALSE,0);
		orig_lo_score = lo_score = CMemory_ScoreLoROM (FALSE,0);
    }
	
    CMemory_CalculatedSize = (TotalFileSize / 0x2000) * 0x2000;
    ZeroMemory (CMemory_ROM + CMemory_CalculatedSize, CMemory_MAX_ROM_SIZE - CMemory_CalculatedSize);
	
	if(CMemory_CalculatedSize >0x400000&&
		!(CMemory_ROM[0x7FD5]==0x32&&((CMemory_ROM[0x7FD6]&0xF0)==0x40)) && //exclude S-DD1
		!(CMemory_ROM[0xFFD5]==0x3A&&((CMemory_ROM[0xFFD6]&0xF0)==0xF0))) //exclude SPC7110
	{
		//you might be a Jumbo!
		CMemory_ExtendedFormat=YEAH;
	}

	//If both vectors are invalid, it's type 1 LoROM

	if(CMemory_ExtendedFormat==NOPE&&((CMemory_ROM[0x7FFC]|(CMemory_ROM[0x7FFD]<<8))<0x8000)&&((CMemory_ROM[0xFFFC]|(CMemory_ROM[0xFFFD]<<8)) <0x8000))
	{
		if(Settings.DisplayColor==0xffff)
		{
			Settings.DisplayColor=BUILD_PIXEL(0,31,0);
			SET_UI_COLOR(0,255,0);
		}
		if(!Settings.ForceInterleaved)
			S9xDeinterleaveType1(TotalFileSize, CMemory_ROM);
	}

	//CMemory_CalculatedSize is now set, so rescore
	orig_hi_score = hi_score = CMemory_ScoreHiROM (FALSE,0);
	orig_lo_score = lo_score = CMemory_ScoreLoROM (FALSE,0);

	if(NOPE!=CMemory_ExtendedFormat)
	{
		int loromscore, hiromscore, swappedlorom, swappedhirom;
		loromscore=CMemory_ScoreLoROM(FALSE,0);
		hiromscore=CMemory_ScoreHiROM(FALSE,0);
		swappedlorom=CMemory_ScoreLoROM(FALSE, 0x400000);
		swappedhirom=CMemory_ScoreHiROM(FALSE, 0x400000);

		//set swapped here.

		if(max(swappedlorom, swappedhirom) >= max(loromscore, hiromscore))
		{
			CMemory_ExtendedFormat = BIGFIRST;
			hi_score=swappedhirom;
			lo_score=swappedlorom;
			RomHeader=CMemory_ROM+0x400000;
		}
		else
		{
			CMemory_ExtendedFormat = SMALLFIRST;
			lo_score=loromscore;
			hi_score=hiromscore;
			RomHeader=CMemory_ROM;
		}


	}

    Interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2;
    if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
    {
		CMemory_LoROM = TRUE;
		CMemory_HiROM = FALSE;
		
		// Ignore map type byte if not 0x2x or 0x3x
		if ((RomHeader [0x7fd5] & 0xf0) == 0x20 || (RomHeader [0x7fd5] & 0xf0) == 0x30)
		{
			switch (RomHeader [0x7fd5] & 0xf)
			{
			case 1:
				Interleaved = TRUE;
				break;
			case 5:
				Interleaved = TRUE;
				Tales = TRUE;
				break;
			}
		}
    }
    else
    {
		if ((RomHeader [0xffd5] & 0xf0) == 0x20 || (RomHeader [0xffd5] & 0xf0) == 0x30)
		{
			switch (RomHeader [0xffd5] & 0xf)
			{
			case 0:
			case 3:
				Interleaved = TRUE;
				break;
			}
		}
		CMemory_LoROM = FALSE;
		CMemory_HiROM = TRUE;
    }
	
    // More 
    if (!Settings.ForceHiROM && !Settings.ForceLoROM &&
		!Settings.ForceInterleaved && !Settings.ForceInterleaved2 &&
		!Settings.ForceNotInterleaved && !Settings.ForcePAL &&
		!Settings.ForceSuperFX && !Settings.ForceDSP1 &&
		!Settings.ForceSA1 && !Settings.ForceC4 &&
		!Settings.ForceSDD1)
    {


#ifdef DETECT_NASTY_FX_INTERLEAVE
//MK: Damn. YI trips a BRK currently. Maybe even on a real cart.

#ifdef LSB_FIRST 
		if(strncmp((char *) &CMemory_ROM [0x7fc0], "YOSHI'S ISLAND", 14) == 0&&(*(uint16*)&ROM[0x7FDE])==57611&&ROM[0x10002]==0xA9)
#else
			if(strncmp((char *) &CMemory_ROM [0x7fc0], "YOSHI'S ISLAND", 14) == 0&&(ROM[0x7FDE]+(ROM[0x7FDF]<<8))==57611&&ROM[0x10002]==0xA9)
#endif
		{
			Interleaved=TRUE;
			Settings.ForceInterleaved2=TRUE;
		}
#endif
		if (strncmp ((char *) &CMemory_ROM [0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0)
		{
			CMemory_LoROM = TRUE;
			CMemory_HiROM = FALSE;
			Interleaved = FALSE;
		}
    }
	
    if (!Settings.ForceNotInterleaved && Interleaved)
    {
		CPU.TriedInterleavedMode2 = TRUE;
		S9xMessage (S9X_INFO, S9X_ROM_INTERLEAVED_INFO,
			"ROM image is in interleaved format - converting...");
		
		if (Tales)
		{
			if(CMemory_ExtendedFormat==BIGFIRST)
			{
				S9xDeinterleaveType1(0x400000, CMemory_ROM);
				S9xDeinterleaveType1(CMemory_CalculatedSize-0x400000, CMemory_ROM+0x400000);
			}
			else
			{
				S9xDeinterleaveType1(CMemory_CalculatedSize-0x400000, CMemory_ROM);
				S9xDeinterleaveType1(0x400000, CMemory_ROM+CMemory_CalculatedSize-0x400000);
				
			}
			
			CMemory_LoROM = FALSE;
			CMemory_HiROM = TRUE;
			
			
		}
		else if (Settings.ForceInterleaved2)
		{
			S9xDeinterleaveType2(FALSE);
		}
		else if (Settings.ForceInterleaveGD24 && CMemory_CalculatedSize ==0x300000)
		{
			bool8 t = CMemory_LoROM;
			
			CMemory_LoROM = CMemory_HiROM;
			CMemory_HiROM = t;
			S9xDeinterleaveGD24(CMemory_CalculatedSize, CMemory_ROM);
		}
		else
		{
			if(Settings.DisplayColor==0xffff)
			{
				Settings.DisplayColor=BUILD_PIXEL(0,31,0);
				SET_UI_COLOR(0,255,0);
			}
			bool8 t = CMemory_LoROM;
			
			CMemory_LoROM = CMemory_HiROM;
			CMemory_HiROM = t;
			
			S9xDeinterleaveType1(CMemory_CalculatedSize, CMemory_ROM);
		}

		hi_score = CMemory_ScoreHiROM (FALSE,0);
		lo_score = CMemory_ScoreLoROM (FALSE,0);
		
		if ((CMemory_HiROM &&
			(lo_score >= hi_score || hi_score < 0)) ||
			(CMemory_LoROM && 
			(hi_score > lo_score || lo_score < 0)))
		{
			if (retry_count == 0)
			{
				S9xMessage (S9X_INFO, S9X_ROM_CONFUSING_FORMAT_INFO,
					"ROM lied about its type! Trying again.");
				Settings.ForceNotInterleaved = TRUE;
				Settings.ForceInterleaved = FALSE;
				retry_count++;
				goto again;
			}
		}
    }

	if(CMemory_ExtendedFormat==SMALLFIRST)
		Tales=TRUE;

    CMemory_FreeSDD1Data ();
    CMemory_InitROM (Tales);
    S9xLoadCheatFile (S9xGetFilename(".cht"));
    S9xInitCheatData ();
	S9xApplyCheats ();
	
    S9xReset ();
	
    return (TRUE);
}

uint32 CMemory_FileLoader (uint8* buffer, const char* filename, int32 maxsize)
{

 
	STREAM ROMFile;
	int32 TotalFileSize = 0;
	int len = 0;
	int nFormat=FILE_DEFAULT;
 
	char dir [_MAX_DIR + 1];
    char drive [_MAX_DRIVE + 1];
    char name [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
    char fname [_MAX_PATH + 1];

	unsigned long FileSize = 0;

#ifdef UNZIP_SUPPORT
	unzFile file=NULL;
#endif
    
	_splitpath (filename, drive, dir, name, ext);
    _makepath (fname, drive, dir, name, ext);
	
#if defined(__WIN32__) || defined(__MACOSX__)
    memmove (&ext [0], &ext[1], 4);
#endif

	if (strcasecmp (ext, "zip") == 0)
		nFormat = FILE_ZIP;
	else if (strcasecmp (ext, "rar") == 0)
		nFormat = FILE_RAR;
	else if (strcasecmp (ext, "jma") == 0)
		nFormat = FILE_JMA;	
	else
		nFormat = FILE_DEFAULT;


	switch( nFormat )
	{
	case FILE_ZIP:

#ifdef UNZIP_SUPPORT

		file = unzOpen(fname);

		if(file != NULL)	
		{
			
			// its a valid ZIP, close it and let LoadZIP handle it.

			unzClose(file);
		
			if (!LoadZip (fname, &TotalFileSize, &CMemory_HeaderCount, CMemory_ROM))
				return (0);
		
			strcpy (CMemory_ROMFilename, fname);

		}
		else
		{
			// its a bad zip file. Walk away

		 	S9xMessage (S9X_ERROR, S9X_ROM_INFO, "Invalid Zip Archive.");
			return (0);
		}
#else
		S9xMessage (S9X_ERROR, S9X_ROM_INFO, "This binary was not created with Zip support.");
		return (0);
#endif
		break;

	case FILE_JMA:
        {
#ifdef JMA_SUPPORT
                size_t FileSize = load_jma_file(fname, ROM);
		
		if (!FileSize)
		{
		 	S9xMessage (S9X_ERROR, S9X_ROM_INFO, "Invalid JMA.");
			return (0);
		}
		
		TotalFileSize = FileSize;
		CMemory_HeaderCount = 0;
		
		size_t calc_size = (FileSize / 0x2000) * 0x2000;
		
		
		if ((FileSize - calc_size == 512 && !Settings.ForceNoHeader) ||
			Settings.ForceHeader)
		{
			memmove (ROM, ROM + 512, calc_size);
			CMemory_HeaderCount = 1;
			FileSize -= 512;
		}

		strcpy (ROMFilename, fname);
#else
		S9xMessage (S9X_ERROR, S9X_ROM_INFO, "This binary was not created with JMA support.");
		return (0);
#endif
		break;
	}
			
	case FILE_RAR:
		// non existant rar loading
		S9xMessage (S9X_ERROR, S9X_ROM_INFO, "Rar Archives are not currently supported.");
		return (0);
		break;

	case FILE_DEFAULT:
	default:
		// any other roms go here

		if ((ROMFile = OPEN_STREAM (fname, "rb")) == NULL)
			return (0);
		
		strcpy (CMemory_ROMFilename, fname);
		
		CMemory_HeaderCount = 0;
		uint8 *ptr = buffer;
		bool8 more = FALSE;
		
		do
		{
			FileSize = READ_STREAM (ptr, maxsize + 0x200 - (ptr - CMemory_ROM), ROMFile);
			CLOSE_STREAM (ROMFile);
			
			int calc_size = (FileSize / 0x2000) * 0x2000;
		
			if ((FileSize - calc_size == 512 && !Settings.ForceNoHeader) ||
				Settings.ForceHeader)
			{
				memmove (ptr, ptr + 512, calc_size);
				CMemory_HeaderCount++;
				FileSize -= 512;
			}
			
			ptr += FileSize;
			TotalFileSize += FileSize;
		

			// check for multi file roms

			if (ptr - CMemory_ROM < maxsize + 0x200 &&
				(isdigit (ext [0]) && ext [1] == 0 && ext [0] < '9'))
			{
				more = TRUE;
				ext [0]++;
#if defined(__WIN32__) || defined(__MACOSX__)
		        memmove (&ext [1], &ext [0], 4);
			    ext [0] = '.';
#endif
				_makepath (fname, drive, dir, name, ext);
			}
			else if (ptr - CMemory_ROM < maxsize + 0x200 &&
					(((len = strlen (name)) == 7 || len == 8) &&
					strncasecmp (name, "sf", 2) == 0 &&
					isdigit (name [2]) && isdigit (name [3]) && isdigit (name [4]) &&
					isdigit (name [5]) && isalpha (name [len - 1])))
			{
				more = TRUE;
				name [len - 1]++;
#if defined(__WIN32__) || defined(__MACOSX__)
				memmove (&ext [1], &ext [0], 4);
				ext [0] = '.';
#endif
				_makepath (fname, drive, dir, name, ext);
			}
			else
				more = FALSE;

		} while (more && (ROMFile = OPEN_STREAM (fname, "rb")) != NULL);
    
		break;
	}
 


    if (CMemory_HeaderCount == 0)
		S9xMessage (S9X_INFO, S9X_HEADERS_INFO, "No ROM file header found.");
    else
    {
		if (CMemory_HeaderCount == 1)
			S9xMessage (S9X_INFO, S9X_HEADERS_INFO,
			"Found ROM file header (and ignored it).");
		else
			S9xMessage (S9X_INFO, S9X_HEADERS_INFO,
			"Found multiple ROM file headers (and ignored them).");
    }
	
	
	return TotalFileSize;

}

#if 0
/**********************************************************************************************/
/* LoadMulti()                                                                                */
/* This function loads a Slotted SNES-Backup image and fills the slot.                        */
/**********************************************************************************************/

bool8 CMemory_LoadMulti (const char *basename, const char *slot1name, const char *slot2name)
{
    unsigned long FileSize = 0;
	
	if(*basename=='\0')
		return FALSE;
	
	SufamiTurbo=TRUE;
	
	int32 offset;

    memset (&SNESGameFixes, 0, sizeof(SNESGameFixes));
    SNESGameFixes.SRAMInitialValue = 0x60;
	
    memset (bytes0x2000, 0, 0x2000);
	
    CMemory_CalculatedSize = 0;
	
	Settings.DisplayColor=0xffff;
	SET_UI_COLOR(255,255,255);
	
    int32 TotalFileSize = CMemory_FileLoader(ROM, basename, MAX_ROM_SIZE);
	
	if(0== TotalFileSize)
		return FALSE;
	else CheckForIPSPatch (basename, CMemory_HeaderCount != 0, TotalFileSize);
	
	CMemory_CalculatedSize=TotalFileSize;

	for(offset=0; offset<TotalFileSize; offset+=0x100000);

	//insert base type test here.
	
	if(slot1name[0]!='\0')
	{
		
		TotalFileSize = CMemory_FileLoader(ROM+offset, slot1name, MAX_ROM_SIZE);
		
		if(0== TotalFileSize)
			return FALSE;
		else CheckForIPSPatch (slot1name, CMemory_HeaderCount != 0, TotalFileSize);
		ROMOffset1=&ROM[offset];
		Slot1Size=TotalFileSize;
	}
	int32 temp=offset;
	for(; offset<temp+TotalFileSize; offset+=0x100000);

	if(slot2name[0]!='\0')
	{
		TotalFileSize = CMemory_FileLoader(ROM+offset, slot2name, MAX_ROM_SIZE);
		
		if(0== TotalFileSize)
			return FALSE;
		else CheckForIPSPatch (slot2name, CMemory_HeaderCount != 0, TotalFileSize);
		ROMOffset2=&ROM[offset];
		Slot2Size=TotalFileSize;
	}

    InitROM (FALSE);
    S9xLoadCheatFile (S9xGetFilename(".cht"));
    S9xInitCheatData ();
    S9xApplyCheats ();
	
    S9xReset ();
	
    return (TRUE);
}

bool8 SufamiTurboBIOSSig(uint8* file, int32 size)
{
	if(!strcmp((char*)file, "BANDAI SFC-ADX")&&!strcmp((char*)(file+0x10), "SFC-ADX BACKUP"))
	{
		//possible match.
		//check size
		if(size!=0x40000)
			return FALSE;
		//and CRC32
		if(0x9B4CA911==caCRC32(file, size,0xFFFFFFFF))
		{
			return TRUE;
		}

	}
	return FALSE;
}

bool8 SufamiTurboCartSig(uint8* file, int32 size)
{
	//test not a BIOS
	if(!strcmp((char*)file, "BANDAI SFC-ADX")&&strcmp((char*)(file+0x10), "SFC-ADX BACKUP"))
	{
		//possible match.
		//check size
		if(size>0x100000||size <0x80000)
			return FALSE;
		//probably a minicart
		return TRUE;
	}
	return FALSE;
}

bool8 SameGameSig(uint8* file, int32 size)
{
	//preheader sig
	if(strcmp((char*)(file+0xFFA0),"1995/12/16 10:2018ZS5J"))
		return FALSE;
	if(size!=0x100000)
		return FALSE;
	if(0x133E1C5B==caCRC32(file, size,0xFFFFFFFF))
		return TRUE;
	return FALSE;
}
bool8 GNextSig(uint8* file, int32 size)
{
	//preheader sig
	if(strcmp((char*)(file+0xFFAA),"GNEXT B2ZX3J"))
		return FALSE;
	if(size!=0x180000)
		return FALSE;
	if(0x845E420D==caCRC32(file, size,0xFFFFFFFF))
		return TRUE;
	return FALSE;
}
int MultiType(uint8* file, int32 size)
{
	//check for ST signiture
	if(SufamiTurboBIOSSig(file, size))
		return 1;
	//check for Same Game signiture
	if(SameGameSig(file, size))
		return 2;
	//check for G-Next signiture
	if(GNextSig(file, size))
		return 3;
	return 0;
}

#endif

//compatibility wrapper
void S9xDeinterleaveMode2 ()
{
	S9xDeinterleaveType2(TRUE);
}

void S9xDeinterleaveType2 (bool8 reset)
{
	if(Settings.DisplayColor==0xffff||Settings.DisplayColor==BUILD_PIXEL(0,31,0))
	{
		Settings.DisplayColor=BUILD_PIXEL(31,14,6);
		SET_UI_COLOR(255,119,25);
		  
	}
    S9xMessage (S9X_INFO, S9X_ROM_INTERLEAVED_INFO,
		"ROM image is in interleaved format - converting...");
	
    int nblocks = CMemory_CalculatedSize >> 16;
    int step = 64;
	
    while (nblocks <= step)
		step >>= 1;
	
    nblocks = step;
    uint8 blocks [256];
    int i;
	
    for (i = 0; i < nblocks * 2; i++)
    {
		blocks [i] = (i & ~0xF) | ((i & 3) << 2) |
			((i & 12) >> 2);
    }
	
    uint8 *tmp = (uint8 *) malloc (0x10000);
	
    if (tmp)
    {
		for (i = 0; i < nblocks * 2; i++)
		{
			int j;
			for (j = i; j < nblocks * 2; j++)
			{
				if (blocks [j] == i)
				{
					memmove (tmp, &CMemory_ROM [blocks [j] * 0x10000], 0x10000);
					memmove (&CMemory_ROM [blocks [j] * 0x10000], 
						&CMemory_ROM [blocks [i] * 0x10000], 0x10000);
					memmove (&CMemory_ROM [blocks [i] * 0x10000], tmp, 0x10000);
					uint8 b = blocks [j];
					blocks [j] = blocks [i];
					blocks [i] = b;
					break;
				}
			}
		}
		free ((char *) tmp);
		tmp=NULL;
    }
	if(reset)
	{
	    CMemory_InitROM (FALSE);
		S9xReset ();
	}
}

//CRC32 for char arrays
static __inline__ uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32)
{
  register uint32 i;
  for (i = 0; i < size; i++)
  {
    crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];
  }
  return ~crc32;
}

void CMemory_InitROM (bool8 Interleaved)
{
#ifndef ZSNES_FX
    SuperFX.nRomBanks = CMemory_CalculatedSize >> 15;
#endif
    Settings.MultiPlayer5Master = Settings.MultiPlayer5;
    Settings.MouseMaster = Settings.Mouse;
    Settings.SuperScopeMaster = Settings.SuperScope;
    Settings.DSP1Master = Settings.ForceDSP1;
    Settings.SuperFX = FALSE;
    Settings.SA1 = FALSE;
    Settings.C4 = FALSE;
    Settings.SDD1 = FALSE;
    Settings.SRTC = FALSE;
	Settings.SPC7110=FALSE;
	Settings.SPC7110RTC=FALSE;
	Settings.BS=FALSE;
	Settings.OBC1=FALSE;
	Settings.SETA=FALSE;
	s7r.DataRomSize = 0;
	CMemory_CalculatedChecksum=0;
	uint8* RomHeader;

	RomHeader=CMemory_ROM+0x7FB0;

	if(CMemory_ExtendedFormat==BIGFIRST)
		RomHeader+=0x400000;

	if(CMemory_HiROM)
		RomHeader+=0x8000;

//	if(!Settings.BS)
	{
		Settings.BS=(-1!=is_bsx(CMemory_ROM+0x7FC0));

		if(Settings.BS)
		{
			CMemory_LoROM=TRUE;
			CMemory_HiROM=FALSE;
		}

		else
		{
			Settings.BS=(-1!=is_bsx(CMemory_ROM+0xFFC0));
			if(Settings.BS)
			{
				CMemory_HiROM=TRUE;
				CMemory_LoROM=FALSE;
			}
		}
	}

    ZeroMemory (CMemory_BlockIsRAM, MEMMAP_NUM_BLOCKS);
    ZeroMemory (CMemory_BlockIsROM, MEMMAP_NUM_BLOCKS);

    memset (CMemory_ROMId, 0, 5);
    memset (CMemory_CompanyId, 0, 3);

	CMemory_ParseSNESHeader(RomHeader);
	
	// Try to auto-detect the DSP1 chip
	if (!Settings.ForceNoDSP1 &&
			(CMemory_ROMType & 0xf) >= 3 && (CMemory_ROMType & 0xf0) == 0)
			Settings.DSP1Master = TRUE;
	
	if (CMemory_HiROM)
    {
//	    Settings.C4 = Settings.ForceC4;
	    // Enable S-RTC (Real Time Clock) emulation for Dai Kaijyu Monogatari 2
	    Settings.SRTC = ((CMemory_ROMType & 0xf0) >> 4) == 5;

		if(((CMemory_ROMSpeed&0x0F)==0x0A)&&((CMemory_ROMType&0xF0)==0xF0))
		{
			Settings.SPC7110=TRUE;
			if((CMemory_ROMType&0x0F)==0x09)
				Settings.SPC7110RTC=TRUE;
		}
		
		if (Settings.BS)
			CMemory_BSHiROMMap ();
		else if(Settings.SPC7110)
		{
			CMemory_SPC7110HiROMMap();
		}
		else if ((CMemory_ROMSpeed & ~0x10) == 0x25)
		{
			CMemory_TalesROMMap (Interleaved);
		}
		else CMemory_HiROMMap ();
    }
    else
    {
		Settings.SuperFX = Settings.ForceSuperFX;
		
		if(CMemory_ROMType==0x25)
		{
			Settings.OBC1=TRUE;
		}

		//BS-X BIOS
		if(CMemory_ROMType==0xE5)
		{
			Settings.BS=TRUE;
		}

		if ((CMemory_ROMType & 0xf0) == 0x10)
			Settings.SuperFX = !Settings.ForceNoSuperFX;
		
		Settings.SDD1 = Settings.ForceSDD1;
		if ((CMemory_ROMType & 0xf0) == 0x40)
			Settings.SDD1 = !Settings.ForceNoSDD1;

		if (Settings.SDD1)
			S9xLoadSDD1Data ();
		
		if(((CMemory_ROMType &0xF0) == 0xF0)&((CMemory_ROMSpeed&0x0F)!=5))
		{
			CMemory_SRAMSize=2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			if((CMemory_ROMType &0x0F)==6)
			{
				if(CMemory_ROM[0x7FD7]==0x09)
				{
					Settings.SETA=ST_011;
					SetSETA=&S9xSetST011;
					GetSETA=&S9xGetST011;
				}
				else
				{
					Settings.SETA=ST_010;
					SetSETA=&S9xSetST010;
					GetSETA=&S9xGetST010;
				}
			}
			else
			{
				Settings.SETA=ST_018;
				CMemory_SRAMSize=2;
			}
		}
		Settings.C4 = Settings.ForceC4;
		if ((CMemory_ROMType & 0xf0) == 0xf0 &&
            (strncmp (CMemory_ROMName, "MEGAMAN X", 9) == 0 ||
			strncmp (CMemory_ROMName, "ROCKMAN X", 9) == 0))
		{
			Settings.C4 = !Settings.ForceNoC4;
		}
		
		if(Settings.SETA&&Settings.SETA!=ST_018)
		{
			CMemory_SetaDSPMap();
		}
		else if (Settings.SuperFX)
		{
			//CMemory_SRAM = CMemory_ROM + 1024 * 1024 * 4;
			CMemory_SuperFXROMMap ();
			Settings.MultiPlayer5Master = FALSE;
			//Settings.MouseMaster = FALSE;
			//Settings.SuperScopeMaster = FALSE;
			Settings.DSP1Master = FALSE;
			Settings.SA1 = FALSE;
			Settings.C4 = FALSE;
			Settings.SDD1 = FALSE;
		}
		else if (Settings.ForceSA1 ||
			(!Settings.ForceNoSA1 && (CMemory_ROMSpeed & ~0x10) == 0x23 && 
			(CMemory_ROMType & 0xf) > 3 && (CMemory_ROMType & 0xf0) == 0x30))
		{
			Settings.SA1 = TRUE;
//			Settings.MultiPlayer5Master = FALSE;
			//Settings.MouseMaster = FALSE;
			//Settings.SuperScopeMaster = FALSE;
			Settings.DSP1Master = FALSE;
			Settings.C4 = FALSE;
			Settings.SDD1 = FALSE;
			CMemory_SA1ROMMap ();
		}
		else if ((CMemory_ROMSpeed & ~0x10) == 0x25)
			CMemory_TalesROMMap (Interleaved);
		else if(CMemory_ExtendedFormat!=NOPE)
			CMemory_JumboLoROMMap(Interleaved);
		else if (strncmp ((char *) &CMemory_ROM [0x7fc0], "SOUND NOVEL-TCOOL", 17) == 0 ||
			strncmp ((char *) &CMemory_ROM [0x7fc0], "DERBY STALLION 96", 17) == 0)
		{
			CMemory_LoROM24MBSMap ();
			Settings.DSP1Master = FALSE;
		}

		else if (strncmp ((char *) &CMemory_ROM [0x7fc0], "THOROUGHBRED BREEDER3", 21) == 0 ||
			strncmp ((char *) &CMemory_ROM [0x7fc0], "RPG-TCOOL 2", 11) == 0)
		{
			CMemory_SRAM512KLoROMMap ();
			Settings.DSP1Master = FALSE;
		}
		else if (strncmp ((char *) &CMemory_ROM [0x7fc0], "ADD-ON BASE CASSETE", 19) == 0)
		{
			Settings.MultiPlayer5Master = FALSE;
			Settings.MouseMaster = FALSE;
			Settings.SuperScopeMaster = FALSE;
			Settings.DSP1Master = FALSE;
			CMemory_SufamiTurboLoROMMap(); 
			CMemory_SRAMSize = 3;
		}
		else if ((CMemory_ROMSpeed & ~0x10) == 0x22 &&
			strncmp (CMemory_ROMName, "Super Street Fighter", 20) != 0)
		{
			CMemory_AlphaROMMap ();
		}
		else if (Settings.BS)
			CMemory_BSLoROMMap();
		else CMemory_LoROMMap ();
    }

	if(Settings.BS)
	{
		CMemory_ROMRegion=0;
	}

	uint32 sum1 = 0;
	uint32 sum2 = 0;
	if(0==CMemory_CalculatedChecksum)
	{
		int power2 = 0;
		int size = CMemory_CalculatedSize;
		
		while (size >>= 1)
			power2++;
	
		size = 1 << power2;
		uint32 remainder = CMemory_CalculatedSize - size;
	
	
		int i;
	
		for (i = 0; i < size; i++)
			sum1 += CMemory_ROM [i];
	
		for (i = 0; i < (int) remainder; i++)
			sum2 += CMemory_ROM [size + i];
	
		int sub = 0;
		if (Settings.BS&& CMemory_ROMType!=0xE5)
		{
			if (CMemory_HiROM)
			{
				for (i = 0; i < 48; i++)
					sub += CMemory_ROM[0xffb0 + i];
			}
			else if (CMemory_LoROM)
			{
				for (i = 0; i < 48; i++)
					sub += CMemory_ROM[0x7fb0 + i];
			}
			sum1 -= sub;
		}


	    if (remainder)
    		{
				sum1 += sum2 * (size / remainder);
    		}
	
	
    sum1 &= 0xffff;
    CMemory_CalculatedChecksum=sum1;
	}
    //now take a CRC32
    CMemory_ROMCRC32 = caCRC32(CMemory_ROM, CMemory_CalculatedSize,0xFFFFFFFF);

	if (Settings.ForceNTSC)
		Settings.PAL = FALSE;
    else if (Settings.ForcePAL)
		Settings.PAL = TRUE;
	else
	{
		//Korea refers to South Korea, which uses NTSC
		switch(CMemory_ROMRegion)
		{
			case 13:
			case 1:
			case 0:
				Settings.PAL=FALSE;
				break;
			default: Settings.PAL=TRUE;
				break;
		}
	}
	if (Settings.PAL)
	{
		snes4all_max_vcounter=SNES_MAX_PAL_VCOUNTER;
		Settings.FrameTime = Settings.FrameTimePAL;
		CMemory_ROMFramesPerSecond = 50;
	}
	else
	{
		snes4all_max_vcounter=SNES_MAX_NTSC_VCOUNTER;
		Settings.FrameTime = Settings.FrameTimeNTSC;
		CMemory_ROMFramesPerSecond = 60;
	}
	
	CMemory_ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen (CMemory_ROMName))
	{
		char *p = CMemory_ROMName + strlen (CMemory_ROMName) - 1;
		
		while (p > CMemory_ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}
	
	{
		CMemory_SRAMMask = CMemory_SRAMSize ?
			((1 << (CMemory_SRAMSize + 3)) * 128) - 1 : 0;
	}
	if((CMemory_ROMChecksum + CMemory_ROMComplementChecksum != 0xffff) || CMemory_ROMChecksum != CMemory_CalculatedChecksum || ((uint32)CMemory_CalculatedSize > (uint32)(((1<<(CMemory_ROMSize-7))*128)*1024)))
	{
		if(Settings.DisplayColor==0xffff || Settings.DisplayColor!=BUILD_PIXEL(31,0,0))
		{
			Settings.DisplayColor=BUILD_PIXEL(31,31,0);
			SET_UI_COLOR(255,255,0);
		}
	}
	
	IAPU.OneCycle = ONE_APU_CYCLE;
	Settings.Shutdown = Settings.ShutdownMaster;
	
#ifndef USE_OLD_DSP1
	SetDSP=&DSP1SetByte;
	GetDSP=&DSP1GetByte;
#endif

	CMemory_ResetSpeedMap();
	CMemory_ApplyROMFixes ();
	sprintf (CMemory_ROMName, "%s", CMemory_Safe (CMemory_ROMName));
	sprintf (CMemory_ROMId, "%s", CMemory_Safe (CMemory_ROMId));
	sprintf (CMemory_CompanyId, "%s", CMemory_Safe (CMemory_CompanyId));
	
		sprintf (String, "\"%s\" [%s] %s, %s, Type: %s, Mode: %s, TV: %s, S-RAM: %s, ROMId: %s Company: %2.2s CRC32: %08X",
		CMemory_ROMName,
		(CMemory_ROMChecksum + CMemory_ROMComplementChecksum != 0xffff ||
		CMemory_ROMChecksum != CMemory_CalculatedChecksum) ? "bad checksum" : "checksum ok",
		CMemory_MapType (),
		CMemory_Size (),
		CMemory_KartContents (),
		CMemory_MapMode (),
		CMemory_TVStandard (),
		CMemory_StaticRAMSize (),
		CMemory_ROMId,
		CMemory_CompanyId,
		CMemory_ROMCRC32);
	
	S9xMessage (S9X_INFO, S9X_ROM_INFO, String);
#ifdef __WIN32__
	#ifndef _XBOX
		EnableMenuItem(GUI.hMenu, IDM_ROM_INFO, MF_ENABLED);
	#endif
	#ifdef RTC_DEBUGGER
		if(Settings.SPC7110RTC)
			EnableMenuItem(GUI.hMenu, IDM_7110_RTC, MF_ENABLED);
		else EnableMenuItem(GUI.hMenu, IDM_7110_RTC, MF_GRAYED);
	#endif
#endif
	Settings.ForceHeader = Settings.ForceHiROM = Settings.ForceLoROM = 
		Settings.ForceInterleaved = Settings.ForceNoHeader = Settings.ForceNotInterleaved = 
		Settings.ForceInterleaved2=FALSE;
}

bool8 CMemory_LoadSRAM (const char *filename)
{
    int size = CMemory_SRAMSize ?
	       (1 << (CMemory_SRAMSize + 3)) * 128 : 0;
	
    memset (CMemory_SRAM, SNESGameFixes.SRAMInitialValue, 0x20000);
	
    if (size > 0x20000)
		size = 0x20000;
	
    if (size)
    {
	extern int	snes4all_autosave;
	if (snes4all_autosave)
	{
		FILE *file;
		if ((file = fopen (filename, "rb")))
		{
			int len = fread ((char*) CMemory_SRAM, 1, 0x20000, file);
			fclose (file);
			if (len - size == 512)
			{
				// S-RAM file has a header - remove it
				memmove (CMemory_SRAM, CMemory_SRAM + 512, size);
			}
			if (len == size + SRTC_SRAM_PAD)
			{
				S9xSRTCPostLoadState ();
				S9xResetSRTC ();
				rtc.index = -1;
				rtc.mode = MODE_READ;
			}
			else
				S9xHardResetSRTC ();
			
			if(Settings.SPC7110RTC)
			{
				S9xLoadSPC7110RTC (&rtc_f9);
			}
			
#ifndef USE_GL
			sprintf(String, "Loaded %s", S9xBasename (filename));
			S9xMessage (S9X_INFO, S9X_FREEZE_FILE_INFO, String);
#endif
			return (TRUE);
		}
	}
	S9xHardResetSRTC ();
	return (FALSE);
    }
    if (Settings.SDD1)
		S9xSDD1LoadLoggedData ();
	
    return (TRUE);
}

bool8 CMemory_SaveSRAM (const char *filename)
{
	if(Settings.SuperFX && CMemory_ROMType < 0x15)
		return TRUE;
	if(Settings.SA1 && CMemory_ROMType == 0x34)
		return TRUE;

    int size = CMemory_SRAMSize ?
	       (1 << (CMemory_SRAMSize + 3)) * 128 : 0;
    if (Settings.SRTC)
    {
		size += SRTC_SRAM_PAD;
		S9xSRTCPreSaveState ();
    }
	
    if (Settings.SDD1)
		S9xSDD1SaveLoggedData ();
	
    if (size > 0x20000)
		size = 0x20000;
	
    if (size && *CMemory_ROMFilename)
    {
	extern int	snes4all_autosave;
	if (snes4all_autosave)
	{
		FILE *file;
		if ((file = fopen (filename, "wb")))
		{
			fwrite ((char *) CMemory_SRAM, size, 1, file);
			fclose (file);
#if defined(__linux)
			chown (filename, getuid (), getgid ());
#endif
			if(Settings.SPC7110RTC)
			{
				S9xSaveSPC7110RTC (&rtc_f9);
			}
#ifndef USE_GL
			sprintf(String, "Saved %s", S9xBasename (filename));
			S9xMessage (S9X_INFO, S9X_FREEZE_FILE_INFO, String);
#endif
			return (TRUE);
		}
	}
    }
    return (FALSE);
}

void CMemory_FixROMSpeed ()
{
    int c;

	if(CPU.FastROMSpeed==0)
		CPU.FastROMSpeed=SLOW_ONE_CYCLE;
	

    for (c = 0x800; c < 0x1000; c++)
    {
		if (c&0x8 || c&0x400)
			CMemory_MemorySpeed [c] = (uint8) CPU.FastROMSpeed;
    }
}


void CMemory_ResetSpeedMap()
{
	int i;
	memset(CMemory_MemorySpeed, SLOW_ONE_CYCLE, 0x1000);
	for(i=0;i<0x400;i+=0x10)
	{
		CMemory_MemorySpeed[i+2]=CMemory_MemorySpeed[0x800+i+2]= ONE_CYCLE;
		CMemory_MemorySpeed[i+3]=CMemory_MemorySpeed[0x800+i+3]= ONE_CYCLE;
		CMemory_MemorySpeed[i+4]=CMemory_MemorySpeed[0x800+i+4]= ONE_CYCLE;
		CMemory_MemorySpeed[i+5]=CMemory_MemorySpeed[0x800+i+5]= ONE_CYCLE;
	}
	CMemory_FixROMSpeed ();
}

void CMemory_WriteProtectROM ()
{
    memmove ((void *) CMemory_WriteMap, (void *) CMemory_Map, sizeof (CMemory_Map));
    int c;
    for (c = 0; c < 0x1000; c++)
    {
		if (CMemory_BlockIsROM [c])
			CMemory_WriteMap [c] = (uint8 *) CMemory_MAP_NONE;
    }
}

void CMemory_MapRAM ()
{
    int c;

	if(CMemory_LoROM&&!Settings.SDD1)
	{
		// Banks 70->77, S-RAM
		for (c = 0; c < 0x0f; c++)
		{
			int i;
			for(i=0;i<8;i++)
			{
				CMemory_Map [(c<<4) + 0xF00+i]=CMemory_Map [(c<<4) + 0x700+i] = (uint8 *) CMemory_MAP_LOROM_SRAM;
				CMemory_BlockIsRAM [(c<<4) + 0xF00+i] =CMemory_BlockIsRAM [(c<<4) + 0x700+i] = TRUE;
				CMemory_BlockIsROM [(c<<4) + 0xF00+i] =CMemory_BlockIsROM [(c<<4) + 0x700+i] = FALSE;
			}
		}
	}
	else if(CMemory_LoROM&&Settings.SDD1)
	{
		// Banks 70->77, S-RAM
		for (c = 0; c < 0x0f; c++)
		{
			int i;
			for(i=0;i<8;i++)
			{
				CMemory_Map [(c<<4) + 0x700+i] = (uint8 *) CMemory_MAP_LOROM_SRAM;
				CMemory_BlockIsRAM [(c<<4) + 0x700+i] = TRUE;
				CMemory_BlockIsROM [(c<<4) + 0x700+i] = FALSE;
			}
		}
	}
    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = CMemory_RAM;
		CMemory_Map [c + 0x7f0] = CMemory_RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
	CMemory_WriteProtectROM ();
}

void CMemory_MapExtraRAM ()
{
    int c;
	
    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = CMemory_RAM;
		CMemory_Map [c + 0x7f0] = CMemory_RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
	
    // Banks 70->73, S-RAM
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x700] = CMemory_SRAM;
		CMemory_Map [c + 0x710] = CMemory_SRAM + 0x8000;
		CMemory_Map [c + 0x720] = CMemory_SRAM + 0x10000;
		CMemory_Map [c + 0x730] = CMemory_SRAM + 0x18000;
		
		CMemory_BlockIsRAM [c + 0x700] = TRUE;
		CMemory_BlockIsROM [c + 0x700] = FALSE;
		CMemory_BlockIsRAM [c + 0x710] = TRUE;
		CMemory_BlockIsROM [c + 0x710] = FALSE;
		CMemory_BlockIsRAM [c + 0x720] = TRUE;
		CMemory_BlockIsROM [c + 0x720] = FALSE;
		CMemory_BlockIsRAM [c + 0x730] = TRUE;
		CMemory_BlockIsROM [c + 0x730] = FALSE;
    }
}

void CMemory_LoROMMap ()
{
    int c;
    int i;
    int j;
	int mask[4];
	for (j=0; j<4; j++)
		mask[j]=0x00ff;

	mask[0]=(CMemory_CalculatedSize/0x8000)-1;

	int x;
	bool8 foundZeros;
	bool8 pastZeros;
	
	for(j=0;j<3;j++)
	{
		x=1;
		foundZeros=FALSE;
		pastZeros=FALSE;

		mask[j+1]=mask[j];

		while (x>0x100&&!pastZeros)
		{
			if(mask[j]&x)
			{
				x<<=1;
				if(foundZeros)
					pastZeros=TRUE;
			}
			else
			{
				foundZeros=TRUE;
				pastZeros=FALSE;
				mask[j+1]|=x;
				x<<=1;
			}
		}
	}


    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		if(Settings.SETA==ST_018)
			CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_SETA_RISC;
		else CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		if (Settings.DSP1Master)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_DSP;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_DSP;
		}
		else if (Settings.C4)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_C4;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_C4;
		}
		else if(Settings.OBC1)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_OBC_RAM;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_OBC_RAM;
		}
		else
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) bytes0x2000 - 0x6000;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) bytes0x2000 - 0x6000;
		}
		
		for (i = c + 8; i < c + 16; i++)
		{
			int e=3;
			int d=c>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}
			CMemory_Map [i] = CMemory_Map [i + 0x800] = CMemory_ROM + (((d)-1)*0x8000);
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    if (Settings.DSP1Master)
    {
		// Banks 30->3f and b0->bf
		for (c = 0x300; c < 0x400; c += 16)
		{
			for (i = c + 8; i < c + 16; i++)
			{
				CMemory_Map [i] = CMemory_Map [i + 0x800] = (uint8 *) CMemory_MAP_DSP;
				CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = FALSE;
			}
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 8; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) % CMemory_CalculatedSize];
		
		for (i = c + 8; i < c + 16; i++)
		{
			int e=3;
			int d=(c+0x400)>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}

			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = CMemory_ROM + (((d)-1)*0x8000);
		}
		
		for (i = c; i < c + 16; i++)	
		{
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    if (Settings.DSP1Master)
    {
		for (c = 0; c < 0x100; c++)
		{
			CMemory_Map [c + 0xe00] = (uint8 *) CMemory_MAP_DSP;
			CMemory_BlockIsROM [c + 0xe00] = FALSE;
		}
    }

	int sum=0, k,l, bankcount;
	bankcount=1<<(CMemory_ROMSize-7);//Mbits

	//safety for corrupt headers
	if(bankcount > 128)
		bankcount = (CMemory_CalculatedSize/0x8000)/4;
	bankcount*=4;//to banks
	bankcount<<=4;//Map banks
	bankcount+=0x800;//normalize
	for(k=0x800;k<(bankcount);k+=16)
	{
		uint8* bank=0x8000+CMemory_Map[k+8];
		for(l=0;l<0x8000;l++)
			sum+=bank[l];
	}
	CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_SetaDSPMap ()
{
    int c;
    int i;
    int j;
	int mask[4];
	for (j=0; j<4; j++)
		mask[j]=0x00ff;

	mask[0]=(CMemory_CalculatedSize/0x8000)-1;

	int x;
	bool8 foundZeros;
	bool8 pastZeros;
	
	for(j=0;j<3;j++)
	{
		x=1;
		foundZeros=FALSE;
		pastZeros=FALSE;

		mask[j+1]=mask[j];

		while (x>0x100&&!pastZeros)
		{
			if(mask[j]&x)
			{
				x<<=1;
				if(foundZeros)
					pastZeros=TRUE;
			}
			else
			{
				foundZeros=TRUE;
				pastZeros=FALSE;
				mask[j+1]|=x;
				x<<=1;
			}
		}
	}


    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) bytes0x2000 - 0x6000;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) bytes0x2000 - 0x6000;
		
		for (i = c + 8; i < c + 16; i++)
		{
			int e=3;
			int d=c>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}
			CMemory_Map [i] = CMemory_Map [i + 0x800] = CMemory_ROM + (((d)-1)*0x8000);
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c + 8; i < c + 16; i++)
		{
			int e=3;
			int d=(c+0x400)>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}

			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = CMemory_ROM + (((d)-1)*0x8000);
		}
		
		//only upper half is ROM
		for (i = c+8; i < c + 16; i++)	
		{
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
	memset(CMemory_SRAM, 0, 0x1000);
	for (c=0x600;c<0x680;c+=0x10)
	{
		for(i=0;i<0x08;i++)
		{
			//where does the SETA chip access, anyway?
			//please confirm this?
			CMemory_Map[c+0x80+i]=(uint8*)CMemory_MAP_SETA_DSP;
			CMemory_BlockIsROM [c+0x80+i] = FALSE;
			CMemory_BlockIsRAM [c+0x80+i] = TRUE;
		}
		
		for(i=0;i<0x04;i++)
		{
			//and this!
			CMemory_Map[c+i]=(uint8*)CMemory_MAP_SETA_DSP;
			CMemory_BlockIsROM [c+i] = FALSE;
		}
	}

	int sum=0, k,l, bankcount;
	bankcount=1<<(CMemory_ROMSize-7);//Mbits
	//safety for corrupt headers
	if(bankcount > 128)
		bankcount = (CMemory_CalculatedSize/0x8000)/4;
	bankcount*=4;//to banks
	bankcount<<=4;//Map banks
	bankcount+=0x800;//normalize
	for(k=0x800;k<(bankcount);k+=16)
	{
		uint8* bank=0x8000+CMemory_Map[k+8];
		for(l=0;l<0x8000;l++)
			sum+=bank[l];
	}
	CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_BSLoROMMap ()
{
    int c;
    int i;
	
	if(Settings.BS)
		CMemory_SRAMSize=5;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_RAM;
//		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_SRAM;
CMemory_BlockIsRAM [c + 5] = CMemory_BlockIsRAM [c + 0x805] = TRUE;
		
//		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *)CMemory_MAP_NONE;
//		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *)CMemory_MAP_NONE;
				CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_RAM;
//		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_SRAM;
CMemory_BlockIsRAM [c + 6] = CMemory_BlockIsRAM [c + 0x806] = TRUE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_RAM;
//		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_SRAM;
CMemory_BlockIsRAM [c + 7] = CMemory_BlockIsRAM [c + 0x807] = TRUE;
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [(c << 11) % CMemory_CalculatedSize] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }

	for(c=0;c<8;c++)
	{
		CMemory_Map[(c<<4)+0x105]=(uint8*)CMemory_MAP_LOROM_SRAM;
		CMemory_BlockIsROM [(c<<4)+0x105] = FALSE;
		CMemory_BlockIsRAM [(c<<4)+0x105] = TRUE;
	}
	
  	
  /*  // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 8; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &ROM [(c << 11) % CMemory_CalculatedSize];
		
		for (i = c + 8; i < c + 16; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &ROM [((c << 11) + 0x200000) % CMemory_CalculatedSize - 0x8000];
		
		for (i = c; i < c + 16; i++)	
		{
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	*/
	for(c=1;c<=4;c++)
	{
		for(i=0;i<16; i++)
		{
			CMemory_Map[0x400+i+(c<<4)]=(uint8*)CMemory_MAP_LOROM_SRAM;
			CMemory_BlockIsRAM[0x400+i+(c<<4)]=TRUE;
			CMemory_BlockIsROM[0x400+i+(c<<4)]=FALSE;
		}
	}

	for(i=0;i<0x80;i++)
	{
		CMemory_Map[0x700+i]=&CMemory_BSRAM[0x10000*(i/16)];
		CMemory_BlockIsRAM[0x700+i]=TRUE;
		CMemory_BlockIsROM[0x700+i]=FALSE;
	}
	for (i=0; i<8;i++)
	{
		CMemory_Map[0x205+(i<<4)]=CMemory_Map[0x285+(i<<4)]=CMemory_Map[0x305+(i<<4)]=CMemory_Map[0x385+(i<<4)]=CMemory_Map[0x705+(i<<4)];
		CMemory_BlockIsRAM[0x205+(i<<4)]=CMemory_BlockIsRAM[0x285+(i<<4)]=CMemory_BlockIsRAM[0x305+(i<<4)]=CMemory_BlockIsRAM[0x385+(i<<4)]=TRUE;
		CMemory_BlockIsROM[0x205+(i<<4)]=CMemory_BlockIsROM[0x285+(i<<4)]=CMemory_BlockIsROM[0x305+(i<<4)]=CMemory_BlockIsROM[0x385+(i<<4)]=FALSE;
	}
	for(c=0;c<8;c++)
	{
		CMemory_Map[(c<<4)+0x005]=CMemory_BSRAM-0x5000;
		CMemory_BlockIsROM [(c<<4)+0x005] = FALSE;
		CMemory_BlockIsRAM [(c<<4)+0x005] = TRUE;
	}	
    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();


}

void CMemory_HiROMMap ()
{
    int i;
	int c;
        int j;

		int mask[4];
	for (j=0; j<4; j++)
		mask[j]=0x00ff;

	mask[0]=(CMemory_CalculatedSize/0x10000)-1;

	if (Settings.ForceSA1 ||
			(!Settings.ForceNoSA1 && (CMemory_ROMSpeed & ~0x10) == 0x23 && 
			(CMemory_ROMType & 0xf) > 3 && (CMemory_ROMType & 0xf0) == 0x30))
	{
			Settings.DisplayColor=BUILD_PIXEL(31,0,0);
			SET_UI_COLOR(255,0,0);
	}


	int x;
	bool8 foundZeros;
	bool8 pastZeros;
	
	for(j=0;j<3;j++)
	{
		x=1;
		foundZeros=FALSE;
		pastZeros=FALSE;

		mask[j+1]=mask[j];

		while (x>0x100&&!pastZeros)
		{
			if(mask[j]&x)
			{
				x<<=1;
				if(foundZeros)
					pastZeros=TRUE;
			}
			else
			{
				foundZeros=TRUE;
				pastZeros=FALSE;
				mask[j+1]|=x;
				x<<=1;
			}
		}
	}

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		
		if (Settings.DSP1Master)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_DSP;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_DSP;
		}
		else
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		}
		
		for (i = c + 8; i < c + 16; i++)
		{
			int e=3;
			int d=c>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}
			CMemory_Map [i] = CMemory_Map [i + 0x800] = CMemory_ROM + (d*0x10000);
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM.
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [0x306 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0x307 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0xb06 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0xb07 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_BlockIsRAM [0x306 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0x307 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0xb06 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0xb07 + (c << 4)] = TRUE;
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
		{
			int e=3;
			int d=(c)>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = CMemory_ROM + (d*0x10000);
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }

	int bankmax=0x40+ (1<<(CMemory_ROMSize-6));
	//safety for corrupt headers
	if(bankmax > 128)
		bankmax = 0x80;
	int sum=0;
	for(i=0x40;i<bankmax; i++)
	{
		uint8 * bank_low=(uint8*)CMemory_Map[i<<4];
		for (c=0;c<0x10000; c++)
		{
			sum+=bank_low[c];
		}
	}
	CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_TalesROMMap (bool8 Interleaved)
{
	  int c;
    int i;
	
	if(Interleaved)
	{
		if(Settings.DisplayColor==0xffff)
		{
			Settings.DisplayColor=BUILD_PIXEL(0,31,0);
			SET_UI_COLOR(0,255,0);
		}
	}
    uint32 OFFSET0 = 0x400000;
    uint32 OFFSET1 = 0x400000;
    uint32 OFFSET2 = 0x000000;
	
    if (Interleaved)
    {
		OFFSET0 = 0x000000;
		OFFSET1 = 0x000000;
		OFFSET2 = CMemory_CalculatedSize-0x400000; //changed to work with interleaved DKJM2.
    }
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;

		//makes more sense to map the range here.
		//ToP seems to use sram to skip intro???
		if(c>=0x300)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_HIROM_SRAM;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_HIROM_SRAM;
			CMemory_BlockIsRAM [6 + c] = CMemory_BlockIsRAM [7 + c] =
				CMemory_BlockIsRAM [0x806 + c]= CMemory_BlockIsRAM [0x807 + c] = TRUE;
		}
		else
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;

		}
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = &CMemory_ROM [((c << 12) % (CMemory_CalculatedSize-0x400000)) + OFFSET0];
			CMemory_Map [i + 0x800] = &CMemory_ROM [((c << 12) % 0x400000) + OFFSET2];
			CMemory_BlockIsROM [i] = TRUE;
			CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 8; i++)
		{
			CMemory_Map [i + 0x400] = &CMemory_ROM [((c << 12) % (CMemory_CalculatedSize-0x400000)) + OFFSET1];
			CMemory_Map [i + 0x408] = &CMemory_ROM [((c << 12) % (CMemory_CalculatedSize-0x400000)) + OFFSET1];
			CMemory_Map [i + 0xc00] = &CMemory_ROM [((c << 12) %0x400000)+ OFFSET2];
			CMemory_Map [i + 0xc08] = &CMemory_ROM [((c << 12) % 0x400000) + OFFSET2];
			CMemory_BlockIsROM [i + 0x400] = TRUE;
			CMemory_BlockIsROM [i + 0x408] = TRUE;
			CMemory_BlockIsROM [i + 0xc00] = TRUE;
			CMemory_BlockIsROM [i + 0xc08] = TRUE;
		}
    }

	if((strncmp("TALES",(char*)CMemory_Map[8]+0xFFC0, 5)==0))
	{
		if(((*(CMemory_Map[8]+0xFFDE))==(*(CMemory_Map[0x808]+0xFFDE))))
		{
			Settings.DisplayColor=BUILD_PIXEL(31,0,0);
			SET_UI_COLOR(255,0,0);
		}
	}

	CMemory_ROMChecksum = *(CMemory_Map[8]+0xFFDE) + (*(CMemory_Map[8]+0xFFDF) << 8);
	CMemory_ROMComplementChecksum = *(CMemory_Map[8]+0xFFDC) + (*(CMemory_Map[8]+0xFFDD) << 8);

int sum=0;
for(i=0x40;i<0x80; i++)
{
	uint8 * bank_low=(uint8*)CMemory_Map[i<<4];
	uint8 * bank_high=(uint8*)CMemory_Map[(i<<4)+0x800];
	for (c=0;c<0x10000; c++)
	{
		sum+=bank_low[c];
		sum+=bank_high[c];
	}
}

CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_AlphaROMMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = TRUE;
		}
    }
	
    // Banks 40->7f and c0->ff
	
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
		{
			CMemory_Map [i + 0x400] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void DetectSuperFxRamSize()
{
	if(CMemory_ROM[0x7FDA]==0x33)
	{
		CMemory_SRAMSize=CMemory_ROM[0x7FBD];
	}
	else
	{
		if(strncmp(CMemory_ROMName, "STAR FOX 2", 10)==0)
		{
			CMemory_SRAMSize=6;
		}
		else CMemory_SRAMSize=5;
	}
}

void CMemory_SuperFXROMMap ()
{
    int c;
    int i;
 
	DetectSuperFxRamSize();

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [0x006 + c] = CMemory_Map [0x806 + c] = (uint8 *) CMemory_SRAM - 0x6000;
		CMemory_Map [0x007 + c] = CMemory_Map [0x807 + c] = (uint8 *) CMemory_SRAM - 0x6000;
		CMemory_BlockIsRAM [0x006 + c] = CMemory_BlockIsRAM [0x007 + c] = CMemory_BlockIsRAM [0x806 + c] = CMemory_BlockIsRAM [0x807 + c] = TRUE;

		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
    
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
		{
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = CMemory_RAM;
		CMemory_Map [c + 0x7f0] = CMemory_RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
	
    // Banks 70->71, S-RAM
    for (c = 0; c < 32; c++)
    {
		CMemory_Map [c + 0x700] = CMemory_SRAM + (((c >> 4) & 1) << 16);
		CMemory_BlockIsRAM [c + 0x700] = TRUE;
		CMemory_BlockIsROM [c + 0x700] = FALSE;
    }
	
    // Replicate the first 2Mb of the ROM at ROM + 2MB such that each 32K
    // block is repeated twice in each 64K block.
    for (c = 0; c < 64; c++)
    {
		memmove (&CMemory_ROM [0x200000 + c * 0x10000], &CMemory_ROM [c * 0x8000], 0x8000);
		memmove (&CMemory_ROM [0x208000 + c * 0x10000], &CMemory_ROM [c * 0x8000], 0x8000);
    }
	
    CMemory_WriteProtectROM ();
}

void CMemory_SA1ROMMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) &CMemory_FillRAM [0x3000] - 0x3000;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_BWRAM;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_BWRAM;
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 40->7f
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
			CMemory_Map [i + 0x400] = (uint8 *) &CMemory_SRAM [(c << 12) & 0x1ffff];
		
		for (i = c; i < c + 16; i++)
		{
			CMemory_BlockIsROM [i + 0x400] = FALSE;
		}
    }
	
    // c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c;  i < c + 16; i++)
		{
			CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = CMemory_RAM;
		CMemory_Map [c + 0x7f0] = CMemory_RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
    CMemory_WriteProtectROM ();
	
    // Now copy the map and correct it for the SA1 CPU.
    memmove ((void *) SA1.WriteMap, (void *) CMemory_WriteMap, sizeof (CMemory_WriteMap));
    memmove ((void *) SA1.Map, (void *) CMemory_Map, sizeof (CMemory_Map));
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		SA1.Map [c + 0] = SA1.Map [c + 0x800] = &CMemory_FillRAM [0x3000];
		SA1.Map [c + 1] = SA1.Map [c + 0x801] = (uint8 *) CMemory_MAP_NONE;
		SA1.WriteMap [c + 0] = SA1.WriteMap [c + 0x800] = &CMemory_FillRAM [0x3000];
		SA1.WriteMap [c + 1] = SA1.WriteMap [c + 0x801] = (uint8 *) CMemory_MAP_NONE;
    }
	
    // Banks 60->6f
    for (c = 0; c < 0x100; c++)
		SA1.Map [c + 0x600] = SA1.WriteMap [c + 0x600] = (uint8 *) CMemory_MAP_BWRAM_BITMAP;
    
    CMemory_BWRAM = CMemory_SRAM;
}

void CMemory_LoROM24MBSMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
        CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
        CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x200; c += 16)
    {
		CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
        CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000 + 0x200000;
			CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 8; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) + 0x200000];
		
		for (i = c + 8; i < c + 16; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) + 0x200000 - 0x8000];
		
		for (i = c; i < c + 16; i++)
		{
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    CMemory_MapExtraRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_SufamiTurboLoROMMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 8; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) + 0x200000];
		
		for (i = c + 8; i < c + 16; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) + 0x200000 - 0x8000];
		
		for (i = c; i < c + 16; i++)
		{
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    if (Settings.DSP1Master)
    {
		for (c = 0; c < 0x100; c++)
		{
			CMemory_Map [c + 0xe00] = (uint8 *) CMemory_MAP_DSP;
			CMemory_BlockIsROM [c + 0xe00] = FALSE;
		}
    }
	
    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = CMemory_RAM;
		CMemory_Map [c + 0x7f0] = CMemory_RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
	
    // Banks 60->67, S-RAM
    for (c = 0; c < 0x80; c++)
    {
		CMemory_Map [c + 0x600] = (uint8 *) CMemory_MAP_LOROM_SRAM;
		CMemory_BlockIsRAM [c + 0x600] = TRUE;
		CMemory_BlockIsROM [c + 0x600] = FALSE;
    }
	
    CMemory_WriteProtectROM ();
}

#if 0

//untested!!
void CMemory_SameGameMap ()
{
    int i;
	int c;
    int j;

	int mask[4];
	int mask2[4];
	for (j=0; j<4; j++)
		mask[j]=mask2[j]=0x00ff;

	mask[0]=(CMemory_CalculatedSize/0x10000)-1;
	mask2[0]=(Slot1Size/0x10000)-1;

	int x;
	bool8 foundZeros;
	bool8 pastZeros;
	
	for(j=0;j<3;j++)
	{
		x=1;
		foundZeros=FALSE;
		pastZeros=FALSE;

		mask[j+1]=mask[j];

		while (x>0x100&&!pastZeros)
		{
			if(mask[j]&x)
			{
				x<<=1;
				if(foundZeros)
					pastZeros=TRUE;
			}
			else
			{
				foundZeros=TRUE;
				pastZeros=FALSE;
				mask[j+1]|=x;
				x<<=1;
			}
		}
	}

	for(j=0;j<3;j++)
	{
		x=1;
		foundZeros=FALSE;
		pastZeros=FALSE;

		mask2[j+1]=mask2[j];

		while (x>0x100&&!pastZeros)
		{
			if(mask2[j]&x)
			{
				x<<=1;
				if(foundZeros)
					pastZeros=TRUE;
			}
			else
			{
				foundZeros=TRUE;
				pastZeros=FALSE;
				mask2[j+1]|=x;
				x<<=1;
			}
		}
	}


    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = RAM;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
  }
	
    // Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM.
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [0x306 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0x307 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0xb06 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0xb07 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_BlockIsRAM [0x306 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0x307 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0xb06 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0xb07 + (c << 4)] = TRUE;
    }

	for c=0; c<0x200; c+=16)
	{
		for(i=0;i<8;i++)
		{
			int e=3;
			int d=c>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}

			int f=3;
			int g=c>>4;
			while(g>mask2[0])
			{
				g&=mask2[f];
				f--;
			}
			
			//stuff in HiROM areas
			Map[c+0x400+i]=&ROM[d*0x10000];
			Map[c+0xC00+i]=&ROM[d*0x10000];
			//MINI
			Map[c+0x600+i]=&ROMOffset1[g*0x10000];
			Map[c+0xE00+i]=&ROMOffset1[g*0x10000];

		}
		for(i=8;i<16;i++)
		{
			int e=3;
			int d=c>>4;
			while(d>mask[0])
			{
				d&=mask[e];
				e--;
			}

			int f=3;
			int g=c>>4;
			while(g>mask2[0])
			{
				g&=mask2[f];
				f--;
			}

	
			//all stuff
			//BASE
			Map[c+i]=&ROM[d*0x10000];
			Map[c+0x800+i]=&ROM[d*0x10000];
			Map[c+0x400+i]=&ROM[d*0x10000];
			Map[c+0xC00+i]=&ROM[d*0x10000];
			//MINI
			Map[c+0x200+i]=&ROMOffset1[g*0x10000];
			Map[c+0xA00+i]=&ROMOffset1[g*0x10000];
			Map[c+0x600+i]=&ROMOffset1[g*0x10000];
			Map[c+0xE00+i]=&ROMOffset1[g*0x10000];
		}

	}

	int bankmax=0x40+ (1<<(CMemory_ROMSize-6));
	//safety for corrupt headers
	if(bankmax > 128)
		bankmax = 0x80;
	int sum=0;
	for(i=0x40;i<bankmax; i++)
	{
		uint8 * bank_low=(uint8*)Map[i<<4];
		for (c=0;c<0x10000; c++)
		{
			sum+=bank_low[c];
		}
	}
	CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}


//Untested!!
void CMemory_GNextROMMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) &CMemory_FillRAM [0x3000] - 0x3000;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_BWRAM;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_BWRAM;
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	

    // Banks 40->4f (was 7f, but SNES docs and GNext overdumping shows nothing here.)
    for (c = 0; c < 0x100; c += 16)
    {
		for (i = c; i < c + 16; i++)
			CMemory_Map [i + 0x400] = (uint8 *) &SRAM [(c << 12) & 0x1ffff];
		
		for (i = c; i < c + 16; i++)
		{
			CMemory_BlockIsROM [i + 0x400] = FALSE;
		}
    }
	
    for (c = 0; c < 0x100; c += 16)
	{
		for (i = c; i < c + 16; i++)
			CMemory_Map [i + 0x700] = (uint8 *) &ROMOffset1 [(c << 12) & (Slot1Size-1)];
	}

    // c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c;  i < c + 16; i++)
		{
			CMemory_Map [i + 0xc00] = &ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = RAM;
		CMemory_Map [c + 0x7f0] = RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
    CMemory_WriteProtectROM ();
	
    // Now copy the map and correct it for the SA1 CPU.
    memmove ((void *) SA1.WriteMap, (void *) CMemory_WriteMap, sizeof (CMemory_WriteMap));
    memmove ((void *) SA1.Map, (void *) Map, sizeof (Map));
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		SA1.Map [c + 0] = SA1.Map [c + 0x800] = &CMemory_FillRAM [0x3000];
		SA1.Map [c + 1] = SA1.Map [c + 0x801] = (uint8 *) CMemory_MAP_NONE;
		SA1.WriteMap [c + 0] = SA1.WriteMap [c + 0x800] = &CMemory_FillRAM [0x3000];
		SA1.WriteMap [c + 1] = SA1.WriteMap [c + 0x801] = (uint8 *) CMemory_MAP_NONE;
    }
	
    // Banks 60->6f
    for (c = 0; c < 0x100; c++)
		SA1.Map [c + 0x600] = SA1.WriteMap [c + 0x600] = (uint8 *) CMemory_MAP_BWRAM_BITMAP;
    
    CMemory_BWRAM = SRAM;
}

void CMemory_SufamiTurboAltROMMap ()
{
    int c;
    int i;
	
	if(Slot1Size!=0)
		Slot1CMemory_SRAMSize=(1<<((uint8)ROMOffset1[0x32]))*1024;
	else Slot1Size=0x8000;
	if(Slot2Size!=0)
		Slot2CMemory_SRAMSize=(1<<((uint8)ROMOffset2[0x32]))*1024;
else Slot2Size=0x8000;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;

//		for (i = c + 8; i < c + 16; i++)
//		{
//			CMemory_Map [i] = CMemory_Map [i + 0x800] = &ROM [c << 11] - 0x8000;
//			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
//		}
		
    }

	//Map Bios

	for (c=0; c<0x200; c+=16)
	{
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &ROM [((c>>4)*0x8000)%CMemory_CalculatedSize] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}

	}
	

		for (c=0x200; c<0x400; c+=16)
		{
			for (i = c + 8; i < c + 16; i++)
			{
				if(Slot1Size!=0)
				{
					CMemory_Map [i] = CMemory_Map [i + 0x800] = &ROMOffset1 [(((c>>4)*0x8000)%Slot1Size)] - 0x8000;
					CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
				}
				else CMemory_Map [i] = CMemory_Map [i + 0x800] = (uint8*)CMemory_MAP_NONE;
			}

		}

   for (c=0x400; c<0x600; c+=16)
	{
		for (i = c; i < c + 8; i++)
		{
			if(Slot2Size!=0)
			{
				CMemory_Map [i] = CMemory_Map [i + 0x800] = &ROMOffset2[(((c>>4)*0x8000)%Slot2Size)];
				CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
			}
			else CMemory_Map [i] = CMemory_Map [i + 0x800] = (uint8*)CMemory_MAP_NONE;

		}
		for (i = c + 8; i < c + 16; i++)
		{
			if(Slot2Size!=0)
			{
				CMemory_Map [i] = CMemory_Map [i + 0x800] = &ROMOffset2[(((c>>4)*0x8000)%Slot2Size)] - 0x8000;
				CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
			}
			else CMemory_Map [i] = CMemory_Map [i + 0x800] = (uint8*)CMemory_MAP_NONE;

		}

   }

    // Banks 60->67 (7F?), S-RAM
	if(Slot1CMemory_SRAMSize!=0)
	{
	    for (c = 0; c < 0x100; c++)
		{
			CMemory_Map [c + 0xE00] = CMemory_Map [c + 0x600] = (uint8 *) CMemory_MAP_LOROM_SRAM;
			CMemory_BlockIsRAM [c + 0xE00] = CMemory_BlockIsRAM [c + 0x600] = TRUE;
			CMemory_BlockIsROM [c + 0xE00] = CMemory_BlockIsROM [c + 0x600] = FALSE;
		}
    }
	if(Slot2CMemory_SRAMSize!=0)
	{
	    for (c = 0; c < 0x100; c++)
		{
			CMemory_Map [c + 0xF00] = CMemory_Map [c + 0x700] = (uint8 *) CMemory_MAP_LOROM_SRAM;
			CMemory_BlockIsRAM [c + 0xF00] = CMemory_BlockIsRAM [c + 0x700] = TRUE;
			CMemory_BlockIsROM [c + 0xF00] = CMemory_BlockIsROM [c + 0x700] = FALSE;
		}
    }
	
    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [c + 0x7e0] = RAM;
		CMemory_Map [c + 0x7f0] = RAM + 0x10000;
		CMemory_BlockIsRAM [c + 0x7e0] = TRUE;
		CMemory_BlockIsRAM [c + 0x7f0] = TRUE;
		CMemory_BlockIsROM [c + 0x7e0] = FALSE;
		CMemory_BlockIsROM [c + 0x7f0] = FALSE;
    }
	
    CMemory_WriteProtectROM ();
}
#endif


void CMemory_SRAM512KLoROMMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [c << 11] - 0x8000;
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 8; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) + 0x200000];
		
		for (i = c + 8; i < c + 16; i++)
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 11) + 0x200000 - 0x8000];
		
		for (i = c; i < c + 16; i++)
		{
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    CMemory_MapExtraRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_BSHiROMMap ()
{
    int c;
    int i;
	
	CMemory_SRAMSize=5;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		// XXX: How large is SRAM??
				CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_RAM;
//		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) SRAM;
		CMemory_BlockIsRAM [c + 5] = CMemory_BlockIsRAM [c + 0x805] = TRUE;
//		CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_NONE;
//		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_NONE;
		
						CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_RAM;
//		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) SRAM;
CMemory_BlockIsRAM [c + 6] = CMemory_BlockIsRAM [c + 0x806] = TRUE;
		CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_RAM;
//		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) SRAM;
CMemory_BlockIsRAM [c + 7] = CMemory_BlockIsRAM [c + 0x807] = TRUE;

		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 60->7d offset 0000->7fff & 60->7f offset 8000->ffff PSRAM
    // XXX: How large is PSRAM?

	//not adjusted, but The Dumper says "4 Mbits"
    for (c = 0x600; c < 0x7e0; c += 16)
    {
		for (i = c; i < c + 8; i++)
		{
			CMemory_Map [i] = &CMemory_ROM [0x400000 + (c << 11)];
			CMemory_BlockIsRAM [i] = TRUE;
		}
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = &CMemory_ROM [0x400000 + (c << 11) - 0x8000];
			CMemory_BlockIsRAM [i] = TRUE;
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
		{
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	for(i=0;i<0x80;i++)
	{
		CMemory_Map[0x700+i]=&CMemory_BSRAM[0x10000*(i/16)];
		CMemory_BlockIsRAM[0x700+i]=TRUE;
		CMemory_BlockIsROM[0x700+i]=FALSE;
	}
	for (i=0; i<8;i++)
	{
		CMemory_Map[0x205+(i<<4)]=CMemory_Map[0x285+(i<<4)]=CMemory_Map[0x305+(i<<4)]=CMemory_Map[0x385+(i<<4)]=CMemory_Map[0x705+(i<<4)];
		CMemory_BlockIsRAM[0x205+(i<<4)]=CMemory_BlockIsRAM[0x285+(i<<4)]=CMemory_BlockIsRAM[0x305+(i<<4)]=CMemory_BlockIsRAM[0x385+(i<<4)]=TRUE;
		CMemory_BlockIsROM[0x205+(i<<4)]=CMemory_BlockIsROM[0x285+(i<<4)]=CMemory_BlockIsROM[0x305+(i<<4)]=CMemory_BlockIsROM[0x385+(i<<4)]=FALSE;
	}

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_JumboLoROMMap (bool8 Interleaved)
{
    int c;
    int i;
	
	uint32 OFFSET0 = 0x400000;
    uint32 OFFSET1 = 0x400000;
    uint32 OFFSET2 = 0x000000;
	
    if (Interleaved)
    {
		OFFSET0 = 0x000000;
		OFFSET1 = 0x000000;
		OFFSET2 = CMemory_CalculatedSize-0x400000; //changed to work with interleaved DKJM2.
    }
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		if (Settings.DSP1Master)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_DSP;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_DSP;
		}
		else if (Settings.C4)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) CMemory_MAP_C4;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) CMemory_MAP_C4;
		}
		else
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = (uint8 *) bytes0x2000 - 0x6000;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = (uint8 *) bytes0x2000 - 0x6000;
		}
		
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i]= &CMemory_ROM [((c << 11) % (CMemory_CalculatedSize - 0x400000)) + OFFSET0] - 0x8000;
			CMemory_Map [i + 0x800] = &CMemory_ROM [((c << 11) % (0x400000)) + OFFSET2] - 0x8000;
			CMemory_BlockIsROM [i + 0x800] = CMemory_BlockIsROM [i] = TRUE;
		}
    }
	
    if (Settings.DSP1Master)
    {
		// Banks 30->3f and b0->bf
		for (c = 0x300; c < 0x400; c += 16)
		{
			for (i = c + 8; i < c + 16; i++)
			{
				CMemory_Map [i + 0x800] = (uint8 *) CMemory_MAP_DSP;
				CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = FALSE;
			}
		}
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0x400; c < 0x800; c += 16)
    {
		//updated mappings to correct A15 mirroring
		for (i = c; i < c + 8; i++)
		{
			CMemory_Map [i]= &CMemory_ROM [((c << 11) % (CMemory_CalculatedSize - 0x400000)) + OFFSET0];
			CMemory_Map [i + 0x800] = &CMemory_ROM [((c << 11) % 0x400000) +OFFSET2];
		}

		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i]= &CMemory_ROM [((c << 11) % (CMemory_CalculatedSize - 0x400000)) + OFFSET0] - 0x8000;
			CMemory_Map [i + 0x800] = &CMemory_ROM [((c << 11) % 0x400000) + OFFSET2 ] - 0x8000;
		}
		
		for (i = c; i < c + 16; i++)	
		{
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }

	//ROM type has to be 64 Mbit header!
	int sum=0, k,l;
	for(k=0;k<256;k++)
	{
		uint8* bank=0x8000+CMemory_Map[8+(k<<4)];//use upper half of the banks, and adjust for LoROM.
		for(l=0;l<0x8000;l++)
			sum+=bank[l];
	}
	CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}

void CMemory_SPC7110HiROMMap ()
{
    int c;
    int i;
	
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		CMemory_Map [c + 0] = CMemory_Map [c + 0x800] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 0] = CMemory_BlockIsRAM [c + 0x800] = TRUE;
		CMemory_Map [c + 1] = CMemory_Map [c + 0x801] = CMemory_RAM;
		CMemory_BlockIsRAM [c + 1] = CMemory_BlockIsRAM [c + 0x801] = TRUE;
		
		CMemory_Map [c + 2] = CMemory_Map [c + 0x802] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 3] = CMemory_Map [c + 0x803] = (uint8 *) CMemory_MAP_PPU;
		CMemory_Map [c + 4] = CMemory_Map [c + 0x804] = (uint8 *) CMemory_MAP_CPU;
		CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = (uint8 *) CMemory_MAP_CPU;
		
		CMemory_Map [c + 6] /*= CMemory_Map [c + 0x806]*/ = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [c + 7] /*= CMemory_Map [c + 0x807]*/ = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [c + 0x806]=CMemory_Map [c + 0x807]= (uint8 *) CMemory_MAP_NONE;
		
		for (i = c + 8; i < c + 16; i++)
		{
			CMemory_Map [i] = CMemory_Map [i + 0x800] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i] = CMemory_BlockIsROM [i + 0x800] = TRUE;
		}
    }
	
    // Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM.
    for (c = 0; c < 16; c++)
    {
		CMemory_Map [0x306 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0x307 + (c << 4)] = (uint8 *) CMemory_MAP_HIROM_SRAM;
		CMemory_Map [0xb06 + (c << 4)] = (uint8 *) CMemory_MAP_NONE;
		CMemory_Map [0xb07 + (c << 4)] = (uint8 *) CMemory_MAP_NONE;
		CMemory_BlockIsRAM [0x306 + (c << 4)] = TRUE;
		CMemory_BlockIsRAM [0x307 + (c << 4)] = TRUE;
		//	CMemory_BlockIsRAM [0xb06 + (c << 4)] = TRUE;
		//	CMemory_BlockIsRAM [0xb07 + (c << 4)] = TRUE;
    }
	
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
		{
			CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &CMemory_ROM [(c << 12) % CMemory_CalculatedSize];
			CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
	for (c=0;c<0x10;c++)
	{
		CMemory_Map [0x500+c]=(uint8 *)CMemory_MAP_SPC7110_DRAM;
		CMemory_BlockIsROM [0x500+c]=TRUE;
	}
	
	for (c=0;c<0x100;c++)
	{
		CMemory_Map [0xD00+c] = (uint8 *) CMemory_MAP_SPC7110_ROM;
		CMemory_Map [0xE00+c] = (uint8 *) CMemory_MAP_SPC7110_ROM;
		CMemory_Map [0xF00+c] = (uint8 *) CMemory_MAP_SPC7110_ROM;
		CMemory_BlockIsROM [0xD00+c] = CMemory_BlockIsROM [0xE00+c] = CMemory_BlockIsROM [0xF00+c] = TRUE;
		
	}
	S9xSpc7110Init();

int sum=0;
for(i=0;i<(int)CMemory_CalculatedSize; i++)
{
	sum+=CMemory_ROM[i];
}

if(CMemory_CalculatedSize==0x300000)
	sum<<=1;
CMemory_CalculatedChecksum=sum&0xFFFF;

    CMemory_MapRAM ();
    CMemory_WriteProtectROM ();
}
void CMemory_SPC7110Sram(uint8 newstate)
{
	if(newstate&0x80)
	{
		CMemory_Map[6]=(uint8 *)CMemory_MAP_HIROM_SRAM;
		CMemory_Map[7]=(uint8 *)CMemory_MAP_HIROM_SRAM;
		CMemory_Map[0x306]=(uint8 *)CMemory_MAP_HIROM_SRAM;
		CMemory_Map[0x307]=(uint8 *)CMemory_MAP_HIROM_SRAM;
		
		
	}
	else
	{
		CMemory_Map[6]=(uint8 *)CMemory_MAP_RONLY_SRAM;
		CMemory_Map[7]=(uint8 *)CMemory_MAP_RONLY_SRAM;
		CMemory_Map[0x306]=(uint8 *)CMemory_MAP_RONLY_SRAM;
		CMemory_Map[0x307]=(uint8 *)CMemory_MAP_RONLY_SRAM;
	}
}
const char *CMemory_TVStandard ()
{
    return (Settings.PAL ? "PAL" : "NTSC");
}

const char *CMemory_Speed ()
{
    return (CMemory_ROMSpeed & 0x10 ? "120ns" : "200ns");
}

const char *CMemory_MapType ()
{
    return (CMemory_HiROM ? "HiROM" : "LoROM");
}

const char *CMemory_StaticRAMSize ()
{
    static char tmp [20];
	
    if (CMemory_SRAMSize > 16)
		return ("Corrupt");
    sprintf (tmp, "%dKB", (CMemory_SRAMMask + 1) / 1024);
    return (tmp);
}

const char *CMemory_Size ()
{
    static char tmp [20];
	
    if (CMemory_ROMSize < 7 || CMemory_ROMSize - 7 > 23)
		return ("Corrupt");
    sprintf (tmp, "%dMbits", 1 << (CMemory_ROMSize - 7));
    return (tmp);
}

const char *CMemory_KartContents ()
{
    static char tmp [30];
    static const char *CoPro [16] = {
		"DSP", "SuperFX", "OBC1", "SA-1", "S-DD1", "S-RTC", "CoPro#6",
			"CoPro#7", "CoPro#8", "CoPro#9", "CoPro#10", "CoPro#11", "CoPro#12",
			"CoPro#13", "CoPro#14", "CoPro-Custom"
    };
    static const char *Contents [3] = {
		"ROM", "ROM+RAM", "ROM+RAM+BAT"
    };
	static const char *DSPSel [4] = {
		"DSP1", "DSP2", "DSP3", "DSP4"
	};
    if (CMemory_ROMType == 0&&!Settings.BS)
		return ("ROM only");
	
    sprintf (tmp, "%s", Contents [(CMemory_ROMType & 0xf) % 3]);
	
	if(Settings.BS)
		sprintf (tmp, "%s+%s", tmp, "BSX");
	else if(Settings.SPC7110&&Settings.SPC7110RTC)
		sprintf (tmp, "%s+%s", tmp, "SPC7110+RTC");
	else if(Settings.SPC7110)
		sprintf (tmp, "%s+%s", tmp, "SPC7110");
	else if(Settings.C4)
		sprintf (tmp, "%s+%s", tmp, "C4");
	else if(Settings.SETA!=0)
	{
		switch(Settings.SETA)
		{
		case ST_010:
			sprintf (tmp, "%s+%s", tmp, "ST-010");
			break;
		case ST_011:
			sprintf (tmp, "%s+%s", tmp, "ST-011");
			break;

		case ST_018:
			sprintf (tmp, "%s+%s", tmp, "ST-018");
			break;

		}
	}
    else if ((CMemory_ROMType & 0xf) >= 3)
	{
		if (CMemory_ROMType & 0xf0) 
			sprintf (tmp, "%s+%s", tmp, CoPro [(CMemory_ROMType & 0xf0) >> 4]);
		else
#ifndef USE_OLD_DSP1
			sprintf (tmp, "%s+%s", tmp, DSPSel [DSP1.version]);
#else
			sprintf (tmp, "%s+%s", tmp, "DSP1");
#endif
	}
	
    return (tmp);
}

const char *CMemory_MapMode ()
{
    static char tmp [4];
    sprintf (tmp, "%02x", CMemory_ROMSpeed & ~0x10);
    return (tmp);
}

const char *CMemory_ROMID ()
{
    return (CMemory_ROMId);
}

void CMemory_ApplyROMFixes ()
{
#ifdef __W32_HEAP
	if(_HEAPOK!=_heapchk())
		MessageBox(GUI.hWnd, "CMemory_ApplyROMFixes", "Heap Corrupt", MB_OK);
#endif

	//don't steal my work! -MK
	if(CMemory_ROMCRC32 == 0x1B4A5616 && strncmp(CMemory_ROMName, "RUDORA NO HIHOU", 15)==0)
	{
		strncpy(CMemory_ROMName, "THIS SCRIPT WAS STOLEN", 22);
		Settings.DisplayColor=BUILD_PIXEL(31,0,0);
		SET_UI_COLOR(255,0,0);
	}

	/*
	HACKS NSRT can fix that we hadn't detected before.
[14:25:13] <@Nach>     case 0x0c572ef0: //So called Hook (US)(2648)
[14:25:13] <@Nach>     case 0x6810aa95: //Bazooka Blitzkreig swapped sizes hack -handled
[14:25:17] <@Nach>     case 0x61E29C06: //The Tick region hack
[14:25:19] <@Nach>     case 0x1EF90F74: //Jikkyou Keiba Simulation Stable Star PAL hack
[14:25:23] <@Nach>     case 0x4ab225b5: //So called Krusty's Super Fun House (E)
[14:25:25] <@Nach>     case 0x77fd806a: //Donkey Kong Country 2 (E) v1.1 bad dump -handled
[14:25:27] <@Nach>     case 0x340f23e5: //Donkey Kong Country 3 (U) copier hack - handled
	*/

	if(CMemory_ROMCRC32==0x6810aa95 || CMemory_ROMCRC32==0x340f23e5 || CMemory_ROMCRC32==0x77fd806a ||
		strncmp (CMemory_ROMName, "HIGHWAY BATTLE 2", 16)==0 ||
		(strcmp (CMemory_ROMName, "FX SKIING NINTENDO 96") == 0 && CMemory_ROM[0x7FDA]==0))
	{
		Settings.DisplayColor=BUILD_PIXEL(31,0,0);
		SET_UI_COLOR(255,0,0);
	}

	//Ambiguous chip function pointer assignments
#ifndef USE_OLD_DSP1
	DSP1.version=0;

	//DSP switching:
	if(strncmp(CMemory_ROMName, "DUNGEON MASTER", 14)==0)
	{
		//Set DSP-2
		DSP1.version=1;
		SetDSP=&DSP2SetByte;
		GetDSP=&DSP2GetByte;
	}

	if(strncmp(CMemory_ROMName, "SD\x0b6\x0de\x0dd\x0c0\x0de\x0d1GX", 10)==0)
	{
		//Set DSP-3
		DSP1.version=2;
		strncpy(CMemory_ROMName, "SD Gundam GX", 13);
		SetDSP = &DSP3SetByte;
		GetDSP = &DSP3GetByte;
		DSP3_Reset();
	}

	if(strncmp(CMemory_ROMName, "TOP GEAR 3000", 13)==0
		||strncmp(CMemory_ROMName, "PLANETS CHAMP TG3000", 20)==0)
	{
		//Set DSP-4
		DSP1.version=3;
		SetDSP=&DSP4SetByte;
		GetDSP=&DSP4GetByte;
	}
#endif

	//memory map corrections
	if(strncmp(CMemory_ROMName, "XBAND",5)==0)
	{
		int c;
		for (c=0xE00;c<0xE10;c++)
		{
			CMemory_Map [c] = (uint8 *) CMemory_MAP_LOROM_SRAM;
			CMemory_BlockIsRAM [c] = TRUE;
			CMemory_BlockIsROM [c] = FALSE;
		}
		CMemory_WriteProtectROM ();
	}

		//not MAD-1 compliant
	if(strcmp (CMemory_ROMName, "WANDERERS FROM YS") == 0)
	{
		int c;
		for(c=0;c<0xE0;c++)
		{
			CMemory_Map[c+0x700]=(uint8*)CMemory_MAP_LOROM_SRAM;
			CMemory_BlockIsROM[c+0x700]=FALSE;
			CMemory_BlockIsRAM[c+0x700]=TRUE;
		}
		CMemory_WriteProtectROM();
	}

    if (strcmp (CMemory_ROMName, "GOGO ACKMAN3") == 0 || 
		strcmp (CMemory_ROMName, "HOME ALONE") == 0)
    {
		// Banks 00->3f and 80->bf
		int c;
		for (c = 0; c < 0x400; c += 16)
		{
			CMemory_Map [c + 6] = CMemory_Map [c + 0x806] = CMemory_SRAM;
			CMemory_Map [c + 7] = CMemory_Map [c + 0x807] = CMemory_SRAM;
			CMemory_BlockIsROM [c + 6] = CMemory_BlockIsROM [c + 0x806] = FALSE;
			CMemory_BlockIsROM [c + 7] = CMemory_BlockIsROM [c + 0x807] = FALSE;
			CMemory_BlockIsRAM [c + 6] = CMemory_BlockIsRAM [c + 0x806] = TRUE;
			CMemory_BlockIsRAM [c + 7] = CMemory_BlockIsRAM [c + 0x807] = TRUE;
		}
		CMemory_WriteProtectROM ();
    }

	if (strcmp (CMemory_ROMName, "RADICAL DREAMERS") == 0 ||
		strcmp (CMemory_ROMName, "TREASURE CONFLIX") == 0)
    {
		int c;
		
		for (c = 0; c < 0x80; c++)
		{
			CMemory_Map [c + 0x700] = CMemory_ROM + 0x200000 + 0x1000 * (c & 0xf0);
			CMemory_BlockIsRAM [c + 0x700] = TRUE;
			CMemory_BlockIsROM [c + 0x700] = FALSE;
		}
		for (c = 0; c < 0x400; c += 16)
		{
			CMemory_Map [c + 5] = CMemory_Map [c + 0x805] = CMemory_ROM + 0x300000;
			CMemory_BlockIsRAM [c + 5] = CMemory_BlockIsRAM [c + 0x805] = TRUE;
		}
		CMemory_WriteProtectROM ();
    }

	if(strncmp(CMemory_ROMName, "WAR 2410", 8)==0)
	{
		CMemory_Map [0x005] = (uint8 *) CMemory_RAM;
		CMemory_BlockIsRAM [0x005] = TRUE;
		CMemory_BlockIsROM [0x005] = FALSE;
	}

    if (strcmp (CMemory_ROMName, "BATMAN--REVENGE JOKER") == 0)
    {
		CMemory_HiROM = FALSE;
		CMemory_LoROM = TRUE;
		CMemory_LoROMMap ();
    }


	//NMI hacks
    CPU.NMITriggerPoint = 4;
    if (strcmp (CMemory_ROMName, "CACOMA KNIGHT") == 0)
		CPU.NMITriggerPoint = 25;
		
	//Disabling a speed-up
    // Games which spool sound samples between the SNES and sound CPU using
    // H-DMA as the sample is playing.
    if (strcmp (CMemory_ROMName, "EARTHWORM JIM 2") == 0 ||
		strcmp (CMemory_ROMName, "PRIMAL RAGE") == 0 ||
		strcmp (CMemory_ROMName, "CLAY FIGHTER") == 0 ||
		strcmp (CMemory_ROMName, "ClayFighter 2") == 0 ||
		strncasecmp (CMemory_ROMName, "MADDEN", 6) == 0 ||
		strncmp (CMemory_ROMName, "NHL", 3) == 0 ||
		strcmp (CMemory_ROMName, "WeaponLord") == 0||
		strncmp(CMemory_ROMName, "WAR 2410", 8)==0)
    {
		Settings.Shutdown = FALSE;
    }
	

	//APU timing hacks
	
    // Stunt Racer FX
    if (strcmp (CMemory_ROMId, "CQ  ") == 0 ||
		// Illusion of Gaia
        strncmp (CMemory_ROMId, "JG", 2) == 0 ||
		strcmp (CMemory_ROMName, "GAIA GENSOUKI 1 JPN") == 0)
    {
		IAPU.OneCycle = 13;
    }
	
    // RENDERING RANGER R2
    if (strcmp (CMemory_ROMId, "AVCJ") == 0 ||
		//Mark Davis
		strncmp(CMemory_ROMName, "THE FISHING MASTER", 18)==0 || //needs >= actual APU timing. (21 is .002 Mhz slower)
		// Star Ocean
		strncmp (CMemory_ROMId, "ARF", 3) == 0 ||
		// Tales of Phantasia
		strncmp (CMemory_ROMId, "ATV", 3) == 0 ||
		// Act Raiser 1 & 2
		strncasecmp (CMemory_ROMName, "ActRaiser", 9) == 0 ||
		// Soulblazer
		strcmp (CMemory_ROMName, "SOULBLAZER - 1 USA") == 0 ||
		strcmp (CMemory_ROMName, "SOULBLADER - 1") == 0 ||

		// Terranigma
		strncmp (CMemory_ROMId, "AQT", 3) == 0 ||
		// Robotrek
		strncmp (CMemory_ROMId, "E9 ", 3) == 0 ||
		strcmp (CMemory_ROMName, "SLAP STICK 1 JPN") == 0 ||
		// ZENNIHON PURORESU2
		strncmp (CMemory_ROMId, "APR", 3) == 0 ||
		// Bomberman 4
		strncmp (CMemory_ROMId, "A4B", 3) == 0 ||
		// UFO KAMEN YAKISOBAN
		strncmp (CMemory_ROMId, "Y7 ", 3) == 0 ||
		strncmp (CMemory_ROMId, "Y9 ", 3) == 0 ||
		// Panic Bomber World
		strncmp (CMemory_ROMId, "APB", 3) == 0 ||
		((strncmp (CMemory_ROMName, "Parlor", 6) == 0 || 
		strcmp (CMemory_ROMName, "HEIWA Parlor!Mini8") == 0 ||
		strncmp (CMemory_ROMName, "SANKYO Fever! \xCC\xA8\xB0\xCA\xDE\xB0!", 21) == 0) && //SANKYO Fever! Fever!
		strcmp (CMemory_CompanyId, "A0") == 0) ||
		strcmp (CMemory_ROMName, "DARK KINGDOM") == 0 ||
		strcmp (CMemory_ROMName, "ZAN3 SFC") == 0 ||
		strcmp (CMemory_ROMName, "HIOUDEN") == 0 ||
		strcmp (CMemory_ROMName, "\xC3\xDD\xBC\xC9\xB3\xC0") == 0 ||  //Tenshi no Uta
		strcmp (CMemory_ROMName, "FORTUNE QUEST") == 0 ||
		strcmp (CMemory_ROMName, "FISHING TO BASSING") == 0 ||
		strncmp (CMemory_ROMName, "TokyoDome '95Battle 7", 21) == 0 ||
		strcmp (CMemory_ROMName, "OHMONO BLACKBASS") == 0 ||
		strncmp (CMemory_ROMName, "SWORD WORLD SFC", 15) == 0 ||
		strcmp (CMemory_ROMName, "MASTERS") ==0 || //Augusta 2 J
		strcmp (CMemory_ROMName, "SFC \xB6\xD2\xDD\xD7\xB2\xC0\xDE\xB0") == 0 || //Kamen Rider
		strncmp (CMemory_ROMName, "LETs PACHINKO(", 14) == 0) //A set of BS games
    {
		IAPU.OneCycle = 15;
    }
    

	//Specific game fixes

	Settings.StarfoxHack = strcmp (CMemory_ROMName, "STAR FOX") == 0 ||
		strcmp (CMemory_ROMName, "STAR WING") == 0;
	Settings.WinterGold = strcmp (CMemory_ROMName, "FX SKIING NINTENDO 96") == 0 ||
		strcmp (CMemory_ROMName, "DIRT RACER") == 0 ||
		Settings.StarfoxHack;
	

	if((strcmp(CMemory_ROMName, "LEGEND")==0&&!Settings.PAL)||
		strcmp(CMemory_ROMName, "King Arthurs World")==0)
	{
		SNESGameFixes.EchoOnlyOutput=TRUE;
	}


	    Settings.DaffyDuck = (strcmp (CMemory_ROMName, "DAFFY DUCK: MARV MISS") == 0) ||
		(strcmp (CMemory_ROMName, "ROBOCOP VS THE TERMIN") == 0) ||
		(strcmp (CMemory_ROMName, "ROBOCOP VS TERMINATOR") == 0); //ROBOCOP VS THE TERMIN
    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
	
	//OAM hacks because we don't fully understand the
	//behavior of the SNES.

	//Totally wacky display...
	//seems to need a disproven behavior, so
	//we're definitely overlooking some other bug?
	if(strncmp(CMemory_ROMName, "UNIRACERS", 9)==0)
		SNESGameFixes.Uniracers=TRUE;


	//is this even useful now?
    if (strcmp (CMemory_ROMName, "ALIENS vs. PREDATOR") == 0)
		SNESGameFixes.alienVSpredetorFix = TRUE;
		
    if (strcmp (CMemory_ROMName, "\xBD\xB0\xCA\xDF\xB0\xCC\xA7\xD0\xBD\xC0") == 0 ||  //Super Famista
		strcmp (CMemory_ROMName, "\xBD\xB0\xCA\xDF\xB0\xCC\xA7\xD0\xBD\xC0 2") == 0 || //Super Famista 2
		strcmp (CMemory_ROMName, "ZENKI TENCHIMEIDOU") == 0 ||
		strcmp (CMemory_ROMName, "GANBA LEAGUE") == 0)
    {
		SNESGameFixes.APU_OutPorts_ReturnValueFix = TRUE;
    }


	//CPU timing hacks
	    Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 
		      Settings.CyclesPercentage) / 100;

		//no need to ifdef for right now...
//#ifdef HDMA_HACKS

	// A Couple of HDMA related hacks - Lantus
	if ((strcmp(CMemory_ROMName, "SFX SUPERBUTOUDEN2")==0) ||
	    (strcmp(CMemory_ROMName, "ALIEN vs. PREDATOR")==0) ||
		(strcmp(CMemory_ROMName, "STONE PROTECTORS")==0) ||
	    (strcmp(CMemory_ROMName, "SUPER BATTLETANK 2")==0))
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 130) / 100;

	if(strcmp(CMemory_ROMName, "HOME IMPROVEMENT")==0)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 200) / 100;
 
	// End HDMA hacks
//#endif

	
    if (strcmp (CMemory_ROMId, "ASRJ") == 0 && Settings.CyclesPercentage == 100)
		// Street Racer
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 95) / 100;

	// Power Rangers Fight
    if (strncmp (CMemory_ROMId, "A3R", 3) == 0 ||
        // Clock Tower
		strncmp (CMemory_ROMId, "AJE", 3) == 0)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 103) / 100;
    
	
    if (strncmp (CMemory_ROMId, "A3M", 3) == 0 && Settings.CyclesPercentage == 100)
		// Mortal Kombat 3. Fixes cut off speech sample
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;
	
	//Darkness Beyond Twilight
	//Crimson beyond blood that flows
	//buried in the stream of time
	//is where your power grows
	//I pledge myself to conquer
	//all the foes who stand
	//before the might gift betsowed
	//in my unworthy hand
    if (strcmp (CMemory_ROMName, "\x0bd\x0da\x0b2\x0d4\x0b0\x0bd\x0de") == 0 &&
		Settings.CyclesPercentage == 100)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 101) / 100;


#ifdef DETECT_NASTY_FX_INTERLEAVE
//XXX: Test without these. Win32 port indicates they aren't needed?	
//Apparently are needed!
    if (strcmp (CMemory_ROMName, "WILD TRAX") == 0 || 
		strcmp (CMemory_ROMName, "STAR FOX 2") == 0 || 
		strcmp (CMemory_ROMName, "YOSSY'S ISLAND") == 0 || 
		strcmp (CMemory_ROMName, "YOSHI'S ISLAND") == 0)
		CPU.TriedInterleavedMode2 = TRUE;
#endif

    // Start Trek: Deep Sleep 9
    if (strncmp (CMemory_ROMId, "A9D", 3) == 0 && Settings.CyclesPercentage == 100)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;

    snes4all_apu_hmax=Settings.H_Max*10000L;

	//SA-1 Speedup settings
    SA1.WaitAddress = NULL;
    SA1.WaitByteAddress1 = NULL;
    SA1.WaitByteAddress2 = NULL;
	
    /* Bass Fishing */
    if (strcmp (CMemory_ROMId, "ZBPJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0093f1 >> MEMMAP_SHIFT] + 0x93f1;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x304a;
    }
    /* DAISENRYAKU EXPERTWW2 */
    if (strcmp (CMemory_ROMId, "AEVJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0ed18d >> MEMMAP_SHIFT] + 0xd18d;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3000;
    }
    /* debjk2 */
    if (strcmp (CMemory_ROMId, "A2DJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x008b62 >> MEMMAP_SHIFT] + 0x8b62;
    }
    /* Dragon Ballz HD */
    if (strcmp (CMemory_ROMId, "AZIJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x008083 >> MEMMAP_SHIFT] + 0x8083;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3020;
    }
    /* SFC SDGUNDAMGNEXT */
    if (strcmp (CMemory_ROMId, "ZX3J") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0087f2 >> MEMMAP_SHIFT] + 0x87f2;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x30c4;
    }
    /* ShougiNoHanamichi */
    if (strcmp (CMemory_ROMId, "AARJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc1f85a >> MEMMAP_SHIFT] + 0xf85a;
		SA1.WaitByteAddress1 = CMemory_SRAM + 0x0c64;
		SA1.WaitByteAddress2 = CMemory_SRAM + 0x0c66;
    }
    /* KATO HIFUMI9DAN SYOGI */
    if (strcmp (CMemory_ROMId, "A23J") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc25037 >> MEMMAP_SHIFT] + 0x5037;
		SA1.WaitByteAddress1 = CMemory_SRAM + 0x0c06;
		SA1.WaitByteAddress2 = CMemory_SRAM + 0x0c08;
    }
    /* idaten */
    if (strcmp (CMemory_ROMId, "AIIJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc100be >> MEMMAP_SHIFT] + 0x00be;
		SA1.WaitByteAddress1 = CMemory_SRAM + 0x1002;
		SA1.WaitByteAddress2 = CMemory_SRAM + 0x1004;
    }
    /* igotais */
    if (strcmp (CMemory_ROMId, "AITJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0080b7 >> MEMMAP_SHIFT] + 0x80b7;
    }
    /* J96 DREAM STADIUM */
    if (strcmp (CMemory_ROMId, "AJ6J") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc0f74a >> MEMMAP_SHIFT] + 0xf74a;
    }
    /* JumpinDerby */
    if (strcmp (CMemory_ROMId, "AJUJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00d926 >> MEMMAP_SHIFT] + 0xd926;
    }
    /* JKAKINOKI SHOUGI */
    if (strcmp (CMemory_ROMId, "AKAJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00f070 >> MEMMAP_SHIFT] + 0xf070;
    }
    /* HOSHI NO KIRBY 3 & KIRBY'S DREAM LAND 3 JAP & US */
    if (strcmp (CMemory_ROMId, "AFJJ") == 0 || strcmp (CMemory_ROMId, "AFJE") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0082d4 >> MEMMAP_SHIFT] + 0x82d4;
		SA1.WaitByteAddress1 = CMemory_SRAM + 0x72a4;
    }
    /* KIRBY SUPER DELUXE JAP */
    if (strcmp (CMemory_ROMId, "AKFJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x008c93 >> MEMMAP_SHIFT] + 0x8c93;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x300a;
		SA1.WaitByteAddress2 = CMemory_FillRAM + 0x300e;
    }
    /* KIRBY SUPER DELUXE US */
    if (strcmp (CMemory_ROMId, "AKFE") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x008cb8 >> MEMMAP_SHIFT] + 0x8cb8;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x300a;
		SA1.WaitByteAddress2 = CMemory_FillRAM + 0x300e;
    }
    /* SUPER MARIO RPG JAP & US */
    if (strcmp (CMemory_ROMId, "ARWJ") == 0 || strcmp (CMemory_ROMId, "ARWE") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc0816f >> MEMMAP_SHIFT] + 0x816f;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3000;
    }
    /* marvelous.zip */
    if (strcmp (CMemory_ROMId, "AVRJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0085f2 >> MEMMAP_SHIFT] + 0x85f2;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3024;
    }
    /* AUGUSTA3 MASTERS NEW */
    if (strcmp (CMemory_ROMId, "AO3J") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00dddb >> MEMMAP_SHIFT] + 0xdddb;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x37b4;
    }
    /* OSHABERI PARODIUS */
    if (strcmp (CMemory_ROMId, "AJOJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x8084e5 >> MEMMAP_SHIFT] + 0x84e5;
    }
    /* PANIC BOMBER WORLD */
    if (strcmp (CMemory_ROMId, "APBJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00857a >> MEMMAP_SHIFT] + 0x857a;
    }
    /* PEBBLE BEACH NEW */
    if (strcmp (CMemory_ROMId, "AONJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00df33 >> MEMMAP_SHIFT] + 0xdf33;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x37b4;
    }
    /* PGA EUROPEAN TOUR */
    if (strcmp (CMemory_ROMId, "AEPE") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x003700 >> MEMMAP_SHIFT] + 0x3700;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3102;
    }
    /* PGA TOUR 96 */
    if (strcmp (CMemory_ROMId, "A3GE") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x003700 >> MEMMAP_SHIFT] + 0x3700;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3102;
    }
    /* POWER RANGERS 4 */
    if (strcmp (CMemory_ROMId, "A4RE") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x009899 >> MEMMAP_SHIFT] + 0x9899;
		SA1.WaitByteAddress1 = CMemory_FillRAM + 0x3000;
    }
    /* PACHISURO PALUSUPE */
    if (strcmp (CMemory_ROMId, "AGFJ") == 0)
    {
		// Never seems to turn on the SA-1!
    }
    /* SD F1 GRAND PRIX */
    if (strcmp (CMemory_ROMId, "AGFJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x0181bc >> MEMMAP_SHIFT] + 0x81bc;
    }
    /* SHOUGI MARJONG */
    if (strcmp (CMemory_ROMId, "ASYJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00f2cc >> MEMMAP_SHIFT] + 0xf2cc;
		SA1.WaitByteAddress1 = CMemory_SRAM + 0x7ffe;
		SA1.WaitByteAddress2 = CMemory_SRAM + 0x7ffc;
    }
    /* shogisai2 */
    if (strcmp (CMemory_ROMId, "AX2J") == 0)
    {
		SA1.WaitAddress = SA1.Map [0x00d675 >> MEMMAP_SHIFT] + 0xd675;
    }
	
    /* SHINING SCORPION */
    if (strcmp (CMemory_ROMId, "A4WJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc048be >> MEMMAP_SHIFT] + 0x48be;
    }
    /* SHIN SHOUGI CLUB */
    if (strcmp (CMemory_ROMId, "AHJJ") == 0)
    {
		SA1.WaitAddress = SA1.Map [0xc1002a >> MEMMAP_SHIFT] + 0x002a;
		SA1.WaitByteAddress1 = CMemory_SRAM + 0x0806;
		SA1.WaitByteAddress2 = CMemory_SRAM + 0x0808;
    }
	

	//Other

    // Additional game fixes by sanmaiwashi ...
    if (strcmp (CMemory_ROMName, "SFX \xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6\xDE\xC0\xD8 1") == 0) // Gundam Knight Story
    {
		bytes0x2000 [0xb18] = 0x4c;
		bytes0x2000 [0xb19] = 0x4b;
		bytes0x2000 [0xb1a] = 0xea;
		SNESGameFixes.SRAMInitialValue = 0x6b;
    }
	
	
    // HITOMI3
    if (strcmp (CMemory_ROMName, "HITOMI3") == 0)
    {
		CMemory_SRAMSize = 1;
		CMemory_SRAMMask = CMemory_SRAMSize ?
			((1 << (CMemory_SRAMSize + 3)) * 128) - 1 : 0;
    }

	//sram value fixes
    if (strcmp (CMemory_ROMName, "SUPER DRIFT OUT") == 0 ||
		strcmp(CMemory_ROMName, "SATAN IS OUR FATHER!") == 0 ||
		strcmp (CMemory_ROMName, "goemon 4") == 0)
		SNESGameFixes.SRAMInitialValue = 0x00;

#if 0
    if(strcmp (CMemory_ROMName, "XBAND JAPANESE MODEM") == 0)
    {
		for (c = 0x200; c < 0x400; c += 16)
		{
			for (int i = c; i < c + 16; i++)
			{
				CMemory_Map [i + 0x400] = CMemory_Map [i + 0xc00] = &ROM[c * 0x1000];
				CMemory_BlockIsRAM [i + 0x400] = CMemory_BlockIsRAM [i + 0xc00] = TRUE;
				CMemory_BlockIsROM [i + 0x400] = CMemory_BlockIsROM [i + 0xc00] = FALSE;
			}
		}
		CMemory_WriteProtectROM ();
    }
#endif
	
#define RomPatch(adr,ov,nv) \
	if (CMemory_ROM [adr] == ov) \
    CMemory_ROM [adr] = nv
	

    // Love Quest
    if (strcmp (CMemory_ROMName, "LOVE QUEST") == 0)
    {
		RomPatch (0x1385ec, 0xd0, 0xea);
		RomPatch (0x1385ed, 0xb2, 0xea);
    }
	//BNE D0 into nops
	
	//seems like the next instruction is a BRA
	//otherwise, this one's too complex for MKendora
    // Nangoku Syonen Papuwa Kun
    if (strcmp (CMemory_ROMName, "NANGOKUSYONEN PAPUWA") == 0)
		RomPatch (0x1f0d1, 0xa0, 0x6b);
	//turns an LDY into an RTL?
	
	//this is a cmp on $00:2140
    // Super Batter Up
    if (strcmp (CMemory_ROMName, "Super Batter Up") == 0)
    {
		RomPatch (0x27ae0, 0xd0, 0xea);
		RomPatch (0x27ae1, 0xfa, 0xea);
    }
	//BNE
}

// Read variable size MSB int from a file
static long ReadInt (FILE *f, unsigned nbytes)
{
    long v = 0;
    while (nbytes--)
    {
		int c = fgetc(f);
		if (c == EOF) 
			return -1;
		v = (v << 8) | (c & 0xFF);
    }
    return (v);
}

#define IPS_EOF 0x00454F46l

void CMemory_CheckForIPSPatch (const char *rom_filename, bool8 header, int32 *rom_size)
{
    char  dir [_MAX_DIR + 1];
    char  drive [_MAX_DRIVE + 1];
    char  name [_MAX_FNAME + 1];
    char  ext [_MAX_EXT + 1];
    char  fname [_MAX_PATH + 1];
    FILE  *patch_file  = NULL;
    long  offset = header ? 512 : 0;
	
    _splitpath (rom_filename, drive, dir, name, ext);
    _makepath (fname, drive, dir, name, "ips");
    
    if (!(patch_file = fopen (fname, "rb")))
    {
		if (!(patch_file = fopen (S9xGetFilename (".ips"), "rb")))
			return;
    }
	
    if (fread (fname, 1, 5, patch_file) != 5 ||
		strncmp (fname, "PATCH", 5) != 0)
    {
		fclose (patch_file);
		return;
    }
	
    int32 ofs;
	
    for (;;)
    {
		long len;
		long rlen;
		int  rchar;
		
		ofs = ReadInt (patch_file, 3);
		if (ofs == -1)
			goto err_eof;
		
		if (ofs == IPS_EOF) 
			break;
		
		ofs -= offset;
		
        len = ReadInt (patch_file, 2);
		if (len == -1)
			goto err_eof;
		
		/* Apply patch block */
		if (len)
		{
			if (ofs + len > CMemory_MAX_ROM_SIZE)
				goto err_eof;
			
			while (len--)
			{
				rchar = fgetc (patch_file);
				if (rchar == EOF) 
					goto err_eof;
				CMemory_ROM [ofs++] = (uint8) rchar;
            }
			if (ofs > *rom_size)
				*rom_size = ofs;
		}
		else
		{
			rlen = ReadInt (patch_file, 2);
			if (rlen == -1) 
				goto err_eof;
			
			rchar = fgetc (patch_file);
			if (rchar == EOF) 
				goto err_eof;
			
			if (ofs + rlen > CMemory_MAX_ROM_SIZE)
				goto err_eof;
			
			while (rlen--) 
				CMemory_ROM [ofs++] = (uint8) rchar;
			
			if (ofs > *rom_size)
				*rom_size = ofs;
		}
    }
	
    // Check if ROM image needs to be truncated
    ofs = ReadInt (patch_file, 3);
    if (ofs != -1 && ofs - offset < *rom_size)
    {
		// Need to truncate ROM image
		*rom_size = ofs - offset;
    }
    fclose (patch_file);
    return;
	
err_eof:
    if (patch_file) 
		fclose (patch_file);
}

static int is_bsx(unsigned char *p)
{
	unsigned c;
	
	if ( p[0x19] & 0x4f )
		goto notbsx;
	c = p[0x1a];
	if ( (c != 0x33) && (c != 0xff) ) // 0x33 = Manufacturer: Nintendo 
		goto notbsx;
	c = (p[0x17] << 8) | p[0x16];
	if ( (c != 0x0000) && (c != 0xffff) )
	{
		if ( (c & 0x040f) != 0 )
			goto notbsx;
		if ( (c & 0xff) > 0xc0 )
			goto notbsx;
	}
	c = p[0x18];
	if ( (c & 0xce) || ((c & 0x30)==0) )
		goto notbsx;
	if ( (p[0x15] & 0x03) != 0 )
		goto notbsx;
	c = p[0x13];
	if ( (c != 0x00) && (c != 0xff) )
		goto notbsx;
	if ( p[0x14] != 0x00 )
		goto notbsx;
	if ( bs_name(p) != 0 )
		goto notbsx;
	return 0; // It's a Satellaview ROM!
notbsx:
	return -1;
}
static int bs_name(unsigned char *p)
{
	unsigned c;
	int lcount;
	int numv; // number of valid name characters seen so far
	numv = 0; 
	for ( lcount = 16; lcount > 0; lcount-- )
	{
		if ( check_char( c = *p++ ) != 0 )
		{
			c = *p++;
			if ( c < 0x20 )
			{
				if ( (numv != 0x0b) || (c != 0) ) // Dr. Mario Hack
					goto notBsName;
			}
			
			numv++;
			lcount--;
			continue;
		}
		else
		{
			if ( c == 0 )
			{
				if ( numv == 0 )
					goto notBsName;
				continue;
			}
			
			if ( c < 0x20 )
				goto notBsName;
			if ( c >= 0x80 )
			{
				if ( (c < 0xa0) || ( c >= 0xf0 ) )
					goto notBsName;
			}
			numv++;
		}
	}
	if ( numv > 0 )
		return 0;
notBsName:
	return -1;
}
static int check_char(unsigned c)
{
	if ( ( c & 0x80 ) == 0 )
		return 0;
	if ( ( c - 0x20 ) & 0x40 )
		return 1;
	else
		return 0;
}

void CMemory_ParseSNESHeader(uint8* RomHeader)
{
		CMemory_SRAMSize = RomHeader [0x28];
		strncpy (CMemory_ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
		CMemory_ROMSpeed = RomHeader [0x25];
		CMemory_ROMType = RomHeader [0x26];
		CMemory_ROMSize = RomHeader [0x27];
		CMemory_ROMChecksum = RomHeader [0x2e] + (RomHeader [0x2f] << 8);
		CMemory_ROMComplementChecksum = RomHeader [0x2c] + (RomHeader [0x2d] << 8);
		CMemory_ROMRegion= RomHeader[0x29];
		memmove (CMemory_ROMId, &RomHeader [0x2], 4);
		if(RomHeader[0x2A]==0x33)
			memmove (CMemory_CompanyId, &RomHeader [0], 2);
		else sprintf(CMemory_CompanyId, "%02X", RomHeader[0x2A]);
}

#if defined(NO_INLINE_SET_GET) || defined(USE_MMU)
#define INLINE
#include "getset.h"
#endif

