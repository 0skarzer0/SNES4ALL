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
#include "cpuops.h"
#include "ppu.h"
#include "cpuexec.h"
#include "debug.h"
#include "snapshot.h"
#include "gfx.h"
#include "missing.h"
#include "apu.h"
#include "dma.h"
#include "fxemu.h"
#include "sa1.h"
#include "spc7110.h"
#include "snes4all.h"

extern "C" void S9xSaveStateProcess();

#ifdef DEBUG_CPU
extern int cpu_debugging;
#endif

#ifndef USE_OPC_ASM_MIXTO
void S9xMainLoop (void)
{
    for (;;)
    {
#ifdef DEBUG_MAXCOUNT
      CPU.GlobalLoopCount++;
      if(Settings.MaxCount && CPU.GlobalLoopCount == Settings.MaxCount) {
        fprintf(stderr, "Max loop count reached: %ld \n", Settings.MaxCount);
        S9xExit();
      }
#endif

#ifdef USE_ALWAYS_APU_SYNC
	APU_EXECUTE ();
#else
	if (snes4all_sound_enable>1) {
		APU_EXECUTE ();
	}
#endif
	if (CPU.Flags)
	{
	    if (CPU.Flags & NMI_FLAG)
	    {
		if (--CPU.NMICycleCount == 0)
		{
		    CPU.Flags &= ~NMI_FLAG;
		    if (CPU.WaitingForInterrupt)
		    {
			CPU.WaitingForInterrupt = FALSE;
			CPU.PC++;
		    }
		    S9xOpcode_NMI ();
		}
	    }

	    CHECK_SOUND ();

	    if (CPU.Flags & IRQ_PENDING_FLAG)
	    {
		if (CPU.IRQCycleCount == 0)
		{
		    if (CPU.WaitingForInterrupt)
		    {
			CPU.WaitingForInterrupt = FALSE;
			CPU.PC++;
		    }
		    if (CPU.IRQActive && !Settings.DisableIRQ)
		    {
			if (!CheckFlag (IRQ))
			    S9xOpcode_IRQ ();
		    }
		    else
			CPU.Flags &= ~IRQ_PENDING_FLAG;
		}
		else
                {
                    if(--CPU.IRQCycleCount==0 && CheckFlag (IRQ))
                        CPU.IRQCycleCount=1;
                }
	    }
	}
#ifdef DEBUG_CPU
	if (cpu_debugging)
		puts("/");
#endif
       {
	unsigned timeslice=snes4all_timeslice;
	if (CPU.Flags) timeslice=1;
	while(timeslice--) {
#ifdef CPU_SHUTDOWN
		CPU.PCAtOpcodeStart = CPU.PC;
#endif
#ifdef USE_MEMORY_SPEED
		CPU.Cycles += CPU.MemSpeed;
#else
		CPU.Cycles += 8;
#endif

#ifdef DEBUG_CPU
		if (cpu_debugging) {
    			S9xPackStatus ();
			printf("PC=%.4X, Opcode=%.2X %s\n",CPU.PC - CPU.PCBase,*CPU.PC,ICPU.S9xOpcodes==S9xOpcodesE1?"E1":ICPU.S9xOpcodes==S9xOpcodesM1X1?"M1X1":ICPU.S9xOpcodes==S9xOpcodesM1X0?"M1X0":ICPU.S9xOpcodes==S9xOpcodesM0X0?"M0X0":ICPU.S9xOpcodes==S9xOpcodesM0X1?"M0X1":"???");
			printf("\tPB=%.2X DB=%.2X P=%.4X A=%.4X D=%.4X S=%.4X X=%.4X Y=%.4X\n",Registers.PB,Registers.DB,Registers.P.W,Registers.A.W,Registers.D.W,Registers.S.W,Registers.X.W,Registers.Y.W);
		}
#endif
		(*ICPU.S9xOpcodes [*CPU.PC++].S9xOpcode) ();
#ifdef DEBUG_CPU
		if (cpu_debugging)
			puts(".");
#endif
	}
       }
#ifdef DEBUG_CPU
	if (cpu_debugging)
		puts("\\");
#endif
	
	if (SA1.Executing) {
#ifdef DEBUG_CPU
		if (cpu_debugging)  {
//			S9xSA1PackStatus();
			printf("/SA1 PC=%.4X, Opcode=%.2X %s\n",SA1.PC - SA1.PCBase,*SA1.PC,SA1.S9xOpcodes==S9xSA1OpcodesM1X1?"M1X1":SA1.S9xOpcodes==S9xSA1OpcodesM1X0?"M1X0":SA1.S9xOpcodes==S9xSA1OpcodesM0X0?"M0X0":SA1.S9xOpcodes==S9xSA1OpcodesM0X1?"M0X1":"???");
			printf("\tPB=%.2X DB=%.2X P=%.4X A=%.4X D=%.4X S=%.4X X=%.4X Y=%.4X\n",SA1Registers.PB,SA1Registers.DB,SA1Registers.P.W,SA1Registers.A.W,SA1Registers.D.W,SA1Registers.S.W,SA1Registers.X.W,SA1Registers.Y.W);
		}
#endif
		S9xSA1MainLoop ();
#ifdef DEBUG_CPU
		if (cpu_debugging)
			printf("SA1\\ PC=%.4X\n",SA1.PC - SA1.PCBase);
#endif
	}
	DO_HBLANK_CHECK();
    }
}

void S9xSetIRQ (uint32 source)
{
    CPU.IRQActive |= source;
    CPU.Flags |= IRQ_PENDING_FLAG;
    CPU.IRQCycleCount = 3;
    if (CPU.WaitingForInterrupt)
    {
	// Force IRQ to trigger immediately after WAI - 
	// Final Fantasy Mystic Quest crashes without this.
	CPU.IRQCycleCount = 0;
	CPU.WaitingForInterrupt = FALSE;
	CPU.PC++;
    }
}

void S9xClearIRQ (uint32 source)
{
    CLEAR_IRQ_SOURCE (source);
}


void S9xDoHBlankProcessing ()
{
    snes4all_prof_start(1);
#ifdef CPU_SHUTDOWN
    CPU.WaitCounter++;
#endif
#ifndef USE_OPC_ASM_MIXTO
    S9xUpdateAPUTimer();
#endif
    switch (CPU.WhichEvent)
    {
    case HBLANK_START_EVENT:
// .hblank_start:
	if (IPPU.HDMA && CPU.V_Counter <= PPU.ScreenHeight)
	    IPPU.HDMA = S9xDoHDMA (IPPU.HDMA);

	break;

    case HBLANK_END_EVENT:
// .hblank_end:
	S9xSuperFXExec ();

#ifndef USE_SOUND_DESYNC
	{
		static int valor=0;
		valor++;
		if (valor>=snes4all_sound_throttle) {
			S9xGenerateSound ();
			valor=0;
		}
	}
#endif

	CPU.Cycles -= Settings.H_Max;
	IAPU.NextAPUTimerPos -= snes4all_apu_hmax;
	if (IAPU.APUExecuting)
	{
	    APU.Cycles -= Settings.H_Max;
#ifdef MK_APU
		S9xCatchupCount();
#endif
	}
	else
	    APU.Cycles = 0;

	CPU.NextEvent = -1;
	ICPU.Scanline++;

	CPU.V_Counter=snes4all_vcounter[CPU.V_Counter];
	if (CPU.V_Counter >= snes4all_max_vcounter)
	{
#ifdef USE_OLD_SYNC_SPEED
		CPU.V_Counter = 0;
		CMemory_FillRAM[0x213F]^=0x80;
		PPU.RangeTimeOver = 0;
		CPU.NMIActive = FALSE;
		ICPU.Frame++;
		PPU.HVBeamCounterLatched = 0;
#endif
	    	S9xSyncSpeed ();
	}
	else
// L162:
	if (CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)
	{
	    // Start of V-blank
	    S9xEndScreenRefresh ();
#ifdef USE_OLD_END_REFRESH
	    IPPU.HDMA = 0;
	    // Bits 7 and 6 of $4212 are computed when read in S9xGetPPU.
	    IPPU.MaxBrightness = PPU.Brightness;
	    PPU.ForcedBlanking = (CMemory_FillRAM [0x2100] >> 7) & 1;

		if(!PPU.ForcedBlanking){
			PPU.OAMAddr = PPU.SavedOAMAddr;
			{
				uint8 tmp = 0;
				if(PPU.OAMPriorityRotation)
					tmp = (PPU.OAMAddr&0xFE)>>1;
				if((PPU.OAMFlip&1) || PPU.FirstSprite!=tmp){
					PPU.FirstSprite=tmp;
					IPPU.OBJChanged=TRUE;
				}
			}
			PPU.OAMFlip = 0;
		}

	    CMemory_FillRAM[0x4210] = 0x80 |Model->_5A22;
	    if (CMemory_FillRAM[0x4200] & 0x80)
	    {
			CPU.NMIActive = TRUE;
			CPU.Flags |= NMI_FLAG;
			CPU.NMICycleCount = CPU.NMITriggerPoint;
	    }
#endif

#ifdef OLD_SNAPSHOT_CODE
	    if (CPU.Flags & SAVE_SNAPSHOT_FLAG)
		{
			CPU.Flags &= ~SAVE_SNAPSHOT_FLAG;
			Registers.PC = CPU.PC - CPU.PCBase;
			S9xPackStatus ();
			S9xAPUPackStatus ();
			Snapshot (NULL);
		}
#else	
		{
			extern int savestate_state;
			if (savestate_state)
				S9xSaveStateProcess();
		}
#endif
	}

// L161:
	if (PPU.VTimerEnabled && !PPU.HTimerEnabled &&
	    CPU.V_Counter == PPU.IRQVBeamPos)
	{
// L163:
	    S9xSetIRQ (PPU_V_BEAM_IRQ_SOURCE);
	}

// L165:
	if (CPU.V_Counter == FIRST_VISIBLE_LINE)
	{
#ifdef USE_OLD_START_REFRESH
	    S9xUpdateJoypads ();
	    CMemory_FillRAM[0x4210] = Model->_5A22;
	    CPU.Flags &= ~NMI_FLAG;
#endif
	    S9xStartScreenRefresh ();
	}
// L177:
	if (CPU.V_Counter >= FIRST_VISIBLE_LINE &&
	    CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE)
	{
	    RenderLine (CPU.V_Counter - FIRST_VISIBLE_LINE);
	}
	break;

    case HTIMER_BEFORE_EVENT:
    case HTIMER_AFTER_EVENT:
	if (PPU.HTimerEnabled &&
	    (!PPU.VTimerEnabled || CPU.V_Counter == PPU.IRQVBeamPos))
	{
	    S9xSetIRQ (PPU_H_BEAM_IRQ_SOURCE);
	}
	break;
    }
    S9xReschedule ();
#ifdef DEBUG_CPU
    if (cpu_debugging) {
	    printf("HSYNC, APU Cyles=%u, PC=%.4X\n\tS=%.2X, X=%.2X, P=%.2X, A=%.2X, Y=%.2X\n",APU.Cycles,APURegisters.PC,APURegisters.S,APURegisters.X,APURegisters.P,APURegisters.YA.B.A,APURegisters.YA.B.Y);
    }
#endif
    snes4all_prof_end(1);
}

#endif // USE_OPC_ASM_MIXTO
