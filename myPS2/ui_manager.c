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
# Date: 11-5-2005
#
# File: myPS2 User Interface.
#		File Manager Menu Code.
#
*/

#include <tamtypes.h>
#include <ui.h>

#define ID_MAKEDIR			1
#define ID_RENAME			2
#define ID_DELETE			3
#define ID_SOURCE			4
#define ID_DEST				5
#define ID_COPY				6
#define ID_GOBACK			7
#define ID_DIRVIEW_SOURCE	8
#define ID_DIRVIEW_DEST		9
#define ID_EDIT_MKDIR		10
#define ID_EDIT_RENAME_DIR	11

typedef struct {
	menuFramework_t	menu;

	menuText_t		MakeDir;
	menuText_t		Rename;
	menuText_t		Delete;

	menuText_t		Source;
	menuText_t		Dest;

	menuText_t		Copy;
	menuText_t		GoBack;

	menuDirView_t	DirViewSrc;
	menuDirView_t	DirViewDest;

	menuProgress_t	Progress;
	char			currentFile[256];	// display file we are currently copying

	menuEditfield_t	EditMkdir;
	menuEditfield_t	EditRename;

	// for file renaming
	char			oldName[256];
	char			newName[256];

} uiManagerMenu_t;

static uiManagerMenu_t s_manager;

//
// UI_InitManagerMenu - Initializes File Manager Controls
//

menuFramework_t *UI_InitManagerMenu( void )
{
	s_manager.menu.draw		= UI_ManagerDraw;
	s_manager.menu.input	= NULL;
	s_manager.menu.clean	= UI_ManagerCleanup;
	s_manager.menu.numItems	= 0;

	s_manager.menu.selectedItem = 0;

	s_manager.Source.generic.type			= MENU_CONTROL_TEXT;
	s_manager.Source.generic.flags			= 0;
	s_manager.Source.generic.x				= 35;
	s_manager.Source.generic.y				= 70;
	s_manager.Source.generic.id				= ID_SOURCE;
	s_manager.Source.generic.callback		= UI_EventManagerMenu;
	s_manager.Source.text					= "Select Source";
	s_manager.Source.size					= GR_FONT_SMALL;
	s_manager.Source.color					= RGB(255, 255, 255);

	s_manager.Dest.generic.type				= MENU_CONTROL_TEXT;
	s_manager.Dest.generic.flags			= 0;
	s_manager.Dest.generic.x				= 35;
	s_manager.Dest.generic.y				= 90;;
	s_manager.Dest.generic.id				= ID_DEST;
	s_manager.Dest.generic.callback			= UI_EventManagerMenu;
	s_manager.Dest.text						= "Select Dest";
	s_manager.Dest.size						= GR_FONT_SMALL;
	s_manager.Dest.color					= RGB(255, 255, 255);

	s_manager.MakeDir.generic.type			= MENU_CONTROL_TEXT;
	s_manager.MakeDir.generic.flags			= CFL_INACTIVE;
	s_manager.MakeDir.generic.x				= 35;
	s_manager.MakeDir.generic.y				= 120;
	s_manager.MakeDir.generic.id			= ID_MAKEDIR;
	s_manager.MakeDir.generic.callback		= UI_EventManagerMenu;
	s_manager.MakeDir.text					= "Make Dir";
	s_manager.MakeDir.size					= GR_FONT_SMALL;
	s_manager.MakeDir.color					= RGB(255, 255, 255);

	s_manager.Rename.generic.type			= MENU_CONTROL_TEXT;
	s_manager.Rename.generic.flags			= CFL_INACTIVE;
	s_manager.Rename.generic.x				= 35;
	s_manager.Rename.generic.y				= 140;
	s_manager.Rename.generic.id				= ID_RENAME;
	s_manager.Rename.generic.callback		= UI_EventManagerMenu;
	s_manager.Rename.text					= "Rename";
	s_manager.Rename.size					= GR_FONT_SMALL;
	s_manager.Rename.color					= RGB(255, 255, 255);

	s_manager.Delete.generic.type			= MENU_CONTROL_TEXT;
	s_manager.Delete.generic.flags			= CFL_INACTIVE;
	s_manager.Delete.generic.x				= 35;
	s_manager.Delete.generic.y				= 160;
	s_manager.Delete.generic.id				= ID_DELETE;
	s_manager.Delete.generic.callback		= UI_EventManagerMenu;
	s_manager.Delete.text					= "Delete";
	s_manager.Delete.size					= GR_FONT_SMALL;
	s_manager.Delete.color					= RGB(255, 255, 255);

	s_manager.Copy.generic.type				= MENU_CONTROL_TEXT;
	s_manager.Copy.generic.flags			= CFL_INACTIVE;
	s_manager.Copy.generic.x				= 35;
	s_manager.Copy.generic.y				= 190;
	s_manager.Copy.generic.id				= ID_COPY;
	s_manager.Copy.generic.callback			= UI_EventManagerMenu;
	s_manager.Copy.text						= "Start Copying";
	s_manager.Copy.size						= GR_FONT_SMALL;
	s_manager.Copy.color					= RGB(255, 255, 255);

	s_manager.GoBack.generic.type			= MENU_CONTROL_TEXT;
	s_manager.GoBack.generic.flags			= 0;
	s_manager.GoBack.generic.x				= 35;
	s_manager.GoBack.generic.y				= 210;
	s_manager.GoBack.generic.id				= ID_GOBACK;
	s_manager.GoBack.generic.callback		= UI_EventManagerMenu;
	s_manager.GoBack.text					= "Go Back";
	s_manager.GoBack.size					= GR_FONT_SMALL;
	s_manager.GoBack.color					= RGB(255, 255, 255);

	s_manager.DirViewSrc.generic.type		= MENU_CONTROL_DIRVIEW;
	s_manager.DirViewSrc.generic.flags		= 0;
	s_manager.DirViewSrc.generic.x			= 200;
	s_manager.DirViewSrc.generic.y			= 40;
	s_manager.DirViewSrc.generic.id			= ID_DIRVIEW_SOURCE;
	s_manager.DirViewSrc.generic.callback	= UI_EventManagerMenu;
	s_manager.DirViewSrc.width				= 400;
	s_manager.DirViewSrc.height				= 200;
	s_manager.DirViewSrc.color				= RGB(81, 112, 164);

	s_manager.DirViewDest.generic.type		= MENU_CONTROL_DIRVIEW;
	s_manager.DirViewDest.generic.flags		= CFL_INACTIVE | CFL_INVISIBLE;
	s_manager.DirViewDest.generic.x			= 200;
	s_manager.DirViewDest.generic.y			= 40;
	s_manager.DirViewDest.generic.id		= ID_DIRVIEW_DEST;
	s_manager.DirViewDest.generic.callback	= UI_EventManagerMenu;
	s_manager.DirViewDest.width				= 400;
	s_manager.DirViewDest.height			= 200;
	s_manager.DirViewDest.color				= RGB(81, 112, 164);

	s_manager.Progress.generic.type			= MENU_CONTROL_PROGRESS;
	s_manager.Progress.generic.flags		= CFL_INACTIVE;
	s_manager.Progress.generic.x			= 220;
	s_manager.Progress.generic.y			= 275;
	s_manager.Progress.width				= 360;
	s_manager.Progress.height				= 20;
	s_manager.Progress.barColor				= RGBA(0,255,0, 32);
	s_manager.Progress.textColor			= RGB(255,255,255);

	s_manager.EditMkdir.generic.type		= MENU_CONTROL_EDITFIELD;
	s_manager.EditMkdir.generic.flags		= CFL_INACTIVE | CFL_INVISIBLE;
	s_manager.EditMkdir.generic.x			= 200;
	s_manager.EditMkdir.generic.y			= 250;
	s_manager.EditMkdir.generic.id			= ID_EDIT_MKDIR;
	s_manager.EditMkdir.generic.callback	= UI_EventManagerMenu;
	s_manager.EditMkdir.width				= 400;
	s_manager.EditMkdir.height				= 200;
	s_manager.EditMkdir.color				= RGB(81, 112, 164);

	s_manager.EditRename.generic.type		= MENU_CONTROL_EDITFIELD;
	s_manager.EditRename.generic.flags		= CFL_INACTIVE | CFL_INVISIBLE;
	s_manager.EditRename.generic.x			= 200;
	s_manager.EditRename.generic.y			= 250;
	s_manager.EditRename.generic.id			= ID_EDIT_RENAME_DIR;
	s_manager.EditRename.generic.callback	= UI_EventManagerMenu;
	s_manager.EditRename.width				= 400;
	s_manager.EditRename.height				= 200;
	s_manager.EditRename.color				= RGB(81, 112, 164);

	// add items to menu container
	UI_AddItemToMenu( &s_manager.menu, &s_manager.Source );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.Dest );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.MakeDir );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.Rename );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.Delete );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.Copy );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.GoBack );

	UI_AddItemToMenu( &s_manager.menu, &s_manager.DirViewSrc );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.DirViewDest );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.Progress );

	UI_AddItemToMenu( &s_manager.menu, &s_manager.EditMkdir );
	UI_AddItemToMenu( &s_manager.menu, &s_manager.EditRename );

	memset( s_manager.currentFile, 0, sizeof(s_manager.currentFile) );

	return &s_manager.menu;
}

//
// UI_ManagerCleanup - Clean up
//

void UI_ManagerCleanup( void )
{
	// clear up dirview controls
	UI_DirView_Clear( &s_manager.DirViewSrc );
	UI_DirView_Clear( &s_manager.DirViewDest );
}

//
// UI_ManagerDraw - Custom Draw Function
//

void UI_ManagerDraw( void )
{
	char		buf[256];
	const char	*p;
	int			numSelected;
	int			nMaxWidth, nIndex;

	// draw sidebar
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	// draw controls
	UI_DrawMenu( &s_manager.menu );

	// draw static text
	GR_SetFontColor( RGB(255, 255, 255) );

	if( s_manager.DirViewSrc.generic.flags & CFL_INVISIBLE )
		GR_DrawTextExt( 210, 25, "Select Destination Directory", GR_FONT_SMALL );
	else
		GR_DrawTextExt( 210, 25, "Select Source Files", GR_FONT_SMALL );

	// only draw status text if editfields are not up
	if( !(s_manager.EditMkdir.generic.flags & CFL_INVISIBLE) )
		return;

	if( !(s_manager.EditRename.generic.flags & CFL_INVISIBLE) )
		return;

	GR_DrawTextExt( 210, 250, "Progress", GR_FONT_SMALL );
	GR_DrawTextExt( 210, 325, "Source Path", GR_FONT_SMALL );
	GR_DrawTextExt( 210, 385, "Destination Path", GR_FONT_SMALL );
	GR_DrawTextExt( 450, 325, "Selected Files", GR_FONT_SMALL );
	GR_DrawTextExt( 450, 385, "Currently Copying", GR_FONT_SMALL );

	// source path
	p = UI_DirView_GetDir( &s_manager.DirViewSrc );
	if( p[0] == 0 ) {
		GR_SetFontColor( RGB(255,0,0) );
		strcpy( buf, "Not Selected" );
	}
	else {
		GR_SetFontColor( RGB(0,255,0) );
		strcpy( buf, p );
	}

	// make sure path string is truncated if it's too long
	nMaxWidth	= 205;
	nIndex		= strlen(buf) - 1;

	while( GR_GetStringWidth( buf, GR_FONT_SMALL ) > nMaxWidth ) {
		buf[ nIndex ] = 0;

		if( nIndex > 1 ) {
			buf[ nIndex - 1 ] = '.';
			buf[ nIndex - 2 ] = '.';
		}

		nIndex--;
	}

	GR_DrawTextExt( 210, 350, buf, GR_FONT_SMALL );

	// dest path
	p = UI_DirView_GetDir( &s_manager.DirViewDest );
	if( p[0] == 0 ) {
		GR_SetFontColor( RGB(255,0,0) );
		strcpy( buf, "Not Selected" );
	}
	else {
		GR_SetFontColor( RGB(0,255,0) );
		strcpy( buf, p );
	}

	// make sure dest string is truncated if it's too long
	nMaxWidth	= 205;
	nIndex		= strlen(buf) -1;

	while( GR_GetStringWidth( buf, GR_FONT_SMALL ) > nMaxWidth ) {
		buf[ nIndex ] = 0;

		if( nIndex > 1 ) {
			buf[ nIndex - 1 ] = '.';
			buf[ nIndex - 2 ] = '.';
		}

		nIndex--;
	}

	GR_DrawTextExt( 210, 410, buf, GR_FONT_SMALL );

	if( s_manager.currentFile[0] == '\0' ) {
		GR_SetFontColor( RGB(255,0,0) );
		GR_DrawTextExt( 450, 410, "None", GR_FONT_SMALL );
	}
	else {
		GR_SetFontColor( RGB(255,255,255) );

		// make sure string is truncated if it's too long
		strcpy( buf, s_manager.currentFile );

		nMaxWidth	= 155;
		nIndex		= strlen(buf) - 1;

		while( GR_GetStringWidth( buf, GR_FONT_SMALL ) > nMaxWidth ) {
			buf[ nIndex ] = 0;

			if( nIndex > 1 ) {
				buf[ nIndex - 1 ] = '.';
				buf[ nIndex - 2 ] = '.';
			}

			nIndex--;
		}

		GR_DrawTextExt( 450, 410, buf, GR_FONT_SMALL );
	}

	numSelected = UI_DirView_MarkedCount( &s_manager.DirViewSrc );

	if( numSelected == 0 )
		GR_SetFontColor( RGB(255,0,0) );
	else
		GR_SetFontColor( RGB(0,255,0) );

	sprintf( buf, "%i", numSelected );
	GR_DrawTextExt( 450, 350, buf, GR_FONT_SMALL );
}

//
// UI_EventManagerMenu
//

void UI_EventManagerMenu( void *pItem, int nCode )
{
	int					id;
	int					nEnable;
	const char			*pStr;
	const char			*pPath;
	char				strDir[256];
	const fileInfo_t	*pFileInfo;

	if(!pItem)
		return;

	id = ((menuCommon_t*)pItem)->id;

	// need to enable "Start Copying" button?
	if( id == ID_DIRVIEW_SOURCE || id == ID_DIRVIEW_DEST )
	{
		nEnable = 1;

		if( UI_DirView_GetDir(&s_manager.DirViewSrc)[0] == '\0' )
			nEnable = 0;

		pStr = UI_DirView_GetDir(&s_manager.DirViewDest);

		// can't copy to cd/dvd
		if( pStr[0] == '\0' || strstr( pStr, "cdfs:" ) != NULL )
			nEnable = 0;

		// no files selected
		if( !UI_DirView_MarkedCount(&s_manager.DirViewSrc) )
			nEnable = 0;

		if( nEnable )
			s_manager.Copy.generic.flags &= ~CFL_INACTIVE;
		else
			s_manager.Copy.generic.flags |= CFL_INACTIVE;
	}

	// need to enable "Make Dir" button?
	if( id == ID_DIRVIEW_SOURCE )
	{
		nEnable = 1;

		pStr = UI_DirView_GetDir( &s_manager.DirViewSrc );

		if( strstr( pStr, "cdfs:" ) || pStr[0] == '\0' )
			nEnable = 0;

		if( nEnable )
			s_manager.MakeDir.generic.flags &= ~CFL_INACTIVE;
		else
			s_manager.MakeDir.generic.flags |= CFL_INACTIVE;

		// may have also to close makedir editfield it it's up
		if( !(s_manager.EditMkdir.generic.flags & CFL_INVISIBLE) )
		{
			// need to hide it
			if( !nEnable )
			{
				s_manager.EditMkdir.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

				// enable progress bar
				s_manager.Progress.generic.flags &= ~CFL_INVISIBLE;
			}
		}
	}

	// need to enable "Delete" button?
	if( id == ID_DIRVIEW_SOURCE )
	{
		nEnable = 1;

		pStr = UI_DirView_GetDir(&s_manager.DirViewSrc);

		// can't delete from cd/dvd
		if( pStr[0] == '\0' || strstr( pStr, "cdfs:" ) != NULL )
			nEnable = 0;

		// no files selected
		if( !UI_DirView_MarkedCount(&s_manager.DirViewSrc) )
			nEnable = 0;

		if( nEnable )
			s_manager.Delete.generic.flags &= ~CFL_INACTIVE;
		else
			s_manager.Delete.generic.flags |= CFL_INACTIVE;
	}

	// need to enable "Rename" button ?
	if( id == ID_DIRVIEW_SOURCE )
	{
		nEnable = 1;

		pStr = UI_DirView_GetDir(&s_manager.DirViewSrc);

		// can't rename files on cd/dvd
		if( pStr[0] == '\0' || strstr( pStr, "cdfs:" ) != NULL )
			nEnable = 0;

		// can only rename one selected file
		if( UI_DirView_MarkedCount(&s_manager.DirViewSrc) != 1 )
			nEnable = 0;

		if( nEnable )
			s_manager.Rename.generic.flags &= ~CFL_INACTIVE;
		else
			s_manager.Rename.generic.flags |= CFL_INACTIVE;

		// may also have to close rename editfield if it's visible
		if( !(s_manager.EditRename.generic.flags & CFL_INVISIBLE) )
		{
			if( !nEnable )
			{
				s_manager.EditRename.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

				// show progress bar
				s_manager.Progress.generic.flags &= ~CFL_INVISIBLE;
			}
		}
	}

	switch( id )
	{
		case ID_MAKEDIR:
			s_manager.EditMkdir.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);

			// hide progress bar
			s_manager.Progress.generic.flags |= CFL_INVISIBLE;

			// hide rename editfield (if its even visible)
			s_manager.EditRename.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

			UI_SelectItemById( &s_manager.menu, ID_EDIT_MKDIR );
			UI_Refresh();
			break;

		case ID_RENAME:
			s_manager.EditRename.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);

			// hide progress bar
			s_manager.Progress.generic.flags |= CFL_INVISIBLE;

			// hide mkdir editfield (if its even visible)
			s_manager.EditMkdir.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

			// get and set selected file
			pFileInfo = UI_DirView_GetMarked( &s_manager.DirViewSrc, 0 );
			UI_Editfield_SetString( &s_manager.EditRename, pFileInfo->name );

			// save old file name
			strcpy( s_manager.oldName, UI_DirView_GetDir(&s_manager.DirViewSrc) );
			strcat( s_manager.oldName, pFileInfo->name );

			UI_SelectItemById( &s_manager.menu, ID_EDIT_RENAME_DIR );
			UI_Refresh();
			break;

		case ID_DELETE:
			UI_ManagerDeleteFiles();
			break;

		// switch to DirViewSrc control
		case ID_SOURCE:
			s_manager.DirViewSrc.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);
			s_manager.DirViewDest.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

			UI_SelectItemById( &s_manager.menu, ID_DIRVIEW_SOURCE );
			UI_Refresh();
			break;

		// switch to DirViewDest control
		case ID_DEST:
			s_manager.DirViewDest.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);
			s_manager.DirViewSrc.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

			UI_SelectItemById( &s_manager.menu, ID_DIRVIEW_DEST );
			UI_Refresh();
			break;

		case ID_COPY:
			// make sure status information is visible when copying stuff
			s_manager.EditMkdir.generic.flags	|= (CFL_INVISIBLE | CFL_INACTIVE);
			s_manager.EditRename.generic.flags	|= (CFL_INVISIBLE | CFL_INACTIVE);

			// show status bar
			s_manager.Progress.generic.flags	&= ~CFL_INVISIBLE;

			UI_ManagerStartCopying();

			// refresh target directory
			UI_DirView_Refresh( &s_manager.DirViewDest );
			UI_Refresh();
			break;

		case ID_GOBACK:
			UI_SetActiveMenu(MENU_ID_MAIN);
			break;

		case ID_DIRVIEW_SOURCE:
			switch( nCode )
			{
				// update displayed source path
				case NOT_DV_CHANGED_DIR:
					UI_Refresh();
					break;

				// update "number of files to copy" display
				case NOT_DV_MARKED_ENTRY:
					UI_Refresh();
					break;

				case NOT_DV_UNMARKED_ENTRY:
					UI_Refresh();
					break;
			}
			break;

		case ID_DIRVIEW_DEST:
			switch( nCode )
			{
				case NOT_DV_CHANGED_DIR:
					UI_Refresh();
					break;
			}
			break;

		case ID_EDIT_MKDIR:
			switch( nCode )
			{
				// user is done entering dir name
				case NOT_EF_CLICKED_OK:
					pStr = UI_Editfield_GetString( &s_manager.EditMkdir );

					// attempt to create new dir and refresh directory view
					pPath = UI_DirView_GetDir( &s_manager.DirViewSrc );

					strcpy( strDir, pPath );
					strcat( strDir, pStr );

					FileMkdir( strDir );

					s_manager.EditMkdir.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);
					s_manager.Progress.generic.flags &= ~CFL_INVISIBLE;

					UI_SelectItemById( &s_manager.menu, ID_DIRVIEW_SOURCE );
					
					UI_DirView_Refresh( &s_manager.DirViewSrc );
					UI_Refresh();
					break;

				// user cancelled operation
				case NOT_EF_CLICKED_CANCEL:
					s_manager.EditMkdir.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

					// show progress bar
					s_manager.Progress.generic.flags &= ~CFL_INVISIBLE;

					UI_SelectItemById( &s_manager.menu, ID_MAKEDIR );
					UI_Refresh();
					break;
			}
			break;

		case ID_EDIT_RENAME_DIR:
			switch( nCode )
			{
				// attempt to rename file
				case NOT_EF_CLICKED_OK:
					pStr = UI_Editfield_GetString( &s_manager.EditRename );

					// build new path
					strcpy( s_manager.newName, UI_DirView_GetDir(&s_manager.DirViewSrc) );
					strcat( s_manager.newName, pStr );
					
					FileRename( s_manager.oldName, s_manager.newName );

					s_manager.EditRename.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);
					s_manager.Progress.generic.flags &= ~CFL_INVISIBLE;

					UI_SelectItemById( &s_manager.menu, ID_DIRVIEW_SOURCE );

					UI_DirView_Refresh( &s_manager.DirViewSrc );
					UI_Refresh();
					break;

				// cancelled
				case NOT_EF_CLICKED_CANCEL:
					s_manager.EditRename.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

					// show progress bar
					s_manager.EditRename.generic.flags &= ~CFL_INACTIVE;

					UI_SelectItemById( &s_manager.menu, ID_RENAME );
					UI_Refresh();
					break;
			}
			break;
	}
}

//
// UI_ManagerStartCopying - Performs copying of files and folders
//

void UI_ManagerStartCopying( void )
{
	u64 totalSize	= 0;
	int numMarked, i;
	char path[256], out[256];
	const fileInfo_t *pFileInfo;

	// loop through all selected entries
	numMarked = UI_DirView_MarkedCount( &s_manager.DirViewSrc );

	for( i = 0; i < numMarked; i++ )
	{
		pFileInfo = UI_DirView_GetMarked( &s_manager.DirViewSrc, i );

		// should never happen
		if( !pFileInfo )
			break;

		// calculate total size of all files
		// FIXME: this is lame for cd right now
		if( pFileInfo->flags & FLAG_DIRECTORY )
		{
			strcpy( path, UI_DirView_GetDir(&s_manager.DirViewSrc) );
			strcat( path, pFileInfo->name );
			strcat( path, "/" );

			totalSize += DirGetSize( path, 0 );
		}
		else {
			totalSize += pFileInfo->size;
		}
	}

#ifdef _DEBUG
	printf("UI_ManagerStartCopy : size of selected files : %li\n", totalSize );
#endif

	// setup progress bar
	UI_Progress_SetBounds( &s_manager.Progress, 0, totalSize );
	UI_Progress_SetPosition( &s_manager.Progress, 0 );

	// refresh screen
	UI_Refresh();

	// start copying stuff now
	for( i = 0; i < numMarked; i++ )
	{
		pFileInfo = UI_DirView_GetMarked( &s_manager.DirViewSrc, i );

		// should never happen
		if( !pFileInfo )
			break;

		// recursively copy directory tree
		if( pFileInfo->flags & FLAG_DIRECTORY )
		{
			strcpy( path, UI_DirView_GetDir(&s_manager.DirViewSrc) );
			strcat( path, pFileInfo->name );
			strcat( path, "/" );

			strcpy( out, UI_DirView_GetDir(&s_manager.DirViewDest) );
			strcat( out, pFileInfo->name );
			strcat( out, "/" );

#ifdef _DEBUG
			printf("UI_ManagerStartCopy : Copy dir from: %s\n", path);
			printf("UI_ManagerStartCopy : Copy dir to: %s\n", out);
#endif
			UI_ManagerCopyDir( path, out );
		}
		else
		{
			// copy a single file
			strcpy( path, UI_DirView_GetDir(&s_manager.DirViewSrc) );
			strcat( path, pFileInfo->name );

			strcpy( out, UI_DirView_GetDir(&s_manager.DirViewDest) );
			strcat( out, pFileInfo->name );

			UI_ManagerCopyFile( path, out );
		}
	}
}

//
// UI_ManagerCopyDir - Recursively copies a directory tree.
//

void UI_ManagerCopyDir( const char *path_in, const char *path_out )
{
	int numFiles, i;
	fileInfo_t *pFileInfo;
	char new_path_in[256];
	char new_path_out[256];

	if( !path_in || !path_out )
		return;

	// attempt to create target directory
	if( FileMkdir( path_out ) == -1 ) {
#ifdef _DEBUG
		printf("UI_ManagerCopyDir : FileMkdir() failed.\n");
#endif
		return;
	}

	pFileInfo = (fileInfo_t*) malloc( sizeof(fileInfo_t) * MAX_DIR_FILES );
	if( !pFileInfo ) {
#ifdef _DEBUG
		printf("UI_ManagerCopyDir : malloc() failed.\n");
#endif
		return;
	}

	numFiles = DirGetContents( path_in, NULL, pFileInfo, MAX_DIR_FILES );
	
	for( i = 0; i < numFiles; i++ )
	{
		if( !strcmp( pFileInfo[i].name, ".." ) || !strcmp( pFileInfo[i].name, "." ) )
			continue;

		if( pFileInfo[i].flags & FLAG_DIRECTORY )
		{
			// build new in path
			strcpy( new_path_in, path_in );
			strcat( new_path_in, pFileInfo[i].name );
			strcat( new_path_in, "/" );

			// build new out path
			strcpy( new_path_out, path_out );
			strcat( new_path_out, pFileInfo[i].name );
			strcat( new_path_out, "/" );

			UI_ManagerCopyDir( new_path_in, new_path_out );
		}
		else
		{
			// copy a single file
			strcpy( new_path_in, path_in );
			strcat( new_path_in, pFileInfo[i].name );

			strcpy( new_path_out, path_out );
			strcat( new_path_out, pFileInfo[i].name );

			UI_ManagerCopyFile( new_path_in, new_path_out );
		}
	}

	free(pFileInfo);
}

//
// UI_ManagerCopyFile - Copies a single file.
//						Also updates the User Interface
//						(progress bar etc.).
//

#define CHUNK_SIZE 65536

void UI_ManagerCopyFile( const char *file_in, const char *file_out )
{
	FHANDLE fh_in, fh_out;
	u8 chunk[CHUNK_SIZE];
	int read;
	u64 pos;
	char *p;

	if( !file_in || !file_out )
		return;

	fh_in = FileOpen( file_in, O_RDONLY );
	if( fh_in.fh == -1 ) {
#ifdef _DEBUG
		printf("UI_ManagerCopyFile : Could not open file %s\n", file_in);
#endif
		return;
	}

	fh_out = FileOpen( file_out, O_RDWR | O_CREAT | O_TRUNC );
	if( fh_out.fh == -1 ) {
#ifdef _DEBUG
		printf("UI_ManagerCopyFile : Could not open file %s\n", file_out);
#endif
		return;
	}

	// update currentFile display
	p = strrchr( file_in, '/' );
	if( !p )
		p = strrchr( file_in, ':' );

	if( p ) {
		p++;
		strcpy( s_manager.currentFile, p );

		if( strlen(s_manager.currentFile) > 16 ) {
			s_manager.currentFile[14] = '.';
			s_manager.currentFile[15] = '.';
			s_manager.currentFile[16] = '\0';
		}

		UI_Refresh();
	}

	// copy file in chunks
	while(1)
	{
		read = FileRead( fh_in, chunk, sizeof(chunk) );

		if( read <= 0 )
			break;

		FileWrite( fh_out, chunk, read );

		// update progress bar
		pos = UI_Progress_GetPosition( &s_manager.Progress );

		// increment position and refresh screen
		pos += read;

		UI_Progress_SetPosition( &s_manager.Progress, pos );
		UI_Refresh();
	}

	FileClose(fh_in);
	FileClose(fh_out);
}

//
// UI_ManagerDeleteFiles - Deletes selected files
//

void UI_ManagerDeleteFiles( void )
{
	int i, numMarked;
	const fileInfo_t *pFile;
	const char *pStr;
	char absPath[256];

	numMarked	= UI_DirView_MarkedCount( &s_manager.DirViewSrc );
	pStr		= UI_DirView_GetDir( &s_manager.DirViewSrc );

	for( i = 0; i < numMarked; i++ )
	{
		pFile = UI_DirView_GetMarked( &s_manager.DirViewSrc, i );

		strcpy( absPath, pStr );
		strcat( absPath, pFile->name );

#ifdef _DEBUG
		printf("UI_ManagerDeleteFiles : Deleting %s\n", absPath);
#endif
		if( pFile->flags & FLAG_DIRECTORY ){
			strcat( absPath, "/" );	// hdd complains if we dont do that

			DirRemove( absPath );
		}
		else {
			FileRemove( absPath );
		}
	}

	// refresh directory view to reflect the changes
	UI_DirView_Refresh( &s_manager.DirViewSrc );
	UI_Refresh();
}
