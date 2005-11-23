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
# Date: 10-16-2005
#
# File: myPS2 User Interface.
#		Picture Viewer Code.
#
*/

#include <tamtypes.h>
#include <ui.h>

#define ID_HDD			1
#define ID_CD			2
#define ID_USB			3
#define ID_PRECACHE		4
#define ID_SORTNAME		5
#define ID_SORTTYPE		6
#define ID_SORTSIZE		7
#define ID_GOBACK		8

#define ID_PREVPAGE		9
#define ID_NEXTPAGE		10

#define ID_THUMBNAILS	4000

#define NUM_DISPLAY_IMAGES	16

typedef struct
{
	menuFramework_t *menu;

	menuText_t		HDD;
	menuText_t		CD;
	menuText_t		USB;
	menuText_t		Precache;
	menuText_t		SortName;
	menuText_t		SortType;
	menuText_t		SortSize;
	menuText_t		GoBack;

	menuBitmap_t	images[NUM_DISPLAY_IMAGES];
	menuText_t		imagetext[NUM_DISPLAY_IMAGES];
	char			text[NUM_DISPLAY_IMAGES][9];

	menuTriBtn_t	prevPage;
	menuTriBtn_t	nextPage;

	menuText_t		PageText;
	char			PageString[128];

	int				numStatic;

	int				oldTexBuf;

	IMG_HANDLE		hFolderImg;
	IMG_HANDLE		hErrorImg;
} uiPicViewMenu_t;

static uiPicViewMenu_t s_picview;

int				numFiles;					// number of files & dirs in currentPath
int				numPages;					// total number of image display pages
int				currentPage;				// page that is currently being displayed
char			currentDir[256];			// current path of directory we are browsing
fileInfo_t		fileInfo[MAX_DIR_FILES];	// Has 'numFiles' valid entries

int				bDetailView;				// 1 if viewing image fullscreen
IMG_HANDLE		hDetailImg;					// Handle to image in detail mode

//
// UI_InitPicViewMenu - Initializes Picture View Controls
//

void UI_InitPicViewMenu( void )
{
	s_picview.menu						= &uis.menus[ MENU_PICVIEW ];

	s_picview.menu->callback			= UI_PicViewCallback;
	s_picview.menu->input				= NULL;

	s_picview.menu->numItems			= 0;

	// cd tab is selected by default
	s_picview.menu->selectedItem		= 0;

	s_picview.CD.generic.type			= MENU_CONTROL_TEXT;
	s_picview.CD.generic.flags			= 0;
	s_picview.CD.generic.x				= 35;
	s_picview.CD.generic.y				= 70;
	s_picview.CD.generic.id				= ID_CD;
	s_picview.CD.text					= "Browse CD";
	s_picview.CD.size					= GR_FONT_SMALL;
	s_picview.CD.color					= RGB(255, 255, 255);

	s_picview.HDD.generic.type			= MENU_CONTROL_TEXT;
	s_picview.HDD.generic.flags			= 0;
	s_picview.HDD.generic.x				= 35;
	s_picview.HDD.generic.y				= 90;
	s_picview.HDD.generic.id			= ID_HDD;
	s_picview.HDD.text					= "Browse HDD";
	s_picview.HDD.size					= GR_FONT_SMALL;
	s_picview.HDD.color					= RGB(255, 255, 255);

	if( HDD_Available() != HDD_AVAIL )
		s_picview.HDD.generic.flags		= CFL_INACTIVE;

	s_picview.USB.generic.type			= MENU_CONTROL_TEXT;
	s_picview.USB.generic.flags			= 0;
	s_picview.USB.generic.x				= 35;
	s_picview.USB.generic.y				= 110;
	s_picview.USB.generic.id			= ID_USB;
	s_picview.USB.text					= "Browse USB";
	s_picview.USB.size					= GR_FONT_SMALL;
	s_picview.USB.color					= RGB(255, 255, 255);

	if( !USB_Available() )
		s_picview.USB.generic.flags		= CFL_INACTIVE;

	s_picview.Precache.generic.type		= MENU_CONTROL_TEXT;
	s_picview.Precache.generic.flags	= 0;
	s_picview.Precache.generic.x		= 35;
	s_picview.Precache.generic.y		= 140;
	s_picview.Precache.generic.id		= ID_PRECACHE;
	s_picview.Precache.text				= "Precache all";
	s_picview.Precache.size				= GR_FONT_SMALL;
	s_picview.Precache.color			= RGB(255, 255, 255);

	if( sysConf.tbnCaching == 0 )
		s_picview.Precache.generic.flags = CFL_INACTIVE;

	s_picview.SortName.generic.type		= MENU_CONTROL_TEXT;
	s_picview.SortName.generic.flags	= 0;
	s_picview.SortName.generic.x		= 35;
	s_picview.SortName.generic.y		= 170;
	s_picview.SortName.generic.id		= ID_SORTNAME;
	s_picview.SortName.text				= "Sort by name";
	s_picview.SortName.size				= GR_FONT_SMALL;
	s_picview.SortName.color			= RGB(255, 255, 255);

	s_picview.SortType.generic.type		= MENU_CONTROL_TEXT;
	s_picview.SortType.generic.flags	= 0;
	s_picview.SortType.generic.x		= 35;
	s_picview.SortType.generic.y		= 190;
	s_picview.SortType.generic.id		= ID_SORTTYPE;
	s_picview.SortType.text				= "Sort by type";
	s_picview.SortType.size				= GR_FONT_SMALL;
	s_picview.SortType.color			= RGB(255, 255, 255);

	s_picview.SortSize.generic.type		= MENU_CONTROL_TEXT;
	s_picview.SortSize.generic.flags	= 0;
	s_picview.SortSize.generic.x		= 35;
	s_picview.SortSize.generic.y		= 210;
	s_picview.SortSize.generic.id		= ID_SORTSIZE;
	s_picview.SortSize.text				= "Sort by size";
	s_picview.SortSize.size				= GR_FONT_SMALL;
	s_picview.SortSize.color			= RGB(255, 255, 255);

	s_picview.GoBack.generic.type		= MENU_CONTROL_TEXT;
	s_picview.GoBack.generic.flags		= 0;
	s_picview.GoBack.generic.x			= 35;
	s_picview.GoBack.generic.y			= 250;
	s_picview.GoBack.generic.id			= ID_GOBACK;
	s_picview.GoBack.text				= "Go back";
	s_picview.GoBack.size				= GR_FONT_SMALL;
	s_picview.GoBack.color				= RGB(255, 255, 255);

	// add items to menu container
	UI_AddItemToMenu( s_picview.menu, &s_picview.CD );
	UI_AddItemToMenu( s_picview.menu, &s_picview.HDD );
	UI_AddItemToMenu( s_picview.menu, &s_picview.USB );
	UI_AddItemToMenu( s_picview.menu, &s_picview.Precache );
	UI_AddItemToMenu( s_picview.menu, &s_picview.SortName );
	UI_AddItemToMenu( s_picview.menu, &s_picview.SortType );
	UI_AddItemToMenu( s_picview.menu, &s_picview.SortSize );

	UI_AddItemToMenu( s_picview.menu, &s_picview.GoBack );

	// all controls after these are dynamic
	// meaning they will be deleted and recreated when the display page
	// is changed.
	s_picview.numStatic = s_picview.menu->numItems;

	// load folder and error thumbnails
	GR_LoadImage( &s_picview.hFolderImg, (void*)&img_folder[0], img_folder_w, img_folder_h, img_folder_psm );
	GR_LoadImage( &s_picview.hErrorImg, (void*)&img_error[0], img_error_w, img_error_h, img_error_psm );

	// save old texture buffer pointer so we can easily reset
	// it when loading a new set of thumbnails.
	s_picview.oldTexBuf	= GR_GetTextureBuffer();

	return;
}

//
// UI_PicViewCallback
//

int UI_PicViewCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	int offset;

	switch( nMsg )
	{
		// reset texture buffer pointer
		case MSG_CLOSE:
			GR_SetTextureBuffer( s_picview.hFolderImg.address );
			return 1;

		case MSG_DRAW:
			if( bDetailView )
				UI_DrawDetailView();
			else
				UI_PicViewDraw();
			return 1;

		case MSG_CONTROL:
			switch( fParam )
			{
				case NOT_CLICKED:
					// check if it's a thumbnail control
					if( sParam >= ID_THUMBNAILS ) {
						offset = sParam - ID_THUMBNAILS;
						offset += currentPage * 16;

						if( offset < 0 || offset >= numFiles ) {
							printf("Error : offset out of range\n");
							return 1;
						}

						// clicked a directory thumbnail
						if( fileInfo[offset].flags & FLAG_DIRECTORY ) {
							char newPath[256];
							char *ptr;

							// go up one level
							if( !strcmp( fileInfo[offset].name, ".." ) ) {
								offset = strlen(currentDir) - 1;
								strncpy( newPath, currentDir, offset );
								newPath[offset] = '\0';

								ptr = strrchr( newPath, '/' );
								if( !ptr )
									return 1;

								offset = ptr - newPath + 1;
								strncpy( newPath, currentDir, offset );
								newPath[offset] = 0;
							}
							else {
								// append directory name to path
								strcpy( newPath, currentDir );
								strcat( newPath, fileInfo[offset].name );
								strcat( newPath, "/" );
							}

							// make sure first thumbnail is always selected
							s_picview.menu->selectedItem = s_picview.numStatic + 1;

							UI_PicViewSetDir( newPath );
							UI_UpdateImageGrid();
						}
						else {
							// clicked an image, switch to detail view
							UI_InitDetailView( &fileInfo[offset], currentDir, &hDetailImg );
						}
						return 1;
					}
					
					switch(sParam)
					{
						// browse CD/DVD for images
						case ID_CD:
							UI_PicViewSetDir( "cdfs:/" );
							UI_UpdateImageGrid();
							return 1;

						// browse HDD for images
						case ID_HDD:
							UI_PicViewSetDir( "pfs0:/" );
							UI_UpdateImageGrid();
							return 1;

						// browse USB for images
						case ID_USB:
							UI_PicViewSetDir( "mass:/" );
							UI_UpdateImageGrid();
							return 1;

						// precache all thumbnails
						case ID_PRECACHE:
							UI_PopupOpen( MENU_POPUP_THUMB, s_picview.menu );
							return 1;

						case ID_SORTNAME:
							qsort( fileInfo, numFiles, sizeof(fileInfo_t), UI_SortByName );
							UI_UpdateImageGrid();
							return 1;

						case ID_SORTSIZE:
							qsort( fileInfo, numFiles, sizeof(fileInfo_t), UI_SortBySize );
							UI_UpdateImageGrid();
							return 1;

						// pointless right now since we only load jpgs anyway
						case ID_SORTTYPE:
//							qsort( fileInfo, numFiles, sizeof(fileInfo_t), UI_SortByType );
//							UI_UpdateImageGrid();
							return 1;

						case ID_PREVPAGE:
							UI_PicViewPrevPage();
							return 1;

						case ID_NEXTPAGE:
							UI_PicViewNextPage();
							return 1;

						case ID_GOBACK:
							UI_SetActiveMenu(MENU_MAIN);
							return 1;
					}
					break;
			}
			break;
	}

	return 0;
}

//
// UI_PicViewDraw - Custom Draw function
//

void UI_PicViewDraw( void )
{
	// draw sidebar
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	// draw controls
	UI_DrawMenu( s_picview.menu );
}

//
// UI_UpdateImageGrid - Updates the image grid. Also adjusts
// selection if necessary.
//

void UI_UpdateImageGrid( void )
{
	int rows, cols;
	int x, y, n;
	int numThumbs;
	int Sel;
	menuCommon_t *pSelItem;
	int nRet;

	// This is ugly
	GR_SetTextureBuffer( s_picview.oldTexBuf );

	// If PREV or NEXT button is currently selected
	// make sure we select it again after dynamic
	// content has been modified.
	Sel = -1;
	pSelItem = UI_GetSelectedItem( s_picview.menu );
	if( pSelItem && pSelItem->id == ID_PREVPAGE ) {
		Sel = ID_PREVPAGE;
	}
	else if( pSelItem && pSelItem->id == ID_NEXTPAGE ) {
		Sel = ID_NEXTPAGE;
	}

	// delete all of the old dynamic content from the menu first
#ifdef _DEBUG
	printf("numStatic %i\n", s_picview.numStatic );
	printf("numItems %i\n", s_picview.menu.numItems );
#endif
	x = s_picview.menu->numItems;
	y = s_picview.numStatic;
	
	for( n = y; n < x; n++ )
		UI_DelItemFromMenu( s_picview.menu, s_picview.menu->items[y] );

	// may have to draw "previous page" button
	if( numPages > 0 ) {
		s_picview.prevPage.generic.type		= MENU_CONTROL_TRIBUTTON;
		s_picview.prevPage.generic.flags	= 0;
		s_picview.prevPage.generic.x		= 160;
		s_picview.prevPage.generic.y		= 234;
		s_picview.prevPage.generic.id		= ID_PREVPAGE;
		s_picview.prevPage.width			= 30;
		s_picview.prevPage.height			= 30;
		s_picview.prevPage.direction		= TRI_LEFT;
		s_picview.prevPage.color			= RGBA( 82, 117, 168, 32 );

		// select this item
		if( Sel == ID_PREVPAGE )
			s_picview.menu->selectedItem = s_picview.menu->numItems;

		UI_AddItemToMenu( s_picview.menu, &s_picview.prevPage );
	}

	y = 60;
	n = 0;

	// if we are viewing the last page, there can be less than
	// 16 images to view
	if( currentPage == (numPages - 1) )
		numThumbs = numFiles % 16;
	else
		numThumbs = 16;

	// draw thumbnail bitmaps now
	for( rows = 0; rows < 4; rows++ )
	{
		x = 200;

		for( cols = 0; cols < 4; cols++ )
		{
			if( n >= numThumbs )
				break;

			nRet = UI_CreateThumbnail( &fileInfo[ currentPage * 16 + n ],
									   currentDir,
									   &s_picview.images[n].hImage );

			// failed to create thumbnail so show some error image instead
			if( 0 == nRet )
				s_picview.images[n].hImage = s_picview.hErrorImg;

			s_picview.images[n].generic.type		= MENU_CONTROL_BITMAP;
			s_picview.images[n].generic.flags		= CFL_FRAME | CFL_CENTER_JUSTIFY;
			s_picview.images[n].generic.x			= x;
			s_picview.images[n].generic.y			= y;
			s_picview.images[n].generic.id			= ID_THUMBNAILS + n;
			s_picview.images[n].width				= 80;
			s_picview.images[n].height				= 80;
			s_picview.images[n].frameColor			= RGBA( 196, 226, 255, 128 );

			// mark erroneous images as inactive so they can be easily skipped
			if( 0 == nRet )
				s_picview.images[n].generic.flags |= CFL_INACTIVE;

			UI_AddItemToMenu( s_picview.menu, &s_picview.images[n] );

			// add image name
			strncpy( s_picview.text[n], fileInfo[ currentPage * 16 + n ].name, 8 );
			s_picview.text[n][8] = '\0';

			if( strlen( fileInfo[ currentPage * 16 + n ].name ) > 8 )
				s_picview.text[n][6] = s_picview.text[n][7] = '.';

			s_picview.imagetext[n].generic.type			= MENU_CONTROL_TEXT;
			s_picview.imagetext[n].generic.flags		= CFL_INACTIVE | CFL_FORCECOLOR;
			s_picview.imagetext[n].generic.x			= x;
			s_picview.imagetext[n].generic.y			= y + 80;
			s_picview.imagetext[n].text					= s_picview.text[n];
			s_picview.imagetext[n].size					= GR_FONT_SMALL;
			s_picview.imagetext[n].color				= RGB(255, 255, 255);

			UI_AddItemToMenu( s_picview.menu, &s_picview.imagetext[n] );

			n++;
			x += 100;
		}

		y += 100;
	}

	// Stop DVD Drive (If it's even running)
	CDVD_Stop();

	// may have to draw "next page" button
	if( numPages > 0 ) {
		s_picview.nextPage.generic.type		= MENU_CONTROL_TRIBUTTON;
		s_picview.nextPage.generic.flags	= 0;
		s_picview.nextPage.generic.x		= 590;
		s_picview.nextPage.generic.y		= 234;
		s_picview.nextPage.generic.id		= ID_NEXTPAGE;
		s_picview.nextPage.width			= 30;
		s_picview.nextPage.height			= 30;
		s_picview.nextPage.direction		= TRI_RIGHT;
		s_picview.nextPage.color			= RGBA( 82, 117, 168, 32 );

		// select this item?
		if( Sel == ID_NEXTPAGE )
			s_picview.menu->selectedItem = s_picview.menu->numItems;

		UI_AddItemToMenu( s_picview.menu, &s_picview.nextPage );
	}

	sprintf( s_picview.PageString, "Browsing Page %i / %i", currentPage + 1, numPages );

	s_picview.PageText.generic.type		= MENU_CONTROL_TEXT;
	s_picview.PageText.generic.flags	= CFL_INACTIVE | CFL_FORCECOLOR;
	s_picview.PageText.generic.x		= 460;
	s_picview.PageText.generic.y		= 460;
	s_picview.PageText.size				= GR_FONT_SMALL;
	s_picview.PageText.text				= s_picview.PageString;
	s_picview.PageText.color			= RGB(255, 255, 255);

	UI_AddItemToMenu( s_picview.menu, &s_picview.PageText );
	UI_Refresh();
}

//
// UI_PicViewSetDir - Switches to another directory.
//					  Sets up numFiles, numPages, currentPage etc.
//

void UI_PicViewSetDir( const char *path )
{
	strcpy( currentDir, path );
	
	numFiles = DirGetContents( currentDir, "jpg", fileInfo, MAX_DIR_FILES );
			
	if( (numFiles % 16) ) {
		numPages = numFiles / 16 + 1;
	}
	else {
		numPages = numFiles / 16;
		if( numPages == 0 )
			numPages = 1;
	}

	currentPage	= 0;

#ifdef _DEBUG
	printf("numFiles : %i\n", numFiles);
	printf("numPages : %i\n", numPages);
#endif
}

//
// UI_PicViewPrevPage
//

void UI_PicViewPrevPage( void )
{
	if( currentPage == 0 )
		return;

	currentPage--;

	// update image grid now
	UI_UpdateImageGrid();
}

//
// UI_PicViewNextPage
//

void UI_PicViewNextPage( void )
{	
	if( currentPage >= (numPages - 1) )
		return;

	currentPage++;

	// update image grid now
	UI_UpdateImageGrid();

}

//
// UI_SortByName - Comparison function for name sort
//

int UI_SortByName( const void *a, const void *b )
{
	fileInfo_t *p1, *p2;

	p1 = (fileInfo_t*)a;
	p2 = (fileInfo_t*)b;

	// directories are always first
	if( (p1->flags & FLAG_DIRECTORY) && !(p2->flags & FLAG_DIRECTORY) )
		return -1;
	else if( !(p1->flags & FLAG_DIRECTORY) && (p2->flags & FLAG_DIRECTORY) )
		return 1;

	return strcmp( p1->name, p2->name );
}

//
// UI_SortBySize - Comparison function for size sort
//

int UI_SortBySize( const void *a, const void *b )
{
	fileInfo_t *p1, *p2;

	p1 = (fileInfo_t*)a;
	p2 = (fileInfo_t*)b;

	// directories are always first
	if( (p1->flags & FLAG_DIRECTORY) && !(p2->flags & FLAG_DIRECTORY) )
		return -1;
	else if( !(p1->flags & FLAG_DIRECTORY) && (p2->flags & FLAG_DIRECTORY) )
		return 1;

	return ( p1->size - p2->size );
}

//
// UI_SortByType - Comparison function for type sort
//

int UI_SortByType( const void *a, const void *b )
{
	fileInfo_t *p1, *p2;
	char *ext1, *ext2;

	p1 = (fileInfo_t*)a;
	p2 = (fileInfo_t*)b;

	if( (p1->flags & FLAG_DIRECTORY) && !(p2->flags & FLAG_DIRECTORY) )
		return -1;
	else if( !(p1->flags & FLAG_DIRECTORY) && (p2->flags & FLAG_DIRECTORY) )
		return 1;

	// grab the file extensions
	ext1 = strrchr( p1->name, '.' );
	ext2 = strrchr( p2->name, '.' );

	// skip dot
	ext1++;
	ext2++;

	return strcmp( ext1, ext2 );
}

//
// UI_CreateThumbnail - Creates a thumbnail image for the Picture Viewer.
// The wider side of the image is resized to 64 pixels, the other 
// side according to the aspect ratio of the image.
//

int UI_CreateThumbnail( const fileInfo_t *pFile, const char *pCurrentDir, IMG_HANDLE *pHandle )
{
	char 		*pFilename;
	u8 			*pRaw;
	FHANDLE		fh;
	jpgData		*pJpg;
	u8			*pImgData;
	int			nWidth, nHeight;
	float		fRatio;
	u8			*pResData;
	int			nRet;

	if( !pFile || !pCurrentDir || !pHandle )
		return 0;

	memset( pHandle, 0, sizeof(IMG_HANDLE) );

	// just use the precached folder image
	if( pFile->flags & FLAG_DIRECTORY ) {
		*pHandle = s_picview.hFolderImg;
		return 1;
	}

	// if it's already in the picture cache just use that
	if( UI_TbnCacheLoad( pFile, pCurrentDir, pHandle ) ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : Using cached picture\n");
#endif
		return 1;
	}
	
	pFilename = (char*) malloc( strlen(pCurrentDir) + strlen(pFile->name) + 1 );
	if( !pFilename ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : malloc() for pFilename failed!\n");
#endif
		return 0;
	}				
	
	strcpy( pFilename, currentDir );
	strcat( pFilename, pFile->name );

	// copy file into memory
	if( (pRaw = (u8*) malloc( pFile->size )) == NULL ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : malloc() for pRaw failed!\n");
#endif
		return 0;
	}

	fh = FileOpen( pFilename, O_RDONLY );
	if( fh.fh == -1 ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : Could not open file %s\n", pFilename );
#endif
		free(pFilename);
		free(pRaw);
		return 0;
	}

	FileRead( fh, pRaw, pFile->size );
	FileClose( fh );

	// attempt to read jpeg header from file
	if( (pJpg = jpgOpenRAW( pRaw, pFile->size, JPG_WIDTH_FIX )) == NULL ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : jpgOpenRAW() failed!\n");
#endif
		free(pFilename);
		free(pRaw);
		return 0;
	}

#ifdef _DEBUG
	printf("Reading JPG Header for %s\n", pFile->name);
#endif

	// We only support 24 Bits per Pixel right now
	if( pJpg->bpp != 24 ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : unsupported jpg colordepth (%i)\n", pJpg->bpp);
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		return 0;
	}

	// attempt to get rgb data from jpg
	pImgData = (u8*) malloc( ((3 * pJpg->width + 3) & ~3) * pJpg->height );
	if( !pImgData ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : malloc() for pImgData failed!\n");
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		return 0;
	}

	if( -1 == jpgReadImage( pJpg, pImgData ) ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : jpgReadImage failed for %s\n", pFilename);
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		free(pImgData);
		return 0;
	}

#ifdef _DEBUG
	printf("Reading RGB Image Data\n");
#endif

	// figure out if image is landscape or portrait format
	if( pJpg->width > pJpg->height ) {
		fRatio	= (float) pJpg->height / pJpg->width;
		nWidth 	= 64;
		nHeight	= (int) (nWidth * fRatio);

		// make sure Height is even
		if( nHeight % 2 )
			nHeight++;
	} 
	else {
		fRatio	= (float) pJpg->width / pJpg->height;
		nHeight	= 64;
		nWidth	= (int) (nHeight * fRatio);

		// make sure width is a multiple of 4 so
		// ScaleBitmap will always work correctly.
		nWidth -= (nWidth % 4);
	}

#ifdef _DEBUG
	printf("Resizing Image from %i, %i to %i, %i\n", pJpg->width, pJpg->height, nWidth, nHeight );
#endif

	// attempt to resize image
	nRet = ScaleBitmap( pImgData, pJpg->width, pJpg->height, &pResData, nWidth, nHeight );
	if( 0 == nRet ) {
#ifdef _DEBUG
		printf("UI_CreateThumbnail : ScaleBitmap failed for %s\n", pFilename);
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		free(pImgData);
		return 0;
	}

	// save cache file so image can be loaded faster next time
	UI_TbnCacheSave( pFile, pCurrentDir, nWidth, nHeight, pResData );

	// upload thumbnail image to video ram
	GR_LoadImage( pHandle, pResData, nWidth, nHeight, GR_PSM_24 );

	jpgClose(pJpg);
	free(pRaw);
	free(pImgData);
	free( pFilename );

	// done!
	return 1;
}

//
// UI_TbnCacheLoad	- Attempts to find a cached version of a thumbnail for an image
//					  in the HDD thumbnail cache. If cached thumbnail is found
//					  it is then uploaded into vram.
//
//					  The function returns 1 on success, otherwise 0.
//

int UI_TbnCacheLoad( const fileInfo_t *pFile, const char *pCurrentDir, IMG_HANDLE *pHandle )
{
	FHANDLE		fHandle;
	TBN_HEADER	tbnHeader;
	char		strPath[256], strTmp[256];
	char		*pStr;
	u8			*pImgData;
	int			nSize;

	// user doesn't have thumbnail caching enabled
	if( sysConf.tbnCaching == 0 )
		return 0;

	// user doesn't have a HDD and thumbnail caching shouldn't
	// be enabled in the first place...
	if( HDD_Available() != HDD_AVAIL )
		return 0;

	if( !sysConf.tbnCachePath[0] )
		return 0;

	// build path to TBN file
	strcpy( strPath, sysConf.tbnCachePath );

	if( strPath[ strlen(strPath) - 1 ] != '/' )
		strcat( strPath, "/" );

	strcpy( strTmp, pCurrentDir );

	if( (pStr = strchr( strTmp, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("UI_TbnCacheLoad : Invalid directory (missing ':')\n");
#endif
		return 0;
	}

	*pStr = 0;
	pStr++;

	strcat( strPath, strTmp );
	strcat( strPath, pStr );

	// make sure directory path exists
	DirCreate( strPath );

	// add filename
	StripFileExt( strTmp, pFile->name );
	strcat( strPath, strTmp );
	strcat( strPath, ".TBN" );

	// If you try to open files that don't exist too fast
	// it seems to crash the IOP (null buffer returned).
	// So as a workaround always open the file with O_CREAT
	// which seems to fix that problem.
	fHandle = FileOpen( strPath, O_RDONLY | O_CREAT );

	// couldn't open file
	if( fHandle.fh < 0 )
	{
		printf("UI_TbnCacheLoad : could not open %s\n", strPath);
		return 0;
	}

	// file was just created by FileOpen
	nSize = FileSeek( fHandle, 0, SEEK_END );
	if( nSize == 0 ) {
		FileClose( fHandle );
		return 0;
	}

	FileSeek( fHandle, 0, SEEK_SET );
	FileRead( fHandle, &tbnHeader, sizeof(tbnHeader) );

	if( tbnHeader.magic[0] != 'T' || tbnHeader.magic[1] != 'C' ) {
		FileClose(fHandle);
		return 0;
	}

	if( tbnHeader.size != pFile->size )
		return 0;

	// read the RGB data
	pImgData = (u8*) memalign( 128, tbnHeader.width * tbnHeader.height * 3 );
	
	if( pImgData == NULL ) {
#ifdef _DEBUG
		printf("UI_TbnCacheLoad : memalign() failed for pImgData\n");
#endif
		FileClose(fHandle);
		return 0;
	}

	FileRead( fHandle, pImgData, tbnHeader.width * tbnHeader.height * 3 );
	FileClose( fHandle );

	// upload image
	GR_LoadImage( pHandle, pImgData, tbnHeader.width, tbnHeader.height, GR_PSM_24 );

	free(pImgData);
	return 1;
}

//
// UI_TbnCacheSave	- Attempts to save generated thumbnail RGB data to HDD
//					  thumbnail cache directory.
//					  RGB data should be 24 Bits per pixel.
//
//					  The function returns 1 on success, otherwise 0.
//

int UI_TbnCacheSave( const fileInfo_t *pFile, const char *pCurrentDir, int nWidth, int nHeight, void *pRGBData )
{
	FHANDLE		fHandle;
	TBN_HEADER	tbnHeader;
	char		strPath[256], strTmp[256];
	char		*pStr;

	if( sysConf.tbnCaching == 0 )
		return 0;

	if( !sysConf.tbnCachePath[0] )
		return 0;

	if( HDD_Available() != HDD_AVAIL )
		return 0;

	// build path to TBN file
	strcpy( strPath, sysConf.tbnCachePath );

	if( strPath[ strlen(strPath) - 1 ] != '/' )
		strcat( strPath, "/" );

	strcpy( strTmp, pCurrentDir );

	if( (pStr = strchr( strTmp, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("UI_TbnCacheSave : Invalid directory (missing ':')\n");
#endif
		return 0;
	}

	*pStr = 0;
	pStr++;

	strcat( strPath, strTmp );
	strcat( strPath, pStr );

	// add filename
	StripFileExt( strTmp, pFile->name );
	strcat( strPath, strTmp );
	strcat( strPath, ".TBN" );

	// attempt to create file
	fHandle = FileOpen( strPath, O_RDWR | O_CREAT | O_TRUNC );

	if( fHandle.fh < 0 )
	{
		printf("UI_TbnCacheSave : Could not open file : %s\n", strPath );
		return 0;
	}

	// write thumbnail cache file header
	tbnHeader.magic[0]	= 'T';
	tbnHeader.magic[1]	= 'C';

	tbnHeader.size		= pFile->size;
	tbnHeader.width		= nWidth;
	tbnHeader.height	= nHeight;

	FileWrite( fHandle, &tbnHeader, sizeof(tbnHeader) );

	// write RGB data to file
	FileWrite( fHandle, pRGBData, nWidth * nHeight * 3 ); 

	// done
	FileClose( fHandle );
	return 1;
}

////////////////////////////////////////////////////////////////////
// MENU_POPUP_THUMB Popup

typedef struct {
	menuFramework_t	*menu;

	menuText_t		Text;
	menuProgress_t	Progress;
} uiPopupThumb_t;

static uiPopupThumb_t s_popupthumb;

//
// UI_InitPopupThumb - Initializes Thumbnail Popup
//

void UI_InitPopupThumb( void )
{
	int i;
	u64 totalSize = 0;

	s_popupthumb.menu				= &uis.menus[ MENU_POPUP_THUMB ];

	s_popupthumb.menu->callback		= UI_PopupThumbCallback;
	s_popupthumb.menu->input		= NULL;
	s_popupthumb.menu->numItems		= 0;

	s_popupthumb.Text.generic.type		= MENU_CONTROL_TEXT;
	s_popupthumb.Text.generic.flags		= 0;
	s_popupthumb.Text.generic.x			= 200;
	s_popupthumb.Text.generic.y			= 100;
	s_popupthumb.Text.text				= "Generating Thumbnails";
	s_popupthumb.Text.color				= RGB(255,255,255);
	s_popupthumb.Text.size				= GR_FONT_SMALL;

	s_popupthumb.Progress.generic.type	= MENU_CONTROL_PROGRESS;
	s_popupthumb.Progress.generic.flags	= CFL_INACTIVE;
	s_popupthumb.Progress.generic.x		= 200;
	s_popupthumb.Progress.generic.y		= 130;
	s_popupthumb.Progress.width			= 200;
	s_popupthumb.Progress.height		= 20;
	s_popupthumb.Progress.textColor		= RGB(255,255,255);
	s_popupthumb.Progress.barColor		= RGBA(0,255,0, 32);

	UI_AddItemToMenu( s_popupthumb.menu, &s_popupthumb.Text );
	UI_AddItemToMenu( s_popupthumb.menu, &s_popupthumb.Progress );

	for( i = 0; i < numFiles; i++ )
		totalSize += fileInfo[i].size;

	UI_Progress_SetBounds( &s_popupthumb.Progress, 0, totalSize );
	UI_Progress_SetPosition( &s_popupthumb.Progress, 64 );
}

//
// UI_PopupThumbCallback
//

int UI_PopupThumbCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	return 0;
}

//
// Image Detail View Stuff
// 
// Image Detail View is a "submenu" for displaying the actual image
// when the user clicks on a thumbnail. It overwrites the input and 
// draw handler of the picture viewer menu but keeps all the
// thumbnails and picture viewer stuff in vram so we can
// easily switch back to the picture viewer and don't have to reload
// anything.
//

float	fZoomFactor;
int		imgOrigin_x, imgOrigin_y;
float	imgOffset_x, imgOffset_y;
int		imgWidth, imgHeight;

//
// UI_InitDetailView
//
// Initializes Detail View Stuff. Makes sure uploaded image dimensions
// do not extend 400 * 300 so it will fit in the remaining space of the
// texure buffer.
//

void UI_InitDetailView( const fileInfo_t *pImage, const char *pCurrentDir, IMG_HANDLE *pHandle )
{
	char		*pFilename;
	u8			*pRaw;
	FHANDLE		fh;
	jpgData		*pJpg;
	u8			*pImgData;
	int			nWidth, nHeight;
	float		fRatio;
	u8			*pResData;
	int			nRet;

	pFilename = (char*) malloc( strlen(pCurrentDir) + strlen(pImage->name) + 1 );
	if( pFilename == NULL )
	{
#ifdef _DEBUG
		printf("UI_InitDetailView : malloc() for pFilename failed!\n");
#endif
		return;
	}

	strcpy( pFilename, pCurrentDir );
	strcat( pFilename, pImage->name );

	// copy file into memory
	if( (pRaw = (u8*) malloc( pImage->size )) == NULL ) {
#ifdef _DEBUG
		printf("UI_InitDetailView : malloc() for pRaw failed!\n");
#endif
		return;
	}

	fh = FileOpen( pFilename, O_RDONLY );
	if( fh.fh == -1 ) {
#ifdef _DEBUG
		printf("UI_InitDetailView : Could not open file %s\n", pFilename );
#endif
		free(pFilename);
		free(pRaw);
		return;
	}

	FileRead( fh, pRaw, pImage->size );
	FileClose( fh );

	// attempt to read jpeg header from file
	if( (pJpg = jpgOpenRAW( pRaw, pImage->size, JPG_WIDTH_FIX )) == NULL ) {
#ifdef _DEBUG
		printf("UI_InitDetailView : jpgOpenRAW() failed!\n");
#endif
		free(pFilename);
		free(pRaw);
		return;
	}

	// attempt to get rgb data from jpg
	pImgData = (u8*) malloc( ((3 * pJpg->width + 3) & ~3) * pJpg->height );
	if( !pImgData ) {
#ifdef _DEBUG
		printf("UI_InitDetailView : malloc() for pImgData failed!\n");
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		return;
	}

	if( -1 == jpgReadImage( pJpg, pImgData ) ) {
#ifdef _DEBUG
		printf("UI_InitDetailView : jpgReadImage failed for %s\n", pFilename);
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		free(pImgData);
		return;
	}

	// resize image
	if( pJpg->width > pJpg->height ) {
		fRatio	= (float) pJpg->height / pJpg->width;

		// make sure width is a multiple of 4
		nWidth = pJpg->width - (pJpg->width % 4);

		if( nWidth > 400 )
			nWidth = 400;

		nHeight	= (int) (nWidth * fRatio);
		nHeight -= (nHeight %2);

	} 
	else {
		fRatio	= (float) pJpg->width / pJpg->height;
	
		nHeight = pJpg->height - (pJpg->height % 2);

		if( nHeight > 300 )
			nHeight = 300;

		nWidth	= (int) (nHeight * fRatio);
		nWidth -= (nWidth % 4);
	}

#ifdef _DEBUG
	printf("Resizing Image from %i, %i to %i, %i\n", pJpg->width, pJpg->height, nWidth, nHeight );
#endif

	// attempt to resize image
	nRet = ScaleBitmap( pImgData, pJpg->width, pJpg->height, &pResData, nWidth, nHeight );
	if( 0 == nRet ) {
#ifdef _DEBUG
		printf("UI_InitDetailView : ScaleBitmap failed for %s\n", pFilename);
#endif
		jpgClose(pJpg);
		free(pFilename);
		free(pRaw);
		free(pImgData);
		return;
	}

	// upload image to video ram
	GR_LoadImage( pHandle, pResData, nWidth, nHeight, GR_PSM_24 );

	jpgClose(pJpg);
	free(pRaw);
	free(pImgData);
	free( pFilename );

	// install custom input handler
	s_picview.menu->input		= UI_DetailViewInput;

	// make it so we receive input events as long as a button is down
	GP_SetPressMode(1);

	// set globals to default values
	fZoomFactor = 1.0f;
	imgOffset_x	= 0;
	imgOffset_y	= 0;

	bDetailView = 1;

	// trigger redraw
	UI_Refresh();
}

//
// UI_DrawDetailView - Used when viewing an image in fullscreen mode
//

void UI_DrawDetailView( void )
{
	float mul;
	int offset_y = 0;
	int offset_x = 0;

	// clear out screen
	GR_SetBkgColor( RGB(0, 0, 0) );
	GR_ClearScreen();

	// 640 / 400 = 1.6
	mul = 1.6f;

	imgOrigin_x = (640 - hDetailImg.width * mul * fZoomFactor) / 2;
	imgOrigin_y = (480 - hDetailImg.height * mul * fZoomFactor) / 2;

	imgWidth	= hDetailImg.width * mul * fZoomFactor;
	imgHeight	= hDetailImg.height * mul * fZoomFactor;

	if( imgWidth > 640 )
		offset_x = imgOffset_x * ( imgWidth - 640 ) / 2;

	if( imgHeight > 480 )
		offset_y = imgOffset_y * ( imgHeight - 480 ) / 2;

	imgOrigin_x += offset_x;
	imgOrigin_y += offset_y;

	// display fullscreen image
	GR_DrawImage( imgOrigin_x, imgOrigin_y, imgWidth, imgHeight, &hDetailImg );
}

//
// UI_DetailViewInput - Used when viewing an image in fullscreen mode
//

void UI_DetailViewInput( u32 buttons )
{
	// quit detail view
	if( buttons & PAD_TRIANGLE )
	{
		s_picview.menu->input		= NULL;

		// reset texture buffer pointer
		GR_SetTextureBuffer( hDetailImg.address );

		bDetailView = 0;

		GP_SetPressMode(0);
		UI_Refresh();
	}
	else if( (buttons & PAD_CIRCLE) || (buttons & PAD_R1) )
	{
		// zoom in
		if( fZoomFactor < 2.0f )
		{
			fZoomFactor += 0.01f;
			UI_Refresh();
		}
	}
	else if( (buttons & PAD_SQUARE) || (buttons & PAD_L1) )
	{
		// zoom out
		if( fZoomFactor > 1.0f ) {
			fZoomFactor -= 0.01f;
			UI_Refresh();
		}
	}
	else if( buttons & PAD_LEFT )
	{
		// move image left if it does not fit on the screen
		if( imgWidth > 640 )
		{
			if( imgOffset_x < 1.0f )
			{
				imgOffset_x += 0.05f;
				UI_Refresh();
			}
		}
	}
	else if( buttons & PAD_RIGHT )
	{
		// move image right if it does not fit on the screen
		if( imgWidth > 640 )
		{
			if( imgOffset_x > -1.0f )
			{
				imgOffset_x -= 0.05f;
				UI_Refresh();
			}
		}
	}
	else if( buttons & PAD_DOWN )
	{
		// move image down if it does not fit on the screen
		if( imgHeight > 480 )
		{
			if( imgOffset_y > -1.0f )
			{
				imgOffset_y -= 0.05;
				UI_Refresh();
			}
		}

	}
	else if( buttons & PAD_UP )
	{
		// move image up if it does not fit on the screen
		if( imgHeight > 480 )
		{
			if( imgOffset_y < 1.0f )
			{
				imgOffset_y += 0.05f;
				UI_Refresh();
			}
		}

	}
}
