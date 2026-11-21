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
#include "asmaddr.h"
#include "asmops.h"
#include "getset.S"


.macro call_S9xGetByte LASTOPCODE
	mov.l S9xGetByte,r3
	jsr @r3
	\LASTOPCODE
.endm

.macro call_S9xGetWord LASTOPCODE
	mov.l S9xGetWord,r3
	jsr @r3
	\LASTOPCODE
.endm

.align 5
	
/* ADC */
Op69M1:
	Immediate8 ADC
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 IMM8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op69M0:
	Immediate16 ADC2
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 IMM16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op65M1:
	Direct8_i ADC2
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DIR8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op65M0:
	Direct8_i ADC
	call_S9xGetWord Direct8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DIR16
#endif
	
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op75M1:
	DirectIndexedX8_i ADC
	call_S9xGetByte DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DIX8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op75M0:
	DirectIndexedX8_i ADC2
	call_S9xGetWord DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DIX16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op72M1:
	DirectIndirect8_i ADC
	call_S9xGetByte DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DI8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op72M0:
	DirectIndirect8_i ADC2
	call_S9xGetWord DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DI16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op61M1:
	DirectIndexedIndirect8_i ADC
	call_S9xGetByte DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DII8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op61M0:
	DirectIndexedIndirect8_i ADC2
	call_S9xGetWord DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DII16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op71M1:
	DirectIndirectIndexed8_i ADC
	call_S9xGetByte DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DIIY8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op71M0:
	DirectIndirectIndexed8_i ADC2
	call_S9xGetWord DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DIIY16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op67M1:
	DirectIndirectLong8_i ADC
	call_S9xGetByte DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DIL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op67M0:
	DirectIndirectLong8_i ADC2
	call_S9xGetWord DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DIL16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op77M1:
	DirectIndirectIndexedLong8_i ADC
	call_S9xGetByte DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 DIIL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op77M0:
	DirectIndirectIndexedLong8_i ADC2
	call_S9xGetWord DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 DIIL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6DM1:
	Absolute8_i ADC
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 ABS8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6DM0:
	Absolute8_i ADC2
	call_S9xGetWord Absolute8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 ABS16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7DM1:
	AbsoluteIndexedX8_i ADC
	call_S9xGetByte AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 ABSX8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7DM0:
	AbsoluteIndexedX8_i ADC2
	call_S9xGetWord AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 ABSX16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op79M1:
	AbsoluteIndexedY8_i ADC
	call_S9xGetByte AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 ABSY8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op79M0:
	AbsoluteIndexedY8_i ADC2
	call_S9xGetWord AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 ABSY16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6FM1:
	AbsoluteLong8_i ADC
	call_S9xGetByte AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 ABSL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6FM0:
	AbsoluteLong8_i ADC2
	call_S9xGetWord AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 ABSL16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7FM1:
	AbsoluteLongIndexedX8_i ADC
	call_S9xGetByte AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 ALX8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7FM0:
	AbsoluteLongIndexedX8_i ADC2
	call_S9xGetWord AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 ALX16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op63M1:
	StackRelative8_i ADC
	call_S9xGetByte StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .Op73M1_Adc8
	nop
#else
	Adc8 SREL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op63M0:
	StackRelative8_i ADC2
	call_S9xGetWord StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .Op73M0_Adc16
	nop
#else
	Adc16 SREL16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op73M1:
	StackRelativeIndirectIndexed8_i ADC
	call_S9xGetByte StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.Op73M1_Adc8:
#endif
	Adc8 SRII8

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op73M0:
	StackRelativeIndirectIndexed8_i ADC2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.Op73M0_Adc16:
#endif
	Adc16 SRII16

/* AND */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op29M1:
	Immediate8 AND
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 IMM8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op29M0:
	Immediate16 AND
	And16 IMM16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op25M1:
	Direct8_i AND
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DIR8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op25M0:
	Direct8_i AND2
	call_S9xGetWord Direct8_f
	And16 DIR16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op35M1:
	DirectIndexedX8_i AND
	call_S9xGetByte DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DIX8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op35M0:
	DirectIndexedX8_i AND2
	call_S9xGetWord DirectIndexedX8_f
	And16 DIX16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op32M1:
	DirectIndirect8_i AND
	call_S9xGetByte DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DI8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op32M0:
	DirectIndirect8_i AND2
	call_S9xGetWord DirectIndirect8_f
	And16 DI16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op21M1:
	DirectIndexedIndirect8_i AND
	call_S9xGetByte DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DII8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op21M0:
	DirectIndexedIndirect8_i AND2
	call_S9xGetWord DirectIndexedIndirect8_f
	And16 DII16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op31M1:
	DirectIndirectIndexed8_i AND
	call_S9xGetByte DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DIIY8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op31M0:
	DirectIndirectIndexed8_i AND2
	call_S9xGetWord DirectIndirectIndexed8_f
	And16 DIIY16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op27M1:
	DirectIndirectLong8_i AND
	call_S9xGetByte DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DIL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op27M0:
	DirectIndirectLong8_i AND2
	call_S9xGetWord DirectIndirectLong8_f
	And16 DIL16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op37M1:
	DirectIndirectIndexedLong8_i AND
	call_S9xGetByte DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 DIIL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op37M0:
	DirectIndirectIndexedLong8_i AND2
	call_S9xGetWord DirectIndirectIndexedLong8_f
	And16 DIIL16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2DM1:
	Absolute8_i AND
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 ABS8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2DM0:
	Absolute8_i AND2
	call_S9xGetWord Absolute8_f
	And16 ABS16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3DM1:
	AbsoluteIndexedX8_i AND
	call_S9xGetByte AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 ABSX8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3DM0:
	AbsoluteIndexedX8_i AND2
	call_S9xGetWord AbsoluteIndexedX8_f
	And16 ABSX16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op39M1:
	AbsoluteIndexedY8_i AND
	call_S9xGetByte AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 ABSY8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op39M0:
	AbsoluteIndexedY8_i AND2
	call_S9xGetWord AbsoluteIndexedY8_f
	And16 ABSY16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2FM1:
	AbsoluteLong8_i AND
	call_S9xGetByte AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 ABSL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2FM0:
	AbsoluteLong8_i AND2
	call_S9xGetWord AbsoluteLong8_f
	And16 ABSL16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3FM1:
	AbsoluteLongIndexedX8_i AND
	call_S9xGetByte AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 ALX8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3FM0:
	AbsoluteLongIndexedX8_i AND2
	call_S9xGetWord AbsoluteLongIndexedX8_f
	And16 ALX16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op23M1:
	StackRelative8_i AND
	call_S9xGetByte StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .Op33M1_And8
	nop
#else
	And8 SREL8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op23M0:
	StackRelative8_i AND2
	call_S9xGetWord StackRelative8_f
	And16 SREL16

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op33M1:
	StackRelativeIndirectIndexed8_i AND
	call_S9xGetByte StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.Op33M1_And8:
#endif
	And8 SRII8

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op33M0:
	StackRelativeIndirectIndexed8_i AND2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
	And16 SRII16

/* ASL */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0AM1:
	extu.b A,r1
	sub r1,A
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	shll r1
	extu.b r1,r2
	mov #Carry,r0
	or r2,A
	cmp/eq r1,r2
	bf/s .Asl8Carry
	or r0,FLAGS
	add #-Carry,FLAGS
.Asl8Carry:
	SetZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0AM0:
	extu.w A,r1
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	shll r1
	mov #Carry,r0
	extu.w r1,A
	cmp/eq r1,A
	bf/s .Asl16Carry
	or r0,FLAGS
	add #-Carry,FLAGS
.Asl16Carry:
	Set16ZNjmp A
	
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op06M1:
	Direct8 ASL
#ifdef USE_ASM_SMALL
	bra .Op1EM1_Asl8
	nop
#else
	Asl8 DIR8
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op06M0:
	Direct8 ASL2
#ifdef USE_ASM_SMALL
	bra .Op1EM0_Asl16
	nop
#else
	Asl16 DIR16
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op16M1:
	DirectIndexedX8 ASL
#ifdef USE_ASM_SMALL
	bra .Op1EM1_Asl8
	nop
#else
	Asl8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op16M0:
	DirectIndexedX8 ASL2
#ifdef USE_ASM_SMALL
	bra .Op1EM0_Asl16
	nop
#else
	Asl16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0EM1:
	Absolute8 ASL
#ifdef USE_ASM_SMALL
	bra .Op1EM1_Asl8
	nop
#else
	Asl8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0EM0:
	Absolute8 ASL2
#ifdef USE_ASM_SMALL
	bra .Op1EM0_Asl16
	nop
#else
	Asl16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1EM1:
	AbsoluteIndexedX8 ASL
#ifdef USE_ASM_SMALL
.Op1EM1_Asl8:
#endif
	Asl8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1EM0:
	AbsoluteIndexedX8 ASL2
#ifdef USE_ASM_SMALL
.Op1EM0_Asl16:
#endif
	Asl16 ABSX

/* BIT */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op89M1:
	Immediate8 BIT
	and A, r0
	extu.w r7,r3
	sub r3,r7
	jmp @r13
	or r0,r7

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op89M0:
	Immediate16 BIT
	and A, r0
	extu.w r7,r3
	sub r3,r7
	jmp @r13
	or r0,r7

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op24M1:
	Direct8 BIT
#ifdef USE_ASM_SMALL
	bra .Op3CM1_Bit8
	nop
#else
	Bit8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op24M0:
	Direct8 BIT
#ifdef USE_ASM_SMALL
	bra .Op3CM0_Bit16
	nop
#else
	Bit16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op34M1:
	DirectIndexedX8 BIT
#ifdef USE_ASM_SMALL
	bra .Op3CM1_Bit8
	nop
#else
	Bit8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op34M0:
	DirectIndexedX8 BIT2
#ifdef USE_ASM_SMALL
	bra .Op3CM0_Bit16
	nop
#else
	Bit16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2CM1:
	Absolute8 BIT
#ifdef USE_ASM_SMALL
	bra .Op3CM1_Bit8
	nop
#else
	Bit8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2CM0:
	Absolute8 BIT2
#ifdef USE_ASM_SMALL
	bra .Op3CM0_Bit16
	nop
#else
	Bit16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3CM1:
	AbsoluteIndexedX8 BIT
#ifdef USE_ASM_SMALL
.Op3CM1_Bit8:
#endif
	Bit8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3CM0:
	AbsoluteIndexedX8 BIT2
#ifdef USE_ASM_SMALL
.Op3CM0_Bit16:
#endif
	Bit16 ABSX

/* CMP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC9M1:
	Immediate8 CMP
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC9M0:
	Immediate16 CMP
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC5M1:
	Direct8_i CMP
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC5M0:
	Direct8_i CMP2
	call_S9xGetWord Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD5M1:
	DirectIndexedX8_i CMP
	call_S9xGetByte DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD5M0:
	DirectIndexedX8_i CMP2
	call_S9xGetWord DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD2M1:
	DirectIndirect8_i CMP
	call_S9xGetByte DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DI
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD2M0:
	DirectIndirect8_i CMP2
	call_S9xGetWord DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DI
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC1M1:
	DirectIndexedIndirect8_i CMP
	call_S9xGetByte DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DII
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC1M0:
	DirectIndexedIndirect8_i CMP2
	call_S9xGetWord DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DII
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD1M1:
	DirectIndirectIndexed8_i CMP
	call_S9xGetByte DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DIIY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD1M0:
	DirectIndirectIndexed8_i CMP2
	call_S9xGetWord DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DIIY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC7M1:
	DirectIndirectLong8_i CMP
	call_S9xGetByte DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC7M0:
	DirectIndirectLong8_i CMP2
	call_S9xGetWord DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD7M1:
	DirectIndirectIndexedLong8_i CMP
	call_S9xGetByte DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 DIIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD7M0:
	DirectIndirectIndexedLong8_i CMP2
	call_S9xGetWord DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 DIIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCDM1:
	Absolute8_i CMP
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCDM0:
	Absolute8_i CMP2
	call_S9xGetWord Absolute8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDDM1:
	AbsoluteIndexedX8_i CMP
	call_S9xGetByte AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 ABSX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDDM0:
	AbsoluteIndexedX8_i CMP2
	call_S9xGetWord AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 ABSX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD9M1:
	AbsoluteIndexedY8_i CMP
	call_S9xGetByte AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 ABSY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD9M0:
	AbsoluteIndexedY8_i CMP2
	call_S9xGetWord AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 ABSY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCFM1:
	AbsoluteLong8_i CMP
	call_S9xGetByte AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 ABSL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCFM0:
	AbsoluteLong8_i CMP2
	call_S9xGetWord AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 ABSL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDFM1:
	AbsoluteLongIndexedX8_i CMP
	call_S9xGetByte AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 ALX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDFM0:
	AbsoluteLongIndexedX8_i CMP2
	call_S9xGetWord AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 ALX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC3M1:
	StackRelative8_i CMP
	call_S9xGetByte StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M1_Cmp8
	nop
#else
	Cmp8 SREL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC3M0:
	StackRelative8_i CMP2
	call_S9xGetWord StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .OpD3M0_Cmp16
	nop
#else
	Cmp16 SREL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD3M1:
	StackRelativeIndirectIndexed8_i CMP
	call_S9xGetByte StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.OpD3M1_Cmp8:
#endif
	Cmp8 SRII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD3M0:
	StackRelativeIndirectIndexed8_i CMP2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.OpD3M0_Cmp16:
#endif
	Cmp16 SRII

/* CPX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE0X1:
	Immediate8 CPX
#ifdef USE_ASM_SMALL
	bra .OpECX1_Cpx8
	nop
#else
	Cpx8 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE0X0:
	Immediate16 CPX
#ifdef USE_ASM_SMALL
	bra .OpECX0_Cpx16
	nop
#else
	Cpx16 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE4X1:
	Direct8_i CPX
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpECX1_Cpx8
	nop
#else
	Cpx8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE4X0:
	Direct8_i CPX2
	call_S9xGetWord Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpECX0_Cpx16
	nop
#else
	Cpx16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpECX1:
	Absolute8_i CPX
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
.OpECX1_Cpx8:
#endif
	Cpx8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpECX0:
	Absolute8_i CPX2
	call_S9xGetWord Absolute8_f
#ifdef USE_ASM_SMALL
.OpECX0_Cpx16:
#endif
	Cpx16 ABS


/* CPY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC0X1:
	Immediate8 CPY
#ifdef USE_ASM_SMALL
	bra .OpCCX1_Cpy8
	nop
#else
	Cpy8 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC0X0:
	Immediate16 CPY
#ifdef USE_ASM_SMALL
	bra .OpCCX0_Cpy16
	nop
#else
	Cpy16 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC4X1:
	Direct8_i CPY
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpCCX1_Cpy8
	nop
#else
	Cpy8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC4X0:
	Direct8_i CPY2
	call_S9xGetWord Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpCCX0_Cpy16
	nop
#else
	Cpy16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCCX1:
	Absolute8_i CPY
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
.OpCCX1_Cpy8:
#endif
	Cpy8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCCX0:
	Absolute8_i CPY2
	call_S9xGetWord Absolute8_f
#ifdef USE_ASM_SMALL
.OpCCX0_Cpy16:
#endif
	Cpy16 ABS

/* DEC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3AM1:
	extu.b A,r0
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	sub r0,A
	add #-1,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	extu.b r0,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	or r0,A
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3AM0:
	add #-1,A
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	extu.w A,A
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	Set16ZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC6M1:
	Direct8 DEC
#ifdef USE_ASM_SMALL
	bra .OpDEM1_Dec8
	nop
#else
	Dec8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC6M0:
	Direct8 DEC2
#ifdef USE_ASM_SMALL
	bra .OpDEM0_Dec16
	nop
#else
	Dec16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD6M1:
	DirectIndexedX8 DEC
#ifdef USE_ASM_SMALL
	bra .OpDEM1_Dec8
	nop
#else
	Dec8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD6M0:
	DirectIndexedX8 DEC2
#ifdef USE_ASM_SMALL
	bra .OpDEM0_Dec16
	nop
#else
	Dec16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCEM1:
	Absolute8 DEC
#ifdef USE_ASM_SMALL
	bra .OpDEM1_Dec8
	nop
#else
	Dec8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCEM0:
	Absolute8 DEC2
#ifdef USE_ASM_SMALL
	bra .OpDEM0_Dec16
	nop
#else
	Dec16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDEM1:
	AbsoluteIndexedX8 DEC

#ifdef USE_ASM_SMALL
.OpDEM1_Dec8:
#endif
	Dec8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDEM0:
	AbsoluteIndexedX8 DEC2
#ifdef USE_ASM_SMALL
.OpDEM0_Dec16:
#endif
	Dec16 ABSX

/* EOR */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op49M1:
	Immediate8 EOR
	Eor8 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op49M0:
	Immediate16 EOR
	Eor16 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op45M1:
	Direct8_i EOR
	call_S9xGetByte Direct8_f
	Eor8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op45M0:
	Direct8_i EOR2
	call_S9xGetWord Direct8_f
	Eor16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op55M1:
	DirectIndexedX8_i EOR
	call_S9xGetByte DirectIndexedX8_f
	Eor8 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op55M0:
	DirectIndexedX8_i EOR2
	call_S9xGetWord DirectIndexedX8_f
	Eor16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op52M1:
	DirectIndirect8_i EOR
	call_S9xGetByte DirectIndirect8_f
	Eor8 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op52M0:
	DirectIndirect8_i EOR2
	call_S9xGetWord DirectIndirect8_f
	Eor16 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op41M1:
	DirectIndexedIndirect8_i EOR
	call_S9xGetByte DirectIndexedIndirect8_f
	Eor8 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op41M0:
	DirectIndexedIndirect8_i EOR2
	call_S9xGetWord DirectIndexedIndirect8_f
	Eor16 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op51M1:
	DirectIndirectIndexed8_i EOR
	call_S9xGetByte DirectIndirectIndexed8_f
	Eor8 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op51M0:
	DirectIndirectIndexed8_i EOR2
	call_S9xGetWord DirectIndirectIndexed8_f
	Eor16 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op47M1:
	DirectIndirectLong8_i EOR
	call_S9xGetByte DirectIndirectLong8_f
	Eor8 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op47M0:
	DirectIndirectLong8_i EOR2
	call_S9xGetWord DirectIndirectLong8_f
	Eor16 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op57M1:
	DirectIndirectIndexedLong8_i EOR
	call_S9xGetByte DirectIndirectIndexedLong8_f
	Eor8 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op57M0:
	DirectIndirectIndexedLong8_i EOR2
	call_S9xGetWord DirectIndirectIndexedLong8_f
	Eor16 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4DM1:
	Absolute8_i EOR
	call_S9xGetByte Absolute8_f
	Eor8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4DM0:
	Absolute8_i EOR2
	call_S9xGetWord Absolute8_f
	Eor16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5DM1:
	AbsoluteIndexedX8_i EOR
	call_S9xGetByte AbsoluteIndexedX8_f
	Eor8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5DM0:
	AbsoluteIndexedX8_i EOR2
	call_S9xGetWord AbsoluteIndexedX8_f
	Eor16 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op59M1:
	AbsoluteIndexedY8_i EOR
	call_S9xGetByte AbsoluteIndexedY8_f
	Eor8 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op59M0:
	AbsoluteIndexedY8_i EOR2
	call_S9xGetWord AbsoluteIndexedY8_f
	Eor16 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4FM1:
	AbsoluteLong8_i EOR
	call_S9xGetByte AbsoluteLong8_f
	Eor8 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4FM0:
	AbsoluteLong8_i EOR2
	call_S9xGetWord AbsoluteLong8_f
	Eor16 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5FM1:
	AbsoluteLongIndexedX8_i EOR
	call_S9xGetByte AbsoluteLongIndexedX8_f
	Eor8 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5FM0:
	AbsoluteLongIndexedX8_i EOR2
	call_S9xGetWord AbsoluteLongIndexedX8_f
	Eor16 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op43M1:
	StackRelative8_i EOR
	call_S9xGetByte StackRelative8_f
	Eor8 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op43M0:
	StackRelative8_i EOR2
	call_S9xGetWord StackRelative8_f
	Eor16 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op53M1:
	StackRelativeIndirectIndexed8_i EOR
	call_S9xGetByte StackRelativeIndirectIndexed8_f
	Eor8 SRII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op53M0:
	StackRelativeIndirectIndexed8_i EOR2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
	Eor16 SRII

/* INC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1AM1:
	extu.b A,r0
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	sub r0,A
	add #1,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	extu.b r0,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	or r0,A
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1AM0:
	add #1,A
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	extu.w A,A
	Set16ZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE6M1:
	Direct8 INC
#ifdef USE_ASM_SMALL
	bra .OpFEM1_Inc8
	nop
#else
	Inc8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE6M0:
	Direct8 INC2
#ifdef USE_ASM_SMALL
	bra .OpFEM0_Inc16
	nop
#else
	Inc16 DIR
#endif
	
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF6M1:
	DirectIndexedX8 INC
#ifdef USE_ASM_SMALL
	bra .OpFEM1_Inc8
	nop
#else
	Inc8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF6M0:
	DirectIndexedX8 INC2
#ifdef USE_ASM_SMALL
	bra .OpFEM0_Inc16
	nop
#else
	Inc16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEEM1:
	Absolute8 INC
#ifdef USE_ASM_SMALL
	bra .OpFEM1_Inc8
	nop
#else
	Inc8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEEM0:
	Absolute8 INC2
#ifdef USE_ASM_SMALL
	bra .OpFEM0_Inc16
	nop
#else
	Inc16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFEM1:
	AbsoluteIndexedX8 INC
#ifdef USE_ASM_SMALL
.OpFEM1_Inc8:
#endif
	Inc8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFEM0:
	AbsoluteIndexedX8 INC2
#ifdef USE_ASM_SMALL
.OpFEM0_Inc16:
#endif
	Inc16 ABSX

/* LDA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA9M1:
	Immediate8 LDA
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA9M0:
	Immediate16 LDA
	Lda16 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA5M1:
	Direct8_i LDA
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA5M0:
	Direct8_i LDA2
	call_S9xGetWord Direct8_f
	Lda16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB5M1:
	DirectIndexedX8_i LDA
	call_S9xGetByte DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB5M0:
	DirectIndexedX8_i LDA2
	call_S9xGetWord DirectIndexedX8_f
	Lda16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB2M1:
	DirectIndirect8_i LDA
	call_S9xGetByte DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DI
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB2M0:
	DirectIndirect8_i LDA2
	call_S9xGetWord DirectIndirect8_f
	Lda16 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA1M1:
	DirectIndexedIndirect8_i LDA
	call_S9xGetByte DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DII
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA1M0:
	DirectIndexedIndirect8_i LDA2
	call_S9xGetWord DirectIndexedIndirect8_f
	Lda16 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB1M1:
	DirectIndirectIndexed8_i LDA
	call_S9xGetByte DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DIIY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB1M0:
	DirectIndirectIndexed8_i LDA2
	call_S9xGetWord DirectIndirectIndexed8_f
	Lda16 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA7M1:
	DirectIndirectLong8_i LDA
	call_S9xGetByte DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA7M0:
	DirectIndirectLong8_i LDA2
	call_S9xGetWord DirectIndirectLong8_f
	Lda16 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB7M1:
	DirectIndirectIndexedLong8_i LDA
	call_S9xGetByte DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 DIIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB7M0:
	DirectIndirectIndexedLong8_i LDA2
	call_S9xGetWord DirectIndirectIndexedLong8_f
	Lda16 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpADM1:
	Absolute8_i LDA
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpADM0:
	Absolute8_i LDA2
	call_S9xGetWord Absolute8_f
	Lda16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBDM1:
	AbsoluteIndexedX8_i LDA
	call_S9xGetByte AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 ABSX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBDM0:
	AbsoluteIndexedX8_i LDA2
	call_S9xGetWord AbsoluteIndexedX8_f
	Lda16 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB9M1:
	AbsoluteIndexedY8_i LDA
	call_S9xGetByte AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 ABSY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB9M0:
	AbsoluteIndexedY8_i LDA2
	call_S9xGetWord AbsoluteIndexedY8_f
	Lda16 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAFM1:
	AbsoluteLong8_i LDA
	call_S9xGetByte AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 ABSL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAFM0:
	AbsoluteLong8_i LDA2
	call_S9xGetWord AbsoluteLong8_f
	Lda16 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBFM1:
	AbsoluteLongIndexedX8_i LDA
	call_S9xGetByte AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 ALX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBFM0:
	AbsoluteLongIndexedX8_i LDA2
	call_S9xGetWord AbsoluteLongIndexedX8_f
	Lda16 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA3M1:
	StackRelative8_i LDA
	call_S9xGetByte StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .OpB3M1_Lda8
	nop
#else
	Lda8 SREL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA3M0:
	StackRelative8_i LDA2
	call_S9xGetWord StackRelative8_f
	Lda16 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB3M1:
	StackRelativeIndirectIndexed8_i LDA
	call_S9xGetByte StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.OpB3M1_Lda8:
#endif
	Lda8 SRII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB3M0:
	StackRelativeIndirectIndexed8_i LDA2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
	Lda16 SRII

/* LDX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA2X1:
	Immediate8 LDX
	Ldx8 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA2X0:
	Immediate16 LDX
	Ldx16 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA6X1:
	Direct8_i LDX
	call_S9xGetByte Direct8_f
	Ldx8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA6X0:
	Direct8_i LDX2
	call_S9xGetWord Direct8_f
	Ldx16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB6X1:
	DirectIndexedY8_i LDX
	call_S9xGetByte DirectIndexedY8_f
	Ldx8 DIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB6X0:
	DirectIndexedY8_i LDX2
	call_S9xGetWord DirectIndexedY8_f
	Ldx16 DIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAEX1:
	Absolute8_i LDX
	call_S9xGetByte Absolute8_f
	Ldx8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAEX0:
	Absolute8_i LDX2
	call_S9xGetWord Absolute8_f
	Ldx16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBEX1:
	AbsoluteIndexedY8_i LDX
	call_S9xGetByte AbsoluteIndexedY8_f
	Ldx8 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBEX0:
	AbsoluteIndexedY8_i LDX2
	call_S9xGetWord AbsoluteIndexedY8_f
	Ldx16 ABSY

/* LDY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA0X1:
	Immediate8 LDY
	Ldy8 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA0X0:
	Immediate16 LDY
	Ldy16 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA4X1:
	Direct8_i LDY
	call_S9xGetByte Direct8_f
	Ldy8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA4X0:
	Direct8_i LDY2
	call_S9xGetWord Direct8_f
	Ldy16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB4X1:
	DirectIndexedX8_i LDY
	call_S9xGetByte DirectIndexedX8_f
	Ldy8 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB4X0:
	DirectIndexedX8_i LDY2
	call_S9xGetWord DirectIndexedX8_f
	Ldy16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpACX1:
	Absolute8_i LDY
	call_S9xGetByte Absolute8_f
	Ldy8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpACX0:
	Absolute8_i LDY2
	call_S9xGetWord Absolute8_f
	Ldy16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBCX1:
	AbsoluteIndexedX8_i LDY
	call_S9xGetByte AbsoluteIndexedX8_f
	Ldy8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBCX0:
	AbsoluteIndexedX8_i LDY2
	call_S9xGetWord AbsoluteIndexedX8_f
	Ldy16 ABSX

/* LSR */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4AM1:
	extu.b A,r1
	sub r1,A
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov #Carry,r0
	shlr r1
	bt/s .Lsr8Carry
	or r0,FLAGS
	add #-Carry,FLAGS
.Lsr8Carry:
	or r1,A
	SetZNjmp r1

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4AM0:
	extu.w A,A
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	shlr A
	mov #Carry,r0
	bt/s .Lsr16Carry
	or r0,FLAGS
	add #-Carry,FLAGS
.Lsr16Carry:
	Set16ZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op46M1:
	Direct8 LSR
#ifdef USE_ASM_SMALL
	bra .Op5EM1_Lsr8
	nop
#else
	Lsr8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op46M0:
	Direct8 LSR2
#ifdef USE_ASM_SMALL
	bra .Op5EM0_Lsr16
	nop
#else
	Lsr16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op56M1:
	DirectIndexedX8 LSR
#ifdef USE_ASM_SMALL
	bra .Op5EM1_Lsr8
	nop
#else
	Lsr8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op56M0:
	DirectIndexedX8 LSR2
#ifdef USE_ASM_SMALL
	bra .Op5EM0_Lsr16
	nop
#else
	Lsr16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4EM1:
	Absolute8 LSR
#ifdef USE_ASM_SMALL
	bra .Op5EM1_Lsr8
	nop
#else
	Lsr8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4EM0:
	Absolute8 LSR2
#ifdef USE_ASM_SMALL
	bra .Op5EM0_Lsr16
	nop
#else
	Lsr16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5EM1:
	AbsoluteIndexedX8 LSR
#ifdef USE_ASM_SMALL
.Op5EM1_Lsr8:
#endif
	Lsr8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5EM0:
	AbsoluteIndexedX8 LSR2
#ifdef USE_ASM_SMALL
.Op5EM0_Lsr16:
#endif
	Lsr16 ABSX

/* ORA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op09M1:
	Immediate8 ORA
	Ora8 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op09M0:
	Immediate16 ORA
	Ora16 IMM

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op05M1:
	Direct8_i ORA
	call_S9xGetByte Direct8_f
	Ora8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op05M0:
	Direct8_i ORA2
	call_S9xGetWord Direct8_f
	Ora16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op15M1:
	DirectIndexedX8_i ORA
	call_S9xGetByte DirectIndexedX8_f
	Ora8 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op15M0:
	DirectIndexedX8_i ORA2
	call_S9xGetWord DirectIndexedX8_f
	Ora16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op12M1:
	DirectIndirect8_i ORA
	call_S9xGetByte DirectIndirect8_f
	Ora8 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op12M0:
	DirectIndirect8_i ORA2
	call_S9xGetWord DirectIndirect8_f
	Ora16 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op01M1:
	DirectIndexedIndirect8_i ORA
	call_S9xGetByte DirectIndexedIndirect8_f
	Ora8 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op01M0:
	DirectIndexedIndirect8_i ORA2
	call_S9xGetWord DirectIndexedIndirect8_f
	Ora16 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op11M1:
	DirectIndirectIndexed8_i ORA
	call_S9xGetByte DirectIndirectIndexed8_f
	Ora8 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op11M0:
	DirectIndirectIndexed8_i ORA2
	call_S9xGetWord DirectIndirectIndexed8_f
	Ora16 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op07M1:
	DirectIndirectLong8_i ORA
	call_S9xGetByte DirectIndirectLong8_f
	Ora8 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op07M0:
	DirectIndirectLong8_i ORA2
	call_S9xGetWord DirectIndirectLong8_f
	Ora16 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op17M1:
	DirectIndirectIndexedLong8_i ORA
	call_S9xGetByte DirectIndirectIndexedLong8_f
	Ora8 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op17M0:
	DirectIndirectIndexedLong8_i ORA2
	call_S9xGetWord DirectIndirectIndexedLong8_f
	Ora16 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0DM1:
	Absolute8_i ORA
	call_S9xGetByte Absolute8_f
	Ora8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0DM0:
	Absolute8_i ORA2
	call_S9xGetWord Absolute8_f
	Ora16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1DM1:
	AbsoluteIndexedX8_i ORA
	call_S9xGetByte AbsoluteIndexedX8_f
	Ora8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1DM0:
	AbsoluteIndexedX8_i ORA2
	call_S9xGetWord AbsoluteIndexedX8_f
	Ora16 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op19M1:
	AbsoluteIndexedY8_i ORA
	call_S9xGetByte AbsoluteIndexedY8_f
	Ora8 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op19M0:
	AbsoluteIndexedY8_i ORA2
	call_S9xGetWord AbsoluteIndexedY8_f
	Ora16 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0FM1:
	AbsoluteLong8_i ORA
	call_S9xGetByte AbsoluteLong8_f
	Ora8 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0FM0:
	AbsoluteLong8_i ORA2
	call_S9xGetWord AbsoluteLong8_f
	Ora16 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1FM1:
	AbsoluteLongIndexedX8_i ORA
	call_S9xGetByte AbsoluteLongIndexedX8_f
	Ora8 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1FM0:
	AbsoluteLongIndexedX8_i ORA2
	call_S9xGetWord AbsoluteLongIndexedX8_f
	Ora16 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op03M1:
	StackRelative8_i ORA
	call_S9xGetByte StackRelative8_f
	Ora8 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op03M0:
	StackRelative8_i ORA2
	call_S9xGetWord StackRelative8_f
	Ora16 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op13M1:
	StackRelativeIndirectIndexed8_i ORA
	call_S9xGetByte StackRelativeIndirectIndexed8_f
	Ora8 SRII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op13M0:
	StackRelativeIndirectIndexed8_i ORA2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
	Ora16 SRII

/* ROL */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2AM1:
	GetCarry
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.b A,r0
	mov #1,r2
	sub r0,A
	or r2,FLAGS
	rotcl r0
	shll8 r2
	tst r2,r0
	extu.b r0,r0
	bf/s .Rol8Carry
	or r0,A
	add #-Carry,FLAGS
.Rol8Carry:
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2AM0:
	GetCarry
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov #1,r2
	rotcl A
	or r2,FLAGS
	shll16 r2
	tst r2,A
	bf/s .Rol16Carry
	extu.w A,A
	add #-Carry,FLAGS
.Rol16Carry:	
	Set16ZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op26M1:
	Direct8 ROL
#ifdef USE_ASM_SMALL
	bra .Op3EM1_Rol8
	nop
#else
	Rol8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op26M0:
	Direct8 ROL2
#ifdef USE_ASM_SMALL
	bra .Op3EM0_Rol16
	nop
#else
	Rol16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op36M1:
	DirectIndexedX8 ROL
#ifdef USE_ASM_SMALL
	bra .Op3EM1_Rol8
	nop
#else
	Rol8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op36M0:
	DirectIndexedX8 ROL2
#ifdef USE_ASM_SMALL
	bra .Op3EM0_Rol16
	nop
#else
	Rol16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2EM1:
	Absolute8 ROL
#ifdef USE_ASM_SMALL
	bra .Op3EM1_Rol8
	nop
#else
	Rol8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2EM0:
	Absolute8 ROL2
#ifdef USE_ASM_SMALL
	bra .Op3EM0_Rol16
	nop
#else
	Rol16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3EM1:
	AbsoluteIndexedX8 ROL
#ifdef USE_ASM_SMALL
.Op3EM1_Rol8:
#endif
	Rol8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3EM0:
	AbsoluteIndexedX8 ROL2
#ifdef USE_ASM_SMALL
.Op3EM0_Rol16:
#endif
	Rol16 ABSX

/* ROR */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6AM1:
	mov #1,r1
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	and FLAGS,r1
	extu.b A,r0
	shll8 r1
	sub r0,A
	or r1,r0
	shlr r0
	mov #Carry,r1
	bt/s .Ror8Carry
	or r1,FLAGS
	add #-Carry,FLAGS
.Ror8Carry:	
	or r0,A
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6AM0:
	mov #1,r1
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	and FLAGS,r1
	extu.w A,A
	shll16 r1
	or r1,A
	shlr A
	mov #Carry,r1
	bt/s .Ror16Carry
	or r1,FLAGS
	add #-Carry,FLAGS
.Ror16Carry:	
	Set16ZNjmp A
	
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op66M1:
	Direct8 ROR
#ifdef USE_ASM_SMALL
	bra .Op7EM1_Ror8
	nop
#else
	Ror8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op66M0:
	Direct8 ROR2
#ifdef USE_ASM_SMALL
	bra .Op7EM0_Ror16
	nop
#else
	Ror16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op76M1:
	DirectIndexedX8 ROR
#ifdef USE_ASM_SMALL
	bra .Op7EM1_Ror8
	nop
#else
	Ror8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op76M0:
	DirectIndexedX8 ROR2
#ifdef USE_ASM_SMALL
	bra .Op7EM0_Ror16
	nop
#else
	Ror16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6EM1:
	Absolute8 ROR
#ifdef USE_ASM_SMALL
	bra .Op7EM1_Ror8
	nop
#else
	Ror8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6EM0:
	Absolute8 ROR2
#ifdef USE_ASM_SMALL
	bra .Op7EM0_Ror16
	nop
#else
	Ror16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7EM1:
	AbsoluteIndexedX8 ROR
#ifdef USE_ASM_SMALL
.Op7EM1_Ror8:
#endif
	Ror8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7EM0:
	AbsoluteIndexedX8 ROR2
#ifdef USE_ASM_SMALL
.Op7EM0_Ror16:
#endif
	Ror16 ABSX

/* SBC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE9M1:
	Immediate8 SBC
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE9M0:
	Immediate16 SBC
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 IMM
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE5M1:
	Direct8_i SBC
	call_S9xGetByte Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE5M0:
	Direct8_i SBC2
	call_S9xGetWord Direct8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF5M1:
	DirectIndexedX8_i SBC
	call_S9xGetByte DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF5M0:
	DirectIndexedX8_i SBC2
	call_S9xGetWord DirectIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DIX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF2M1:
	DirectIndirect8_i SBC
	call_S9xGetByte DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DI
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF2M0:
	DirectIndirect8_i SBC2
	call_S9xGetWord DirectIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DI
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE1M1:
	DirectIndexedIndirect8_i SBC
	call_S9xGetByte DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DII
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE1M0:
	DirectIndexedIndirect8_i SBC2
	call_S9xGetWord DirectIndexedIndirect8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DII
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF1M1:
	DirectIndirectIndexed8_i SBC
	call_S9xGetByte DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DIIY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF1M0:
	DirectIndirectIndexed8_i SBC2
	call_S9xGetWord DirectIndirectIndexed8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DIIY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE7M1:
	DirectIndirectLong8_i SBC
	call_S9xGetByte DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE7M0:
	DirectIndirectLong8_i SBC2
	call_S9xGetWord DirectIndirectLong8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF7M1:
	DirectIndirectIndexedLong8_i SBC
	call_S9xGetByte DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 DIIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF7M0:
	DirectIndirectIndexedLong8_i SBC2
	call_S9xGetWord DirectIndirectIndexedLong8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 DIIL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEDM1:
	Absolute8_i SBC
	call_S9xGetByte Absolute8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEDM0:
	Absolute8_i SBC2
	call_S9xGetWord Absolute8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 ABS
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFDM1:
	AbsoluteIndexedX8_i SBC
	call_S9xGetByte AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 ABSX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFDM0:
	AbsoluteIndexedX8_i SBC2
	call_S9xGetWord AbsoluteIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 ABSX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF9M1:
	AbsoluteIndexedY8_i SBC
	call_S9xGetByte AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 ABSY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF9M0:
	AbsoluteIndexedY8_i SBC2
	call_S9xGetWord AbsoluteIndexedY8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 ABSY
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEFM1:
	AbsoluteLong8_i SBC
	call_S9xGetByte AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 ABSL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEFM0:
	AbsoluteLong8_i SBC2
	call_S9xGetWord AbsoluteLong8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 ABSL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFFM1:
	AbsoluteLongIndexedX8_i SBC
	call_S9xGetByte AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 ALX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFFM0:
	AbsoluteLongIndexedX8_i SBC2
	call_S9xGetWord AbsoluteLongIndexedX8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 ALX
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE3M1:
	StackRelative8_i SBC
	call_S9xGetByte StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M1_Sbc8
	nop
#else
	Sbc8 SREL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE3M0:
	StackRelative8_i SBC2
	call_S9xGetWord StackRelative8_f
#ifdef USE_ASM_SMALL
	bra .OpF3M0_Sbc16
	nop
#else
	Sbc16 SREL
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF3M1:
	StackRelativeIndirectIndexed8_i SBC
	call_S9xGetByte StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.OpF3M1_Sbc8:
#endif
	Sbc8 SRII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF3M0:
	StackRelativeIndirectIndexed8_i SBC2
	call_S9xGetWord StackRelativeIndirectIndexed8_f
#ifdef USE_ASM_SMALL
.OpF3M0_Sbc16:
#endif
	Sbc16 SRII

/* STA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op85M1:
	Direct8 STA
	Sta8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op85M0:
	Direct8 STA2
	Sta16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op95M1:
	DirectIndexedX8 STA
	Sta8 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op95M0:
	DirectIndexedX8 STA2
	Sta16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op92M1:
	DirectIndirect8 STA
	Sta8 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op92M0:
	DirectIndirect8 STA2
	Sta16 DI

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op81M1:
	DirectIndexedIndirect8 STA
	Sta8 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op81M0:
	DirectIndexedIndirect8 STA2
	Sta16 DII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op91M1:
	DirectIndirectIndexed8 STA
	Sta8 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op91M0:
	DirectIndirectIndexed8 STA2
	Sta16 DIIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op87M1:
	DirectIndirectLong8 STA
	Sta8 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op87M0:
	DirectIndirectLong8 STA2
	Sta16 DIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op97M1:
	DirectIndirectIndexedLong8 STA
	Sta8 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op97M0:
	DirectIndirectIndexedLong8 STA2
	Sta16 DIIL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8DM1:
	Absolute8 STA
	Sta8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8DM0:
	Absolute8 STA
	Sta16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9DM1:
	AbsoluteIndexedX8 STA
	Sta8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9DM0:
	AbsoluteIndexedX8 STA2
	Sta16 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op99M1:
	AbsoluteIndexedY8 STA
	Sta8 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op99M0:
	AbsoluteIndexedY8 STA2
	Sta16 ABSY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8FM1:
	AbsoluteLong8 STA
	Sta8 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8FM0:
	AbsoluteLong8 STA2
	Sta16 ABSL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9FM1:
	AbsoluteLongIndexedX8 STA
	Sta8 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9FM0:
	AbsoluteLongIndexedX8 STA2
	Sta16 ALX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op83M1:
	StackRelative8 STA
	Sta8 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op83M0:
	StackRelative8 STA2
	Sta16 SREL

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op93M1:
	StackRelativeIndirectIndexed8 STA
	Sta8 SRII

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op93M0:
	StackRelativeIndirectIndexed8 STA2
	Sta16 SRII

/* STX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op86X1:
	Direct8 STX
	Stx8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op86X0:
	Direct8 STX2
	Stx16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op96X1:
	DirectIndexedY8 STX
	Stx8 DIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op96X0:
	DirectIndexedY8 STX2
	Stx16 DIY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8EX1:
	Absolute8 STX
	Stx8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8EX0:
	Absolute8 STX2
	Stx16 ABS

/* STY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op84X1:
	Direct8 STY
	Sty8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op84X0:
	Direct8 STY2
	Sty16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op94X1:
	DirectIndexedX8 STY
	Sty8 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op94X0:
	DirectIndexedX8 STY2
	Sty16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8CX1:
	Absolute8 STY
	Sty8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8CX0:
	Absolute8 STY2
	Sty16 ABS

/* STZ */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op64M1:
	Direct8 STZ
	Stz8 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op64M0:
	Direct8 STZ2
	Stz16 DIR

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op74M1:
	DirectIndexedX8 STZ
	Stz8 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op74M0:
	DirectIndexedX8 STZ2
	Stz16 DIX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9CM1:
	Absolute8 STZ
	Stz8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9CM0:
	Absolute8 STZ2
	Stz16 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9EM1:
	AbsoluteIndexedX8 STZ
	Stz8 ABSX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9EM0:
	AbsoluteIndexedX8 STZ2
	Stz16 ABSX

/* TRB */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op14M1:
	Direct8 TRB
#ifdef USE_ASM_SMALL
	bra .Op1CM1_Trb8
	nop
#else
	Trb8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op14M0:
	Direct8 TRB2
#ifdef USE_ASM_SMALL
	bra .Op1CM0_Trb16
	nop
#else
	Trb16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1CM1:
	Absolute8 TRB
#ifdef USE_ASM_SMALL
.Op1CM1_Trb8:
#endif
	Trb8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1CM0:
	Absolute8 TRB2
#ifdef USE_ASM_SMALL
.Op1CM0_Trb16:
#endif
	Trb16 ABS

/* TSB */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op04M1:
	Direct8 TSB
#ifdef USE_ASM_SMALL
	bra .Op04M1_Tsb8
	nop
#else
	Tsb8 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op04M0:
	Direct8 TSB2
#ifdef USE_ASM_SMALL
	bra .Op0CM0_Tsb16
	nop
#else
	Tsb16 DIR
#endif

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0CM1:
	Absolute8 TSB
#ifdef USE_ASM_SMALL
.Op04M1_Tsb8:
#endif
	Tsb8 ABS

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0CM0:
	Absolute8 TSB2
#ifdef USE_ASM_SMALL
.Op0CM0_Tsb16:
#endif
	Tsb16 ABS

/* BCC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op90:
	Relative
	BranchCheck0 BCC
	GetNotCarry
	bf .BCC_EXIT
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BCC
#endif
.BCC_EXIT:
	jmp @r13
	nop

/* BCS */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB0:
	Relative
	BranchCheck0 BCS
	GetNotCarry
	bt .BCS_EXIT
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BCS
#endif
.BCS_EXIT:
	jmp @r13
	nop

/* BEQ */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF0:
	Relative
	BranchCheck2 BEQ
	extu.w r7,r3
	tst r3,r3
	bf .BEQ_EXIT
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BEQ
#endif
.BEQ_EXIT:
	jmp @r13
	nop

/* BMI */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op30:
	Relative
	BranchCheck1 BMI
	cmp/pz r7
	bt .BMI_EXIT
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BMI
#endif
.BMI_EXIT:
	jmp @r13
	nop

/* BNE */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD0:
	Relative
	BranchCheck1 BNE
	extu.w r7,r3
	tst r3,r3
	bt .BNE_EXIT
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BNE
#endif
.BNE_EXIT:
	jmp @r13
	nop

/* BPL */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op10:
	Relative
	BranchCheck1 BPL
	cmp/pz r7
	bf .BPL_EXIT
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BPL
#endif
.BPL_EXIT:
	jmp @r13
	nop

/* BRA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op80:
	Relative
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	extu.w r4,PC
	mov.l PCBase,r3
	jmp @r13
	add r3,PC

/* BVC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op50:
	Relative
	BranchCheck0 BVC
	mov #Overflow,r1
	tst r1,FLAGS
	bf .BVC_EXIT:
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
	bra .Op70_CPUShutdown
	nop
#else
	CPUShutdown BVC
#endif
.BVC_EXIT:
	jmp @r13
	nop

/* BVS */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op70:
	Relative
	BranchCheck0 BVS
	mov #Overflow,r1
	tst r1,FLAGS
	bt .BVS_EXIT:
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	add #1,CYCLES
#endif
	mov.l PCBase,r3
	extu.w r4,PC
	add r3,PC
#ifdef USE_ASM_SMALL
.Op70_CPUShutdown:
#endif
	CPUShutdown BVS
.BVS_EXIT:
	jmp @r13
	nop
	
/* BRL */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op82:
	RelativeLong BRL
	extu.w r4,r4
	mov.l ICPU,r0
	mov.l @(16,r0),r2
	mov.l S9xSetPCBase,r3
	or r2,r4
	Xlink

/* CLC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op18:
	mov #~Carry,r0
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	jmp @r13
	and r0,FLAGS

/* CLD */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD8:
	mov #~Decimal,r0
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	jmp @r13
	and r0,FLAGS

/* CLI */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op58:
	mov #~IRQ,r2
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b IRQActive,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bt .CLI_EXIT
	/* XXX: test for Settings.DisableIRQ */
	mov.l S9xOpcode_IRQ,r3
	and r2,FLAGS
	Xlink
.CLI_EXIT:
	jmp @r13
	and r2,FLAGS

/* CLV */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpB8:
	mov #~Overflow,r0
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	jmp @r13
	and r0,FLAGS

/* DEX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCAX1:
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	mov.b XL,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #-1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.b r0,XL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCAX0:
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	mov.w XX,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #-1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.w r0,XX
	Set16ZNjmp r0

/* DEY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op88X1:
#ifdef VAR_CYCLES
	add #6,CYCLES
#endif
	mov.b YL,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #-1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.b r0,YL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op88X0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w YY,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #-1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.w r0,YY
	Set16ZNjmp r0

/* INX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE8X1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b XL,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.b r0, XL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE8X0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w XX,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.w r0, XX
	Set16ZNjmp r0

/* INY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC8X1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b YL,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.b r0, YL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC8X0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w YY,r0
#ifdef CPU_SHUTDOWN
	mov #0,r3
#endif
	add #1,r0
#ifdef CPU_SHUTDOWN
	mov.l r3,WaitAddress
#endif
	mov.w r0,YY
	Set16ZNjmp r0

/* NOP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEA:
	jmp @r13
#ifdef VAR_CYCLES
	add #6,CYCLES
#else
	nop
#endif

/* PEA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF4:
	Immediate16 PEA
	PushWordjmp PEA

/* PEI */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpD4:
	DirectIndirect8 PEI
	extu.w r4,r0
	PushWordjmp PEI

/* PER */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op62:
	RelativeLong PER
	extu.w r4,r0
	PushWordjmp PER

/* PHA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op48M1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.b A,r0
	PushBytejmp PHA

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op48M0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.w A,r0
	PushWordjmp PHA

/* PHB */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
#ifndef USE_REGISTER_AS_DB
	mov.b DB, r0
#else
	extu.b DB, r0
#endif
	PushBytejmp PHB

/* PHD */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op0B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w DD, r0
	PushWordjmp PHD

/* PHK */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
#ifndef USE_REGISTER_AS_PB
	mov.b PB, r0
#else
	extu.b PB, r0
#endif
	PushBytejmp PHK

/* PHP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op08:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	S9xPackStatus PHP
	extu.b FLAGS, r0
	PushBytejmp PHP

/* PHX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDAX1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b XL, r0
	PushBytejmp PHX

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDAX0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w XX, r0
	PushWordjmp PHX

/* PHY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5AX1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b YL, r0
	PushBytejmp PHY

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5AX0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w YY, r0
	PushWordjmp PHY

/* PLA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op68M1:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullByte PLA
	extu.b A,r1
	add r0,A
	sub r1,A
	SetZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op68M0:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullWord PLA
	extu.w r0,A
	Set16ZNjmp A

/* PLB */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAB:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullByte PLB
#ifndef USE_REGISTER_AS_DB
	mov.b r0, DB
#else
	mov r0, DB
#endif
	extu.b r0,r1
	mov.l ICPU,r2
	shll16 r1
	mov.l r1,@(20,r2)
	SetZNjmp r0

/* PLD */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op2B:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullWord PLD
	mov.w r0 DD
	Set16ZNjmp r0
	
/* PLP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op28:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullByte PLP
	extu.b FLAGS,r1
	add r0,FLAGS
	mov #IndexFlag,r2
	sub r1,FLAGS
	tst r2,r0
	bt .PLP16
	mov #0,r0
	mov.b r0,XH
	mov.b r0,YH
.PLP16:
#ifdef USE_ASM_SMALL
	bra .RTI16
	nop
#else
	S9xUnpackStatus PLP
	S9xFixCycles PLP
	CheckForIrqjmp PLP /* Removed in 1.39. See asmops.h */
#endif

/* PLX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFAX1:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullByte PLX
	mov.b r0, XL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFAX0:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullWord PLX
	mov.w r0, XX
	Set16ZNjmp r0

/* PLY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7AX1:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullByte PLY
	mov.b r0, YL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7AX0:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullWord PLY
	mov.w r0, YY
	Set16ZNjmp r0

/* SEC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op38:
	mov #Carry,r0
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	jmp @r13
	or r0,FLAGS

/* SED */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpF8:
	
	mov #Decimal,r0
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	jmp @r13
	or r0,FLAGS

/* SEI */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op78:
	mov #IRQ,r0
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	jmp @r13
	or r0,FLAGS

/* TAX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAAX1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.b A,r0
	mov.b r0, XL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpAAX0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.w A,r0
	mov.w r0, XX
	Set16ZNjmp r0

/* TAY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA8X1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.b A,r0
	mov.b r0, YL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpA8X0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.w A,r0
	mov.w r0, YY
	Set16ZNjmp r0

/* TCD */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.w A,r0
	mov.w r0, DD
	Set16ZNjmp r0

/* TCS */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op1B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	extu.w A,r0
	mov #Emulation/4,r1
	mov.w r0, SS
	shll2 r1
	tst r1,FLAGS16
	bt .TCS_EXIT
	mov #1,r0
	mov.b r0,SH
.TCS_EXIT:
	jmp @r13
	nop

/* TDC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w DD, r0
	extu.w r0,A
	Set16ZNjmp A

/* TSC */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op3B:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w SS, r0
	extu.w r0,A
	Set16ZNjmp A

/* TSX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBAX1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b SL, r0
	mov.b r0, XL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBAX0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w SS, r0
	mov.w r0, XX
	Set16ZNjmp r0

/* TXA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8AM1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b XL, r0
	extu.b A,r1
	extu.b r0,r0
	sub r1,A
	add r0,A
	SetZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op8AM0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w XX, r0
	extu.w r0,A
	Set16ZNjmp A

/* TXS */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9A:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w XX, r0
	mov #Emulation/4,r1
	mov.w r0, SS
	shll2 r1
	tst r1,FLAGS16
	bt .TXS_EXIT
	mov #1,r0
	mov.b r0,SH
.TXS_EXIT:
	jmp @r13
	nop

/* TXY */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9BX1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b XL, r0
	mov.b r0, YL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op9BX0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w XX, r0
	mov.w r0, YY
	Set16ZNjmp r0

/* TYA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op98M1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b YL, r0
	extu.b A,r1
	extu.b r0,r0
	sub r1,A
	add r0,A
	SetZNjmp A

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op98M0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w YY, r0
	extu.w r0,A
	Set16ZNjmp A

/* TYX */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBBX1:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.b YL, r0
	mov.b r0, XL
	SetZNjmp r0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpBBX0:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.w YY, r0
	mov.w r0, XX
	Set16ZNjmp r0

/* XCE */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFB:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov FLAGS16, r0
	mov.w .notemcymask,r1
	and r1,FLAGS16
	tst #Carry,r0
	mov.w .emflag0,r1
	bt .XCE_NO_CARRY
	or r1,FLAGS16
.XCE_NO_CARRY:
	tst r1,r0
	bt/s .XCE_NO_EMU
	tst r1,FLAGS
	add #Carry,FLAGS
.XCE_NO_EMU:
	bt/s .XCE_NO_EMULATION2
	mov #IndexFlag,r1
	mov #MemoryFlag|IndexFlag,r0
	or r0,FLAGS
	mov #1,r0
	mov.b r0,SH
.XCE_NO_EMULATION2:
	tst r1,FLAGS
	bt .XCE_NO_INDEX
	mov #0,r0
	mov.b r0, XH
	mov.b r0, YH
.XCE_NO_INDEX:
#ifdef USE_ASM_SMALL
	bra .SEP16
	nop
#else
	S9xFixCycles XCE
	jmp @r13
	nop
#endif

	.align 2
.emflag0:
	.word	Emulation
	.align 2

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op00:
	mov #1,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0,BRKTriggered
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.w .emflag,r0
	tst r0,FLAGS16
	bf .BRK_EMULATION
#ifndef USE_REGISTER_AS_PB
	mov.b PB, r0
#else
	extu.b PB, r0
#endif
	PushByte BRK1
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #1,r0
	PushWord BRK2
	
	S9xPackStatus BRK
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte BRK3
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #12, CYCLES
#else
	add #8, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-0x1a,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	extu.w r0,r4
	Xlink
.BRK_EMULATION:
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #1,r0
	PushWord BRK2
	
	S9xPackStatus BRK2
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte BRK3
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #6, CYCLES
#else
	add #6, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-2,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	extu.w r0,r4
	Xlink

	.align 2
.notemcymask:
	.word	~(Emulation|Carry)
.emflag:
	.word	Emulation

	.align 5
.globl S9xOpcode_IRQ_r
S9xOpcode_IRQ_r:
	mov.w .emflag1,r0
	sts.l pr,@-r15
	tst r0,FLAGS16
	bf .IRQ_EMULATION
#ifndef USE_REGISTER_AS_PB
	mov.b PB, r0
#else
	extu.b PB, r0
#endif
	PushByte IRQ1
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	PushWord IRQ2
	
	S9xPackStatus IRQ
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte IRQ3
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #12, CYCLES
#else
	add #8, CYCLES
#endif
	mov.l Settings,r3
	add #82,r3	! Settings.SA1
	mov.b @r3,r0
	tst r0,r0
	bt .noirqsa1vector
	mov.l .FillRAM2,r3
	mov.w .n2209,r0
	add r0,r3
	mov.b @r3,r0
	tst #0x40,r0
	bt .noirqsa1vector
	add #5,r3
	lds.l @r15+,pr
	mov.w @r3,r4
	mov.l S9xSetPCBase,r3
	jmp @r3
	extu.w r4,r4
.noirqsa1vector:
	mov.l S9xGetWord,r3
	mov #-0x12,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	lds.l @r15+,pr
	jmp @r3
	extu.w r0,r4
.IRQ_EMULATION:
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	PushWord IRQ4
	
	S9xPackStatus IRQ2
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte IRQ5
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #6, CYCLES
#else
	add #6, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-0x2,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	lds.l @r15+,pr
	jmp @r3
	extu.w r0,r4

	.align 2
.emflag1:
	.word	Emulation

	.align 2
.FillRAM2:
	.long FillRAM
.n2209:
	.word 0x2209


	.align 5
.globl S9xOpcode_NMI_r

S9xOpcode_NMI_r:
	mov.w .emflag2,r0
	sts.l pr,@-r15
	tst r0,FLAGS16
	bf .NMI_EMULATION
#ifndef USE_REGISTER_AS_PB
	mov.b PB, r0
#else
	extu.b PB, r0
#endif
	PushByte NMI1
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	PushWord NMI2

	S9xPackStatus NMI
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte NMI3
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #12, CYCLES
#else
	add #8, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-0x16,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	lds.l @r15+,pr
	jmp @r3
	extu.w r0,r4
.NMI_EMULATION:
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	PushWord NMI4
	
	S9xPackStatus NMI2
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte NMI5
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #6, CYCLES
#else
	add #6, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-0x6,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	lds.l @r15+,pr
	jmp @r3
	extu.w r0,r4

	.align 2
.emflag2:
	.word	Emulation

	.align 2
	
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op02:
	mov.w .emflag25,r0
	tst r0,FLAGS16
	bf .COP_EMULATION
#ifndef USE_REGISTER_AS_PB
	mov.b PB, r0
#else
	extu.b PB, r0
#endif
	PushByte COP1
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #1,r0
	PushWord COP2
	
	S9xPackStatus COP
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte COP3
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #12, CYCLES
#else
	add #8, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-0x1c,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	extu.w r0,r4
	Xlink
.COP_EMULATION:
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #1,r0
	PushWord COP4
	
	S9xPackStatus COP2
	extu.b FLAGS, r0
	mov #Decimal|IRQ,r1
	or r1,FLAGS
	add #-Decimal, FLAGS
	
	PushByte COP5
	mov.l ICPU,r3
	mov #0,r0
	mov.l r0,@(16,r3)
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #6, CYCLES
#else
	add #6, CYCLES
#endif
	mov.l S9xGetWord,r3
	mov #-0xc,r4
	jsr @r3
	extu.w r4,r4
	mov.l S9xSetPCBase,r3
	extu.w r0,r4
	Xlink

	.align 2
.emflag25:
	.word	Emulation

	
	.align 2
/* JML */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDC:
	AbsoluteIndirectLong8 JML
	mov.l ICPU,r3
	mov.l r0,@(16,r3)
	shlr16 r0
	mov.l S9xSetPCBase,r3
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	Xlink

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op5C:
	AbsoluteLong8 JML
	mov.l ICPU,r3
	mov.l r0,@(16,r3)
	shlr16 r0
	mov.l S9xSetPCBase,r3
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
	Xlink

/* JMP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op4C:
	Absolute8 JMP
	mov.l ICPU,r3
	extu.w r4,r4
	mov.l @(16,r3),r0
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6C:
	AbsoluteIndirect8 JMP
	mov.l ICPU,r3
	extu.w r4,r4
	mov.l @(16,r3),r0
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op7C:
	AbsoluteIndexedIndirect8 JMP
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.l ICPU,r3
	extu.w r4,r4
	mov.l @(16,r3),r0
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

/* JSL */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op22:
#ifndef USE_REGISTER_AS_PB
	mov.b PB, r0
#else
	extu.b PB, r0
#endif
	PushByte JSL_ABSL
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #2,r0
	PushWord JSL_ABSL
	AbsoluteLong8 JSL
	mov.l ICPU,r3
	mov.l r0,@(16,r3)
	shlr16 r0
	mov.l S9xSetPCBase,r3
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
	Xlink

/* RTL */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op6B:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullWord RTL
	mov.l r0,@-r15
	PullByte RTL
	mov.l @r15+,r4
#ifndef USE_REGISTER_AS_PB
	mov.b r0, PB
#else
	mov r0, PB
#endif
	add #1,r4
	shll16 r0
	extu.w r4,r4
	mov.l ICPU,r3
	mov.l r0,@(16,r3)
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

/* JSR ABS */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op20:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #1,r0
	PushWord JSR_ABS
	Absolute8 JSR_ABS
	mov.l ICPU,r3
	extu.w r4,r4
	mov.l @(16,r3),r0
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

/* JSR ABS INDEXED INDIRECT */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpFC:
#ifdef VAR_CYCLES
	add #6, CYCLES
#endif
	mov.l PCBase,r0
	neg r0,r0
	add PC, r0
	add #1,r0
	PushWord JSR_AII
	AbsoluteIndexedIndirect8 JSR
	mov.l ICPU,r3
	extu.w r4,r4
	mov.l @(16,r3),r0
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

/* RTS */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op60:
#ifdef VAR_CYCLES
	add #6 * 3, CYCLES
#endif
	PullWord RTS
	add #1,r0
	mov.l ICPU,r3
	extu.w r0,r4
	mov.l @(16,r3),r0
	mov.l S9xSetPCBase,r3
	or r0,r4
	Xlink

/* MVN */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op54X1:
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
	add #12,CYCLES
#else
	add #28,CYCLES
#endif
#endif
	mov.b @PC+,r0
#ifndef USE_REGISTER_AS_DB
	mov.b r0,DB
#else
	mov r0, DB
#endif
	extu.b r0,r0
	mov.l ICPU,r1
	shll16 r0
	mov.l r0,ShiftedDB
	mov.l r0,@-r15
	mov.b @PC+,r4
	mov.l S9xGetByte,r3
	extu.b r4,r4
	mov.w XX,r0
	shll16 r4
	extu.w r0,r0
	jsr @r3
	add r0,r4
#ifdef USE_ASM_R5
	mov r0,r5
	mov.w YY,r0
	mov.l S9xSetByte,r3
	mov.l @r15+,r4
	extu.w r0,r1
	mov r5,r0
#else
	mov r0,r3
	mov.w YY,r0
	extu.w r0,r1
	mov r3,r0
	mov.l S9xSetByte,r3
	mov.l @r15+,r4
#endif
	jsr @r3
	add r1,r4
	mov.b XL,r0
	add #1,r0
	mov.b r0,XL
	mov.b YL,r0
	add #1,r0
	mov.b r0,YL
	add #-1,A
	cmp/pz A
	bf/s .MVN_EXIT8
	extu.w A,A
	jmp @r13
	add #-3,PC
.MVN_EXIT8:
	jmp @r13
	nop

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op54X0:
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
	add #12,CYCLES
#else
	add #28,CYCLES
#endif
#endif
	mov.b @PC+,r0
#ifndef USE_REGISTER_AS_DB
	mov.b r0,DB
#else
	mov r0, DB
#endif
	extu.b r0,r0
	mov.l ICPU,r1
	shll16 r0
	mov.l r0,ShiftedDB
	mov.l r0,@-r15
	mov.b @PC+,r4
	mov.l S9xGetByte,r3
	extu.b r4,r4
	mov.w XX,r0
	shll16 r4
	extu.w r0,r1
	add #1,r0
	add r1,r4
	jsr @r3
	mov.w r0,XX
#ifdef USE_ASM_R5
	mov r0,r5
	mov.w YY,r0
	mov.l S9xSetByte,r3
	mov.l @r15+,r4
	extu.w r0,r1
	add #1,r0
	add r1,r4
	mov.w r0,YY
	jsr @r3
	mov r5,r0
#else
	mov r0,r1
	mov.w YY,r0
	mov.l @r15+,r4
	extu.w r0,r3
	add #1,r0
	add r3,r4
	mov.l S9xSetByte,r3
	mov.w r0,YY
	jsr @r3
	mov r1,r0
#endif
	add #-1,A
	cmp/pz A
	bf/s .MVN_EXIT16
	extu.w A,A
	jmp @r13
	add #-3,PC
.MVN_EXIT16:
	jmp @r13
	nop

/* MVP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op44X1:
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
	add #12,CYCLES
#else
	add #28,CYCLES
#endif
#endif
	mov.b @PC+,r0
#ifndef USE_REGISTER_AS_DB
	mov.b r0,DB
#else
	mov r0, DB
#endif
	extu.b r0,r0
	mov.l ICPU,r1
	shll16 r0
	mov.l r0,ShiftedDB
	mov.l r0,@-r15
	mov.b @PC+,r4
	mov.l S9xGetByte,r3
	extu.b r4,r4
	mov.w XX,r0
	shll16 r4
	extu.w r0,r0
	jsr @r3
	add r0,r4
#ifdef USE_ASM_R5
	mov r0,r5
	mov.w YY,r0
	mov.l S9xSetByte,r3
	mov.l @r15+,r4
	extu.w r0,r1
	mov r5,r0
	jsr @r3
	add r1,r4
#else
	mov r0,r3
	mov.w YY,r0
	mov.l @r15+,r4
	extu.w r0,r1
	mov r3,r0
	mov.l S9xSetByte,r3
	jsr @r3
	add r1,r4
#endif
	mov.b XL,r0
	add #-1,r0
	mov.b r0,XL
	mov.b YL,r0
	add #-1,r0
	mov.b r0,YL
	add #-1,A
	cmp/pz A
	bf/s .MVP_EXIT8
	extu.w A,A
	jmp @r13
	add #-3,PC
.MVP_EXIT8:
	jmp @r13
	nop

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op44X0:
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeedx2,r2
	add r2,CYCLES
	add #12,CYCLES
#else
	add #28,CYCLES
#endif
#endif
	mov.b @PC+,r0
#ifndef USE_REGISTER_AS_DB
	mov.b r0,DB
#else
	mov r0, DB
#endif
	extu.b r0,r0
	mov.l ICPU,r1
	shll16 r0
	mov.l r0,ShiftedDB
	mov.l r0,@-r15
	mov.b @PC+,r4
	mov.l S9xGetByte,r3
	extu.b r4,r4
	mov.w XX,r0
	shll16 r4
	extu.w r0,r1
	add #-1,r0
	add r1,r4
	jsr @r3
	mov.w r0,XX
#ifdef USE_ASM_R5
	mov r0,r5
	mov.w YY,r0
	mov.l S9xSetByte,r3
	mov.l @r15+,r4
	extu.w r0,r1
	add #-1,r0
	add r1,r4
	mov.w r0,YY
	jsr @r3
	mov r5,r0
#else
	mov r0,r3
	mov.w YY,r0
	mov.l @r15+,r4
	extu.w r0,r1
	add #-1,r0
	mov.w r0,YY
	mov r3,r0
	mov.l S9xSetByte,r3
	jsr @r3
	add r1,r4
#endif
	add #-1,A
	cmp/pz A
	bf/s .MVP_EXIT16
	extu.w A,A
	jmp @r13
	add #-3,PC
.MVP_EXIT16:
	jmp @r13
	nop

/* REP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpC2:
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #6,CYCLES
#else
	add #14,CYCLES
#endif
#endif
	mov.b @PC+,r0
	mov #Zero,r1
	extu.b r0,r0
	mov #-1,r2
	and r0,r1
	not r0,r0
	or r1,r7
	and r0,FLAGS
	extu.w r2,r2
	exts.b r0,r1
	mov.w .emflag3,r0
	or r2,r1
	tst r0,FLAGS16
	bt/s .REP_NO_EMU
	and r1,r7
	mov #(MemoryFlag | IndexFlag), r0
	or r0, FLAGS
	
.REP_NO_EMU:
	mov #IndexFlag,r0
	tst r0,FLAGS
	bt .REP16
	mov #0,r0
	mov.b r0,XH
	mov.b r0,YH
.REP16:
#ifdef USE_ASM_SMALL
	bra .Op40_FixCycles
	nop
#else
	S9xFixCycles REP
	CheckForIrqjmp REP /* Removed in 1.39. See asmops.h */
#endif

/* SEP */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpE2:
#ifdef VAR_CYCLES
#ifdef USE_MEMORY_SPEED
	mov.l MemSpeed,r2
	add r2,CYCLES
	add #6,CYCLES
#else
	add #14,CYCLES
#endif
#endif
	mov.b @PC+,r0
	mov #Zero,r1
	extu.b r0,r0
	and r0,r1
	exts.b r0,r2
	shll8 r1
	or r0,FLAGS
	shlr2 r1
	extu.w r2,r0
	exts.b r1,r1
	sub r0,r2
	shlr16 r1
	or r2,r7
	mov.w .emflag3,r0
	not r1,r1
	tst r0,FLAGS16
	bt/s .SEP_NO_EMU
	and r1,r7

	mov #(MemoryFlag | IndexFlag), r0
	or r0, FLAGS
	
.SEP_NO_EMU:
	mov #IndexFlag,r0
	tst r0,FLAGS
	bt .SEP16
	mov #0,r0
	mov.b r0,XH
	mov.b r0,YH
.SEP16:
	S9xFixCycles SEP
	jmp @r13
	nop


/* XBA */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpEB:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	swap.b A,A
	SetZNjmp A

/* RTI */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op40:
#ifdef VAR_CYCLES
	add #12, CYCLES
#endif
	PullByte RTI
	extu.b FLAGS,r1
	add r0,FLAGS
	sub r1,FLAGS
	PullWord RTI
	mov.w .emflag3,r1
	mov.l r0,@-r15
	tst r1, FLAGS16
	bf .RTI_EMU
	PullByte RTI2
	bra .RTI_SKIP_EMU
	extu.b r0,r0
.RTI_EMU:
	mov #(MemoryFlag | IndexFlag), r0
	mov.l ICPU,r3
	or r0,FLAGS
	/*XXX: HERE xorl %eax, %eax*/
	mov.l @(16,r3),r0
	shlr16 r0
.RTI_SKIP_EMU:
#ifndef USE_REGISTER_AS_PB
	mov.b r0,PB
#else
	mov r0,PB
#endif
	mov.l @r15+,r4
	shll16 r0
	mov.l ICPU,r3
	mov.l r0,@(16,r3)

	mov.l S9xSetPCBase,r3
	jsr @r3
	or r0,r4
	mov #IndexFlag,r0
	tst r0,FLAGS
	bt .RTI16
	mov #0,r0
	mov.b r0,XH
	mov.b r0,YH
.RTI16:
	S9xUnpackStatus RTI
#ifdef USE_ASM_SMALL
.Op40_FixCycles:
#endif
	S9xFixCycles RTI
#ifdef USE_ASM_SMALL
.Op40_CheckForIrqjmp:
#endif
	CheckForIrqjmp RTI /* Removed in 1.39. See asmops.h */

	.align 2
.emflag3:
	.word	Emulation

	.align 2
/* WAI */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpCB:
#ifdef USE_NO_WAIT_WHEN_IRQ
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b IRQActive,r0
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	bf .OpCB_normal
	jmp @r13
#ifdef VAR_CYCLES
	add #12,CYCLES
#else
	add #2,CYCLES
#endif
#endif

.OpCB_normal:
	mov #1,r0
	add #-1,PC
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.b r0, WaitingForInterrupt
#ifndef USE_ASM_R12
	add #-16,r11
#endif

	mov.l Settings,r1
	mov.b Shutdown,r0
	tst r0,r0
	bt .NoShutdown

#ifdef ARMAPU
	mov #0,r2
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l NextEvent, CYCLES
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	jmp @r13
	mov.l r2,WaitAddress
#else
	mov.l IAPU,r1
	mov #0,r2
	mov.b APUExecuting,r0
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l NextEvent, CYCLES
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	tst r0,r0
	mov.l r2,WaitAddress
	bt .EndWAIShutdown

	mov.l ICPU,r1
	mov #0,r0
	mov.b r0, CPUExecuting
.WAITExecAPU:
#ifdef SPC700_C
	mov.l IAPU,r1
	SAVE_CYCLES
	mov.l APUPC,r0

	PUSH_REGISTERS
	mov.b @r0,r0
#else
	mov.b @APUPC,r0
#endif
	mov.l .WAIAPUCycles,r1
	extu.b r0,r0
	mov.l .WAIApuOpcodes,r2
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
	LOAD_CYCLES
#endif
	mov.l APU,r1
#ifndef USE_ASM_R12
	add #16,r11
#endif
	mov.l NextEvent,r2
#ifndef USE_ASM_R12
	add #-16,r11
#endif
	mov.l APUCycles,r3
	cmp/ge r2,r3
	bf .WAITExecAPU
	mov.l ICPU,r1
	mov #1,r0
	jmp @r13
	mov.b r0, CPUExecuting

.EndWAIShutdown:
	jmp @r13
	nop

	.align 2
.WAIAPUCycles:	
	.long _S9xAPUCycles
.WAIApuOpcodes:
	.long _S9xApuOpcodes
#endif /* ARMAPU */

.NoShutdown:
	jmp @r13
#ifdef VAR_CYCLES
	add #12, CYCLES
#else
	add #2, CYCLES
#endif
	.align 2

/* ??? */
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
OpDB:
	mov.l Flags,r0
	add #-1,PC
	or #DEBUG_MODE_FLAG, r0
	jmp @r13
	mov.l r0,Flags

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#endif
Op42:
	jmp @r13
	nop

.globl _S9xOpcodesM1X1
.globl _S9xOpcodesE1

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#else
	.align 2
#endif
_S9xOpcodesM1X1:
_S9xOpcodesE1:
	.long Op00
	.long Op01M1
	.long Op02
	.long Op03M1
	.long Op04M1
	.long Op05M1
	.long Op06M1
	.long Op07M1
	.long Op08
	.long Op09M1
	.long Op0AM1
	.long Op0B
	.long Op0CM1
	.long Op0DM1
	.long Op0EM1
	.long Op0FM1
	.long Op10
	.long Op11M1
	.long Op12M1
	.long Op13M1
	.long Op14M1
	.long Op15M1
	.long Op16M1
	.long Op17M1
	.long Op18
	.long Op19M1
	.long Op1AM1
	.long Op1B
	.long Op1CM1
	.long Op1DM1
	.long Op1EM1
	.long Op1FM1
	.long Op20
	.long Op21M1
	.long Op22
	.long Op23M1
	.long Op24M1
	.long Op25M1
	.long Op26M1
	.long Op27M1
	.long Op28
	.long Op29M1
	.long Op2AM1
	.long Op2B
	.long Op2CM1
	.long Op2DM1
	.long Op2EM1
	.long Op2FM1
	.long Op30
	.long Op31M1
	.long Op32M1
	.long Op33M1
	.long Op34M1
	.long Op35M1
	.long Op36M1
	.long Op37M1
	.long Op38
	.long Op39M1
	.long Op3AM1
	.long Op3B
	.long Op3CM1
	.long Op3DM1
	.long Op3EM1
	.long Op3FM1
	.long Op40
	.long Op41M1
	.long Op42
	.long Op43M1
	.long Op44X1
	.long Op45M1
	.long Op46M1
	.long Op47M1
	.long Op48M1
	.long Op49M1
	.long Op4AM1
	.long Op4B
	.long Op4C
	.long Op4DM1
	.long Op4EM1
	.long Op4FM1
	.long Op50
	.long Op51M1
	.long Op52M1
	.long Op53M1
	.long Op54X1
	.long Op55M1
	.long Op56M1
	.long Op57M1
	.long Op58
	.long Op59M1
	.long Op5AX1
	.long Op5B
	.long Op5C
	.long Op5DM1
	.long Op5EM1
	.long Op5FM1
	.long Op60
	.long Op61M1
	.long Op62
	.long Op63M1
	.long Op64M1
	.long Op65M1
	.long Op66M1
	.long Op67M1
	.long Op68M1
	.long Op69M1
	.long Op6AM1
	.long Op6B
	.long Op6C
	.long Op6DM1
	.long Op6EM1
	.long Op6FM1
	.long Op70
	.long Op71M1
	.long Op72M1
	.long Op73M1
	.long Op74M1
	.long Op75M1
	.long Op76M1
	.long Op77M1
	.long Op78
	.long Op79M1
	.long Op7AX1
	.long Op7B
	.long Op7C
	.long Op7DM1
	.long Op7EM1
	.long Op7FM1
	.long Op80
	.long Op81M1
	.long Op82
	.long Op83M1
	.long Op84X1
	.long Op85M1
	.long Op86X1
	.long Op87M1
	.long Op88X1
	.long Op89M1
	.long Op8AM1
	.long Op8B
	.long Op8CX1
	.long Op8DM1
	.long Op8EX1
	.long Op8FM1
	.long Op90
	.long Op91M1
	.long Op92M1
	.long Op93M1
	.long Op94X1
	.long Op95M1
	.long Op96X1
	.long Op97M1
	.long Op98M1
	.long Op99M1
	.long Op9A
	.long Op9BX1
	.long Op9CM1
	.long Op9DM1
	.long Op9EM1
	.long Op9FM1
	.long OpA0X1
	.long OpA1M1
	.long OpA2X1
	.long OpA3M1
	.long OpA4X1
	.long OpA5M1
	.long OpA6X1
	.long OpA7M1
	.long OpA8X1
	.long OpA9M1
	.long OpAAX1
	.long OpAB
	.long OpACX1
	.long OpADM1
	.long OpAEX1
	.long OpAFM1
	.long OpB0
	.long OpB1M1
	.long OpB2M1
	.long OpB3M1
	.long OpB4X1
	.long OpB5M1
	.long OpB6X1
	.long OpB7M1
	.long OpB8
	.long OpB9M1
	.long OpBAX1
	.long OpBBX1
	.long OpBCX1
	.long OpBDM1
	.long OpBEX1
	.long OpBFM1
	.long OpC0X1
	.long OpC1M1
	.long OpC2
	.long OpC3M1
	.long OpC4X1
	.long OpC5M1
	.long OpC6M1
	.long OpC7M1
	.long OpC8X1
	.long OpC9M1
	.long OpCAX1
	.long OpCB
	.long OpCCX1
	.long OpCDM1
	.long OpCEM1
	.long OpCFM1
	.long OpD0
	.long OpD1M1
	.long OpD2M1
	.long OpD3M1
	.long OpD4
	.long OpD5M1
	.long OpD6M1
	.long OpD7M1
	.long OpD8
	.long OpD9M1
	.long OpDAX1
	.long OpDB
	.long OpDC
	.long OpDDM1
	.long OpDEM1
	.long OpDFM1
	.long OpE0X1
	.long OpE1M1
	.long OpE2
	.long OpE3M1
	.long OpE4X1
	.long OpE5M1
	.long OpE6M1
	.long OpE7M1
	.long OpE8X1
	.long OpE9M1
	.long OpEA
	.long OpEB
	.long OpECX1
	.long OpEDM1
	.long OpEEM1
	.long OpEFM1
	.long OpF0
	.long OpF1M1
	.long OpF2M1
	.long OpF3M1
	.long OpF4
	.long OpF5M1
	.long OpF6M1
	.long OpF7M1
	.long OpF8
	.long OpF9M1
	.long OpFAX1
	.long OpFB
	.long OpFC
	.long OpFDM1
	.long OpFEM1
	.long OpFFM1

.globl _S9xOpcodesM1X0
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#else
	.align 2
#endif
_S9xOpcodesM1X0:
	.long Op00
	.long Op01M1
	.long Op02
	.long Op03M1
	.long Op04M1
	.long Op05M1
	.long Op06M1
	.long Op07M1
	.long Op08
	.long Op09M1
	.long Op0AM1
	.long Op0B
	.long Op0CM1
	.long Op0DM1
	.long Op0EM1
	.long Op0FM1
	.long Op10
	.long Op11M1
	.long Op12M1
	.long Op13M1
	.long Op14M1
	.long Op15M1
	.long Op16M1
	.long Op17M1
	.long Op18
	.long Op19M1
	.long Op1AM1
	.long Op1B
	.long Op1CM1
	.long Op1DM1
	.long Op1EM1
	.long Op1FM1
	.long Op20
	.long Op21M1
	.long Op22
	.long Op23M1
	.long Op24M1
	.long Op25M1
	.long Op26M1
	.long Op27M1
	.long Op28
	.long Op29M1
	.long Op2AM1
	.long Op2B
	.long Op2CM1
	.long Op2DM1
	.long Op2EM1
	.long Op2FM1
	.long Op30
	.long Op31M1
	.long Op32M1
	.long Op33M1
	.long Op34M1
	.long Op35M1
	.long Op36M1
	.long Op37M1
	.long Op38
	.long Op39M1
	.long Op3AM1
	.long Op3B
	.long Op3CM1
	.long Op3DM1
	.long Op3EM1
	.long Op3FM1
	.long Op40
	.long Op41M1
	.long Op42
	.long Op43M1
	.long Op44X0
	.long Op45M1
	.long Op46M1
	.long Op47M1
	.long Op48M1
	.long Op49M1
	.long Op4AM1
	.long Op4B
	.long Op4C
	.long Op4DM1
	.long Op4EM1
	.long Op4FM1
	.long Op50
	.long Op51M1
	.long Op52M1
	.long Op53M1
	.long Op54X0
	.long Op55M1
	.long Op56M1
	.long Op57M1
	.long Op58
	.long Op59M1
	.long Op5AX0
	.long Op5B
	.long Op5C
	.long Op5DM1
	.long Op5EM1
	.long Op5FM1
	.long Op60
	.long Op61M1
	.long Op62
	.long Op63M1
	.long Op64M1
	.long Op65M1
	.long Op66M1
	.long Op67M1
	.long Op68M1
	.long Op69M1
	.long Op6AM1
	.long Op6B
	.long Op6C
	.long Op6DM1
	.long Op6EM1
	.long Op6FM1
	.long Op70
	.long Op71M1
	.long Op72M1
	.long Op73M1
	.long Op74M1
	.long Op75M1
	.long Op76M1
	.long Op77M1
	.long Op78
	.long Op79M1
	.long Op7AX0
	.long Op7B
	.long Op7C
	.long Op7DM1
	.long Op7EM1
	.long Op7FM1
	.long Op80
	.long Op81M1
	.long Op82
	.long Op83M1
	.long Op84X0
	.long Op85M1
	.long Op86X0
	.long Op87M1
	.long Op88X0
	.long Op89M1
	.long Op8AM1
	.long Op8B
	.long Op8CX0
	.long Op8DM1
	.long Op8EX0
	.long Op8FM1
	.long Op90
	.long Op91M1
	.long Op92M1
	.long Op93M1
	.long Op94X0
	.long Op95M1
	.long Op96X0
	.long Op97M1
	.long Op98M1
	.long Op99M1
	.long Op9A
	.long Op9BX0
	.long Op9CM1
	.long Op9DM1
	.long Op9EM1
	.long Op9FM1
	.long OpA0X0
	.long OpA1M1
	.long OpA2X0
	.long OpA3M1
	.long OpA4X0
	.long OpA5M1
	.long OpA6X0
	.long OpA7M1
	.long OpA8X0
	.long OpA9M1
	.long OpAAX0
	.long OpAB
	.long OpACX0
	.long OpADM1
	.long OpAEX0
	.long OpAFM1
	.long OpB0
	.long OpB1M1
	.long OpB2M1
	.long OpB3M1
	.long OpB4X0
	.long OpB5M1
	.long OpB6X0
	.long OpB7M1
	.long OpB8
	.long OpB9M1
	.long OpBAX0
	.long OpBBX0
	.long OpBCX0
	.long OpBDM1
	.long OpBEX0
	.long OpBFM1
	.long OpC0X0
	.long OpC1M1
	.long OpC2
	.long OpC3M1
	.long OpC4X0
	.long OpC5M1
	.long OpC6M1
	.long OpC7M1
	.long OpC8X0
	.long OpC9M1
	.long OpCAX0
	.long OpCB
	.long OpCCX0
	.long OpCDM1
	.long OpCEM1
	.long OpCFM1
	.long OpD0
	.long OpD1M1
	.long OpD2M1
	.long OpD3M1
	.long OpD4
	.long OpD5M1
	.long OpD6M1
	.long OpD7M1
	.long OpD8
	.long OpD9M1
	.long OpDAX0
	.long OpDB
	.long OpDC
	.long OpDDM1
	.long OpDEM1
	.long OpDFM1
	.long OpE0X0
	.long OpE1M1
	.long OpE2
	.long OpE3M1
	.long OpE4X0
	.long OpE5M1
	.long OpE6M1
	.long OpE7M1
	.long OpE8X0
	.long OpE9M1
	.long OpEA
	.long OpEB
	.long OpECX0
	.long OpEDM1
	.long OpEEM1
	.long OpEFM1
	.long OpF0
	.long OpF1M1
	.long OpF2M1
	.long OpF3M1
	.long OpF4
	.long OpF5M1
	.long OpF6M1
	.long OpF7M1
	.long OpF8
	.long OpF9M1
	.long OpFAX0
	.long OpFB
	.long OpFC
	.long OpFDM1
	.long OpFEM1
	.long OpFFM1

.globl _S9xOpcodesM0X0
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#else
	.align 2
#endif
_S9xOpcodesM0X0:
	.long Op00
	.long Op01M0
	.long Op02
	.long Op03M0
	.long Op04M0
	.long Op05M0
	.long Op06M0
	.long Op07M0
	.long Op08
	.long Op09M0
	.long Op0AM0
	.long Op0B
	.long Op0CM0
	.long Op0DM0
	.long Op0EM0
	.long Op0FM0
	.long Op10
	.long Op11M0
	.long Op12M0
	.long Op13M0
	.long Op14M0
	.long Op15M0
	.long Op16M0
	.long Op17M0
	.long Op18
	.long Op19M0
	.long Op1AM0
	.long Op1B
	.long Op1CM0
	.long Op1DM0
	.long Op1EM0
	.long Op1FM0
	.long Op20
	.long Op21M0
	.long Op22
	.long Op23M0
	.long Op24M0
	.long Op25M0
	.long Op26M0
	.long Op27M0
	.long Op28
	.long Op29M0
	.long Op2AM0
	.long Op2B
	.long Op2CM0
	.long Op2DM0
	.long Op2EM0
	.long Op2FM0
	.long Op30
	.long Op31M0
	.long Op32M0
	.long Op33M0
	.long Op34M0
	.long Op35M0
	.long Op36M0
	.long Op37M0
	.long Op38
	.long Op39M0
	.long Op3AM0
	.long Op3B
	.long Op3CM0
	.long Op3DM0
	.long Op3EM0
	.long Op3FM0
	.long Op40
	.long Op41M0
	.long Op42
	.long Op43M0
	.long Op44X0
	.long Op45M0
	.long Op46M0
	.long Op47M0
	.long Op48M0
	.long Op49M0
	.long Op4AM0
	.long Op4B
	.long Op4C
	.long Op4DM0
	.long Op4EM0
	.long Op4FM0
	.long Op50
	.long Op51M0
	.long Op52M0
	.long Op53M0
	.long Op54X0
	.long Op55M0
	.long Op56M0
	.long Op57M0
	.long Op58
	.long Op59M0
	.long Op5AX0
	.long Op5B
	.long Op5C
	.long Op5DM0
	.long Op5EM0
	.long Op5FM0
	.long Op60
	.long Op61M0
	.long Op62
	.long Op63M0
	.long Op64M0
	.long Op65M0
	.long Op66M0
	.long Op67M0
	.long Op68M0
	.long Op69M0
	.long Op6AM0
	.long Op6B
	.long Op6C
	.long Op6DM0
	.long Op6EM0
	.long Op6FM0
	.long Op70
	.long Op71M0
	.long Op72M0
	.long Op73M0
	.long Op74M0
	.long Op75M0
	.long Op76M0
	.long Op77M0
	.long Op78
	.long Op79M0
	.long Op7AX0
	.long Op7B
	.long Op7C
	.long Op7DM0
	.long Op7EM0
	.long Op7FM0
	.long Op80
	.long Op81M0
	.long Op82
	.long Op83M0
	.long Op84X0
	.long Op85M0
	.long Op86X0
	.long Op87M0
	.long Op88X0
	.long Op89M0
	.long Op8AM0
	.long Op8B
	.long Op8CX0
	.long Op8DM0
	.long Op8EX0
	.long Op8FM0
	.long Op90
	.long Op91M0
	.long Op92M0
	.long Op93M0
	.long Op94X0
	.long Op95M0
	.long Op96X0
	.long Op97M0
	.long Op98M0
	.long Op99M0
	.long Op9A
	.long Op9BX0
	.long Op9CM0
	.long Op9DM0
	.long Op9EM0
	.long Op9FM0
	.long OpA0X0
	.long OpA1M0
	.long OpA2X0
	.long OpA3M0
	.long OpA4X0
	.long OpA5M0
	.long OpA6X0
	.long OpA7M0
	.long OpA8X0
	.long OpA9M0
	.long OpAAX0
	.long OpAB
	.long OpACX0
	.long OpADM0
	.long OpAEX0
	.long OpAFM0
	.long OpB0
	.long OpB1M0
	.long OpB2M0
	.long OpB3M0
	.long OpB4X0
	.long OpB5M0
	.long OpB6X0
	.long OpB7M0
	.long OpB8
	.long OpB9M0
	.long OpBAX0
	.long OpBBX0
	.long OpBCX0
	.long OpBDM0
	.long OpBEX0
	.long OpBFM0
	.long OpC0X0
	.long OpC1M0
	.long OpC2
	.long OpC3M0
	.long OpC4X0
	.long OpC5M0
	.long OpC6M0
	.long OpC7M0
	.long OpC8X0
	.long OpC9M0
	.long OpCAX0
	.long OpCB
	.long OpCCX0
	.long OpCDM0
	.long OpCEM0
	.long OpCFM0
	.long OpD0
	.long OpD1M0
	.long OpD2M0
	.long OpD3M0
	.long OpD4
	.long OpD5M0
	.long OpD6M0
	.long OpD7M0
	.long OpD8
	.long OpD9M0
	.long OpDAX0
	.long OpDB
	.long OpDC
	.long OpDDM0
	.long OpDEM0
	.long OpDFM0
	.long OpE0X0
	.long OpE1M0
	.long OpE2
	.long OpE3M0
	.long OpE4X0
	.long OpE5M0
	.long OpE6M0
	.long OpE7M0
	.long OpE8X0
	.long OpE9M0
	.long OpEA
	.long OpEB
	.long OpECX0
	.long OpEDM0
	.long OpEEM0
	.long OpEFM0
	.long OpF0
	.long OpF1M0
	.long OpF2M0
	.long OpF3M0
	.long OpF4
	.long OpF5M0
	.long OpF6M0
	.long OpF7M0
	.long OpF8
	.long OpF9M0
	.long OpFAX0
	.long OpFB
	.long OpFC
	.long OpFDM0
	.long OpFEM0
	.long OpFFM0

.globl _S9xOpcodesM0X1
#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#else
	.align 2
#endif
_S9xOpcodesM0X1:
	.long Op00
	.long Op01M0
	.long Op02
	.long Op03M0
	.long Op04M0
	.long Op05M0
	.long Op06M0
	.long Op07M0
	.long Op08
	.long Op09M0
	.long Op0AM0
	.long Op0B
	.long Op0CM0
	.long Op0DM0
	.long Op0EM0
	.long Op0FM0
	.long Op10
	.long Op11M0
	.long Op12M0
	.long Op13M0
	.long Op14M0
	.long Op15M0
	.long Op16M0
	.long Op17M0
	.long Op18
	.long Op19M0
	.long Op1AM0
	.long Op1B
	.long Op1CM0
	.long Op1DM0
	.long Op1EM0
	.long Op1FM0
	.long Op20
	.long Op21M0
	.long Op22
	.long Op23M0
	.long Op24M0
	.long Op25M0
	.long Op26M0
	.long Op27M0
	.long Op28
	.long Op29M0
	.long Op2AM0
	.long Op2B
	.long Op2CM0
	.long Op2DM0
	.long Op2EM0
	.long Op2FM0
	.long Op30
	.long Op31M0
	.long Op32M0
	.long Op33M0
	.long Op34M0
	.long Op35M0
	.long Op36M0
	.long Op37M0
	.long Op38
	.long Op39M0
	.long Op3AM0
	.long Op3B
	.long Op3CM0
	.long Op3DM0
	.long Op3EM0
	.long Op3FM0
	.long Op40
	.long Op41M0
	.long Op42
	.long Op43M0
	.long Op44X1
	.long Op45M0
	.long Op46M0
	.long Op47M0
	.long Op48M0
	.long Op49M0
	.long Op4AM0
	.long Op4B
	.long Op4C
	.long Op4DM0
	.long Op4EM0
	.long Op4FM0
	.long Op50
	.long Op51M0
	.long Op52M0
	.long Op53M0
	.long Op54X1
	.long Op55M0
	.long Op56M0
	.long Op57M0
	.long Op58
	.long Op59M0
	.long Op5AX1
	.long Op5B
	.long Op5C
	.long Op5DM0
	.long Op5EM0
	.long Op5FM0
	.long Op60
	.long Op61M0
	.long Op62
	.long Op63M0
	.long Op64M0
	.long Op65M0
	.long Op66M0
	.long Op67M0
	.long Op68M0
	.long Op69M0
	.long Op6AM0
	.long Op6B
	.long Op6C
	.long Op6DM0
	.long Op6EM0
	.long Op6FM0
	.long Op70
	.long Op71M0
	.long Op72M0
	.long Op73M0
	.long Op74M0
	.long Op75M0
	.long Op76M0
	.long Op77M0
	.long Op78
	.long Op79M0
	.long Op7AX1
	.long Op7B
	.long Op7C
	.long Op7DM0
	.long Op7EM0
	.long Op7FM0
	.long Op80
	.long Op81M0
	.long Op82
	.long Op83M0
	.long Op84X1
	.long Op85M0
	.long Op86X1
	.long Op87M0
	.long Op88X1
	.long Op89M0
	.long Op8AM0
	.long Op8B
	.long Op8CX1
	.long Op8DM0
	.long Op8EX1
	.long Op8FM0
	.long Op90
	.long Op91M0
	.long Op92M0
	.long Op93M0
	.long Op94X1
	.long Op95M0
	.long Op96X1
	.long Op97M0
	.long Op98M0
	.long Op99M0
	.long Op9A
	.long Op9BX1
	.long Op9CM0
	.long Op9DM0
	.long Op9EM0
	.long Op9FM0
	.long OpA0X1
	.long OpA1M0
	.long OpA2X1
	.long OpA3M0
	.long OpA4X1
	.long OpA5M0
	.long OpA6X1
	.long OpA7M0
	.long OpA8X1
	.long OpA9M0
	.long OpAAX1
	.long OpAB
	.long OpACX1
	.long OpADM0
	.long OpAEX1
	.long OpAFM0
	.long OpB0
	.long OpB1M0
	.long OpB2M0
	.long OpB3M0
	.long OpB4X1
	.long OpB5M0
	.long OpB6X1
	.long OpB7M0
	.long OpB8
	.long OpB9M0
	.long OpBAX1
	.long OpBBX1
	.long OpBCX1
	.long OpBDM0
	.long OpBEX1
	.long OpBFM0
	.long OpC0X1
	.long OpC1M0
	.long OpC2
	.long OpC3M0
	.long OpC4X1
	.long OpC5M0
	.long OpC6M0
	.long OpC7M0
	.long OpC8X1
	.long OpC9M0
	.long OpCAX1
	.long OpCB
	.long OpCCX1
	.long OpCDM0
	.long OpCEM0
	.long OpCFM0
	.long OpD0
	.long OpD1M0
	.long OpD2M0
	.long OpD3M0
	.long OpD4
	.long OpD5M0
	.long OpD6M0
	.long OpD7M0
	.long OpD8
	.long OpD9M0
	.long OpDAX1
	.long OpDB
	.long OpDC
	.long OpDDM0
	.long OpDEM0
	.long OpDFM0
	.long OpE0X1
	.long OpE1M0
	.long OpE2
	.long OpE3M0
	.long OpE4X1
	.long OpE5M0
	.long OpE6M0
	.long OpE7M0
	.long OpE8X1
	.long OpE9M0
	.long OpEA
	.long OpEB
	.long OpECX1
	.long OpEDM0
	.long OpEEM0
	.long OpEFM0
	.long OpF0
	.long OpF1M0
	.long OpF2M0
	.long OpF3M0
	.long OpF4
	.long OpF5M0
	.long OpF6M0
	.long OpF7M0
	.long OpF8
	.long OpF9M0
	.long OpFAX1
	.long OpFB
	.long OpFC
	.long OpFDM0
	.long OpFEM0
	.long OpFFM0

#ifdef USE_ASM_ALIGN_OPCODES
.align 5
#else
	.align 2
#endif

