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
# Date: 11-15-2005
#
# File: myPS2 User Interface.
#		Installation menus that are displayed when 
#		myPS2 is run for the first time to let the user
#		configure stuff like HDD partition size etc.
#
*/

#include <tamtypes.h>
#include <ui.h>

//
// ***** HDD PARTITION DIALOG *****
//

#define ID_YES		0
#define ID_NO		1
#define ID_SLIDER	2
#define ID_CREATE	3
#define ID_CANCEL	4
#define ID_CONTINUE	5

typedef struct {
	menuFramework_t		menu;

	menuText_t			Yes;
	menuText_t			No;

	menuSlider_t		Slider;
	menuText_t			Create;
	menuText_t			Cancel;

	menuText_t			Continue;

	int					nPage;
	char				strMsg[256];

} uiCreatePart;

static uiCreatePart s_createPart;

menuFramework_t *UI_InitCreatePart( void )
{
	s_createPart.menu.draw			= UI_CreatePartDraw;
	s_createPart.menu.clean			= NULL;
	s_createPart.menu.input			= NULL;
	s_createPart.menu.numItems		= 0;
	s_createPart.menu.selectedItem	= 0;

	// page 1
	s_createPart.Yes.generic.type			= MENU_CONTROL_TEXT;
	s_createPart.Yes.generic.flags			= 0;
	s_createPart.Yes.generic.x				= 300;
	s_createPart.Yes.generic.y				= 260;
	s_createPart.Yes.generic.id				= ID_YES;
	s_createPart.Yes.generic.callback		= UI_EventCreatePart;
	s_createPart.Yes.text					= "Yes";
	s_createPart.Yes.color					= RGB(255, 255, 255);
	s_createPart.Yes.size					= GR_FONT_SMALL;

	s_createPart.No.generic.type			= MENU_CONTROL_TEXT;
	s_createPart.No.generic.flags			= 0;
	s_createPart.No.generic.x				= 302;
	s_createPart.No.generic.y				= 285;
	s_createPart.No.generic.id				= ID_NO;
	s_createPart.No.generic.callback		= UI_EventCreatePart;
	s_createPart.No.text					= "No";
	s_createPart.No.color					= RGB(255, 255, 255);
	s_createPart.No.size					= GR_FONT_SMALL;

	UI_AddItemToMenu( &s_createPart.menu, &s_createPart.Yes );
	UI_AddItemToMenu( &s_createPart.menu, &s_createPart.No );

	// page 2
	s_createPart.Slider.generic.type		= MENU_CONTROL_SLIDER;
	s_createPart.Slider.generic.flags		= CFL_SL_HORIZONTAL;
	s_createPart.Slider.generic.x			= 170;
	s_createPart.Slider.generic.y			= 218;
	s_createPart.Slider.generic.id			= ID_SLIDER;
	s_createPart.Slider.generic.callback	= UI_EventCreatePart;
	s_createPart.Slider.barColor			= RGBA( 82, 117, 168, 40 );
	s_createPart.Slider.width				= 300;
	s_createPart.Slider.height				= 20;

	s_createPart.Create.generic.type		= MENU_CONTROL_TEXT;
	s_createPart.Create.generic.flags		= 0;
	s_createPart.Create.generic.x			= 170;
	s_createPart.Create.generic.y			= 300;
	s_createPart.Create.generic.id			= ID_CREATE;
	s_createPart.Create.generic.callback	= UI_EventCreatePart;
	s_createPart.Create.text				= "Create Partition";
	s_createPart.Create.color				= RGB(255, 255, 255);
	s_createPart.Create.size				= GR_FONT_SMALL;

	s_createPart.Cancel.generic.type		= MENU_CONTROL_TEXT;
	s_createPart.Cancel.generic.flags		= 0;
	s_createPart.Cancel.generic.x			= 380;
	s_createPart.Cancel.generic.y			= 300;
	s_createPart.Cancel.generic.id			= ID_CANCEL;
	s_createPart.Cancel.generic.callback	= UI_EventCreatePart;
	s_createPart.Cancel.text				= "Cancel";
	s_createPart.Cancel.color				= RGB(255, 255, 255);
	s_createPart.Cancel.size				= GR_FONT_SMALL;

	// page 3
	s_createPart.Continue.generic.type		= MENU_CONTROL_TEXT;
	s_createPart.Continue.generic.flags		= 0;
	s_createPart.Continue.generic.x			= 228;
	s_createPart.Continue.generic.y			= 280;
	s_createPart.Continue.generic.id		= ID_CONTINUE;
	s_createPart.Continue.generic.callback	= UI_EventCreatePart;
	s_createPart.Continue.text				= "Continue to main menu";
	s_createPart.Continue.color				= RGB(255, 255, 255);
	s_createPart.Continue.size				= GR_FONT_SMALL;

	s_createPart.nPage = 0;

	return &s_createPart.menu;
}

void UI_CreatePartDraw( void )
{
	int		len;
	int		origin_x;
	int		x, y;
	char	*p;
	char	strBuf[64];

	// draw menu controls
	UI_DrawMenu( &s_createPart.menu );

	// draw all the static text
	origin_x = 120;

	// draw dialog background
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( origin_x, 140, 400, 200 );
	GR_SetBlendMode( GR_BLEND_NONE );

	p	= "myPS2 Installation";
	len	= GR_GetStringWidth( p, GR_FONT_SMALL );
	x	= origin_x + (400 - len) / 2;
	y	= 160;

	GR_SetFontColor( RGB(255, 0, 0) );
	GR_DrawTextExt( x, y, p, GR_FONT_SMALL );

	// first page
	if( s_createPart.nPage == 0 )
	{
		p	= "There is no myPS2 partition on your HDD";
		len	= GR_GetStringWidth( p, GR_FONT_SMALL );
		x	= origin_x + (400 - len) / 2;
		y	= y + 40;
	
		GR_SetFontColor( RGB( 255, 255, 255 ) );
		GR_DrawTextExt( x, y, p, GR_FONT_SMALL );

		p	= "Would you like to create one now ? ";
		len	= GR_GetStringWidth( p,GR_FONT_SMALL );
		x	= origin_x + (400 - len) / 2;
		y	= y + 20;

		GR_DrawTextExt( x, y, p, GR_FONT_SMALL );
	}
	else if( s_createPart.nPage == 1 )
	{
		// second page
		p	= "Please select a size for the myPS2 partition";
		len	= GR_GetStringWidth( p, GR_FONT_SMALL );
		x	= origin_x + (400 - len) / 2;
		y	= y + 20;

		GR_SetFontColor( RGB( 255, 255, 255 ) );
		GR_DrawTextExt( x, y, p, GR_FONT_SMALL );

		sprintf( strBuf, "Partition Size : %i MB", (u32)UI_Slider_GetPos(&s_createPart.Slider) );
		len	= GR_GetStringWidth( strBuf, GR_FONT_SMALL );
		x	= origin_x + (400 - len) / 2;
		y	= 260;

		GR_DrawTextExt( x, y, strBuf, GR_FONT_SMALL );
	}
	else if( s_createPart.nPage == 2 )
	{
		// third page
		p	= s_createPart.strMsg;
		len	= GR_GetStringWidth( p, GR_FONT_SMALL );
		x	= origin_x + (400 - len) / 2;
		y	= y + 40;

		GR_SetFontColor( RGB( 255, 255, 255 ) );
		GR_DrawTextExt( x, y, p, GR_FONT_SMALL );
	}
}

void UI_EventCreatePart( void *pItem, int nCode )
{
	int	nRet;
	u64 nPartSize;

	switch( ((menuCommon_t*)pItem)->id )
	{
		// switch to page 2 of dialog
		case ID_YES:
			s_createPart.nPage++;

			// can now remove page 1 controls
			UI_DelItemFromMenu( &s_createPart.menu, &s_createPart.Yes );
			UI_DelItemFromMenu( &s_createPart.menu, &s_createPart.No );

			// add page 2 controls
			UI_AddItemToMenu( &s_createPart.menu, &s_createPart.Slider );
			UI_AddItemToMenu( &s_createPart.menu, &s_createPart.Create );
			UI_AddItemToMenu( &s_createPart.menu, &s_createPart.Cancel );
			UI_SelectItemById( &s_createPart.menu, ID_SLIDER );

			// setup slider
			UI_Slider_SetBounds( &s_createPart.Slider, 0, HDD_GetFreeSpace() );
			UI_Slider_SetPos( &s_createPart.Slider, 0 );
			UI_Slider_SetStepSize( &s_createPart.Slider, 50 );

			// refresh everything
			UI_Refresh();
			break;

		case ID_NO:
		case ID_CANCEL:
		case ID_CONTINUE:
			UI_SetActiveMenu( MENU_ID_MAIN );
			break;

		// switch to page 3 of dialog
		case ID_CREATE:
			s_createPart.nPage++;

			// attempt to create partition
			nPartSize = UI_Slider_GetPos( &s_createPart.Slider );

			// normal message displayed
			sprintf( s_createPart.strMsg, "Partition created successfully (%i MB)", (u32) nPartSize  );

			nRet = HDD_CreatePartition( PARTITION_NAME, nPartSize );
			
			// some error occured
			if( !nRet ) {
				strcpy( s_createPart.strMsg, "Could not create partition" );
			}
			else {

				// created partition, now try to mount it
				nRet = HDD_MountPartition( "pfs0:", PARTITION_NAME );

				// could not mount partition, something must have gone wrong
				if( !nRet )
					strcpy( s_createPart.strMsg, "Could not mount created partition" );
			}

			// can now remove page 2 controls
			UI_DelItemFromMenu( &s_createPart.menu, &s_createPart.Slider );
			UI_DelItemFromMenu( &s_createPart.menu, &s_createPart.Create );
			UI_DelItemFromMenu( &s_createPart.menu, &s_createPart.Cancel );

			// add page 3 controls
			UI_AddItemToMenu( &s_createPart.menu, &s_createPart.Continue );
			UI_SelectItemById( &s_createPart.menu, ID_CONTINUE );

			// refresh everything
			UI_Refresh();
			break;
	}
}
