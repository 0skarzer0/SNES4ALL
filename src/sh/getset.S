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

	.align 5
.globl S9xGetByte_r, _S9xGetByte_callback, _S9xGetByte_callback_jmp

	! GetByte preserves r4

#ifdef USE_MMU_DIRECT_INDEX
#ifdef VAR_CYCLES
	add #8,CYCLES
#else
	nop
#endif
	mov.b @r4,r0
	rts
	extu.b r0,r0
#endif
S9xGetByte_r:
#if !defined(USE_MMU) || defined(USE_MEMORY_SPEED) || defined(USE_BLOCK_RAM)
	mov r4,r0
	mov #-MEMMAP_SHIFT,r3
	shld r3,r0
	mov.w .MEMMAP_MSK1,r3
	and r3,r0			! r0=block
#endif
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l .MemorySpeed0,r3
	mov.b @(r0,r3),r1
#else
	add #8, CYCLES
#endif
#endif
#if defined(CPU_SHUTDOWN) && defined(USE_BLOCK_RAM)
	mov.l .BlockIsRAM0,r3
	mov.b @(r0,r3),r2
#ifdef USE_MMU
	tst r2,r2
	bt .GBNotRAM0
	mov.l PCAtOpcodeStart,r0
	mov.l r0,WaitAddress
.GBNotRAM0:
#endif
#endif

#ifdef USE_MMU
#ifdef USE_MMU_MASK
	mov.l .MMUMASK1, r3
	and	r4, r3
#else
#ifdef USE_ASM_PREFETCH
	pref @r4
#endif
	mov #-12,r0
	mov r4,r3
	shld r0,r3
	mov #1,r0
	cmp/hi r0,r3
	bf.s _S9xGetByte_callback
	add #2,r0
	cmp/hi r0,r3
	bf.s .GBPPU
	add #2,r0
	cmp/hi r0,r3
	bf .GBCPU
#endif
_S9xGetByte_callback:
#ifdef USE_MMU_MASK
	mov.b @r3,r0
#else
	mov.b @r4,r0
#endif
	rts
	extu.b r0,r0
#ifdef USE_MMU_MASK
.align 2
.MMUMASK1: .long 0x0fffffff
#endif
.align 2
_S9xGetByte_callback_jmp:
	mov r4,r0
	mov #-MEMMAP_SHIFT,r3
	shld r3,r0
	mov.w .MEMMAP_MSK1,r3
	and r3,r0			! r0=block
! USE_MMU
#endif
	shll2 r0
	mov.l .Map0_MSK1,r3
	mov.l @(r0,r3),r3
#ifndef USE_MMU
	cmp/pz r3
	bt .GBSpecial
#ifdef CPU_SHUTDOWN
#ifdef USE_BLOCK_RAM
	tst r2,r2
	bt .GBNotRAM
	mov.l PCAtOpcodeStart,r0
	mov.l r0,WaitAddress
.GBNotRAM:
	extu.w r4,r2
	add r2,r3
#else
	extu.w r4,r2
	mov.l PCAtOpcodeStart,r0
	add r2,r3
#ifdef USE_ASM_PREFETCH
	pref @r3
#endif
	mov.l r0,WaitAddress
#endif
#else
	extu.w r4,r2
	add r2,r3
#endif	
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
#if defined(USE_BLOCK_RAM) && defined(USE_ASM_PREFETCH)
	pref @r3
#endif
	add r1,CYCLES
#endif
	mov.b @r3,r0
	rts
	extu.b r0,r0
	
	.align 2
.Map0_MSK1:
	.long Map
.MEMMAP_MSK1:	
	.word MEMMAP_MASK

	.align 2
.GBSpecial:
! USE_MMU
#endif
	mova .GBJmpTable,r0
	shll2 r3
	add r3,r0
	mov.l @r0,r3
	jmp @r3
	nop

#ifdef USE_MMU
	.align 2
.Map0_MSK1:
	.long Map
.MEMMAP_MSK1:	
	.word MEMMAP_MASK
	.align 2
#endif
.GBJmpTable:
	.long .GBPPU
	.long .GBCPU
	.long .GBDSP
	.long .GBLSRAM
	.long .GBHSRAM
	.long .GBNONE
	.long .GBDEBUG
	.long .GBC4
	.long .GBBWRAM
	.long .GBNONE
	.long .GBNONE
	.long .GBNONE

	.align 2
.GBPPU: /* MAP_PPU */
#ifdef USE_ASM_GET_SPC
	mov.w .GBPPU_SPC, r0
	mov #63, r3
	add r4, r0
	extu.w r0, r0
	cmp/hi r3, r0
	bt .GBPPU_NO_SPC

#ifndef USE_ALWAYS_APU_SYNC
	mov.l .GBPPU_SPC_SND, r0
	mov.l @r0,r0
	mov #1, r3
	cmp/hi r3,r0
	bt .GBPPU_SPC_MAJOR
	mov.l APU,r1
	mov.l APUCycles,r0
	cmp/gt CYCLES,r0
	bf .GBPPU_NO_SPC
.GBPPU_SPC_MAJOR:
#endif

	mov.l IAPU,r2
	mov.l @(12,r2),r1
	add #1, r1
	mov.l   r1,@(12,r2)

	mov.l Settings,r0
	add #3,r2
	mov.b @r0,r0
	mov.b r0,@r2

	mov #3,r1
	mov.l APU,r0
	add #7,r0
	and r4,r1
	mov.b @(r0,r1),r0
	rts
	extu.b r0,r0

.GBPPU_SPC: .word 0xdec0
	.align 2
#ifndef USE_ALWAYS_APU_SYNC
.GBPPU_SPC_SND: .long _snes4all_sound_enable
#endif
.GBPPU_NO_SPC:
#endif
#ifdef USE_GETPPU_TABLE
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetPPU_func,r3
	extu.w r4, r0
	shll2 r0
	mov.l @(r0,r3),r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r4
	extu.b r0,r0
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.S9xGetPPU_func:
	.long _S9xGetPPU_func
#else
#ifdef VAR_CYCLES
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b InDMA,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bf .GBPPU_NOADD
	add #6, CYCLES
.GBPPU_NOADD:
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetPPU_NOADD,r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r4
	extu.b r0,r0
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.S9xGetPPU_NOADD:
	.long _S9xGetPPU
#endif

	.align 2

.GBCPU: /* MAP_CPU */
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetCPU_BG,r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r4
	extu.b r0,r0
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.S9xGetCPU_BG:
	.long _S9xGetCPU

	.align 2
.GBDSP: /* MAP_DSP */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetDSP_GB,r3
#ifndef USE_OLD_DSP1
	mov.l @r3, r3
#endif
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r4
	extu.b r0,r0
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.S9xGetDSP_GB:
#ifndef USE_OLD_DSP1
	.long _GetDSP
#else
	.long _S9xGetDSP
#endif

	.align 2
.GBLSRAM: /* MAP_LOROM_SRAM */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	mov.l .SRAM_GBLSRAM,r3
	mov.l .SRAMMask_GBLSRAM,r2
	mov.w @r2,r2
	extu.w r2,r2
	and r4,r2
	add r2,r3
	mov.b @r3,r0
	rts
	extu.b r0,r0

	.align 2
.SRAM_GBLSRAM:
	.long SRAM
	.align 2
.SRAMMask_GBLSRAM:
	.long SRAMMask
	
	.align 2
.GBHSRAM: /* MAP_HIROM_SRAM */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	mov.w .n7fffA_GBHSRAM,r3
	and r4,r3
	mov.w .n6000A,r2
	sub r2,r3
	mov.l .nf0000_GBHSRAM,r2
	and r4,r2
	shlr2 r2
	shlr r2
	add r2,r3
	mov.l .SRAM_GBHSRAM,r0
	mov.l .SRAMMask_GBHSRAM,r2
	mov.w @r2,r2
	extu.w r2,r2
	and r3,r2
	mov.b @(r0,r2),r0
	rts
	extu.b r0,r0

	.align 2
.n7fffA_GBHSRAM:
	.word 0x7fff
.n6000A_GBHSRAM:
	.word 0x6000
.nf0000_GBHSRAM:
	.long 0xf0000
.SRAMMask_GBHSRAM:
	.long SRAMMask
.SRAM_GBHSRAM:
	.long SRAM

	.align 2
.GBNONE:
	extu.w	r4,r0
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	rts
	shlr8	r0

	.align 2
.GBDEBUG: /* MAP_DEBUG */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	rts
	mov #0,r0

	.align 2
.GBC4: /* MAP_C4 */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	mov.l  .RAMGetC4_GBC4, r0
	mov.b  @r0, r0
	rts
	extu.b r0, r0
	.align 2
.RAMGetC4_GBC4:
	.long   C4RAM - 0x6000

	.align 2
.GBBWRAM: /* MAP_BWRAM */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	mov.w .n7fffA,r3
	and r4,r3
	mov.w .n6000A,r2
	sub r2,r3
	mov.l .BWRAM_GBBWRAM,r0
	mov.l @r0, r0
	mov.b @(r0,r3),r0
	rts
	extu.b r0,r0

	.align 2
.n7fffA:
	.word 0x7fff
.n6000A:
	.word 0x6000
.BWRAM_GBBWRAM:
	.long BWRAM

	.align 5

.globl S9xGetWord_r, _S9xGetWord_callback, _S9xGetWord_callback_jmp

	! GetWord preserves r4
#ifdef USE_MMU_DIRECT_INDEX
#ifdef VAR_CYCLES
	add #16,CYCLES
#else
	nop
#endif
	mov.b @r4+,r0
	extu.b r0, r3
	mov.b @r4,r0
	shll8 r0
	or r3,r0
	rts
	extu.w r0,r0
#endif
S9xGetWord_r:
#if !defined(USE_MMU) || defined(USE_MMU_BOUNDARY)
#if 0
	mov r4,r2
	mov.w .n1fff,r3
	and r3,r2
	cmp/eq r3,r2
	bf .GWNotAtBlockBoundary
	sts.l pr,@-r15
	bsr S9xGetByte_r
	nop
	mov.l r0,@-r15
	bsr S9xGetByte_r
	add #1,r4
	mov.l @r15+,r2
	shll8 r0
	lds.l @r15+,pr
	add #-1,r4
	rts
	or r2,r0

	.align 2
.n1fff:
	.word	0x1fff
	.align 2

.GWNotAtBlockBoundary:
#endif
#endif
#if !defined(USE_MMU) || defined(USE_MEMORY_SPEED) || defined(USE_BLOCK_RAM)
	mov r4,r0
	mov #-MEMMAP_SHIFT,r3
	shld r3,r0
	mov.w .MEMMAP_MSK15,r3
	and r3,r0		! r0=block
#endif
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l .MemorySpeed0,r3
	mov.b @(r0,r3),r1
#else
	add #16,CYCLES
#endif
#endif
#if defined(CPU_SHUTDOWN) && defined(USE_BLOCK_RAM)
	mov.l .BlockIsRAM0,r3
	mov.b @(r0,r3),r2
#ifdef USE_MMU
	tst r2,r2
	bt .GWNotRAM
	mov.l PCAtOpcodeStart,r2
	mov.l r2,WaitAddress
.GWNotRAM:
#endif
#endif

#ifdef USE_MMU
#ifdef USE_MMU_MASK
	mov.l .MMUMASK2, r3
	and	r4, r3
	mov	r3, r2 
#else
#ifdef USE_ASM_PREFETCH
	pref @r4
#endif
	mov r4, r2
	mov #-12,r0
	mov r4,r3
	shld r0,r3
	mov #1,r0
	cmp/hi r0,r3
	bf.s _S9xGetWord_callback
	add #2,r0
	cmp/hi r0,r3
	bf.s .GWPPU
	add #2,r0
	cmp/hi r0,r3
	bf .GWCPU
#endif
_S9xGetWord_callback:
#ifdef USE_MMU_MASK
	mov.b @r3, r0
#else
	mov.b @r4, r0
#endif
	add #1, r2
	mov.b @r2, r2
	extu.b r0, r0
	extu.b r2, r2
	shll8 r2
	rts
	or r2, r0
#ifdef USE_MMU_MASK
.align 2
.MMUMASK2: .long 0x0fffffff
#endif
.align 2
_S9xGetWord_callback_jmp:
	mov r4,r0
	mov #-MEMMAP_SHIFT,r3
	shld r3,r0
	mov.w .MEMMAP_MSK15,r3
	and r3,r0		! r0=block
! USE_MMU	
#endif
	shll2 r0
	mov.l .Map0_MSK15,r3
	mov.l @(r0,r3),r3
#ifndef USE_MMU
	cmp/pz r3
	bt .GWSpecial
#ifdef CPU_SHUTDOWN
#ifdef USE_BLOCK_RAM
	tst r2,r2
	bt .GWNotRAM
	mov.l PCAtOpcodeStart,r2
	mov.l r2,WaitAddress
.GWNotRAM:
	extu.w r4,r2
	add r2,r3
#else
	extu.w r4,r2
	mov.l PCAtOpcodeStart,r0
	add r2,r3
#ifdef USE_ASM_PREFETCH
	pref @r3
#endif
	mov.l r0,WaitAddress
#endif
#else
	extu.w r4,r2
	add r2,r3
#endif	
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
#if defined(USE_BLOCK_RAM) && defined(USE_ASM_PREFETCH)
	pref @r3
#endif
	add r1,CYCLES
	add r1,CYCLES
#endif
	mov.b @r3+,r2
	mov.b @r3,r0
	extu.b r2,r2
	shll8 r0
	or r2,r0
	rts
	extu.w r0,r0

	.align 2
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
.MemorySpeed0:
	.long MemorySpeed
#endif
#if defined(CPU_SHUTDOWN) && defined(USE_BLOCK_RAM)
.BlockIsRAM0:
	.long BlockIsRAM
#endif
.Map0_MSK15:
	.long Map
.MEMMAP_MSK15:	
	.word MEMMAP_MASK
	.align 2

.GWSpecial:
! USE_MMU
#endif
	mova .GWJmpTable,r0
	shll2 r3
	add r3,r0
	mov.l @r0,r3
	jmp @r3
	nop

	.align 2
#ifdef USE_MMU
.Map0_MSK15:
	.long Map
.MEMMAP_MSK15:	
	.word MEMMAP_MASK
	.align 2
#endif
.GWJmpTable:
	.long .GWPPU
	.long .GWCPU
	.long .GWDSP
	.long .GWLSRAM
	.long .GWHSRAM
	.long .GWNONE
	.long .GWDEBUG
	.long .GWC4
	.long .GWBWRAM
	.long .GWNONE
	.long .GWNONE
	.long .GWNONE

	.align 2
.GWPPU: /* MAP_PPU */
#ifdef USE_GETPPU_TABLE
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetPPU_func2,r3
	extu.w r4,r0
	shll2 r0
	mov.l @(r0,r3),r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15,r4
	mov.l r0,@-r15
	mov.l .S9xGetPPU_func2,r3
	add #1,r4
	extu.w r4,r0
	shll2 r0
	mov.l @(r0,r3),r3
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r1
	shll8 r0
	mov.l @r15+,r4
	or r1,r0
	lds.l @r15+,pr
	POP_REGISTERS
	LOAD_CYCLES
	rts
	extu.w r0,r0
	.align 2
.S9xGetPPU_func2:
	.long _S9xGetPPU_func
#else
#ifdef VAR_CYCLES
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b InDMA,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bf .GWPPU_NOADD
	add #12, CYCLES
.GWPPU_NOADD:
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetPPU,r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15,r4
	mov.l r0,@-r15
	mov.l .S9xGetPPU,r3
	add #1,r4
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r1
	shll8 r0
	mov.l @r15+,r4
	or r1,r0
	lds.l @r15+,pr
	POP_REGISTERS
	LOAD_CYCLES
	rts
	extu.w r0,r0

	.align 2
.S9xGetPPU:
	.long _S9xGetPPU
#endif

	.align 2
.GWCPU: /* MAP_CPU */
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetCPU,r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15,r4
	mov.l r0,@-r15
	mov.l .S9xGetCPU,r3
	add #1,r4
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r1
	shll8 r0
	mov.l @r15+,r4
	or r1,r0
	lds.l @r15+,pr
	POP_REGISTERS
	LOAD_CYCLES
	rts
	extu.w r0,r0

	.align 2
.S9xGetCPU:
	.long _S9xGetCPU

	.align 2
.GWDSP: /* MAP_DSP */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetDSP,r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15,r4
	mov.l r0,@-r15
	mov.l .S9xGetDSP,r3
	add #1,r4
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r1
	shll8 r0
	mov.l @r15+,r4
	or r1,r0
	lds.l @r15+,pr
	POP_REGISTERS
	LOAD_CYCLES
	rts
	extu.w r0,r0

	.align 2
.GWLSRAM: /* MAP_LOROM_SRAM */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	mov.l .SRAM0,r0
	mov.l .SRAMMask0,r2
	mov.w @r2,r2
	extu.w r2,r2
	mov r2,r1
	and r4,r1
	mov.b @(r0,r1),r1
	add #1,r4
	extu.b r1,r1
	and r4,r2
	add #-1,r4
	mov.b @(r0,r2),r0
	shll8 r0
	or r1,r0
	rts
	extu.w r0,r0

	.align 2
.GWHSRAM: /* MAP_HIROM_SRAM */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	mov.w .n7fff_GWHSRAM,r3
	and r4,r3
	mov.w .n6000_GWHSRAM,r2
	sub r2,r3
	mov.l .nf0000_GWHSRAM,r2
	and r4,r2
	shlr2 r2
	shlr r2
	add r2,r3
	mov.l .SRAM0,r1
	mov.l .SRAMMask0,r2
	mov.w @r2,r2
	extu.w r2,r2
	and r3,r2
	add r2,r1
	mov.b @r1+,r2
	mov.b @r1,r0
	extu.b r2,r2
	shll8 r0
	or r2,r0
	rts
	extu.w r0,r0

	.align 2
.n7fff_GWHSRAM:
	.word 0x7fff
.n6000_GWHSRAM:
	.word 0x6000
.nf0000_GWHSRAM:
	.long 0xf0000

	.align 2
.GWNONE:
	extu.w	r4,r0
	extu.b	r4,r1
	mov r0,r2
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	shlr8	r0
	add	r2,r0
	rts
	sub	r1,r0

	.align 2
	
.GWDEBUG: /* MAP_DEBUG */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	rts
	mov #0,r0

	.align 2
.GWC4:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	mov.l  .RAMGetC4, r0
	mov.b   @r0, r2
	extu.b	r2, r2
	mov	r2, r0
	shll8	r2
	rts
	or	r2, r0
.RAMGetC4:
	.long   C4RAM - 0x6000

	.align 2
	
.GWBWRAM: /* MAP_BWRAM */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	mov.w .n7fff,r3
	and r4,r3
	mov.w .n6000,r2
	sub r2,r3
	mov.l .BWRAM2,r2
	mov.l @r2,r2
	add r2,r3
	mov.b @r3+,r2
	mov.b @r3,r0
	extu.b r2,r2
	shll8 r0
	or r2,r0
	rts
	extu.w r0,r0

	.align 2
.BWRAM2:
	.long BWRAM
.S9xGetDSP:
#ifndef USE_OLD_DSP1
.global _GetDSP
_GetDSP:
	.long 0
#else
	.long _S9xGetDSP
#endif
.S9xGetC4:
	.long _S9xGetC4
.SRAM0:
	.long SRAM
.n7fff:
	.word 0x7fff
.n6000:
	.word 0x6000

	.align 5
.globl S9xSetByte_r, _S9xSetByte_callback, _S9xSetByte_callback_jmp
#ifdef USE_MMU_DIRECT_INDEX
#ifdef VAR_CYCLES
	add #8,CYCLES
#else
	nop
#endif
#ifdef CPU_SHUTDOWN
	mov #0,r3
	mov.l r3,WaitAddress
#else
	nop
	nop
#endif
	rts
	mov.b r0,@r4
	nop
#endif
S9xSetByte_r:
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
#if !defined(USE_MMU) || defined(USE_MEMORY_SPEED)
	mov r4,r2
#endif
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
#if !defined(USE_MMU) || defined(USE_MEMORY_SPEED)
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK175,r3
	and r3,r2	! r2 = block
#endif
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l .MemorySpeed3,r3
	add r2,r3
#ifndef USE_ASM_R5
#error IMPOSIBLE_NO_R5_Y_MEMORY_SPEED
#endif
	mov.b @r3,r5
#ifdef USE_MMU
	add r5, CYCLES
#endif
#else
	add #8,CYCLES
#endif
#endif

#ifdef USE_MMU
#ifdef USE_MMU_MASK
	mov.l .MMUMASK3, r3
	and	r4, r3
#else
	mov #-12,r2
	mov r4,r3
	shld r2,r3
	mov #1,r2
	cmp/hi r2,r3
	bf.s _S9xSetByte_callback
	add #2,r2
	cmp/hi r2,r3
	bf.s .SBPPU_direct
	add #2,r2
	cmp/hi r2,r3
	bf .SBCPU_direct
#endif
_S9xSetByte_callback:
	rts
#ifdef USE_MMU_MASK
	mov.b r0, @r3
#else
	mov.b r0, @r4
#endif
#ifdef USE_MMU_MASK
.align 2
.MMUMASK3: .long 0x0fffffff
#endif
.align 2
_S9xSetByte_callback_jmp:
	mov r4,r2
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK175,r3
	and r3,r2	! r2 = block
! USE_MMU
#endif
	shll2 r2
	mov.l .WriteMap0,r3
	add r2,r3
	mov.l @r3,r3
#ifndef USE_MMU
	cmp/pz r3
	bt .SBSpecial
	extu.w r4,r2
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	add r5, CYCLES
#endif
#ifdef CPU_SHUTDOWN
	mov.l SA1,r1
	add r2,r3
	mov.l SA1WaitByteAddress1,r2
	cmp/eq r2,r3
	bt .Matched
	mov.l SA1WaitByteAddress2,r2
	cmp/eq r2,r3
	bf .NoMatch
.Matched:
	mov.l SA1Opcodes,r2
	mov.b r0,@r3
	mov #0,r0
	cmp/hi r0,r2
	mov.l r0,SA1WaitCounter
	rotcl r0
	rts
	mov.b r0,SA1Executing
.NoMatch:
	rts
	mov.b r0,@r3
#else
	add r2,r3
	rts
	mov.b r0,@r3
#endif
	.align 2
	
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
.MemorySpeed3:
	.long MemorySpeed
#endif
.MEMMAP_MSK175:	
	.word MEMMAP_MASK
	.align 2

.SBSpecial:
! USE_MMU
#endif
	mov r4,r2
	extu.b r0,r4
	mova .SBJmpTable,r0
	shll2 r3
	add r3,r0
	mov.l @r0,r3
	jmp @r3
#ifdef USE_ASM_R5
	extu.w r2,r5
#else
	nop
#endif
	.align 2
#ifdef USE_MMU
.MEMMAP_MSK175:	
	.word MEMMAP_MASK
	.align 2
#endif
.SBJmpTable:
	.long .SBPPU
	.long .SBCPU
	.long .SBDSP
	.long .SBLSRAM
	.long .SBHSRAM
	.long .SBNONE
	.long .SBDEBUG
	.long .SBC4
	.long .SBBWRAM
	.long .SBNONE
	.long .SBNONE
	.long .SBNONE

	.align 2
#ifdef USE_MMU
.SBPPU_direct:
	mov r4,r2
	extu.b r0,r4
#endif
.SBPPU: /* MAP_PPU */
#ifdef USE_ASM_SET_MATRIX
	mov.w .SBPPU_MATRIX, r0
	mov #3, r3
#ifdef USE_ASM_R5
	add r5, r0
#else
	add r2, r0
#endif
	extu.w r0, r0
	cmp/hi r3, r0
	bt .SBPPU_NO_MATRIX
#ifndef USE_ASM_R5
	extu.w r2, r3
#else
	extu.w r5, r3
#endif
	and #3, r0
	shll r0
	mov.l PPU,r1
	add r0, r1
	add #62, r1
	extu.b r4, r2
	shll8 r2
	mov.w @r1, r0
	shlr8 r0
	extu.b r0, r0
	or r2,r0
	mov.w r0,@r1

	mov.l PPU,r1
	mov #1,r0
	add #51,r1
	mov.b r0,@r1

	mov.l .SBPPU_FillRAM,r0
	rts 
	mov.b r4,@(r0,r3)

.SBPPU_FillRAM:
	.long FillRAM
.SBPPU_MATRIX: .word 0xdee5
	.align 2
.SBPPU_NO_MATRIX:
#endif
#ifdef USE_SETPPU_TABLE
	mov.l .S9xSetPPU_func,r3
#ifdef USE_ASM_R5
	extu.w r5, r0
#else
	extu.w r2, r0
#endif
	shll2 r0
	add r0,r3
#ifdef USE_ASM_PREFETCH
	pref @r3
#endif
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l @r3,r3
	jsr @r3
#ifdef USE_ASM_R5
	nop
#else
	extu.w r2,r5
#endif
	lds.l @r15+,pr
	POP_REGISTERS_i
	rts
	POP_REGISTERS_f
	
#else
#ifdef VAR_CYCLES
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b InDMA,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bf .SBPPU_NOADD
	add #6, CYCLES
.SBPPU_NOADD:
#endif
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetPPU,r3
	jsr @r3
#ifdef USE_ASM_R5
	nop
#else
	extu.w r2,r5
#endif
	lds.l @r15+,pr
	POP_REGISTERS_i
	rts
	POP_REGISTERS_f
#endif

	.align 2
#ifdef USE_MMU
.SBCPU_direct:
	mov r4,r2
	extu.b r0,r4
#endif
.SBCPU: /* MAP_CPU */
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
#ifdef USE_REGISTER_LEFT_TIMESLICE
	mov #2, USE_REGISTER_LEFT_TIMESLICE
#else
	mov.l .left_timeslice,r0
	mov #1, r1
	mov.l r1,@r0
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetCPU,r3
	jsr @r3
#ifdef USE_ASM_R5
	nop
#else
	extu.w r2,r5
#endif
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
#ifndef USE_REGISTER_LEFT_TIMESLICE
.left_timeslice: .long left_timeslice
#endif

.SBDSP: /* MAP_DSP */
#ifdef VAR_CYCLES
	add #8, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetDSP,r3
	jsr @r3
#ifdef USE_ASM_R5
	nop
#else
	extu.w r2,r5
#endif
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.SBLSRAM: /* MAP_LOROM_SRAM */
	mov.l .SRAM1,r0
#ifdef USE_ASM_R5
	mov.l .SRAMMask0,r5
	mov.l @r5,r5
	extu.w r5,r5
	tst r5,r5
	and r5,r2
#else
	mov.l .SRAMMask0,r3
	mov.l @r3,r3
	extu.w r3,r3
	tst r3,r3
	and r3,r2
#endif
	bt .SBLSRAM_SKIP
	mov.b r4,@(r0,r2)
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r4, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
.SBLSRAM_SKIP:
	rts
#ifdef VAR_CYCLES
	add #8, CYCLES
#else
	nop
#endif
	.align 2

.SBHSRAM: /* MAP_HIROM_SRAM */
	mov.w .n7fff15,r3
	and r2,r3
#ifdef USE_ASM_R5
	mov.w .n600015,r5
	sub r5,r3
	mov.l .nf00002,r5
	and r5,r2
#else
	mov.w .n600015,r0
	sub r0,r3
	mov.l .nf00002,r0
	and r0,r2
#endif
	shlr2 r2
	shlr r2
	add r2,r3
	mov.l .SRAM1,r0
	mov.l .SRAMMask0,r2
	mov.l @r2,r2
	extu.w r2,r2
	tst r2,r2
	and r3,r2
	bt .SBHSRAM_SKIP
	mov.b r4,@(r0,r2)
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r4, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
.SBHSRAM_SKIP:
	rts
#ifdef VAR_CYCLES
	add #8, CYCLES
#else
	nop
#endif
	.align 2
.SRAM1:
	.long SRAM
.SRAMMask0:
	.long SRAMMask
	.align 2

.SBNONE:
.SBDEBUG: /* MAP_DEBUG */
	rts
#ifdef VAR_CYCLES
	add #8, CYCLES
#else
	nop
#endif
	.align 2

.SBC4: /* MAP_C4 */
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetC4,r3
	jsr @r3
#ifdef USE_ASM_R5
	nop
#else
	extu.w r2,r5
#endif
	lds.l @r15+,pr
	POP_REGISTERS
	rts
#ifdef VAR_CYCLES
	add #6, CYCLES
#else
	nop
#endif
	.align 2


.SBBWRAM: /* MAP_BWRAM */
	mov.w .n7fff15,r0
	and r2,r0
	mov.w .n600015,r2
	sub r2,r0
	mov.l BWRAM,r2
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r2, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.b r4,@(r0,r2)
	rts
#ifdef VAR_CYCLES
	add #8, CYCLES
#else
	nop
#endif
	.align 2

.n7fff15:
	.word 0x7fff
.n600015:
	.word 0x6000

	.align 5
.globl S9xSetWord_r, _S9xSetWord_callback, _S9xSetWord_callback_jmp

#ifdef USE_MMU_DIRECT_INDEX
#ifdef VAR_CYCLES
	add #8,CYCLES
#else
	nop
#endif
#ifdef CPU_SHUTDOWN
	mov #0,r3
	mov.l r3,WaitAddress
#else
	nop
	nop
#endif
	mov.b r0,@r4
	shlr8 r0
	rts
	mov.b r0,@(1,r4)
	nop
#endif
S9xSetWord_r:
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif	
#if !defined(USE_MMU) || defined(USE_MEMORY_SPEED)
	mov r4,r2
#endif
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
#if !defined(USE_MMU) || defined(USE_MEMORY_SPEED)
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK19,r3
	and r3,r2
#endif
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l .MemorySpeed2,r3
	add r2,r3
#ifndef USE_ASM_R5
#error IMPOSIBLE_NO_R5_Y_MEMORY_SPEED
#endif
	mov.b @r3,r5
#ifdef USE_MMU
	add r5, CYCLES
	add r5, CYCLES
#endif
#else
	add #16,CYCLES
#endif
#endif

#ifdef USE_MMU
#ifdef USE_MMU_MASK
	mov.l .MMUMASK4, r3
	and	r4, r3
#else
	mov #-12,r2
	mov r4,r3
	shld r2,r3
	mov #1,r2
	cmp/hi r2,r3
	bf.s _S9xSetWord_callback
	add #2,r2
	cmp/hi r2,r3
	bf.s .SWPPU_direct
	add #2,r2
	cmp/hi r2,r3
	bf .SWCPU_direct
#endif
_S9xSetWord_callback:
#ifdef USE_MMU_MASK
	mov.b r0,@r3
#else
	mov.b r0,@r4
#endif
	shlr8 r0
	rts
	mov.b r0,@(1,r4)
#ifdef USE_MMU_MASK
.align 2
.MMUMASK4: .long 0x0fffffff
#endif
.align 2
_S9xSetWord_callback_jmp:
	mov r4,r2
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK19,r3
	and r3,r2
! USE_MMU
#endif
	shll2 r2
	mov.l .WriteMap0,r3
	add r2,r3
	mov.l @r3,r3
#ifndef USE_MMU
	cmp/pz r3
	bt .SWSpecial
	extu.w r4,r2
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	add r5, CYCLES
	add r5, CYCLES
#endif
#ifdef CPU_SHUTDOWN
	mov.l SA1,r1
	add r2,r3
	mov.l SA1WaitByteAddress1,r2
	mov.b r0,@r3
	cmp/eq r2,r3
	bt/s .Matched2
	shlr8 r0
	mov.l SA1WaitByteAddress2,r2
	cmp/eq r2,r3
	bf .NoMatch2
.Matched2:
	mov.l SA1Opcodes,r2
	mov.b r0,@(1,r3)
	mov #0,r0
	cmp/hi r0,r2
	mov.l r0,SA1WaitCounter
	rotcl r0
	rts
	mov.b r0,SA1Executing
.NoMatch2:
	rts
	mov.b r0,@(1,r3)
#else
	add r2,r3
	mov.b r0,@r3
	shlr8 r0
	rts
	mov.b r0,@(1,r3)
#endif
	
	.align 2
.WriteMap0:
	.long WriteMap
.MEMMAP_MSK19:	
	.word MEMMAP_MASK
	.align 2

.SWSpecial:
! USE_MMU
#endif
	mov r4,r2
	extu.w r0,r4
	mova .SWJmpTable,r0
	shll2 r3
	add r3,r0
	mov.l @r0,r3
	jmp @r3
#ifdef USE_ASM_R5
	extu.w r2,r5
#else
	nop
#endif
	.align 2
#ifdef USE_MMU
.WriteMap0:
	.long WriteMap
.MEMMAP_MSK19:	
	.word MEMMAP_MASK
	.align 2
#endif
.SWJmpTable:
	.long .SWPPU
	.long .SWCPU
	.long .SWDSP
	.long .SWLSRAM
	.long .SWHSRAM
	.long .SWNONE
	.long .SWDEBUG
	.long .SWC4
	.long .SWBWRAM
	.long .SWNONE
	.long .SWNONE
	.long .SWNONE

	.align 2
#ifdef USE_MMU
.SWPPU_direct:
	mov r4,r2
	extu.w r0,r4
#endif
.SWPPU: /* MAP_PPU */
#ifdef USE_ASM_SET_MATRIX
	mov.w .SWPPU_MATRIX, r0
	mov #3, r3
#ifdef USE_ASM_R5
	add r5, r0
#else
	add r2, r0
#endif
	extu.w r0, r0
	cmp/hi r3, r0
	bt .SWPPU_NO_MATRIX
#ifndef USE_ASM_R5
	extu.w r2, r3
#else
	extu.w r5, r3
#endif
	and #3, r0
	shll r0
	mov.l PPU,r1
	add r0, r1
	add #62, r1
	extu.w r4, r2
	shlr8 r2
	shll8 r2
	mov.w @r1, r0
	shlr8 r0
	extu.b r0, r0
	or r2,r0
	mov.w r0,@r1

	add #2,r1
	extu.b r4,r2
	mov.w @r1, r0
	shlr8 r0
	extu.b r0, r0
	or r2,r0
	mov.w r0,@r1

	mov.l PPU,r1
	mov #1,r0
	add #51,r1
	mov.b r0,@r1

	mov.l .FillRAM0,r0
	rts 
	mov.b r4,@(r0,r3)

.SWPPU_MATRIX: .word 0xdee5
	.align 2
.SWPPU_NO_MATRIX:
#endif
#ifdef USE_SETPPU_TABLE
	mov.l .S9xSetPPU_func,r3
#ifdef USE_ASM_R5
	extu.w r5,r0
#else
	extu.w r2,r0
#endif
	shll2 r0
	add r0,r3
#ifdef USE_ASM_PREFETCH
	pref @r3
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l @r3,r3
#ifndef USE_ASM_R5
	extu.w r2,r5
#endif
	mov.l r4,@-r15
	mov.l r5,@-r15
	jsr @r3
	extu.b r4,r4
	mov.l .S9xSetPPU_func,r3
	mov.l @r15+,r5
	mov.l @r15+,r4
	add #1,r5
	extu.w r5,r0
	shll2 r0
	mov.l @(r0,r3),r3
	shlr8 r4
	jsr @r3
	extu.w r5,r5
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

        .align 2
.S9xSetPPU_func:
        .long _S9xSetPPU_func

#else
#ifdef VAR_CYCLES
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b InDMA,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bf .SWPPU_NOADD
	add #12, CYCLES
.SWPPU_NOADD:
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetPPU,r3
#ifndef USE_ASM_R5
	extu.w r2,r5
#endif
	mov.l r4,@-r15
	mov.l r5,@-r15
	jsr @r3
	extu.b r4,r4
	mov.l .S9xSetPPU,r3
	mov.l @r15+,r5
	mov.l @r15+,r4
	add #1,r5
	shlr8 r4
	jsr @r3
	extu.w r5,r5
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.S9xSetPPU:
	.long _S9xSetPPU
#endif

	.align 2
#ifdef USE_MMU
.SWCPU_direct:
	mov r4,r2
	extu.w r0,r4
#endif
.SWCPU: /* MAP_CPU */
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetCPU,r3
	mov.l r4,@-r15
#ifndef USE_ASM_R5
	extu.w r2,r5
#endif
	mov.l r5,@-r15
	jsr @r3
	extu.b r4,r4
	mov.l .S9xSetCPU,r3
	mov.l @r15+,r5
	mov.l @r15+,r4
	add #1,r5
	shlr8 r4
	jsr @r3
	extu.w r5,r5
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES
	
	.align 2
.S9xSetCPU:
	.long _S9xSetCPU
	.align 2
.SWDSP: /* MAP_DSP */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetDSP,r3
	mov.l r4,@-r15
#ifndef USE_ASM_R5
	extu.w r2,r5
#endif
	mov.l r5,@-r15
	jsr @r3
	extu.b r4,r4
	mov.l .S9xSetDSP,r3
	mov.l @r15+,r5
	mov.l @r15+,r4
	add #1,r5
	shlr8 r4
	jsr @r3
	extu.w r5,r5
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2
.S9xSetDSP:
#ifndef USE_OLD_DSP1
.global _SetDSP
_SetDSP:
	.long 0
#else
	.long _S9xSetDSP
#endif
	.align 2
.SWLSRAM: /* MAP_LOROM_SRAM */
	mov.l .SRAM2,r0
#ifdef USE_ASM_R5
	mov.l .SRAMMask1,r5
	mov.l @r5,r5
	extu.w r5,r1
	tst r1,r1
	mov r1,r5
	and r2,r1
	bt/s .SWLSRAM_SKIP
	mov.b r4,@(r0,r1)
	add #1,r2
	shlr8 r4
	and r2,r5
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r4, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	rts
	mov.b r4,@(r0,r5)
#else
	mov.w SRAMMask,r1
	extu.w r1,r1
	tst r1,r1
	mov r1,r3
	and r2,r1
	bt/s .SWLSRAM_SKIP
	mov.b r4,@(r0,r1)
	add #1,r2
	shlr8 r4
	and r2,r3
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r4, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	rts
	mov.b r4,@(r0,r3)
#endif
.SWLSRAM_SKIP:
	rts
#ifdef VAR_CYCLES
	add #16, CYCLES
#else
	nop
#endif
#ifdef USE_ASM_R5
	.align 2
.SRAMMask1:
	.long SRAMMask
#endif
	.align 2

.SWHSRAM: /* MAP_HIROM_SRAM */
	mov.w .n7fff2,r3
	and r2,r3
#ifdef USE_ASM_R5
	mov.w .n60003,r5
	sub r5,r3
	mov.l .nf00002,r5
	and r5,r2
#else
	mov.w .n60003,r0
	sub r0,r3
	mov.l .nf00002,r0
	and r0,r2
#endif
	shlr2 r2
	shlr r2
	add r2,r3
	mov.l .SRAM2,r0
	mov.w SRAMMask,r2
	extu.w r2,r2
	tst r2,r2
	and r3,r2
	bt .SWHSRAM_SKIP
	mov.b r4,@(r0,r2)
	add #1,r2
	shlr8 r4
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r4, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	rts
	mov.b r4,@(r0,r2)
.SWHSRAM_SKIP:
	rts
#ifdef VAR_CYCLES
	add #16, CYCLES
#else
	nop
#endif
	.align 2
.nf00002:
	.long 0xf0000
.global SRAMMask
SRAMMask:
	.long 0
	.align 2

.SWNONE:
.SWDEBUG: /* MAP_DEBUG */
	rts
#ifdef VAR_CYCLES
	add #16, CYCLES
#else
	nop
#endif
	.align 2

.SWC4: /* MAP_C4 */
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	SAVE_CYCLES
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetC4,r3
	mov.l r4,@-r15
#ifndef USE_ASM_R5
	extu.w r2,r5
#endif
	mov.l r5,@-r15
	jsr @r3
	extu.b r4,r4
	mov.l .S9xSetC4,r3
	mov.l @r15+,r5
	mov.l @r15+,r4
	add #1,r5
	shlr8 r4
	jsr @r3
	extu.w r5,r5
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	LOAD_CYCLES

	.align 2

.SWBWRAM: /* MAP_BWRAM */
#ifdef VAR_CYCLES
	add #16, CYCLES
#endif
	mov.w .n7fff2,r3
	and r2,r3
	mov.w .n60002,r2
	sub r2,r3
	mov.l BWRAM,r0
	mov.b r4,@(r0,r3)
	add #1,r3
	shlr8 r4
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r4, SRAMModified
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	rts
	mov.b r4,@(r0,r3)
	.align 2
.global BWRAM
BWRAM:	
	.long 0
.S9xSetC4:
	.long _S9xSetC4

	.align 5
.globl S9xSetPCBase_r

S9xSetPCBase_r:
	mov r4,r2
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK2,r3
	and r3,r2	! r2 = block
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov.l .MemorySpeed2,r3
	add r2,r3
	mov.b @r3,r1
#endif

	shll2 r2
	mov.l .Map2,r3
	add r2,r3
	mov.l @r3,r3
	cmp/pz r3
	bt .SPCSpecial
	extu.w r4,r2
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov.l r1,MemSpeed
	shll r1
	mov.l r1,MemSpeedx2
#endif
#ifdef USE_MMU
	mov.l .SPCMask,r3
	and r4,r3
	mov.l r3,PCBase
#ifdef USE_MMU_MASK
	mov.l .MMUMASK5, r3
	and r4,r3
	rts
	mov r3,PC
#else
	rts
	mov r4,PC
#endif
.align 2
.SPCMask:
	.long 0x0fff0000
#ifdef USE_MMU_MASK
.MMUMASK5: .long 0x0fffffff
#endif
#else
	mov.l r3,PCBase
	add r2,r3
	rts
	mov r3,PC
#endif

.SPCSpecial:
	mova .SPCJmpTable,r0
	shll2 r3
	add r3,r0
	mov.l @r0,r3
	jmp @r3
	nop

	.align 2
.SPCJmpTable:
	.long .SPCPPU
	.long .SPCCPU
	.long .SPCDSP
	.long .SPCLSRAM
	.long .SPCHSRAM
	.long .SPCNONE
	.long .SPCDEBUG
	.long .SPCC4
	.long .SPCBWRAM
	.long .SPCNONE
	.long .SPCNONE
	.long .SPCNONE

#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
.MemorySpeed2:
	.long MemorySpeed
#endif
.n7fff2:
	.word 0x7fff
.n60003:
	.word 0x6000

	.align 2
.SPCPPU: /* MAP_PPU */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #6,r1
	mov.l r1, MemSpeed
	mov #12,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
//	mov.l .FillRAM0,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .FillRAM0,PC
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SPCCPU: /* MAP_CPU */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #6,r1
	mov.l r1, MemSpeed
	mov #12,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
//	mov.l .FillRAM0,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .FillRAM0,PC
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SPCDSP: /* MAP_DSP */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #8,r1
	mov.l r1, MemSpeed
	mov #16,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
//	mov.l .FillRAM0,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .FillRAM0,PC
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SPCLSRAM: /* MAP_LOROM_SRAM */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #8,r1
	mov.l r1, MemSpeed
	mov #16,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
//	mov.l .SRAM2,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .SRAM2,PC
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SPCHSRAM: /* MAP_HIROM_SRAM */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #8,r1
	mov.l r1, MemSpeed
	mov #16,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
//	mov.l .SRAM2,r3
	mov.w .n60002,r2
//	mov.l r3,PC
	mov.l .SRAM2,PC
	add r2,PC
	extu.w r4,r2
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SPCNONE:
.SPCDEBUG: /* MAP_DEBUG */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #8,r1
	mov.l r1, MemSpeed
	mov #16,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
	mov.l .SRAM2,PC
	extu.w r4,r2
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SRAM2:
	.long SRAM
	.align 2
.SPCC4: /* MAP_C4 */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #8,r1
	mov.l r1, MemSpeed
	mov #16,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
//	mov.l .C4RAM0, r3
	mov.w .n60002,r2
//	mov.l r3,PC
	mov.l .C4RAM0,PC
	sub r2,PC
	extu.w r4,r2
	mov.l PC,PCBase
	rts
	add r2,PC


	.align 2
.SPCBWRAM: /* MAP_BWRAM */
#if defined(VAR_CYCLES) && defined(USE_MEMORY_SPEED)
	mov #8,r1
	mov.l r1, MemSpeed
	mov #16,r1
	mov.l r1, MemSpeedx2
#endif
// CUIDADO:
	mov.l .SRAM2_,r3
	mov.w .n60002,r2
	add r2,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov r3,PC
	mov.l PC,PCBase
	rts
	add r2,PC

	.align 2
.SRAM2_:
	.long SRAM
.Map2:
	.long Map
.FillRAM0:
	.long FillRAM
.C4RAM0:
	.long C4RAM
.n60002:
	.word 0x6000
.MEMMAP_MSK2:	
	.word MEMMAP_MASK

	.align 2


