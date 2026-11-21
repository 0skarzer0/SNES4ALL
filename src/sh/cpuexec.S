/*** Renesas SH assembler *****************************************************
  SH assembler code partly based on x86 assembler code

  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se)
******************************************************************************/

/******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2003 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2003 Matthew Kendora and
                            Brad Jorsch (anomie@users.sourceforge.net)
 

                      
  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and
                            Nach (n-a-c-h@users.sourceforge.net)
                                          
  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2003 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman (jweidman@slip.net),
                            neviksti (neviksti@hotmail.com), and
                            Kris Bleakley (stinkfish@bigpond.com)
 
  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2003 zsKnight, pagefault (pagefault@zsnes.com)
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar and Gary Henderson.



 
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
******************************************************************************/
.text
.little
#include "asmstruc.h"
#include "asmops.h"
#include "spcops.h"
#include "aputimer.h"

#ifdef USE_OPC_ASM_MIXTO
.extern _S9xDoHBlankProcessing
#endif

.macro	RUN_CPU
	mov.l ICPU,r1
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed, r2
	mov.b @PC,r0
	add r2,CYCLES
	extu.b r0,r0
#else
	mov.b @PC,r0
	add #8,CYCLES
	extu.b r0,r0
#endif
#else
	mov.b @PC,r0
	mov.l CPUSpeed,r4
	extu.b r0,r0
	mov.b @(r0,r4),r2
	add r2,CYCLES
#endif
	mov.l CPUOpcodes,r2
	shll2 r0
	mov.l @(r0,r2),r0
#ifdef CPU_SHUTDOWN
	mov.l PC, PCAtOpcodeStart
#endif	
	jmp @r0
	add #1,PC
.endm



.globl MainAsmLoop

.text
	.align 5
.globl _S9xMainLoop
_S9xMainLoop:
	mov.l r14,@-r15
	sts.l pr,@-r15
	mov.l r13,@-r15
	mov.l r12,@-r15
	mov.l r11,@-r15
	mov.l r10,@-r15
	mov.l r9,@-r15
	mov.l r8,@-r15
	SETUP_PREDEF
	LOAD_REGISTERS MainLoop
#ifndef USE_REGISTER_LEFT_TIMESLICE
	mov.l _snes4all_timeslice,r1
	mova left_timeslice,r0
	bra .L9
	mov.l r1,@r0
#else
	mov.l _snes4all_timeslice, USE_REGISTER_LEFT_TIMESLICE
	bra .L9
	nop
#endif
	
	.align 5
MainAsmLoop:
#ifndef USE_REGISTER_LEFT_TIMESLICE
	mova left_timeslice,r0
	mov.l @r0,r1
	add #-1,r1
	tst r1,r1
	bt/s .L8
	mov.l r1,@r0

	RUN_CPU
.L8:
	mov.l _snes4all_timeslice,r1
	mov.l r1,@r0
#else
	add #-1, USE_REGISTER_LEFT_TIMESLICE
	tst USE_REGISTER_LEFT_TIMESLICE, USE_REGISTER_LEFT_TIMESLICE
	bt .L8
	RUN_CPU
.L8:	
#endif
	extu.b r0,r0
	mov.l SA1,r1
	mov.b SA1Executing,r0
	tst r0,r0
	bt .nosa1
	STORE_REGISTERS_i	MAL1
	mov.l .S9xSA1MainLoop,r3
	jsr @r3
	STORE_REGISTERS_f
	LOAD_REGISTERS MAL1
.nosa1:
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l NextEvent,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif

	cmp/ge r0,CYCLES
	bf .L9
	STORE_REGISTERS_i MAL2
#ifdef USE_OPC_ASM_MIXTO
#ifdef ASMSTRUC_NOP_PATCH
	STORE_REGISTERS_f
#endif
	UPDATE_APUTIMER _1
	mov.l ._S9xDoHBlankProcessing,r0
	jsr @r0
	nop
#else
#ifndef USE_ALWAYS_APU_SYNC
	mov.l .S9xApuExecute,r1
	jsr @r1
	STORE_REGISTERS_f
	bsr S9xDoHBlankProcessing
	nop
#else
	bsr S9xDoHBlankProcessing
	STORE_REGISTERS_f
#endif
#endif
	LOAD_REGISTERS MAL2

.L9:
#ifndef USE_ALWAYS_APU_SYNC
	mov.l _snes4all_sound_enable,r0
	mov #1,r1
	cmp/hi r1,r0
	bf .L12
#endif
#ifndef ARMAPU
	extu.b r0,r0
	mov.l IAPU,r1
	mov.b APUExecuting,r0
	tst r0,r0
	bt .L12

#ifndef USE_OLD_APUEXEC
	mov.l APU,r1
	mov.l APUCycles,r0
	cmp/gt CYCLES,r0
	bt .L12

	SAVE_CYCLES
	PUSH_REGISTERS
	mov.l .S9xApuExecute,r0
	jsr @r0
	nop
	POP_REGISTERS
	LOAD_CYCLES
#else

.apuloop:
	mov.l APU,r1
	mov.l APUCycles,r0
	cmp/gt CYCLES,r0
	bt .L12
#ifdef SPC700_C
	mov.l IAPU,r1
	SAVE_CYCLES
	mov.l APUPC,r0

	PUSH_REGISTERS
	mov.b @r0,r0
#else
	mov.b @APUPC,r0
#endif
	mov.l .S9xAPUCycles,r1
	extu.b r0,r0
	mov.l .S9xApuOpcodes,r2
	shll2 r0
	mov.l @(r0,r1),r4
	mov.l @(r0,r2),r0
	mov.l APU,r1
	mov.l APUCycles,r2
	add r4,r2

	jsr @r0
	mov.l r2,APUCycles
#ifdef SPC700_C
	POP_REGISTERS

	bra .apuloop
	LOAD_CYCLES
#else
	bra .apuloop
	nop
#endif
#endif

	.align 2
#endif	/* ARMAPU */

.L12:
	mov.l Flags,r0
	tst r0,r0
	bt .L15
	tst #128,r0
	bt .NO_NMI
	mov.l NMICycleCount,r1
	dt r1
	bf/s .NO_NMI
	mov.l r1,NMICycleCount
	add #-NMI_FLAG,r0
	mov.l r0,Flags
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WaitingForInterrupt,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .L17
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	add #1,PC
.L17:
	mov.l S9xOpcode_NMI,r0
	jsr @r0
	nop
	mov.l Flags,r0
.NO_NMI:
	mov.l .pIRQ_PENDING_FLAG,r1
	tst r1, r0
	bt .L15
	mov.l IRQCycleCount,r0
	tst r0,r0
	bf/s .DEC_IRQ_CYCLE_COUNT
	add #-1,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WaitingForInterrupt,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	extu.b r0,r0
	tst r0,r0
	bt .NOT_WAITING
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	add #1,PC
.NOT_WAITING:	
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b IRQActive,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .CLEAR_PENDING_IRQ_FLAG
	/* XXX:	test for Settings.DisableIRQ */
	mov #IRQ,r0
	tst r0,FLAGS
	bf .L15
	mov.l S9xOpcode_IRQ,r3
	jsr @r3
	nop
	bra .L15
	nop
.CLEAR_PENDING_IRQ_FLAG:
	mov.l Flags,r0
	mov.l .nIRQ_PENDING_FLAG,r1
	and r1,r0
	bra .L15
	mov.l r0,Flags

	.align 2
#ifndef USE_REGISTER_LEFT_TIMESLICE
.global left_timeslice
left_timeslice:
	.long 0
#endif
._S9xDoHBlankProcessing:
	.long _S9xDoHBlankProcessing
#ifdef USE_OPC_ASM_MIXTO
.S9xUpdateAPUTimer_CONST00002710:
        .long 0x00002710
.S9xUpdateAPUTimer_CONST003334bb:
	.long 0x003334bb
.S9xUpdateAPUTimer_APUTimer_ptr:
	.long _APU+APUTimer
.S9xUpdateAPUTimer_CONST000000ff:
	.long 0x000000ff
.S9xUpdateAPUTimer_CONST000000fd:
	.long 0x000000fd
.S9xUpdateAPUTimer_CONST000000fe:
	.long 0x000000fe
#endif
.pIRQ_PENDING_FLAG:
	.long IRQ_PENDING_FLAG
.nIRQ_PENDING_FLAG:
	.long ~IRQ_PENDING_FLAG


	.align 2
.DEC_IRQ_CYCLE_COUNT:
	mov.l r0,IRQCycleCount

.L15:
#ifdef USE_REGISTER_LEFT_TIMESLICE
	mov.l _snes4all_timeslice, USE_REGISTER_LEFT_TIMESLICE
#else
	mov.l _snes4all_timeslice,r1
#endif
	mov.l Flags,r0
	tst r0,r0
	bt .L16
#ifdef USE_REGISTER_LEFT_TIMESLICE
	mov #1, USE_REGISTER_LEFT_TIMESLICE
#else
	mov #1, r1
#endif

.L16:
#ifndef USE_REGISTER_LEFT_TIMESLICE
	mova left_timeslice,r0
	mov.l r1,@r0
#endif
	RUN_CPU

	.align 2
.globl _snes4all_timeslice
_snes4all_timeslice: .long 4
.S9xSA1MainLoop:
	.long _S9xSA1MainLoop

	.align 2
	
#ifndef ARMAPU
#ifdef USE_OLD_APUEXEC
.S9xAPUCycles:
	.long _S9xAPUCycles
.S9xApuOpcodes:
	.long _S9xApuOpcodes
#ifndef USE_ALWAYS_APU_SYNC
.S9xApuExecute:
	.long _S9xApuExecute
#endif
#else
.S9xApuExecute:
	.long _S9xApuExecute
#endif
#endif	/* ARMAPU */
#ifndef USE_ALWAYS_APU_SYNC
.global _snes4all_sound_enable
_snes4all_sound_enable:
#if defined(DEBUG_TILECACHE) || defined(PROFILER_SNES4ALL)
	.long 0
#else
	.long 1
#endif
#endif

	.align 2
	
#ifndef USE_OPC_ASM_MIXTO
.globl _S9xDoHBlankProcessing
_S9xDoHBlankProcessing:
	mov.l r14,@-r15
	mov.l r13,@-r15
	sts.l pr,@-r15
	mov.l r12,@-r15
	mov.l r11,@-r15
	SETUP_PREDEF
	bsr S9xDoHBlankProcessing
	nop
	mov.l @r15+,r11
	mov.l @r15+,r12
	lds.l @r15+,pr
	mov.l @r15+,r13
	rts
	mov.l @r15+,r14
#endif

	MISC 

#ifndef USE_OPC_ASM_MIXTO
S9xDoHBlankProcessing:
	sts.l pr,@-r15

#ifndef USE_OLD_APUTIMER
	UPDATE_APUTIMER _1
#endif

#ifdef CPU_SHUTDOWN
	mov.l WaitCounter,r1
#endif
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WhichEvent,r0
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
#ifdef CPU_SHUTDOWN
	add #1,r1
#endif
	tst r0,r0
#ifdef CPU_SHUTDOWN
	mov.l r1,WaitCounter
#endif
	bt/s .hblank_start
	cmp/eq #1,r0
	bt/s .hblank_end
	mov #3,r1
	cmp/hi r1,r0
	bt .reschedule

!.htimer_trigger:
	mov.l PPU,r1
	mov.b HTimerEnabled,r0
	tst r0,r0
	bt .reschedule
	mov.b VTimerEnabled,r0
	tst r0,r0
	bt .L191
	mov.w IRQVBeamPos,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r3
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	cmp/eq r0,r3
	bf .reschedule
.L191:
#ifndef USE_ASM_IRQ_INLINE
	bsr _S9xSetIRQ
	mov #1,r4
#else
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b IRQActive,r0
	mov #3,r1
	or #1,r0
	mov.b r0,IRQActive
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l r1,IRQCycleCount
	mov.l Flags,r2
	mov.l .pIRQ_PENDING_FLAG20,r0
	or r0,r2
	mov.l r2,Flags
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WaitingForInterrupt,r0
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .reschedule
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l PCS,r1
	mov.l r0,IRQCycleCount
	add #1,r1
	mov.l r1,PCS
#endif

	bra .reschedule
	nop

#ifdef USE_ASM_IRQ_INLINE
	.align 2
.pIRQ_PENDING_FLAG20:
	.long IRQ_PENDING_FLAG
#endif

.hblank_start:		
	mov.l IPPU,r1
	mov.b HDMA,r0
	extu.b r0,r4
	tst r4,r4
	bt .reschedule
	mov.l PPU,r1
	mov.w ScreenHeight,r0
	extu.w r0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r1
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	cmp/hi r0,r1
	bt .reschedule
	mov.l .S9xDoHDMA,r3
	jsr @r3
	nop
	mov.l IPPU,r1
	mov.b r0,HDMA

.reschedule:
#ifndef USE_HDMA_EVENT
	mov.l IPPU,r1
	mov.b HDMA,r0
	extu.b r0,r5
#endif
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WhichEvent,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	extu.b r0,r0
	tst r0,r0
	bt/s .next_is_hblank_end
	mov.l Settings,r1
	cmp/eq #3,r0
#ifdef USE_HDMA_EVENT
	bf .next_is_hblank_start
#else
	bt .next_is_hblank_end
	tst r5,r5
	bf .next_is_hblank_start
#endif
.next_is_hblank_end:
	mov #1,r4
	bra .skip
	mov.l H_Max,r5
/* AQUI NO ES NECESARIO CAMBIAR R5? */
.next_is_hblank_start:
	mov #0,r4
	mov.l HBlankStart,r5
.skip:
	mov.l PPU,r1
	mov.b HTimerEnabled,r0
	tst r0,r0
	bt .not_enabled
	mov.w HTimerPosition,r0
	cmp/ge r5,r0
	bt .not_enabled
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l NextEvent,r3
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov r0,r6
	cmp/gt r3,r0
	bf .not_enabled
	mov.b VTimerEnabled,r0
	extu.b r0,r0
	tst r0,r0
	bt .enabled
	mov.w IRQVBeamPos,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r2
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	cmp/eq r0,r2
	bf .not_enabled
.enabled:
	mov.l Settings,r1
	mov #2,r4
	mov.l HBlankStart,r0
	cmp/gt r6,r0
	bt/s .before
	mov r6,r5
	mov #3,r4
.before:
.not_enabled:
	mov r4,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l r5,NextEvent
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	lds.l @r15+,pr
#ifdef USE_ASM_R12
	rts
	mov.b r0,WhichEvent
#else
	add #16,r11
	mov.b r0,WhichEvent
	rts
	add #-16,r11
#endif

.hblank_end:
	mov.l Settings,r1
	add  #80, r1
//mov.b SuperFXEnabled,r3
	mov.b @r1,r3
	extu.b r3,r3
	tst  r3,r3
	bt   .nofx
	mov.l .S9xSuperFXExec,r3
	jsr @r3
	nop
.nofx:
#ifndef USE_SOUND_DESYNC
	mova  .cuenta_sound_throttle, r0
	mov.l @r0, r2
	add   #-1, r2
	tst   r2,r2
	bt/s  .nosoundsync
	mov.l r2,@r0

	mov.l _snes4all_sound_throttle, r2
	mov.l .S9xGenerateSound,r3
	jsr   @r3
	mov.l r2,@r0
#endif

.nosoundsync:
	mov.l Settings,r1
	mov.l Cycles,r0
	mov.l H_Max,r2
	sub r2,r0
	mov.l r0,Cycles
#ifndef ARMAPU
	mov.l IAPU,r1

#ifndef USE_OLD_APUTIMER
	mov.l NextAPUTimerPos,r0
	mov.l _snes4all_apu_hmax, r3
	sub r3,r0
	mov.l r0,NextAPUTimerPos
#endif
	mov.b APUExecuting,r0
	extu.b r0,r0
	tst r0,r0
	bt/s .apunotrunning
	mov.l APU,r1
	mov.l APUCycles,r0
	sub r2,r0

.apunotrunning:
	mov.l r0,APUCycles
.apucycleskip:	
#endif
	mov.l ICPU,r1
	mov.l Scanline,r2
	add #1,r2
	mov.l r2,Scanline
	mov #-1,r2
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l r2,NextEvent
#ifndef USE_ASM_R12
	add #-16,r11
#endif

	mov.l PPU,r1
	mov.l ._snes4all_vcounter,r2
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r0
	add r0,r0
	mov.w @(r0,r2),r0
	extu.w r0,r0
	mov.l r0,V_Counter
#ifndef USE_ASM_R12
	add #-16,r11
#endif

	mov.l _snes4all_max_vcounter,r3
	cmp/hi r3,r0
	bf .L162

#ifdef USE_OLD_SYNC_SPEED
!.ntsc_tv:
	mov.l PPU,r1
	mov.w SavedOAMAddr,r0
	mov.w r0,OAMAddr
	mov #0,r0
	mov.b r0,OAMFlip
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,NMIActive
	mov.l r0,V_Counter
#ifndef USE_ASM_R12
	add #-16,r11
#endif
#endif
	mov.l .S9xSyncSpeed2,r3
	jsr @r3
	nop

	mov.l  _savestate_state,r0
	tst r0,r0
	bt  .L161
	mov.l PCBase,r0
	neg r0,r0
	add PC,r0
	mov.w r0,PCR
#ifdef SPC700_C
	mov.l IAPU,r1
	mov.l APUPC, r0
	mov.l APURAM, r1
	sub r1,r0
	mov.l APURegisters,r1
	mov.w r0,APUPCR
#else
	mov.l APURAM,r1
	sub r1,APUPC
	mov.w APUPC, APUPCR
#endif
	APUS9xPackStatus SSProc2
	mov.l .S9xSaveStateProcess,r3
	jsr @r3
	nop
.L161:
	mov.l PPU,r1
	mov.b VTimerEnabled,r0
	extu.b r0,r0
	tst r0,r0
	bt .L165
	mov.b HTimerEnabled,r0
	tst r0,r0
	bf .L165
	mov.w IRQVBeamPos,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r2
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	cmp/eq r0,r2
	bf .L165
#ifndef USE_ASM_R12
	add #16,r11
#endif
#ifdef USE_OLD_SYNC_SPEED
	mov.b IRQActive,r0
	or #2,r0
	mov.b r0,IRQActive
	mov.b WaitingForInterrupt,r0
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .L163
	mov.l PCS,r0
	add #1,r0
	mov.l r0,PCS
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif
#endif
.L163:
#ifndef USE_ASM_IRQ_INLINE
	bsr _S9xSetIRQ
	mov #2,r4
#else
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b IRQActive,r0
	mov #3,r1
	or #2,r0
	mov.b r0,IRQActive
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l r1,IRQCycleCount
	mov.l Flags,r2
	mov.l .pIRQ_PENDING_FLAG21,r0
	or r0,r2
	mov.l r2,Flags
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WaitingForInterrupt,r0
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .L165
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l PCS,r1
	mov.l r0,IRQCycleCount
	add #1,r1
	mov.l r1,PCS
#endif
	bra .L165
	nop
.L162:
	mov.l PPU,r1
	mov.w ScreenHeight,r0
	extu.w r0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r2
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	add #1,r0
	cmp/eq r0,r2
	bf .L161

	mov.l .S9xEndScreenRefresh,r0
	jsr @r0
	nop

#ifdef USE_OLD_END_REFRESH
	mov.l PPU,r1
	mov #0,r0
	mov.b r0,FirstSprite
	mov.b Brightness,r0
	mov.l IPPU,r1
	mov.b r0,MaxBrightness
	mov #0,r0
	mov.b r0,HDMA
	mov.l .FillRAM0,r0
	mov.w .nx2100,r2
	mov.b @(r0,r2),r3
	shll r2
	shll r3
	mov.l PPU,r1
	shlr8 r3
	mov #1,r0
	and r3,r0
	mov.b r0,ForcedBlanking
	mov.l .FillRAM0,r0
	mov.b @(r0,r2),r3
	add #0x10,r2
	cmp/pz r3
	mov #0x80,r3
	bt/s .L161
	mov.b r3,@(r0,r2)
	mov.l Flags,r0
	or #NMI_FLAG,r0
	mov.l r0,Flags
	mov #NMITriggerPoint_offs,r0
#ifdef USE_ASM_R12
	mov.l @(r0,r12),r2
#else
	mov.l @(r0,r11),r2
#endif
	mov #1,r0
	mov.l r2,NMICycleCount
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,NMIActive
#ifndef USE_ASM_R12
	add #-16,r11
#endif
#endif
	bra .L161
	nop
.L165:
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	cmp/eq #1,r0
	bf .L177
#ifndef USE_OLD_START_REFRESH
	mov.l .S9xStartScreenRefresh,r3
	jsr @r3
	nop
#else
	mov.l .S9xUpdateJoypads,r3
	jsr @r3
	nop
	mov.l .FillRAM0,r3
	mov #2,r2
	mov.w .n4210,r1
	mov.l Flags,r0
	add r1,r3
	mov.b r2,@r3
	or #NMI_FLAG,r0
	mov.l .S9xStartScreenRefresh,r3
	add #-NMI_FLAG,r0
	jsr @r3
	mov.l r0,Flags
#endif
.L177:
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r3
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l PPU,r1
	tst r3,r3
	bt .L178
	mov.w ScreenHeight,r0
	extu.w r0,r0
	add #1,r0
	cmp/ge r0,r3
	bt .L178
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l .RenderLine,r3
	add #-1,r0
	jsr @r3
	extu.b r0,r4

.L178:
#ifndef USE_OLD_APUTIMER
	bra .reschedule
	nop
#else
#ifndef ARMAPU
	mov.l .APUTimer_ptr,r2
	mov.l IAPU,r1
	mov.b @(APUTimerEnabled - APUTimer + 2,r2),r0
	extu.b r0,r0
	tst r0,r0
	bt .L179
	mov.w @(4,r2),r0
	add #4,r0
	mov.w r0,@(4,r2)
	extu.w r0,r3
	mov.w @(APUTimerTarget - APUTimer + 4,r2),r0
	extu.w r0,r0
	cmp/hs r0,r3
	bf .L179
.L182:
	mov.w .n255,r0
	extu.w r0,r0
	mov.l APURAM,r3
	add r0,r3
	mov.b @r3,r0
	add #1,r0
	and #15,r0
	mov.b r0,@r3
	mov.w @(APUTimerTarget - APUTimer + 4,r2),r0
	extu.w r0,r3
	mov.w @(4,r2),r0
#ifdef SPC700_SHUTDOWN
	mov.l APUWaitCounter,r4
	sub r3,r0
	add #1,r4
	mov.w r0,@(4,r2)
	extu.w r0,r0
	mov.l r4,APUWaitCounter
	cmp/hs r3,r0
	mov #1,r0
	bt/s .L182
	mov.b r0,APUExecuting
#else
	sub r3,r0
	mov.w r0,@(4,r2)
	extu.w r0,r0
	cmp/hs r3,r0
	bt .L182
#endif
.L179:
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l V_Counter,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst #1,r0
	bt 1f
	mov.b @(APUTimerEnabled - APUTimer,r2),r0
	extu.b r0,r0
	tst r0,r0
	bt .L185
	mov.w @r2,r0
	add #1,r0
	mov.w r0,@r2
	mov r0,r3
	mov.w @(APUTimerTarget - APUTimer,r2),r0
	cmp/hs r0,r3
	bf .L185
	mov.w .n253,r0
	mov.l APURAM,r3
	add r0,r3
	mov.b @r3,r0
	add #1,r0
	and #15,r0
	mov.b r0,@r3
#ifdef SPC700_SHUTDOWN
	mov.l APUWaitCounter,r3
	mov #0,r0
	add #1,r3
	mov.w r0,@r2
	add #1,r0
	mov.l r3,APUWaitCounter
	mov.b r0,APUExecuting
#else
	mov #0,r0
	mov.w r0,@r2
#endif
.L185:
	mov.b @(APUTimerEnabled - APUTimer + 1,r2),r0
	extu.b r0,r0
	tst r0,r0
	bt 1f
	mov.w @(2, r2),r0
	add #1,r0
	mov.w r0,@(2, r2)
	mov r0,r3
	mov.w @(APUTimerTarget - APUTimer + 2,r2),r0
	cmp/hs r0,r3
	bf 1f
	mov.w .n254,r0
	mov.l APURAM,r3
	add r0,r3
	mov.b @r3,r0
	add #1,r0
	and #15,r0
	mov.b r0,@r3
#ifdef SPC700_SHUTDOWN
	mov.l APUWaitCounter,r3
	mov #0,r0
	add #1,r3
	mov.w r0,@(2,r2)
	add #1,r0
	mov.l r3,APUWaitCounter
	bra .reschedule
	mov.b r0,APUExecuting
#else
	mov #0,r0
	bra .reschedule
	mov.w r0,@(2,r2)
#endif
#endif
1:	
#endif
	bra .reschedule
	nop

	.align 2
#ifdef USE_ASM_IRQ_INLINE
.pIRQ_PENDING_FLAG21:
	.long IRQ_PENDING_FLAG
#endif

.S9xDoHDMA:
	.long _S9xDoHDMA
.S9xSuperFXExec:	
	.long _S9xSuperFXExec	
#ifndef USE_SOUND_DESYNC
.S9xGenerateSound:
	.long _S9xGenerateSound
#endif

//.S9xStartHDMA:	
//	.long _S9xStartHDMA
.S9xEndScreenRefresh:	
	.long _S9xEndScreenRefresh
#if defined(USE_OLD_END_REFRESH) || defined(USE_OLD_START_REFRESH)
.FillRAM0:	
	.long FillRAM
#endif
#ifdef USE_OLD_START_REFRESH
.S9xUpdateJoypads:	
	.long _S9xUpdateJoypads
#endif
.S9xStartScreenRefresh:
	.long _S9xStartScreenRefresh
.RenderLine:
	.long _RenderLine
#ifdef USE_OLD_START_REFRESH
.n4210:
	.word 0x4210
#endif
#ifdef USE_OLD_END_REFRESH
.nx2100:
	.word 0x2100
#endif
.n253:
	.word 253
.n254:
	.word 254
.n255:
	.word 255
.n262:
	.word 262
.n312:
	.word 312

	.align 2
.global _snes4all_sound_throttle
_snes4all_sound_throttle:
	.long 1
.cuenta_sound_throttle:
	.long 1
.S9xUpdateAPUTimer_CONST00002710:
        .long 0x00002710
.S9xUpdateAPUTimer_CONST003334bb:
	.long 0x003334bb
.S9xUpdateAPUTimer_APUTimer_ptr:
	.long _APU+APUTimer
.S9xUpdateAPUTimer_CONST000000ff:
	.long 0x000000ff
.S9xUpdateAPUTimer_CONST000000fd:
	.long 0x000000fd
.S9xUpdateAPUTimer_CONST000000fe:
	.long 0x000000fe
#endif

.S9xSyncSpeed2:
	.long _S9xSyncSpeed
.global _savestate_state
_savestate_state:
	.long  0
.S9xSaveStateProcess:
	.long _S9xSaveStateProcess

#ifndef ARMAPU
.APUTimer_ptr:
	.long _APU+APUTimer
#endif

._snes4all_vcounter:
	.long _snes4all_vcounter
.global _snes4all_max_vcounter
_snes4all_max_vcounter:
	.long 343
.global _snes4all_apu_hmax
_snes4all_apu_hmax:
	.long 1

	.align 2
.globl _S9xSetIRQ
_S9xSetIRQ:
#ifdef USE_ASM_R12
	mov.l r12,@-r15
	mov.l ._CPU,r12
#else
	mov.l r11,@-r15
	mov.l ._Registers,r11
	add #16,r11
#endif
	mov.b IRQActive,r0
	mov #3,r1
	or r4,r0
	mov.b r0,IRQActive
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l r1,IRQCycleCount
	mov.l Flags,r2
	mov.l .pIRQ_PENDING_FLAG2,r0
	or r0,r2
	mov.l r2,Flags
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b WaitingForInterrupt,r0
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .NoIncPC
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l PCS,r1
	mov.l r0,IRQCycleCount
	add #1,r1
	mov.l r1,PCS
.NoIncPC:
	rts
#ifdef USE_ASM_R12
	mov.l @r15+,r12
#else
	mov.l @r15+,r11
#endif


	.align 2
.globl _S9xClearIRQ
_S9xClearIRQ:
#ifdef USE_ASM_R12
	mov.l r12,@-r15
	mov.l ._CPU,r12
#else
	mov.l r11,@-r15
	mov.l ._Registers,r11
	add #16,r11
#endif
	not r4,r3
	mov.b IRQActive,r0
	and r3,r0
	mov.b r0,IRQActive
	extu.b r0,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bf .irqsstillpending
	mov.l .nIRQ_PENDING_FLAG2,r0
	mov.l Flags,r2
	and r0,r2
	mov.l r2,Flags
.irqsstillpending:
	rts
#ifdef USE_ASM_R12
	mov.l @r15+,r12
#else
	mov.l @r15+,r11
#endif

	.align 2
#ifdef USE_ASM_R12
._CPU:
	.long _CPU
#else
._Registers:
	.long _Registers
#endif
.pIRQ_PENDING_FLAG2:
	.long IRQ_PENDING_FLAG
.nIRQ_PENDING_FLAG2:
	.long ~IRQ_PENDING_FLAG

	.align 2
/*
.global _APU
_APU:
	.rept (224/4)
	.long 0
	.endr
	.align 2
.global _IAPU
_IAPU:
	.rept (68/4)
	.long 0
	.endr
	.align 2
*/
/*
.global _APURegisters
_APURegisters:
	.rept (8/4)
	.long 0
	.endr
	.align 2
*/
