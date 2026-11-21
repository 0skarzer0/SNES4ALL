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

	.align 2
S9xSA1GetByte_r:
	mov r4,r0
	mov #-MEMMAP_SHIFT,r3
	shld r3,r0
	mov.w .MEMMAP_MSK1,r3
	and r3,r0
	shll2 r0
	mov #SA1Map_offs,r3
	add SA1REG,r3
	mov.l @(r0,r3),r3
	cmp/pz r3
	bt .GBSpecial
	extu.w r4,r2
	add r2,r3
	mov.b @r3,r0
	rts
	extu.b r0,r0

	.align 2
.MEMMAP_MSK1:	
	.word MEMMAP_MASK
	.align 2

.GBSpecial:
	mova .GBJmpTable,r0
	shll2 r3
	add r3,r0
	mov.l @r0,r3
	jmp @r3
	nop

	.align 2

.GBPPU: /* MAP_PPU */
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xGetSA1,r3
	mov.l r4,@-r15
	jsr @r3
	extu.w r4,r4
	mov.l @r15+,r4
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	extu.b r0,r0

	.align 2
.GBCPU: /* MAP_CPU */

.GBDSP: /* MAP_DSP */
	rts
	mov #0,r0

	.align 2
.GBLSRAM: /* MAP_LOROM_SRAM */
	mov.l .SRAM0,r3
	mov.l .SRAMMask0,r2
	mov.w @r2,r2
	extu.w r2,r2
	and r4,r2
	add r2,r3
	mov.b @r3,r0
	rts
	extu.b r0,r0

	.align 2
.GBHSRAM: /* MAP_HIROM_SRAM */
	mov.w .n7fffA,r3
	and r4,r3
	mov.w .n6000A,r2
	sub r2,r3
	mov.l .nf0000,r2
	and r4,r2
	shlr2 r2
	shlr r2
	add r2,r3
	mov.l .SRAM0,r0
	mov.l .SRAMMask0,r2
	mov.w @r2,r2
	extu.w r2,r2
	and r3,r2
	mov.b @(r0,r2),r0
	rts
	extu.b r0,r0

	.align 2
.GBNONE:
.GBDEBUG: /* MAP_DEBUG */
	rts
	mov #0,r0

	.align 2
.GBBWRAM: /* MAP_BWRAM */
	mov.w .n7fffA,r3
	and r4,r3
	mov.w .n6000A,r2
	sub r2,r3
	mov.l SA1BWRAM,r0
	mov.b @(r0,r3),r0
	rts
	extu.b r0,r0

	.align 2
.GBBWBITMAP:
	mov.l .FillRAM0,r3
	mov.w .n6000A,r2
	mov.w .n223f0,r0
	shll8 r2
	mov.b @(r0,r3),r0
	mov r4,r3
	tst #0x80,r0
	sub r2,r3
	bt .GB4

	// Depth 2
	mov #3,r2
	shlr2 r3
	and r4,r2
	extu.w r3,r3
// CUIDADO:
	mov.l .SRAM0,r0
	shll r2
	mov.b @(r0,r3),r0
	neg r2,r2
	shld r2,r0
	rts
	and #3,r0

.GB4:
	mov #1,r2
	shlr r3
	and r4,r2
	extu.w r3,r3
// CUIDADO:
	mov.l .SRAM0,r0
	shll2 r2
	mov.b @(r0,r3),r0
	neg r2,r2
	shld r2,r0
	rts
	and #15,r0


	.align 2
.GBBWBITMAP2:
	mov.l .FillRAM0,r3
	mov.w .n6000A,r2
	mov.w .n223f0,r0
	mov.b @(r0,r3),r0
	extu.w r4,r3
	tst #0x80,r0
	sub r2,r3
	bt .GB42

	// Depth 2
	mov #3,r2
	shlr2 r3
	and r4,r2
	extu.w r3,r3
	mov.l SA1BWRAM,r0
	shll r2
	mov.b @(r0,r3),r0
	neg r2,r2
	shld r2,r0
	rts
	and #3,r0

.GB42:
	mov #1,r2
	shlr r3
	and r4,r2
	extu.w r3,r3
	mov.l SA1BWRAM,r0
	shll2 r2
	mov.b @(r0,r3),r0
	neg r2,r2
	shld r2,r0
	rts
	and #15,r0

	.align 2
.n7fffA:
	.word 0x7fff
.n6000A:
	.word 0x6000
.n223f0:
	.word 0x223f
	.align 2


S9xSA1GetWord_r:
	sts.l pr,@-r15
	bsr S9xSA1GetByte_r
	nop
	mov.l r0,@-r15
	bsr S9xSA1GetByte_r
	add #1,r4
	mov.l @r15+,r1
	add #-1,r4
	lds.l @r15+,pr
	shll8 r0
	rts
	or r1,r0

	.align 2
S9xSA1SetByte_r:
	mov r4,r2
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK_,r3
	and r3,r2

	shll2 r2
	mov.w .wSA1WriteMap_offs,r3
	add SA1REG,r3
	add r2,r3
	mov.l @r3,r3
	cmp/pz r3
	bt .SBSpecial
	extu.w r4,r2
	add r2,r3
	rts
	mov.b r0,@r3

	.align 2
.MEMMAP_MSK_:	
	.word MEMMAP_MASK
.wSA1WriteMap_offs:
	.word SA1WriteMap_offs
	.align 2

.SBSpecial:
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

.SBPPU: /* MAP_PPU */
	PUSH_REGISTERS
	sts.l pr,@-r15
	mov.l .S9xSetSA1,r3
	jsr @r3
#ifdef USE_ASM_R5
	nop
#else
	extu.w r2,r5
#endif
	lds.l @r15+,pr
	POP_REGISTERS
	rts
	nop
	
	.align 2
.SBCPU: /* MAP_CPU */
	rts
	nop

	.align 2
.SBDSP: /* MAP_DSP */
	rts
	nop

	.align 2
.SBLSRAM: /* MAP_LOROM_SRAM */
	mov.l .SRAM0,r0
#ifdef USE_ASM_R5
	mov.l .SRAMMask0,r5
	mov.w @r5,r5
	extu.w r5,r5
	tst r5,r5
	and r5,r2
#else
	mov.l .SRAMMask0,r3
	mov.w @r3,r3
	extu.w r3,r3
	tst r3,r3
	and r3,r2
#endif
	bt .SBLSRAM_SKIP
	mov.b r4,@(r0,r2)
.SBLSRAM_SKIP:
	rts
	nop

	.align 2
.SBHSRAM: /* MAP_HIROM_SRAM */
	mov.w .n7fff,r3
	and r4,r3
#ifdef USE_ASM_R5
	mov.w .n6000,r5
	sub r5,r3
	mov.l .nf0000,r5
	and r5,r2
#else
	mov.w .n6000,r0
	sub r0,r3
	mov.l .nf0000,r0
	and r0,r2
#endif
	shlr2 r2
	shlr r2
	add r2,r3
	mov.l .SRAM0,r0
	mov.l .SRAMMask0,r2
	mov.w @r2,r2
	extu.w r2,r2
	tst r2,r2
	and r3,r2
	bt .SBHSRAM_SKIP
	mov.b r4,@(r0,r2)
.SBHSRAM_SKIP:
	rts
	nop

	.align 2
.SBNONE:
.SBDEBUG: /* MAP_DEBUG */
	rts
	nop

	.align 2
.SBBWRAM: /* MAP_BWRAM */
	mov.w .n7fff,r0
	and r2,r0
	mov.w .n6000,r2
	sub r2,r0
	mov.l SA1BWRAM,r2
	rts
	mov.b r4,@(r0,r2)

	.align 2
.SBBWBITMAP:
	mov.l .FillRAM0,r0
	mov.w .n223f,r2
	mov.l .nnx600000,r1
	mov.b @(r0,r2),r0
	tst #0x80,r0
	bt/s .SB4
	add r2,r1

	// Depth 2
	mov #3,r0
	shlr2 r1
	and r2,r0
	extu.w r1,r1
// CUIDADO:
	mov.l .SRAM0,r2
	shll r0
	add r2,r1
	mov #3,r2
	mov #3,r3
	shld r0,r2
	and r4,r3
	not r2,r2
	shld r0,r3
	mov.b @r1,r0
	and r2,r0
	or r3,r0
	rts
	mov.b r0,@r1

.SB4:
	mov #1,r0
	shlr r1
	and r2,r0
	extu.w r1,r1
// CUIDADO:
	mov.l .SRAM0,r2
	shll2 r0
	add r2,r1
	mov #15,r2
	mov #15,r3
	shld r0,r2
	and r4,r3
	not r2,r2
	shld r0,r3
	mov.b @r1,r0
	and r2,r0
	or r3,r0
	rts
	mov.b r0,@r1


	.align 2
.SBBWBITMAP2:
#ifndef USE_ASM_R5
	extu.w r2,r3
#endif
	mov.l .FillRAM0,r0
	mov.w .n223f,r2
	mov.w .nnx6000,r1
	mov.b @(r0,r2),r0
	tst #0x80,r0
	bt/s .SB42
#ifdef USE_ASM_R5
	add r5,r1
#else
	add r3,r1
#endif

	// Depth 2
	mov #3,r0
	shlr2 r1
#ifdef USE_ASM_R5
	and r5,r0
#else
	and r3,r0
#endif
	extu.w r1,r1
	mov.l SA1BWRAM,r2
	shll r0
	add r2,r1
	mov #3,r2
	mov #3,r3
	shld r0,r2
	and r4,r3
	not r2,r2
	shld r0,r3
	mov.b @r1,r0
	and r2,r0
	or r3,r0
	rts
	mov.b r0,@r1

.SB42:
	mov #1,r0
	shlr r1
#ifdef USE_ASM_R5
	and r5,r0
#else
	and r3,r0
#endif
	extu.w r1,r1
	mov.l SA1BWRAM,r2
	shll2 r0
	add r2,r1
	mov #15,r2
	mov #15,r3
	shld r0,r2
	and r4,r3
	not r2,r2
	shld r0,r3
	mov.b @r1,r0
	and r2,r0
	or r3,r0
	rts
	mov.b r0,@r1

	.align 2

.GBJmpTable:
	.long .GBPPU
	.long .GBCPU
	.long .GBDSP
	.long .GBLSRAM
	.long .GBHSRAM
	.long .GBNONE
	.long .GBDEBUG
	.long .GBNONE
	.long .GBBWRAM
	.long .GBBWBITMAP
	.long .GBBWBITMAP2

.SBJmpTable:
	.long .SBPPU
	.long .SBCPU
	.long .SBDSP
	.long .SBLSRAM
	.long .SBHSRAM
	.long .SBNONE
	.long .SBDEBUG
	.long .SBNONE
	.long .SBBWRAM
	.long .SBBWBITMAP
	.long .SBBWBITMAP2

.S9xGetSA1:
	.long _S9xGetSA1
.S9xSetSA1:
	.long _S9xSetSA1
.SRAM0:
	.long SRAM
.SRAMMask0:
	.long SRAMMask
.FillRAM0:
	.long FillRAM
.nf0000:
	.long 0xf0000
.nnx600000:
	.long -0x600000
.n7fff:
	.word 0x7fff
.n6000:
	.word 0x6000
.nnx6000:
	.word -0x6000
.n223f:
	.word 0x223f
	.align 2
		
S9xSA1SetWord_r:
	sts.l pr,@-r15
	mov.l r0,@-r15
	bsr S9xSA1SetByte_r
	mov.l r4,@-r15
	mov.l @r15+,r4
	mov.l @r15+,r0
	add #1,r4
	shlr8 r0
	bra S9xSA1SetByte_r
	lds.l @r15+,pr

S9xSA1SetPCBase_r:
	mov r4,r2
	mov #-MEMMAP_SHIFT,r3
	shld r3,r2
	mov.w .MEMMAP_MSK,r3
	and r3,r2
	shll2 r2
	mov #SA1Map_offs,r3
	add SA1REG,r3
	add r2,r3
	mov.l @r3,r3
	cmp/pz r3
	bt .SPCSpecial
	extu.w r4,r2
	mov.l r3,SA1PCBase
	add r2,r3
	rts
	mov r3,PC

	.align 2
.MEMMAP_MSK:	
	.word MEMMAP_MASK
	.align 2

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
	.long .SPCNONE
	.long .SPCBWRAM
	.long .SPCNONE
	.long .SPCNONE


	.align 2
.SPCPPU: /* MAP_PPU */
// CUIDADO:
//	mov.l .FillRAMB,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .FillRAMB,PC
	mov.l PC,SA1PCBase
	rts
	add r2,PC

	.align 2
.SPCCPU: /* MAP_CPU */
// CUIDADO:
//	mov.l .FillRAMB,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .FillRAMB,PC
	mov.l PC,SA1PCBase
	rts
	add r2,PC

	.align 2
.SPCDSP: /* MAP_DSP */
// CUIDADO:
//	mov.l .FillRAMB,r3
	extu.w r4,r2
//	mov.l r3,PC
	mov.l .FillRAMB,PC
	mov.l PC,SA1PCBase
	rts
	add r2,PC

	.align 2
.SPCLSRAM: /* MAP_LOROM_SRAM */
	mov.l .SRAM2,r3
	extu.w r4,r2
	mov.l @r3,PC
	mov.l PC,SA1PCBase
	rts
	add r2,PC

	.align 2
.SPCHSRAM: /* MAP_HIROM_SRAM */
	mov.l .SRAM2,r3
	mov.w .n60002,r2
	mov.l @r3,PC
	add r2,PC
	extu.w r4,r2
	mov.l PC,SA1PCBase
	rts
	add r2,PC

	.align 2
.SPCNONE:
.SPCDEBUG: /* MAP_DEBUG */
	mov.l .SRAM2,PC
	rts
	mov.l PC,SA1PCBase

	.align 2
.SPCBWRAM: /* MAP_BWRAM */
	mov.l SA1BWRAM,r3
	mov.w .n60002,r2
	add r2,r3
	extu.w r4,r2
	mov.l @r3,PC
	mov.l PC,SA1PCBase
	rts
	add r2,PC

	.align 2
.SRAM2:
	.long SRAM
.FillRAMB:
	.long FillRAM
.n60002:
	.word 0x6000
	.align 2

