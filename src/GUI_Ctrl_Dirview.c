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
#include <File.h>

void GUI_Ctrl_Dirview_Draw( const GUIControl_t *pCtrl )
{
	unsigned int nNumItems, bHasFocus, bItemFocus;
	unsigned int i, nItemIdx, nItemPosY;
	u64 uTextColor, uTexColor;
	unsigned int nItemHeight, nTextPosY, nTextPosX, nStrIndex;
	const GUIMenuImage_t *pTexNoFocus, *pTexFocus, *pTexMarked,
		*pTex, *pScrollTex;
	const GUIMenuFont_t *pFont;
	fileInfo_t *pItem;
	char szStr[ MAX_PATH + 1 ];
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( pDir->nTexture == -1 || pDir->nTextureFocus == -1 ||
		pDir->nTextureMarked == -1 || pDir->nFontIdx == - 1)
		return;

	pTexNoFocus = GUI_MenuGetImage( pCtrl->pParent, pDir->nTexture );
	if( !pTexNoFocus )
		return;

	pTexFocus = GUI_MenuGetImage( pCtrl->pParent, pDir->nTextureFocus );
	if( !pTexFocus )
		return;
	
	pTexMarked = GUI_MenuGetImage( pCtrl->pParent, pDir->nTextureMarked );
	if( !pTexMarked )
		return;

	pFont = GUI_MenuGetFont( pCtrl->pParent, pDir->nFontIdx );
	if( !pFont )
		return;

	nItemHeight = pDir->nItemHeight ? pDir->nItemHeight : gsLib_font_height(pFont->gsFont);
	nNumItems	= pDir->nHeight / (nItemHeight + pDir->nPadding);
	bHasFocus	= pCtrl->pParent->iSelID == pCtrl->nID;
	nItemPosY	= pCtrl->nPosY;
	uTexColor	= pDir->nColorDiffuse ? pDir->nColorDiffuse : colorTex;

	for( i = 0; i < nNumItems; i++ )
	{
		nItemIdx = pDir->nStartItem + i;

		if( nItemIdx >= pDir->nNumItems )
			break;

		pItem = &pDir->pItems[nItemIdx];
		bItemFocus = (bHasFocus) && (nItemIdx == pDir->nSelectedItem);
		
		pTex = (pItem->flags & FLAG_MARKED) ? pTexMarked : pTexNoFocus;

		if( bItemFocus )
			pTex = pTexFocus;

		gsLib_prim_sprite_texture(	pTex->gsTexture, pCtrl->nPosX, nItemPosY, pDir->nWidth,
									nItemHeight, 0, 0, pTex->gsTexture->Width,
									pTex->gsTexture->Height, uTexColor );

		uTextColor	= (pItem->flags & FLAG_MARKED) ? pDir->nMarkedColor : pDir->nTextColor;

		if( bItemFocus )
			uTextColor = pDir->nSelectedColor;

		nTextPosX	= pCtrl->nPosX + pDir->nTextOffset;
		nTextPosY	= nItemPosY + (nItemHeight - gsLib_font_height(pFont->gsFont)) / 2;

		CharsetConvert_UTF8ToCharset( szStr, pItem->name, sizeof(szStr) );
		nStrIndex	= strlen(szStr) - 1;

		while( gsLib_font_width( pFont->gsFont, szStr ) > (pDir->nWidth - pDir->nTextOffset * 2) )
		{
			szStr[ nStrIndex ] = 0;

			if( nStrIndex > 1 )
			{
				szStr[ nStrIndex - 1 ] = '.';
				szStr[ nStrIndex - 2 ] = '.';
			}

			nStrIndex--;
		}

		gsLib_font_print( pFont->gsFont, nTextPosX, nTextPosY, uTextColor, szStr );
		nItemPosY += nItemHeight + pDir->nPadding;
	}

	if( pDir->nNumItems > nNumItems )
	{
		if( pDir->nStartItem > 0 )
		{
			pScrollTex = GUI_MenuGetImage( pCtrl->pParent, pDir->nUpTexture );

			if( pScrollTex )
			{
				gsLib_prim_sprite_texture(	pScrollTex->gsTexture, pCtrl->nPosX + pDir->nUpTexPosX,
											pCtrl->nPosY + pDir->nUpTexPosY,
											pScrollTex->gsTexture->Width,
											pScrollTex->gsTexture->Height, 0, 0,
											pScrollTex->gsTexture->Width,
											pScrollTex->gsTexture->Height, colorTex );
			}
		}

		if( (pDir->nStartItem + nNumItems) < pDir->nNumItems )
		{
			pScrollTex = GUI_MenuGetImage( pCtrl->pParent, pDir->nDownTexture );

			if( pScrollTex )
			{
				gsLib_prim_sprite_texture(	pScrollTex->gsTexture, pCtrl->nPosX +
											pDir->nDownTexPosX,
											pCtrl->nPosY + pDir->nDownTexPosY,
											pScrollTex->gsTexture->Width,
											pScrollTex->gsTexture->Height,
											0, 0, pScrollTex->gsTexture->Width,
											pScrollTex->gsTexture->Height, colorTex );
			}
		}
	}
}

void GUI_Ctrl_Dirview_Init( GUIControl_t *pCtrl )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;
	int			nHDD, i;
	char		szPfs[32];
	fileInfo_t	file;
	const char	*pMnt;

	GUI_Ctrl_Dirview_Clean(pCtrl);

	pDir->szFilter[0]	= 0;
	pDir->szPath[0]		= 0;

	if( (nHDD = HDD_Available()) )
	{
		if( nHDD == HDD_AVAIL )
		{
			strcpy( file.name, "pfs0:" );

			file.flags	= FLAG_DIRECTORY;
			file.size	= 0;

			GUI_Ctrl_Dirview_AddItem( pCtrl, &file );

			// add boot partition
			if( GetBootMode() == BOOT_HDD )
			{
				if( (pMnt = BootMntPoint()) )
				{
					if( strcmp( pMnt, "pfs0" ) )
					{
						strcpy( file.name, "pfs1:" );

						file.flags	= FLAG_DIRECTORY;
						file.size	= 0;

						GUI_Ctrl_Dirview_AddItem( pCtrl, &file );
					}
				}
			}
		}

		// add other mounted partitions to list
		for( i = 0; i < HDD_NumMounted(); i++ )
		{
			snprintf( szPfs, sizeof(szPfs), "pfs%i:", i + 2 );

			strcpy( file.name, szPfs );

			file.flags	= FLAG_DIRECTORY;
			file.size	= 0;

			GUI_Ctrl_Dirview_AddItem( pCtrl, &file );
		}
	}

	if( MC_Available(0) )
	{
		strcpy( file.name, "mc0:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		GUI_Ctrl_Dirview_AddItem( pCtrl, &file );
	}

	if( MC_Available(1) )
	{
		strcpy( file.name, "mc1:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		GUI_Ctrl_Dirview_AddItem( pCtrl, &file );
	}

	if( USB_Available() )
	{
		strcpy( file.name, "mass:" );

		file.flags	= FLAG_DIRECTORY;
		file.size	= 0;

		GUI_Ctrl_Dirview_AddItem( pCtrl, &file );
	}

	strcpy( file.name, "cdfs:" );

	file.flags	= FLAG_DIRECTORY;
	file.size	= 0;

	GUI_Ctrl_Dirview_AddItem( pCtrl, &file );
	GUI_Ctrl_Dirview_SetCursor( pCtrl, 0 );
}

int GUI_Ctrl_Dirview_AddItem( GUIControl_t *pCtrl, const fileInfo_t *pInfo )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;
	fileInfo_t *pItems;
	unsigned int nMax;

	pItems = malloc( sizeof(fileInfo_t) * (pDir->nNumItems + 1) );	

	if(!pItems)
	{
		pDir->nNumItems = 0;
		return -1;
	}

	if( pDir->pItems )
	{
		memcpy( pItems, pDir->pItems, pDir->nNumItems * sizeof(fileInfo_t) );
		free( pDir->pItems );
	}

	pDir->pItems	= pItems;
	pItems			= &pDir->pItems[ pDir->nNumItems ];

	pItems->flags	= pInfo->flags;
	pItems->size	= pInfo->size;

	nMax = sizeof(pItems->name) - 1;

	strncpy( pItems->name, pInfo->name, nMax );
	pItems->name[ nMax ] = 0;

	pDir->nNumItems++;

	return (pDir->nNumItems - 1);
}

void GUI_Ctrl_Dirview_Clean( GUIControl_t *pCtrl )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( pDir->pItems )
		free(pDir->pItems);

	pDir->pItems		= NULL;
	pDir->nNumItems		= 0;
	pDir->nStartItem	= 0;
	pDir->nSelectedItem	= 0;
}

void GUI_Ctrl_Dirview_SetDir( GUIControl_t *pCtrl, const char *pDirectory )
{
	unsigned int i, nNumFiles;
	fileInfo_t *pFileInfo;
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;
	const char *pFilter;

	GUI_Ctrl_Dirview_Clean(pCtrl);

	// add '..' entries if browsing pfs0:/, cdfs:/, mc0:/ or mass:/ root
	// so user can switch back to 'select device' list
	if( IsPartitionRoot(pDirectory) || !strcmp( pDirectory, "cdfs:/" ) ||
		!strcmp( pDirectory, "mc0:/" )  || !strcmp( pDirectory, "mc1:/")   ||
		!strcmp( pDirectory, "mass:/" ) )
	{
		fileInfo_t f;

		f.flags		= FLAG_DIRECTORY;
		f.size		= 0;

		strcpy( f.name, ".." );
	
		GUI_Ctrl_Dirview_AddItem( pCtrl, &f );
	}

	strncpy( pDir->szPath, pDirectory, MAX_PATH );
	pDir->szPath[ MAX_PATH ] = 0;

	pFileInfo = (fileInfo_t*) malloc( sizeof(fileInfo_t) * MAX_DIR_FILES );
	if( !pFileInfo )
		return;

	pFilter	= pDir->szFilter[0] ? pDir->szFilter : NULL;	
	nNumFiles = DirGetContents( pDir->szPath, pFilter, pFileInfo, MAX_DIR_FILES );

	for( i = 0; i < nNumFiles; i++ )
		GUI_Ctrl_Dirview_AddItem( pCtrl, &pFileInfo[i] );

	free(pFileInfo);
}

void GUI_Ctrl_Dirview_SetCursor( GUIControl_t *pCtrl, unsigned int nPos )
{
	unsigned int nItemHeight, nNumDraw, nOld;
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;
	const GUIMenuFont_t *pFont;

	pFont = GUI_MenuGetFont( pCtrl->pParent, pDir->nFontIdx );

	nItemHeight = pDir->nItemHeight ? pDir->nItemHeight : gsLib_font_height(pFont->gsFont);
	nNumDraw	= pDir->nHeight / (nItemHeight + pDir->nPadding);

	if( nPos < pDir->nStartItem )
	{
		pDir->nStartItem = nPos;
	}
	else if( nPos >= (pDir->nStartItem + nNumDraw) )
	{
		pDir->nStartItem = nPos - nNumDraw + 1;
	}

	nOld = pDir->nSelectedItem;
	pDir->nSelectedItem = nPos;

	if( nOld != pDir->nSelectedItem )
	{
		pCtrl->pParent->pfnCallback(	pCtrl->pParent, GUI_MSG_CONTROL,
										MAKEPARAM( pCtrl->nID, GUI_NOT_LIST_POS ),
										pDir->nSelectedItem );
	}
}

const char *GUI_Ctrl_Dirview_GetDir( const GUIControl_t *pCtrl )
{
	const GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	return pDir->szPath;
}

int GUI_Ctrl_Dirview_Input( GUIControl_t *pCtrl, unsigned int nPadBtns )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;
	char szNewPath[256], *pStr;
	unsigned int nOffset;

	if( !pDir->nNumItems )
		return 0;

	if( nPadBtns & PAD_DOWN )
	{
		if( pDir->nSelectedItem < (pDir->nNumItems - 1) )
		{
			GUI_Ctrl_Dirview_SetCursor( pCtrl, pDir->nSelectedItem + 1 );
			GUI_Render();
			return 1;
		}
	}
	else if( nPadBtns & PAD_UP )
	{
		if( pDir->nSelectedItem > 0 )
		{
			GUI_Ctrl_Dirview_SetCursor( pCtrl, pDir->nSelectedItem - 1 );
			GUI_Render();
			return 1;
		}
	}
	else if( nPadBtns & PAD_L1 )
	{
		GUI_Ctrl_Dirview_SetCursor( pCtrl, 0 );
		GUI_Render();
		return 1;
	}
	else if( nPadBtns & PAD_R1 )
	{
		GUI_Ctrl_Dirview_SetCursor( pCtrl, pDir->nNumItems - 1 );
		GUI_Render();
		return 1;
	}
	else if( nPadBtns & PAD_CROSS ) 
	{
		if( pDir->pItems[ pDir->nSelectedItem ].flags & FLAG_DIRECTORY )
		{
			if( !strcmp( pDir->pItems[ pDir->nSelectedItem ].name, ".." ) )
			{
				nOffset = strlen(pDir->szPath) - 1;
				strncpy( szNewPath, pDir->szPath, nOffset );
				szNewPath[ nOffset ] = 0;

				pStr = strrchr( szNewPath, '/' );

				if( !pStr )
				{
					// path must be pfs0:, cdfs: or mc0: etc. and user attempts to switch
					// to next higher level. take him back to the 'select device'
					// screen.
					// we can just re-init the control for that
					GUI_Ctrl_Dirview_Init(pCtrl);
					GUI_Render();

					pCtrl->pParent->pfnCallback(	pCtrl->pParent, GUI_MSG_CONTROL,
													MAKEPARAM( pCtrl->nID, GUI_NOT_DIRVIEW_DIR ),
													0 );
					return 1;
				}

				nOffset = pStr - szNewPath + 1;
				
				strncpy( pDir->szPath, szNewPath, nOffset );
				pDir->szPath[ nOffset ] = 0;
			}
			else
			{
				snprintf(	pDir->szPath, MAX_PATH, "%s%s/", pDir->szPath,
							pDir->pItems[ pDir->nSelectedItem ].name );
			}

			GUI_Ctrl_Dirview_SetDir( pCtrl, pDir->szPath );
			GUI_Render();

			pCtrl->pParent->pfnCallback(	pCtrl->pParent, GUI_MSG_CONTROL,
											MAKEPARAM( pCtrl->nID, GUI_NOT_DIRVIEW_DIR ),
											0 );
			return 1;
		}
		else
		{
			pCtrl->pParent->pfnCallback(	pCtrl->pParent, GUI_MSG_CONTROL,
											MAKEPARAM( pCtrl->nID, GUI_NOT_DIRVIEW_CLICKED ),
											pDir->nSelectedItem );
			return 1;
		}
	}
	else if( nPadBtns & PAD_SQUARE )
	{
		if( !strcmp( pDir->pItems[ pDir->nSelectedItem ].name, "." ) ||
			!strcmp( pDir->pItems[ pDir->nSelectedItem ].name, ".." ) )
			return 1;

		if( pDir->pItems[ pDir->nSelectedItem ].flags & FLAG_MARKED )
		{
			pDir->pItems[ pDir->nSelectedItem ].flags &= ~FLAG_MARKED;
			
			pCtrl->pParent->pfnCallback(	pCtrl->pParent, GUI_MSG_CONTROL,
											MAKEPARAM( pCtrl->nID, GUI_NOT_DIRVIEW_UNMARK ),
											0 );
		}
		else
		{
			pDir->pItems[ pDir->nSelectedItem ].flags |= FLAG_MARKED;

			pCtrl->pParent->pfnCallback(	pCtrl->pParent, GUI_MSG_CONTROL,
											MAKEPARAM( pCtrl->nID, GUI_NOT_DIRVIEW_MARK ),
											0 );
		}

		return 1;
	}

	return 0;
}

void GUI_Ctrl_Dirview_Refresh( GUIControl_t *pCtrl )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( pDir->szPath[0] )
	{
		GUI_Ctrl_Dirview_SetDir( pCtrl, pDir->szPath );
	}
	else
	{
		GUI_Ctrl_Dirview_Init(pCtrl);
	}
}

const fileInfo_t *GUI_Ctrl_Dirview_GetSel( const GUIControl_t *pCtrl )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	return &pDir->pItems[ pDir->nSelectedItem ];
}

unsigned int GUI_Ctrl_Dirview_NumMark( const GUIControl_t *pCtrl )
{
	unsigned int i, nNum;
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	for( i = 0, nNum = 0; i < pDir->nNumItems; i++ )
	{
		if( pDir->pItems[i].flags & FLAG_MARKED )
			nNum++;
	}

	return nNum;
}

const fileInfo_t *GUI_Ctrl_Dirview_GetMarked( const GUIControl_t *pCtrl, unsigned int n )
{
	unsigned int i, nNum;
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( n >= pDir->nNumItems )
		return NULL;

	for( i = 0, nNum = 0; i < pDir->nNumItems; i++ )
	{
		if( pDir->pItems[i].flags & FLAG_MARKED )
		{
			if( nNum == n )
				return &pDir->pItems[i];

			nNum++;
		}
	}

	return NULL;
}

void GUI_Ctrl_Dirview_SetMarked( const GUIControl_t *pCtrl, unsigned int nIndex,
								 unsigned int bMarked )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( nIndex >= pDir->nNumItems )
		return;

	if( bMarked )
		pDir->pItems[nIndex].flags |= FLAG_MARKED;
	else
		pDir->pItems[nIndex].flags &= ~FLAG_MARKED;
}

void GUI_Ctrl_Dirview_SetFilter( GUIControl_t *pCtrl, const char *pFilter )
{
	unsigned int nSize;
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( !pFilter )
	{
		pDir->szFilter[0] = 0;
		return;
	}

	nSize = sizeof(pDir->szFilter) - 1;
	strncpy( pDir->szFilter, pFilter, nSize );
	pDir->szFilter[ nSize ] = 0;
}

int GUI_Ctrl_Dirview_IsRoot( GUIControl_t *pCtrl )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	return (pDir->szPath[0] == 0);
}

unsigned int GUI_Ctrl_Dirview_GetCount( const GUIControl_t *pCtrl )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	return pDir->nNumItems;
}

const fileInfo_t *GUI_Ctrl_Dirview_GetItem( const GUIControl_t *pCtrl, unsigned int nIndex )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	if( nIndex >= pDir->nNumItems )
		return NULL;

	return &pDir->pItems[nIndex];
}

void GUI_Ctrl_Dirview_Sort( GUIControl_t *pCtrl, int nSortFunc )
{
	GUICtrl_Dirview_t *pDir = pCtrl->pCtrl;

	switch(nSortFunc)
	{
		case GUI_SORT_NAME:
			qsort( pDir->pItems, pDir->nNumItems, sizeof(fileInfo_t), SortPanelByName );
			break;

		case GUI_SORT_SIZE:
			qsort( pDir->pItems, pDir->nNumItems, sizeof(fileInfo_t), SortPanelBySize );
			break;

		case GUI_SORT_TYPE:
			qsort( pDir->pItems, pDir->nNumItems, sizeof(fileInfo_t), SortPanelByType );
			break;
	}
}
