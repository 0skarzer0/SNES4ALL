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

#if defined(USE_MMU_DIRECT_INDEX) && !defined(USE_MMU)
#undef USE_MMU_DIRECT_INDEX
#endif

.macro GetCarry
	mov FLAGS,r1
	shlr r1
.endm

.macro GetNotCarry
	mov #Carry,r1
	tst r1,FLAGS
.endm

.macro SetZNjmp R
	jmp	@r13
	exts.b	\R,r7
.endm

.macro Set16ZNjmp R
	jmp	@r13
	exts.w	\R,r7
.endm

.macro SetZNlink R
	lds	r13,pr
	jmp	@r3
	exts.b	\R,r7
.endm

.macro Set16ZNlink R
	lds	r13,pr
	jmp	@r3
	exts.w	\R,r7
.endm

.macro Xlink
	jmp	@r3
	lds	r13,pr
.endm
	
/************* IMMEDIATE8 ****************/
.Macro Immediate8 K
	mov.b @PC+,r0
	extu.b r0,r0
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
.endm

/************* IMMEDIATE16 ****************/
.macro Immediate16 K
	mov.b @PC+,r0
	shll8 r0
	mov.b @PC+,r2
	extu.b r2,r2
	or r2,r0
	swap.b r0,r0
	extu.w r0,r0
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
.endm

/************* Relative ****************/
.macro Relative K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
	mov.b @PC+,r4
!	exts.b r4,r4
	mov.l PCBase,r2
	add PC,r4
	sub r2,r4
.endm

/************* RelativeLong ****************/
.macro RelativeLong K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r4
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	extu.b r4,r4
	mov.b @PC+,r0
	shll8 r0
	mov.l PCBase,r2
	or r0,r4
	add PC,r4
	sub r2,r4
.endm

	
/************* AbsoluteIndexedIndirect8 ****************/
.macro AbsoluteIndexedIndirect8 K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r4
	shll8 r4
	mov.w XX,r0
	add r2,r4
	add r0,r4
	mov.l S9xGetWord,r3
	extu.w r4,r4
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	mov.l ICPU,r1
	mov.l ShiftedPB,r2
	jsr @r3
	or r2,r4
	extu.w r0,r4
.endm

/************* AbsoluteIndirectLong8 ****************/
.macro AbsoluteIndirectLong8 K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r4
	shll8 r4
#ifndef USE_MMU_DIRECT_INDEX
	mov.l S9xGetWord,r3
#endif
	or r2,r4
#ifndef USE_MMU_DIRECT_INDEX
	jsr @r3
#endif
	extu.w r4,r4
#ifdef USE_MMU_DIRECT_INDEX
	mov.b @r4+,r0
	add #24,CYCLES
	extu.b r0, r3
	mov.b @r4+,r0
	shll8 r0
	or r3,r0
	extu.w r0,r3
	mov.b @r4,r0
	extu.b r0,r0
	shll16 r0
	or r0,r3
	mov r3,r4
#else
	mov.l r0,@-r15
	mov.l S9xGetByte,r3
	add #2,r4
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r4
	shll16 r0
	or r0,r4
#endif
.endm

.macro AbsoluteIndirect8 K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r4
	shll8 r4
	mov.l S9xGetWord,r3
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	or r2,r4
	jsr @r3
	extu.w r4,r4
	extu.w r0,r4
.endm

.macro Absolute8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r3
	shll8 r3
	mov.l ICPU,r1
	mov.l ShiftedDB,r4
	or r2,r3
	extu.w r3,r0
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro Absolute8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro Absolute8 K
	Absolute8_i \K
#ifdef ASMADDR_NOP_PATCH
	Absolute8_f
#endif
.endm


.macro AbsoluteLong8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add r2,CYCLES
	add r2,CYCLES
#else
	add #24,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r4
	extu.b r4,r4
	mov.b @PC+,r0
	extu.b r0,r0
	shll16 r0
	shll8 r4
	or r0,r2
#ifndef ASMADDR_NOP_PATCH
	or r2,r4
#endif
.endm

.macro AbsoluteLong8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	or r2,r4
#endif
.endm

.macro AbsoluteLong8 K
	AbsoluteLong8_i \K
#ifdef ASMADDR_NOP_PATCH
	AbsoluteLong8_f
#endif
.endm

.macro Direct8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
#if 0
	cmpb $0, DL
	je .Direct8\K
	addl $6, CYCLES
.Direct8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
#ifndef ASMADDR_NOP_PATCH
	extu.w r4,r4
#endif
.endm

.macro Direct8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	extu.w r4,r4
#endif
.endm

.macro Direct8 K
	Direct8_i \K
#ifdef ASMADDR_NOP_PATCH
	Direct8_f
#endif
.endm

.macro DirectIndirectIndexed8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
#if 0
	cmpb $0, DD
	je .DirectIndirectIndexed8\K
	addl $6, CYCLES
.DirectIndirectIndexed8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
	mov.l S9xGetWord,r3
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	jsr @r3
	extu.w r4,r4
	mov.l ICPU,r3
	mov.l @(20,r3),r4
	add r0,r4
	mov.w YY,r0
	extu.w r0,r0
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro DirectIndirectIndexed8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro DirectIndirectIndexed8 K
	DirectIndirectIndexed8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndirectIndexed8_f
#endif
.endm

.macro DirectIndirectIndexedLong8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
#if 0
	cmpb $0, DD
	je .DirectIndirectIndexedLong8\K
	addl $6, CYCLES
.DirectIndirectIndexedLong8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
#ifndef USE_MMU_DIRECT_INDEX
	mov.l S9xGetWord,r3
	jsr @r3
#endif
	extu.w r4,r4
#ifndef USE_MMU_DIRECT_INDEX
	mov.l r0,@-r15
	mov.l S9xGetByte,r3
	jsr @r3
	add #2,r4
	mov.l @r15+,r4
	shll16 r0
	or r0,r4
#else
	mov.b @r4+,r0
	add #24,CYCLES
	extu.b r0, r3
	mov.b @r4+,r0
	shll8 r0
	or r3,r0
	extu.w r0,r3
	mov.b @r4,r0
	extu.b r0,r0
	shll16 r0
	or r0,r3
	mov r3,r4
#endif
	mov.w YY,r0
	extu.w r0,r0
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm
	
.macro DirectIndirectIndexedLong8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm
	
.macro DirectIndirectIndexedLong8 K
	DirectIndirectIndexedLong8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndirectIndexedLong8_f
#endif
.endm

.macro DirectIndexedIndirect8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #6, CYCLES
#else
	add #14,CYCLES
#endif
#endif
#if 0
	cmpb $0, DD
	je .DirectIndexedIndirect8\K
	addl $6, CYCLES
.DirectIndexedIndirect8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
	mov.w XX,r0
	add r0,r4
	mov.l S9xGetWord,r3
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	jsr @r3
	extu.w r4,r4
	mov.l ICPU,r3
	mov.l @(20,r3),r4
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro DirectIndexedIndirect8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro DirectIndexedIndirect8 K
	DirectIndexedIndirect8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndexedIndirect8_f
#endif
.endm

.macro DirectIndexedX8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #6, CYCLES
#else
	add #14, CYCLES
#endif
#endif
#if 0
	cmpb $0, DD
	je .DirectIndexedX8\K
	addl $6, CYCLES
.DirectIndexedX8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
	mov.w XX,r0
	add r0,r4
#ifndef ASMADDR_NOP_PATCH
	extu.w r4,r4
#endif
.endm

.macro DirectIndexedX8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	extu.w r4,r4
#endif
.endm

.macro DirectIndexedX8 K
	DirectIndexedX8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndexedX8_f
#endif
.endm

.macro DirectIndexedY8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #6, CYCLES
#else
	add #14, CYCLES
#endif
#endif
#if 0
	cmpb $0, DD
	je .DirectIndexedY8\K
	addl $6, CYCLES
.DirectIndexedY8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
	mov.w YY,r0
	add r0,r4
#ifndef ASMADDR_NOP_PATCH
	extu.w r4,r4
#endif
.endm

.macro DirectIndexedY8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	extu.w r4,r4
#endif
.endm

.macro DirectIndexedY8 K
	DirectIndexedY8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndexedY8_f
#endif
.endm

.macro AbsoluteIndexedX8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r3
	shll8 r3
	mov.l ICPU,r1
	mov.l ShiftedDB,r4
	or r2,r3
	extu.w r3,r3
	mov.w XX,r0
	add r3,r4
	extu.w r0,r0
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro AbsoluteIndexedX8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro AbsoluteIndexedX8 K
	AbsoluteIndexedX8_i \K
#ifdef ASMADDR_NOP_PATCH
	AbsoluteIndexedX8_f
#endif
.endm


.macro AbsoluteIndexedY8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
#else
	add #16,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r3
	shll8 r3
	mov.l ICPU,r1
	mov.l ShiftedDB,r4
	or r2,r3
	extu.w r3,r3
	mov.w YY,r0
	add r3,r4
	extu.w r0,r0
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro AbsoluteIndexedY8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro AbsoluteIndexedY8 K
	AbsoluteIndexedY8_i \K
#ifdef ASMADDR_NOP_PATCH
	AbsoluteIndexedY8_f
#endif
.endm

.macro AbsoluteLongIndexedX8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add r2,CYCLES
	add r2,CYCLES
#else
	add #24,CYCLES
#endif
#endif
	mov.b @PC+,r2
	extu.b r2,r2
	mov.b @PC+,r3
	extu.b r3,r3
	mov.b @PC+,r4
	extu.b r4,r4
	shll8 r3
	or r2,r3
	mov.w XX,r0
	shll16 r4
	extu.w r0,r0
	or r3,r4
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro AbsoluteLongIndexedX8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro AbsoluteLongIndexedX8 K
	AbsoluteLongIndexedX8_i \K
#ifdef ASMADDR_NOP_PATCH
	AbsoluteLongIndexedX8_f
#endif
.endm

.macro DirectIndirect8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
#if 0
	cmpb $0, DL
	je .DirectIndirect8\K
	addl $6, CYCLES
.DirectIndirect8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
	mov.l S9xGetWord,r3
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	jsr @r3
	extu.w r4,r4
	mov.l ICPU,r3
	mov.l @(20,r3),r4
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro DirectIndirect8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro DirectIndirect8 K
	DirectIndirect8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndirect8_f
#endif
.endm

.macro DirectIndirectLong8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
#else
	add #8,CYCLES
#endif
#endif
#if 0
	cmpb $0, DL
	je .DirectIndirectLong8\K
	addl $6, CYCLES
.DirectIndirectLong8\K:
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w DD,r0
	add r0,r4
#ifndef USE_MMU_DIRECT_INDEX
	mov.l S9xGetWord,r3
	jsr @r3
#endif
	extu.w r4,r4
#ifndef USE_MMU_DIRECT_INDEX
	mov.l r0,@-r15
	mov.l S9xGetByte,r3
	jsr @r3
	add #2,r4
	mov.l @r15+,r4
	shll16 r0
#else
	mov.b @r4+,r0
	add #24,CYCLES
	extu.b r0, r3
	mov.b @r4+,r0
	shll8 r0
	or r3,r0
	extu.w r0,r3
	mov.b @r4,r0
	extu.b r0,r0
	shll16 r0
	mov r3,r4
#endif
#ifndef ASMADDR_NOP_PATCH
	or r0,r4
#endif
.endm

.macro DirectIndirectLong8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	or r0,r4
#endif
.endm

.macro DirectIndirectLong8 K
	DirectIndirectLong8_i \K
#ifdef ASMADDR_NOP_PATCH
	DirectIndirectLong8_f
#endif
.endm

.macro StackRelative8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #6,CYCLES
#else
	add #14,CYCLES
#endif
#endif
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w SS,r0
	add r0,r4
#ifndef ASMADDR_NOP_PATCH
	extu.w r4,r4
#endif
.endm

.macro StackRelative8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	extu.w r4,r4
#endif
.endm

.macro StackRelative8 K
	StackRelative8_i \K
#ifdef ASMADDR_NOP_PATCH
	StackRelative8_f
#endif
.endm

.macro StackRelativeIndirectIndexed8_i K
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #12,CYCLES
#else
	add #20,CYCLES
#endif	
#endif	
	mov.b @PC+,r4
	extu.b r4,r4
	mov.w SS,r0
	mov.l S9xGetWord,r3
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	add r0,r4
	jsr @r3
	extu.w r4,r4
	mov.l ICPU,r3
	mov.l @(20,r3),r4
	add r0,r4
	mov.w YY,r0
	extu.w r0,r0
#ifndef ASMADDR_NOP_PATCH
	add r0,r4
#endif
.endm

.macro StackRelativeIndirectIndexed8_f
#ifndef ASMADDR_NOP_PATCH
	nop
#else
	add r0,r4
#endif
.endm

.macro StackRelativeIndirectIndexed8 K
	StackRelativeIndirectIndexed8_i \K
#ifdef ASMADDR_NOP_PATCH
	StackRelativeIndirectIndexed8_f
#endif
.endm


.macro BranchCheck0 K
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b BranchSkip,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .BranchCheck0S9xExit\K
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,BranchSkip
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov PC,r0
	mov.l PCBase,r1
	extu.w r4,r4
	sub r1,r0
	extu.w r0,r0
	cmp/hs r4,r0
	bt .BranchCheck0S9xExit\K
	jmp @r13
	nop
.BranchCheck0S9xExit\K:
.endm

.macro BranchCheck1 K
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b BranchSkip,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .BranchCheck1S9xExit\K
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,BranchSkip
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov PC,r0
	mov.l PCBase,r1
	extu.w r4,r4
	sub r1,r0
	extu.w r0,r0
	cmp/hs r4,r0
	bt .BranchCheck1S9xExit\K
	jmp @r13
	nop
.BranchCheck1S9xExit\K:
.endm

.macro BranchCheck2 K
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b BranchSkip,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .BranchCheck2S9xExit\K
	mov #0,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,BranchSkip
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov PC,r0
	mov.l PCBase,r1
	extu.w r4,r4
	sub r1,r0
	extu.w r0,r0
	cmp/hs r4,r0
	bt .BranchCheck2S9xExit\K
	jmp @r13
	nop
.BranchCheck2S9xExit\K:
.endm

.macro PushBytejmp K
	mov r0,r1
	mov.w SS,r0
	extu.w r0,r4
	mov.l S9xSetByte,r3
	add #-1,r0
#ifdef USE_MMU_DIRECT_INDEX
	add #-12,r3
#endif
	mov.w r0,SS
	mov r1,r0
	Xlink
.endm

.macro PushWordjmp K
	mov r0,r1
	mov.w SS,r0
	add #-1,r0
	extu.w r0,r4
	mov.l S9xSetWord,r3
	add #-1,r0
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	mov.w r0,SS
	mov r1,r0
	Xlink
.endm

.macro PushByte K
	mov r0,r1
	mov.w SS,r0
	extu.w r0,r4
	mov.l S9xSetByte,r3
	add #-1,r0
#ifdef USE_MMU_DIRECT_INDEX
	add #-12,r3
#endif
	mov.w r0,SS
	jsr @r3
	mov r1,r0
.endm

.macro PushWord K
	mov r0,r1
	mov.w SS,r0
	add #-1,r0
	extu.w r0,r4
	mov.l S9xSetWord,r3
	add #-1,r0
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	mov.w r0,SS
	jsr @r3
	mov r1,r0
.endm

.macro PullByte K
	mov.w SS,r0
	add #1,r0
	mov.l S9xGetByte,r3
#ifdef USE_MMU_DIRECT_INDEX
	add #-8,r3
#endif
	extu.w r0,r4
	jsr @r3
	mov.w r0,SS
.endm

.macro PullWord K
	mov.w SS,r0
	add #1,r0
	mov.l S9xGetWord,r3
	extu.w r0,r4
#ifdef USE_MMU_DIRECT_INDEX
	add #-16,r3
#endif
	add #1,r0
	jsr @r3
	mov.w r0,SS
.endm
