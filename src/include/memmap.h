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

#ifndef _memmap_h_
#define _memmap_h_

#include "snes9x.h"
#include "snes4all.h"

#ifdef FAST_LSB_WORD_ACCESS
#define READ_WORD(s) (*(uint16 *) (s))
#define READ_DWORD(s) (*(uint32 *) (s))
#define WRITE_WORD(s, d) (*(uint16 *) (s)) = (d)
#define WRITE_DWORD(s, d) (*(uint32 *) (s)) = (d)

#define READ_3WORD(s) (0x00ffffff & *(uint32 *) (s))
#define WRITE_3WORD(s, d) *(uint16 *) (s) = (uint16)(d),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16)


#else
#define READ_WORD(s) ( *(uint8 *) (s) |\
		      (*((uint8 *) (s) + 1) << 8))
#define READ_DWORD(s) ( *(uint8 *) (s) |\
		       (*((uint8 *) (s) + 1) << 8) |\
		       (*((uint8 *) (s) + 2) << 16) |\
		       (*((uint8 *) (s) + 3) << 24))
#define WRITE_WORD(s, d) *(uint8 *) (s) = (d), \
                         *((uint8 *) (s) + 1) = (d) >> 8
#define WRITE_DWORD(s, d) *(uint8 *) (s) = (uint8) (d), \
                          *((uint8 *) (s) + 1) = (uint8) ((d) >> 8),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16),\
                          *((uint8 *) (s) + 3) = (uint8) ((d) >> 24)
#define WRITE_3WORD(s, d) *(uint8 *) (s) = (uint8) (d), \
                          *((uint8 *) (s) + 1) = (uint8) ((d) >> 8),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16)
#define READ_3WORD(s) ( *(uint8 *) (s) |\
                       (*((uint8 *) (s) + 1) << 8) |\
                       (*((uint8 *) (s) + 2) << 16))
#endif

#define MEMMAP_BLOCK_SIZE (0x1000)
#define MEMMAP_NUM_BLOCKS (0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_BLOCKS_PER_BANK (0x10000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT 12
#define MEMMAP_MASK (MEMMAP_BLOCK_SIZE - 1)
#define MEMMAP_MAX_SDD1_LOGGED_ENTRIES (0x10000 / 8)

//Extended ROM Formats
#define NOPE 0
#define YEAH 1
#define BIGFIRST 2
#define SMALLFIRST 3

//File Formats go here
enum file_formats { FILE_ZIP, FILE_RAR, FILE_JMA, FILE_DEFAULT };

bool8 CMemory_LoadROM (const char *);
uint32 CMemory_FileLoader (uint8* buffer, const char* filename, int32 maxsize);
void CMemory_InitROM (bool8);
bool8 CMemory_LoadSRAM (const char *);
bool8 CMemory_SaveSRAM (const char *);
bool8 CMemory_Init ();
void CMemory_Deinit ();
void CMemory_FreeSDD1Data ();
void CMemory_WriteProtectROM ();
void CMemory_FixROMSpeed ();
void CMemory_MapRAM ();
void CMemory_MapExtraRAM ();
char *CMemory_Safe (const char *);
void CMemory_BSLoROMMap();
void CMemory_JumboLoROMMap (bool8);
void CMemory_LoROMMap ();
void CMemory_LoROM24MBSMap ();
void CMemory_SRAM512KLoROMMap ();
//void CMemory_SRAM1024KLoROMMap ();
void CMemory_SufamiTurboLoROMMap ();
void CMemory_HiROMMap ();
void CMemory_SuperFXROMMap ();
void CMemory_TalesROMMap (bool8);
void CMemory_AlphaROMMap ();
void CMemory_SA1ROMMap ();
void CMemory_BSHiROMMap ();
void CMemory_SPC7110HiROMMap();
void CMemory_SPC7110Sram(uint8);
void CMemory_SetaDSPMap();
bool8 CMemory_AllASCII (uint8 *b, int size);
int CMemory_ScoreHiROM (bool8 skip_header, int32 offset);
int CMemory_ScoreLoROM (bool8 skip_header, int32 offset);
//void CMemory_SufamiTurboAltROMMap();
void CMemory_ApplyROMFixes ();
void CMemory_CheckForIPSPatch (const char *rom_filename, bool8 header, int32 *rom_size);
const char *CMemory_TVStandard ();
const char *CMemory_Speed ();
const char *CMemory_StaticRAMSize ();
const char *CMemory_MapType ();
const char *CMemory_MapMode ();
const char *CMemory_KartContents ();
const char *CMemory_Size ();
const char *CMemory_Headers ();
const char *CMemory_ROMID ();
const char *CMemory_CompanyID ();
void CMemory_ParseSNESHeader(uint8*);
enum {
	CMemory_MAP_PPU, CMemory_MAP_CPU, CMemory_MAP_DSP, CMemory_MAP_LOROM_SRAM, CMemory_MAP_HIROM_SRAM,
	CMemory_MAP_NONE, CMemory_MAP_DEBUG, CMemory_MAP_C4, CMemory_MAP_BWRAM, CMemory_MAP_BWRAM_BITMAP,
	CMemory_MAP_BWRAM_BITMAP2, CMemory_MAP_SA1RAM, CMemory_MAP_SPC7110_ROM, CMemory_MAP_SPC7110_DRAM,
	CMemory_MAP_RONLY_SRAM, CMemory_MAP_OBC_RAM, CMemory_MAP_SETA_DSP, CMemory_MAP_SETA_RISC, CMemory_MAP_LAST
    };

#define CMemory_MAX_ROM_SIZE SNES4ALL_MAX_ROM_SIZE

#ifdef __cplusplus
extern "C"{
#endif
#ifdef USE_MEMORY_POINTERS
extern uint8 *CMemory_RAM;
extern uint8 *CMemory_ROM;
extern uint8 *CMemory_VRAM;
extern uint8 *CMemory_SRAM;
extern uint8 *CMemory_FillRAM;
extern uint8 *CMemory_C4RAM;
#else
extern uint8 CMemory_RAM[0x20000];
extern uint8 CMemory_VRAM[0x10000];
extern uint8 CMemory_SRAM[0x20000];
extern uint8 CMemory_FillRAM[SNES4ALL_MAX_ROM_SIZE + 0x200 +0x8000];
//extern uint8 CMemory_FillRAM[SNES4ALL_MAX_ROM_SIZE + 0x200 +0x8000] __asm__ ("0x00000000");
#if defined(DREAMCAST) || defined(WIN32)
extern uint8 CMemory_ROM[SNES4ALL_MAX_ROM_SIZE + 0x200] __asm__ ("_CMemory_FillRAM + 0x8000");
extern uint8 CMemory_C4RAM[SNES4ALL_MAX_ROM_SIZE + 0x200 + 0x8000 - 0x400000 - 0x10000] __asm__ ("_CMemory_FillRAM + 0x8000 + 0x400000 + 0x10000");
//extern uint8 CMemory_ROM[SNES4ALL_MAX_ROM_SIZE + 0x200] __asm__ ("0x8000");
//extern uint8 CMemory_C4RAM[SNES4ALL_MAX_ROM_SIZE + 0x200 + 0x8000 - 0x400000 - 0x10000] __asm__ ("0x8000 + 0x400000 + 0x10000");
#else
extern uint8 CMemory_ROM[SNES4ALL_MAX_ROM_SIZE + 0x200] __asm__ ("CMemory_FillRAM + 0x8000");
extern uint8 CMemory_C4RAM[SNES4ALL_MAX_ROM_SIZE + 0x200 + 0x8000 - 0x400000 - 0x10000] __asm__ ("CMemory_FillRAM + 0x8000 + 0x400000 + 0x10000");
#endif
#endif
extern uint8 *CMemory_BWRAM;
extern bool8 CMemory_HiROM;
extern bool8 CMemory_LoROM;
extern uint32 CMemory_SRAMMask;
extern uint8 CMemory_SRAMSize;
extern uint8 *CMemory_Map [MEMMAP_NUM_BLOCKS];
extern uint8 *CMemory_WriteMap [MEMMAP_NUM_BLOCKS];
extern uint8 CMemory_MemorySpeed [MEMMAP_NUM_BLOCKS];
extern uint8 CMemory_BlockIsRAM [MEMMAP_NUM_BLOCKS];
extern uint8 CMemory_BlockIsROM [MEMMAP_NUM_BLOCKS];
extern char  CMemory_ROMName [ROM_NAME_LEN];
extern char  CMemory_ROMId [5];
extern char  CMemory_CompanyId [3];
extern uint8 CMemory_ROMSpeed;
extern uint8 CMemory_ROMType;
extern uint8 CMemory_ROMSize;
extern int32 CMemory_ROMFramesPerSecond;
extern int32 CMemory_HeaderCount;
extern uint32 CMemory_CalculatedSize;
extern uint32 CMemory_CalculatedChecksum;
extern uint32 CMemory_ROMChecksum;
extern uint32 CMemory_ROMComplementChecksum;
extern uint8 *CMemory_SDD1Index;
extern uint8 *CMemory_SDD1Data;
extern uint32 CMemory_SDD1Entries;
extern uint32 CMemory_SDD1LoggedDataCountPrev;
extern uint32 CMemory_SDD1LoggedDataCount;
extern uint8  CMemory_SDD1LoggedData [MEMMAP_MAX_SDD1_LOGGED_ENTRIES];
extern char CMemory_ROMFilename [_MAX_PATH];
extern uint8 CMemory_ROMRegion;
extern uint32 CMemory_ROMCRC32;
extern uint8 CMemory_ExtendedFormat;
#if 0
extern bool8 CMemory_SufamiTurbo;
extern char CMemory_Slot1Filename [_MAX_PATH];
extern char CMemory_Slot2Filename [_MAX_PATH];
extern uint8* CMemory_ROMOffset1;
extern uint8* CMemory_ROMOffset2;
extern uint8* CMemory_SRAMOffset1;
extern uint8* CMemory_SRAMOffset2;
extern uint32 CMemory_Slot1Size;
extern uint32 CMemory_Slot2Size;
extern uint32 CMemory_Slot1SRAMSize;
extern uint32 CMemory_Slot2SRAMSize;
extern uint8 CMemory_SlotContents;
#endif
extern uint8 CMemory_BSRAM[0x80000];
#ifdef __cplusplus
}
#endif

void CMemory_ResetSpeedMap();
#if 0
bool8 CMemory_LoadMulti (const char *,const char *,const char *);
#endif

//#define SRAM CMemory_SRAM
//#define ROM CMemory_ROM
//#define ROM CMemory_ROM
//#define RegRAM CMemory_FillRAM
START_EXTERN_C
void S9xDeinterleaveMode2 ();
bool8 LoadZip(const char* zipname,
	      int32 *TotalFileSize,
	      int32 *headers, 
              uint8 *buffer);
END_EXTERN_C

void S9xAutoSaveSRAM ();

#if defined(NO_INLINE_SET_GET) || defined(USE_MMU)
uint8 S9xGetByte (uint32 Address);
uint16 S9xGetWord (uint32 Address);
void S9xSetByte (uint8 Byte, uint32 Address);
void S9xSetWord (uint16 Byte, uint32 Address);
void S9xSetPCBase (uint32 Address);
uint8 *S9xGetMemPointer (uint32 Address);
uint8 *GetBasePointer (uint32 Address);

#ifdef __cplusplus
extern "C"{
#endif
extern uint8 OpenBus;
#ifdef __cplusplus
}
#endif
#else
#define INLINE static __inline__
#include "getset.h"
#endif // NO_INLINE_SET_GET

#endif // _memmap_h_

