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

typedef struct
{
	u32		Width;
	u32		Height;
	u32		PSM;
	u32		Vram;
} GSTEXTURE;

u32			gsLib_texture_size( int width, int height, int psm );
void		gsLib_texture_send( u32 *mem, int width, int height, u32 tbp, u32 psm );
GSTEXTURE*	gsLib_texture_raw( u32 width, u32 height, u32 psm, void *ee_mem );
void		gsLib_texture_free( GSTEXTURE *texture );
void		gsLib_prim_sprite_texture(	const GSTEXTURE *Texture, int x, int y, int w, int h,
										int u0, int v0, int u1, int v1, u64 color );

