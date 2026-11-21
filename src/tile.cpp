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

#include "snes4all.h"

#include "memmap.h"
#include "ppu.h"
#include "display.h"
#include "gfx.h"
#include "tile.h"

//#define USE_OLD_WRITE_PIXEL
//#define USE_OLD_DRAW_TILE
//#undef TILES_INLINE

extern uint32 HeadMask [4];
extern uint32 TailMask [5];

#ifdef TILES_INLINE
#define STATIC_INLINE static __inline__
#else
#define STATIC_INLINE static
#endif


uint8 ConvertTile (uint8 *pCache, uint32 TileAddr)
{
//printf("ConvertTile(%i), shift=%i\n",TileAddr, BG.BitShift);
    register uint8 *tp = &CMemory_VRAM[TileAddr];
    uint32 *p = (uint32 *) pCache;
    uint32 non_zero = 0;
    uint8 line;

    switch (BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;

#if defined(DREAMCAST) && defined(USE_CONVERTILE_PREFETCH)
	    asm("pref @%0" : : "r" (tp + 32));
#endif
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1 |= odd_high[2][pix >> 4];
		p2 |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1 |= even_high[2][pix >> 4];
		p2 |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1 |= odd_high[3][pix >> 4];
		p2 |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1 |= even_high[3][pix >> 4];
		p2 |= even_low[3][pix & 0xf];
	    }
	    *p++ = p1;
	    *p++ = p2;
	    non_zero |= p1 | p2;
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;
#if defined(DREAMCAST) && defined(USE_CONVERTILE_PREF)
	    asm("pref @%0" : : "r" (tp + 32));
#endif
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    *p++ = p1;
	    *p++ = p2;
	    non_zero |= p1 | p2;
	}
	break;

    case 2:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;
#if defined(DREAMCAST) && defined(USE_CONVERTILE_PREF)
	    asm("pref @%0" : : "r" (tp + 32));
#endif
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    *p++ = p1;
	    *p++ = p2;
	    non_zero |= p1 | p2;
	}
	break;
    }
//printf("=%i\n",non_zero);
    return (non_zero ? TRUE : BLANK_TILE);
}


/*
STATIC_INLINE void WRITE_4PIXELS (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS_FLIPPED (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

STATIC_INLINE void WRITE_4PIXELSx2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel((N<<1)+1, Pixel); \
	TILE_SetPixel((N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS_FLIPPEDx2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel((N<<1)+1, Pixel); \
	TILE_SetPixel((N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

STATIC_INLINE void WRITE_4PIXELSx2x2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel((N<<1), Pixel); \
	TILE_SetPixel((N<<1)+1, Pixel); \
	TILE_SetPixel(SNES4ALL_SCREEN_PITCH+(N<<1), Pixel); \
	TILE_SetPixel(SNES4ALL_SCREEN_PITCH+(N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS_FLIPPEDx2x2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel(N<<1, Pixel); \
	TILE_SetPixel((N<<1)+1, Pixel); \
	TILE_SetPixel(SNES4ALL_SCREEN_PITCH+(N<<1), Pixel); \
	TILE_SetPixel(SNES4ALL_SCREEN_PITCH+(N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

void DrawTile (uint32 Tile, uint32 Offset, uint32 StartLine,
	       uint32 LineCount)
{
    TILE_PREAMBLE

    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS, WRITE_4PIXELS_FLIPPED, 4)
}

void DrawClippedTile (uint32 Tile, uint32 Offset,
		      uint32 StartPixel, uint32 Width,
		      uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS, WRITE_4PIXELS_FLIPPED, 4)
}

void DrawTilex2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		 uint32 LineCount)
{
    TILE_PREAMBLE

    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELSx2, WRITE_4PIXELS_FLIPPEDx2, 8)
}

void DrawClippedTilex2 (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELSx2, WRITE_4PIXELS_FLIPPEDx2, 8)
}

void DrawTilex2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
    TILE_PREAMBLE

    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELSx2x2, WRITE_4PIXELS_FLIPPEDx2x2, 8)
}

void DrawClippedTilex2x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELSx2x2, WRITE_4PIXELS_FLIPPEDx2x2, 8)
}
*/

STATIC_INLINE void WRITE_4PIXELS16 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
    }
    FN() Screen++; Depth++; Pixels++;
    FN() Screen++; Depth++; Pixels++;
    FN() Screen++; Depth++; Pixels++;
    FN()
#endif
#undef FN
}

#ifndef USE_OLD_DRAW_TILE
static void WRITE_4PIXELS16_2 (uint32 Offset, uint8 *Pixels, uint32  LineCount)
{
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
    for (uint32 l = LineCount; l != 0; l--) {
    uint32 Pixel;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    if (*(uint32 *)Pixels) {
	FN(0)
	FN(1)
	FN(2)
	FN(3)
    }
    if (*(uint32 *)(Pixels+4)) {
	FN(4)
	FN(5)
	FN(6)
	FN(7)
    }
    Pixels+=8;
    Screen+=SNES4ALL_SCREEN_PITCH2;
    Depth+=SNES4ALL_SCREEN_PITCH2;
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
    }
    if (*(uint32 *)Pixels) {
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    } else {
    	Pixels+=4; Depth+=4; Screen+=4;
    }
    if (*(uint32 *)Pixels) {
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    } else {
    	Pixels+=4; Depth+=4; Screen+=4;
    }
    Screen+=SNES4ALL_SCREEN_PITCH2-8;
    Depth+=SNES4ALL_SCREEN_PITCH2-8;
#endif
    }
#undef FN
}

static void WRITE_4PIXELS16_2V (uint32 Offset, uint8 *Pixels, uint32  LineCount)
{
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
    for (uint32 l = LineCount; l != 0; l--) {
    uint32 Pixel;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    if (*(uint32 *)Pixels) {
	FN(0)
	FN(1)
	FN(2)
	FN(3)
    }
    if (*(uint32 *)(Pixels+4)) {
	FN(4)
	FN(5)
	FN(6)
	FN(7)
    }
    Pixels-=8;
    Screen+=SNES4ALL_SCREEN_PITCH2;
    Depth+=SNES4ALL_SCREEN_PITCH2;
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
    }
    if (*(uint32 *)Pixels) {
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    } else {
    	Pixels+=4; Depth+=4; Screen+=4;
    }
    if (*(uint32 *)Pixels) {
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels++;
    	FN() Screen++; Depth++; Pixels-=15;
    } else  {
    	Pixels-=12; Depth+=4; Screen+=4;
    }
    Screen+=SNES4ALL_SCREEN_PITCH2-8;
    Depth+=SNES4ALL_SCREEN_PITCH2-8;
#endif
    }
#undef FN
}
#endif

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
    }

    Pixels += 3;
    FN() Screen++; Depth++; Pixels--;
    FN() Screen++; Depth++; Pixels--;
    FN() Screen++; Depth++; Pixels--;
    FN()
#endif
#undef FN
}

#ifndef USE_OLD_DRAW_TILE
static void WRITE_4PIXELS16_FLIPPED_2 (uint32 Offset, uint8 *Pixels, uint32 LineCount)
{
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
#ifndef USE_OLD_WRITE_PIXEL
    Pixels+=4;
#endif
    for (uint32 l = LineCount; l != 0; l--) {
    uint32 Pixel;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N,M) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[M])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    if (*(uint32 *)(Pixels+4)) {
	FN(0,4+3)
	FN(1,4+2)
	FN(2,4+1)
	FN(3,4+0)
    }
    if (*(uint32 *)(Pixels)) {
	FN(4+0,3)
	FN(4+1,2)
	FN(4+2,1)
	FN(4+3,0)
    }
    Pixels+=8;
    Screen+=SNES4ALL_SCREEN_PITCH2;
    Depth+=SNES4ALL_SCREEN_PITCH2;
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
    }

    if (*(uint32 *)Pixels) {
	Pixels += 3;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; 
    } else {
    	Depth+=4; Screen+=4;
    }
    Pixels -= 4;
    if (*(uint32 *)Pixels) {
	Pixels+=3;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; 
    } else {
    	Depth+=4; Screen+=4;
    }
    Pixels+=12;
    Screen+=SNES4ALL_SCREEN_PITCH2-8;
    Depth+=SNES4ALL_SCREEN_PITCH2-8;

#endif
    }
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_2V (uint32 Offset, uint8 *Pixels, uint32 LineCount)
{
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
#ifndef USE_OLD_WRITE_PIXEL
    Pixels+=4;
#endif
    for (uint32 l = LineCount; l != 0; l--) {
    uint32 Pixel;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N,M) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[M])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    if (*(uint32 *)(Pixels+4)) {
	FN(0,4+3)
	FN(1,4+2)
	FN(2,4+1)
	FN(3,4+0)
    }
    if (*(uint32 *)(Pixels)) {
	FN(4+0,3)
	FN(4+1,2)
	FN(4+2,1)
	FN(4+3,0)
    }
    Pixels-=8;
    Screen+=SNES4ALL_SCREEN_PITCH2;
    Depth+=SNES4ALL_SCREEN_PITCH2;
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
    }

    if (*(uint32 *)Pixels) {
	Pixels += 3;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++;
    } else {
	Depth+=4; Screen+=4;
    }
    Pixels-=4;
    if (*(uint32 *)Pixels) {
	Pixels += 3;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++; Pixels--;
	FN() Screen++; Depth++;
    } else {
    	Depth+=4; Screen+=4;
    }
    Pixels-=4;
    Screen+=SNES4ALL_SCREEN_PITCH2-8;
    Depth+=SNES4ALL_SCREEN_PITCH2-8;

#endif
    }
#undef FN
}
#endif

#if 0
STATIC_INLINE void WRITE_4PIXELS16x2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16((N<<1), Pixel); \
	TILE_SetPixel16((N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
    } else \
	Screen+=2; Depth+=2; \

    FN() Pixels++;
    FN() Pixels++;
    FN() Pixels++;
    FN()
#endif
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPEDx2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel16((N<<1), Pixel); \
	TILE_SetPixel16((N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
    } else \
	Screen+=2; Depth+=2; \

    Pixels += 3;
    FN() Pixels--;
    FN() Pixels--;
    FN() Pixels--;
    FN()
#endif
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16x2x2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16((N<<1), Pixel); \
	TILE_SetPixel16((N<<1)+1, Pixel); \
	TILE_SetPixel16((SNES4ALL_SCREEN_PITCH>>1)+(N<<1), Pixel); \
	TILE_SetPixel16((SNES4ALL_SCREEN_PITCH>>1)+(N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
	TILE_SetPixel16_(Pixel); \
	Screen+=(SNES4ALL_SCREEN_PITCH>>1)-1; Depth+=(SNES4ALL_SCREEN_PITCH>>1)-1; \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
	TILE_SetPixel16_(Pixel); \
	Screen-=(SNES4ALL_SCREEN_PITCH>>1)-1; Depth-=(SNES4ALL_SCREEN_PITCH>>1)-1; \
    } else \
    	Screen+=2; Depth+=2;

    FN() Pixels++;
    FN() Pixels++;
    FN() Pixels++;
    FN()
#endif
#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPEDx2x2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#ifdef USE_OLD_WRITE_PIXEL
#define FN(N) \
    if (GFX.Z1 > Depth [N << 1] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel16((N<<1), Pixel); \
	TILE_SetPixel16((N<<1)+1, Pixel); \
	TILE_SetPixel16((SNES4ALL_SCREEN_PITCH>>1)+(N<<1), Pixel); \
	TILE_SetPixel16((SNES4ALL_SCREEN_PITCH>>1)+(N<<1)+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#else
    const uint8 _GFX_Z2_=GFX.Z2;
    const uint8 _GFX_Z1_=GFX.Z1;
#define FN() \
    if (_GFX_Z1_ > *Depth && (Pixel = *Pixels)) { \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
	TILE_SetPixel16_(Pixel); \
	Screen+=(SNES4ALL_SCREEN_PITCH>>1)-1; Depth+=(SNES4ALL_SCREEN_PITCH>>1)-1; \
	TILE_SetPixel16_(Pixel); \
	Screen++; Depth++; \
	TILE_SetPixel16_(Pixel); \
	Screen-=(SNES4ALL_SCREEN_PITCH>>1)-1; Depth-=(SNES4ALL_SCREEN_PITCH>>1)-1; \
    } else \
    	Screen+=2; Depth+=2;

    Pixels+=3;
    FN() Pixels--;
    FN() Pixels--;
    FN() Pixels--;
    FN()
#endif
#undef FN
}
#endif

void DrawTile16 (uint32 Tile, uint32 Offset, uint32 StartLine,
	         uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

#ifdef USE_OLD_DRAW_TILE
    RENDER_TILE(WRITE_4PIXELS16, WRITE_4PIXELS16_FLIPPED, 4)
#else
    RENDER_TILE2(WRITE_4PIXELS16_2, WRITE_4PIXELS16_2V, WRITE_4PIXELS16_FLIPPED_2, WRITE_4PIXELS16_FLIPPED_2V)
#endif
}

void DrawClippedTile16 (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16, WRITE_4PIXELS16_FLIPPED, 4)
}

#if 0
void DrawTile16x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;
    RENDER_TILE(WRITE_4PIXELS16x2, WRITE_4PIXELS16_FLIPPEDx2, 8)
}

void DrawClippedTile16x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16x2, WRITE_4PIXELS16_FLIPPEDx2, 8)
}

void DrawTile16x2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		     uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16x2x2, WRITE_4PIXELS16_FLIPPEDx2x2, 8)
}

void DrawClippedTile16x2x2 (uint32 Tile, uint32 Offset,
			    uint32 StartPixel, uint32 Width,
			    uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16x2x2, WRITE_4PIXELS16_FLIPPEDx2x2, 8)
}
#endif

STATIC_INLINE void WRITE_4PIXELS16_ADD (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectAddPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_ADD (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectAddPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_ADD1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_ADD1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_SUB (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectSubPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_SUB (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectSubPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_SUB1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_SUB1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}


void DrawTile16Add (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_ADD, WRITE_4PIXELS16_FLIPPED_ADD, 4)
}

void DrawClippedTile16Add (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_ADD, WRITE_4PIXELS16_FLIPPED_ADD, 4)
}

void DrawTile16Add1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_ADD1_2, WRITE_4PIXELS16_FLIPPED_ADD1_2, 4)
}

void DrawClippedTile16Add1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_ADD1_2, WRITE_4PIXELS16_FLIPPED_ADD1_2, 4)
}

void DrawTile16Sub (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4)
}

void DrawClippedTile16Sub (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4)
}

void DrawTile16Sub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_SUB1_2, WRITE_4PIXELS16_FLIPPED_SUB1_2, 4)
}

void DrawClippedTile16Sub1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUB1_2, WRITE_4PIXELS16_FLIPPED_SUB1_2, 4)
}

STATIC_INLINE void WRITE_4PIXELS16_ADDF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectFAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_ADDF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectFAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_SUBF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectFSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

STATIC_INLINE void WRITE_4PIXELS16_FLIPPED_SUBF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectFSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

void DrawTile16FixedAdd1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_ADDF1_2, WRITE_4PIXELS16_FLIPPED_ADDF1_2, 4)
}

void DrawClippedTile16FixedAdd1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_ADDF1_2, 
			WRITE_4PIXELS16_FLIPPED_ADDF1_2, 4)
}

void DrawTile16FixedSub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_SUBF1_2, WRITE_4PIXELS16_FLIPPED_SUBF1_2, 4)
}

void DrawClippedTile16FixedSub1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUBF1_2, 
			WRITE_4PIXELS16_FLIPPED_SUBF1_2, 4)
}

#define PLOT_PIXEL(screen, pixel) (pixel)
#if 0
void DrawLargePixel (uint32 Tile, uint32 Offset,
		     uint32 StartPixel, uint32 Pixels,
		     uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint8 *sp = GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
    uint8 pixel;

    RENDER_TILE_LARGE (((uint8) GFX.ScreenColors [pixel]), PLOT_PIXEL)
}
#endif

void DrawLargePixel16 (uint32 Tile, uint32 Offset,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
    uint16 pixel;

    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], PLOT_PIXEL)
}

void DrawLargePixel16Add (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_ADD_PIXEL(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_ADD (p, *(s + GFX.Delta))    : \
			       COLOR_ADD (p, GFX.FixedColour)) \
			    : p)
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_ADD_PIXEL)
}

void DrawLargePixel16Add1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_ADD_PIXEL1_2(s, p) \
((uint16) (Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_ADD1_2 (p, *(s + GFX.Delta))    : \
			       COLOR_ADD (p, GFX.FixedColour)) \
			    : p))
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_ADD_PIXEL1_2)
}

void DrawLargePixel16Sub (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_SUB_PIXEL(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_SUB (p, *(s + GFX.Delta))    : \
			       COLOR_SUB (p, GFX.FixedColour)) \
			    : p)
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_SUB_PIXEL)
}

void DrawLargePixel16Sub1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_SUB_PIXEL1_2(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_SUB1_2 (p, *(s + GFX.Delta))    : \
			       COLOR_SUB (p, GFX.FixedColour)) \
			    : p)
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_SUB_PIXEL1_2)
}

