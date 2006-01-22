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

#define GSLIB_FTYPE_BFT		0x00
#define GSLIB_FTYPE_TTF		0x01

typedef struct
{
	u8 Type;
	GSTEXTURE *Texture;
	u32 *TexCoords;
	u32 Attributes;
} GSFONT;

GSFONT	*gsLib_font_create( u8 type, u8 *data );
int		gsLib_font_destroy( GSFONT *gsFont );
void	gsLib_font_print( GSFONT *gsFont, u32 x, u32 y, u64 color, const char *string );
int		gsLib_font_width( GSFONT *gsFont, const char *string );
int		gsLib_font_height( GSFONT *gsFont );
int		gsLib_font_print_char( GSFONT *gsFont, u32 x, u32 y, u64 color, char c );
