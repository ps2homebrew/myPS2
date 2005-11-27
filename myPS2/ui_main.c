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
# Date: 10-14-2005
#
# File: myPS2 User Interface.
#		Main Menu Code.
#
*/

#include <tamtypes.h>
#include <ui.h>

#define ID_PICTUREVIEWER	1
#define ID_MANAGER			2
#define ID_ELFLOADER		3
#define ID_OPTIONS			4

typedef struct {
	menuFramework_t	*menu;

	menuText_t		PicViewer;
	menuText_t		Manager;
	menuText_t		ElfLoader;
	menuText_t		Options;

	IMG_HANDLE		hImgLogo;
} uiMainMenu_t;

static uiMainMenu_t s_main;

//
// UI_InitMainMenu - Initializes main menu controls.
//

void UI_InitMainMenu( void )
{
	s_main.menu							= &uis.menus[ MENU_MAIN ];

	s_main.menu->callback				= UI_MainCallback;
	s_main.menu->input					= NULL;
	s_main.menu->numItems				= 0;

	// picture view is selected by default
	s_main.menu->selectedItem			= 0;

	s_main.PicViewer.generic.type		= MENU_CONTROL_TEXT;
	s_main.PicViewer.generic.flags		= 0;
	s_main.PicViewer.generic.x			= 35;
	s_main.PicViewer.generic.y			= 70;
	s_main.PicViewer.generic.id			= ID_PICTUREVIEWER;
	s_main.PicViewer.text				= "My Pictures";
	s_main.PicViewer.size				= GR_FONT_SMALL;
	s_main.PicViewer.color				= RGB(255, 255, 255);

	s_main.Manager.generic.type			= MENU_CONTROL_TEXT;
	s_main.Manager.generic.flags		= 0;
	s_main.Manager.generic.x			= 35;
	s_main.Manager.generic.y			= 90;
	s_main.Manager.generic.id			= ID_MANAGER;
	s_main.Manager.text					= "My Files";
	s_main.Manager.size					= GR_FONT_SMALL;
	s_main.Manager.color				= RGB(255, 255, 255);

	s_main.ElfLoader.generic.type		= MENU_CONTROL_TEXT;
	s_main.ElfLoader.generic.flags		= 0;
	s_main.ElfLoader.generic.x			= 35;
	s_main.ElfLoader.generic.y			= 110;
	s_main.ElfLoader.generic.id			= ID_ELFLOADER;
	s_main.ElfLoader.text				= "My Programs";
	s_main.ElfLoader.size				= GR_FONT_SMALL;
	s_main.ElfLoader.color				= RGB(255, 255, 255);

	s_main.Options.generic.type			= MENU_CONTROL_TEXT;
	s_main.Options.generic.flags		= 0;
	s_main.Options.generic.x			= 35;
	s_main.Options.generic.y			= 140;
	s_main.Options.generic.id			= ID_OPTIONS;
	s_main.Options.text					= "Options";
	s_main.Options.size					= GR_FONT_SMALL;
	s_main.Options.color				= RGB(255, 255, 255);

	// add items to menu container
	UI_AddItemToMenu( s_main.menu, &s_main.PicViewer );
	UI_AddItemToMenu( s_main.menu, &s_main.Manager );
	UI_AddItemToMenu( s_main.menu, &s_main.ElfLoader );
	UI_AddItemToMenu( s_main.menu, &s_main.Options );

	// precache images
	GR_LoadImage( &s_main.hImgLogo, (void*)&img_logo[0], img_logo_w, img_logo_h, img_logo_psm );
}

//
// UI_MainCallback - Main Menu Callback
//

int UI_MainCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	switch( nMsg )
	{
		case MSG_CONTROL:
			switch( sParam )
			{
				case ID_PICTUREVIEWER:
					UI_SetActiveMenu(MENU_PICVIEW);
					return 1;

				case ID_MANAGER:
					UI_SetActiveMenu(MENU_MANAGER);
					return 1;

				case ID_ELFLOADER:
					UI_SetActiveMenu(MENU_ELFLOADER);
					return 1;

				case ID_OPTIONS:
					UI_SetActiveMenu(MENU_OPTIONS);
					return 1;
			}
			break;

		case MSG_DRAW:
			UI_MainDraw();
			return 1;

		// reset texture buffer pointer
		case MSG_CLOSE:
			GR_SetTextureBuffer( s_main.hImgLogo.address );
			return 1;
	}

	return 0;
}

//
// UI_MainDraw - Draws some status information
//

void UI_MainDraw( void )
{
	char	strDate[128];
	int		offset = 5;

	// draw sidebar
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	// draw standard controls
	UI_DrawMenu( s_main.menu );

	// draw myPS2 logo
	GR_SetBlendMode( GR_BLEND_SRC_ALPHA );
	GR_DrawImage( 190, 50, s_main.hImgLogo.width, s_main.hImgLogo.height, &s_main.hImgLogo );
	GR_SetBlendMode( GR_BLEND_NONE );

	GR_SetFontColor( RGB(255,255,255) );

	// display information about version etc.
	GR_DrawTextExt( 210, 150 + offset, "Build Version", GR_FONT_SMALL );
	GR_DrawTextExt( 210, 170 + offset, "Build Date", GR_FONT_SMALL );

	sprintf( strDate, "%s , %s", __TIME__, __DATE__ );


	GR_DrawTextExt( 350, 150 + offset, MYPS2_VERSION, GR_FONT_SMALL );
	GR_DrawTextExt( 350, 170 + offset, strDate, GR_FONT_SMALL );

	GR_DrawTextExt( 210, 200 + offset, "Written by Torben \"ntba2\" Koenke", GR_FONT_SMALL );

	GR_DrawTextExt( 210, 230 + offset, "Newest version can be found at", GR_FONT_SMALL );

	GR_SetFontColor( RGB(0,255,0) );
	GR_DrawTextExt( 210, 245 + offset, "http://www.ntba2.de/", GR_FONT_SMALL );
	GR_SetFontColor( RGB(255,255,255) );

	GR_DrawTextExt( 210, 270 + offset, "Thanks to", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 290 + offset, "- all the people that made PS2SDK", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 305 + offset, "- Dreamtime", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 320 + offset, "- Ivaylo Byalkov", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 335 + offset, "- Pixel", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 350 + offset, "- Ole", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 365 + offset, "- Hiryu and Sjeep", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 380 + offset, "- Jesper Svennevid", GR_FONT_SMALL );
	GR_DrawTextExt( 220, 395 + offset, "- EP", GR_FONT_SMALL );
}
