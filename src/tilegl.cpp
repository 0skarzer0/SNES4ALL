#ifdef USE_GL

#include "snes9x.h"

#include "snes4all.h"

#include "memmap.h"
#include "ppu.h"
#include "display.h"
#include "gfx.h"
#include "tile.h"

#include "videogl.h"

//#define SHOW_TODO
#ifdef USE_VIDEOGL_DIRECT_SLOT
#define get_free_slot(TILE) (unsigned short *)(((unsigned)videogl_cache)+((TILE)*8*8*2));
#else
static __inline__ unsigned short *get_free_slot(unsigned tile) {
//static unsigned iter_min=2*VIDEO_GL_MAX_TEX;
//static unsigned iter_max=0;
//static double iter_media=0.0;
//static int dime=0;
//unsigned iter=0;
	unsigned t=0;
	{
		unsigned short c=videogl_cache_tile_slot[tile];
		if (c!=0xffff)
			videogl_cache_slot[c]=0;
	}
	for(unsigned i=videogl_cache_slot_last;i<VIDEO_GL_MAX_TEX;i++)
		if (!videogl_cache_slot[i]) {
			t=i+1;
			break;
		}
//else iter++;
	if (t)
		t--;
	else {
		for(unsigned i=0;i<videogl_cache_slot_last;i++)
			if (!videogl_cache_slot[i]) {
				t=i;
				break;
			}
//else iter++;
//dime++;
	}
//if (dime){
//static unsigned cuantos=0;
//if (iter>iter_max) iter_max=iter;
//if (iter<iter_min) iter_min=iter;
//iter_media=((((((double)iter_media)*((double)cuantos))+((double)iter)))/((double)(cuantos+1)));
//printf("%i: min=%i, max=%i, media=%.2f\n",cuantos++,iter_min,iter_max,iter_media);
//}
//printf("%i (%i/%i)\n",t,videogl_cache_slot_last,VIDEO_GL_MAX_TEX);
	videogl_cache_slot[t]=tile;
	videogl_cache_tile_slot[tile]=t;
	videogl_cache_slot_last=t+1;
	return (unsigned short *)(((unsigned)videogl_cache)+(t*8*8*2));
}
#endif

uint8 ConvertTile (uint8 *pCache, uint32 TileAddr);

void DrawTile16hw (uint32 Tile, uint32 Offset,
		 uint32 StartLine, uint32 LineCount)
{
	if (videogl_tiles>VIDEO_GL_MAX_TILES) return;

	TILE_PREAMBLE

	if (!videogl_tiles)
		videogl_first_brightness=PPU.Brightness;
//	else videogl_last_brightness=PPU.Brightness;
//static int cuantos=0;
//cuantos++;
//if (cuantos!=10 && cuantos!=11 && cuantos<73) return;
//if (cuantos!=563 && cuantos<569) return;
//printf("DrawTile16hw Tile=%i(%i), Offset=%i, StartLine=%i, LineCount=%i, Z=%i\n",TileNumber,Tile,Offset,StartLine,LineCount,GFX.Z2);
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
#ifdef USE_QSORT
	videogl_tile_p[videogl_tiles]=tile;
#else
	if (GFX.Z2>videogl_max_z) videogl_max_z=GFX.Z2;
	if (GFX.Z2<videogl_min_z) videogl_min_z=GFX.Z2;
	videogl_tiles_z[GFX.Z2]++;
#endif
	videogl_tiles++;
	tile->z=GFX.Z2;
	tile->x=Offset;//&0xff;
	tile->y=(Offset>>8);//&0xff;
	tile->flip=((Tile & (V_FLIP | H_FLIP))>>14)&3;
	tile->clipx=0;
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=0;
#endif
	tile->tile=TileNumber;
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
	tile->clipy=(StartLine>>3);
	tile->y1=LineCount;
	tile->mosaic=0;
#else
	tile->clipy=((StartLine>>3)&0xf)|(LineCount<<4);
#endif
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
	tile->pal= ((((unsigned)&GFX.ScreenColors[0])-((unsigned)&IPPU.ScreenColors[0]))>>1);
//if (tile->pal) printf("pal %i (%i) Tile=%i\n",tile->pal,(((unsigned)&GFX.ScreenColors[0])-((unsigned)&IPPU.ScreenColors[0])),(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette);
#endif

	if (BG.Buffered==IPPU.TileCached[TILE_2BIT]) {
		if (tile->tile>1024)
			tile->tile-=1024;
		tile->tile&=0x7ff;
		tile->tile+=(((Tile>>10)&7)*0x800);
	} else {
		tile->tile+=(8*0x800)+(((Tile>>10)&7)*0x800);
	}

//printf("Tile %i, pal=%i (shift=%i) (tiles=%i)\n",tile->tile,tile->pal,BG.BitShift,videogl_tiles);
#if 0
	if (BG.Buffered==IPPU.TileCached[TILE_4BIT])
//{
//extern unsigned tiles_4bit_frame, tiles_4bit_min, tiles_4bit_max, tiles_4bit_minshift, tiles_4bit_maxshift, tiles_4bit_minmask, tiles_4bit_maxmask;
//tiles_4bit_frame++; 
//if (tile->tile>tiles_4bit_max) tiles_4bit_max=tile->tile;
//if (tile->tile<tiles_4bit_min) tiles_4bit_min=tile->tile;
//if (BG.PaletteMask>tiles_4bit_maxmask) tiles_4bit_maxmask=BG.PaletteMask;
//if (BG.PaletteMask<tiles_4bit_minmask) tiles_4bit_minmask=BG.PaletteMask;
//if (((Tile>>10)&BG.PaletteMask)>tiles_4bit_maxshift) tiles_4bit_maxshift=((Tile>>10)&BG.PaletteMask);
//if (((Tile>>10)&BG.PaletteMask)<tiles_4bit_minshift) tiles_4bit_minshift=((Tile>>10)&BG.PaletteMask);
		tile->tile+=(8*MAX_2BIT_TILES)+(((Tile>>10)&BG.PaletteMask)*MAX_4BIT_TILES);
//}
	else
	if (BG.Buffered==IPPU.TileCached[TILE_8BIT])
//{	
//extern unsigned tiles_8bit_frame, tiles_8bit_min, tiles_8bit_max, tiles_8bit_minshift, tiles_8bit_maxshift, tiles_8bit_minmask, tiles_8bit_maxmask;
//tiles_8bit_frame++; 
//if (tile->tile>tiles_8bit_max) tiles_8bit_max=tile->tile;
//if (tile->tile<tiles_8bit_min) tiles_8bit_min=tile->tile;
//if (BG.PaletteMask>tiles_8bit_maxmask) tiles_8bit_maxmask=BG.PaletteMask;
//if (BG.PaletteMask<tiles_8bit_minmask) tiles_8bit_minmask=BG.PaletteMask;
//if (((Tile>>10)&BG.PaletteMask)>tiles_8bit_maxshift) tiles_8bit_maxshift=((Tile>>10)&BG.PaletteMask);
//if (((Tile>>10)&BG.PaletteMask)<tiles_8bit_minshift) tiles_8bit_minshift=((Tile>>10)&BG.PaletteMask);
//		tile->tile+=(8*(MAX_2BIT_TILES+MAX_4BIT_TILES))+(((Tile>>10)&(7&BG.PaletteMask))*MAX_8BIT_TILES);
//}
	else
//{
//extern unsigned tiles_2bit_frame, tiles_2bit_min, tiles_2bit_max, tiles_2bit_minshift, tiles_2bit_maxshift, tiles_2bit_minmask, tiles_2bit_maxmask;
//tiles_2bit_frame++; 
//if (tile->tile>tiles_2bit_max) tiles_2bit_max=tile->tile;
//if (tile->tile<tiles_2bit_min) tiles_2bit_min=tile->tile;
//if (BG.PaletteMask>tiles_2bit_maxmask) tiles_2bit_maxmask=BG.PaletteMask;
//if (BG.PaletteMask<tiles_2bit_minmask) tiles_2bit_minmask=BG.PaletteMask;
//if (((Tile>>10)&BG.PaletteMask)>tiles_2bit_maxshift) tiles_2bit_maxshift=((Tile>>10)&BG.PaletteMask);
//if (((Tile>>10)&BG.PaletteMask)<tiles_2bit_minshift) tiles_2bit_minshift=((Tile>>10)&BG.PaletteMask);
//		tile->tile+=(((Tile>>10)&(7&BG.PaletteMask))*MAX_2BIT_TILES);
}
//else if (BG.Buffered!=IPPU.TileCached[0]) {
//puts("NO SE LOCALIZA EL BUFFERED"); exit(0); }
#endif
//if (tile->tile>=VIDEO_GL_MAX_TEX) { printf("TileNumber=%i (%i). MAX=%i\n",tile->tile,TileNumber,VIDEO_GL_MAX_TEX); exit(0); }
//printf("TileNumber=%i (%i/%i) %s\n",t,TileNumber, BG.Buffered==IPPU.TileCached[TILE_4BIT]?1:BG.Buffered==IPPU.TileCached[TILE_8BIT]?2:0, tile_converted?"converted":"cached");
//videogl_z=VIDEO_GL_TILE_Z_INIT+((256-GFX.Z2)*VIDEO_GL_TILE_Z_INC);
//videogl_z=VIDEO_GL_TILE_Z_INIT+(GFX.Z2*VIDEO_GL_TILE_Z_INC);
//printf("DrawTile16hw Tile=%i, %i,%i (Z=%i,%i) %s\n",tile->tile,tile->x,tile->y,GFX.Z1,GFX.Z2,tile_converted?"converted":videogl_cache_reseted?"reseted":videogl_cached[tile->tile]?"cached":"empty");
//printf(" z=%f, GFX.Z1=%i GFX.Z2=%i (%i)\n",videogl_z,GFX.Z1,GFX.Z2,*((int8 *)(GFX.DB + Offset + 4)));
//static unsigned total=0;
//static unsigned nocacheados=0;
//total++;
	if (tile_converted || !videogl_cached[tile->tile] /*|| videogl_cache_reseted*/)
	{
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
		if (BG.BitShift<=4) {
			videogl_selective_cache[(tile->pal*VIDEO_GL_SELECTIVE_MAX_CACHE)+videogl_selective[tile->pal]]=tile->tile;
			videogl_selective[tile->pal]++;
			if (videogl_selective[tile->pal]>VIDEO_GL_SELECTIVE_MAX_CACHE) videogl_cache_to_reseted=4;
		}
#endif
//nocacheados++;
#if !defined(DREAMCAST) || !defined(USE_VIDEOGL_PVRTXR)
		unsigned short *tex=get_free_slot(tile->tile);
#else
		unsigned short *pvr_tex=get_free_slot(tile->tile);
		unsigned short *tex=(unsigned short *)videogl_dc_buffer;
#endif
		unsigned char *s=(unsigned char *)pCache;//+ StartLine;
#ifndef DREAMCAST
		unsigned short *d=tex;
		for(int i=0;i<8*8;i++) {
			register unsigned char ds=*s++;
//if (ds>16) printf("MAL col=%i\n",ds);
			*d++=ds?GFX.ScreenColors[ds]:0;
		}
#else
		{
		unsigned short back_zero=GFX.ScreenColors[0];
		GFX.ScreenColors[0]=0;
#ifndef USE_MMU
		unsigned int *d = (unsigned int *)(void *)
			(0xe0000000 | (((unsigned long)tex) & 0x03ffffe0));
		QACR0 = ((((unsigned int)tex)>>26)<<2)&0x1c;
		QACR1 = ((((unsigned int)tex)>>26)<<2)&0x1c;
#else
		unsigned int *d = (unsigned int *)(void *)
			((0xe0000000 | (((unsigned long)tex) & 0x03ffffe0))|((((unsigned)tex)>>3)&0x3800000));
#endif
		unsigned n=(8*8*2)>>5;
		while(n--) {
			asm("pref @%0" : : "r" (s + 8));
			{register unsigned char ds0=*s++, ds1=*s++; d[0]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[1]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[2]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[3]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[4]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[5]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[6]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			{register unsigned char ds0=*s++, ds1=*s++; d[7]=(((unsigned)GFX.ScreenColors[ds1])<<16)|((unsigned)GFX.ScreenColors[ds0]); }
			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		d = (unsigned int *)0xe0000000;
		d[0] = d[8] = 0;
		GFX.ScreenColors[0]=back_zero;
		}
#endif
//} else {
//for(i=0;i<8*8;i++) { unsigned char ds=*s++; unsigned short dd=*d++;
//if (dd!=GFX.ScreenColors[ds]) { printf("MAL CACHEADO %i = %i(%i/%i)\n",i,dd,GFX.ScreenColors[ds],ds); exit(0); }
//}}
//		glBindTexture(GL_TEXTURE_2D, videogl_tex[tile->tile]);
//		loadTextureParams(SNES4ALL_FILTER_NONE);
//		glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0,
//			GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, tex);
#if defined(DREAMCAST) && defined(USE_VIDEOGL_PVRTXR)
		pvr_txr_load(tex,pvr_tex,8*8*2);
#endif
		videogl_cached[tile->tile]=1;
	}
//printf("Cached %i/%i (%.2f%%)\n",total-nocacheados,total, (((double)(total-nocacheados))*100.0)/((double)total));
}

void DrawClippedTile16hw (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset+StartPixel,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->clipx=StartPixel;
#else
	tile->clipx=StartPixel>7?7:StartPixel;
#endif
}

#if 0
void DrawTile16x2hw (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
}

void DrawClippedTile16x2hw (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16x2x2hw (uint32 Tile, uint32 Offset, uint32 StartLine,
		     uint32 LineCount)
{
}

void DrawClippedTile16x2x2hw (uint32 Tile, uint32 Offset,
			    uint32 StartPixel, uint32 Width,
			    uint32 StartLine, uint32 LineCount)
{
}
#endif

void DrawTile16Addhw (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawTile16Addhw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=1;
#else
	tile->clipx=8;
#endif
}

void DrawClippedTile16Addhw (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawClippedTile16Addhw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset+StartPixel,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->clipx=StartPixel;
	tile->add=1;
#else
	tile->clipx=8|(StartPixel>7?7:StartPixel);
#endif
}

void DrawTile16Add1_2hw (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawTile16Add1_2hw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=2;
#else
	tile->clipx=8;
#endif
}


void DrawClippedTile16Add1_2hw (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawClippedTile16Add1_2hw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset+StartPixel,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=2;
	tile->clipx=StartPixel;
#else
	tile->clipx=8|(StartPixel>7?7:StartPixel);
#endif
}

void DrawTile16Subhw (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawTile16Subhw");
#endif
	DrawTile16hw(Tile,Offset,StartLine,LineCount);
}


void DrawClippedTile16Subhw (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawClippedTile16Subhw");
#endif
	DrawClippedTile16hw(Tile, Offset, StartPixel, Width, StartLine,LineCount);
}


void DrawTile16Sub1_2hw (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawTile16Sub1_2hw");
#endif
	DrawTile16hw(Tile,Offset,StartLine,LineCount);
}

void DrawClippedTile16Sub1_2hw (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawClippedTile16Sub1_2hw");
#endif
	DrawClippedTile16hw(Tile, Offset, StartPixel, Width, StartLine,LineCount);
}

void DrawTile16FixedAdd1_2hw (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawTile16FixedAdd1_2hw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=3;
#else
	tile->clipx=8;
#endif
}


void DrawClippedTile16FixedAdd1_2hw (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawClippedTile16FixedAdd1_2hw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawTile16hw(Tile,Offset+StartPixel,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=3;
	tile->clipx=StartPixel;
#else
	tile->clipx=8|(StartPixel>7?7:StartPixel);
#endif
}


void DrawTile16FixedSub1_2hw (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawTile16FixedSub1_2hw");
#endif
	DrawTile16hw(Tile,Offset,StartLine,LineCount);
}


void DrawClippedTile16FixedSub1_2hw (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawClippedTile16FixedSub1_2hw");
#endif
	DrawClippedTile16hw(Tile, Offset, StartPixel, Width, StartLine,LineCount);
}


#if 0
void DrawLargePixelhw (uint32 Tile, uint32 Offset,
		     uint32 StartPixel, uint32 Pixels,
		     uint32 StartLine, uint32 LineCount)
{
//puts("DrawLargePixelhw");
}
#endif

void DrawLargePixel16hw (uint32 Tile, uint32 Offset,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount)
{
	int c=Pixels; //StartPixel;
	int j=LineCount;
	if (c>7) c=7;
	if (j>31) j=31;
	while(j>0){
		if (j>=8) j-=8;
		else j=0;
		int i=Pixels;
		if (i>31) i=31;
		while(i>0){
			videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
			if (i>=8) i-=8;
			else i=0;
			DrawTile16hw(Tile,Offset+i+(j<<8),StartLine,LineCount);
#if defined(USE_VIDEOGL_EXTEND_TILELIST) && !defined(USE_VGL_TABLE)
			tile->mosaic=c;
#else
			tile->flip|=c<<2;
#endif
		};	
	};
//{ extern int pintado; pintado=1;}
//printf("Tile %i Pixels %i Start=%i, Offset=%i, StartLine=%i, LineCount=%i\n",tile->tile,Pixels,StartPixel,Offset,StartLine,LineCount);
}

void DrawLargePixel16Addhw (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawLargePixel16Addhw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawLargePixel16hw(Tile,Offset,StartPixel,Pixels,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=1;
#else
	tile->clipx=8;
#endif
}


void DrawLargePixel16Add1_2hw (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawLargePixel16Add1_2hw");
#endif
	videogl_tile_list_t *tile=&videogl_tile[videogl_tiles];
	DrawLargePixel16hw(Tile,Offset,StartPixel,Pixels,StartLine,LineCount);
#ifdef USE_VIDEOGL_EXTEND_TILELIST
	tile->add=2;
#else
	tile->clipx=8;
#endif
}

void DrawLargePixel16Subhw (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawLargePixel16Subhw");
#endif
	DrawLargePixel16hw(Tile,Offset,StartPixel,Pixels,StartLine,LineCount);
}

void DrawLargePixel16Sub1_2hw (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
#ifdef SHOW_TODO
puts("DrawLargePixel16Sub1_2hw");
#endif
	DrawLargePixel16hw(Tile,Offset,StartPixel,Pixels,StartLine,LineCount);
}

#endif
