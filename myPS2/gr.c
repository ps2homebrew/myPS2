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
# File: Small Graphics Library.
#		This code is based on PS2DEV's Graph library and 
#		Tony "Dreamtime" Saveski's Tutorials.
#
*/

#include <tamtypes.h>

#include <gr.h>

grSettings_t grSettings;

GR_MODE grModes[9] =
{
	// GR_MODE_NTSC
	{  640,  448, 0x02, 1,  286720, GS_SET_DISPLAY(632, 50, 3, 0, 2559,  447) }, 

	// GR_MODE_PAL	
	{  640,  512, 0x03, 1,  286720, GS_SET_DISPLAY(652, 72, 3, 0, 2559,  511) },
 
	// GR_MODE_HDTV_480P
	{  720,  480, 0x50, 0,  393216, GS_SET_DISPLAY(232, 35, 1, 0, 1439,  479) },

	// GR_MODE_HDTV_720P
	{ 1280,  720, 0x51, 0,  983040, GS_SET_DISPLAY(302, 24, 0, 0, 1279,  719) }, 
	
	// GR_MODE_HDTV_1080I
	{ 1920, 1080, 0x52, 1, 2088960, GS_SET_DISPLAY(238, 40, 0, 0, 1919, 1079) }, 

	// GR_MODE_VGA_640
	{  640,  480, 0x1A, 0,  327680, GS_SET_DISPLAY(276, 34, 1, 0, 1279,  479) }, 

	// GR_MODE_VGA_800	
	{  800,  600, 0x2B, 0,  532480, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) }, 

	// GR_MODE_VGA_1024	
	{ 1024,  768, 0x3B, 0,  786432, GS_SET_DISPLAY(580, 34, 1, 0, 2047,  767) }, 
	
	// GR_MODE_VGA_1280
	{ 1280, 1024, 0x4A, 0, 1310720, GS_SET_DISPLAY(348, 40, 0, 0, 1279, 1023) }, 
};

//
// GR_Init - Initializes the CRT Controller and GS
//

int GR_Init( int grMode, int grPSM, int grDB )
{
	// Reset the GIF on the EE
	ResetEE(0x08);

	// Reset and flush the gs.
	GS_REG_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0);

	// Reset grSettings
	memset( &grSettings, 0, sizeof(grSettings) );

	if( packet_allocate( &grSettings.grPacket, 1024 ) < 0 )
		return 0;

	if( grMode == GR_MODE_AUTO )
	{
		if( *(volatile char *)(0x1FC7FF52) == 'E' )
			grMode = GR_MODE_PAL;
		else
			grMode = GR_MODE_NTSC;
	}

	// Set the mode
	SetGsCrt( grModes[ grMode ].interlace, grModes[ grMode ].mode, 0 );

	// Setup CRTC Display Registers
	GS_REG_PMODE	= GS_SET_PMODE(1, 1, 1, 0, 0, 0xFF); // Turn on Read Circuit 1 AND 2
														 // Use ALP Register for Alpha
														 // Blending.
													     // FIXME: SLBG Register?
	GS_REG_DISPLAY1 = grModes[ grMode ].display;
	GS_REG_DISPLAY2 = grModes[ grMode ].display;

	grSettings.grPSM = grPSM;
	grSettings.grMode = grMode;
	grSettings.grDBuffering = grDB;
	grSettings.grInit = 1;

	switch(grPSM) {
		case GR_PSM_32:
			grSettings.grBPP = 32;
			break;

		case GR_PSM_24:
			grSettings.grBPP = 24;
			break;

		case GR_PSM_16:
			grSettings.grBPP = 16;
			break;
	}

#ifdef _DEBUG
	printf("GR_Init\n");
	printf("\tMode Width : %i\n", grModes[ grMode ].width);
	printf("\tMode Height : %i\n", grModes[ grMode ].height);
	printf("\tBits per Pixel : %i\n", grSettings.grBPP);
#endif

	return 1;
}

//
// GR_Shutdown - performs clean up work
//

void GR_ShutDown( void )
{
	if( !grSettings.grInit )
		return;

	// free DMA packet
	packet_free( &grSettings.grPacket );

	grSettings.grInit = 0;
}

//
// GR_SetFrameBuffers - Sets up the frame buffer(s).
// Returns next free memory address after frame buffers on success.
// If any error occurs 0 is returned.
//

int GR_SetFrameBuffers( int startAddress )
{
	int frBufSize;

	if( !grSettings.grInit )
		return 0;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return 0;

	// set up the first frame buffer
	grSettings.grFrameBuf[0] = startAddress;

	// frame buffer size depends on width, height and color depth
	// of the selected graphics mode
	frBufSize = grModes[ grSettings.grMode ].width * grModes[ grSettings.grMode ].height *
				(grSettings.grBPP >> 3);

#ifdef _DEBUG
	printf("GR_SetFrameBuffers\n");
	printf("\tFrame Buffer 0 Address : 0x%x\n", grSettings.grFrameBuf[0] );
	printf("\tFrame Buffer Size : 0x%x\n", frBufSize );
#endif

	// if double buffering is enabled, we will need a second frame buffer.
	// allocate space for it behind the first one.
	if( grSettings.grDBuffering == GR_DOUBLE_BUFFERING ) {
		grSettings.grFrameBuf[1] = startAddress + frBufSize;

#ifdef _DEBUG
		printf("\tFrame Buffer 1 Address : 0x%x\n", grSettings.grFrameBuf[1]);
#endif
	}

	// prepare GIF packet
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(5, 1, 0, 0, GIF_TAG_PACKED, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this tells the GS which frame buffer to use for drawing operations.
	packet_append_64( &grSettings.grPacket, 
					  GIF_SET_FRAME( startAddress >> 13, grModes[ grSettings.grMode].width >> 6, 
					  grSettings.grPSM, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_FRAME_1 );

	// set up GS area scissoring.
	packet_append_64( &grSettings.grPacket, 
					  GIF_SET_SCISSOR( 0, grModes[ grSettings.grMode].width - 1, 0, 
					  grModes[ grSettings.grMode].height - 1) );
	packet_append_64( &grSettings.grPacket, GIF_REG_SCISSOR_1 );

	// set up alpha and depth testing (GS User Manual, Page 124)
	// turn it all off for now.
	packet_append_64( &grSettings.grPacket, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 0, 0) );

	packet_append_64( &grSettings.grPacket, GIF_REG_TEST_1 );

	packet_append_64( &grSettings.grPacket, 
					  GIF_SET_XYOFFSET((2048 - (grModes[ grSettings.grMode].width >> 1)) << 4,
					  (2048 - (grModes[ grSettings.grMode].height >> 1)) << 4));
	packet_append_64( &grSettings.grPacket, GIF_REG_XYOFFSET_1 );

	// this tells GS to use the drawing parameters from the PRIM register
	packet_append_64( &grSettings.grPacket, GIF_SET_PRMODECONT(1) );
	packet_append_64( &grSettings.grPacket, GIF_REG_PRMODECONT );

	// send GIF packet
	if( packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL ) < 0 )
		return 0;

	if( grSettings.grDBuffering == GR_DOUBLE_BUFFERING )
		return (startAddress + frBufSize * 2);

	return startAddress + frBufSize;
}

//
// GR_SetDisplayBuffer - Sets up CRT's display buffer register
//

int GR_SetDisplayBuffer( int address )
{
	if( !grSettings.grInit )
		return 0;

	// set read circuit 1 display address
	GS_REG_DISPFB1 = GS_SET_DISPFB( address >> 13, grModes[ grSettings.grMode ].width >> 6, 
									grSettings.grPSM, 0, 0 );
	
	// set read circuit 2 display address
	GS_REG_DISPFB2 = GS_SET_DISPFB( address >> 13, grModes[ grSettings.grMode ].width >> 6,
									grSettings.grPSM, 0, 0 );

	return 1;
}

//
// GR_SetTextureBuffer - Sets the texture buffer pointer in GS memory.
//

int GR_SetTextureBuffer( int address )
{
	if( !grSettings.grInit )
		return 0;

	grSettings.grTexBufPtr = address;
	return 1;
}

//
// GR_GetTextureBuffer - Returns the texture buffer pointer
//

int GR_GetTextureBuffer( void )
{
	if( !grSettings.grInit )
		return 0;

	return grSettings.grTexBufPtr;
}

//
// GR_WaitForVSync - Waits for next vertical retrace
//

void GR_WaitForVSync( void )
{
	if( !grSettings.grInit )
		return;

	// enable the vsync interrupt.
	GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

	// wait for the vsync interrupt.
	while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

	// disable the vsync interrupt.
	GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

//
// GR_SetDisplayFrame - sets the display frame
//

int GR_SetDisplayFrame( int frame )
{
	if( !grSettings.grInit )
		return 0;

	if( grSettings.grDBuffering != GR_DOUBLE_BUFFERING )
		return 0;

	GR_SetDisplayBuffer( grSettings.grFrameBuf[ frame ] );
	grSettings.grDisplayFrame = frame;

	return 1;
}

//
// GR_SetDrawFrame - sets the draw frame
//

int GR_SetDrawFrame( int frame )
{
	if( !grSettings.grInit )
		return 0;

	if( grSettings.grDBuffering != GR_DOUBLE_BUFFERING )
		return 0;

	packet_reset( &grSettings.grPacket );

	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	packet_append_64( &grSettings.grPacket, 
					  GIF_SET_FRAME( grSettings.grFrameBuf[ frame ] >> 13, 
					  grModes[ grSettings.grMode].width >> 6, grSettings.grPSM, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_FRAME_1 );

	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );

	grSettings.grDrawFrame = frame;
	
	return 1;
}

//
// GR_SwapBuffers - Swaps display and draw buffers when using double buffering
//

int GR_SwapBuffers( void )
{
	if( !grSettings.grInit )
		return 0;

	if( grSettings.grDBuffering != GR_DOUBLE_BUFFERING )
		return 0;

	GR_WaitForVSync();
	GR_SetDisplayFrame( 1 - grSettings.grDisplayFrame );
	GR_SetDrawFrame( 1 - grSettings.grDrawFrame );

	return 1;
}

//
// GR_SetBkgColor - Sets the background color that is used when the screen is cleared
//

void GR_SetBkgColor( int rgb )
{
	grSettings.grBkgColor = rgb;
}

//
// GR_SetDrawColor - Sets the color for drawing primitives
//

void GR_SetDrawColor( int rgb )
{
	grSettings.grDrawColor = rgb;
}

//
// GR_ClearScreen - Clears the screen and colors it in the selected bkgColor
//

void GR_ClearScreen( void )
{
	int r,g,b;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	r = grSettings.grBkgColor & 255;
	g = (grSettings.grBkgColor >> 8) & 255;
	b = (grSettings.grBkgColor >> 16) & 255;

	// this just draws a big rectangle covering the whole screen
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E);
	packet_append_64( &grSettings.grPacket, GIF_SET_PRIM(GR_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );
	packet_append_64( &grSettings.grPacket, GIF_SET_RGBAQ(r, g, b, 0x80, 0x3F800000) );
	packet_append_64( &grSettings.grPacket, GIF_REG_RGBAQ );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ(0x0000, 0x0000, 0x0000) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ(0xFFFF, 0xFFFF, 0x0000) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
}

//
// GR_AdjustFrom640 - All drawing is done in a virtual 640*480 resolution
//

void GR_AdjustFrom640( int *x, int *y, int *w, int *h )
{
	int mWidth, mHeight;

	if( !grSettings.grInit )
		return;

	mWidth = grModes[ grSettings.grMode ].width;
	mHeight = grModes[ grSettings.grMode ].height;

	if(x) {
		*x = (2048 - mWidth/2) + (*x/640.0) * mWidth;
		*x += grSettings.grOffset_x;
	}

	if(y) {
		*y = (2048 - mHeight/2) + (*y/480.0) * mHeight;
		*y += grSettings.grOffset_y;
	}
	
	if(w)
		*w *= mWidth / 640.0;
	
	if(h)
		*h *= mHeight / 480.0;
}

//
// GR_SetScreenAdjust - Sets horizontal and vertical screen adjustment
//

void GR_SetScreenAdjust( int offset_x, int offset_y )
{
	if( !grSettings.grInit )
		return;

	grSettings.grOffset_x	= offset_x;
	grSettings.grOffset_y	= offset_y;
}

//
// GR_ImageTWValue - Determines the power of two value as needed by the TEX0 register
//

int GR_ImageTWValue( int width )
{
	int tw;

	if( width <= 2 )
		tw = 1;
	else if( width <= 4 )
		tw = 2;
	else if( width <= 8 )
		tw = 3;
	else if( width <= 16 )
		tw = 4;
	else if( width <= 32 )
		tw = 5;
	else if( width <= 64 )
		tw = 6;
	else if( width <= 128 )
		tw = 7;
	else if( width <= 256 )
		tw = 8;
	else if( width <= 512 )
		tw = 9;
	else
		tw = 10;

	return tw;
}

//
// GR_PSM2BPP - Returns bits per pixel for pixel storage mode.
//

int GR_PSM2BPP( int psm )
{
	switch(psm)
	{
		case GR_PSM_32:
			return 32;

		// 24 Bit RGB uses 32 bits per pixel.
		// The alpha byte is not used.
		case GR_PSM_24:
			return 32;

		case GR_PSM_16:
			return 16;

		case GR_PSM_8:
		case GR_PSM_8H:
			return 8;

		case GR_PSM_4:
		case GR_PSM_4HH:
		case GR_PSM_4HL:
			return 4;
	}

	return 0;
}

//
// GR_LoadImage - Loads an image into the GS video ram.
//
// Supports 32 Bit RGBA and 24 Bit RGB image data.
//

int GR_LoadImage( IMG_HANDLE *pHandle, void *pImageData, int imageWidth, int imageHeight, int psm )
{
	int size = imageWidth * imageHeight * (GR_PSM2BPP(psm) >> 3);
	int qFragment;
	int loopCount;
	int i;
	int align;
	int dbw;

	if( !grSettings.grInit )
		return 0;

	pHandle->address = grSettings.grTexBufPtr;
	pHandle->width = imageWidth;
	pHandle->height = imageHeight;
	pHandle->psm = psm;

#ifdef _DEBUG
	printf("GR_LoadImage\n");
	printf("\tEE Data Address : 0x%x\n", (int)pImageData);
	printf("\tGS Destination Address : 0x%x\n", pHandle->address);
	printf("\tImage Width=%i , Image Height=%i\n", pHandle->width, pHandle->height);
#endif

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return 0;

	// transfer the next 5 quadwords and read the suceeding quadword as next DMATAG
	// (EEUSER_E.PDF, P.60)
	packet_append_64( &grSettings.grPacket, DMA_SET_TAG(5, 0, DMA_TAG_CNT, 0, 0, 0) );
	packet_append_64( &grSettings.grPacket, 0x00 );

	// need to tell GS about the upcoming image transfer (1.QW)
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// destination buffer width has to be at least as wide as the texture's
	// width
	if( (pHandle->width % 64) != 0 )	// if width is not a multiple of 64
		dbw = 1 + (pHandle->width / 64);
	else
		dbw = pHandle->width / 64;

	// set up the BITBLTBUF Register (2.QW)
	packet_append_64( &grSettings.grPacket, GIF_SET_BITBLTBUF(0, 0, 0, pHandle->address >> 8, 
					  dbw, psm) );
	packet_append_64( &grSettings.grPacket, GIF_REG_BITBLTBUF );

#ifdef _DEBUG
	printf("\tBITBLTBUF Register:\n");
	printf("\t\tDest Buf Ptr : 0x%x\n", pHandle->address >> 8);
	printf("\t\tDest Buf Width : 0x%x\n", dbw );
#endif

	// TRXPOS is only important when transfering from GS to GS (3.QW)
	packet_append_64( &grSettings.grPacket, GIF_SET_TRXPOS(0, 0, 0, 0, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_TRXPOS );

	// Set up TRXREG register. This contains the actual size of the data (4.QW)
	packet_append_64( &grSettings.grPacket, GIF_SET_TRXREG(imageWidth, imageHeight) );
	packet_append_64( &grSettings.grPacket, GIF_REG_TRXREG );

	// TRXDIR specifies the transfer direction (5.QW)
	packet_append_64( &grSettings.grPacket, GIF_SET_TRXDIR(GR_XDIR_EE_GS) );
	packet_append_64( &grSettings.grPacket, GIF_REG_TRXDIR);

#ifdef _DEBUG
	printf( "\tImage Transfer Data in Quadwords : 0x%x\n", size >> 4 );
#endif

	//
	// The NLOOP field in the GIFTag structure is only 15 bits wide and you
	// can 'only' transfer 2^15 - 1 quadwords in DMA image mode at a time.
	// That means you have to break the transfer down in to slices if you
	// want to transfer more than 2^15 - 1 = 32767 quadwords of data.
	// (EEUSER_E.PDF, P.151)
	qFragment	= (size >> 4) % 32767;
	loopCount	= (size >> 4) / 32767;

	if( qFragment == 0 )
		qFragment = 32767;
	else
		loopCount++;

#ifdef _DEBUG
	printf("\tqFragment : %i\n", qFragment);
	printf("\tloopCount : %i\n", loopCount);
#endif

	for( i=0; i < loopCount; i++ )
	{
#ifdef _DEBUG
		printf("\tTransfering qFragment of size : %i\n", qFragment);
#endif
		// transfer next 1 quadword and read the succeeding quadword as next DMATAG
		packet_append_64( &grSettings.grPacket, DMA_SET_TAG(1, 0, DMA_TAG_CNT, 0, 0, 0) );
		packet_append_64( &grSettings.grPacket, 0x00 );

		// tell GS about image transfer
		packet_append_64( &grSettings.grPacket, GIF_SET_TAG( qFragment, 1, 0, 0, GIF_TAG_IMAGE, 1) );
		packet_append_64( &grSettings.grPacket, 0x00 );

		// transfers qFragment quadwords from address pImageData and read following quadword
		// as next DMATAG
		packet_append_64( &grSettings.grPacket, DMA_SET_TAG( qFragment, 0, DMA_TAG_REF, 0, (u32)pImageData, 0) );
		packet_append_64( &grSettings.grPacket, 0x00 );

		pImageData += qFragment * 16;
		qFragment = 32767;
	}

	// transfer next 2 quadwords and signal end of DMA transfer
	packet_append_64( &grSettings.grPacket, DMA_SET_TAG(2, 0, DMA_TAG_END, 0, 0, 0) );
	packet_append_64( &grSettings.grPacket, 0x00 );

	// tell GS to switch back from IMAGE to PACKED mode
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// alert texflush register of our texture upload
	packet_append_64( &grSettings.grPacket, 0x00 );
	packet_append_64( &grSettings.grPacket, GIF_REG_TEXFLUSH );

	if( packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_CHAIN ) < 0 )
		return 0;

	// if image dimensions are not powers of two, need to calculate
	// the next higher power of two value from the image dimension.
	size =	(int) powf( 2, GR_ImageTWValue(pHandle->width)  ) *
			(int) powf( 2, GR_ImageTWValue(pHandle->height) ) *
			(int) (GR_PSM2BPP(pHandle->psm) >> 3);

	// increase texture buffer pointer
	grSettings.grTexBufPtr += size;

	// align to 256 Bit boundary
	align = grSettings.grTexBufPtr % 256;
	if(align)
		grSettings.grTexBufPtr += 256 - align;

#ifdef _DEBUG
	printf("\tTex Buf Ptr : %i\n", grSettings.grTexBufPtr);
	printf("\%i padding bytes\n", align);
	printf("Texture Upload complete.\n");
#endif

	return 1;
}

//
// GR_DrawImage - Draws an image into draw buffer
//

void GR_DrawImage( int x, int y, int width, int height, const IMG_HANDLE *pHandle )
{
	int tbw;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	GR_AdjustFrom640( &x, &y, &width, &height );

#ifdef _DEBUG
	printf("GR_DrawImage\n");
	printf("\tx=%i , y=%i, width=%i , height=%i\n", x, y, width, height);
#endif

	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(6, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// make sure texture buffer width is at least as wide as texture's width.
	// this is the same as the destination buffer width (dbw) set up in
	// GR_LoadImage.
	if( (pHandle->width % 64) != 0 )	 // if width is not a multiple of 64
		tbw = 1 + (pHandle->width / 64);
	else
		tbw = pHandle->width / 64;

	// FIXME : use Texture Color Component (TCC) for 24 Bit RGB images ?
	// Set up TEX0 register.
	packet_append_64( &grSettings.grPacket, GIF_SET_TEX0( pHandle->address >> 8,
					  tbw,
					  pHandle->psm, GR_ImageTWValue(pHandle->width),
					  GR_ImageTWValue(pHandle->height), 1, 1, 0, 0, 0, 0, 0 ) );
	packet_append_64( &grSettings.grPacket, GIF_REG_TEX0_1 );

#ifdef _DEBUG
	printf("\tTEX0 :\n");
	printf("\t\tTEX BASE PTR = 0x%x\n", pHandle->address >> 8);
	printf("\t\tTEX BUF WIDTH = 0x%x\n", pHandle->width >> 6);
#endif

	// sprite with texture mapping. Use coordinates from UV register.
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// upper reft
	packet_append_64( &grSettings.grPacket, GIF_SET_UV( 0, 0 ) );
	packet_append_64( &grSettings.grPacket, GIF_REG_UV );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	// lower right
	packet_append_64( &grSettings.grPacket, GIF_SET_UV( (pHandle->width-1) << 4, (pHandle->height-1) << 4 ) );
	packet_append_64( &grSettings.grPacket, GIF_REG_UV );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( (x+width) << 4, (y+height) << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	if( packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL ) < 0 )
		return;
}


//
// GR_DrawFillRect - Draws a filled rectangle into draw buffer
//

void GR_DrawFillRect( int x, int y, int width, int height )
{
	int r,g,b;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	r = grSettings.grDrawColor & 255;
	g = (grSettings.grDrawColor >> 8) & 255;
	b = (grSettings.grDrawColor >> 16) & 255;

	// prepare GIF transfer
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(4, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this sets the primitive type
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 0, 0, 1, 0, 0, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// this sets the drawing color
	packet_append_64(&grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, 0x80, 0x3F800000));
	packet_append_64(&grSettings.grPacket, GIF_REG_RGBAQ);

	// adjust coordinates to VCK units
	GR_AdjustFrom640( &x, &y, &width, &height );
	
	// coordinates
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( (x+width) << 4, (y+height) << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
}

//
// GR_DrawRect - Draws a rectangle primitive into the draw buffer
//

void GR_DrawRect( int x, int y, int width, int height )
{
	int r,g,b;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	r = grSettings.grDrawColor & 255;
	g = (grSettings.grDrawColor >> 8) & 255;
	b = (grSettings.grDrawColor >> 16) & 255;

	// prepare GIF transfer
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(7, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this sets the primitive type
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_LINESTRIP, 0, 0, 0, 1, 0, 0, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_LINESTRIP, 0, 0, 0, 0, 0, 0, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// this sets the drawing color
	packet_append_64(&grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, 0x80, 0x3F800000));
	packet_append_64(&grSettings.grPacket, GIF_REG_RGBAQ);

	// adjust coordinates to VCK units
	GR_AdjustFrom640( &x, &y, &width, &height );

	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( (x + width) << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( (x + width) << 4, (y + height) << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, (y + height) << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	// start DMA transfer
	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
}

//
// GR_DrawTriangle - Draws a triangle primitive into draw buffer
//

void GR_DrawTriangle( point_t p1, point_t p2, point_t p3 )
{
	int r,g,b;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	r = grSettings.grDrawColor & 255;
	g = (grSettings.grDrawColor >> 8) & 255;
	b = (grSettings.grDrawColor >> 16) & 255;

	// prepare GIF transfer
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(5, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this sets the primitive type
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_TRIANGLE, 0, 0, 0, 1, 0, 0, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_TRIANGLE, 0, 0, 0, 0, 0, 0, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// this sets the drawing color
	packet_append_64(&grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, 0x80, 0x3F800000));
	packet_append_64(&grSettings.grPacket, GIF_REG_RGBAQ);

	// 1. Vertex
	GR_AdjustFrom640( &p1.x, &p1.y, NULL, NULL );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( p1.x << 4, p1.y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	// 2. Vertex
	GR_AdjustFrom640( &p2.x, &p2.y, NULL, NULL );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( p2.x << 4, p2.y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	// 3. Vertex
	GR_AdjustFrom640( &p3.x, &p3.y, NULL, NULL );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( p3.x << 4, p3.y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	// start DMA transfer
	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
}

//
// GR_DrawDot - Draws a dot using the triangle fan primitive
//

void GR_DrawDot( int x, int y, int radius )
{
	int			i;
	int			origin_x, origin_y;
	float		angle;
	int			r,g,b;
	const int	numPieces = 36;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	origin_x	= x;
	origin_y	= y;

	GR_AdjustFrom640( &origin_x, &origin_y, NULL, NULL );

	r = grSettings.grDrawColor & 255;
	g = (grSettings.grDrawColor >> 8) & 255;
	b = (grSettings.grDrawColor >> 16) & 255;

	// prepare GIF transfer
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG( numPieces + 4, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this sets the primitive type
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_TRIANGLEFAN, 0, 0, 0, 1, 0, 0, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_TRIANGLEFAN, 0, 0, 0, 0, 0, 0, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// this sets the drawing color
	packet_append_64( &grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, 0x80, 0x3F800000) );
	packet_append_64( &grSettings.grPacket, GIF_REG_RGBAQ );

	angle = 360.0f / numPieces;

	// origin vertex
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( origin_x << 4, origin_y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	x = origin_x + radius * cosf(0);
	y = origin_y - radius * sinf(0);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	x = origin_x + radius * cosf( angle / 180 * M_PI );
	y = origin_y - radius * sinf( angle / 180 * M_PI );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	for( i = 2; i <= numPieces; i++ )
	{
		x = origin_x + radius * cosf( i * angle / 180 * M_PI );
		y = origin_y - radius * sinf( i * angle / 180 * M_PI );

		packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
		packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );
	}

	// start DMA transfer
	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
}

//
// GR_SetFont - Selects a font for text drawing
//

int GR_SetFont( IMG_HANDLE *pFontMap, int *texCoords )
{
	if( !grSettings.grInit )
		return 0;

	grSettings.grFontMap.address	= pFontMap->address;
	grSettings.grFontMap.height		= pFontMap->height;
	grSettings.grFontMap.width		= pFontMap->width;

	grSettings.grFontCoords = texCoords;
	grSettings.grFontColor = 0xFFFFFF;

	return 1;
}

//
// GR_SetFontColor - Sets a new font color
//

int GR_SetFontColor( int rgba )
{
	int oldColor;

	if( !grSettings.grInit )
		return 0;

	oldColor = grSettings.grFontColor;
	grSettings.grFontColor = rgba;

	return oldColor;
}

//
// GR_GetStringWidth - Returns the width of pStr
//

int GR_GetStringWidth( const char *pStr, int fontSize )
{
	int *tc;
	int x0, x1, y0, y1;
	int w, c, width;

	if( !grSettings.grInit )
		return 0;

	width = 0;
	c = *pStr;
	
	while(c) {
		tc	= &grSettings.grFontCoords[ c * 4 ];
		x0	= *tc++;
		y0	= *tc++;
		x1	= *tc++;
		y1	= *tc++;
		w	= x1 - x0 + 1;

		width += w * fontSize;

		pStr++;
		c = *pStr;
	}

	return width;
}

//
// GR_DrawTextExt - Draws text using the currently selected font
//
//

void GR_DrawTextExt( int x, int y, const char *s, int fontSize )
{
	char c;
	int *tc;
	int x0, x1, y0, y1;
	int w, h;
	int oldMode;
	int r,g,b,a;

	if( !grSettings.grInit )
		return;

	// get colors
	r = grSettings.grFontColor & 255;
	g = (grSettings.grFontColor >> 8) & 255;
	b = (grSettings.grFontColor >> 16) & 255;
	a = (grSettings.grFontColor >> 24) & 255;

	if( a > 0x80 )
		a = 0x80;

	// set up blend function
	oldMode = GR_SetBlendMode( GR_BLEND_SRC_ALPHA );

	GR_AdjustFrom640( &x, &y, NULL, NULL );

	c = *s;
	while(c)
	{
		tc = &grSettings.grFontCoords[ c * 4 ];
		x0	= *tc++;
		y0	= *tc++;
		x1	= *tc++;
		y1	= *tc++;
		w	= x1 - x0 + 1;
		h	= y1 - y0 + 1;

		if( packet_reset( &grSettings.grPacket ) < 0 )
			return;

		packet_append_64( &grSettings.grPacket, GIF_SET_TAG(7, 1, 0, 0, 0, 1) );
		packet_append_64( &grSettings.grPacket, 0x0E );

		// set up TEX0 register
		packet_append_64( &grSettings.grPacket, GIF_SET_TEX0( grSettings.grFontMap.address >> 8,
						  grSettings.grFontMap.width >> 6,
						  grSettings.grPSM, GR_ImageTWValue(grSettings.grFontMap.width),
						  GR_ImageTWValue(grSettings.grFontMap.width), 1, GR_TEXFUNC_MODULATE, 
						  0, 0, 0, 0, 0 ) );
		packet_append_64( &grSettings.grPacket, GIF_REG_TEX0_1 );

		// setup vertex color register with font color
		packet_append_64(&grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, a, 0x3F800000));
		packet_append_64(&grSettings.grPacket, GIF_REG_RGBAQ);

		// draw texture mapped sprite with alpha blending
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0) );
		packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

		// upper left
		packet_append_64( &grSettings.grPacket, GIF_SET_UV( x0 << 4, y0 << 4 ) );
		packet_append_64( &grSettings.grPacket, GIF_REG_UV );
		packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
		packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

		// lower right
		packet_append_64( &grSettings.grPacket, GIF_SET_UV( (x1+1) << 4, (y1+1) << 4 ) );
		packet_append_64( &grSettings.grPacket, GIF_REG_UV );
		packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( (x+w*fontSize) << 4, (y+h*fontSize) << 4, 0) );
		packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

		if( packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL ) < 0 )
			return;

		// advance drawing position
		x += w * fontSize;

		s++;
		c = *s;
	}

	GR_SetBlendMode( oldMode );
}


//
// GR_SetAlpha - Sets alpha value for constant alpha blending.
//
// Value of 0.0f indicates 100% transparency.
// Value of 1.0f indicates 0% transparency.
//
// Returns old alpha value.
//

float GR_SetAlpha( float alpha )
{
	float ret;
	int value;

	ret = (float) grSettings.grAlpha / 128.0f;

	if( alpha < 0.0f )
		alpha = 0.0f;

	if( alpha > 1.0f )
		alpha = 1.0f;
	
	value = alpha * 128.0f;
	grSettings.grAlpha = value;

	return ret;
}

//
// GR_SetBlendMode - Sets the blend function to be used for alpha blending.
//
// For details on blend func stuff, see GS User Manual, p. 100
// Returns old blend mode.
//

int GR_SetBlendMode( grBlendMode_e mode )
{
	int oldMode;

	if( !grSettings.grInit )
		return 0;

	// don't need to set up any registers so just return
	if( mode == GR_BLEND_NONE ) {
		oldMode = grSettings.grBlendMode;
		grSettings.grBlendMode = mode;
		return oldMode;
	}

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return 0;

	// set up GIF packet
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// set up alpha register for selected blend function
	switch(mode)
	{
		// shut up gcc
		case GR_BLEND_NONE:
			break;

		case GR_BLEND_CONSTANT:
			packet_append_64( &grSettings.grPacket, GIF_SET_ALPHA(0, 1, 2, 1, grSettings.grAlpha) );
			break;

		case GR_BLEND_SRC_ALPHA:
			packet_append_64( &grSettings.grPacket, GIF_SET_ALPHA(0, 1, 0, 1, 0) );
			break;
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_ALPHA_1 );
	
	// start dma transfer
	if( packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL ) < 0 )
		return 0;

	oldMode = grSettings.grBlendMode;
	grSettings.grBlendMode = mode;

	return oldMode;
}


//
// GR_DrawRoundRect - Draws a filled rectangle with round edges
//

// helper
void _drawfillrect( int x, int y, int width, int height )
{
	int r,g,b;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	r = grSettings.grDrawColor & 255;
	g = (grSettings.grDrawColor >> 8) & 255;
	b = (grSettings.grDrawColor >> 16) & 255;

	// prepare GIF transfer
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG(4, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this sets the primitive type
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 0, 0, 1, 0, 0, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// this sets the drawing color
	packet_append_64(&grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, 0x80, 0x3F800000));
	packet_append_64(&grSettings.grPacket, GIF_REG_RGBAQ);
	
	// coordinates
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( (x+width) << 4, (y+height) << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2);

	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );

}

void _drawarc( int x, int y, int radius, int start_angle, int degrees  )
{
	int	r,g,b,i;
	int step;
	int numPieces;
	int origin_x, origin_y;

	if( !grSettings.grInit )
		return;

	if( packet_reset( &grSettings.grPacket ) < 0 )
		return;

	origin_x	= x;
	origin_y	= y;

	r = grSettings.grDrawColor & 255;
	g = (grSettings.grDrawColor >> 8) & 255;
	b = (grSettings.grDrawColor >> 16) & 255;

	step		= 10;
	degrees 	= degrees % 360;
	numPieces 	= degrees / step;
	start_angle = start_angle - 90;

	// prepare GIF transfer
	packet_append_64( &grSettings.grPacket, GIF_SET_TAG( numPieces + 4, 1, 0, 0, 0, 1) );
	packet_append_64( &grSettings.grPacket, 0x0E );

	// this sets the primitive type
	if( grSettings.grBlendMode != GR_BLEND_NONE ) {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_TRIANGLEFAN, 0, 0, 0, 1, 0, 0, 0, 0) );
	}
	else {
		packet_append_64( &grSettings.grPacket, GIF_SET_PRIM( GR_PRIM_TRIANGLEFAN, 0, 0, 0, 0, 0, 0, 0, 0) );
	}

	packet_append_64( &grSettings.grPacket, GIF_REG_PRIM );

	// this sets the drawing color
	packet_append_64( &grSettings.grPacket, GIF_SET_RGBAQ( r, g, b, 0x80, 0x3F800000) );
	packet_append_64( &grSettings.grPacket, GIF_REG_RGBAQ );

	// origin vertex
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( origin_x << 4, origin_y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	x = origin_x + radius * cosf( start_angle / 180.0f * M_PI );
	y = origin_y + radius * sinf( start_angle / 180.0f * M_PI );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	x = origin_x + radius * cosf( (step + start_angle) / 180.0f * M_PI );
	y = origin_y + radius * sinf( (step + start_angle) / 180.0f * M_PI );
	packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
	packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );

	for( i = 2; i <= numPieces; i++ )
	{
		x = origin_x + radius * cosf( (i * step + start_angle) / 180.0f * M_PI );
		y = origin_y + radius * sinf( (i * step + start_angle) / 180.0f * M_PI );

		packet_append_64( &grSettings.grPacket, GIF_SET_XYZ( x << 4, y << 4, 0) );
		packet_append_64( &grSettings.grPacket, GIF_REG_XYZ2 );
	}

	// start DMA transfer
	packet_send( &grSettings.grPacket, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );

	// FIXME : I am getting weird graphic bugs when I don't add another
	// dma_channel_wait after this...
	// I haven't figured out whether it's a problem in my code
	// or a problem inside the dma/packet stuff from ps2sdk.
	dma_channel_wait( DMA_CHANNEL_GIF, -1 , DMA_FLAG_NORMAL );
}

void GR_DrawRoundRect( int x, int y, int width, int height )
{
	int nWidth, nHeight;
	int nPosX, nPosY;
	int offset		= 20;
	int frameSize	= 2;

	if( !grSettings.grInit )
		return;

	GR_AdjustFrom640( &x, &y, &width, &height );

	// body part
	nWidth	= width  - offset * 2;
	nHeight	= height - frameSize * 2;
	nPosX	= x + offset;
	nPosY	= y + frameSize;
	_drawfillrect( nPosX, nPosY, nWidth, nHeight );

	// left part
	nWidth	= offset - frameSize;
	nHeight	= height - 2 * offset;
	nPosX	= x + frameSize;
	nPosY	= y + offset;
	_drawfillrect( nPosX, nPosY, nWidth, nHeight );

	// right part
	nWidth	= offset - frameSize;
	nHeight	= height - 2 * offset;
	nPosX	= x + width - offset;
	nPosY	= y + offset;
	_drawfillrect( nPosX, nPosY, nWidth, nHeight );

	// upper left edge
	nPosX	= x + offset;
	nPosY	= y + offset;
	_drawarc( nPosX, nPosY, offset - frameSize, 270, 90 );

	// upper right edge
	nPosX	= x + width - offset;
	nPosY	= y + offset;
	_drawarc( nPosX, nPosY, offset - frameSize, 0, 90 );

	// lower right edge
	nPosX	= x + width - offset;
	nPosY	= y + height - offset;
	_drawarc( nPosX, nPosY, offset - frameSize, 90, 90 );

	// lower left edge
	nPosX	= x + offset;
	nPosY	= y + height - offset;
	_drawarc( nPosX, nPosY, offset - frameSize, 180, 90 );

}

