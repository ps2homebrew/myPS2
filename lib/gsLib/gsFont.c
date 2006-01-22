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

GSFONT *gsLib_font_create( u8 type, u8 *data )
{
	unsigned short width, height;
	GSFONT	*gsFont;
	u8		*rgba;
	u32		*charWidths;
	int		i;
	int		x0, y0, x1, y1;

	gsFont = calloc(1, sizeof(GSFONT));

	gsFont->Type	= type;

	if( type == GSLIB_FTYPE_BFT )
	{
		if( *data++ != 'B' || *data++ != 'F' || *data++ != 'T' ||
			*data++ != '0' )
			return NULL;

		width	= (data[1] << 8) + data[0];
		height	= (width / 16) * 6;
		data += 2;

		if(width != 128 && width != 256 && width != 512)
			return NULL;

		rgba = memalign( 128, width * height * 4 );
		for( i = 0; i < width * height; i++ )
		{
			rgba[i*4 + 0] = 0xff;
			rgba[i*4 + 1] = 0xff;
			rgba[i*4 + 2] = 0xff;
			rgba[i*4 + 3] = *data++;
		}

		gsFont->Texture = gsLib_texture_raw( width, height, GS_PSM_CT32, rgba );
		if( !gsFont->Texture )
			return NULL;

		free(rgba);
		charWidths = malloc( sizeof(u32) * 96 );
		memcpy( charWidths, data, sizeof(u32) * 96 );
		gsFont->TexCoords = calloc(1, sizeof(u32) * 4 * 128 );

		x0 = x1 = y1 = 0;
		y0 = -(width / 16);

		for( i = 0; i < 96; i++ )
		{
			x0 %= width;
			x1 = x0 + charWidths[i];
			if( (i % 16) == 0 )
			{
				y0 += (width / 16);
				y1 = y0 + (width / 16) - 1;
			}

			gsFont->TexCoords[ 32 * 4 + i * 4 + 0 ] = x0;
			gsFont->TexCoords[ 32 * 4 + i * 4 + 1 ] = y0;
			gsFont->TexCoords[ 32 * 4 + i * 4 + 2 ] = x1;
			gsFont->TexCoords[ 32 * 4 + i * 4 + 3 ] = y1;

			x0 += (width / 16);
		}

		free(charWidths);
	}

	return gsFont;
}

int gsLib_font_destroy( GSFONT *gsFont )
{
	gsLib_texture_free(gsFont->Texture);
	free(gsFont->TexCoords);
	free(gsFont);
	return 1;
}

void gsLib_font_print( GSFONT *gsFont, u32 x, u32 y, u64 color, const char *string )
{
	u32 *tc;
	int l, i, x0, x1, y0, y1,w, h, cx, cy;
	char c;

	cx	= x;
	cy	= y;
	l	= strlen(string);

	if( gsFont->Type == GSLIB_FTYPE_BFT )
	{
		for( i = 0; i < l; i++ )
		{
			c = string[i];
			if( c == '\n' )
			{
				h	=	gsFont->TexCoords[ '0' * 4 + 3 ] -
						gsFont->TexCoords[ '0' * 4 + 1 ] + 1;

				cx	= x;
				cy	= cy + h;
			}
			else
			{
				tc	= &gsFont->TexCoords[ c * 4 ];
				x0	= *tc++;
				y0	= *tc++;
				x1	= *tc++;
				y1	= *tc++;
				w	= (x1 - x0 +1);
				h	= (y1 - y0 +1);

				gsLib_prim_sprite_texture(	gsFont->Texture, cx, cy, w, h, x0, y0,
											x1 + 1, y1 + 1, color );

				cx += w;
			}
		}
	}
}

int gsLib_font_print_char( GSFONT *gsFont, u32 x, u32 y, u64 color, char c )
{
	u32 *tc;
	int x0, x1, y0, y1, w, h;

	w = 0;
	if( gsFont->Type == GSLIB_FTYPE_BFT )
	{
		tc	= &gsFont->TexCoords[ c * 4 ];
		x0	= *tc++;
		y0	= *tc++;
		x1	= *tc++;
		y1	= *tc++;
		w	= (x1 - x0 +1);
		h	= (y1 - y0 +1);

		gsLib_prim_sprite_texture(	gsFont->Texture, x, y, w, h, x0, y0,
									x1 + 1, y1 + 1, color );
	}

	return w;
}

int gsLib_font_width( GSFONT *gsFont, const char *string )
{
	u32 *tc;
	int l, i, x0, x1, y0, y1, w, ret;
	char c;

	l	= strlen(string);
	ret	= 0;

	if( gsFont->Type == GSLIB_FTYPE_BFT )
	{
		for( i = 0; i < l; i++ )
		{
			c = string[i];

			tc	= &gsFont->TexCoords[ c * 4 ];
			x0	= *tc++;
			y0	= *tc++;
			x1	= *tc++;
			y1	= *tc++;
			w	= x1 - x0 + 1;

			ret += w;
		}
	}

	return ret;
}

int gsLib_font_height( GSFONT *gsFont )
{
	int h;

	if( gsFont->Type == GSLIB_FTYPE_BFT )
	{
		h = gsFont->TexCoords[ '0' * 4 + 3 ] - gsFont->TexCoords[ '0' * 4 + 1 ] + 1;
		return h;
	}

	return 0;
}
