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
#define ID_CHARSET			101

#define DLG_LANG_CONFIRM	0

static Charset_t Charset[] =
{
	{	"ISO-8859-1",	"Western European (ISO-8859-1)"			},
	{	"ISO-8859-2",	"Central European (ISO-8859-2)"			},
	{	"ISO-8859-3",	"Southern European (ISO-8859-3)"		},
	{	"ISO-8859-4",	"Northern European (ISO-8859-4)"		},
	{	"ISO-8859-5",	"Latin/Cyrillic (ISO-8859-5)"			},
	{	"ISO-8859-6",	"Latin/Arabic (ISO-8859-6)"				},
	{	"ISO-8859-7",	"Latin/Greek (ISO-8859-7)"				},
	{	"ISO-8859-8",	"Latin/Hebrew (ISO-8859-8)"				},
	{	"ISO-8859-9",	"Latin-5 Turkish (ISO-8859-9)"			},
	{	"ISO-8859-10",	"Latin-6 Nordic (ISO-8859-10)"			},
	{	"ISO-8859-11",	"Latin/Thai (ISO-8859-11)"				},
	{	"ISO-8859-13",	"Latin-7 Baltic Rim (ISO-8859-13)"		},
	{	"ISO-8859-14",	"Latin-8 Celtic (ISO-8859-14)"			},
	{	"ISO-8859-15",	"Latin-9 (ISO-8859-15)"					},
	{	"ISO-8859-16",	"Latin-10 SE European (ISO-8859-16)"	}
};

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
	unsigned int	nNum, i, nIndex;
	char			*pStr, szFileName[MAX_PATH + 1], szOldLang[MAX_PATH + 1];
	const char		*pSkinName;
	char			szCharset[256];

	switch( nGUIMsg )
	{
		case GUI_MSG_OPEN:
			ListLanguages();

			pCtrl = GUI_ControlByID(ID_CHARSET);
			if( pCtrl && GUI_Ctrl_Combo_Empty(pCtrl) )
			{
				nIndex = 0;
				SC_GetValueForKey_Str( "lang_charset", szCharset );

				for( i = 0; i < (sizeof(Charset) / sizeof(Charset[0])); i++ )
				{
					if( !strcmp( szCharset, Charset[i].lpCharset ) )
						nIndex = i;

					GUI_Ctrl_Combo_Add( pCtrl, Charset[i].lpDesc, i );
				}

				GUI_Ctrl_Combo_SetCurSel( pCtrl, nIndex );
			}
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

				case ID_CHARSET:
					SC_SetValueForKey_Str( "lang_charset", Charset[ nOther ].lpCharset );

					// reset charset converter
					CharsetConvert_Reset();
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

						SC_GetValueForKey_Str( "lang_file", szOldLang );
						SC_SetValueForKey_Str( "lang_file", pStr );
#ifdef _DEVELOPER
						snprintf( szFileName, sizeof(szFileName),
									"mc0:/BOOT/MYPS2/language/%s", pStr );
#else
						snprintf( szFileName, sizeof(szFileName), "%slanguage/%s",
								  GetElfPath(), pStr );
#endif

						GUI_FreeLangTable();
						if( !GUI_LoadLangTable( szFileName ) ) {
#ifdef _DEVELOPER
							printf("Failed loading language file : %s\n", pStr);
							SleepThread();
#else
							// re-load old file if new file couldn't be loaded
							SC_SetValueForKey_Str( "lang_file", szOldLang );

							snprintf( szFileName, sizeof(szFileName), "%slanguage/%s",
									  GetElfPath(), pStr );

							GUI_LoadLangTable( szFileName );
#endif
						}

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
