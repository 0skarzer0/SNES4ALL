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

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "missing.h"
#include "apu.h"
#include "dma.h"
#include "gfx.h"
#include "display.h"
#include "sa1.h"
#include "netplay.h"
#include "sdd1.h"
#include "srtc.h"
#include "spc7110.h"
#include "movie.h"

#ifndef ZSNES_FX
#include "fxemu.h"
#include "fxinst.h"
extern struct FxInit_s SuperFX;
#else
EXTERN_C void S9xSuperFXWriteReg (uint8, uint32);
EXTERN_C uint8 S9xSuperFXReadReg (uint32);
#endif

uint32 justifiers=0xFFFF00AA;
uint8 in_bit=0;

extern uint8 *HDMAMemPointers [8];

static __inline__ void S9xLatchCounters (bool force)
{
	if(!force && !(CMemory_FillRAM[0x4213] & 0x80)) return;

	// Latch h and v counters, like the gun
#if 0
# ifdef CPU_SHUTDOWN
	CPU.WaitAddress = CPU.PCAtOpcodeStart;
# endif
#endif
	PPU.HVBeamCounterLatched = 1;
	PPU.VBeamPosLatched = (uint16) CPU.V_Counter;
	PPU.HBeamPosLatched = (uint16) ((CPU.Cycles * SNES_HCOUNTER_MAX) / Settings.H_Max);

	// Causes screen flicker for Yoshi's Island if uncommented
	//CLEAR_IRQ_SOURCE (PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE);

	CMemory_FillRAM [0x213F] |= 0x40;
}

void S9xUpdateJustifiers();

void S9xUpdateHTimer ()
{
	if (PPU.HTimerEnabled)
	{
		PPU.HTimerPosition = PPU.IRQHBeamPos * Settings.H_Max / SNES_HCOUNTER_MAX;
		if (PPU.HTimerPosition == Settings.H_Max ||
			PPU.HTimerPosition == Settings.HBlankStart)
		{
			PPU.HTimerPosition--;
		}

		if (!PPU.VTimerEnabled || CPU.V_Counter == PPU.IRQVBeamPos)
		{
			if (PPU.HTimerPosition < CPU.Cycles)
			{
				// Missed the IRQ on this line already
				if (CPU.WhichEvent == HBLANK_END_EVENT ||
					CPU.WhichEvent == HTIMER_AFTER_EVENT)
				{
					CPU.WhichEvent = HBLANK_END_EVENT;
					CPU.NextEvent = Settings.H_Max;
				}
				else
				{
					CPU.WhichEvent = HBLANK_START_EVENT;
					CPU.NextEvent = Settings.HBlankStart;
				}
			}
			else
			{
				if (CPU.WhichEvent == HTIMER_BEFORE_EVENT ||
					CPU.WhichEvent == HBLANK_START_EVENT)
				{
					if (PPU.HTimerPosition > Settings.HBlankStart)
					{
						// HTimer was to trigger before h-blank start,
						// now triggers after start of h-blank
						CPU.NextEvent = Settings.HBlankStart;
						CPU.WhichEvent = HBLANK_START_EVENT;
					}
					else
					{
						CPU.NextEvent = PPU.HTimerPosition;
						CPU.WhichEvent = HTIMER_BEFORE_EVENT;
					}
				}
				else
				{
					CPU.WhichEvent = HTIMER_AFTER_EVENT;
					CPU.NextEvent = PPU.HTimerPosition;
				}
			}
		}
	}
}

void S9xFixColourBrightness()
{
//printf("FixColourBrightness %i, max=%i\n",PPU.Brightness,IPPU.MaxBrightness);
//static unsigned back=512;
//printf("RESET por cambiar de %i a %i\n",back,PPU.Brightness);
//back=PPU.Brightness;
	IPPU.XB = mul_brightness [PPU.Brightness];
	for (register int i = 0; i < 256; i++)
	{
		IPPU.Red [i] = IPPU.XB [PPU.CGDATA [i] & 0x1f];
		IPPU.Green [i] = IPPU.XB [(PPU.CGDATA [i] >> 5) & 0x1f];
		IPPU.Blue [i] = IPPU.XB [(PPU.CGDATA [i] >> 10) & 0x1f];
		IPPU.ScreenColors [i] = BUILD_PIXEL (IPPU.Red [i], IPPU.Green [i], IPPU.Blue [i]);
	}
}

#ifdef USE_GL
static __inline__ void S9xFixColourBrightnessCond() {
//printf("FixColourBrightnessCond %i, max=%i\n",PPU.Brightness,IPPU.MaxBrightness);
	extern int videogl_hw_render;
	if (!videogl_hw_render)
		S9xFixColourBrightness();
}
#else
#define S9xFixColourBrightnessCond() S9xFixColourBrightness()
#endif


/******************************************************************************/
/* S9xSetPPU()                                                                */
/* This function sets a PPU Register to a specific byte                       */
/******************************************************************************/
#ifdef DIRECT_COLOR_MAP
#define SETPPU_BRIGHTNESS(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2100]) { \
		FLUSH_REDRAW (); \
		if (PPU.Brightness != ((DAT) & 0xF)) { \
			/*IPPU.ColorsChanged = TRUE;*/ \
			IPPU.DirectColourMapsNeedRebuild = TRUE; \
			PPU.Brightness = (DAT) & 0xF; \
			S9xFixColourBrightnessCond(); \
			if (PPU.Brightness > IPPU.MaxBrightness) \
				IPPU.MaxBrightness = PPU.Brightness; \
		} \
		if ((CMemory_FillRAM[0x2100] & 0x80) != ((DAT) & 0x80)) { \
			/*IPPU.ColorsChanged = TRUE;*/ \
			PPU.ForcedBlanking = ((DAT) >> 7) & 1; \
		} \
		CMemory_FillRAM[0x2100] = (DAT); \
	}
#else
#define SETPPU_BRIGHTNESS(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2100]) { \
		FLUSH_REDRAW (); \
		if (PPU.Brightness != ((DAT) & 0xF)) { \
			/*IPPU.ColorsChanged = TRUE;*/ \
			PPU.Brightness = (DAT) & 0xF; \
			S9xFixColourBrightnessCond(); \
			if (PPU.Brightness > IPPU.MaxBrightness) \
				IPPU.MaxBrightness = PPU.Brightness; \
		} \
		if ((CMemory_FillRAM[0x2100] & 0x80) != ((DAT) & 0x80)) { \
			/*IPPU.ColorsChanged = TRUE;*/ \
			PPU.ForcedBlanking = ((DAT) >> 7) & 1; \
		} \
		CMemory_FillRAM[0x2100] = (DAT); \
	}
#endif

#define SETPPU_OBJ(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2101]) { \
		FLUSH_REDRAW (); \
		PPU.OBJNameBase   = ((DAT) & 3) << 14; \
		PPU.OBJNameSelect = (((DAT) >> 3) & 3) << 13; \
		PPU.OBJSizeSelect = ((DAT) >> 5) & 7; \
		IPPU.OBJChanged = TRUE; \
		CMemory_FillRAM[0x2101] = (DAT); \
	} 

#define SETPPU_SPR_WRITE_LOW(DAT) \
	PPU.OAMAddr = ((CMemory_FillRAM[0x2103]&1)<<8) | (DAT); \
	PPU.OAMFlip = 2;\
	PPU.OAMReadFlip = 0;\
	PPU.SavedOAMAddr = PPU.OAMAddr;\
	if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1)) { \
		PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1; \
		IPPU.OBJChanged = TRUE; \
	} \
	CMemory_FillRAM[0x2102] = (DAT);

#define SETPPU_SPR_WRITE_HIGH(DAT) \
	PPU.OAMAddr = (((DAT)&1)<<8) | CMemory_FillRAM[0x2102]; \
	PPU.OAMPriorityRotation=((DAT) & 0x80)? 1 : 0; \
	if (PPU.OAMPriorityRotation) { \
		if (PPU.FirstSprite != (PPU.OAMAddr >> 1)) { \
			PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1; \
			IPPU.OBJChanged = TRUE; \
		} \
	} else { \
		if (PPU.FirstSprite != 0) { \
			PPU.FirstSprite = 0; \
			IPPU.OBJChanged = TRUE; \
		} \
	} \
	PPU.OAMFlip = 0; \
	PPU.OAMReadFlip = 0; \
	PPU.SavedOAMAddr = PPU.OAMAddr; \
	CMemory_FillRAM[0x2103] = (DAT);

#define SETPPU_SPR_REG_WRITE(DAT) \
	REGISTER_2104((DAT)); \
	CMemory_FillRAM[0x2104] = (DAT); 

#define SETPPU_SCREEN_MODE(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2105]) { \
		FLUSH_REDRAW (); \
		PPU.BG[0].BGSize = ((DAT) >> 4) & 1; \
		PPU.BG[1].BGSize = ((DAT) >> 5) & 1; \
		PPU.BG[2].BGSize = ((DAT) >> 6) & 1; \
		PPU.BG[3].BGSize = ((DAT) >> 7) & 1; \
		PPU.BGMode = (DAT) & 7; \
		PPU.BG3Priority  = (((DAT) & 0x0f) == 0x09); \
		if(PPU.BGMode==5||PPU.BGMode==6) \
			IPPU.Interlace = CMemory_FillRAM[0x2133]&1; \
		CMemory_FillRAM[0x2105] = (DAT); \
	}

#define SETPPU_MOSAIC_PIXEL(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2106]) { \
		FLUSH_REDRAW (); \
		PPU.Mosaic = ((DAT) >> 4) + 1; \
		PPU.BGMosaic [0] = ((DAT) & 1) && PPU.Mosaic > 1; \
		PPU.BGMosaic [1] = ((DAT) & 2) && PPU.Mosaic > 1; \
		PPU.BGMosaic [2] = ((DAT) & 4) && PPU.Mosaic > 1; \
		PPU.BGMosaic [3] = ((DAT) & 8) && PPU.Mosaic > 1; \
		CMemory_FillRAM[0x2106] = (DAT); \
	} 

#define SETPPU_BG0SC(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2107]) { \
		FLUSH_REDRAW (); \
		PPU.BG[0].SCSize = (DAT) & 3; \
		PPU.BG[0].SCBase = ((DAT) & 0x7c) << 8; \
		CMemory_FillRAM[0x2107] = (DAT); \
	}

#define SETPPU_BG1SC(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2108]) { \
		FLUSH_REDRAW (); \
		PPU.BG[1].SCSize = (DAT) & 3; \
		PPU.BG[1].SCBase = ((DAT) & 0x7c) << 8; \
		CMemory_FillRAM[0x2108] = (DAT); \
	}

#define SETPPU_BG2SC(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2109]) { \
		FLUSH_REDRAW (); \
		PPU.BG[2].SCSize = (DAT) & 3; \
		PPU.BG[2].SCBase = ((DAT) & 0x7c) << 8; \
		CMemory_FillRAM[0x2109] = (DAT); \
	}

#define SETPPU_BG3SC(DAT) \
	if ((DAT) != CMemory_FillRAM [0x210a]) { \
		FLUSH_REDRAW (); \
		PPU.BG[3].SCSize = (DAT) & 3; \
		PPU.BG[3].SCBase = ((DAT) & 0x7c) << 8; \
		CMemory_FillRAM[0x210a] = (DAT); \
	}

#define SETPPU_BG01NBA(DAT) \
	if ((DAT) != CMemory_FillRAM [0x210b]) { \
		FLUSH_REDRAW (); \
		PPU.BG[0].NameBase = ((DAT) & 7) << 12; \
		PPU.BG[1].NameBase = (((DAT) >> 4) & 7) << 12; \
		CMemory_FillRAM[0x210b] = (DAT); \
	}

#define SETPPU_BG23NBA(DAT) \
	if ((DAT) != CMemory_FillRAM [0x210c]) { \
		FLUSH_REDRAW (); \
		PPU.BG[2].NameBase = ((DAT) & 7) << 12; \
		PPU.BG[3].NameBase = (((DAT) >> 4) & 7) << 12; \
		CMemory_FillRAM[0x210c] = (DAT); \
	}

#define SETPPU_210D(DAT) \
	PPU.BG[0].HOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x210d] = (DAT);

#define SETPPU_210E(DAT) \
	PPU.BG[0].VOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x210e] = (DAT);

#define SETPPU_210F(DAT) \
	PPU.BG[1].HOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x210f] = (DAT);

#define SETPPU_2110(DAT) \
	PPU.BG[1].VOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x2110] = (DAT);

#define SETPPU_2111(DAT) \
	PPU.BG[2].HOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x2111] = (DAT);

#define SETPPU_2112(DAT) \
	PPU.BG[2].VOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x2112] = (DAT);

#define SETPPU_2113(DAT) \
	PPU.BG[3].HOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x2113] = (DAT);

#define SETPPU_2114(DAT) \
	PPU.BG[3].VOffset = ((DAT)<<8) | PPU.BGnxOFSbyte; \
	PPU.BGnxOFSbyte = (DAT); \
	CMemory_FillRAM[0x2114] = (DAT);

#define SETPPU_VRAM_ACCESS(DAT) \
	PPU.VMA.High = ((DAT) & 0x80) == 0 ? FALSE : TRUE; \
	switch ((DAT) & 3) { \
	  case 0: PPU.VMA.Increment = 1; break; \
	  case 1: PPU.VMA.Increment = 32; break; \
	  case 2: PPU.VMA.Increment = 128; break; \
	  case 3: PPU.VMA.Increment = 128; break; \
	} \
	if ((DAT) & 0x0c) { \
		static uint16 IncCount [4] = { 0, 32, 64, 128 }; \
		static uint16 Shift [4] = { 0, 5, 6, 7 }; \
		/*PPU.VMA.Increment = 1;*/ \
		uint8 i = ((DAT) & 0x0c) >> 2; \
		PPU.VMA.FullGraphicCount = IncCount [i]; \
		PPU.VMA.Mask1 = IncCount [i] * 8 - 1; \
		PPU.VMA.Shift = Shift [i]; \
	} else \
		PPU.VMA.FullGraphicCount = 0; \
	CMemory_FillRAM[0x2115] = (DAT);

#ifdef CORRECT_VRAM_READS
#define SETPPU_VRAM_ADDR_LOW(DAT) \
	PPU.VMA.Address &= 0xFF00; \
	PPU.VMA.Address |= (DAT); \
	if (PPU.VMA.FullGraphicCount) { \
		uint32 addr = PPU.VMA.Address; \
		uint32 rem = addr & PPU.VMA.Mask1; \
		uint32 address = (addr & ~PPU.VMA.Mask1) + \
			(rem >> PPU.VMA.Shift) + \
			((rem & (PPU.VMA.FullGraphicCount - 1)) << 3); \
		IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((address << 1) & 0xFFFF)); \
	} else \
		IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((PPU.VMA.Address << 1) & 0xffff)); \
	CMemory_FillRAM[0x2116] = (DAT);
#else
#define SETPPU_VRAM_ADDR_LOW(DAT) \
	PPU.VMA.Address &= 0xFF00; \
	PPU.VMA.Address |= (DAT); \
	IPPU.FirstVRAMRead = TRUE; \
	CMemory_FillRAM[0x2116] = (DAT);
#endif

#ifdef CORRECT_VRAM_READS
#define SETPPU_VRAM_ADDR_HIGH(DAT) \
	PPU.VMA.Address &= 0x00FF; \
	PPU.VMA.Address |= (DAT) << 8; \
	if (PPU.VMA.FullGraphicCount) { \
		uint32 addr = PPU.VMA.Address; \
		uint32 rem = addr & PPU.VMA.Mask1; \
		uint32 address = (addr & ~PPU.VMA.Mask1) + \
			(rem >> PPU.VMA.Shift) + \
			((rem & (PPU.VMA.FullGraphicCount - 1)) << 3); \
		IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((address << 1) & 0xFFFF)); \
	} else \
		IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((PPU.VMA.Address << 1) & 0xffff)); \
	CMemory_FillRAM[0x2117] = (DAT);
#else
#define SETPPU_VRAM_ADDR_HIGH(DAT) \
	PPU.VMA.Address &= 0x00FF; \
	PPU.VMA.Address |= (DAT) << 8; \
	IPPU.FirstVRAMRead = TRUE; \
	CMemory_FillRAM[0x2117] = (DAT);
#endif

#ifndef CORRECT_VRAM_READS
#define SETPPU_VRAM_WRITE_LOW(DAT) \
	IPPU.FirstVRAMRead = TRUE; \
	REGISTER_2118((DAT)); \
	CMemory_FillRAM[0x2118] = (DAT);
#else
#define SETPPU_VRAM_WRITE_LOW(DAT) \
	REGISTER_2118((DAT)); \
	CMemory_FillRAM[0x2118] = (DAT);
#endif

#ifndef CORRECT_VRAM_READS
#define SETPPU_VRAM_WRITE_HIGH(DAT) \
	IPPU.FirstVRAMRead = TRUE; \
	REGISTER_2119((DAT)); \
	CMemory_FillRAM[0x2119] = (DAT);
#else
#define SETPPU_VRAM_WRITE_HIGH(DAT) \
	REGISTER_2119((DAT)); \
	CMemory_FillRAM[0x2119] = (DAT);
#endif

#define SETPPU_MODE7_ROT(DAT) \
	if ((DAT) != CMemory_FillRAM [0x211a]) { \
		FLUSH_REDRAW (); \
		PPU.Mode7Repeat = (DAT) >> 6; \
		if (PPU.Mode7Repeat == 1) \
			PPU.Mode7Repeat = 0; \
		PPU.Mode7VFlip = ((DAT) & 2) >> 1; \
		PPU.Mode7HFlip = (DAT) & 1; \
		CMemory_FillRAM[0x211a] = (DAT); \
	}

#define SETPPU_MODE7_MATRIX_A(DAT) \
	PPU.MatrixA = ((PPU.MatrixA >> 8) & 0xff) | ((DAT) << 8); \
	PPU.Need16x8Mulitply = TRUE; \
	CMemory_FillRAM[0x211b] = (DAT);

#define SETPPU_MODE7_MATRIX_B(DAT) \
	PPU.MatrixB = ((PPU.MatrixB >> 8) & 0xff) | ((DAT) << 8); \
	PPU.Need16x8Mulitply = TRUE; \
	CMemory_FillRAM[0x211c] = (DAT);

#define SETPPU_MODE7_MATRIX_C(DAT) \
	PPU.MatrixC = ((PPU.MatrixC >> 8) & 0xff) | ((DAT) << 8); \
	CMemory_FillRAM[0x211d] = (DAT);

#define SETPPU_MODE7_MATRIX_D(DAT) \
	PPU.MatrixD = ((PPU.MatrixD >> 8) & 0xff) | ((DAT) << 8); \
	CMemory_FillRAM[0x211e] = (DAT);

#define SETPPU_MODE7_CENTRE_X(DAT) \
	PPU.CentreX = ((PPU.CentreX >> 8) & 0xff) | ((DAT) << 8); \
	CMemory_FillRAM[0x211f] = (DAT);

#define SETPPU_MODE7_CENTRE_Y(DAT) \
	PPU.CentreY = ((PPU.CentreY >> 8) & 0xff) | ((DAT) << 8); \
	CMemory_FillRAM[0x2120] = (DAT);

#define SETPPU_CGRAM_ADDR(DAT) \
	PPU.CGFLIP = 0; \
	PPU.CGFLIPRead = 0; \
	PPU.CGADD = (DAT); \
	CMemory_FillRAM[0x2121] = (DAT);

#define SETPPU_2122(DAT) \
	REGISTER_2122((DAT)); \
	CMemory_FillRAM[0x2122] = (DAT);

#define SETPPU_ENABLE_WIN_12(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2123]) { \
		FLUSH_REDRAW (); \
		PPU.ClipWindow1Enable [0] = !!((DAT) & 0x02); \
		PPU.ClipWindow1Enable [1] = !!((DAT) & 0x20); \
		PPU.ClipWindow2Enable [0] = !!((DAT) & 0x08); \
		PPU.ClipWindow2Enable [1] = !!((DAT) & 0x80); \
		PPU.ClipWindow1Inside [0] = !((DAT) & 0x01); \
		PPU.ClipWindow1Inside [1] = !((DAT) & 0x10); \
		PPU.ClipWindow2Inside [0] = !((DAT) & 0x04); \
		PPU.ClipWindow2Inside [1] = !((DAT) & 0x40); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2123] = (DAT); \
	}

#define SETPPU_ENABLE_WIN_34(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2124]) { \
		FLUSH_REDRAW (); \
		PPU.ClipWindow1Enable [2] = !!((DAT) & 0x02); \
		PPU.ClipWindow1Enable [3] = !!((DAT) & 0x20); \
		PPU.ClipWindow2Enable [2] = !!((DAT) & 0x08); \
		PPU.ClipWindow2Enable [3] = !!((DAT) & 0x80); \
		PPU.ClipWindow1Inside [2] = !((DAT) & 0x01); \
		PPU.ClipWindow1Inside [3] = !((DAT) & 0x10); \
		PPU.ClipWindow2Inside [2] = !((DAT) & 0x04); \
		PPU.ClipWindow2Inside [3] = !((DAT) & 0x40); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2124] = (DAT); \
	}

#define SETPPU_ENABLE_WIN_OBJ(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2125]) { \
		FLUSH_REDRAW (); \
		PPU.ClipWindow1Enable [4] = !!((DAT) & 0x02); \
		PPU.ClipWindow1Enable [5] = !!((DAT) & 0x20); \
		PPU.ClipWindow2Enable [4] = !!((DAT) & 0x08); \
		PPU.ClipWindow2Enable [5] = !!((DAT) & 0x80); \
		PPU.ClipWindow1Inside [4] = !((DAT) & 0x01); \
		PPU.ClipWindow1Inside [5] = !((DAT) & 0x10); \
		PPU.ClipWindow2Inside [4] = !((DAT) & 0x04); \
		PPU.ClipWindow2Inside [5] = !((DAT) & 0x40); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2125] = (DAT); \
	}

#define SETPPU_WIN_1_LEFT(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2126]) { \
		FLUSH_REDRAW (); \
		PPU.Window1Left = (DAT); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2126] = (DAT); \
	}

#define SETPPU_WIN_1_RIGHT(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2127]) { \
		FLUSH_REDRAW (); \
		PPU.Window1Right = (DAT); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2127] = (DAT); \
	}

#define SETPPU_WIN_2_LEFT(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2128]) { \
		FLUSH_REDRAW (); \
		PPU.Window2Left = (DAT); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2128] = (DAT); \
	}

#define SETPPU_WIN_2_RIGHT(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2129]) { \
		FLUSH_REDRAW (); \
		PPU.Window2Right = (DAT); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2129] = (DAT); \
	}

#define SETPPU_WIN_OVERLAP_14(DAT) \
	if ((DAT) != CMemory_FillRAM [0x212a]) { \
		FLUSH_REDRAW (); \
		PPU.ClipWindowOverlapLogic [0] = ((DAT) & 0x03); \
		PPU.ClipWindowOverlapLogic [1] = ((DAT) & 0x0c) >> 2; \
		PPU.ClipWindowOverlapLogic [2] = ((DAT) & 0x30) >> 4; \
		PPU.ClipWindowOverlapLogic [3] = ((DAT) & 0xc0) >> 6; \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x212a] = (DAT); \
	}

#define SETPPU_WIN_OVERLAP_OBJ(DAT) \
	if ((DAT) != CMemory_FillRAM [0x212b]) { \
		FLUSH_REDRAW (); \
		PPU.ClipWindowOverlapLogic [4] = (DAT) & 0x03; \
		PPU.ClipWindowOverlapLogic [5] = ((DAT) & 0x0c) >> 2; \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x212a] = (DAT); \
	}

#define SETPPU_MAIN_SCREEN_DEST(DAT) \
	if ((DAT) != CMemory_FillRAM [0x212c]) { \
		FLUSH_REDRAW (); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x212c] = (DAT); \
	}

#define SETPPU_SUB_SCREEN_DEST(DAT) \
	if ((DAT) != CMemory_FillRAM [0x212d]) { \
		FLUSH_REDRAW (); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x212d] = (DAT); \
	}

#define SETPPU_MAIN_SCREEN_MASK(DAT) \
	if ((DAT) != CMemory_FillRAM [0x212e]) { \
		FLUSH_REDRAW (); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x212e] = (DAT); \
	}

#define SETPPU_SUB_SCREEN_MASK(DAT) \
	if ((DAT) != CMemory_FillRAM [0x212f]) { \
		FLUSH_REDRAW (); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x212f] = (DAT); \
	}

#define SETPPU_FIXED_ADD(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2130]) { \
		FLUSH_REDRAW (); \
		PPU.RecomputeClipWindows = TRUE; \
		CMemory_FillRAM[0x2130] = (DAT); \
	}

#define SETPPU_COL_ADD(DAT) \
	if ((DAT) != CMemory_FillRAM[0x2131]) { \
		FLUSH_REDRAW (); \
		CMemory_FillRAM[0x2131] = (DAT); \
	}

#define SETPPU_FIXED_SUB(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2132]) { \
		FLUSH_REDRAW (); \
		if ((DAT) & 0x80) \
			PPU.FixedColourBlue = (DAT) & 0x1f; \
		if ((DAT) & 0x40) \
			PPU.FixedColourGreen = (DAT) & 0x1f; \
		if ((DAT) & 0x20) \
			PPU.FixedColourRed = (DAT) & 0x1f; \
		CMemory_FillRAM[0x2132] = (DAT); \
	}

#define SETPPU_SCREEN_SETS(DAT) \
	if ((DAT) != CMemory_FillRAM [0x2133]) { \
		if ((DAT) & 0x04) { \
			PPU.ScreenHeight = SNES_HEIGHT_EXTENDED; \
			IPPU.RenderedScreenHeight = PPU.ScreenHeight; \
		} \
		else PPU.ScreenHeight = SNES_HEIGHT; \
		/*if(((DAT) & 1)&&(PPU.BGMode==5||PPU.BGMode==6))*/ \
		/*IPPU.Interlace=1;*/ \
		if((CMemory_FillRAM [0x2133] ^ (DAT))&3) { \
			FLUSH_REDRAW (); \
			if((CMemory_FillRAM [0x2133] ^ (DAT))&2) \
				IPPU.OBJChanged = TRUE; \
                        if(PPU.BGMode==5||PPU.BGMode==6) \
                                IPPU.Interlace = (DAT)&1; \
			IPPU.InterlaceSprites=0; \
			/*IPPU.InterlaceSprites = ((DAT)&2)>>1;*/ \
		} \
		CMemory_FillRAM[0x2133] = (DAT); \
	}

#define SETPPU_NULL() 

#ifdef SPCTOOL
#define SETPPU_SPC700(ADDR,DAT) \
	_SPCInPB ((ADDR) & 3, (DAT)); \
	CMemory_FillRAM [(ADDR)] = (DAT);
#else	
#ifdef SPC700_SHUTDOWN
#define SETPPU_SPC700_COMMON(ADDR,DAT) \
	/*CPU.Flags |= DEBUG_MODE_FLAG;*/ \
	CMemory_FillRAM [(ADDR)] = (DAT); \
	IAPU.RAM [((ADDR) & 3) + 0xf4] = (DAT); \
	IAPU.APUExecuting = Settings.APUEnabled; \
	IAPU.WaitCounter++;
#else
#define SETPPU_SPC700_COMMON(ADDR,DAT) \
	/*CPU.Flags |= DEBUG_MODE_FLAG;*/ \
	CMemory_FillRAM [(ADDR)] = (DAT); \
	IAPU.RAM [((ADDR) & 3) + 0xf4] = (DAT);
#endif
#ifdef USE_ALWAYS_APU_SYNC
#define SETPPU_SPC700(ADDR,DAT) SETPPU_SPC700_COMMON(ADDR,DAT)
#else
#define SETPPU_SPC700(ADDR,DAT) \
	if (snes4all_sound_enable<=1) \
		S9xApuExecute(); \
	SETPPU_SPC700_COMMON(ADDR,DAT) 
#endif
#endif

#define SETPPU_2180(DAT) \
	REGISTER_2180((DAT)); \
	CMemory_FillRAM[0x2180] = (DAT);

#define SETPPU_2181(DAT) \
	PPU.WRAM &= 0x1FF00; \
	PPU.WRAM |= (DAT); \
	CMemory_FillRAM[0x2181] = (DAT);

#define SETPPU_2182(DAT) \
	PPU.WRAM &= 0x100FF; \
	PPU.WRAM |= (DAT) << 8; \
	CMemory_FillRAM[0x2182] = (DAT);

#define SETPPU_2183(DAT) \
	PPU.WRAM &= 0x0FFFF; \
	PPU.WRAM |= (DAT) << 16; \
	PPU.WRAM &= 0x1FFFF; \
	CMemory_FillRAM[0x2183] = (DAT);

#define SETPPU_SA1(ADDR,DAT) \
	if (Settings.SA1) { \
		if ((ADDR) >= 0x2200 && (ADDR) <0x23ff) \
			S9xSetSA1 ((DAT), (ADDR)); \
		else \
			CMemory_FillRAM [(ADDR)] = (DAT); \
		return; \
	}

#define SETPPU_EMPTY(ADDR,DAT) \
	CMemory_FillRAM [(ADDR)] = (DAT);

#define SETPPU_2801(DAT) \
	if (Settings.SRTC) \
		S9xSetSRTC ((DAT), 0x2801); \
	CMemory_FillRAM [0x2801] = (DAT);

#define SETPPU_SUPERFX() \
	if (!Settings.SuperFX) \
		return;

#ifdef ZSNES_FX
#define SETPPU_3030(DAT) \
	CMemory_FillRAM [0x3030] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3030);
#else
#define SETPPU_3030(DAT) \
	if ((CMemory_FillRAM [0x3030] ^ (DAT)) & FLG_G) { \
		CMemory_FillRAM [0x3030] = (DAT); \
		if ((DAT) & FLG_G) \
    			if (Settings.SuperFX) { \
				S9xSuperFXExec (); \
			} \
		else \
			FxFlushCache (); \
	} else \
		CMemory_FillRAM [0x3030] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_3031(DAT) \
	CMemory_FillRAM [0x3031] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3031);
#else
#define SETPPU_3031(DAT) \
	CMemory_FillRAM [0x3031] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_3033(DAT) \
	CMemory_FillRAM [0x3033] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3033);
#else
#define SETPPU_3033(DAT) \
	CMemory_FillRAM [0x3033] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_3034(DAT) \
	CMemory_FillRAM [0x3034] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3034);
#else
#define SETPPU_3034(DAT) \
	CMemory_FillRAM [0x3034] = (DAT) & 0x7f;
#endif

#ifdef ZSNES_FX
#define SETPPU_3036(DAT) \
	CMemory_FillRAM [0x3036] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3036);
#else
#define SETPPU_3036(DAT) \
	CMemory_FillRAM [0x3036] = (DAT) & 0x7f;
#endif

#ifdef ZSNES_FX
#define SETPPU_3037(DAT) \
	CMemory_FillRAM [0x3037] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3037);
#else
#define SETPPU_3037(DAT) \
	CMemory_FillRAM [0x3037] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_3038(DAT) \
	CMemory_FillRAM [0x3038] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3038);
#else
#define SETPPU_3038(DAT) \
	CMemory_FillRAM [0x3038] = (DAT); \
	fx_dirtySCBR();
#endif

#ifdef ZSNES_FX
#define SETPPU_3039(DAT) \
	CMemory_FillRAM [0x3039] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x3039);
#else
#define SETPPU_3039(DAT) \
	CMemory_FillRAM [0x3039] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_303A(DAT) \
	CMemory_FillRAM [0x303a] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x303a);
#else
#define SETPPU_303A(DAT) \
	CMemory_FillRAM [0x303a] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_303B(DAT) \
	CMemory_FillRAM [0x303b] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x303b);
#else
#define SETPPU_303B(DAT) 
#endif

#ifdef ZSNES_FX
#define SETPPU_303C(DAT) \
	CMemory_FillRAM [0x303c] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x303c);
#else
#define SETPPU_303C(DAT) \
	CMemory_FillRAM [0x303c] = (DAT); \
	fx_updateRamBank((DAT));
#endif

#ifdef ZSNES_FX
#define SETPPU_303F(DAT) \
	CMemory_FillRAM [0x303f] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x303f);
#else
#define SETPPU_303F(DAT) \
	CMemory_FillRAM [0x303f] = (DAT);
#endif

#ifdef ZSNES_FX
#define SETPPU_301F(DAT) \
	CMemory_FillRAM [0x301f] = (DAT); \
	S9xSuperFXWriteReg ((DAT), 0x301f);
#else
#define SETPPU_301F(DAT) \
	CMemory_FillRAM [0x301f] = (DAT); \
	CMemory_FillRAM [0x3000 + GSU_SFR] |= FLG_G; \
    	if (Settings.SuperFX) \
		S9xSuperFXExec ();
#endif

#ifdef ZSNES_FX
#define SETPPU_DEFAULT_FX(ADDR,DAT) \
	CMemory_FillRAM [(ADDR)] = (DAT); \
	if ((ADDR) < 0x3040) \
		S9xSuperFXWriteReg ((BYTE), (ADDR));
#else
#define SETPPU_DEFAULT_FX(ADDR,DAT) \
	CMemory_FillRAM[(ADDR)] = (DAT); \
		if ((ADDR) >= 0x3100){ \
			FxCacheWriteAccess (ADDR); \
		}
#endif

#ifdef ZSNES_FX
#define SETPPU_NULL_FX(ADDR,DAT) \
	CMemory_FillRAM [(ADDR)] = (DAT); \
	if ((ADDR) < 0x3040) \
		S9xSuperFXWriteReg ((BYTE), (ADDR));
#else
#define SETPPU_NULL_FX(ADDR,DAT)
#endif

#if defined(USE_SETPPU_TABLE) || defined(USE_SETPPU_SMALL_TABLE)
static void S9xSetPPU_brightness(uint8 Byte, uint16 Address) { SETPPU_BRIGHTNESS(Byte); }
static void S9xSetPPU_obj(uint8 Byte, uint16 Address) { SETPPU_OBJ(Byte); }
static void S9xSetPPU_spr_write_low(uint8 Byte, uint16 Address) { SETPPU_SPR_WRITE_LOW(Byte); }
static void S9xSetPPU_spr_write_high(uint8 Byte, uint16 Address) { SETPPU_SPR_WRITE_HIGH(Byte); }
static void S9xSetPPU_spr_reg_write(uint8 Byte, uint16 Address) { SETPPU_SPR_REG_WRITE(Byte); }
static void S9xSetPPU_screen_mode(uint8 Byte, uint16 Address) { SETPPU_SCREEN_MODE(Byte); }
static void S9xSetPPU_mosaic_pixel(uint8 Byte, uint16 Address) { SETPPU_MOSAIC_PIXEL(Byte); }
static void S9xSetPPU_bg0sc(uint8 Byte, uint16 Address) { SETPPU_BG0SC(Byte); }
static void S9xSetPPU_bg1sc(uint8 Byte, uint16 Address) { SETPPU_BG1SC(Byte); }
static void S9xSetPPU_bg2sc(uint8 Byte, uint16 Address) { SETPPU_BG2SC(Byte); }
static void S9xSetPPU_bg3sc(uint8 Byte, uint16 Address) { SETPPU_BG3SC(Byte); }
static void S9xSetPPU_bg01nba(uint8 Byte, uint16 Address) { SETPPU_BG01NBA(Byte); }
static void S9xSetPPU_bg23nba(uint8 Byte, uint16 Address) { SETPPU_BG23NBA(Byte); }
static void S9xSetPPU_210d(uint8 Byte, uint16 Address) { SETPPU_210D(Byte); }
static void S9xSetPPU_210e(uint8 Byte, uint16 Address) { SETPPU_210E(Byte); }
static void S9xSetPPU_210f(uint8 Byte, uint16 Address) { SETPPU_210F(Byte); }
static void S9xSetPPU_2110(uint8 Byte, uint16 Address) { SETPPU_2110(Byte); }
static void S9xSetPPU_2111(uint8 Byte, uint16 Address) { SETPPU_2111(Byte); }
static void S9xSetPPU_2112(uint8 Byte, uint16 Address) { SETPPU_2112(Byte); }
static void S9xSetPPU_2113(uint8 Byte, uint16 Address) { SETPPU_2113(Byte); }
static void S9xSetPPU_2114(uint8 Byte, uint16 Address) { SETPPU_2114(Byte); }
static void S9xSetPPU_vram_access(uint8 Byte, uint16 Address) { SETPPU_VRAM_ACCESS(Byte); }
static void S9xSetPPU_vram_addr_low(uint8 Byte, uint16 Address) { SETPPU_VRAM_ADDR_LOW(Byte); }
static void S9xSetPPU_vram_addr_high(uint8 Byte, uint16 Address) { SETPPU_VRAM_ADDR_HIGH(Byte); }
static void S9xSetPPU_vram_write_low(uint8 Byte, uint16 Address) { SETPPU_VRAM_WRITE_LOW(Byte); }
static void S9xSetPPU_vram_write_high(uint8 Byte, uint16 Address) { SETPPU_VRAM_WRITE_HIGH(Byte); }
static void S9xSetPPU_mode7_rot(uint8 Byte, uint16 Address) { SETPPU_MODE7_ROT(Byte); }
static void S9xSetPPU_mode7_matrix_a(uint8 Byte, uint16 Address) { SETPPU_MODE7_MATRIX_A(Byte); }
static void S9xSetPPU_mode7_matrix_b(uint8 Byte, uint16 Address) { SETPPU_MODE7_MATRIX_B(Byte); }
static void S9xSetPPU_mode7_matrix_c(uint8 Byte, uint16 Address) { SETPPU_MODE7_MATRIX_C(Byte); }
static void S9xSetPPU_mode7_matrix_d(uint8 Byte, uint16 Address) { SETPPU_MODE7_MATRIX_D(Byte); }
static void S9xSetPPU_mode7_centre_x(uint8 Byte, uint16 Address) { SETPPU_MODE7_CENTRE_X(Byte); }
static void S9xSetPPU_mode7_centre_y(uint8 Byte, uint16 Address) { SETPPU_MODE7_CENTRE_Y(Byte); }
static void S9xSetPPU_cgram_addr(uint8 Byte, uint16 Address) { SETPPU_CGRAM_ADDR(Byte); }
static void S9xSetPPU_2122(uint8 Byte, uint16 Address) { SETPPU_2122(Byte); }
static void S9xSetPPU_enable_win_12(uint8 Byte, uint16 Address) { SETPPU_ENABLE_WIN_12(Byte); }
static void S9xSetPPU_enable_win_34(uint8 Byte, uint16 Address) { SETPPU_ENABLE_WIN_34(Byte); }
static void S9xSetPPU_enable_win_obj(uint8 Byte, uint16 Address) { SETPPU_ENABLE_WIN_OBJ(Byte); }
static void S9xSetPPU_win_1_left(uint8 Byte, uint16 Address) { SETPPU_WIN_1_LEFT(Byte); }
static void S9xSetPPU_win_1_right(uint8 Byte, uint16 Address) { SETPPU_WIN_1_RIGHT(Byte); }
static void S9xSetPPU_win_2_left(uint8 Byte, uint16 Address) { SETPPU_WIN_2_LEFT(Byte); }
static void S9xSetPPU_win_2_right(uint8 Byte, uint16 Address) { SETPPU_WIN_2_RIGHT(Byte); }
static void S9xSetPPU_win_overlap_14(uint8 Byte, uint16 Address) { SETPPU_WIN_OVERLAP_14(Byte); }
static void S9xSetPPU_win_overlap_obj(uint8 Byte, uint16 Address) { SETPPU_WIN_OVERLAP_OBJ(Byte); }
static void S9xSetPPU_main_screen_dest(uint8 Byte, uint16 Address) { SETPPU_MAIN_SCREEN_DEST(Byte); }
static void S9xSetPPU_sub_screen_dest(uint8 Byte, uint16 Address) { SETPPU_SUB_SCREEN_DEST(Byte); }
static void S9xSetPPU_main_screen_mask(uint8 Byte, uint16 Address) { SETPPU_MAIN_SCREEN_MASK(Byte); }
static void S9xSetPPU_sub_screen_mask(uint8 Byte, uint16 Address) { SETPPU_SUB_SCREEN_MASK(Byte); }
static void S9xSetPPU_fixed_add(uint8 Byte, uint16 Address) { SETPPU_FIXED_ADD(Byte); }
static void S9xSetPPU_col_add(uint8 Byte, uint16 Address) { SETPPU_COL_ADD(Byte); }
static void S9xSetPPU_fixed_sub(uint8 Byte, uint16 Address) { SETPPU_FIXED_SUB(Byte); }
static void S9xSetPPU_screen_sets(uint8 Byte, uint16 Address) { SETPPU_SCREEN_SETS(Byte); }
static void S9xSetPPU_null(uint8 Byte, uint16 Address) { SETPPU_NULL(); }
static void S9xSetPPU_spctool(uint8 Byte, uint16 Address) { SETPPU_SPC700(Address,Byte); }
static void S9xSetPPU_2180(uint8 Byte, uint16 Address) { SETPPU_2180(Byte); }
static void S9xSetPPU_2181(uint8 Byte, uint16 Address) { SETPPU_2181(Byte); }
static void S9xSetPPU_2182(uint8 Byte, uint16 Address) { SETPPU_2182(Byte); }
static void S9xSetPPU_2183(uint8 Byte, uint16 Address) { SETPPU_2183(Byte); }
static void S9xSetPPU_2801(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_2801(Byte); }
static void S9xSetPPU_empty(uint8 Byte, uint16 Address) { SETPPU_EMPTY(Address,Byte); }
static void S9xSetPPU_sa1_empty(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_EMPTY(Address,Byte); }
static void S9xSetPPU_3030(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3030(Byte); }
static void S9xSetPPU_3031(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3031(Byte); }
static void S9xSetPPU_3033(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3033(Byte); }
static void S9xSetPPU_3034(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3034(Byte); }
static void S9xSetPPU_3036(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3036(Byte); }
static void S9xSetPPU_3037(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3037(Byte); }
static void S9xSetPPU_3038(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3038(Byte); }
static void S9xSetPPU_3039(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_3039(Byte); }
static void S9xSetPPU_303a(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_303A(Byte); }
static void S9xSetPPU_303b(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_303B(Byte); }
static void S9xSetPPU_303c(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_303C(Byte); }
static void S9xSetPPU_303f(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_303F(Byte); }
static void S9xSetPPU_301f(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_301F(Byte); }
static void S9xSetPPU_default_fx(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_DEFAULT_FX(Address,Byte); }
static void S9xSetPPU_null_fx(uint8 Byte, uint16 Address) { SETPPU_SA1(Address,Byte) SETPPU_SUPERFX() SETPPU_NULL_FX(Address,Byte); }
#include "setppu_table.h"
#ifdef USE_SETPPU_SMALL_TABLE
void S9xSetPPU (uint8 Byte, uint16 Address) {
#ifdef DREAMCAST
	unsigned *ptr=((unsigned *)(&S9xSetPPU_func[Address-0x2100]));
	asm("pref @%0" : : "r" (ptr));
#endif
	if (Address<0x2100) { S9xSetPPU_empty(Byte,Address); return; }
	else if (Address>=0x3300) { S9xSetPPU_sa1_empty(Byte,Address); return; }
#ifdef DREAMCAST
	else ((S9xSetPPU_func_t)(*ptr))(Byte,Address);
#else
	else S9xSetPPU_func[Address-0x2100](Byte,Address);
#endif
}
#endif
#else

void S9xSetPPU (uint8 Byte, uint16 Address) {
//    fprintf(stderr, "%03d: %02x to %04x\n", CPU.V_Counter, Byte, Address);
	if (Address <= 0x2183) {
		switch (Address) {
		  case 0x2100: SETPPU_BRIGHTNESS(Byte) return;
		  case 0x2101: SETPPU_OBJ(Byte) return;
		  case 0x2102: SETPPU_SPR_WRITE_LOW(Byte) return;
		  case 0x2103: SETPPU_SPR_WRITE_HIGH(Byte) return;
		  case 0x2104: SETPPU_SPR_REG_WRITE(Byte) return;
		  case 0x2105: SETPPU_SCREEN_MODE(Byte) return;
		  case 0x2106: SETPPU_MOSAIC_PIXEL(Byte) return;
		  case 0x2107: SETPPU_BG0SC(Byte) return;
		  case 0x2108: SETPPU_BG1SC(Byte) return;
		  case 0x2109: SETPPU_BG2SC(Byte) return;
		  case 0x210A: SETPPU_BG3SC(Byte) return;
		  case 0x210B: SETPPU_BG01NBA(Byte) return;
		  case 0x210C: SETPPU_BG23NBA(Byte) return;
		  case 0x210D: SETPPU_210D(Byte) return;
		  case 0x210E: SETPPU_210E(Byte) return;
		  case 0x210F: SETPPU_210F(Byte) return;
		  case 0x2110: SETPPU_2110(Byte) return;
		  case 0x2111: SETPPU_2111(Byte) return;
		  case 0x2112: SETPPU_2112(Byte) return;
		  case 0x2113: SETPPU_2113(Byte) return;
		  case 0x2114: SETPPU_2114(Byte) return;
		  case 0x2115: SETPPU_VRAM_ACCESS(Byte) return;
		  case 0x2116: SETPPU_VRAM_ADDR_LOW(Byte) return;
		  case 0x2117: SETPPU_VRAM_ADDR_HIGH(Byte) return;
		  case 0x2118: SETPPU_VRAM_WRITE_LOW(Byte) return;
		  case 0x2119: SETPPU_VRAM_WRITE_HIGH(Byte) return;
		  case 0x211a: SETPPU_MODE7_ROT(Byte) return;
		  case 0x211b: SETPPU_MODE7_MATRIX_A(Byte) return;
		  case 0x211c: SETPPU_MODE7_MATRIX_B(Byte) return;
		  case 0x211d: SETPPU_MODE7_MATRIX_C(Byte) return;
		  case 0x211e: SETPPU_MODE7_MATRIX_D(Byte) return;
		  case 0x211f: SETPPU_MODE7_CENTRE_X(Byte) return;
		  case 0x2120: SETPPU_MODE7_CENTRE_Y(Byte) return;
		  case 0x2121: SETPPU_CGRAM_ADDR(Byte) return;
		  case 0x2122: SETPPU_2122(Byte) return;
		  case 0x2123: SETPPU_ENABLE_WIN_12(Byte) return;
		  case 0x2124: SETPPU_ENABLE_WIN_34(Byte) return;
		  case 0x2125: SETPPU_ENABLE_WIN_OBJ(Byte) return;
		  case 0x2126: SETPPU_WIN_1_LEFT(Byte) return;
		  case 0x2127: SETPPU_WIN_1_RIGHT(Byte) return;
		  case 0x2128: SETPPU_WIN_2_LEFT(Byte) return;
		  case 0x2129: SETPPU_WIN_2_RIGHT(Byte) return;
		  case 0x212a: SETPPU_WIN_OVERLAP_14(Byte) return;
		  case 0x212b: SETPPU_WIN_OVERLAP_OBJ(Byte) return;
		  case 0x212c: SETPPU_MAIN_SCREEN_DEST(Byte) return;
		  case 0x212d: SETPPU_SUB_SCREEN_DEST(Byte) return;
		  case 0x212e: SETPPU_MAIN_SCREEN_MASK(Byte) return;
		  case 0x212f: SETPPU_SUB_SCREEN_MASK(Byte) return;
		  case 0x2130: SETPPU_FIXED_ADD(Byte) return;
		  case 0x2131: SETPPU_COL_ADD(Byte) return;
		  case 0x2132: SETPPU_FIXED_SUB(Byte) return;
		  case 0x2133: SETPPU_SCREEN_SETS(Byte) return;
		  case 0x2134: case 0x2135: case 0x2136: case 0x2137:
		  case 0x2138: case 0x2139: case 0x213a: case 0x213b:
		  case 0x213c: case 0x213d: case 0x213e: case 0x213f:
			       SETPPU_NULL() return;
		  case 0x2140: case 0x2141: case 0x2142: case 0x2143:
		  case 0x2144: case 0x2145: case 0x2146: case 0x2147:
		  case 0x2148: case 0x2149: case 0x214a: case 0x214b:
		  case 0x214c: case 0x214d: case 0x214e: case 0x214f:
		  case 0x2150: case 0x2151: case 0x2152: case 0x2153:
		  case 0x2154: case 0x2155: case 0x2156: case 0x2157:
		  case 0x2158: case 0x2159: case 0x215a: case 0x215b:
		  case 0x215c: case 0x215d: case 0x215e: case 0x215f:
		  case 0x2160: case 0x2161: case 0x2162: case 0x2163:
		  case 0x2164: case 0x2165: case 0x2166: case 0x2167:
		  case 0x2168: case 0x2169: case 0x216a: case 0x216b:
		  case 0x216c: case 0x216d: case 0x216e: case 0x216f:
		  case 0x2170: case 0x2171: case 0x2172: case 0x2173:
		  case 0x2174: case 0x2175: case 0x2176: case 0x2177:
		  case 0x2178: case 0x2179: case 0x217a: case 0x217b:
		  case 0x217c: case 0x217d: case 0x217e: case 0x217f:
			       SETPPU_SPC700(Address,Byte) return;
		  case 0x2180: SETPPU_2180(Byte) return;
		  case 0x2181: SETPPU_2181(Byte) return;
		  case 0x2182: SETPPU_2182(Byte) return;
		  case 0x2183: SETPPU_2183(Byte) return;
		  default: SETPPU_EMPTY(Address,Byte); return;
		}
	} else {
		SETPPU_SA1(Address,Byte)
		if (Address == 0x2801) {
			SETPPU_2801(Byte); return;
		} else
			if (Address < 0x3000 || Address >= 0x3000 + 768) {
				SETPPU_EMPTY(Address,Byte); return;
			} else {
				SETPPU_SUPERFX()
				switch (Address) {
					  case 0x3030: SETPPU_3030(Byte); return;
					  case 0x3031: SETPPU_3031(Byte); return;
					  case 0x3033: SETPPU_3033(Byte); return;
					  case 0x3034: SETPPU_3034(Byte); return;
					  case 0x3036: SETPPU_3036(Byte); return;
					  case 0x3037: SETPPU_3037(Byte); return;
					  case 0x3038: SETPPU_3038(Byte); return;
					  case 0x3039: SETPPU_3039(Byte); return;
					  case 0x303a: SETPPU_303A(Byte); return;
					  case 0x303b: SETPPU_303B(Byte); return;
					  case 0x303c: SETPPU_303C(Byte); return;
					  case 0x303f: SETPPU_303F(Byte); return;
					  case 0x301f: SETPPU_301F(Byte); return;
					  default: SETPPU_DEFAULT_FX(Address,Byte); return;
				}
//				SETPPU_NULL_FX(Address,Byte) return;
			}
	}
//	SETPPU_EMPTY(Address,Byte); 
}
#endif

/******************************************************************************/
/* S9xGetPPU()                                                                */
/* This function retrieves a PPU Register                                     */
/******************************************************************************/

#define GETPPU_OPENBUS \
		return OpenBus;

#define GETPPU_PPU_OPENBUS \
		return PPU.OpenBus1;

#define GETPPU_16X8_MUL(ADDR) \
	    if (PPU.Need16x8Mulitply) \
	    { \
		int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8); \
		CMemory_FillRAM[0x2134] = (uint8) r; \
		CMemory_FillRAM[0x2135] = (uint8)(r >> 8); \
		CMemory_FillRAM[0x2136] = (uint8)(r >> 16); \
		PPU.Need16x8Mulitply = FALSE; \
	    } \
	    return (PPU.OpenBus1 = CMemory_FillRAM[ADDR]);

#define GETPPU_LATCH_COUNTERS \
		S9xLatchCounters(0); \
		return OpenBus;

#define GETPPU_READ_OAM \
	{ \
 		uint8 byte /*= OpenBus*/; \
		if(PPU.OAMAddr&0x100){ \
			if (!(PPU.OAMFlip&1)) { \
				byte = PPU.OAMData [(PPU.OAMAddr&0x10f) << 1]; \
			} else { \
				byte = PPU.OAMData [((PPU.OAMAddr&0x10f) << 1) + 1]; \
				PPU.OAMAddr=(PPU.OAMAddr+1)&0x1ff; \
				if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1)) { \
					PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1; \
					IPPU.OBJChanged = TRUE; \
				} \
			} \
		} else { \
			if (!(PPU.OAMFlip&1)) { \
				byte = PPU.OAMData [PPU.OAMAddr << 1]; \
			} else { \
				byte = PPU.OAMData [(PPU.OAMAddr << 1) + 1]; \
				++PPU.OAMAddr; \
				if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1)) { \
					PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1; \
					IPPU.OBJChanged = TRUE; \
				} \
			} \
		} \
		PPU.OAMFlip ^= 1; \
		return (PPU.OpenBus1 = byte); \
	}

#ifdef CORRECT_VRAM_READS
#define GETPPU_READ_LOW_VRAM \
	{ \
		uint8 byte = IPPU.VRAMReadBuffer & 0xff; \
		if (!PPU.VMA.High) { \
			if (PPU.VMA.FullGraphicCount) { \
				uint32 addr = PPU.VMA.Address; \
				uint32 rem = addr & PPU.VMA.Mask1; \
				uint32 address = (addr & ~PPU.VMA.Mask1) +  \
					(rem >> PPU.VMA.Shift) + \
					((rem & (PPU.VMA.FullGraphicCount - 1)) << 3); \
				IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((address << 1) & 0xFFFF)); \
			} else \
				IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((PPU.VMA.Address << 1) & 0xffff)); \
			PPU.VMA.Address += PPU.VMA.Increment; \
		} \
		PPU.OpenBus1 = byte; \
		return byte; \
	}
#else
#define GETPPU_READ_LOW_VRAM \
	{ \
 		uint8 byte/* = OpenBus*/; \
		if (IPPU.FirstVRAMRead) \
			byte = CMemory_VRAM[(PPU.VMA.Address << 1)&0xFFFF]; \
		else \
			if (PPU.VMA.FullGraphicCount) { \
				uint32 addr = PPU.VMA.Address - 1; \
				uint32 rem = addr & PPU.VMA.Mask1; \
				uint32 address = (addr & ~PPU.VMA.Mask1) + \
					(rem >> PPU.VMA.Shift) + \
					((rem & (PPU.VMA.FullGraphicCount - 1)) << 3); \
				byte = CMemory_VRAM [((address << 1) - 2) & 0xFFFF]; \
			} else \
				byte = CMemory_VRAM[((PPU.VMA.Address << 1) - 2) & 0xffff]; \
			if (!PPU.VMA.High) { \
				PPU.VMA.Address += PPU.VMA.Increment; \
				IPPU.FirstVRAMRead = FALSE; \
			} \
		PPU.OpenBus1 = byte; \
		return byte; \
	}
#endif

#ifdef CORRECT_VRAM_READS
#define GETPPU_READ_HIGH_VRAM \
	{ \
		uint8 byte = (IPPU.VRAMReadBuffer>>8) & 0xff; \
		if (PPU.VMA.High) { \
			if (PPU.VMA.FullGraphicCount) { \
				uint32 addr = PPU.VMA.Address; \
				uint32 rem = addr & PPU.VMA.Mask1; \
				uint32 address = (addr & ~PPU.VMA.Mask1) + \
					(rem >> PPU.VMA.Shift) + \
					((rem & (PPU.VMA.FullGraphicCount - 1)) << 3); \
				IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((address << 1) & 0xFFFF)); \
			} else \
				IPPU.VRAMReadBuffer = READ_WORD(CMemory_VRAM+((PPU.VMA.Address << 1) & 0xffff)); \
			PPU.VMA.Address += PPU.VMA.Increment; \
		} \
		PPU.OpenBus1 = byte; \
		return byte; \
	}
#else
#define GETPPU_READ_HIGH_VRAM \
	{ \
 		uint8 byte/* = OpenBus*/; \
		if (IPPU.FirstVRAMRead) \
			byte = CMemory_VRAM[((PPU.VMA.Address << 1) + 1) & 0xffff]; \
		else \
			if (PPU.VMA.FullGraphicCount) { \
				uint32 addr = PPU.VMA.Address - 1; \
				uint32 rem = addr & PPU.VMA.Mask1; \
				uint32 address = (addr & ~PPU.VMA.Mask1) + \
					(rem >> PPU.VMA.Shift) + \
					((rem & (PPU.VMA.FullGraphicCount - 1)) << 3); \
				byte = CMemory_VRAM [((address << 1) - 1) & 0xFFFF]; \
			} else \
				byte = CMemory_VRAM[((PPU.VMA.Address << 1) - 1) & 0xFFFF]; \
			if (PPU.VMA.High) { \
				PPU.VMA.Address += PPU.VMA.Increment; \
				IPPU.FirstVRAMRead = FALSE; \
			} \
		PPU.OpenBus1 = byte; \
		return byte; \
	}
#endif

#define GETPPU_READ_PAL_DATA \
	{ \
 	    uint8 byte/* = OpenBus*/; \
	    if (PPU.CGFLIPRead) \
		byte = PPU.CGDATA [PPU.CGADD++] >> 8; \
	    else \
		byte = PPU.CGDATA [PPU.CGADD] & 0xff; \
	    PPU.CGFLIPRead ^= 1; \
	    return (PPU.OpenBus2 = byte); \
	}

#define GETPPU_H_COUNTER \
	{ \
 	    uint8 byte/* = OpenBus*/; \
	    if (PPU.HBeamFlip) \
		byte = (PPU.OpenBus2 & 0xfe) | ((PPU.HBeamPosLatched >> 8) & 0x01); \
	    else \
		byte = (uint8)PPU.HBeamPosLatched; \
            PPU.OpenBus2 = byte; \
	    PPU.HBeamFlip ^= 1; \
	    return byte; \
	} 

#define GETPPU_V_COUNTER \
	{ \
 	    uint8 byte/* = OpenBus*/; \
	    if (PPU.VBeamFlip) \
                byte = (PPU.OpenBus2 & 0xfe) | ((PPU.VBeamPosLatched >> 8) & 0x01); \
	    else \
                byte = (uint8)PPU.VBeamPosLatched; \
            PPU.OpenBus2 = byte; \
	    PPU.VBeamFlip ^= 1; \
	    return byte; \
	}

#define GETPPU_REDRAW \
		FLUSH_REDRAW (); \
		return (PPU.OpenBus1 = (Model->_5C77 | PPU.RangeTimeOver)); 

#define GETPPU_VCLOCK \
	    PPU.VBeamFlip = PPU.HBeamFlip = 0; \
	    return ((Settings.PAL ? 0x10 : 0) | (CMemory_FillRAM[0x213f] & 0xc0)| Model->_5C78) | (~PPU.OpenBus2 & 0x20);

unsigned totales=0;
unsigned evitados=0;
#ifdef SPCTOOL
#define GETPPU_SPC700(ADDR) \
	    return ((uint8) _SPCOutP [ADDR & 3]);
#else
#define GETPPU_SPC700_COMMON(ADDR) \
	    if (Settings.APUEnabled) { \
		if (SNESGameFixes.APU_OutPorts_ReturnValueFix && \
		    ADDR >= 0x2140 && ADDR <= 0x2143 && !CPU.V_Counter) { \
                    return (uint8)((ADDR & 1) ? ((rand() & 0xff00) >> 8) :  \
				   (rand() & 0xff)); \
		} \
		return (APU.OutPorts [ADDR & 3]); \
	    } \
	    CPU.BranchSkip = TRUE; \
	    if ((ADDR & 3) < 2) { \
		int r = rand (); \
		if (r & 2) { \
		    if (r & 4) \
			return ((ADDR & 3) == 1 ? 0xaa : 0xbb); \
		    else \
			return ((r >> 3) & 0xff); \
		} \
	    } else { \
		int r = rand (); \
		if (r & 2) \
		    return ((r >> 3) & 0xff); \
	    } \
	    return (CMemory_FillRAM[ADDR]);
#ifdef USE_ALWAYS_APU_SYNC
#define GETPPU_SPC700_APU() 
#else
#define GETPPU_SPC700_APU() \
	if (snes4all_sound_enable<=1) \
		S9xApuExecute();
#endif
#ifdef SPC700_SHUTDOWN	
#define GETPPU_SPC700(ADDR) \
	    GETPPU_SPC700_APU() \
	    IAPU.APUExecuting = Settings.APUEnabled; \
	    IAPU.WaitCounter++; \
	    GETPPU_SPC700_COMMON(ADDR)
#else
#define GETPPU_SPC700(ADDR) \
	    GETPPU_SPC700_APU() \
	    GETPPU_SPC700_COMMON(ADDR)
#endif
#endif // SPCTOOL

#if !defined(USE_MMU) || defined(USE_MMU_BASEPOINTER)
#define GETPPU_READ_WRAM \
	{ \
 	    uint8 byte = CMemory_RAM [PPU.WRAM++]; \
	    PPU.WRAM &= 0x1FFFF; \
	    return byte; \
	}
#else
#define GETPPU_READ_WRAM \
	{ \
	    uint8 byte; \
	    uint8 *mmu_ram=(uint8 *)(0); \
	    if (PPU.WRAM>=0x2000) \
	    	mmu_ram=(uint8 *)(0x7e0000); \
 	    byte = mmu_ram[PPU.WRAM++]; \
	    PPU.WRAM &= 0x1FFFF; \
	    return byte; \
	}
#endif

#define GETPPU_MODEL_5C77_20 \
	if(Model->_5C77 ==2) \
	       	return (0x20); \
	return OpenBus;

#define GETPPU_MODEL_5C77_0 \
	if(Model->_5C77 ==2) \
	        return (0); \
	return OpenBus;

#define GETPPU_MODE_SRTC \
	if (Settings.SRTC) \
		return (S9xGetSRTC (0x2800)); \
	return OpenBus;
			    
#define GETPPU_SA1 \
	if (Settings.SA1) \
	    return (S9xGetSA1 (Address));

#ifdef CPU_SHUTDOWN
#define GETPPU_SUPERFX_SHUTDOWN(ADDR) \
	if (ADDR == 0x3030) \
	    CPU.WaitAddress = CPU.PCAtOpcodeStart; \
	else
#else
#define GETPPU_SUPERFX_SHUTDOWN(ADDR)
#endif

#ifdef ZSNES_FX
#define GETPPU_SUPERFX_ZSNES(ADDR) \
	{ \
		uint8 byte; \
		if (ADDR < 0x3040) \
		    byte = S9xSuperFXReadReg (ADDR); \
		else \
		    byte = CMemory_FillRAM [ADDR]; \
		GETPPU_SUPERFX_SHUTDOWN(ADDR) \
		if (ADDR == 0x3031) \
		    CLEAR_IRQ_SOURCE (GSU_IRQ_SOURCE); \
		return byte; \
	}
#else
#define GETPPU_SUPERFX_ZSNES(ADDR) \
	{ \
		uint8 byte = CMemory_FillRAM [ADDR]; \
		GETPPU_SUPERFX_SHUTDOWN(ADDR) \
		if (ADDR == 0x3031) { \
		    CLEAR_IRQ_SOURCE (GSU_IRQ_SOURCE); \
		    CMemory_FillRAM [0x3031] = byte & 0x7f; \
		} \
		return (byte); \
	}
#endif
	
#define GETPPU_SUPERFX(ADDR) \
	if (!Settings.SuperFX) \
			return OpenBus; \
	GETPPU_SUPERFX_ZSNES(ADDR)

#ifdef USE_GETPPU_TABLE
static uint8 S9xGetPPU_openbus(uint16 Address) { GETPPU_OPENBUS }
static uint8 S9xGetPPU_ppu_openbus(uint16 Address) { GETPPU_PPU_OPENBUS }
static uint8 S9xGetPPU_16x8_mul(uint16 Address) { GETPPU_16X8_MUL(Address) }
static uint8 S9xGetPPU_latch_counters(uint16 Address) { GETPPU_LATCH_COUNTERS }
static uint8 S9xGetPPU_read_oam(uint16 Address) { GETPPU_READ_OAM }
static uint8 S9xGetPPU_read_low_vram(uint16 Address) { GETPPU_READ_LOW_VRAM }
static uint8 S9xGetPPU_read_high_vram(uint16 Address) { GETPPU_READ_HIGH_VRAM }
static uint8 S9xGetPPU_read_pal_data(uint16 Address) { GETPPU_READ_PAL_DATA }
static uint8 S9xGetPPU_h_counter(uint16 Address) { GETPPU_H_COUNTER }
static uint8 S9xGetPPU_v_counter(uint16 Address) { GETPPU_V_COUNTER }
static uint8 S9xGetPPU_redraw(uint16 Address) { GETPPU_REDRAW }
static uint8 S9xGetPPU_vclock(uint16 Address) { GETPPU_VCLOCK }
static uint8 S9xGetPPU_spctool(uint16 Address) { GETPPU_SPC700(Address) }
static uint8 S9xGetPPU_read_wram(uint16 Address) { GETPPU_READ_WRAM }
static uint8 S9xGetPPU_sa1(uint16 Address) { GETPPU_SA1 GETPPU_OPENBUS }
static uint8 S9xGetPPU_superfx(uint16 Address) { GETPPU_SA1 GETPPU_SUPERFX(Address) }
static uint8 S9xGetPPU_model_5c77_20(uint16 Address) { GETPPU_SA1 GETPPU_MODEL_5C77_20 }
static uint8 S9xGetPPU_model_5c77_0(uint16 Address) { GETPPU_SA1 GETPPU_MODEL_5C77_0 }
static uint8 S9xGetPPU_mode_srtc(uint16 Address) { GETPPU_MODE_SRTC }
#include "getppu_table.h"
#else

uint8 S9xGetPPU (uint16 Address) {
	if(Address<0x2100)
		GETPPU_OPENBUS
    if (Address <= 0x2190) {
 	switch (Address) {
  	case 0x2100: case 0x2101: case 0x2102: case 0x2103:
	    		GETPPU_OPENBUS
	case 0x2104: case 0x2105: case 0x2106:
			GETPPU_PPU_OPENBUS
	case 0x2107:	GETPPU_OPENBUS
	case 0x2108: case 0x2109: case 0x210a:
			GETPPU_PPU_OPENBUS
	case 0x210b: case 0x210c: case 0x210d: case 0x210e:
	case 0x210f: case 0x2110: case 0x2111: case 0x2112:
	case 0x2113: 	GETPPU_OPENBUS
	case 0x2114: case 0x2115: case 0x2116:
			GETPPU_PPU_OPENBUS
	case 0x2117:	GETPPU_OPENBUS
	case 0x2118: case 0x2119: case 0x211a:
			GETPPU_PPU_OPENBUS
	case 0x211b: case 0x211c: case 0x211d: case 0x211e:
	case 0x211f: case 0x2120:
	    		GETPPU_OPENBUS
	case 0x2121: case 0x2122: case 0x2123:
	    		GETPPU_OPENBUS
	case 0x2124: case 0x2125: case 0x2126:
			GETPPU_PPU_OPENBUS
	case 0x2127:	GETPPU_OPENBUS
	case 0x2128: case 0x2129: case 0x212a:
			GETPPU_PPU_OPENBUS
	case 0x212b: case 0x212c: case 0x212d: case 0x212e:
	case 0x212f: case 0x2130: case 0x2131: case 0x2132:
	case 0x2133:	GETPPU_OPENBUS
	case 0x2134: case 0x2135: case 0x2136:
	    		GETPPU_16X8_MUL(Address)
	case 0x2137:	GETPPU_LATCH_COUNTERS
	case 0x2138:	GETPPU_READ_OAM
	case 0x2139: 	GETPPU_READ_LOW_VRAM
	case 0x213A:	GETPPU_READ_HIGH_VRAM
	case 0x213B:	GETPPU_READ_PAL_DATA
	case 0x213C:	GETPPU_H_COUNTER
	case 0x213D:	GETPPU_V_COUNTER
	case 0x213E:	GETPPU_REDRAW
	case 0x213F:	GETPPU_VCLOCK
	case 0x2140: case 0x2141: case 0x2142: case 0x2143:
	case 0x2144: case 0x2145: case 0x2146: case 0x2147:
	case 0x2148: case 0x2149: case 0x214a: case 0x214b:
	case 0x214c: case 0x214d: case 0x214e: case 0x214f:
	case 0x2150: case 0x2151: case 0x2152: case 0x2153:
	case 0x2154: case 0x2155: case 0x2156: case 0x2157:
	case 0x2158: case 0x2159: case 0x215a: case 0x215b:
	case 0x215c: case 0x215d: case 0x215e: case 0x215f:
	case 0x2160: case 0x2161: case 0x2162: case 0x2163:
	case 0x2164: case 0x2165: case 0x2166: case 0x2167:
	case 0x2168: case 0x2169: case 0x216a: case 0x216b:
	case 0x216c: case 0x216d: case 0x216e: case 0x216f:
	case 0x2170: case 0x2171: case 0x2172: case 0x2173:
	case 0x2174: case 0x2175: case 0x2176: case 0x2177:
	case 0x2178: case 0x2179: case 0x217a: case 0x217b:
	case 0x217c: case 0x217d: case 0x217e: case 0x217f:
			GETPPU_SPC700(Address)
	case 0x2180:	GETPPU_READ_WRAM
	case 0x2181: case 0x2182: case 0x2183:
	    		GETPPU_OPENBUS
	default:	GETPPU_OPENBUS
	}
    } else {
   	GETPPU_SA1
	if (Address <= 0x2fff || Address >= 0x3000 + 768) {
	    switch (Address) {
		    case 0x21c2: GETPPU_MODEL_5C77_20
		    case 0x21c3: GETPPU_MODEL_5C77_0
		    case 0x2800: GETPPU_MODE_SRTC
		    default:	 GETPPU_OPENBUS
	    }
	}
   	GETPPU_SUPERFX(Address)
    }
    GETPPU_OPENBUS
}
// GETPPU_TABLE
#endif

/******************************************************************************/
/* S9xSetCPU()                                                                */
/* This function sets a CPU/DMA Register to a specific byte                   */
/******************************************************************************/
void S9xSetCPU (uint8 byte, uint16 Address)
{
	int d;
	if (Address < 0x4200)
	{
		CPU.Cycles += ONE_CYCLE;
		switch (Address)
		{
		  case 0x4016:
			// S9xReset reading of old-style joypads
			if ((byte & 1) && !(CMemory_FillRAM [Address] & 1))
			{
				PPU.Joypad1ButtonReadPos = 0;
				PPU.Joypad2ButtonReadPos = 0;
				PPU.Joypad3ButtonReadPos = 0;
			}
			break;
		  case 0x4017:
			break;
		  default:
			break;
		}
	}
	else
		switch (Address)
		{
		  case 0x4200:
			// NMI, V & H IRQ and joypad reading enable flags
			if (byte & 0x20)
			{
				//if(!SNESGameFixes.umiharakawaseFix && PPU.IRQVBeamPos==262) fprintf(stderr, "PPU.IRQVBeamPos = %d, CPU.V_Counter = %d\n", PPU.IRQVBeamPos, CPU.V_Counter);
				if (!PPU.VTimerEnabled)
				{
					PPU.VTimerEnabled = TRUE;
					if (PPU.HTimerEnabled)
						S9xUpdateHTimer ();
					else
						if (PPU.IRQVBeamPos == CPU.V_Counter)
							S9xSetIRQ (PPU_V_BEAM_IRQ_SOURCE);
				}
			}
			else
			{
				PPU.VTimerEnabled = FALSE;
//	    if (SNESGameFixes.umiharakawaseFix)
//		byte &= ~0x20;
			}

			if (byte & 0x10)
			{
				if (!PPU.HTimerEnabled)
				{
					PPU.HTimerEnabled = TRUE;
					S9xUpdateHTimer ();
				}
			}
			else
			{
				// No need to check for HTimer being disabled as the scanline
				// event trigger code won't trigger an H-IRQ unless its enabled.
				PPU.HTimerEnabled = FALSE;
				PPU.HTimerPosition = Settings.H_Max + 1;
			}
			if (!Settings.DaffyDuck)
				CLEAR_IRQ_SOURCE (PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE);

			if ((byte & 0x80) && 
				!(CMemory_FillRAM [0x4200] & 0x80) &&
				CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE &&
				CPU.V_Counter <= PPU.ScreenHeight + 
				(SNESGameFixes.alienVSpredetorFix ? 25 : 15) &&   //jyam 15->25 alien vs predetor
// Panic Bomberman clears the NMI pending flag @ scanline 230 before enabling
// NMIs again. The NMI routine crashes the CPU if it is called without the NMI
// pending flag being set...
				(CMemory_FillRAM [0x4210] & 0x80) &&
				!CPU.NMIActive)
			{
				CPU.Flags |= NMI_FLAG;
				CPU.NMIActive = TRUE;
				CPU.NMICycleCount = CPU.NMITriggerPoint;
			}
			break;
		  case 0x4201:
			if((byte&0x80)==0 && (CMemory_FillRAM[0x4213]&0x80)==0x80)
				S9xLatchCounters(1);
			CMemory_FillRAM[0x4201] = CMemory_FillRAM[0x4213] = byte;
			break;
		  case 0x4202:
			// Multiplier (for multply)
			break;
		  case 0x4203:
			{
				// Multiplicand
				uint32 res = CMemory_FillRAM[0x4202] * byte;

				CMemory_FillRAM[0x4216] = (uint8) res;
				CMemory_FillRAM[0x4217] = (uint8) (res >> 8);
				break;
			}
		  case 0x4204:
		  case 0x4205:
			// Low and high muliplier (for divide)
			break;
		  case 0x4206:
			{
				// Divisor
				uint16 a = CMemory_FillRAM[0x4204] + (CMemory_FillRAM[0x4205] << 8);
				uint16 div = byte ? a / byte : 0xffff;
				uint16 rem = byte ? a % byte : a;

				CMemory_FillRAM[0x4214] = (uint8)div;
				CMemory_FillRAM[0x4215] = div >> 8;
				CMemory_FillRAM[0x4216] = (uint8)rem;
				CMemory_FillRAM[0x4217] = rem >> 8;
				break;
			}
		  case 0x4207:
			d = PPU.IRQHBeamPos;
			PPU.IRQHBeamPos = (PPU.IRQHBeamPos & 0xFF00) | byte;

			if (PPU.HTimerEnabled && PPU.IRQHBeamPos != d)
				S9xUpdateHTimer ();
			break;

		  case 0x4208:
			d = PPU.IRQHBeamPos;
			PPU.IRQHBeamPos = (PPU.IRQHBeamPos & 0xFF) | ((byte & 1) << 8);

			if (PPU.HTimerEnabled && PPU.IRQHBeamPos != d)
				S9xUpdateHTimer ();

			break;

		  case 0x4209:
			d = PPU.IRQVBeamPos;
			PPU.IRQVBeamPos = (PPU.IRQVBeamPos & 0xFF00) | byte;
			if (PPU.VTimerEnabled && PPU.IRQVBeamPos != d)
			{
				if (PPU.HTimerEnabled)
					S9xUpdateHTimer ();
				else
				{
					if (PPU.IRQVBeamPos == CPU.V_Counter)
						S9xSetIRQ (PPU_V_BEAM_IRQ_SOURCE);
				}
			}
			break;

		  case 0x420A:
			d = PPU.IRQVBeamPos;
			PPU.IRQVBeamPos = (PPU.IRQVBeamPos & 0xFF) | ((byte & 1) << 8);
			if (PPU.VTimerEnabled && PPU.IRQVBeamPos != d)
			{
				if (PPU.HTimerEnabled)
					S9xUpdateHTimer ();
				else
				{
					if (PPU.IRQVBeamPos == CPU.V_Counter)
						S9xSetIRQ (PPU_V_BEAM_IRQ_SOURCE);
				}
			}
			break;

		  case 0x420B:
			if ((byte & 0x01) != 0)
				S9xDoDMA (0);
			if ((byte & 0x02) != 0)
				S9xDoDMA (1);
			if ((byte & 0x04) != 0)
				S9xDoDMA (2);
			if ((byte & 0x08) != 0)
				S9xDoDMA (3);
			if ((byte & 0x10) != 0)
				S9xDoDMA (4);
			if ((byte & 0x20) != 0)
				S9xDoDMA (5);
			if ((byte & 0x40) != 0)
				S9xDoDMA (6);
			if ((byte & 0x80) != 0)
				S9xDoDMA (7);
			break;
		  case 0x420C:
#ifndef ENABLE_HDMA
			byte = 0;
#else
#ifndef USE_HDMA_EVENT
			if ((unsigned)CPU.NextEvent > (unsigned)Settings.HBlankStart) {
				CPU.WhichEvent = HBLANK_START_EVENT;
				CPU.NextEvent = Settings.HBlankStart;
			}
#endif
#endif
			CMemory_FillRAM[0x420c] = byte;
			IPPU.HDMA = byte;
			break;

		  case 0x420d:
			// Cycle speed 0 - 2.68Mhz, 1 - 3.58Mhz (banks 0x80 +)
			if ((byte & 1) != (CMemory_FillRAM [0x420d] & 1))
			{
				if (byte & 1)
				{
					CPU.FastROMSpeed = ONE_CYCLE;
				}
				else CPU.FastROMSpeed = SLOW_ONE_CYCLE;

				CMemory_FixROMSpeed ();
			}
			break;

		  case 0x420e:
		  case 0x420f:
			// --->>> Unknown
			break;
		  case 0x4210:
			// NMI ocurred flag (reset on read or write)
			CMemory_FillRAM[0x4210] = Model->_5A22;
			return;
		  case 0x4211:
			// IRQ ocurred flag (reset on read or write)
			CLEAR_IRQ_SOURCE (PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE);
			break;
		  case 0x4212:
			// v-blank, h-blank and joypad being scanned flags (read-only)
		  case 0x4213:
			// I/O Port (read-only)
		  case 0x4214:
		  case 0x4215:
			// Quotent of divide (read-only)
		  case 0x4216:
		  case 0x4217:
			// Multiply product (read-only)
			return;
		  case 0x4218:
		  case 0x4219:
		  case 0x421a:
		  case 0x421b:
		  case 0x421c:
		  case 0x421d:
		  case 0x421e:
		  case 0x421f:
			// Joypad values (read-only)
			return;

		  case 0x4300:
		  case 0x4310:
		  case 0x4320:
		  case 0x4330:
		  case 0x4340:
		  case 0x4350:
		  case 0x4360:
		  case 0x4370:
			d = (Address >> 4) & 0x7;
			DMA[d].TransferDirection = (byte & 128) != 0 ? 1 : 0;
			DMA[d].HDMAIndirectAddressing = (byte & 64) != 0 ? 1 : 0;
			DMA[d].AAddressDecrement = (byte & 16) != 0 ? 1 : 0;
			DMA[d].AAddressFixed = (byte & 8) != 0 ? 1 : 0;
			DMA[d].TransferMode = (byte & 7);
			break;

		  case 0x4301:
		  case 0x4311:
		  case 0x4321:
		  case 0x4331:
		  case 0x4341:
		  case 0x4351:
		  case 0x4361:
		  case 0x4371:
			DMA[((Address >> 4) & 0x7)].BAddress = byte;
			break;

		  case 0x4302:
		  case 0x4312:
		  case 0x4322:
		  case 0x4332:
		  case 0x4342:
		  case 0x4352:
		  case 0x4362:
		  case 0x4372:
			d = (Address >> 4) & 0x7;
			DMA[d].AAddress &= 0xFF00;
			DMA[d].AAddress |= byte;
			break;

		  case 0x4303:
		  case 0x4313:
		  case 0x4323:
		  case 0x4333:
		  case 0x4343:
		  case 0x4353:
		  case 0x4363:
		  case 0x4373:
			d = (Address >> 4) & 0x7;
			DMA[d].AAddress &= 0xFF;
			DMA[d].AAddress |= byte << 8;
			break;

		  case 0x4304:
		  case 0x4314:
		  case 0x4324:
		  case 0x4334:
		  case 0x4344:
		  case 0x4354:
		  case 0x4364:
		  case 0x4374:
			DMA[((Address >> 4) & 0x7)].ABank = byte;
			HDMAMemPointers[((Address >> 4) & 0x7)]=NULL;

			break;

		  case 0x4305:
		  case 0x4315:
		  case 0x4325:
		  case 0x4335:
		  case 0x4345:
		  case 0x4355:
		  case 0x4365:
		  case 0x4375:
			d = (Address >> 4) & 0x7;
			DMA[d].TransferBytes &= 0xFF00;
			DMA[d].TransferBytes |= byte;
			DMA[d].IndirectAddress &= 0xff00;
			DMA[d].IndirectAddress |= byte;
			HDMAMemPointers[d]=NULL;
			break;

		  case 0x4306:
		  case 0x4316:
		  case 0x4326:
		  case 0x4336:
		  case 0x4346:
		  case 0x4356:
		  case 0x4366:
		  case 0x4376:
			d = (Address >> 4) & 0x7;
			DMA[d].TransferBytes &= 0xFF;
			DMA[d].TransferBytes |= byte << 8;
			DMA[d].IndirectAddress &= 0xff;
			DMA[d].IndirectAddress |= byte << 8;
			HDMAMemPointers[d]=NULL;
			break;

		  case 0x4307:
		  case 0x4317:
		  case 0x4327:
		  case 0x4337:
		  case 0x4347:
		  case 0x4357:
		  case 0x4367:
		  case 0x4377:
			DMA[d = ((Address >> 4) & 0x7)].IndirectBank = byte;
			HDMAMemPointers[d]=NULL;
			break;

		  case 0x4308:
		  case 0x4318:
		  case 0x4328:
		  case 0x4338:
		  case 0x4348:
		  case 0x4358:
		  case 0x4368:
		  case 0x4378:
			d = (Address >> 4) & 7;
			DMA[d].Address &= 0xff00;
			DMA[d].Address |= byte;
			HDMAMemPointers[d] = NULL;
			break;

		  case 0x4309:
		  case 0x4319:
		  case 0x4329:
		  case 0x4339:
		  case 0x4349:
		  case 0x4359:
		  case 0x4369:
		  case 0x4379:
			d = (Address >> 4) & 0x7;
			DMA[d].Address &= 0xff;
			DMA[d].Address |= byte << 8;
			HDMAMemPointers[d] = NULL;
			break;

		  case 0x430A:
		  case 0x431A:
		  case 0x432A:
		  case 0x433A:
		  case 0x434A:
		  case 0x435A:
		  case 0x436A:
		  case 0x437A:
			d = (Address >> 4) & 0x7;
			DMA[d].LineCount = byte & 0x7f;
			DMA[d].Repeat = !(byte & 0x80);
			break;

		  case 0x430F:
		  case 0x431F:
		  case 0x432F:
		  case 0x433F:
		  case 0x434F:
		  case 0x435F:
		  case 0x436F:
		  case 0x437F:
			Address &= ~4; // Convert 43xF to 43xB
			/* fall through */
		  case 0x430B:
		  case 0x431B:
		  case 0x432B:
		  case 0x433B:
		  case 0x434B:
		  case 0x435B:
		  case 0x436B:
		  case 0x437B:

			// Unknown, but they seem to be RAM-ish
			fprintf(stderr, "Write %02x to %04x!\n", byte, Address);
			break;

			//These registers are used by both the S-DD1 and the SPC7110
		  case 0x4800:
		  case 0x4801:
		  case 0x4802:
		  case 0x4803:
			if(Settings.SPC7110)
				S9xSetSPC7110(byte, Address);
			//printf ("%02x->%04x\n", byte, Address);
			break;

		  case 0x4804:
		  case 0x4805:
		  case 0x4806:
		  case 0x4807:
			//printf ("%02x->%04x\n", byte, Address);
			if(Settings.SPC7110)
				S9xSetSPC7110(byte, Address);
			else if (Settings.SDD1)
				S9xSetSDD1MemoryMap (Address - 0x4804, byte & 7);
			break;

			//these are used by the SPC7110
		  case 0x4808:
		  case 0x4809:
		  case 0x480A:
		  case 0x480B:
		  case 0x480C:
		  case 0x4810:
		  case 0x4811:
		  case 0x4812:
		  case 0x4813:
		  case 0x4814:
		  case 0x4815:
		  case 0x4816:
		  case 0x4817:
		  case 0x4818:
		  case 0x481A:
		  case 0x4820:
		  case 0x4821:
		  case 0x4822:
		  case 0x4823:
		  case 0x4824:
		  case 0x4825:
		  case 0x4826:
		  case 0x4827:
		  case 0x4828:
		  case 0x4829:
		  case 0x482A:
		  case 0x482B:
		  case 0x482C:
		  case 0x482D:
		  case 0x482E:
		  case 0x482F:
		  case 0x4830:
		  case 0x4831:
		  case 0x4832:
		  case 0x4833:
		  case 0x4834:
		  case 0x4840:
		  case 0x4841:
		  case 0x4842:
			if(Settings.SPC7110)
			{
				S9xSetSPC7110(byte, Address);
				break;
			}

		  default:
			break;
		}
	CMemory_FillRAM [Address] = byte;
}

/******************************************************************************/
/* S9xGetCPU()                                                                */
/* This function retrieves a CPU/DMA Register                                 */
/******************************************************************************/
uint8 S9xGetCPU (uint16 Address)
{
    uint8 byte;
//    fprintf(stderr, "read from %04x\n", Address);

    if (Address < 0x4200)
	{
		CPU.Cycles += ONE_CYCLE;
		switch (Address)
		{
		  case 0x4016:
			{
				if (CMemory_FillRAM [0x4016] & 1)
				{
					if ((!Settings.SwapJoypads &&
						 IPPU.Controller == SNES_MOUSE_SWAPPED) ||
						(Settings.SwapJoypads &&
						 IPPU.Controller == SNES_MOUSE))
					{
						if (++PPU.MouseSpeed [0] > 2)
							PPU.MouseSpeed [0] = 0;
					}
					return (0);
				}

				int ind = Settings.SwapJoypads ? 1 : 0;
				byte = IPPU.Joypads[ind] >> (PPU.Joypad1ButtonReadPos ^ 15);
				PPU.Joypad1ButtonReadPos++;
				return (byte & 1);
			}
		  case 0x4017:
			{
				if (CMemory_FillRAM [0x4016] & 1)
				{
					// MultiPlayer5 adaptor is only allowed to be plugged into port 2
					switch (IPPU.Controller)
					{
					  case SNES_MULTIPLAYER5:
						return (2);
					  case SNES_MOUSE_SWAPPED:
						if (Settings.SwapJoypads && ++PPU.MouseSpeed [0] > 2)
							PPU.MouseSpeed [0] = 0;
						break;

					  case SNES_MOUSE:
						if (!Settings.SwapJoypads && ++PPU.MouseSpeed [0] > 2)
							PPU.MouseSpeed [0] = 0;
						break;
					}
					return (0x00);
				}

				int ind = Settings.SwapJoypads ? 0 : 1;

				if (IPPU.Controller == SNES_MULTIPLAYER5)
				{
					if (CMemory_FillRAM [0x4201] & 0x80)
					{
						byte = ((IPPU.Joypads[ind] >> (PPU.Joypad2ButtonReadPos ^ 15)) & 1) |
							(((IPPU.Joypads[2] >> (PPU.Joypad2ButtonReadPos ^ 15)) & 1) << 1);
						PPU.Joypad2ButtonReadPos++;
						return (byte);
					}
					else
					{
						byte = ((IPPU.Joypads[3] >> (PPU.Joypad3ButtonReadPos ^ 15)) & 1) |
							(((IPPU.Joypads[4] >> (PPU.Joypad3ButtonReadPos ^ 15)) & 1) << 1);
						PPU.Joypad3ButtonReadPos++;
						return (byte);
					}
				}
				else if(IPPU.Controller ==SNES_JUSTIFIER || IPPU.Controller ==SNES_JUSTIFIER_2)
				{
					uint8 rv;
					rv=(1&(justifiers>>in_bit));
					in_bit++;
					in_bit%=32;
					return rv;
				}
				return ((IPPU.Joypads[ind] >> (PPU.Joypad2ButtonReadPos++ ^ 15)) & 1);
			}
		  default:
			return OpenBus;

		}
//		return (CMemory_FillRAM [Address]);
	}
    else
    switch (Address)
    {
	  case 0x4200:
	  case 0x4201:
	  case 0x4202:
	  case 0x4203:
	  case 0x4204:
	  case 0x4205:
	  case 0x4206:
	  case 0x4207:
	  case 0x4208:
	  case 0x4209:
	  case 0x420a:
	  case 0x420b:
	  case 0x420c:
	  case 0x420d:
	  case 0x420e:
	  case 0x420f:
		return OpenBus;

	  case 0x4210:
#ifdef CPU_SHUTDOWN
		CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif	
		byte = CMemory_FillRAM[0x4210];
		CMemory_FillRAM[0x4210] = Model->_5A22;
		//SNEeSe returns 2 for 5A22 version.
		return ((byte&0x80)|(OpenBus&0x70)|Model->_5A22);

	  case 0x4211:
		byte = (CPU.IRQActive & (PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE)) ? 0x80 : 0;
		// Super Robot Wars Ex ROM bug requires this.
		byte |= CPU.Cycles >= Settings.HBlankStart ? 0x40 : 0;
		CLEAR_IRQ_SOURCE (PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE);

		// Maybe? Register Scan indicated open bus...
		byte |= OpenBus&0x3f;

		return (byte);

	  case 0x4212:
		// V-blank, h-blank and joypads being read flags (read-only)
#ifdef CPU_SHUTDOWN
		CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif
		return (REGISTER_4212()|(OpenBus&0x3E));

	  case 0x4213:
		// I/O port input - returns 0 wherever $4201 is 0, and 1 elsewhere
		// unless something else pulls it down (i.e. a gun)
		return CMemory_FillRAM[0x4213];

	  case 0x4214:
	  case 0x4215:
		// Quotient of divide result
	  case 0x4216:
	  case 0x4217:
		// Multiplcation result (for multiply) or remainder of
		// divison.
		return (CMemory_FillRAM[Address]);
	  case 0x4218:
	  case 0x4219:
	  case 0x421a:
	  case 0x421b:
	  case 0x421c:
	  case 0x421d:
	  case 0x421e:
	  case 0x421f:
		// Joypads 1-4 button and direction state.
		return (CMemory_FillRAM [Address]);

	  case 0x4300:
	  case 0x4310:
	  case 0x4320:
	  case 0x4330:
	  case 0x4340:
	  case 0x4350:
	  case 0x4360:
	  case 0x4370:
		// DMA direction, address type, fixed flag,
		return (CMemory_FillRAM[Address]);

	  case 0x4301:
	  case 0x4311:
	  case 0x4321:
	  case 0x4331:
	  case 0x4341:
	  case 0x4351:
	  case 0x4361:
	  case 0x4371:
		return (CMemory_FillRAM[Address]);

	  case 0x4302:
	  case 0x4312:
	  case 0x4322:
	  case 0x4332:
	  case 0x4342:
	  case 0x4352:
	  case 0x4362:
	  case 0x4372:
		return (CMemory_FillRAM[Address]);

	  case 0x4303:
	  case 0x4313:
	  case 0x4323:
	  case 0x4333:
	  case 0x4343:
	  case 0x4353:
	  case 0x4363:
	  case 0x4373:
		return (CMemory_FillRAM[Address]);

	  case 0x4304:
	  case 0x4314:
	  case 0x4324:
	  case 0x4334:
	  case 0x4344:
	  case 0x4354:
	  case 0x4364:
	  case 0x4374:
		return (CMemory_FillRAM[Address]);

	  case 0x4305:
	  case 0x4315:
	  case 0x4325:
	  case 0x4335:
	  case 0x4345:
	  case 0x4355:
	  case 0x4365:
	  case 0x4375:
		return (CMemory_FillRAM[Address]);

	  case 0x4306:
	  case 0x4316:
	  case 0x4326:
	  case 0x4336:
	  case 0x4346:
	  case 0x4356:
	  case 0x4366:
	  case 0x4376:
		return (CMemory_FillRAM[Address]);

	  case 0x4307:
	  case 0x4317:
	  case 0x4327:
	  case 0x4337:
	  case 0x4347:
	  case 0x4357:
	  case 0x4367:
	  case 0x4377:
		return (DMA[(Address >> 4) & 7].IndirectBank);

	  case 0x4308:
	  case 0x4318:
	  case 0x4328:
	  case 0x4338:
	  case 0x4348:
	  case 0x4358:
	  case 0x4368:
	  case 0x4378:
		return (CMemory_FillRAM[Address]);

	  case 0x4309:
	  case 0x4319:
	  case 0x4329:
	  case 0x4339:
	  case 0x4349:
	  case 0x4359:
	  case 0x4369:
	  case 0x4379:
		return (CMemory_FillRAM[Address]);

	  case 0x430A:
	  case 0x431A:
	  case 0x432A:
	  case 0x433A:
	  case 0x434A:
	  case 0x435A:
	  case 0x436A:
	  case 0x437A:
		{
			int d = (Address & 0x70) >> 4;
			if (IPPU.HDMA & (1 << d))
			{
				return (DMA[d].LineCount);
			}
			return (CMemory_FillRAM[Address]);
		}

	  case 0x430F:
	  case 0x431F:
	  case 0x432F:
	  case 0x433F:
	  case 0x434F:
	  case 0x435F:
	  case 0x436F:
	  case 0x437F:
		Address &= ~4; // Convert 43xF to 43xB
		/* fall through */
	  case 0x430B:
	  case 0x431B:
	  case 0x432B:
	  case 0x433B:
	  case 0x434B:
	  case 0x435B:
	  case 0x436B:
	  case 0x437B:
		// Unknown, but they seem to be RAM-ish
		return (CMemory_FillRAM[Address]);
	  default:
		if(Address>= 0x4800&&Settings.SPC7110)
			return S9xGetSPC7110(Address);

		if(Address>=0x4800&&Address<=0x4807&&Settings.SDD1)
		{
			return CMemory_FillRAM[Address];
		}

		return OpenBus;
    }
//    return (CMemory_FillRAM[Address]);
}

void S9xResetPPU ()
{
	PPU.BGMode = 0;
	PPU.BG3Priority = 0;
	PPU.Brightness = 0;
	PPU.VMA.High = 0;
	PPU.VMA.Increment = 1;
	PPU.VMA.Address = 0;
	PPU.VMA.FullGraphicCount = 0;
	PPU.VMA.Shift = 0;

	for (uint8 B = 0; B != 4; B++)
	{
		PPU.BG[B].SCBase = 0;
		PPU.BG[B].VOffset = 0;
		PPU.BG[B].HOffset = 0;
		PPU.BG[B].BGSize = 0;
		PPU.BG[B].NameBase = 0;
		PPU.BG[B].SCSize = 0;

		PPU.ClipCounts[B] = 0;
		PPU.ClipWindowOverlapLogic [B] = CLIP_OR;
		PPU.ClipWindow1Enable[B] = FALSE;
		PPU.ClipWindow2Enable[B] = FALSE;
		PPU.ClipWindow1Inside[B] = TRUE;
		PPU.ClipWindow2Inside[B] = TRUE;
	}

	PPU.ClipCounts[4] = 0;
	PPU.ClipCounts[5] = 0;
	PPU.ClipWindowOverlapLogic[4] = PPU.ClipWindowOverlapLogic[5] = CLIP_OR;
	PPU.ClipWindow1Enable[4] = PPU.ClipWindow1Enable[5] = FALSE;
	PPU.ClipWindow2Enable[4] = PPU.ClipWindow2Enable[5] = FALSE;
	PPU.ClipWindow1Inside[4] = PPU.ClipWindow1Inside[5] = TRUE;
	PPU.ClipWindow2Inside[4] = PPU.ClipWindow2Inside[5] = TRUE;

	PPU.CGFLIP = 0;
	int c;
	for (c = 0; c < 256; c++)
	{
		IPPU.Red [c] = (c & 7) << 2;
		IPPU.Green [c] = ((c >> 3) & 7) << 2;
		IPPU.Blue [c] = ((c >> 6) & 2) << 3;
		PPU.CGDATA [c] = IPPU.Red [c] | (IPPU.Green [c] << 5) |
			(IPPU.Blue [c] << 10);
	}

	PPU.FirstSprite = 0;
	PPU.LastSprite = 127;
	for (int Sprite = 0; Sprite < 128; Sprite++)
	{
		PPU.OBJ[Sprite].HPos = 0;
		PPU.OBJ[Sprite].VPos = 0;
		PPU.OBJ[Sprite].VFlip = 0;
		PPU.OBJ[Sprite].HFlip = 0;
		PPU.OBJ[Sprite].Priority = 0;
		PPU.OBJ[Sprite].Palette = 0;
		PPU.OBJ[Sprite].Name = 0;
		PPU.OBJ[Sprite].Size = 0;
	}
	PPU.OAMPriorityRotation = 0;
	PPU.OAMWriteRegister = 0;
	PPU.RangeTimeOver = 0;
	PPU.OpenBus1 = 0;
	PPU.OpenBus2 = 0;

	PPU.OAMFlip = 0;
	PPU.OAMTileAddress = 0;
	PPU.OAMAddr = 0;
	PPU.IRQVBeamPos = 0;
	PPU.IRQHBeamPos = 0;
	PPU.VBeamPosLatched = 0;
	PPU.HBeamPosLatched = 0;

	PPU.HBeamFlip = 0;
	PPU.VBeamFlip = 0;
	PPU.HVBeamCounterLatched = 0;

	PPU.MatrixA = PPU.MatrixB = PPU.MatrixC = PPU.MatrixD = 0;
	PPU.CentreX = PPU.CentreY = 0;
	PPU.Joypad1ButtonReadPos = 0;
	PPU.Joypad2ButtonReadPos = 0;
	PPU.Joypad3ButtonReadPos = 0;

	PPU.CGADD = 0;
	PPU.FixedColourRed = PPU.FixedColourGreen = PPU.FixedColourBlue = 0;
	PPU.SavedOAMAddr = 0;
	PPU.ScreenHeight = SNES_HEIGHT;
	PPU.WRAM = 0;
	PPU.BG_Forced = 0;
	PPU.ForcedBlanking = TRUE;
	PPU.OBJThroughMain = FALSE;
	PPU.OBJThroughSub = FALSE;
	PPU.OBJSizeSelect = 0;
	PPU.OBJNameSelect = 0;
	PPU.OBJNameBase = 0;
	PPU.OBJAddition = FALSE;
	PPU.OAMReadFlip = 0;
	PPU.BGnxOFSbyte = 0;
	ZeroMemory (PPU.OAMData, 512 + 32);

	PPU.VTimerEnabled = FALSE;
	PPU.HTimerEnabled = FALSE;
	PPU.HTimerPosition = Settings.H_Max + 1;
	PPU.Mosaic = 0;
	PPU.BGMosaic [0] = PPU.BGMosaic [1] = FALSE;
	PPU.BGMosaic [2] = PPU.BGMosaic [3] = FALSE;
	PPU.Mode7HFlip = FALSE;
	PPU.Mode7VFlip = FALSE;
	PPU.Mode7Repeat = 0;
	PPU.Window1Left = 1;
	PPU.Window1Right = 0;
	PPU.Window2Left = 1;
	PPU.Window2Right = 0;
	PPU.RecomputeClipWindows = TRUE;
	PPU.CGFLIPRead = 0;
	PPU.Need16x8Mulitply = FALSE;
	PPU.MouseSpeed[0] = PPU.MouseSpeed[1] = 0;

	IPPU.ColorsChanged = TRUE;
	IPPU.HDMA = 0;
	IPPU.HDMAStarted = FALSE;
	IPPU.MaxBrightness = 0;
	IPPU.LatchedBlanking = 0;
	IPPU.OBJChanged = TRUE;
	IPPU.RenderThisFrame = TRUE;
#ifdef DIRECT_COLOR_MAP
	IPPU.DirectColourMapsNeedRebuild = TRUE;
#endif
	IPPU.FrameCount = 0;
	IPPU.RenderedFramesCount = 0;
	IPPU.DisplayedRenderedFrameCount = 0;
	IPPU.SkippedFrames = 0;
	IPPU.FrameSkip = 0;
	ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES);
	ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES);
	ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES);
#ifdef CORRECT_VRAM_READS
	IPPU.VRAMReadBuffer = 0; // XXX: FIXME: anything better?
#else
	IPPU.FirstVRAMRead = FALSE;
#endif
	IPPU.Interlace = FALSE;
	IPPU.InterlaceSprites = FALSE;
	IPPU.DoubleWidthPixels = FALSE;
	IPPU.DoubleHeightPixels = FALSE;
	IPPU.RenderedScreenWidth = SNES_WIDTH;
	IPPU.RenderedScreenHeight = SNES_HEIGHT;
	IPPU.XB = NULL;
	for (c = 0; c < 256; c++)
		IPPU.ScreenColors [c] = c;
	S9xFixColourBrightness ();
	IPPU.PreviousLine = IPPU.CurrentLine = 0;
	IPPU.Joypads[0] = IPPU.Joypads[1] = IPPU.Joypads[2] = 0;
	IPPU.Joypads[3] = IPPU.Joypads[4] = 0;
	IPPU.SuperScope = 0;
	IPPU.Mouse[0] = IPPU.Mouse[1] = 0;
	IPPU.PrevMouseX[0] = IPPU.PrevMouseX[1] = 256 / 2;
	IPPU.PrevMouseY[0] = IPPU.PrevMouseY[1] = 224 / 2;

	if (Settings.ControllerOption == 0)
		IPPU.Controller = SNES_MAX_CONTROLLER_OPTIONS - 1;
	else
		IPPU.Controller = Settings.ControllerOption - 1;
	S9xNextController ();

	for (c = 0; c < 2; c++)
		memset (&IPPU.Clip [c], 0, sizeof (struct ClipData));

	if (Settings.MouseMaster)
	{
		S9xProcessMouse (0);
		S9xProcessMouse (1);
	}
	for (c = 0; c < 0x8000; c += 0x100)
	{
		if ( !Settings.SuperFX )
		{
			memset (&CMemory_FillRAM [c], c >> 8, 0x100);
		}
		else if ( (unsigned)c < 0x3000 || (unsigned)c >= 0x3300 )
		{
			/* Don't overwrite SFX pvRegisters at 0x3000-0x32FF,
			 * they were set in FxReset.
			 */
			memset (&CMemory_FillRAM [c], c >> 8, 0x100);
		}
	}

	ZeroMemory (&CMemory_FillRAM [0x2100], 0x100);
	ZeroMemory (&CMemory_FillRAM [0x4200], 0x100);
	ZeroMemory (&CMemory_FillRAM [0x4000], 0x100);
	// For BS Suttehakkun 2...
	ZeroMemory (&CMemory_FillRAM [0x1000], 0x1000);

	CMemory_FillRAM[0x4201]=CMemory_FillRAM[0x4213]=0xFF;
}

void S9xSoftResetPPU ()
{
	PPU.BGMode = 0;
	PPU.BG3Priority = 0;
	PPU.Brightness = 0;
	PPU.VMA.High = 0;
	PPU.VMA.Increment = 1;
	PPU.VMA.Address = 0;
	PPU.VMA.FullGraphicCount = 0;
	PPU.VMA.Shift = 0;

	for (uint8 B = 0; B != 4; B++)
	{
		PPU.BG[B].SCBase = 0;
		PPU.BG[B].VOffset = 0;
		PPU.BG[B].HOffset = 0;
		PPU.BG[B].BGSize = 0;
		PPU.BG[B].NameBase = 0;
		PPU.BG[B].SCSize = 0;

		PPU.ClipCounts[B] = 0;
		PPU.ClipWindowOverlapLogic [B] = CLIP_OR;
		PPU.ClipWindow1Enable[B] = FALSE;
		PPU.ClipWindow2Enable[B] = FALSE;
		PPU.ClipWindow1Inside[B] = TRUE;
		PPU.ClipWindow2Inside[B] = TRUE;
	}

	PPU.ClipCounts[4] = 0;
	PPU.ClipCounts[5] = 0;
	PPU.ClipWindowOverlapLogic[4] = PPU.ClipWindowOverlapLogic[5] = CLIP_OR;
	PPU.ClipWindow1Enable[4] = PPU.ClipWindow1Enable[5] = FALSE;
	PPU.ClipWindow2Enable[4] = PPU.ClipWindow2Enable[5] = FALSE;
	PPU.ClipWindow1Inside[4] = PPU.ClipWindow1Inside[5] = TRUE;
	PPU.ClipWindow2Inside[4] = PPU.ClipWindow2Inside[5] = TRUE;

	PPU.CGFLIP = 0;
	int c;
	for (c = 0; c < 256; c++)
	{
		IPPU.Red [c] = (c & 7) << 2;
		IPPU.Green [c] = ((c >> 3) & 7) << 2;
		IPPU.Blue [c] = ((c >> 6) & 2) << 3;
		PPU.CGDATA [c] = IPPU.Red [c] | (IPPU.Green [c] << 5) |
			(IPPU.Blue [c] << 10);
	}

	PPU.FirstSprite = 0;
	PPU.LastSprite = 127;
	for (int Sprite = 0; Sprite < 128; Sprite++)
	{
		PPU.OBJ[Sprite].HPos = 0;
		PPU.OBJ[Sprite].VPos = 0;
		PPU.OBJ[Sprite].VFlip = 0;
		PPU.OBJ[Sprite].HFlip = 0;
		PPU.OBJ[Sprite].Priority = 0;
		PPU.OBJ[Sprite].Palette = 0;
		PPU.OBJ[Sprite].Name = 0;
		PPU.OBJ[Sprite].Size = 0;
	}
	PPU.OAMPriorityRotation = 0;
	PPU.OAMWriteRegister = 0;
	PPU.RangeTimeOver = 0;
	PPU.OpenBus1 = 0;
	PPU.OpenBus2 = 0;

	PPU.OAMFlip = 0;
	PPU.OAMTileAddress = 0;
	PPU.OAMAddr = 0;
	PPU.IRQVBeamPos = 0;
	PPU.IRQHBeamPos = 0;
	PPU.VBeamPosLatched = 0;
	PPU.HBeamPosLatched = 0;

	PPU.HBeamFlip = 0;
	PPU.VBeamFlip = 0;
	PPU.HVBeamCounterLatched = 0;

	PPU.MatrixA = PPU.MatrixB = PPU.MatrixC = PPU.MatrixD = 0;
	PPU.CentreX = PPU.CentreY = 0;
//	PPU.Joypad1ButtonReadPos = 0;
//	PPU.Joypad2ButtonReadPos = 0;
//	PPU.Joypad3ButtonReadPos = 0;
	PPU.CGADD = 0;
	PPU.FixedColourRed = PPU.FixedColourGreen = PPU.FixedColourBlue = 0;
	PPU.SavedOAMAddr = 0;
	PPU.ScreenHeight = SNES_HEIGHT;
	PPU.WRAM = 0;
	PPU.BG_Forced = 0;
	PPU.ForcedBlanking = TRUE;
	PPU.OBJThroughMain = FALSE;
	PPU.OBJThroughSub = FALSE;
	PPU.OBJSizeSelect = 0;
	PPU.OBJNameSelect = 0;
	PPU.OBJNameBase = 0;
	PPU.OBJAddition = FALSE;
	PPU.OAMReadFlip = 0;
	PPU.BGnxOFSbyte = 0;
	ZeroMemory (PPU.OAMData, 512 + 32);

	PPU.VTimerEnabled = FALSE;
	PPU.HTimerEnabled = FALSE;
	PPU.HTimerPosition = Settings.H_Max + 1;
	PPU.Mosaic = 0;
	PPU.BGMosaic [0] = PPU.BGMosaic [1] = FALSE;
	PPU.BGMosaic [2] = PPU.BGMosaic [3] = FALSE;
	PPU.Mode7HFlip = FALSE;
	PPU.Mode7VFlip = FALSE;
	PPU.Mode7Repeat = 0;
	PPU.Window1Left = 1;
	PPU.Window1Right = 0;
	PPU.Window2Left = 1;
	PPU.Window2Right = 0;
	PPU.RecomputeClipWindows = TRUE;
	PPU.CGFLIPRead = 0;
	PPU.Need16x8Mulitply = FALSE;
	PPU.MouseSpeed[0] = PPU.MouseSpeed[1] = 0;

	IPPU.ColorsChanged = TRUE;
	IPPU.HDMA = 0;
	IPPU.HDMAStarted = FALSE;
	IPPU.MaxBrightness = 0;
	IPPU.LatchedBlanking = 0;
	IPPU.OBJChanged = TRUE;
	IPPU.RenderThisFrame = TRUE;
#ifdef DIRECT_COLOR_MAP
	IPPU.DirectColourMapsNeedRebuild = TRUE;
#endif
	IPPU.FrameCount = 0;
	IPPU.RenderedFramesCount = 0;
	IPPU.DisplayedRenderedFrameCount = 0;
	IPPU.SkippedFrames = 0;
	IPPU.FrameSkip = 0;
	ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES);
	ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES);
	ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES);
#ifdef CORRECT_VRAM_READS
	IPPU.VRAMReadBuffer = 0; // XXX: FIXME: anything better?
#else
	IPPU.FirstVRAMRead = FALSE;
#endif
	IPPU.Interlace = FALSE;
	IPPU.InterlaceSprites = FALSE;
	IPPU.DoubleWidthPixels = FALSE;
	IPPU.DoubleHeightPixels = FALSE;
	IPPU.RenderedScreenWidth = SNES_WIDTH;
	IPPU.RenderedScreenHeight = SNES_HEIGHT;
	IPPU.XB = NULL;
	for (c = 0; c < 256; c++)
		IPPU.ScreenColors [c] = c;
	S9xFixColourBrightness ();
	IPPU.PreviousLine = IPPU.CurrentLine = 0;
//	IPPU.Joypads[0] = IPPU.Joypads[1] = IPPU.Joypads[2] = 0;
//	IPPU.Joypads[3] = IPPU.Joypads[4] = 0;
//	IPPU.SuperScope = 0;
//	IPPU.Mouse[0] = IPPU.Mouse[1] = 0;
//	IPPU.PrevMouseX[0] = IPPU.PrevMouseX[1] = 256 / 2;
//	IPPU.PrevMouseY[0] = IPPU.PrevMouseY[1] = 224 / 2;

	if (Settings.ControllerOption == 0)
		IPPU.Controller = SNES_MAX_CONTROLLER_OPTIONS - 1;
	else
		IPPU.Controller = Settings.ControllerOption - 1;
	S9xNextController ();

	for (c = 0; c < 2; c++)
		memset (&IPPU.Clip [c], 0, sizeof (struct ClipData));

	if (Settings.MouseMaster)
	{
		S9xProcessMouse (0);
		S9xProcessMouse (1);
	}
	for (c = 0; c < 0x8000; c += 0x100)
		memset (&CMemory_FillRAM [c], c >> 8, 0x100);

	ZeroMemory (&CMemory_FillRAM [0x2100], 0x100);
	ZeroMemory (&CMemory_FillRAM [0x4200], 0x100);
	ZeroMemory (&CMemory_FillRAM [0x4000], 0x100);
	// For BS Suttehakkun 2...
	ZeroMemory (&CMemory_FillRAM [0x1000], 0x1000);

	CMemory_FillRAM[0x4201]=CMemory_FillRAM[0x4213]=0xFF;
}

void S9xProcessMouse (int which1)
{
    int x, y;
    uint32 buttons;
    
    if ((IPPU.Controller == SNES_MOUSE || IPPU.Controller == SNES_MOUSE_SWAPPED) && S9xReadMousePosition (which1, x, y, buttons))
    {
		int delta_x, delta_y;
#define MOUSE_SIGNATURE 0x1
		IPPU.Mouse [which1] = MOUSE_SIGNATURE | 
			      (PPU.MouseSpeed [which1] << 4) |
		              ((buttons & 1) << 6) | ((buttons & 2) << 6);

	delta_x = x - IPPU.PrevMouseX[which1];
	delta_y = y - IPPU.PrevMouseY[which1];

	if (delta_x > 63)
	{
	    delta_x = 63;
	    IPPU.PrevMouseX[which1] += 63;
	}
	else
	if (delta_x < -63)
	{
	    delta_x = -63;
	    IPPU.PrevMouseX[which1] -= 63;
	}
	else
	    IPPU.PrevMouseX[which1] = x;

	if (delta_y > 63)
	{
	    delta_y = 63;
	    IPPU.PrevMouseY[which1] += 63;
	}
	else
	if (delta_y < -63)
	{
	    delta_y = -63;
	    IPPU.PrevMouseY[which1] -= 63;
	}
	else
	    IPPU.PrevMouseY[which1] = y;

	if (delta_x < 0)
	{
	    delta_x = -delta_x;
	    IPPU.Mouse [which1] |= (delta_x | 0x80) << 16;
	}
	else
	    IPPU.Mouse [which1] |= delta_x << 16;

	if (delta_y < 0)
	{
	    delta_y = -delta_y;
	    IPPU.Mouse [which1] |= (delta_y | 0x80) << 24;
	}
	else
	    IPPU.Mouse [which1] |= delta_y << 24;

	if (IPPU.Controller == SNES_MOUSE_SWAPPED)
	    IPPU.Joypads [0] = IPPU.Mouse [which1];
	else
	    IPPU.Joypads [1] = IPPU.Mouse [which1];
    }
}

void ProcessSuperScope ()
{
    int x, y;
    uint32 buttons;

    if (IPPU.Controller == SNES_SUPERSCOPE &&
		S9xReadSuperScopePosition (x, y, buttons))
	{
#define SUPERSCOPE_SIGNATURE 0x00ff
		uint32 scope;

		scope = SUPERSCOPE_SIGNATURE | ((buttons & 1) << (7 + 8)) |
			((buttons & 2) << (5 + 8)) | ((buttons & 4) << (3 + 8)) |
			((buttons & 8) << (1 + 8));
		if(CMemory_FillRAM[0x4201]&0x80){
			x+=40;
			if (x > 295)
				x = 295;
			if (x < 40)
				x = 40;
			if (y > PPU.ScreenHeight - 1)
				y = PPU.ScreenHeight - 1;
			if (y < 0)
				y = 0;

			PPU.VBeamPosLatched = (uint16) (y + 1);
			PPU.HBeamPosLatched = (uint16) x;
			PPU.HVBeamCounterLatched = TRUE;
			CMemory_FillRAM [0x213F] |= 0x40 | Model->_5C78;
		}
		IPPU.Joypads [1] = scope;
	}
}

void S9xNextController ()
{
    switch (IPPU.Controller)
    {
	  case SNES_MULTIPLAYER5:
		IPPU.Controller = SNES_JOYPAD;
		break;
	  case SNES_JOYPAD:
		if (Settings.MouseMaster)
		{
			IPPU.Controller = SNES_MOUSE_SWAPPED;
			break;
		}
	  case SNES_MOUSE_SWAPPED:
		if (Settings.MouseMaster)
		{
			IPPU.Controller = SNES_MOUSE;
			break;
		}
	  case SNES_MOUSE:
		if (Settings.SuperScopeMaster)
		{
			IPPU.Controller = SNES_SUPERSCOPE;
			break;
		}
	  case SNES_SUPERSCOPE:
		if (Settings.JustifierMaster)
		{
			IPPU.Controller = SNES_JUSTIFIER;
			break;
		}
	  case SNES_JUSTIFIER:
		if(Settings.JustifierMaster)
		{
			IPPU.Controller=SNES_JUSTIFIER_2;
			break;
		}
	  case SNES_JUSTIFIER_2:
		if(Settings.MultiPlayer5Master)
		{
			IPPU.Controller=SNES_MULTIPLAYER5;
			break;
		}
	  default:
		IPPU.Controller = SNES_JOYPAD;
		break;
    }
}

void S9xUpdateJustifiers()
{
	static bool last_p1;
	in_bit=0;
//	static int p1count;
	justifiers=0xFFFF00AA;

	bool offscreen=JustifierOffscreen();

	JustifierButtons(justifiers);
//	if(p1count==32)
//	{
		last_p1=!last_p1;
//		p1count=0;
//	}
//	p1count++;

	if(!last_p1)
		justifiers|=0x1000;

	int x,y;
	uint32 buttons;

	if(CMemory_FillRAM[0x4201]&0x80)
	{
		
		S9xReadSuperScopePosition(x,y,buttons);
		
		x+=40;
		if (x > 295)
			x = 295;
		if (x < 40)
			x = 40;
		if (y > PPU.ScreenHeight - 1)
			y = PPU.ScreenHeight - 1;
		if (y < 0)
			y = 0;
		
		if(last_p1)
		{
			
			PPU.HVBeamCounterLatched = FALSE;
			CMemory_FillRAM [0x213F] = Model->_5C78;
			
			//process latch as Justifier 2
			if(Settings.SecondJustifier)
			{
				if(IPPU.Controller==SNES_JUSTIFIER_2)
				{
					if(!offscreen)
					{
						
						PPU.VBeamPosLatched = (uint16) (y + 1);
						PPU.HBeamPosLatched = (uint16) x;
						PPU.HVBeamCounterLatched = TRUE;
						CMemory_FillRAM [0x213F] |= 0x40 | Model->_5C78;
					}
				}
			}
		}
		else
		{
			
			PPU.HVBeamCounterLatched = FALSE;
			CMemory_FillRAM [0x213F] = Model->_5C78;
			
			//emulate player 1.
			if(IPPU.Controller==SNES_JUSTIFIER)
			{
				if(!offscreen)
				{
					PPU.VBeamPosLatched = (uint16) (y + 1);
					PPU.HBeamPosLatched = (uint16) x;
					PPU.HVBeamCounterLatched = TRUE;
					CMemory_FillRAM [0x213F] |= 0x40 | Model->_5C78;
				}
			}
		}
		
		//needs restructure
		if(!offscreen)
		{
			
			if((!last_p1&&IPPU.Controller==SNES_JUSTIFIER)||(last_p1&&IPPU.Controller==SNES_JUSTIFIER_2))
			{
				PPU.VBeamPosLatched = (uint16) (y + 1);
				PPU.HBeamPosLatched = (uint16) x;
				PPU.HVBeamCounterLatched = TRUE;
				CMemory_FillRAM [0x213F] |= 0x40 | Model->_5C78;
			}
			else
			{
				PPU.HVBeamCounterLatched = FALSE;
				CMemory_FillRAM [0x213F] = Model->_5C78;
				
			}
		}
		else
		{
			PPU.HVBeamCounterLatched = FALSE;
			CMemory_FillRAM [0x213F] = Model->_5C78;
		}
	}
}

void S9xUpdateJoypads ()
{
    int i;

	for (i = 0; i < 5; i++)
	{
		IPPU.Joypads [i] = S9xReadJoypad (i);
		if (IPPU.Joypads [i] & SNES_LEFT_MASK)
			IPPU.Joypads [i] &= ~SNES_RIGHT_MASK;
		if (IPPU.Joypads [i] & SNES_UP_MASK)
			IPPU.Joypads [i] &= ~SNES_DOWN_MASK;
	}
	
    // BJ: This is correct behavior AFAICT (used to be Touhaiden hack)
    if (IPPU.Controller == SNES_JOYPAD || IPPU.Controller == SNES_MULTIPLAYER5)
    {
		for (i = 0; i < 5; i++)
		{
			if (IPPU.Joypads [i])
				IPPU.Joypads [i] |= 0xffff0000;
		}
    }
	
    // Read mouse position if enabled
    if (Settings.MouseMaster)
    {
		for (i = 0; i < 2; i++)
			S9xProcessMouse (i);
    }
	
    // Read SuperScope if enabled
    if (Settings.SuperScopeMaster)
		ProcessSuperScope ();
	
    if (CMemory_FillRAM [0x4200] & 1)
    {
		PPU.Joypad1ButtonReadPos = 16;
		if (CMemory_FillRAM [0x4201] & 0x80)
		{
			PPU.Joypad2ButtonReadPos = 16;
			PPU.Joypad3ButtonReadPos = 0;
		}
		else
		{
			PPU.Joypad2ButtonReadPos = 0;
			PPU.Joypad3ButtonReadPos = 16;
		}
		int ind = Settings.SwapJoypads ? 1 : 0;
		
		CMemory_FillRAM [0x4218] = (uint8) IPPU.Joypads [ind];
		CMemory_FillRAM [0x4219] = (uint8) (IPPU.Joypads [ind] >> 8);
		CMemory_FillRAM [0x421a] = (uint8) IPPU.Joypads [ind ^ 1];
		CMemory_FillRAM [0x421b] = (uint8) (IPPU.Joypads [ind ^ 1] >> 8);
		if (CMemory_FillRAM [0x4201] & 0x80)
		{
			CMemory_FillRAM [0x421c] = (uint8) IPPU.Joypads [ind];
			CMemory_FillRAM [0x421d] = (uint8) (IPPU.Joypads [ind] >> 8);
			CMemory_FillRAM [0x421e] = (uint8) IPPU.Joypads [2];
			CMemory_FillRAM [0x421f] = (uint8) (IPPU.Joypads [2] >> 8);
		}
		else
		{
			CMemory_FillRAM [0x421c] = (uint8) IPPU.Joypads [3];
			CMemory_FillRAM [0x421d] = (uint8) (IPPU.Joypads [3] >> 8);
			CMemory_FillRAM [0x421e] = (uint8) IPPU.Joypads [4];
			CMemory_FillRAM [0x421f] = (uint8) (IPPU.Joypads [4] >> 8);
		}
    }
	if(Settings.Justifier||Settings.SecondJustifier)
	{
		CMemory_FillRAM [0x421a] = 0x0E;
		CMemory_FillRAM [0x421b] = 0;
		S9xUpdateJustifiers();
	}
 
}

#ifndef ZSNES_FX
void S9xSuperFXExec ()
{
//printf("FX:%i (%i)\n",Settings.SuperFX,(unsigned)&Settings.SuperFX-(unsigned)&Settings);
#if 1
    if (Settings.SuperFX)
    {
	if ((CMemory_FillRAM [0x3000 + GSU_SFR] & FLG_G) &&
	    (CMemory_FillRAM [0x3000 + GSU_SCMR] & 0x18) == 0x18)
	{
	    if (!Settings.WinterGold||Settings.StarfoxHack)
		FxEmulate (~0);
	    else
		FxEmulate ((CMemory_FillRAM [0x3000 + GSU_CLSR] & 1) ? 700 : 350);
	    int GSUStatus = CMemory_FillRAM [0x3000 + GSU_SFR] |
			    (CMemory_FillRAM [0x3000 + GSU_SFR + 1] << 8);
	    if ((GSUStatus & (FLG_G | FLG_IRQ)) == FLG_IRQ)
	    {
		// Trigger a GSU IRQ.
		S9xSetIRQ (GSU_IRQ_SOURCE);
	    }
	}
    }
#else
    uint32 tmp =  (CMemory_FillRAM[0x3034] << 16) + *(uint16 *) &CMemory_FillRAM [0x301e];

#if 0
    if (tmp == 0x018428)
    {
	*(uint16 *) &SRAM [0x0064] = 0xbc00;
	*(uint16 *) &SRAM [0x002c] = 0x8000;
    }
#endif
    if (tmp == -1)//0x018428) //0x01bfc3) //0x09edaf) //-1) //0x57edaf)
    {
	while (CMemory_FillRAM [0x3030] & 0x20)
	{
	    int i;
	    int32 vError;
	    uint8 avReg[0x40];
	    char tmp[128];
	    uint8 vPipe;
	    uint8 vColr;
	    uint8 vPor;

	    FxPipeString (tmp);
	    /* Make the string 32 chars long */
	    if(strlen(tmp) < 32) { memset(&tmp[strlen(tmp)],' ',32-strlen(tmp)); tmp[32] = 0; }

	    /* Copy registers (so we can see if any changed) */
	    vColr = FxGetColorRegister();
	    vPor = FxGetPlotOptionRegister();
	    memcpy(avReg,SuperFX.pvRegisters,0x40);

	    /* Print the pipe string */
	    printf(tmp);

	    /* Execute the instruction in the pipe */
	    vPipe = FxPipe();
	    vError = FxEmulate(1);

	    /* Check if any registers changed (and print them if they did) */
	    for(i=0; i<16; i++)
	    {
		uint32 a = 0;
		uint32 r1 = ((uint32)avReg[i<<1]) | (((uint32)avReg[(i<<1)+1])<<8);
		uint32 r2 = (uint32)(SuperFX.pvRegisters[i<<1]) | (((uint32)SuperFX.pvRegisters[(i<<1)+1])<<8);
		if(i==15)
		    a = OPCODE_BYTES(vPipe);
		if(((r1+a)&0xffff) != r2)
		    printf(" r%d=$%04x",i,r2);
	    }
	    {
		/* Check SFR */
		uint32 r1 = ((uint32)avReg[0x30]) | (((uint32)avReg[0x31])<<8);
		uint32 r2 = (uint32)(SuperFX.pvRegisters[0x30]) | (((uint32)SuperFX.pvRegisters[0x31])<<8);
		if((r1&(1<<1)) != (r2&(1<<1)))
		    printf(" Z=%d",(uint32)(!!(r2&(1<<1))));
		if((r1&(1<<2)) != (r2&(1<<2)))
		    printf(" CY=%d",(uint32)(!!(r2&(1<<2))));
		if((r1&(1<<3)) != (r2&(1<<3)))
		    printf(" S=%d",(uint32)(!!(r2&(1<<3))));
		if((r1&(1<<4)) != (r2&(1<<4)))
		    printf(" OV=%d",(uint32)(!!(r2&(1<<4))));
		if((r1&(1<<5)) != (r2&(1<<5)))
		    printf(" G=%d",(uint32)(!!(r2&(1<<5))));
		if((r1&(1<<6)) != (r2&(1<<6)))
		    printf(" R=%d",(uint32)(!!(r2&(1<<6))));
		if((r1&(1<<8)) != (r2&(1<<8)))
		    printf(" ALT1=%d",(uint32)(!!(r2&(1<<8))));
		if((r1&(1<<9)) != (r2&(1<<9)))
		    printf(" ALT2=%d",(uint32)(!!(r2&(1<<9))));
		if((r1&(1<<10)) != (r2&(1<<10)))
		    printf(" IL=%d",(uint32)(!!(r2&(1<<10))));
		if((r1&(1<<11)) != (r2&(1<<11)))
		    printf(" IH=%d",(uint32)(!!(r2&(1<<11))));
		if((r1&(1<<12)) != (r2&(1<<12)))
		    printf(" B=%d",(uint32)(!!(r2&(1<<12))));
		if((r1&(1<<15)) != (r2&(1<<15)))
		    printf(" IRQ=%d",(uint32)(!!(r2&(1<<15))));
	    }
	    {
		/* Check PBR */
		uint32 r1 = ((uint32)avReg[0x34]);
		uint32 r2 = (uint32)(SuperFX.pvRegisters[0x34]);
		if(r1 != r2)
		    printf(" PBR=$%02x",r2);
	    }
	    {
		/* Check ROMBR */
		uint32 r1 = ((uint32)avReg[0x36]);
		uint32 r2 = (uint32)(SuperFX.pvRegisters[0x36]);
		if(r1 != r2)
		    printf(" ROMBR=$%02x",r2);
	    }
	    {
		/* Check RAMBR */
		uint32 r1 = ((uint32)avReg[0x3c]);
		uint32 r2 = (uint32)(SuperFX.pvRegisters[0x3c]);
		if(r1 != r2)
		    printf(" RAMBR=$%02x",r2);
	    }
	    {
		/* Check CBR */
		uint32 r1 = ((uint32)avReg[0x3e]) | (((uint32)avReg[0x3f])<<8);
		uint32 r2 = (uint32)(SuperFX.pvRegisters[0x3e]) | (((uint32)SuperFX.pvRegisters[0x3f])<<8);
		if(r1 != r2)
		    printf(" CBR=$%04x",r2);
	    }
	    {
		/* Check COLR */
		if(vColr != FxGetColorRegister())
		    printf(" COLR=$%02x",FxGetColorRegister());
	    }
	    {
		/* Check POR */
		if(vPor != FxGetPlotOptionRegister())
		    printf(" POR=$%02x",FxGetPlotOptionRegister());
	    }
	    printf ("\n");
	}
	S9xExit ();
    }
    else
    {
	uint32 t = (CMemory_FillRAM [0x3034] << 16) +
		   (CMemory_FillRAM [0x301f] << 8) +
		   (CMemory_FillRAM [0x301e] << 0);

printf ("%06x: %d\n", t, FxEmulate (2000000));
//	FxEmulate (2000000);
    }
#if 0
    if (!(CPU.Flags & TRACE_FLAG))
    {
	static int z = 1;
	if (z == 0)
	{
	    extern FILE *trace;
	    CPU.Flags |= TRACE_FLAG;
	    trace = fopen ("trace.log", "wb");
	}
	else
	z--;
    }
#endif
    CMemory_FillRAM [0x3030] &= ~0x20;
    if (CMemory_FillRAM [0x3031] & 0x80)
    {
	S9xSetIRQ (GSU_IRQ_SOURCE);
    }
#endif
}
#endif

