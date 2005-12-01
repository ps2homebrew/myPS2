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
#		Code for menu controls.
#
*/

#include <tamtypes.h>
#include <ui.h>

uiStatic_t uis;

//
// UI_Thread - UI thread entry function
//

void UI_Thread( void )
{
	u32 padBtns, padBtnsOld;

	// query Gamepad
	padBtns = padBtnsOld = 0;

	while(1)
	{
		if( !GP_GetPressMode() )
		{
			// a button was pressed, inform user interface
			if( padBtns > padBtnsOld )
				UI_GamepadInput( padBtns );
		}
		else
		{
			// continuously inform user interface as long as
			// any buttons are down
			if( padBtns > 0 )
				UI_GamepadInput( padBtns );
		}

		padBtnsOld	= padBtns;
		padBtns		= GP_GetButtons();
	}
}

//
// UI_Init - Initializes User Interface
//
// Loads persistent images and then opens up main menu screen.
//

void UI_Init( void )
{
	memset( uis.menus, 0, sizeof(uis.menus) );

	// load background image and fontmap
	GR_LoadImage( &uis.hBackground, (void*)&img_aqua[0], img_aqua_w, img_aqua_h, img_aqua_psm );
	GR_LoadImage( &uis.hFontmap, (void*)&font_arial[0], font_arial_w, font_arial_h, font_arial_psm );

	uis.fontCoords = font_arial_tc;
	GR_SetFont( &uis.hFontmap, uis.fontCoords );
	GR_SetFontColor( RGB(255, 255, 255) );

	// myPS2 partition couldn't be found, but user has HDD
	// so ask user if he wants to create one.
	if( HDD_Available() == HDD_AVAIL_NO_PART ) {
		UI_SetActiveMenu( MENU_CREATEPART );
	}
	else {
		// otherwise just fire up the main menu
		UI_SetActiveMenu(MENU_MAIN);
	}
}

//
// UI_AddItemToMenu - Adds an item to a menu.
//
// Returns 1 on sucess, otherwise 0.
//

int UI_AddItemToMenu( menuFramework_t *menu, void *item )
{
	menuCommon_t *ptr;

	// menu already has too many items to add another
	if( menu->numItems > MENU_MAX_ITEMS )
		return 0;

	menu->items[ menu->numItems ] = item;

	// set parent of control to menu
	((menuCommon_t*)menu->items[ menu->numItems ])->parent = menu;

	ptr = (menuCommon_t*)item;

	// perform item specific initializations
	switch(ptr->type)
	{
		case MENU_CONTROL_DIRVIEW:
			UI_InitDirViewControl( (menuDirView_t*)item );
			break;
			
		case MENU_CONTROL_PROGRESS:
			UI_InitProgressControl( (menuProgress_t*)item );
			break;

		case MENU_CONTROL_EDITFIELD:
			UI_InitEditControl( (menuEditfield_t*)item );
			break;

		case MENU_CONTROL_SLIDER:
			UI_InitSliderControl( (menuSlider_t*)item );
			break;

		case MENU_CONTROL_RADIO:
			UI_InitRadioControl( (menuRadio_t*)item );
			break;

		case MENU_CONTROL_COMBO:
			UI_InitComboControl( (menuCombo_t*)item );
			break;
	}

	menu->numItems++;
	return 1;
}

//
// UI_DelItemFromMenu - Deletes an item from a menu.
//

void UI_DelItemFromMenu( menuFramework_t *menu, void *item )
{
	int i;
	int pos;

	if( menu->numItems <= 0 )
		return;

	for( pos = 0; pos < menu->numItems; pos++ ) {
		if( menu->items[pos] == item )
			break;
	}

	if( pos == menu->numItems ) {
#ifdef _DEBUG
		printf("UI_DelItemFromMenu : Item not found.\n");
#endif
		return;
	}

	// shift all items one forward
	for( i = pos; i < menu->numItems - 1; i++ )
		menu->items[i] = menu->items[i + 1];

	menu->numItems--;
}

//
// UI_GetSelectedItem - Returns currently selected item in menu
//

void *UI_GetSelectedItem( menuFramework_t *menu )
{
	// should never happen
	if( menu->selectedItem < 0 || menu->selectedItem >= menu->numItems )
		return NULL;

	return menu->items[ menu->selectedItem ];
}

//
// UI_SelectItemById - Selects an menu item by its id
//

void UI_SelectItemById( menuFramework_t *menu, int id )
{
	int i;

	for( i = 0; i < menu->numItems; i++ ) {
		if( ((menuCommon_t*)menu->items[i])->id == id ) {
			menu->selectedItem = i;
			return;
		}
	}
}

//
// UI_DrawTextControl - Handles Drawing of Text Control.
//

void UI_DrawTextControl( menuText_t *t )
{
	int old;

	// draw selected item green
	if( UI_GetSelectedItem( t->generic.parent ) == t ) {
		old = GR_SetFontColor( RGB(0, 255, 0 ) );
		GR_DrawTextExt( t->generic.x, t->generic.y, t->text, t->size );
		GR_SetFontColor( old );
	}
	else {

		// draw disabled text controls red unless color is forced
		if( (t->generic.flags & CFL_INACTIVE) && !(t->generic.flags & CFL_FORCECOLOR) )
			old = GR_SetFontColor( RGB(255, 0, 0) );
		else
			old = GR_SetFontColor( t->color );

		GR_DrawTextExt( t->generic.x, t->generic.y, t->text, t->size );
		GR_SetFontColor( old );
	}
}

//
// UI_DrawBitmapControl - Handles Drawing of Bitmap Control
//

void UI_DrawBitmapControl( menuBitmap_t *bm )
{
	int old;
	int r,g,b,a;
	int x, y, offset;
	
	// draw frame around bitmap control
	if( bm->generic.flags & CFL_FRAME ) {
		r = bm->frameColor & 255;
		g = (bm->frameColor >> 8) & 255;
		b = (bm->frameColor >> 16) & 255;
		a = (bm->frameColor >> 24) & 255;

		if( UI_GetSelectedItem( bm->generic.parent ) == bm )
			GR_SetDrawColor( RGB(0, 255, 0) );
		else
			GR_SetDrawColor( RGB(r, g, b) );

		GR_SetAlpha( 0.25f );
		old = GR_SetBlendMode( GR_BLEND_CONSTANT );
		GR_DrawFillRect( bm->generic.x, bm->generic.y, bm->width, bm->height );
		GR_SetBlendMode( old );
	}

	// justify image in the center
	if( bm->generic.flags & CFL_CENTER_JUSTIFY ) {
		offset = (bm->width - bm->hImage.width) / 2;
		x = bm->generic.x + offset;

		offset = (bm->height - bm->hImage.height) / 2;
		y = bm->generic.y + offset;
	}
	else {
		x = bm->generic.x;
		y = bm->generic.y;
	}

	// draw image
	GR_DrawImage( x, y, bm->hImage.width, bm->hImage.height, &bm->hImage );
}

//
// UI_DrawTriBtnControl - Handles Drawing of Triangle Button Control
//

void UI_DrawTriBtnControl( menuTriBtn_t *tb )
{
	int old;
	int r,g,b,a;
	point_t coords[3];

	r = tb->color & 255;
	g = (tb->color >> 8) & 255;
	b = (tb->color >> 16) & 255;
	a = (tb->color >> 24) & 127;

	if( UI_GetSelectedItem( tb->generic.parent ) == tb )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(r, g, b) );

	GR_SetAlpha( a / 128.0f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	switch(tb->direction)
	{
		case TRI_UP:
			coords[0].x = tb->generic.x + tb->width / 2;	
			coords[0].y = tb->generic.y;
			coords[1].x = tb->generic.x + tb->width;		
			coords[1].y = tb->generic.y + tb->height;
			coords[2].x = tb->generic.x;
			coords[2].y = tb->generic.y + tb->height;
			break;

		case TRI_RIGHT:
			coords[0].x	= tb->generic.x;
			coords[0].y = tb->generic.y;
			coords[1].x = tb->generic.x + tb->width;
			coords[1].y = tb->generic.y + tb->height / 2;
			coords[2].x = tb->generic.x;
			coords[2].y = tb->generic.y + tb->height;
			break;

		case TRI_DOWN:
			coords[0].x = tb->generic.x;
			coords[0].y = tb->generic.y;
			coords[1].x = tb->generic.x + tb->width;
			coords[1].y = tb->generic.y;
			coords[2].x = tb->generic.x + tb->width / 2;
			coords[2].y = tb->generic.y + tb->height;
			break;

		case TRI_LEFT:
			coords[0].x = tb->generic.x;
			coords[0].y = tb->generic.y + tb->height / 2;
			coords[1].x = tb->generic.x + tb->width;
			coords[1].y = tb->generic.y;
			coords[2].x = tb->generic.x + tb->width;
			coords[2].y = tb->generic.y + tb->height;
			break;
	}
	
	GR_DrawTriangle( coords[0], coords[1], coords[2] );
	GR_SetBlendMode( old );
}

//
// UI_DrawMenu - Handles Standard Menu Drawing.
//

void UI_DrawMenu( menuFramework_t *menu )
{
	int i;
	menuCommon_t *ptr;

	for( i = 0; i < menu->numItems; i++ )
	{
		ptr = (menuCommon_t*)menu->items[i];

		if( ptr->flags & CFL_INVISIBLE )
			continue;

		switch(ptr->type)
		{
			case MENU_CONTROL_TEXT:
				UI_DrawTextControl( (menuText_t*)ptr );
				break;

			case MENU_CONTROL_BITMAP:
				UI_DrawBitmapControl( (menuBitmap_t*)ptr );
				break;

			case MENU_CONTROL_TRIBUTTON:
				UI_DrawTriBtnControl( (menuTriBtn_t*)ptr );
				break;

			case MENU_CONTROL_DIRVIEW:
				UI_DrawDirViewControl( (menuDirView_t*)ptr );
				break;

			case MENU_CONTROL_PROGRESS:
				UI_DrawProgressControl( (menuProgress_t*)ptr );
				break;

			case MENU_CONTROL_EDITFIELD:
				UI_DrawEditControl( (menuEditfield_t*)ptr );
				break;

			case MENU_CONTROL_SLIDER:
				UI_DrawSliderControl( (menuSlider_t*)ptr );
				break;

			case MENU_CONTROL_RADIO:
				UI_DrawRadioControl( (menuRadio_t*)ptr );
				break;

			case MENU_CONTROL_COMBO:
				UI_DrawComboControl( (menuCombo_t*)ptr );
				break;
		}
	}
}

//
// UI_Refresh - Refreshes the screen.
//
// Called whenever menu needs to be redrawn.
//

void UI_Refresh( void )
{
	GR_ClearScreen();

	// render background image
	GR_DrawImage( 0, 0, 640, 480, &uis.hBackground );

	// if menu is a popup, draw parent menu first
	if( uis.active->parent )
		if( uis.active->parent->callback( uis.active->parent, MSG_DRAW, 0, 0 ) == 0 )
			UI_DrawMenu(uis.active->parent);

	// if callback returns 0, call default draw function
	if( uis.active->callback( uis.active, MSG_DRAW, 0, 0 ) == 0 )
		UI_DrawMenu(uis.active);

	// done drawing, swap buffers
	GR_SwapBuffers();
}

//
// UI_SetActiveMenu - Activates another menu
//

void UI_SetActiveMenu( int menuId )
{
	if( menuId < 0 || menuId >= MENU_NUM_MENUS )
		return;

	// give old menu a chance to clean up
	if( uis.active )
		uis.active->callback( uis.active, MSG_CLOSE, 0, 0 );

	uis.active = &uis.menus[ menuId ];

	// let new menu set up it's controls
	switch( menuId )
	{
		case MENU_CREATEPART:
			UI_InitCreatePart();
			break;

		case MENU_MAIN:
			UI_InitMainMenu();
			break;

		case MENU_PICVIEW:
			UI_InitPicViewMenu();
			break;

		case MENU_MANAGER:
			UI_InitManagerMenu();
			break;

		case MENU_ELFLOADER:
			UI_InitElfLoaderMenu();
			break;

		case MENU_OPTIONS:
			UI_InitOptionsMenu();
			break;

		case MENU_NETWORK:
			UI_InitNetworkMenu();
			break;

		case MENU_OPTIONS_EDIT:
			UI_InitEditMenu();
			break;

		case MENU_POPUP_THUMB:
			UI_InitPopupThumb();
			break;
	}

	// refresh everything
	UI_Refresh();
}

//
// UI_PopupOpen - Opens up a popup menu
//

void UI_PopupOpen( int popupId, menuFramework_t *parent )
{
	if( popupId < 0 || popupId >= MENU_NUM_MENUS )
		return;

	if( !parent )
		return;

	uis.menus[ popupId ].parent = parent;

	// since parent isn't really closed, it must NOT call
	// it's clean stuff yet. Just set uis.active to NULL
	// it will be re-initialized by UI_SetActiveMenu anyway
	uis.active = NULL;

	UI_SetActiveMenu( popupId );	// calls UI_Refresh already
}

//
// UI_PopupClose - Closes a popup and returns control to its parent
//

void UI_PopupClose( menuFramework_t *popup )
{
	popup->callback( popup, MSG_CLOSE, 0, 0 );

	uis.active = popup->parent;
	UI_Refresh();
}

//
// UI_GamepadInput - Called whenever a button is pressed. Hands over button
//					 mask to input handler of currently active menu or
//					 calls default input handler.
//

void UI_GamepadInput( u32 buttons )
{
	if( uis.active->input )
		uis.active->input( buttons );
	else
		UI_DefaultInputHandler( uis.active, buttons );
}

//
// UI_DefaultInput - Default Input Handler
//

void UI_DefaultInputHandler( menuFramework_t *menu, u32 buttons )
{
	menuCommon_t *item;
	int bHandled;

	if( !menu || !menu->numItems )
		return;

	item = (menuCommon_t*) UI_GetSelectedItem( menu );
	bHandled = 0;

	switch(item->type)
	{

		case MENU_CONTROL_DIRVIEW:
			bHandled = UI_InputDirViewControl( (menuDirView_t*)item, menu, buttons );
			break;

		case MENU_CONTROL_EDITFIELD:
			bHandled = UI_InputEditControl( (menuEditfield_t*)item, menu, buttons );
			break;

		case MENU_CONTROL_SLIDER:
			bHandled = UI_InputSliderControl( (menuSlider_t*)item, menu, buttons );
			break;

		case MENU_CONTROL_RADIO:
			bHandled = UI_InputRadioControl( (menuRadio_t*)item, menu, buttons );
			break;

		case MENU_CONTROL_COMBO:
			bHandled = UI_InputComboControl( (menuCombo_t*)item, menu, buttons );
			break;
	}

	if(bHandled)
		return;

	if( (buttons & PAD_DOWN) || (buttons & PAD_R2) ) 
	{
		int newSel = menu->selectedItem;

		while(1)
		{
			newSel++;

			if( newSel >= menu->numItems )
				break;

			// skip static controls
			if( ((menuCommon_t*)menu->items[newSel])->flags & CFL_INACTIVE )
				continue;
			else
				break;
		}

		if( newSel >= menu->numItems )
			return;

		menu->selectedItem = newSel;
		UI_Refresh();

		// always make sure Gamepad Pressmode is disabled when switching to
		// another control because some controls like slider or dirview may enable it
		// for faster scrolling.
		GP_SetPressMode(0);
	}
	else if( (buttons & PAD_UP) || (buttons & PAD_L2) )
	{
		int newSel = menu->selectedItem;

		while(1)
		{
			newSel--;

			if( newSel < 0 )
				break;

			// skip static controls
			if( ((menuCommon_t*)menu->items[newSel])->flags & CFL_INACTIVE )
				continue;
			else
				break;
		}

		if( newSel < 0 )
			return;

		menu->selectedItem = newSel;
		UI_Refresh();

		// always make sure Gamepad Pressmode is disabled when switching to
		// another control because some controls like slider or dirview may enable it
		// for faster scrolling.
		GP_SetPressMode(0);
	}
	else if( buttons & PAD_CROSS )
		item->parent->callback( item->parent, MSG_CONTROL, NOT_CLICKED, item->id );
}

//////////////////////////////////////////////////////////////////////////////


//
// UI_InitDirViewControl - Initializes DirView Control
//

void UI_InitDirViewControl( menuDirView_t *pDirView )
{
	int			spacing = 2;
	fileInfo_t	file;

	if( !pDirView )
		return;

	pDirView->pEntries		= NULL;
	pDirView->numEntries	= 0;
	pDirView->selEntry		= 0;
	pDirView->visEntry		= 0;
	pDirView->absPath[0]	= 0;

	// this assumes the used font has a height of 15 pixels
	pDirView->numDraw = ( pDirView->height - UI_DIRVIEW_MARGIN * 2 ) / (15 + spacing);

	if( HDD_Available() == HDD_AVAIL )
	{
		strcpy( file.name, "pfs0:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		UI_DirView_AddFile( pDirView, &file );
	}

	if( MC_Available(0) )
	{
		strcpy( file.name, "mc0:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		UI_DirView_AddFile( pDirView, &file );
	}

	if( MC_Available(1) )
	{
		strcpy( file.name, "mc1:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		UI_DirView_AddFile( pDirView, &file );
	}

	if( USB_Available() )
	{
		strcpy( file.name, "mass:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		UI_DirView_AddFile( pDirView, &file );
	}

	strcpy( file.name, "cdfs:" );

	file.flags	= FLAG_DIRECTORY;
	file.size	= 0;

	UI_DirView_AddFile( pDirView, &file );
	UI_DirView_SetCursor( pDirView, 0 );
}

//
// UI_InputDirViewControl - Handles User Input for DirView Controls 
//
// If Button is handled 1 is returned.
//

int UI_InputDirViewControl( menuDirView_t *pDirView, menuFramework_t *menu, u32 buttons )
{
	if( !pDirView )
		return 0;

	if( buttons & PAD_DOWN ) 
	{
		if( pDirView->selEntry < (pDirView->numEntries - 1) ) {

			UI_DirView_SetCursor( pDirView, pDirView->selEntry + 1 );
			UI_Refresh();

			return 1;
		}
	}
	else if( buttons & PAD_UP ) 
	{
		if( pDirView->selEntry > 0 ) {

			UI_DirView_SetCursor( pDirView, pDirView->selEntry - 1 );
			UI_Refresh();

			return 1;
		}
	}
	else if( buttons & PAD_L1 )
	{
		// jump to the top of the list
		UI_DirView_SetCursor( pDirView, 0 );
		UI_Refresh();

		return 1;
	}
	else if( buttons & PAD_R1 )
	{
		// jump to the end of the list
		UI_DirView_SetCursor( pDirView, pDirView->numEntries - 1 );
		UI_Refresh();

		return 1;
	}
	else if( buttons & PAD_CROSS ) 
	{
		// switch to new directory
		if( pDirView->pEntries[ pDirView->selEntry ].flags & FLAG_DIRECTORY ) 
		{
			// go up one level
			if( !strcmp( pDirView->pEntries[ pDirView->selEntry ].name, ".." ) )
			{
				char newPath[256];
				char *ptr;
				int offset;

				offset = strlen(pDirView->absPath) - 1;
				strncpy( newPath, pDirView->absPath, offset );
				newPath[offset] = '\0';

				ptr = strrchr( newPath, '/' );

				if( !ptr )
				{
					// path must be pfs0:, cdfs: or mc0: etc. and user attempts to switch
					// to next higher level. take him back to the 'select device'
					// screen.
					// we can just re-init the control for that
					UI_InitDirViewControl( pDirView );
					UI_Refresh();

					pDirView->generic.parent->callback( pDirView->generic.parent, MSG_CONTROL, NOT_DV_CHANGED_DIR, pDirView->generic.id );
					return 1;
				}

				offset = ptr - newPath + 1;

				strncpy( pDirView->absPath, newPath, offset );
				pDirView->absPath[offset] = '\0';
			}
			else
			{
				strcat( pDirView->absPath, pDirView->pEntries[ pDirView->selEntry ].name );
				strcat( pDirView->absPath, "/" );
			}

			UI_DirView_SetDir( pDirView, pDirView->absPath );
			UI_Refresh();

			pDirView->generic.parent->callback( pDirView->generic.parent, MSG_CONTROL, NOT_DV_CHANGED_DIR, pDirView->generic.id );
			return 1;
		}
		else
		{
			pDirView->generic.parent->callback( pDirView->generic.parent, MSG_CONTROL, NOT_DV_CLICKED_ENTRY, pDirView->generic.id );				
			return 1;
		}
	}
	else if( buttons & PAD_SQUARE )
	{
		// cannot mark '.' or '..'
		if( !strcmp( pDirView->pEntries[ pDirView->selEntry ].name, "." ) ||
			!strcmp( pDirView->pEntries[ pDirView->selEntry ].name, ".." ) )
			return 1;

		// unmark entry
		if( pDirView->pEntries[ pDirView->selEntry ].flags & FLAG_MARKED )
		{
			pDirView->pEntries[ pDirView->selEntry ].flags &= ~FLAG_MARKED;
			pDirView->generic.parent->callback( pDirView->generic.parent, MSG_CONTROL, NOT_DV_UNMARKED_ENTRY, pDirView->generic.id );
		}
		else
		{
			// mark entry
			pDirView->pEntries[ pDirView->selEntry ].flags |= FLAG_MARKED;
			pDirView->generic.parent->callback( pDirView->generic.parent, MSG_CONTROL, NOT_DV_MARKED_ENTRY, pDirView->generic.id );
		}

		return 1;
	}

	// input was not handled so return 0
	return 0;
}

//
// UI_DrawDirViewControl - Handles Drawing of DirView Control
//

void UI_DrawDirViewControl( menuDirView_t *pDirView )
{
	int spacing	= 2;
	int oldColor, color;
	int i, c;
	char str[256];
	int	strIndex;	

	// render background
	GR_SetDrawColor( pDirView->color );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode(GR_BLEND_CONSTANT);
	GR_DrawRoundRect( pDirView->generic.x, pDirView->generic.y, 
					  pDirView->width, pDirView->height );
	GR_SetBlendMode(GR_BLEND_NONE);

	for( i = pDirView->visEntry, c = 0; i < pDirView->numEntries; i++, c++ )
	{
		// can't display more items
		if( c >= pDirView->numDraw )
			break;

		color = RGB(255, 255, 255);

		// if dirview control has focus, color the selected entry green
		if( (UI_GetSelectedItem(pDirView->generic.parent) == pDirView) && (i == pDirView->selEntry) ) {
			color = RGB(0, 255, 0);
		}
		else if( pDirView->pEntries[i].flags & FLAG_MARKED ) {
			// color marked entries pink
			color = RGB(255, 64, 255);
		}
		else if( pDirView->pEntries[i].flags & FLAG_DIRECTORY ) {
			// color directories yellow
			color = RGB(255, 128, 64);
		}

		oldColor = GR_SetFontColor( color );

		// make sure string fits into dirview control
		strncpy( str, pDirView->pEntries[i].name, sizeof(str) );
		strIndex = strlen(str) - 1;

		while( GR_GetStringWidth( str, GR_FONT_SMALL ) > (pDirView->width - UI_DIRVIEW_MARGIN * 2) )
		{
			str[ strIndex ] = 0;

			if( strIndex > 1 ) {
				str[ strIndex - 1 ] = '.';
				str[ strIndex - 2 ] = '.';
			}

			strIndex--;
		}

		GR_DrawTextExt( pDirView->generic.x + UI_DIRVIEW_MARGIN,
						pDirView->generic.y + UI_DIRVIEW_MARGIN + c * (15 + spacing),
						str,
						GR_FONT_SMALL );

		GR_SetFontColor( oldColor );
	}

	// do we need to display a scroll indicator?
	if( pDirView->numEntries > pDirView->numDraw )
	{
		point_t p[3];

		if( pDirView->visEntry > 0 )
		{
			p[0].x = pDirView->generic.x + pDirView->width - 25;
			p[0].y = pDirView->generic.y + 10;

			p[1].x = pDirView->generic.x + pDirView->width - 20;
			p[1].y = pDirView->generic.y + 20;

			p[2].x = pDirView->generic.x + pDirView->width - 30;
			p[2].y = pDirView->generic.y + 20;

			GR_SetDrawColor( RGB(0, 255, 0) );
			GR_DrawTriangle( p[0], p[1], p[2] );
		}

		if( (pDirView->visEntry + pDirView->numDraw) < pDirView->numEntries )
		{	
			p[0].x = pDirView->generic.x + pDirView->width  - 25;
			p[0].y = pDirView->generic.y + pDirView->height - 10;

			p[1].x = pDirView->generic.x + pDirView->width  - 20;
			p[1].y = pDirView->generic.y + pDirView->height - 20;

			p[2].x = pDirView->generic.x + pDirView->width  - 30;
			p[2].y = pDirView->generic.y + pDirView->height - 20;

			GR_SetDrawColor( RGB(0, 255, 0) );
			GR_DrawTriangle( p[0], p[1], p[2] );
		}

	}
}

//
// UI_DirView_AddFile - Adds a fileInfo_t Entry to DirView Control
//

void UI_DirView_AddFile( menuDirView_t *pDirView, const fileInfo_t *pInfo )
{
	if( !pDirView || !pInfo )
		return;

	pDirView->pEntries = (fileInfo_t*) realloc( pDirView->pEntries, (1 + pDirView->numEntries) *
												sizeof(fileInfo_t) );

	if( !pDirView->pEntries )
		return;

	strcpy( pDirView->pEntries[ pDirView->numEntries ].name, pInfo->name );

	pDirView->pEntries[ pDirView->numEntries ].flags	= pInfo->flags;
	pDirView->pEntries[ pDirView->numEntries ].size		= pInfo->size;
	
	pDirView->numEntries++;
}

//
// UI_DirView_SetDir - Sets the current directory of DirView Control
//					   and updates pEntry file table.
//

void UI_DirView_SetDir( menuDirView_t *pDirView, const char *pDir )
{
	int i, numFiles;
	fileInfo_t *pFileInfo;

	// delete old entries
	UI_DirView_Clear( pDirView );

	// add '..' entries if browsing pfs0:/, cdfs:/, mc0:/ or mass:/ root
	// so user can switch back to 'select device' list
	if( !strcmp( pDir, "pfs0:/" ) || !strcmp( pDir, "cdfs:/" ) ||
		!strcmp( pDir, "mc0:/" )  || !strcmp( pDir, "mc1:/")   ||
		!strcmp( pDir, "mass:/" ) )
	{
		fileInfo_t f;

		f.flags		= FLAG_DIRECTORY;
		f.size		= 0;

		strcpy( f.name, ".." );
	
		UI_DirView_AddFile( pDirView, &f );
	}

	strncpy( pDirView->absPath, pDir, sizeof(pDirView->absPath) );

	pFileInfo = (fileInfo_t*) malloc( sizeof(fileInfo_t) * MAX_DIR_FILES );
	if( !pFileInfo ) {
#ifdef _DEBUG
		printf("UI_DirView_SetDir : malloc() failed.\n");
#endif
		return;
	}

	numFiles = DirGetContents( pDirView->absPath, pDirView->filter, pFileInfo, MAX_DIR_FILES );

	for( i = 0; i < numFiles; i++ )
		UI_DirView_AddFile( pDirView, &pFileInfo[i] );

	free(pFileInfo);
}

//
// UI_DirView_SetCursor - Sets the currently selected item in DirView.
//						  Also adjusts visEntry if necessary.
//

void UI_DirView_SetCursor( menuDirView_t *pDirView, int nCursor )
{
	int oldEntry;

	if( !pDirView )
		return;

	if( nCursor < 0 || nCursor >= pDirView->numEntries )
		return;

	if( nCursor < pDirView->visEntry ) {
		pDirView->visEntry = nCursor;
	}
	else if( nCursor >= (pDirView->visEntry + pDirView->numDraw) ) {
		pDirView->visEntry = nCursor - pDirView->numDraw + 1;
	}

	oldEntry = pDirView->selEntry;
	pDirView->selEntry = nCursor;

	// notify of selection change
	if( oldEntry != pDirView->selEntry ) 
		pDirView->generic.parent->callback( pDirView->generic.parent, MSG_CONTROL, NOT_DV_SEL_CHANGED, pDirView->generic.id );
}

//
// UI_DirView_Clear - Deletes all items in DirView Control
//

void UI_DirView_Clear( menuDirView_t *pDirView )
{
	if( !pDirView )
		return;

	if( pDirView->pEntries )
		free( pDirView->pEntries );

	pDirView->pEntries		= NULL;
	pDirView->numEntries	= 0;
	pDirView->selEntry		= 0;
	pDirView->visEntry		= 0;

}

//
// UI_DirView_GetDir - Returns the absolute Path
//

const char *UI_DirView_GetDir( const menuDirView_t *pDirView )
{
	if( !pDirView )
		return NULL;

	return pDirView->absPath;
}

//
// UI_DirView_MarkedCount - Returns the number of "marked" entries in DirView Control
//

int UI_DirView_MarkedCount( const menuDirView_t *pDirView )
{
	int i;
	int num = 0;

	if( !pDirView )
		return 0;

	for( i = 0; i < pDirView->numEntries; i++ )
	{
		if( pDirView->pEntries[i].flags & FLAG_MARKED )
			num++;
	}

	return num;
}

//
// UI_DirView_GetMarked - Returns a marked entry from DirView Control
//						  n ranges from 0 to (count - 1).
//

const fileInfo_t *UI_DirView_GetMarked( const menuDirView_t *pDirView, int n )
{
	int i, num = 0;

	if( !pDirView )
		return NULL;

	if( n < 0 || n >= UI_DirView_MarkedCount( pDirView ) )
		return NULL;

	for( i = 0; i < pDirView->numEntries; i++ )
	{
		if( pDirView->pEntries[i].flags & FLAG_MARKED )
		{
			if( num == n )
				return &pDirView->pEntries[i];

			num++;
		}
	}

	return NULL;
}

//
// UI_DirView_GetSelected - Returns the currently selected entry from DirView
//							Control
//

const fileInfo_t *UI_DirView_GetSelected( const menuDirView_t *pDirView )
{
	if( !pDirView )
		return NULL;

	return &pDirView->pEntries[ pDirView->selEntry ];
}

//
// UI_DirView_Refresh - Refreshes Directory View Control
//

void UI_DirView_Refresh( menuDirView_t *pDirView )
{
	if( !pDirView )
		return;

	UI_DirView_SetDir( pDirView, pDirView->absPath );
}

//////////////////////////////////////////////////////////////////////////////

//
// UI_InitProgressControl - Initialize Progress Bar Control
//

void UI_InitProgressControl( menuProgress_t *pProgress )
{
	if( !pProgress )
		return;

	UI_Progress_SetBounds( pProgress, 0, 100 );
	UI_Progress_SetPosition( pProgress, 0 );
}

//
// UI_DrawProgressControl - Handles drawing of Progress Bar Control
//

void UI_DrawProgressControl( menuProgress_t *pProgress )
{
	int old;
	int pos_x, pos_y;
	int width;
	double f;
	int percent;
	char str[32];
	int alpha;

	if( !pProgress )
		return;

	f		= (double) pProgress->pos / (pProgress->maxs - pProgress->mins);
	percent = f * 100.0f;
	width	= pProgress->width * f; 

	sprintf( str, "%i %%", percent );

	alpha = (pProgress->barColor >> 24) & 127;
	GR_SetAlpha( alpha / 128.0f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	GR_SetDrawColor( pProgress->barColor );
	GR_DrawFillRect( pProgress->generic.x, pProgress->generic.y, width, pProgress->height );

	GR_SetBlendMode( old );
	
	pos_x = pProgress->generic.x;
	pos_y = pProgress->generic.y;

	pos_x += pProgress->width / 2 - GR_GetStringWidth( str, GR_FONT_SMALL ) / 2;
	pos_y += (pProgress->height - 15) / 2;

	old = GR_SetFontColor( pProgress->textColor );
	GR_DrawTextExt( pos_x, pos_y, str, GR_FONT_SMALL ); 
	GR_SetFontColor( old );

	// draw a frame around the control
	if( pProgress->generic.flags & CFL_FRAME ) {
		GR_SetDrawColor( pProgress->frameColor );
		GR_DrawRect( pProgress->generic.x, pProgress->generic.y, pProgress->width, pProgress->height );
	}
}

//
// UI_Progess_SetBounds - Set the mins and maxs for Progress Bar
//

void UI_Progress_SetBounds( menuProgress_t *pProgress, u64 mins, u64 maxs )
{
	if( !pProgress )
		return;

	pProgress->mins	= mins;
	pProgress->maxs	= maxs;
}

//
// UI_Progress_SetPosition - Set the Position in a Progress Bar Control
//

void UI_Progress_SetPosition( menuProgress_t *pProgress, u64 pos )
{
	if( !pProgress )
		return;

	if( pos < pProgress->mins )
		pos = pProgress->mins;

	if( pos > pProgress->maxs )
		pos = pProgress->maxs;

	pProgress->pos = pos;
}

//
// UI_Progress_GetPosition - Retrieves the current "Slider" Position
//

u64 UI_Progress_GetPosition( const menuProgress_t *pProgress )
{
	if( !pProgress )
		return 0;

	return pProgress->pos;
}


//////////////////////////////////////////////////////////////////////////////
// EDITFIELD CONTROL

const char EditCTL_CharMap[] = 
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
	'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', '_', '(', ')', '.', ':', '/'
};

const int EditCTL_CharMapSize = sizeof(EditCTL_CharMap) / sizeof(const char);

#define NUM_LINE_CHARS	15

//
// UI_InitEditControl - Initializes Editfield Control
//

void UI_InitEditControl( menuEditfield_t *pEditfield )
{
	if( !pEditfield )
		return;

	pEditfield->string[0]	= 0;
	pEditfield->caretPos	= 0;
	pEditfield->chrIndex	= 0;
}

//
// UI_InputEditControl - Handles User Input for Editfield Control
//
// If Button is handled 1 is returned.
//

int UI_InputEditControl( menuEditfield_t *pEditfield, menuFramework_t *menu, u32 buttons )
{
	int nIndex;

	int		BtnSpace	= EditCTL_CharMapSize + 0;
	int		BtnDelete	= EditCTL_CharMapSize + 1;
	int		BtnCancel	= EditCTL_CharMapSize + 2;
	int		BtnOk		= EditCTL_CharMapSize + 3;

	if( !pEditfield )
		return 0;

	// select character below currently selected character
	if( buttons & PAD_DOWN )
	{
		// OK or CANCEL etc. button is selected so just hand over
		if( pEditfield->chrIndex >= EditCTL_CharMapSize ) {
			return 0;
		}
		else if( (pEditfield->chrIndex + NUM_LINE_CHARS) >= EditCTL_CharMapSize ) {
			// must be the last row so just select SPACE button
			pEditfield->chrIndex = BtnSpace;
		}
		else {
			// advance position to next row
			pEditfield->chrIndex += NUM_LINE_CHARS;
		}

		UI_Refresh();
		return 1;
	}
	else if( buttons & PAD_UP )
	{		
		// OK or CANCEL etc. button is selected
		if( pEditfield->chrIndex >= EditCTL_CharMapSize ) {

			// just select first character of last row
			nIndex = EditCTL_CharMapSize - (EditCTL_CharMapSize % NUM_LINE_CHARS);

			pEditfield->chrIndex = nIndex;
		}
		else if( (pEditfield->chrIndex - NUM_LINE_CHARS) < 0 ) {
			// must be top row so pass it over to default input handler
			return 0;
		}
		else {
			// advance position to previous row
			pEditfield->chrIndex -= NUM_LINE_CHARS;
		}

		UI_Refresh();
		return 1;
	}
	else if( buttons & PAD_LEFT )
	{
		// advance position one to the left
		if( pEditfield->chrIndex > 0 )
			pEditfield->chrIndex--;

		UI_Refresh();
		return 1;
	}
	else if( buttons & PAD_RIGHT )
	{
		// advance position one to the right
		if( pEditfield->chrIndex < BtnOk )
			pEditfield->chrIndex++;

		UI_Refresh();
		return 1;
	}
	else if( buttons & PAD_L1 )
	{
		// advanve caret position one to the left
		if( pEditfield->caretPos > 0 )
		{
			pEditfield->caretPos--;
			UI_Refresh();
		}

		return 1;
	}
	else if( buttons & PAD_R1 )
	{
		// advance caret position one to the right
		if( pEditfield->caretPos < strlen(pEditfield->string) )
		{
			pEditfield->caretPos++;
			UI_Refresh();
		}

		return 1;
	}
	else if( buttons & PAD_CROSS )
	{
		if( pEditfield->chrIndex >= EditCTL_CharMapSize )
		{
			// special treatment required
			if( pEditfield->chrIndex == BtnSpace )
			{
				UI_Editfield_AddCharAt( pEditfield, ' ', pEditfield->caretPos );
				pEditfield->caretPos++;
			}
			else if( pEditfield->chrIndex == BtnDelete )
			{
				if( pEditfield->caretPos < strlen(pEditfield->string) )
					UI_Editfield_DelCharAt( pEditfield, pEditfield->caretPos );
			}
			else if( pEditfield->chrIndex == BtnCancel )
			{
				// inform parent
				pEditfield->generic.parent->callback( pEditfield->generic.parent, MSG_CONTROL, NOT_EF_CLICKED_CANCEL, pEditfield->generic.id );
				return 0;
			}
			else if( pEditfield->chrIndex == BtnOk )
			{
				// inform parent
				pEditfield->generic.parent->callback( pEditfield->generic.parent, MSG_CONTROL, NOT_EF_CLICKED_OK, pEditfield->generic.id );
				return 0;
			}
		}
		else
		{
			UI_Editfield_AddCharAt( pEditfield, EditCTL_CharMap[ pEditfield->chrIndex ],
									pEditfield->caretPos );

			pEditfield->caretPos++;
		}

		UI_Refresh();
		return 1;
	}

	// was not handled
	return 0;
}

//
// UI_DrawEditControl - Handles Drawing of Editfield Control
//

void UI_DrawEditControl( menuEditfield_t *pEditfield )
{
	int		nHeight, nWidth, nPos;
	int		nSpace, nOffset;
	char	strChar[32], strBuf[256];
	int		i, c;
	float	alpha;
	point_t	p[3];

	// this is ugly
	int		BtnSpace	= EditCTL_CharMapSize + 0;
	int		BtnDelete	= EditCTL_CharMapSize + 1;
	int		BtnCancel	= EditCTL_CharMapSize + 2;
	int		BtnOk		= EditCTL_CharMapSize + 3;

	if( !pEditfield )
		return;

	if( !(pEditfield->generic.flags & CFL_EF_NOBKG) ) {
		alpha = ((pEditfield->color >> 24) & 127) / 128.0f;
 
		// render background
		GR_SetDrawColor( pEditfield->color );
		GR_SetAlpha( alpha );
		GR_SetBlendMode(GR_BLEND_CONSTANT);
		GR_DrawRoundRect( pEditfield->generic.x, pEditfield->generic.y, 
						  pEditfield->width, pEditfield->height );
		GR_SetBlendMode(GR_BLEND_NONE);
	}

	// draw string
	nHeight = pEditfield->generic.y + UI_EDIT_MARGIN;
	nSpace	= 22;

	GR_SetFontColor( RGB(255, 255, 255) );

	// center in the middle
	nOffset = (pEditfield->width - NUM_LINE_CHARS * nSpace) / 2;

	// TODO : Make sure displayed text does not extend control's width.
	//
	// - fixed font spacing looks ugly
	// - displaying a fixed number of characters based is stupid
	//   for non-system fonts.
	//
	//  Need to make it so characters can be drawn in "chunks"
	GR_DrawTextExt( pEditfield->generic.x + nOffset,
					nHeight,
					pEditfield->string,
					GR_FONT_SMALL );

	// draw caret thingee
	strcpy( strBuf, pEditfield->string );
	strBuf[ pEditfield->caretPos ] = '\0';

	nPos = pEditfield->generic.x + nOffset + GR_GetStringWidth( strBuf, GR_FONT_SMALL );

	GR_SetAlpha( 0.75f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_SetDrawColor( RGB( 0, 255, 0 ) );

	p[0].x = nPos;
	p[0].y = nHeight - 2;
	
	p[1].x = nPos - 4;
	p[1].y = nHeight - 10;

	p[2].x = nPos + 4;
	p[2].y = nHeight - 10;

	GR_DrawTriangle( p[0], p[1], p[2] );
	GR_SetBlendMode( GR_BLEND_NONE );

	nHeight	= nHeight + 35;

	// draw character map
	for( c = 0, i = 0; c < EditCTL_CharMapSize; c++, i++ )
	{
		strChar[0] = EditCTL_CharMap[c];
		strChar[1] = '\0';

		if( i == NUM_LINE_CHARS ) {
			nHeight += 20;
			i = 0;
		}

		// if edit control has focus, color the selected character green
		if( UI_GetSelectedItem(pEditfield->generic.parent) == pEditfield && pEditfield->chrIndex == c ) {
			GR_SetFontColor( RGB(0, 255, 0) );
		}
		else {
			GR_SetFontColor( RGB(255, 255, 255) );
		}

		GR_DrawTextExt( pEditfield->generic.x + nOffset + i * nSpace,
						nHeight, strChar,
						GR_FONT_SMALL );
	}

	// draw other buttons
	nHeight += 35;

	if( UI_GetSelectedItem(pEditfield->generic.parent) == pEditfield && pEditfield->chrIndex == BtnSpace )
		GR_SetFontColor( RGB( 0, 255, 0 ) );
	else
		GR_SetFontColor( RGB( 255, 255, 255 ) );

	GR_DrawTextExt( pEditfield->generic.x + nOffset, nHeight, "Space", GR_FONT_SMALL );

	nWidth = GR_GetStringWidth( "Space", GR_FONT_SMALL );
	nWidth = nWidth + 25;

	if( UI_GetSelectedItem(pEditfield->generic.parent) == pEditfield && pEditfield->chrIndex == BtnDelete )
		GR_SetFontColor( RGB( 0, 255, 0 ) );
	else
		GR_SetFontColor( RGB( 255, 255, 255 ) );

	GR_DrawTextExt( pEditfield->generic.x + nOffset + nWidth, nHeight, "Delete", GR_FONT_SMALL );

	nWidth += GR_GetStringWidth( "Delete", GR_FONT_SMALL );
	nWidth += 25;

	if( UI_GetSelectedItem(pEditfield->generic.parent) == pEditfield && pEditfield->chrIndex == BtnCancel )
		GR_SetFontColor( RGB( 0, 255, 0 ) );
	else
		GR_SetFontColor( RGB( 255, 255, 255 ) );

	GR_DrawTextExt( pEditfield->generic.x + nOffset + nWidth, nHeight, "Cancel", GR_FONT_SMALL );

	nWidth += GR_GetStringWidth( "Space", GR_FONT_SMALL );
	nWidth += 25;

	if( UI_GetSelectedItem(pEditfield->generic.parent) == pEditfield && pEditfield->chrIndex == BtnOk )
		GR_SetFontColor( RGB( 0, 255, 0 ) );
	else
		GR_SetFontColor( RGB( 255, 255, 255 ) );

	GR_DrawTextExt( pEditfield->generic.x + nOffset + nWidth, nHeight, "OK", GR_FONT_SMALL );

}

//
// UI_Editfield_AddCharAt - Inserts a character at the specified position
//

void UI_Editfield_AddCharAt( menuEditfield_t *pEditfield, char c, int pos )
{
	char	strBuf[256];
	int		nLength;

	if( !pEditfield )
		return;

	nLength = strlen(pEditfield->string);

	if( nLength >= 256 )
		return;

	if( pos < 0 )
		pos = 0;

	if( pos > nLength )
		pos = nLength;

	strcpy( strBuf, pEditfield->string + pos );

	pEditfield->string[ pos + 0 ] = c;
	pEditfield->string[ pos + 1 ] = 0;

	strcat( pEditfield->string, strBuf );
}

//
// UI_Editfield_DelCharAt - Deletes a character at the specified position
//

void UI_Editfield_DelCharAt( menuEditfield_t *pEditfield, int pos )
{
	char	strBuf[256];
	int		nLength;
	
	if( !pEditfield )
		return;

	nLength = strlen(pEditfield->string);

	if( nLength >= 256 )
		return;

	if( pos < 0 )
		pos = 0;

	if( pos > nLength - 1 )
		return;

	strcpy( strBuf, pEditfield->string + pos + 1 );

	pEditfield->string[ pos ] = 0;

	strcat( pEditfield->string, strBuf );
}

//
// UI_Editfield_SetString - Sets Editfield Value
//

void UI_Editfield_SetString( menuEditfield_t *pEditfield, const char *pStr )
{
	if( !pEditfield )
		return;

	strcpy( pEditfield->string, pStr );
}

//
// UI_Editfield_GetString - Retrieves Editfield Value
//

const char *UI_Editfield_GetString( const menuEditfield_t *pEditfield )
{
	if( !pEditfield )
		return NULL;

	return pEditfield->string;
}

//
// UI_Editfield_Clear - Clears Editfield
//

void UI_Editfield_Clear( menuEditfield_t *pEditfield )
{
	UI_InitEditControl( pEditfield );
}



//////////////////////////////////////////////////////////////////////////////
// SLIDER CONTROL

//
// UI_InitSliderControl - Initializes Slider Control
//

void UI_InitSliderControl( menuSlider_t *pSlider )
{
	if( !pSlider )
		return;

	UI_Slider_SetBounds( pSlider, 0, 100 );
	UI_Slider_SetPos( pSlider, 0 );
}

//
// UI_InputSliderControl - Handles User Input for Slider Control
//
// If Button is handled 1 is returned.
//

int UI_InputSliderControl( menuSlider_t *pSlider, menuFramework_t *menu, u32 buttons )
{
	static u64 delayTimer;
	static u64 lastInput;
	s64 pos;

	// if no input for over 250 msecs reset press mode
	if( tnTimeMsec() > lastInput + 250 )
		GP_SetPressMode(0);

	lastInput = tnTimeMsec();

	// L1 and R1 set the indicator to the min or max value
	if( buttons & PAD_L1 ) {
		if( UI_Slider_GetPos( pSlider ) > pSlider->mins )
		{
			UI_Slider_SetPos( pSlider, pSlider->mins );
			UI_Refresh();

			pSlider->generic.parent->callback( pSlider->generic.parent, MSG_CONTROL, NOT_SL_POS_CHANGED, pSlider->generic.id );
			return 1;
		}
	}
	else if( buttons & PAD_R1 ) {
		if( UI_Slider_GetPos( pSlider ) < pSlider->maxs )
		{
			UI_Slider_SetPos( pSlider, pSlider->maxs );
			UI_Refresh();

			pSlider->generic.parent->callback( pSlider->generic.parent, MSG_CONTROL, NOT_SL_POS_CHANGED, pSlider->generic.id );
			return 1;
		}
	}

	// Horizontal Slider
	if( pSlider->generic.flags & CFL_SL_HORIZONTAL ) {

		// delay input a little so user can finetune slider value
		if( (delayTimer > tnTimeMsec()) && (GP_GetPressMode()) )
			return 1;

		if( GP_GetPressMode() == 0 ) {
			delayTimer = tnTimeMsec() + 500;
			GP_SetPressMode(1);
		}

		if( buttons & PAD_RIGHT ) {
			pos = UI_Slider_GetPos( pSlider );

			if( pos < pSlider->maxs ) {
				pos += UI_Slider_GetStepSize(pSlider);

				UI_Slider_SetPos( pSlider, pos );
				UI_Refresh();

				// inform parent
				pSlider->generic.parent->callback( pSlider->generic.parent, MSG_CONTROL, NOT_SL_POS_CHANGED, pSlider->generic.id );
			}

			return 1;
		}
		else if( buttons & PAD_LEFT ) {
			pos = UI_Slider_GetPos( pSlider );

			if( pos > pSlider->mins ) {
				pos -= UI_Slider_GetStepSize(pSlider);

				UI_Slider_SetPos( pSlider, pos );
				UI_Refresh();

				// inform parent
				pSlider->generic.parent->callback( pSlider->generic.parent, MSG_CONTROL, NOT_SL_POS_CHANGED, pSlider->generic.id );
			}

			return 1;
		}

	}
	else {

		// delay input a little so user can finetune slider value
		if( (delayTimer > tnTimeMsec()) && (GP_GetPressMode()) )
			return 1;

		if( GP_GetPressMode() == 0 ) {
			delayTimer = tnTimeMsec() + 500;
			GP_SetPressMode(1);
		}

		// Vertical Slider
		if( buttons & PAD_UP ) {
			pos = UI_Slider_GetPos( pSlider );

			if( pos < pSlider->maxs ) {
				pos += UI_Slider_GetStepSize(pSlider);

				UI_Slider_SetPos( pSlider, pos );
				UI_Refresh();

				// inform parent
				pSlider->generic.parent->callback( pSlider->generic.parent, MSG_CONTROL, NOT_SL_POS_CHANGED, pSlider->generic.id );
			}

			return 1;
		}
		else if( buttons & PAD_DOWN ) {
			pos = UI_Slider_GetPos( pSlider );

			if( pos > pSlider->mins ) {
				pos -= UI_Slider_GetStepSize(pSlider);

				UI_Slider_SetPos( pSlider, pos );
				UI_Refresh();

				// inform parent
				pSlider->generic.parent->callback( pSlider->generic.parent, MSG_CONTROL, NOT_SL_POS_CHANGED, pSlider->generic.id );
			}

			return 1;
		}
	}

	// button was not handled
	return 0;
}

//
// UI_DrawSliderControl - Handles drawing of Slider Control
//

void UI_DrawSliderControl( menuSlider_t *pSlider )
{
	int		nBarWidth, nBarPos;
	int		nDotRadius, nDotX, nDotY;
	float	alpha;

	if( !pSlider )
		return;

	// shut up gcc
	nBarWidth = nBarPos	= 0;
	nDotRadius = nDotX = nDotY = 0;

	// draw bar
	alpha = ((pSlider->barColor >> 24) & 127) / 128.0f;

	GR_SetAlpha( alpha );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_SetDrawColor( pSlider->barColor );

	// horizontal slider
	if( pSlider->generic.flags & CFL_SL_HORIZONTAL ) {
		nBarWidth	= pSlider->height / 4;
		nBarPos		= pSlider->generic.y + (pSlider->height - nBarWidth) / 2;
		
		nDotRadius	= pSlider->height / 3;
		nDotY		= nBarPos + nBarWidth / 2;
		nDotX		= pSlider->generic.x + 
					  ((float) fabsf( pSlider->pos - pSlider->mins ) / (pSlider->maxs - pSlider->mins))
					  * pSlider->width;

		GR_DrawFillRect( pSlider->generic.x, nBarPos, pSlider->width, nBarWidth );
	}
	else {
		// vertical slider
		nBarWidth	= pSlider->width / 4;
		nBarPos		= pSlider->generic.x + (pSlider->width - nBarWidth) / 2;

		nDotRadius	= pSlider->width / 3;
		nDotX		= nBarPos + nBarWidth / 2;
		nDotY		= pSlider->generic.y + pSlider->height -
					  ((float) fabsf( pSlider->pos - pSlider->mins ) / (pSlider->maxs - pSlider->mins))
					  * pSlider->height;

		GR_DrawFillRect( nBarPos, pSlider->generic.y, nBarWidth, pSlider->height );
	}

	GR_SetBlendMode( GR_BLEND_NONE );

	// draw position indicator dot
	GR_SetAlpha( 0.8f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );

	// color green if control has focus
	if( UI_GetSelectedItem(pSlider->generic.parent) == pSlider )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(255, 255, 255) );

	GR_DrawDot( nDotX, nDotY, nDotRadius );
	GR_SetBlendMode( GR_BLEND_NONE );
}

//
// UI_Slider_SetBounds - Set the mins and maxs for Slider
//

void UI_Slider_SetBounds( menuSlider_t *pSlider, s64 mins, s64 maxs )
{
	if( !pSlider )
		return;

	pSlider->mins	= mins;
	pSlider->maxs	= maxs;
}

//
// UI_Slider_SetPos - Set the position of indicator in Slider Control
//

void UI_Slider_SetPos( menuSlider_t *pSlider, s64 pos )
{
	if( !pSlider )
		return;

	if( pos < pSlider->mins )
		pos = pSlider->mins;
	
	if( pos > pSlider->maxs )
		pos = pSlider->maxs;

	pSlider->pos = pos;
}

//
// UI_Slider_GetPos - Retrieves the position of indicator of Slider
//

s64 UI_Slider_GetPos( const menuSlider_t *pSlider )
{
	if( !pSlider )
		return 0;

	return pSlider->pos;
}

//
// UI_Slider_SetStepSize - Sets the Step Size for Slider
//

void UI_Slider_SetStepSize( menuSlider_t *pSlider, u64 stepSize )
{
	if( !pSlider )
		return;

	if( stepSize > pSlider->maxs )
		stepSize = pSlider->maxs;

	pSlider->stepSize = stepSize;
}

//
// UI_Slider_GetStepSize - Retrieves Step Size of Slider Control
//

u64 UI_Slider_GetStepSize( const menuSlider_t *pSlider )
{
	if( !pSlider )
		return 0;

	return pSlider->stepSize;
}


//////////////////////////////////////////////////////////////////////////////
// RADIOBUTTON CONTROL

//
// UI_InitRadioControl - Initializes Radiobutton Control
//

void UI_InitRadioControl( menuRadio_t *pRadio )
{
	if( !pRadio )
		return;

	// nothing to do here
}

//
// UI_DrawRadioControl - Handles drawing of radio control
//

void UI_DrawRadioControl( menuRadio_t *pRadio )
{
	int		old, color;
	int		xPos;
	int		nWidth;
	char	*pStr;

	if( !pRadio )
		return;

	// draw selected item green
	if( UI_GetSelectedItem( pRadio->generic.parent ) == pRadio ) {
		old = GR_SetFontColor( RGB(0, 255, 0 ) );
		GR_DrawTextExt( pRadio->generic.x, pRadio->generic.y, pRadio->text, GR_FONT_SMALL );
		GR_SetFontColor( old );
	}
	else {
		// draw disabled control red
		if( (pRadio->generic.flags & CFL_INACTIVE)  )
			old = GR_SetFontColor( RGB(255, 0, 0) );
		else
			old = GR_SetFontColor( pRadio->color );

		GR_DrawTextExt( pRadio->generic.x, pRadio->generic.y, pRadio->text, GR_FONT_SMALL );
		GR_SetFontColor( old );
	}

	// button state
	if( pRadio->state == RB_STATE_ENABLED ) {
		pStr	= "<YES>";
		color	= RGB(0,255,0);
	}
	else {
		pStr	= "<NO>";
		color	= RGB(255,0,0);
	}

	old = GR_SetFontColor( color );

	nWidth	= GR_GetStringWidth( pStr, GR_FONT_SMALL );

	xPos	= pRadio->generic.x + pRadio->width;
	xPos	= xPos - nWidth / 2;

	GR_DrawTextExt( xPos, pRadio->generic.y, pStr, GR_FONT_SMALL );
	GR_SetFontColor( old );
}

//
// UI_InputRadioControl - Handles User Input for Radiobutton Control
//
// Returns 1 if button was handled.
//

int UI_InputRadioControl( menuRadio_t *pRadio, menuFramework_t *menu, u32 buttons )
{
	// flip state
	if( buttons & PAD_CROSS ) {
		pRadio->state ^= 1;

		pRadio->generic.parent->callback( pRadio->generic.parent, MSG_CONTROL, NOT_RB_FLIPPED, pRadio->generic.id );
		UI_Refresh();
		return 1;
	}

	return 0;
}

//
// UI_Radio_SetState - Sets the state of a radio button
//

void UI_Radio_SetState( menuRadio_t *pRadio, int state )
{
	if( !pRadio )
		return;

	if( state != RB_STATE_DISABLED && state != RB_STATE_ENABLED )
		return;

	pRadio->state = state;
}

//
// UI_Radio_GetState - Retrieves the current state of a radio button
//

int UI_Radio_GetState( const menuRadio_t *pRadio )
{
	if( !pRadio )
		return 0;

	return pRadio->state;
}


//////////////////////////////////////////////////////////////////////////////
// COMBOBOX CONTROL

//
// UI_InitComboControl - Initializes Combobox Control
//

void UI_InitComboControl( menuCombo_t *pCombo )
{
	if( !pCombo )
		return;

	pCombo->numEntries	= 0;
	pCombo->selected	= 0;
	pCombo->pEntries	= NULL;
}

//
// UI_DrawComboControl - Handles drawing of combo control
//

void UI_DrawComboControl( menuCombo_t *pCombo )
{
	int old;
	int xPos, nWidth;

	if( !pCombo )
		return;

	// draw selected item green
	if( UI_GetSelectedItem( pCombo->generic.parent ) == pCombo )
		old = GR_SetFontColor( RGB(0, 255, 0 ) );
	else
		old = GR_SetFontColor( pCombo->color );

	if( pCombo->text )
		GR_DrawTextExt( pCombo->generic.x, pCombo->generic.y, pCombo->text, GR_FONT_SMALL );

	if( pCombo->selected >= pCombo->numEntries )
		return;

	nWidth	= GR_GetStringWidth( pCombo->pEntries[ pCombo->selected ].string, GR_FONT_SMALL );
	xPos	= pCombo->generic.x + pCombo->width;
	xPos	= xPos - nWidth / 2;

	GR_DrawTextExt( xPos, pCombo->generic.y, pCombo->pEntries[ pCombo->selected ].string,
					GR_FONT_SMALL );

	GR_SetFontColor(old);
}

//
// UI_InputComboControl - Handles User Input for Radiobutton Control
//
// Returns 1 if button was handled.
//

int UI_InputComboControl( menuCombo_t *pCombo, menuFramework_t *menu, u32 buttons )
{
	if( buttons & PAD_CROSS )
	{
		pCombo->selected++;

		if( pCombo->selected >= pCombo->numEntries )
			pCombo->selected = 0;

		pCombo->generic.parent->callback( pCombo->generic.parent, MSG_CONTROL, NOT_CB_SEL_CHANGED, pCombo->generic.id );

		UI_Refresh();
		return 1;
	}

	return 0;
}

//
// UI_Combo_AddString - Adds a string to a Combobox
//

void UI_Combo_AddString( menuCombo_t *pCombo, const char *pStr )
{
	comboEntry_t *pThis;

	if( !pCombo || !pStr )
		return;

	pCombo->pEntries = (comboEntry_t*) realloc( pCombo->pEntries, (1 + pCombo->numEntries) *
												sizeof(comboEntry_t) );

	if( !pCombo->pEntries )
		return;

	pThis = pCombo->pEntries + pCombo->numEntries;

	pThis->next = NULL;

	if( pCombo->numEntries > 0 ) {
		pThis->prev = &pCombo->pEntries[ pCombo->numEntries - 1 ];
		pCombo->pEntries[ pCombo->numEntries - 1 ].next = pThis;
	}
	else {
		pThis->prev = NULL;
	}

	pThis->string = (char*) malloc( strlen(pStr) + 1 );

	if( !pThis->string )
		return;

	strcpy( pThis->string, pStr );

	pCombo->numEntries++;

}

//
// UI_Combo_Clean - Removes all entries from combobox and frees up memory
//

void UI_Combo_Clean( menuCombo_t *pCombo )
{
	comboEntry_t *t, *p;

	p = pCombo->pEntries;

	while(p) {
		t = p;
		p = p->next;

		if(t->string)
			free(t->string);

		free(t);
	}

	pCombo->numEntries	= 0;
	pCombo->selected	= 0;
	pCombo->pEntries	= NULL;
}

//
// UI_Combo_GetSelected - Retrieves the selected string in a combobox
//

const char *UI_Combo_GetSelected( const menuCombo_t *pCombo )
{
	return pCombo->pEntries[ pCombo->selected ].string;
}

//
// UI_Combo_Select - Selects an entry in a combobox by its index
//

int UI_Combo_Select( menuCombo_t *pCombo, int nIndex )
{
	if( !pCombo )
		return -1;

	if( nIndex < 0 || nIndex >= pCombo->numEntries )
		return -1;

	pCombo->selected = nIndex;
	return 1;
}

//
// UI_Combo_SelectByName - Selects an entry in a combobox by its name
//

int UI_Combo_SelectByName( menuCombo_t *pCombo, const char *pName )
{
	int i;
	const comboEntry_t *p;

	i = 0;
	p = pCombo->pEntries;

	while(p) {
		if( !strcmp( p->string, pName ) )
			return UI_Combo_Select( pCombo, i );

		i++;
		p = p->next;
	}

	return -1;
}
