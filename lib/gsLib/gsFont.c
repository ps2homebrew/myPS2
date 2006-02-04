/*
=================================================================
Copyright (C) 2005-2006 Torben "ntba2" Koenke

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
=================================================================
*/

#include "gsLib.h"

GSFONT *gsLib_font_create( u8 type, u8 *png_file, u32 png_size, u8 *dat_file, u32 dat_size )
{
	GSFONT				*gsFont;
	u8					*rgba, CharWidth, CharHeight;
	int					a,b,c,d,i;
	u32					StartX, StartY;
	pngData				*pPng;
	u32					nImageWidth, nImageHeight;

	if( !(pPng = pngOpenRAW( png_file, png_size )) )
		return NULL;

	if( pPng->bpp != 32 )
		return NULL;

	if( !(rgba = malloc( pPng->width * pPng->height * (pPng->bpp >> 3) )) )
		return NULL;

	if( pngReadImage( pPng, rgba ) < 0 )
		return NULL;

	nImageWidth		= pPng->width;
	nImageHeight	= pPng->height;

	pngClose(pPng);

	CharWidth	= nImageWidth  / 16;
	CharHeight	= nImageHeight / 16;

	gsFont				= calloc(1, sizeof(GSFONT) );	
	gsFont->Type		= type;
	gsFont->CharGrid	= CharWidth;

	for( a = 0; a < 16; a++ )
	{
		for( b = 0; b < 16; b++ )
		{
			c		= a * 16 + b;
			StartX	= CharWidth  * b * 4;
			StartY	= CharHeight * a * nImageWidth * 4;

			gsFont->Chars[c].Data = memalign( 128, CharWidth * CharHeight * 4 );

			for( d = 0; d < CharHeight; d++ )
			{
				memcpy( &gsFont->Chars[c].Data[ d * CharWidth * 4 ],
						&rgba[ StartY + d * nImageWidth * 4 + StartX],
						CharWidth * 4 );
			}
		}
	}

	free(rgba);

	// Parse Widths from DAT
	c = 0;

	for( i = 0; i < 256; i++ )
	{
		gsFont->Chars[i].Width	= *dat_file;
		gsFont->Chars[i].Height	= CharHeight;

		dat_file += 2;
	}

	return gsFont;
}

int gsLib_font_destroy( GSFONT *gsFont )
{
	int i;

	for( i = 0; i < 256; i++ )
		free( gsFont->Chars[i].Data );

	free( gsFont );

	return 1;
}

void gsLib_font_print( GSFONT *gsFont, u32 x, u32 y, u64 color, const char *string )
{
	int l, i, h, cx, cy;
	int c;

	cx	= x;
	cy	= y;
	l	= strlen(string);

	if( gsFont->Type == GSLIB_FTYPE_BFT )
	{
		for( i = 0; i < l; i++ )
		{
			c = (unsigned char) string[i];

			if( c == '\n' )
			{
				h	= gsFont->Chars[0].Height + 1;
				cx	= x;
				cy	= cy + h;
			}
			else
			{
				cx += gsLib_font_print_char( gsFont, cx, cy, color, c );
			}
		}
	}
}

int gsLib_font_print_char( GSFONT *gsFont, u32 x, u32 y, u64 color, u32 c )
{
	GSTEXTURE *gsTexture;
	int i;

	int			size;
	u32			vram;
	static u32	last_vram;

	i = (unsigned char) c;

	//
	size	= gsLib_texture_size( gsFont->CharGrid, gsFont->CharGrid, GS_PSM_CT32 );
	vram	= gsLib_vram_alloc( size );

	if( vram == last_vram )
	{
		last_vram	= vram;
		vram		= gsLib_vram_alloc( size );

		gsLib_vram_free(last_vram);
	}

	last_vram = vram;

	gsTexture = malloc( sizeof(GSTEXTURE) );
	
	gsTexture->Vram		= vram;
	gsTexture->Width	= gsFont->CharGrid;
	gsTexture->Height	= gsFont->CharGrid;
	gsTexture->PSM		= GS_PSM_CT32;

	gsLib_texture_send( (void*) gsFont->Chars[i].Data, gsFont->CharGrid, gsFont->CharGrid,
						vram, GS_PSM_CT32 );

/*	gsTexture = gsLib_texture_raw(	gsFont->CharGrid, gsFont->CharGrid,
									GS_PSM_CT32, gsFont->Chars[i].Data );
*/
	gsLib_prim_sprite_texture(	gsTexture, x, y,
								gsFont->Chars[i].Width + 1, gsFont->Chars[i].Height + 1,
								0, 0,
								gsFont->Chars[i].Width + 1, gsFont->Chars[i].Height + 1,
								color );

	gsLib_texture_free( gsTexture );

	return gsFont->Chars[i].Width + 1;
}

int gsLib_font_width( GSFONT *gsFont, const char *string )
{
	int l, i, ret;
	int c;

	l	= strlen(string);
	ret	= 0;

	if( gsFont->Type == GSLIB_FTYPE_BFT )
	{
		for( i = 0; i < l; i++ )
		{
			c = (unsigned char) string[i];

			ret += gsFont->Chars[c].Width + 1;
		}
	}

	return ret;
}

int gsLib_font_height( GSFONT *gsFont )
{
	if( gsFont->Type == GSLIB_FTYPE_BFT )
		return gsFont->CharGrid;

	return 0;
}
