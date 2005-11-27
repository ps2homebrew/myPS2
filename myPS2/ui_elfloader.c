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
#		ELF Loader Menu Code.
#
*/

#include <tamtypes.h>
#include <ui.h>
#include <elf.h>

#define ID_SELECTELF	1
#define ID_EDITINFO		2
#define ID_GOBACK		3
#define ID_DIRVIEW		4
#define ID_EDITFIELD	5

typedef struct {
	menuFramework_t		*menu;

	menuText_t			SelectElf;
	menuText_t			EditInfo;

	menuText_t			GoBack;

	menuDirView_t		DirView;
	menuEditfield_t		EditField;

	// descriptions for elf files
	descEntry_t			descTable[MAX_DESC_ENTRIES];
	int					numEntries;

} uiElfLoaderMenu_t;

static uiElfLoaderMenu_t s_elfloader;

//
// UI_InitElfLoaderMenu - Initializes ELF loader menu controls
//

void UI_InitElfLoaderMenu( void )
{
	s_elfloader.menu						= &uis.menus[ MENU_ELFLOADER ];

	s_elfloader.menu->callback				= UI_ElfLoaderCallback;
	s_elfloader.menu->input					= NULL;

	s_elfloader.menu->numItems				= 0;
	s_elfloader.menu->selectedItem			= 0;

	s_elfloader.SelectElf.generic.type		= MENU_CONTROL_TEXT;
	s_elfloader.SelectElf.generic.flags		= 0;
	s_elfloader.SelectElf.generic.x			= 35;
	s_elfloader.SelectElf.generic.y			= 70;
	s_elfloader.SelectElf.generic.id		= ID_SELECTELF;
	s_elfloader.SelectElf.text				= "Select ELF";
	s_elfloader.SelectElf.size				= GR_FONT_SMALL;
	s_elfloader.SelectElf.color				= RGB(255, 255, 255);

	s_elfloader.EditInfo.generic.type		= MENU_CONTROL_TEXT;
	s_elfloader.EditInfo.generic.flags		= CFL_INACTIVE;
	s_elfloader.EditInfo.generic.x			= 35;
	s_elfloader.EditInfo.generic.y			= 90;
	s_elfloader.EditInfo.generic.id			= ID_EDITINFO;
	s_elfloader.EditInfo.text				= "Edit Info";
	s_elfloader.EditInfo.size				= GR_FONT_SMALL;
	s_elfloader.EditInfo.color				= RGB(255, 255, 255);

	s_elfloader.GoBack.generic.type			= MENU_CONTROL_TEXT;
	s_elfloader.GoBack.generic.flags		= 0;
	s_elfloader.GoBack.generic.x			= 35;
	s_elfloader.GoBack.generic.y			= 120;
	s_elfloader.GoBack.generic.id			= ID_GOBACK;
	s_elfloader.GoBack.text					= "Go Back";
	s_elfloader.GoBack.size					= GR_FONT_SMALL;
	s_elfloader.GoBack.color				= RGB(255, 255, 255);

	s_elfloader.DirView.generic.type		= MENU_CONTROL_DIRVIEW;
	s_elfloader.DirView.generic.flags		= 0;
	s_elfloader.DirView.generic.x			= 200;
	s_elfloader.DirView.generic.y			= 40;
	s_elfloader.DirView.generic.id			= ID_DIRVIEW;
	s_elfloader.DirView.width				= 400;
	s_elfloader.DirView.height				= 200;
	s_elfloader.DirView.filter				= "ELF";
	s_elfloader.DirView.color				= RGB(81, 112, 164);

	s_elfloader.EditField.generic.type		= MENU_CONTROL_EDITFIELD;
	s_elfloader.EditField.generic.flags		= CFL_INACTIVE | CFL_INVISIBLE;
	s_elfloader.EditField.generic.x			= 200;
	s_elfloader.EditField.generic.y			= 275;
	s_elfloader.EditField.generic.id		= ID_EDITFIELD;
	s_elfloader.EditField.width				= 400;
	s_elfloader.EditField.height			= 200;
	s_elfloader.EditField.color				= RGBA(81, 112, 164, 32);

	// add items to menu container
	UI_AddItemToMenu( s_elfloader.menu, &s_elfloader.SelectElf );
	UI_AddItemToMenu( s_elfloader.menu, &s_elfloader.EditInfo );
	UI_AddItemToMenu( s_elfloader.menu, &s_elfloader.GoBack );
	UI_AddItemToMenu( s_elfloader.menu, &s_elfloader.DirView );
	UI_AddItemToMenu( s_elfloader.menu, &s_elfloader.EditField );

	// load text file with program descriptions
	s_elfloader.numEntries = UI_LoadDescriptions( s_elfloader.descTable, MAX_DESC_ENTRIES );
}

//
// UI_ElfLoaderCallback - Elf Loader Callback
//

int UI_ElfLoaderCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	int					nEnable;
	const fileInfo_t	*pFileInfo;
	const char			*pDir, *pStr;
	char				strBuf[256];
	const descEntry_t	*pDesc;

	switch( nMsg )
	{
		case MSG_DRAW:
			UI_ElfLoaderDraw();
			return 1;

		case MSG_CONTROL:
			switch( sParam )
			{
				case ID_DIRVIEW:
					switch( fParam )
					{
						case NOT_DV_CHANGED_DIR:
						case NOT_DV_MARKED_ENTRY:
						case NOT_DV_UNMARKED_ENTRY:
							nEnable = 1;

							if( UI_DirView_MarkedCount( &s_elfloader.DirView ) != 1 )
								nEnable = 0;
					
							pFileInfo = UI_DirView_GetMarked( &s_elfloader.DirView, 0 );

							if( !CmpFileExtension( pFileInfo->name, "ELF" ) )
								nEnable = 0;

							if( (nEnable) && (s_elfloader.EditInfo.generic.flags & CFL_INACTIVE) )
							{
								s_elfloader.EditInfo.generic.flags &= ~CFL_INACTIVE;
								UI_Refresh();
							}
							else if( !(nEnable) && !(s_elfloader.EditInfo.generic.flags & CFL_INACTIVE) )
							{
								s_elfloader.EditInfo.generic.flags |= CFL_INACTIVE;

								// also make sure editfield is disabled
								s_elfloader.EditField.generic.flags |= (CFL_INACTIVE | CFL_INVISIBLE);

								UI_Refresh();
							}
							return 1;

						// user clicked an ELF file, attempt to run it
						case NOT_DV_CLICKED_ENTRY:
							// build path
							pDir		= UI_DirView_GetDir( &s_elfloader.DirView );
							pFileInfo	= UI_DirView_GetSelected( &s_elfloader.DirView );

							strcpy( strBuf, pDir );
							strcat( strBuf, pFileInfo->name );

							// good bye
							if( CheckELFHeader( strBuf ) ) {
								RunLoaderELF( strBuf );
							}
							else {
#ifdef _DEBUG
								printf("%s is not a valid ELF file!\n", strBuf );
#endif
							}
							return 1;
					}
					return 1;

				case ID_SELECTELF:
					UI_SelectItemById( s_elfloader.menu, ID_DIRVIEW );
					UI_Refresh();
					return 1;

				case ID_EDITINFO:
					// enable editfield control
					s_elfloader.EditField.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);

					// grab description
					pDir		= UI_DirView_GetDir( &s_elfloader.DirView );
					pFileInfo	= UI_DirView_GetMarked( &s_elfloader.DirView, 0 );

					strcpy( strBuf, pDir );
					strcat( strBuf, pFileInfo->name );

					if( (pDesc = UI_GetDescByName( strBuf )) == NULL )
						pStr = "Not available";
					else
						pStr = pDesc->fileDesc;

					UI_Editfield_SetString( &s_elfloader.EditField, pStr );

					UI_SelectItemById( s_elfloader.menu, ID_EDITFIELD );
					UI_Refresh();
					break;

				case ID_GOBACK:
					UI_SetActiveMenu(MENU_MAIN);
					break;

				case ID_EDITFIELD:
					switch( fParam )
					{
						case NOT_EF_CLICKED_CANCEL:
							s_elfloader.EditField.generic.flags |= (CFL_INACTIVE | CFL_INVISIBLE);
					
							UI_SelectItemById( s_elfloader.menu, ID_DIRVIEW );
							UI_Refresh();
							return 1;

						case NOT_EF_CLICKED_OK:
							s_elfloader.EditField.generic.flags |= (CFL_INACTIVE | CFL_INVISIBLE);

							pDir		= UI_DirView_GetDir( &s_elfloader.DirView );
							pFileInfo	= UI_DirView_GetMarked( &s_elfloader.DirView, 0 );

							strcpy( strBuf, pDir );
							strcat( strBuf, pFileInfo->name );

							// add the new description
							pStr = UI_Editfield_GetString( &s_elfloader.EditField );
							UI_AddDescription( strBuf, pStr );

							UI_SelectItemById( s_elfloader.menu, ID_DIRVIEW );
							UI_Refresh();
							return 1;
					}
					return 1;
			}
			break;
	}
	return 0;
}

//
// UI_ElfLoaderDraw
//

void UI_ElfLoaderDraw( void )
{
	const fileInfo_t	*pFileInfo;
	const descEntry_t	*pDesc;
	const char			*pDir;
	const char			*pElfName;
	const char			*pElfDesc;

	int					nElfSize;
	char				strBuf[256];

	// draw sidebar
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	// draw controls
	UI_DrawMenu( s_elfloader.menu );

	// draw static text
	GR_SetFontColor( RGB(255, 255, 255) );

	GR_DrawTextExt( 210, 25, "Select ELF Executable", GR_FONT_SMALL );

	// only draw program information if editfield is not up
	if( !(s_elfloader.EditField.generic.flags & CFL_INVISIBLE) ) {
		GR_DrawTextExt( 210, 260, "Enter New Description", GR_FONT_SMALL );
		return;
	}

	GR_DrawTextExt( 210, 260, "Program Information", GR_FONT_SMALL );

	GR_DrawTextExt( 210, 300, "File Name", GR_FONT_SMALL );
	GR_DrawTextExt( 210, 315, "File Size", GR_FONT_SMALL );

	GR_DrawTextExt( 210, 340, "Program Description", GR_FONT_SMALL );

	GR_SetFontColor( RGB(0, 255,0 ) );

	pFileInfo = UI_DirView_GetSelected( &s_elfloader.DirView );

	if( !CmpFileExtension( pFileInfo->name, "ELF") ) {
		pElfName = "Not Selected";
		nElfSize = 0;
	}
	else {
		pElfName = pFileInfo->name;
		nElfSize = pFileInfo->size;
	}

	GR_DrawTextExt( 300, 300, pElfName, GR_FONT_SMALL );

	sprintf( strBuf, "%i KB", nElfSize / 1024 );
	GR_DrawTextExt( 300, 315, strBuf, GR_FONT_SMALL );

	// build ELF path
	pDir = UI_DirView_GetDir( &s_elfloader.DirView );

	strcpy( strBuf, pDir );
	strcat( strBuf, pElfName );

	pDesc = UI_GetDescByName( strBuf );

	if( pDesc )
		pElfDesc = pDesc->fileDesc;
	else
		pElfDesc = "Not available";

	GR_DrawTextExt( 210, 360, pElfDesc, GR_FONT_SMALL );
}

//
// UI_LoadDescriptions - Attempts to parse program description file
//						 on mc0.
//						 Returns number of items read.
//

int UI_LoadDescriptions( descEntry_t *descTable, int maxItems )
{
	FHANDLE fHandle;
	char	strLine[256];
	int		numRead;
	char	*p_start, *p_end;

	if( !MC_Available(0) )
		return 0;

	fHandle = FileOpen( "mc0:/SYS-CONF/MYPS2/DESC.TXT", O_RDONLY );

	// file doesn't exist or some other error occured
	if( fHandle.fh < 0 )
	{
#ifdef _DEBUG
		printf("UI_LoadDescriptions : Could not open file!\n");
#endif
		return 0;
	}

	numRead = 0;

	// parse buffer line by line
	while( FileGets( strLine, sizeof(strLine), fHandle ) )
	{
		if( numRead >= maxItems )
			break;
		
		// skip empty lines
		if( strLine[0] == '\n' || strLine[0] == '\r' )
			continue;

		// new entry name should be enclosed in []
		p_start = strchr( strLine, '[' );

		if( !p_start ) {
#ifdef _DEBUG
			printf("UI_LoadDescription : Error parsing file. Expected '['.\n");
#endif
			break;
		}

		// skip '['
		p_start++;

		p_end = strrchr( strLine, ']' );

		if( !p_end ) {
#ifdef _DEBUG
			printf("UI_LoadDescription : Error parsing file. Expected ']'.\n");
#endif
			break;
		}

		// copy file path
		strncpy( descTable[ numRead ].filePath, p_start, p_end - p_start );

		// read description line
		FileGets( strLine, sizeof(strLine), fHandle );
		strncpy( descTable[ numRead ].fileDesc, strLine, strlen(strLine) - 1 );

		// null terminate
		descTable[ numRead ].fileDesc[ strlen(strLine) - 1 ] = 0;

		numRead++;
	}

	FileClose(fHandle);

	return numRead;
}


//
// UI_GetDescByName - Returns description entry by name.
//					  If nothing is found NULL is returned.
//

const descEntry_t *UI_GetDescByName( const char *name )
{
	int i;

	for( i = 0; i < s_elfloader.numEntries; i++ )
	{
		if( !strcmp( name, s_elfloader.descTable[i].filePath ) )
			return &s_elfloader.descTable[i];
	}

	return NULL;
}

//
// UI_AddDescription -  Adds a description for an ELF file to
//						the DESC.TXT file, then reloads the file to
//						reflect the changes.
//
//						Returns 1 on success, otherwise 0.
//

int UI_AddDescription( const char *strFile, const char *strDesc )
{
	int i;
	const descEntry_t *pDesc;
	FHANDLE fHandle;
	char strBuf[256];

	if( !strFile || !strDesc )
		return 0;

	if( !MC_Available(0) )
		return 0;

	pDesc = UI_GetDescByName( strFile );

	//
	// if we are editing one of the existing entries
	// just rebuild the whole file
	if( pDesc )
	{
		fHandle = FileOpen( "mc0:/SYS-CONF/MYPS2/DESC.TXT", O_WRONLY | O_CREAT | O_TRUNC );

		if( fHandle.fh < 0 ) {
#ifdef _DEBUG
			printf("UI_AddDescription : Could not open DESC.TXT!\n");
			return 0;
#endif
		}

		for( i = 0; i < s_elfloader.numEntries; i++ )
		{
			// write entry name
			strcpy( strBuf, "[" );
			strcat( strBuf, s_elfloader.descTable[i].filePath );
			strcat( strBuf, "]" );
			strcat( strBuf, "\r\n" );

			FileWrite( fHandle, strBuf, strlen(strBuf) );

			// replace description of this entry with new description
			if( pDesc == &s_elfloader.descTable[i] ) {
				strcpy( strBuf, strDesc );
			}
			else {
				strcpy( strBuf, s_elfloader.descTable[i].fileDesc );
			}

			// add newline character
			strcat( strBuf, "\r\n" );

			// write out description to file
			FileWrite( fHandle, strBuf, strlen(strBuf) );
		}

		// done
		FileClose(fHandle);
	}
	else
	{
		// if we are adding a new entry, append it
		// to the end of the file
		fHandle = FileOpen( "mc0:/SYS-CONF/MYPS2/DESC.TXT", O_RDWR /*| O_APPEND*/ );

		// the file probably just doesn't exist, try to create it
		if( fHandle.fh < 0 )
			fHandle = FileOpen( "mc0:/SYS-CONF/MYPS2/DESC.TXT", O_RDWR | O_CREAT | O_TRUNC );

		if( fHandle.fh < 0 ) {
#ifdef _DEBUG
			printf("UI_AddDescription : Could not open DESC.TXT!\n");
#endif
			return 0;
		}

		// i can't get O_APPEND to work so just seek to the end of file
		FileSeek( fHandle, 0, SEEK_END );

		// append new entry
		strcpy( strBuf, "[" );
		strcat( strBuf, strFile );
		strcat( strBuf, "]" );
		strcat( strBuf, "\r\n" );

		FileWrite( fHandle, strBuf, strlen(strBuf) );

		// append new description
		strcpy( strBuf, strDesc );
		strcat( strBuf, "\r\n" );

		FileWrite( fHandle, strBuf, strlen(strBuf) );
		FileClose( fHandle );
	}

	// reload DESC.TXT file
	s_elfloader.numEntries = UI_LoadDescriptions( s_elfloader.descTable, MAX_DESC_ENTRIES );

	return 1;
}
