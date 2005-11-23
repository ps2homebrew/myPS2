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
# Date: 11-18-2005
#
# File: myPS2 User Interface.
#		Options menu code.
#
*/

#include <tamtypes.h>
#include <ui.h>

typedef struct {
	menuFramework_t	*menu;

} uiOptionsMenu_t;

static uiOptionsMenu_t	s_options;

//
// UI_InitOptionsMenu - Initializes options menu controls
//

void UI_InitOptionsMenu( void )
{
	s_options.menu				= &uis.menus[ MENU_OPTIONS ];

	s_options.menu->callback	= UI_OptionsCallback;
	s_options.menu->input		= NULL;
	s_options.menu->numItems	= 0;
}

//
// UI_OptionsCallback
//

int UI_OptionsCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	return 0;
}

//
// UI_OptionsDraw - Custom draw function for options menu
//

void UI_OptionsDraw( void )
{
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_DrawRoundRect( 175, 25, 400, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

}
