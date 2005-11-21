/*
=================================================================
Copyright (C) 2005 Torben "ntba2" Koenke

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

/*
# myPS2
# Author: ntba2
# Date: 10-7-2005
#
# File: Small Graphics Library Header.
#		This code is based on PS2DEV's Graph library and 
#		Tony Saveski's Tutorials.
#
*/

#ifndef _GR_H
#define _GR_H

#include <dma.h>
#include <packet.h>
#include <gr_regs.h>
#include <stdio.h>
#include <kernel.h>		// ResetEE, SetGsCrt
#include <string.h>		// memset
#include <floatlib.h>	// powf

#define RGB(r,g,b)		((0x80 << 24) | (b << 16) | (g << 8) | (r))
#define RGBA(r,g,b,a)	((a << 24) | (b << 16) | (g << 8) | (r))

#ifndef M_PI
#define M_PI			3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

typedef struct {			// for 2d primitive drawing
	int x;
	int y;
} point_t;

typedef struct {			// This is returned by GR_LoadImage and acts
	int width;				// as a handle which subsequently can be passed
	int height;				// to GR_DrawImage.
	int address;
	int psm;				// psm of image, determines bbp (either 32 RGBA or 24 RGB)
} IMG_HANDLE;

typedef struct {
	int		grInit;			// 1 if we are initialized
	PACKET	grPacket;		// DMA Packet
	int		grPSM;			// Pixel Storage Mode. Defines how colors are stored in memory
	int		grMode;			// Selected Graphics Mode
	int		grBPP;			// Bits Per Pixel

	int		grFrameBuf[2];	// Address of Frame Buffer(s) in GS Memory
	int		grTexBufPtr;	// Points to the next free memory location in the Texture Buffer
	
	int		grDBuffering;	// 1 if double buffering is on
	int		grDrawFrame;	// All drawing operations go here
	int		grDisplayFrame;	// Frame that is currently being displayed on the screen

	int		grBkgColor;		// Background Color
	int		grDrawColor;	// Drawing Color

	IMG_HANDLE grFontMap;
	int		*grFontCoords;
	int		grFontColor;

	int		grBlendMode;	// Specifies blend function for alpha blending
	int		grAlpha;		// alpha value for constant alpha blending

	int		grOffset_x;		// for horizontal screen adjustment
	int		grOffset_y;		// for vertical screen adjustment

} grSettings_t;

typedef struct {
	int width;
	int height;
	int mode;
	int interlace;
	int size;
	u64 display;

} GR_MODE;

#define GR_MODE_NTSC			0x00	//  640 x  448
#define GR_MODE_PAL				0x01	//  640 x  512
#define GR_MODE_HDTV_480P		0x02	//  640 x  480
#define GR_MODE_HDTV_720P		0x03	// 1280 x  720
#define GR_MODE_HDTV_1080I		0x04	// 1920 x 1080
#define GR_MODE_VGA_640			0x05	//  640 x  480
#define GR_MODE_VGA_800			0x06	//  800 x  600
#define GR_MODE_VGA_1024		0x07	// 1024 x  768
#define GR_MODE_VGA_1280		0x08	// 1280 x 1024
#define GR_MODE_AUTO			0xFF	// Automatic NTSC or PAL mode setting

#define GR_PSM_32				0x00	// 32 bits per pixel.
#define GR_PSM_24				0x01	// 24 bits per pixel.
#define GR_PSM_16				0x02	// 16 bits per pixel.
#define GR_PSM_8				0x13	// 8 bits per pixel, palettized.
#define GR_PSM_4				0x14	// 4 bits per pixel, palettized.
#define GR_PSM_8H				0x1B	// 8 bits per pixel, palettized.
#define GR_PSM_4HH				0x24	// 4 bits per pixel, palettized.
#define GR_PSM_4HL				0x2C	// 4 bits per pixel, palettized.

#define GR_SINGLE_BUFFERING		0x00	// Single Buffering
#define GR_DOUBLE_BUFFERING		0x01	// Double Buffering

#define GR_PRIM_POINT			0x00
#define GR_PRIM_LINE			0x01
#define GR_PRIM_LINESTRIP		0x02
#define GR_PRIM_TRIANGLE		0x03
#define GR_PRIM_TRIANGLESTRIP	0x04
#define GR_PRIM_TRIANGLEFAN		0x05
#define GR_PRIM_SPRITE			0x06

#define GR_TEXFUNC_MODULATE		0x00	// See GSUSER_E.PDF, P.61
#define GR_TEXFUNC_DECAL		0x01
#define GR_TEXFUNC_HIGHLIGHT	0x02
#define GR_TEXFUNC_HIGHLIGHT2	0x03

#define GR_XDIR_EE_GS			0x00
#define GR_XDIR_GS_EE			0x01
#define GR_XDIR_GS_GS			0x02
#define GR_XDIR_DEACTIVATE		0x03

// some pre defined font sizes
#define GR_FONT_SMALL			1
#define GR_FONT_BIG				2
#define GR_FONT_HUGE			3

// Blend functions
typedef enum {
	GR_BLEND_NONE,			// no alpha blending is performed
	GR_BLEND_CONSTANT,		// use a constant alpha value
	GR_BLEND_SRC_ALPHA		// use source alpha value from image
} grBlendMode_e;

// function ceclarations
int GR_Init( int grMode, int grPSM, int grDB );
void GR_ShutDown( void );
int GR_SetFrameBuffers( int startAddress );
int GR_SetDisplayBuffer( int address );
int GR_SetTextureBuffer( int address );
int GR_GetTextureBuffer( void );
void GR_WaitForVSync( void );
int GR_SetDisplayFrame( int frame );
int GR_SetDrawFrame( int frame );
int GR_SwapBuffers( void );
void GR_SetBkgColor( int rgb );
void GR_SetDrawColor( int rgb );
void GR_ClearScreen( void );
void GR_AdjustFrom640( int *x, int *y, int *w, int *h );
void GR_SetScreenAdjust( int offset_x, int offset_y );
int GR_PSM2BPP( int psm );
int GR_LoadImage( IMG_HANDLE *pHandle, void *pImageData, int imageWidth, int imageHeight, int psm );
void GR_DrawImage( int x, int y, int width, int height, const IMG_HANDLE *pHandle );
void GR_DrawFillRect( int x, int y, int width, int height );
void GR_DrawRect( int x, int y, int width, int height );
void GR_DrawTriangle( point_t p1, point_t p2, point_t p3 );
void GR_DrawDot( int x, int y, int radius );
int GR_SetFont( IMG_HANDLE *pFontMap, int *texCoords );
int GR_SetFontColor( int rgb );
int GR_GetStringWidth( const char *pStr, int fontSize );
void GR_DrawTextExt( int x, int y, const char *s, int fontSize );
float GR_SetAlpha( float alpha );
int GR_SetBlendMode( grBlendMode_e mode );
void GR_DrawRoundRect( int x, int y, int width, int height );

#endif
