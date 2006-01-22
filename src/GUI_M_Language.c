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

#include <GUI.h>
#include <GUI_Ctrl.h>
#include <GUI_Menu.h>
#include <Sysconf.h>

#define ID_LIST				100

#define DLG_LANG_CONFIRM	0

static void ListLanguages( void )
{
	char			szName[MAX_PATH + 1], *pStr;
	int				nNumFiles, i;
	fileInfo_t		*pFiles;
	GUIControl_t	*pCtrl;

	pCtrl = GUI_ControlByID(ID_LIST);

	if( !(pFiles = malloc( sizeof(fileInfo_t) * MAX_DIR_FILES )) )
		return;

#ifdef _DEVELOPER
	snprintf( szName, sizeof(szName), "mc0:/BOOT/MYPS2/language/" );
#else
	snprintf( szName, sizeof(szName), "%slanguage/", GetElfPath() );
#endif
	
	nNumFiles = DirGetContents( szName, "xml", pFiles, MAX_DIR_FILES );

	for( i = 0; i < nNumFiles; i++ )
	{
		if( pFiles[i].flags & FLAG_DIRECTORY )
			continue;

		strncpy( szName, pFiles[i].name, MAX_PATH );
		szName[MAX_PATH] = 0;

		StripFileExt( szName, szName );
		ucfirst(strtolower(szName));

		if( !(pStr = malloc( strlen(pFiles[i].name) + 1 )) )
			return;

		strcpy( pStr, pFiles[i].name );
		GUI_Ctrl_List_AddItem( pCtrl, szName, (unsigned int) pStr );
	}

	free(pFiles);
}

unsigned int GUI_CB_Language( GUIMenu_t *lpGUIMenu, unsigned int nGUIMsg,
							  unsigned int nCtrlParam, unsigned int nOther )
{
	GUIControl_t	*pCtrl;
	unsigned int	nNum, i;
	char			*pStr, szFileName[MAX_PATH + 1];
	const char		*pSkinName;

	switch( nGUIMsg )
	{
		case GUI_MSG_OPEN:
			ListLanguages();
			break;

		// free associated item data
		case GUI_MSG_CLOSE:
			pCtrl	= GUI_ControlByID(ID_LIST);
			nNum	= GUI_Ctrl_List_GetCount(pCtrl);

			for( i = 0; i < nNum; i++ )
			{
				pStr = (char*) GUI_Ctrl_List_GetItemData( pCtrl, i );
				free( pStr );
			}

			GUI_Ctrl_List_Clean(pCtrl);
			break;

		case GUI_MSG_CONTROL:
			switch( LOWORD(nCtrlParam) )
			{
				case ID_LIST:
					switch( HIWORD(nCtrlParam) )
					{
						case GUI_NOT_LIST_CLICK:
							GUI_DlgYesNo( GUI_GetLangStr(LANG_STR_LANG_CONFIRM),
										  GUI_GetLangStr(LANG_STR_CONFIRM),
										  DLG_LANG_CONFIRM );
							break;
					}
					break;

				case ID_GO_BACK:
					GUI_OpenMenu( GUI_MENU_SETTINGS );
					break;
			}
			break;

		case GUI_MSG_DIALOG:
			switch( LOWORD(nCtrlParam) )
			{
				case DLG_LANG_CONFIRM:
					if( HIWORD(nCtrlParam) == DLG_RET_YES )
					{
						pCtrl	= GUI_ControlByID(ID_LIST);
						pStr	= (char*) GUI_Ctrl_List_GetItemData( pCtrl,
									GUI_Ctrl_List_GetSelIndex(pCtrl) );

						SC_SetValueForKey_Str( "lang_file", pStr );
						snprintf( szFileName, sizeof(szFileName), "%slanguage/%s",
								  GetElfPath(), pStr );

						GUI_FreeLangTable();
						GUI_LoadLangTable( szFileName );

						// reloading the whole skin takes long...
						pSkinName = SC_GetValueForKey_Str( "skin_name", NULL );
						snprintf( szFileName, sizeof(szFileName), "%sskins/%s",
								  GetElfPath(), pSkinName );

						GUI_FreeSkin();
						GUI_LoadSkin(szFileName);

						GUI_OpenMenu( GUI_MENU_LANGUAGE );
					}
					break;
			}
			break;
	}
	return 0;
}
