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
#		"Radio" menu code
#
*/

#include <tamtypes.h>
#include <ui.h>
#include <net.h>

#define ID_RETRIEVE_LIST	4
#define ID_SORT_NAME		5
#define ID_SORT_GENRE		6
#define ID_SORT_BITRATE		7
#define ID_SORT_LISTENERS	8
#define ID_BOOKMARKS		9
#define ID_GOBACK			10
#define ID_STATION_LIST		11
#define ID_BOOKMARK_LIST	12

typedef struct {
	menuFramework_t	*menu;

	menuText_t		List;
	menuText_t		Sort;
	menuText_t		SortName;
	menuText_t		SortGenre;
	menuText_t		SortBR;
	menuText_t		SortLC;
	menuText_t		Bookmarks;
	menuText_t		GoBack;

	menuList_t		StationList;
	menuList_t		BmList;
	const char		*pStatus;

} uiRadioMenu_t;

static uiRadioMenu_t s_radio;

// keep station list in memory so it can be loaded faster
static shoutcastStation_t	*pStations 		= NULL;
static int					nNumStations	= 0;		// number of entries in pStations
static int					nNextRefresh	= 0;		// next time we are allowed to
														// refresh station list

static bookmark_t			*pBookmarks		= NULL;
static int					nNumBookmarks	= 0;

//
// UI_InitRadioMenu - Initializes Radio Menu Controls
//

void UI_InitRadioMenu( void )
{
	int y;

	s_radio.menu						= &uis.menus[ MENU_RADIO ];

	s_radio.menu->callback				= UI_RadioCallback;
	s_radio.menu->input					= NULL;

	s_radio.menu->numItems				= 0;
	s_radio.menu->selectedItem			= 0;

	y = 70;

	s_radio.List.generic.type			= MENU_CONTROL_TEXT;
	s_radio.List.generic.id				= ID_RETRIEVE_LIST;
	s_radio.List.generic.x				= 35;
	s_radio.List.generic.y				= y;
	s_radio.List.generic.flags			= 0;
	s_radio.List.text					= "Retrieve List";
	s_radio.List.size					= GR_FONT_SMALL;
	s_radio.List.color					= RGB(255, 255, 255);

	y += 30;

	s_radio.Sort.generic.type			= MENU_CONTROL_TEXT;
	s_radio.Sort.generic.x				= 35;
	s_radio.Sort.generic.y				= y;
	s_radio.Sort.generic.flags			= CFL_INACTIVE | CFL_FORCECOLOR;
	s_radio.Sort.text					= "Sort By";
	s_radio.Sort.size					= GR_FONT_SMALL;
	s_radio.Sort.color					= RGB(255, 255, 255);

	y += 20;

	s_radio.SortName.generic.type		= MENU_CONTROL_TEXT;
	s_radio.SortName.generic.id			= ID_SORT_NAME;
	s_radio.SortName.generic.x			= 35;
	s_radio.SortName.generic.y			= y;
	s_radio.SortName.generic.flags		= 0;
	s_radio.SortName.text				= " - Name";
	s_radio.SortName.size				= GR_FONT_SMALL;
	s_radio.SortName.color				= RGB(255, 255, 255);

	y += 15;

	s_radio.SortGenre.generic.type		= MENU_CONTROL_TEXT;
	s_radio.SortGenre.generic.id		= ID_SORT_GENRE;
	s_radio.SortGenre.generic.x			= 35;
	s_radio.SortGenre.generic.y			= y;
	s_radio.SortGenre.generic.flags		= 0;
	s_radio.SortGenre.text				= " - Genre";
	s_radio.SortGenre.size				= GR_FONT_SMALL;
	s_radio.SortGenre.color				= RGB(255, 255, 255);

	y += 15;

	s_radio.SortBR.generic.type			= MENU_CONTROL_TEXT;
	s_radio.SortBR.generic.id			= ID_SORT_BITRATE;
	s_radio.SortBR.generic.x			= 35;
	s_radio.SortBR.generic.y			= y;
	s_radio.SortBR.generic.flags		= 0;
	s_radio.SortBR.text					= " - Bitrate";
	s_radio.SortBR.size					= GR_FONT_SMALL;
	s_radio.SortBR.color				= RGB(255, 255, 255);

	y += 15;

	s_radio.SortLC.generic.type			= MENU_CONTROL_TEXT;
	s_radio.SortLC.generic.id			= ID_SORT_LISTENERS;
	s_radio.SortLC.generic.x			= 35;
	s_radio.SortLC.generic.y			= y;
	s_radio.SortLC.generic.flags		= 0;
	s_radio.SortLC.text					= " - Listeners";
	s_radio.SortLC.size					= GR_FONT_SMALL;
	s_radio.SortLC.color				= RGB(255, 255, 255);

	y += 30;

	s_radio.Bookmarks.generic.type		= MENU_CONTROL_TEXT;
	s_radio.Bookmarks.generic.id		= ID_BOOKMARKS;
	s_radio.Bookmarks.generic.x			= 35;
	s_radio.Bookmarks.generic.y			= y;
	s_radio.Bookmarks.generic.flags		= 0;
	s_radio.Bookmarks.text				= "Bookmarks";
	s_radio.Bookmarks.size				= GR_FONT_SMALL;
	s_radio.Bookmarks.color				= RGB(255, 255, 255);

	y += 20;

	s_radio.GoBack.generic.type			= MENU_CONTROL_TEXT;
	s_radio.GoBack.generic.id			= ID_GOBACK;
	s_radio.GoBack.generic.x			= 35;
	s_radio.GoBack.generic.y			= y;
	s_radio.GoBack.generic.flags		= 0;
	s_radio.GoBack.text					= "Go Back";
	s_radio.GoBack.size					= GR_FONT_SMALL;
	s_radio.GoBack.color				= RGB(255, 255, 255);

	s_radio.StationList.generic.type	= MENU_CONTROL_LIST;
	s_radio.StationList.generic.id		= ID_STATION_LIST;
	s_radio.StationList.generic.x		= 200;
	s_radio.StationList.generic.y		= 40;
	s_radio.StationList.generic.flags	= 0;
	s_radio.StationList.width			= 400;
	s_radio.StationList.height			= 200;
	s_radio.StationList.color			= RGB(81,112, 164);

	s_radio.BmList.generic.type			= MENU_CONTROL_LIST;
	s_radio.BmList.generic.id			= ID_BOOKMARK_LIST;
	s_radio.BmList.generic.x			= 200;
	s_radio.BmList.generic.y			= 40;
	s_radio.BmList.generic.flags		= CFL_INACTIVE|CFL_INVISIBLE;
	s_radio.BmList.width				= 400;
	s_radio.BmList.height				= 200;
	s_radio.BmList.color				= RGB(81,112, 164);

	UI_AddItemToMenu( s_radio.menu, &s_radio.List );
	UI_AddItemToMenu( s_radio.menu, &s_radio.Sort );
	UI_AddItemToMenu( s_radio.menu, &s_radio.SortName );
	UI_AddItemToMenu( s_radio.menu, &s_radio.SortGenre );
	UI_AddItemToMenu( s_radio.menu, &s_radio.SortBR );
	UI_AddItemToMenu( s_radio.menu, &s_radio.SortLC );
	UI_AddItemToMenu( s_radio.menu, &s_radio.Bookmarks );
	UI_AddItemToMenu( s_radio.menu, &s_radio.GoBack );
	UI_AddItemToMenu( s_radio.menu, &s_radio.StationList );
	UI_AddItemToMenu( s_radio.menu, &s_radio.BmList );

	// look for Winamp.bm
	UI_RadioLoadBookmarks();

	s_radio.pStatus	= "Idle";

	// if station list is already in memory, fill listview control with it
	if( (pStations) && (nNumStations > 0) ) {
		UI_RadioRefreshListControl();
		return;
	}

	// if user has an old station list saved on HDD we can parse that
	if( SC_GetValueForKey_Int( "radio_cache_list", NULL ) )
	{
		char		strPath[256];
		FHANDLE		fHandle;
		int			nSize;
		char		*pXMLFile;

		SC_GetValueForKey_Str( "radio_cache_path", strPath );
		strcat( strPath, "STATIONS.XML" );

		fHandle = FileOpen( strPath, O_RDONLY );
		if( fHandle.fh < 0 )
			return;

		nSize = FileSeek( fHandle, 0, SEEK_END );
		if( !nSize ) {
			FileClose( fHandle );
			return;
		}

		FileSeek( fHandle, 0, SEEK_SET );

		pXMLFile = (char*) malloc( nSize );
		if( pXMLFile == NULL )
			return;

		FileRead( fHandle, pXMLFile, nSize );
		FileClose( fHandle );

		s_radio.pStatus = "Parsing HDD station list...please wait";
		UI_Refresh();
		
		if( UI_RadioParseStationXML( pXMLFile ) == 0 )
			s_radio.pStatus = "Error parsing Station List";
		else
			s_radio.pStatus	= "Idle";

		free( pXMLFile );

		// fill list control with entries
		UI_RadioRefreshListControl();
	}


}

//
// UI_RadioCallback
//

char	strStationURL[256];

int UI_RadioCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	int		nRet;
//	char	strStationURL[256];

	switch( nMsg )
	{
		case MSG_DRAW:
			UI_RadioDraw();
			return 1;

		// clean up list entries
		case MSG_CLOSE:
			UI_List_Clear( &s_radio.StationList );
			UI_List_Clear( &s_radio.BmList );

			// clean up bookmarks
			UI_RadioFreeBookmarks();
			return 1;

		case MSG_CONTROL:
			switch( sParam )
			{
				case ID_RETRIEVE_LIST:
					// make sure net thread is shut down before downloading
					MP3_Stop();

					UI_RadioGetStations();
					UI_Refresh();
					return 1;

				case ID_SORT_NAME:
					qsort( pStations, nNumStations, sizeof(shoutcastStation_t), UI_RadioSortName );
					UI_RadioRefreshListControl();
					return 1;

				case ID_SORT_GENRE:
					qsort( pStations, nNumStations, sizeof(shoutcastStation_t), UI_RadioSortGenre );
					printf("Sorted\n");
					UI_RadioRefreshListControl();
					return 1;

				case ID_SORT_BITRATE:
					qsort( pStations, nNumStations, sizeof(shoutcastStation_t), UI_RadioSortBR );
					UI_RadioRefreshListControl();
					return 1;

				case ID_SORT_LISTENERS:
					qsort( pStations, nNumStations, sizeof(shoutcastStation_t), UI_RadioSortLC );
					UI_RadioRefreshListControl();
					return 1;

				case ID_GOBACK:
					UI_SetActiveMenu(MENU_MYMUSIC);
					return 1;

				case ID_STATION_LIST:
					switch( fParam )
					{
						// update station information
						case NOT_LV_SEL_CHANGED:
							UI_Refresh();
							return 1;

						case NOT_LV_CLICKED_ENTRY:
							nRet = UI_List_GetSelIndex( &s_radio.StationList );

							if( nRet < 0 || nRet > nNumStations )
								return 1;

							// send/recv not thread safe so be sure any running
							// net thread is shut down before downloading
							MP3_Stop();

							nRet = UI_RadioGetStationURL( &pStations[ nRet ], strStationURL, sizeof(strStationURL) );
							if( !nRet )
								return 1;

							// try to connect
							s_radio.pStatus = "Connecting to server...please wait";
							UI_Refresh();

							nRet = MP3_OpenStream( strStationURL );
							if( nRet != STREAM_ERROR_OK ) {
								s_radio.pStatus = MP3_GetStreamError( nRet );
							}
							else {
								s_radio.pStatus = "Connected, starting playback";
								MP3_PlayStream();
							}

							UI_Refresh();
							return 1;
					}
					return 1;

				case ID_BOOKMARK_LIST:
					switch( fParam )
					{
						case NOT_LV_CLICKED_ENTRY:
							nRet = UI_List_GetSelIndex( &s_radio.BmList );

							if( nRet < 0 || nRet > nNumBookmarks )
								return 1;

							// try to connect
							s_radio.pStatus = "Connecting to server...please wait";
							UI_Refresh();

							nRet = MP3_OpenStream( pBookmarks[ nRet ].url );
							if( nRet != STREAM_ERROR_OK ) {
								s_radio.pStatus = MP3_GetStreamError( nRet );
							}
							else {
								s_radio.pStatus = "Connected, starting playback";
								MP3_PlayStream();
							}

							UI_Refresh();
							return 1;
					}
					return 1;

				// switch between bookmark- and station list
				case ID_BOOKMARKS:
					if( s_radio.BmList.generic.flags & CFL_INVISIBLE ) {
						// show bookmark list
						s_radio.BmList.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);

						// hide station list
						s_radio.StationList.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

						s_radio.Bookmarks.text = "Station List";
					}
					else {
						// hide bookmark list
						s_radio.BmList.generic.flags |= (CFL_INVISIBLE | CFL_INACTIVE);

						// show station list
						s_radio.StationList.generic.flags &= ~(CFL_INVISIBLE | CFL_INACTIVE);

						s_radio.Bookmarks.text = "Bookmarks";
					}
					UI_Refresh();
					return 1;

			}
			return 1;
	}
	return 0;
}

//
// UI_RadioDraw
//

void UI_RadioDraw( void )
{
	int		nSelected;
	char	strTmp[256];
	int		nMaxWidth, nIndex;

	const shoutcastStation_t *pEntry;

	// draw sidebar
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_DrawRoundRect( 200, 250, 400, 155 );
	GR_SetBlendMode( GR_BLEND_NONE );

	// draw controls
	UI_DrawMenu( s_radio.menu );

	GR_SetFontColor( RGB(255, 255, 255) );
	GR_DrawTextExt( 210, 25, "Select a Radio Station", GR_FONT_SMALL );

	// station information
	GR_DrawTextExt( 210, 260, "Station Information", GR_FONT_SMALL );

	// this is an offset into the pStations array
	nSelected = UI_List_GetSelIndex( &s_radio.StationList );

	GR_SetFontColor( RGBA(81, 112, 164, 64) );

	if( (nSelected < 0) || !(pStations) || (nSelected >= nNumStations) ) {
		GR_DrawTextExt( 220, 280, "No station selected", GR_FONT_SMALL );
	}
	else
	{
		pEntry = &pStations[ nSelected ];

		// draw station description
		nMaxWidth = 360;

		if( pEntry->name )
		{
			strncpy( strTmp, pEntry->name, 256 );
			strTmp[255] = 0x0;

			nIndex = strlen(strTmp) - 1;

			while( GR_GetStringWidth( strTmp, GR_FONT_SMALL ) > nMaxWidth ) {
				strTmp[ nIndex ] = 0;

				if( nIndex > 1 ) {
					strTmp[ nIndex - 1 ] = '.';
					strTmp[ nIndex - 2 ] = '.';
				}

				nIndex--;
			}

			GR_DrawTextExt( 220, 280, strTmp, GR_FONT_SMALL );
		}

		GR_SetFontColor( RGB(255, 255, 255) );
		GR_DrawTextExt( 220, 305, "Genre", GR_FONT_SMALL );
		GR_DrawTextExt( 220, 320, "Playing", GR_FONT_SMALL );
		GR_DrawTextExt( 220, 335, "Listeners", GR_FONT_SMALL );
		GR_DrawTextExt( 220, 350, "Bitrate", GR_FONT_SMALL );

		GR_SetFontColor( RGBA(81, 112, 164, 64) );

		nMaxWidth = 265;
		
		// draw genre
		if( pEntry->genre )
		{
			strncpy( strTmp, pEntry->genre, 256 );
			strTmp[255] = 0x0;

			nIndex = strlen(strTmp) - 1;

			while( GR_GetStringWidth( strTmp, GR_FONT_SMALL ) > nMaxWidth ) {
				strTmp[ nIndex ] = 0;

				if( nIndex > 1 ) {
					strTmp[ nIndex - 1 ] = '.';
					strTmp[ nIndex - 2 ] = '.';
				}

				nIndex--;
			}

			GR_DrawTextExt( 325, 305, strTmp, GR_FONT_SMALL );
		}

		// draw currently playing
		if( pEntry->playing )
		{
			strncpy( strTmp, pEntry->playing, 256 );
			strTmp[255] = 0x0;

			nIndex = strlen(strTmp) - 1;

			while( GR_GetStringWidth( strTmp, GR_FONT_SMALL ) > nMaxWidth ) {
				strTmp[ nIndex ] = 0;

				if( nIndex > 1 ) {
					strTmp[ nIndex - 1 ] = '.';
					strTmp[ nIndex - 2 ] = '.';
				}

				nIndex--;
			}

			GR_DrawTextExt( 325, 320, strTmp, GR_FONT_SMALL );
		}

		// draw number of listeners
		sprintf( strTmp, "%i", pEntry->listeners );
		GR_DrawTextExt( 325, 335, strTmp, GR_FONT_SMALL );

		// draw bitrate
		sprintf( strTmp, "%i Kbps", pEntry->bitrate );
		GR_DrawTextExt( 325, 350, strTmp, GR_FONT_SMALL );
	}

	GR_SetFontColor( RGB(255, 255, 255) );
	GR_DrawTextExt( 210, 375, "Status :", GR_FONT_SMALL );
	GR_DrawTextExt( 280, 375, s_radio.pStatus, GR_FONT_SMALL );
}

//
// UI_RadioDownload - Callback function called while station list is download
//

static int UI_RadioDownload( int nRecvBytes, int nTotalBytes )
{
	static char strBuf[128];

	sprintf( strBuf, "Downloading Station List (%i KB)", nRecvBytes / 1024 );

	s_radio.pStatus = strBuf;
	UI_Refresh();

	return 1;
}

//
// UI_RadioGetStations - Retrieves a list of radio stations from shoutcast.com
//

int UI_RadioGetStations( void )
{
	char	*pBuffer;
	int		nBufSize;
	int		nRet;
	char	strPath[256];
	FHANDLE	fHandle;

	// only allow to refresh once in 5 minutes
	if( nNextRefresh > tnTimeMsec() ) {
		s_radio.pStatus = "Can only refresh list every 5 minutes";
		return 0;
	}

	s_radio.pStatus = "Connecting to server";
	UI_Refresh();

	nRet = HttpDownload( SHOUTCAST_HOST, SHOUTCAST_PORT, SHOUTCAST_FILE,
						 &pBuffer, &nBufSize, UI_RadioDownload );

	// some error occured
	if( nRet != 1 ) {
		s_radio.pStatus = GetHttpError( nRet );
		return 0;
	}

	if( SC_GetValueForKey_Int( "radio_cache_list", NULL ) ) {

		// save xml file to hdd
		SC_GetValueForKey_Str( "radio_cache_path", strPath );

		// make sure directory path exists
		DirCreate( strPath );

		strcat( strPath, "STATIONS.XML" );

		// fixme shouldnt have to do this
		fHandle = FileOpen( strPath, O_RDONLY );
		if( fHandle.fh >= 0 ) {
			FileClose(fHandle);
			FileRemove( strPath );
		}

		fHandle = FileOpen( strPath, O_WRONLY | O_CREAT | O_TRUNC );
		if( fHandle.fh >= 0 ) {
			FileWrite( fHandle, pBuffer, nBufSize );
			FileClose( fHandle );
		}
	}

	s_radio.pStatus = "Parsing station list...please wait";
	UI_Refresh();

	if( UI_RadioParseStationXML( pBuffer ) == 0 )
		s_radio.pStatus = "Error parsing Station List";
	else
		s_radio.pStatus = "Station List download complete";

	// free memory allocated by HttpDownload
	free( pBuffer );

	// fill list control with entries
	UI_RadioRefreshListControl();

	nNextRefresh = tnTimeMsec() + 1000 * 60 * 5;
	return 1;
}

//
// UI_RadioRefreshListControl - Clears listview control and re-fills it
//								with entries from the global pStations
//								array.
//

void UI_RadioRefreshListControl( void )
{
	int i;

	// do nothing if we haven't loaded stations
	if( !pStations || !nNumStations )
		return;

	// clear all old entries
	UI_List_Clear( &s_radio.StationList );

	// add entries
	for( i = 0; i < nNumStations; i++ )
	{
		if( pStations[i].name )
			UI_List_AddString( &s_radio.StationList, pStations[i].name );
	}
}

//
// UI_RadioParseStationXML - Parses the stations.xml file and populates
//							 the global pStations array.
//
//							 Returns 1 on success, otherwise 0.
//

int UI_RadioParseStationXML( const char *pXMLFile )
{
	int i;

	McbXMLElement	*pRoot;
	McbXMLNode		node;
	McbXMLResults	Results;

	if( !pXMLFile )
		return 0;

	// clean up existing entries first
	if( pStations )
	{
		for( i = 0; i < nNumStations; i++ ) {
			if( pStations[i].name )
				free( pStations[i].name );

			if( pStations[i].genre )
				free( pStations[i].genre );

			if( pStations[i].url )
				free( pStations[i].url );

			if( pStations[i].playing )
				free( pStations[i].playing );
		}
	}

	pStations		= NULL;
	nNumStations	= 0;

	// read the xml file
	if( (pRoot = McbParseXML( pXMLFile, &Results )) == NULL ) {
#ifdef _DEBUG
		printf("UI_RadioParseStationXML: Error while parsing xml file\n");
		printf("Error %i in Line %i, Column %i\n", Results.error, Results.nLine, Results.nColumn );
#endif
		return 0;
	}

	// there should be 500 entries in the xml file
	pStations = (shoutcastStation_t*) malloc( sizeof(shoutcastStation_t) * SHOUTCAST_STATIONS );

	if( pStations == NULL ) {
#ifdef _DEBUG
		printf("UI_RadioParseStationXML: malloc() failed for pStations!\n");
#endif
		McbDeleteRoot(pRoot);
		return 0;
	}

	node.node.pElement	= pRoot;
	node.type			= eNodeElement;

	ParseXMLTree( &node );

	McbDeleteRoot(pRoot);

#ifdef _DEBUG
	printf("UI_RadioParseStationXML: number of stations read : %i\n", nNumStations );
#endif
	return 1;
}

//
// ParseXMLTree	- Helper
//				  Recursively goes through the nodes of the xml file.
//

void ParseXMLTree( McbXMLNode *pNode )
{
	McbXMLNode *pChild;
	int			nIndex	= 0;

	while( (pChild = McbEnumNodes( pNode->node.pElement, &nIndex)) ) {
		switch( pChild->type )
		{
			// found a radio station entry
			case eNodeElement:
				if( !strcmp( "entry", pChild->node.pElement->lpszName) ) {

					if( nNumStations >= SHOUTCAST_STATIONS )
						break;

					ParseStationEntry( pChild );
					nNumStations++;
				}
				else {
					ParseXMLTree(pChild);
				}
				break;

			default:
				break;
		}
	}
}

//
// ParseStationEntry - Helper
//					   Parses a radio station entry from the xml file.
//

void ParseStationEntry( McbXMLNode *pNode )
{
	McbXMLNode	*pChild;
	int			nIndex	= 0;
	char		*p;
	int			nLen;

	while( (pChild = McbEnumNodes( pNode->node.pElement, &nIndex)) )
	{
		switch( pChild->type )
		{
			case eNodeAttribute:
				if( !strcmp( pChild->node.pAttrib->lpszName, "Playstring" ) )
				{
					if( !strcmp( pNode->node.pElement->lpszName, "entry" ) )
					{
						p = (char*) malloc( strlen( pChild->node.pAttrib->lpszValue) + 1 );
						if( p == NULL )
						{
#ifdef _DEBUG
							printf("ParseStationEntry: malloc() failed!\n");
#endif
							return;
						}
						
						strcpy( p, pChild->node.pAttrib->lpszValue );

						pStations[ nNumStations ].url = p;
					}
				}
				break;

			case eNodeElement:
				ParseStationEntry( pChild );
				break;

			case eNodeText:

				nLen = strlen(pChild->node.pText->lpszValue) + 1;

				// parse the station name
				if( !strcmp( pNode->node.pElement->lpszName, "Name" ) )
				{
					p = (char*) malloc( nLen );
					if( p == NULL ) {
#ifdef _DEBUG
						printf("ParseStationEntry: malloc() failed!\n");
#endif
						return;
					}

					strcpy( p, pChild->node.pText->lpszValue );
					pStations[ nNumStations ].name = p;
				}
				else if( !strcmp( pNode->node.pElement->lpszName, "Genre" ) )
				{
					// parse station genre
					p = (char*) malloc( nLen );
					if( p == NULL ) {
#ifdef _DEBUG
						printf("ParseStationEntry: malloc() failed!\n");
#endif
						return;
					}

					strcpy( p,pChild->node.pText->lpszValue );
					pStations[ nNumStations ].genre = p;
				}
				else if( !strcmp( pNode->node.pElement->lpszName, "Nowplaying" ) )
				{
					// parse the current track
					p = (char*) malloc( nLen );
					if( p == NULL ) {
#ifdef _DEBUG
						printf("ParseStationEntry: malloc() failed!\n");
#endif
						return;
					}

					strcpy( p, pChild->node.pText->lpszValue );
					pStations[ nNumStations ].playing = p;
				}
				else if( !strcmp( pNode->node.pElement->lpszName, "Bitrate" ) )
				{
					// parse the bitrate of the station
					pStations[ nNumStations ].bitrate = atoi(pChild->node.pText->lpszValue);
				}
				else if( !strcmp( pNode->node.pElement->lpszName, "Listeners" ) )
				{
					// parse the number of listeners of the station
					pStations[ nNumStations ].listeners = atoi(pChild->node.pText->lpszValue);
				}
				break;

			default:
				break;
		}
	}
}

//
// UI_RadioLoadBookmarks - Looks for a Winamp bookmark file and parses it if
//						   it exists.
//

void UI_RadioLoadBookmarks( void )
{
	char	strPath[256];
	char	strLine[1024], strNext[1024];
	FHANDLE	fHandle;
	int		nSize, nRead;
	char	*pFile, *pBufPtr;
	char	*pEnd;

	SC_GetValueForKey_Str( "radio_bookmarks", strPath );

	if(!strPath[0])
		return;

	fHandle = FileOpen( strPath, O_RDONLY );
	if( fHandle.fh < 0 )
		return;

	nSize = FileSeek( fHandle, 0, SEEK_END );
	if( !nSize ) {
		FileClose(fHandle);
		return;
	}

	FileSeek( fHandle, 0, SEEK_SET );
	
	pFile = (char*) malloc( nSize + 1 );
	if( !pFile ) {
#ifdef _DEBUG
		printf("UI_RadioLoadBookmarks: malloc() failed!\n");
#endif
		FileClose(fHandle);
		return;
	}

	nRead = FileRead( fHandle, pFile, nSize );
	pFile[ nRead ] = 0;

	FileClose( fHandle );

	pBufPtr = pFile;

	if( pBookmarks ) {
		free( pBookmarks );

		pBookmarks		= NULL;
		nNumBookmarks	= 0;
	}

	UI_List_Clear( &s_radio.BmList );

	while( ReadBufLine( &pBufPtr, strLine ) )
	{
		pBookmarks = (bookmark_t*) realloc( pBookmarks, (nNumBookmarks + 1) * sizeof(bookmark_t) );
		if( !pBookmarks ) {
#ifdef _DEBUG
			printf("UI_RadioLoadBookmarks: realloc() failed!\n");
#endif
			nNumBookmarks = 0;
			break;
		}

		// first line is the station address, second line station name
		ReadBufLine( &pBufPtr, strNext );

		// remove line end from url
		pEnd = strLine + strlen(strLine) - 1;

		while( *pEnd == '\n' || *pEnd == '\r' || *pEnd == ' ' ) {
			*pEnd = 0;
			pEnd--;
		}

		pBookmarks[ nNumBookmarks ].url = (char*) malloc( strlen(strLine) + 1 );
		
		if( !pBookmarks[ nNumBookmarks ].url ) {
#ifdef _DEBUG
			printf("UI_RadioLoadBookmarks: malloc() failed!\n");
#endif
			break;
		}

		strcpy( pBookmarks[ nNumBookmarks ].url, strLine );

		// remove line end from name
		pEnd = strNext + strlen(strNext) - 1;

		while( *pEnd == '\n' || *pEnd == '\r' || *pEnd == ' ' ) {
			*pEnd = 0;
			pEnd--;
		}

		pBookmarks[ nNumBookmarks ].name = (char*) malloc( strlen(strNext) + 1 );

		if( !pBookmarks[ nNumBookmarks ].name ) {
#ifdef _DEBUG
			printf("UI_RadioLoadBookmarks: malloc() failed!\n");
#endif
			break;
		}

		strcpy( pBookmarks[ nNumBookmarks ].name, strNext );

		UI_List_AddString( &s_radio.BmList, pBookmarks[ nNumBookmarks ].name );
		nNumBookmarks++;
	}

	free(pFile);

#ifdef _DEBUG
	printf("UI_RadioLoadBookmarks: Read %i bookmarks\n", nNumBookmarks);
#endif
}

//
// UI_RadioFreeBookmarks - Frees bookmark memory
//

void UI_RadioFreeBookmarks( void )
{
	if( pBookmarks )
		free( pBookmarks );

	pBookmarks		= NULL;
	nNumBookmarks	= 0;
}

//
// UI_RadioSortName - Sort by name
//

int UI_RadioSortName( const void *a, const void *b )
{
	shoutcastStation_t *p1, *p2;

	p1 = (shoutcastStation_t*)a;
	p2 = (shoutcastStation_t*)b;

	if( !p1->name )
		return -1;

	if( !p2->name )
		return 1;

	return strcmp( p1->name, p2->name );
}

//
// UI_RadioSortGenre - Sort by genre
//

int UI_RadioSortGenre( const void *a, const void *b )
{
	shoutcastStation_t *p1, *p2;

	p1 = (shoutcastStation_t*)a;
	p2 = (shoutcastStation_t*)b;

	if( !p1->genre )
		return -1;

	if( !p2->genre )
		return 1;

	return strcmp( p1->genre, p2->genre );
}

//
// UI_RadioSortBR - Sort by bitrate
//

int UI_RadioSortBR( const void *a, const void *b )
{
	shoutcastStation_t *p1, *p2;

	p1 = (shoutcastStation_t*)a;
	p2 = (shoutcastStation_t*)b;

	return (p1->bitrate - p2->bitrate);
}

//
// UI_RadioSortLC - Sort by listener count
//

int UI_RadioSortLC( const void *a, const void *b )
{
	shoutcastStation_t *p1, *p2;

	p1 = (shoutcastStation_t*)a;
	p2 = (shoutcastStation_t*)b;

	return (p1->listeners - p2->listeners);
}

//
// UI_RadioGetStationURL - Retrieves the URL of a radio stream server.
//
//						   Entries in the XML file don't link to the
//						   stations directly but have a download link
//						   to a playlist which contains the actual
//						   url of the stream server.
//

int UI_RadioGetStationURL( const shoutcastStation_t *pEntry, char *pStationURL, int nSize )
{
	int		nRet;
	char	strHost[256], strFile[256];
	int		nPort;
	char	*pBuffer, *pStr, *pEnd;
	int		nBufSize;
	char	strLine[1024];

	if( !pEntry->url || !pStationURL )
		return 0;

	// extract host, port and filename from url
	nRet = TokenizeURL( pEntry->url, strHost, strFile, &nPort );

	if( !nRet ) {
#ifdef _DEBUG
		printf("UI_RadioGetStationURL: TokenizeURL failed!\n");
#endif
		return 0;
	}

	// download the playlist
	nRet = HttpDownload( strHost, nPort, strFile, &pBuffer, &nBufSize, NULL );

	// some error occured
	if( nRet != 1 ) {
#ifdef _DEBUG
		printf("UI_RadioGetStationURL: HttpDownload failed (%s)!\n", GetHttpError(nRet) );
#endif
		return 0;
	}

	// parse the playlist
	// currently we only care for the first File1 entry and discard
	// all other playlist entries
	if( (pStr = strstr( pBuffer, "File1=" )) == NULL ) {
#ifdef _DEBUG
		printf("UI_RadioGetStationURL: unexpected playlist format!\n");
#endif
		free(pBuffer);
		return 0;
	}

	ReadBufLine( &pStr, strLine );

	// remove line end from url
	pEnd = strLine + strlen(strLine) - 1;

	while( *pEnd == '\n' || *pEnd == '\r' || *pEnd == ' ' ) {
		*pEnd = 0;
		pEnd--;
	}

	// parse URL
	if( (pStr = strchr( strLine, '=' )) == NULL ) {
#ifdef _DEBUG
		printf("UI_RadioGetStationURL: pStr is NULL\n");
#endif
		free(pBuffer);
		return 0;
	}

	// skip '='
	pStr++;

	strncpy( pStationURL, pStr, nSize - 1 );
	pStationURL[ nSize ] = 0;

	free( pBuffer );
	printf("UI_RadioGetStationURL : %s\n", pStationURL );
	return 1;
}
