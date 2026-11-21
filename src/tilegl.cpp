#ifdef USE_GL

#include "snes9x.h"

#include "snes4all.h"

#include "memmap.h"
#include "ppu.h"
#include "display.h"
#include "gfx.h"
#include "tile.h"

GLint videogl_tex[0x1000];

unsigned short videogl_cache[0x1000 * 8 * 8];
unsigned char marca[0x1000];

void DrawTile16 (uint32 Tile, uint32 Offset,
		 uint32 StartLine, uint32 LineCount)
{
//printf("DrawTile16 Tile=%i, Offset=%i, StartLine=%i, LineCount=%i\n",Tile,Offset,StartLine,LineCount);
	TILE_PREAMBLE

	double x0 = Offset % SNES4ALL_SCREEN_PITCH;
	double y0 = (Offset / SNES4ALL_SCREEN_PITCH)*2;
	double x1 = x0 + 8.0;
	double y1 = y0 + 8.0;

	double tx0, ty0, tx1, ty1;

	if (!(Tile & (V_FLIP | H_FLIP)))
	{
		tx0 = 0.0;
		ty0 = 0.0;
		tx1 = 1.0;
		ty1 = 1.0;
	}
	else
	if (!(Tile & V_FLIP))
	{
		tx0 = 1.0;
		ty0 = 0.0;
		tx1 = 0.0;
		ty1 = 1.0;
	}
	else
	if (Tile & H_FLIP)
	{
		tx0 = 1.0;
		ty0 = 1.0;
		tx1 = 0.0;
		ty1 = 0.0;
	}
	else
	{
		tx0 = 0.0;
		ty0 = 1.0;
		tx1 = 1.0;
		ty1 = 0.0;
	}

	unsigned t=TileNumber; //Tile & 0xfff;
if (TileNumber>0xfff)
{
printf("TileNumber=%X\n",TileNumber);
exit(0);
}
if (marca[t])
{
	puts("REPETIDO");
//return;
//	exit(0);
}
printf("DrawTile16 Tile=%i, %.2f,%.2f -> %.2f,%.2f\n",t,x0,y0,x1,y1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, videogl_tex[t]);
//	glPixelStorei(GL_UNPACK_ROW_LENGTH, 8);
	loadTextureParams(PSX4ALL_FILTER_NONE);
//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//glEnable(GL_BLEND);

if (!marca[t])
	{
		int i;
		unsigned short *tex=(unsigned short *)&videogl_cache[t*8*8];
		unsigned char *s=(unsigned char *)pCache; // + StartLine;
		unsigned short *d=tex;
		for(i=0;i<8*8;i++)
			*d++=GFX.ScreenColors[*s++];
/*
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0,
			GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, tex);
*/
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 8, 8, 0,
			GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tex);
	}
marca[t]=1;
	glBegin(GL_QUADS);
		glTexCoord2f(tx0,ty0);
		glVertex3f(x0,y0,videogl_z);
		glTexCoord2f(tx1,ty0);
		glVertex3f(x1,y0,videogl_z);
		glTexCoord2f(tx1,ty1);
		glVertex3f(x1,y1,videogl_z);
		glTexCoord2f(tx0,ty1);
		glVertex3f(x0,y1,videogl_z);
	glEnd();
	videogl_z+=TILE_Z_INC;
}

void DrawClippedTile16 (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
}

void DrawClippedTile16x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16x2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		     uint32 LineCount)
{
}

void DrawClippedTile16x2x2 (uint32 Tile, uint32 Offset,
			    uint32 StartPixel, uint32 Width,
			    uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16Add (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
}

void DrawClippedTile16Add (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16Add1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
}


void DrawClippedTile16Add1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16Sub (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
}


void DrawClippedTile16Sub (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
}


void DrawTile16Sub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
}

void DrawClippedTile16Sub1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
}

void DrawTile16FixedAdd1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
}


void DrawClippedTile16FixedAdd1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
}


void DrawTile16FixedSub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
}


void DrawClippedTile16FixedSub1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
}


void DrawLargePixel (uint32 Tile, uint32 Offset,
		     uint32 StartPixel, uint32 Pixels,
		     uint32 StartLine, uint32 LineCount)
{
}

void DrawLargePixel16 (uint32 Tile, uint32 Offset,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount)
{
}

void DrawLargePixel16Add (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
}


void DrawLargePixel16Add1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
}

void DrawLargePixel16Sub (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
}

void DrawLargePixel16Sub1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
}


#ifdef USE_GLIDE
void DrawTile3dfx (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
    TILE_PREAMBLE

    float x = Offset % SNES4ALL_SCREEN_PITCH;
    float y = Offset / SNES4ALL_SCREEN_PITCH;

    Glide.sq [0].x = Glide.x_offset + x * Glide.x_scale;
    Glide.sq [0].y = Glide.y_offset + y * Glide.y_scale;
    Glide.sq [1].x = Glide.x_offset + (x + 8.0) * Glide.x_scale;
    Glide.sq [1].y = Glide.y_offset + y * Glide.y_scale;
    Glide.sq [2].x = Glide.x_offset + (x + 8.0) * Glide.x_scale;
    Glide.sq [2].y = Glide.y_offset + (y + LineCount) * Glide.y_scale;
    Glide.sq [3].x = Glide.x_offset + x * Glide.x_scale;
    Glide.sq [3].y = Glide.y_offset + (y + LineCount) * Glide.y_scale;

    if (!(Tile & (V_FLIP | H_FLIP)))
    {
	// Normal
	Glide.sq [0].tmuvtx [0].sow = 0.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine;
	Glide.sq [1].tmuvtx [0].sow = 8.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine;
	Glide.sq [2].tmuvtx [0].sow = 8.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [3].tmuvtx [0].sow = 0.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine + LineCount;
    }
    else
    if (!(Tile & V_FLIP))
    {
	// Flipped
	Glide.sq [0].tmuvtx [0].sow = 8.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine;
	Glide.sq [1].tmuvtx [0].sow = 0.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine;
	Glide.sq [2].tmuvtx [0].sow = 0.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [3].tmuvtx [0].sow = 8.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine + LineCount;
    }
    else
    if (Tile & H_FLIP)
    {
	// Horizontal and vertical flip
	Glide.sq [0].tmuvtx [0].sow = 8.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [1].tmuvtx [0].sow = 0.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [2].tmuvtx [0].sow = 0.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine;
	Glide.sq [3].tmuvtx [0].sow = 8.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine;
    }
    else
    {
	// Vertical flip only
	Glide.sq [0].tmuvtx [0].sow = 0.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [1].tmuvtx [0].sow = 8.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [2].tmuvtx [0].sow = 8.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine;
	Glide.sq [3].tmuvtx [0].sow = 0.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine;
    }
    grTexDownloadMipMapLevel (GR_TMU0, Glide.texture_mem_start,
			      GR_LOD_8, GR_LOD_8, GR_ASPECT_1x1,
			      GR_TEXFMT_RGB_565,
			      GR_MIPMAPLEVELMASK_BOTH,
			      (void *) pCache);
    grTexSource (GR_TMU0, Glide.texture_mem_start,
		 GR_MIPMAPLEVELMASK_BOTH, &Glide.texture);
    grDrawTriangle (&Glide.sq [0], &Glide.sq [3], &Glide.sq [2]);
    grDrawTriangle (&Glide.sq [0], &Glide.sq [1], &Glide.sq [2]);
}

void DrawClippedTile3dfx (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4)
}
#endif
#endif
