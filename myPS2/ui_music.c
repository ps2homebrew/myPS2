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
#		"My Music" menu code
#
*/

#include <tamtypes.h>
#include <ui.h>

#define ID_GOBACK		1
#define ID_REPEAT		2
#define	ID_DIRVIEW		3
#define ID_SEEKBAR		4
#define ID_PREV			5
#define ID_PLAY			6
#define ID_PAUSE		7
#define ID_STOP			8
#define ID_NEXT			9
#define ID_VOLUMEBAR	10

typedef struct {
	menuFramework_t	*menu;

	menuText_t		GoBack;
	menuText_t		Repeat;

	menuDirView_t	DirView;

	menuSlider_t	SeekBar;

	menuButton_t	Prev;
	menuButton_t	Play;
	menuButton_t	Pause;
	menuButton_t	Stop;
	menuButton_t	Next;

	menuSlider_t	VolumeBar;

} uiMyMusicMenu_t;

static uiMyMusicMenu_t s_music;

//
// UI_InitMyMusicMenu - Initializes Controls
//

void UI_InitMyMusicMenu( void )
{
	int nPos_x, nPos_y;

	s_music.menu					= &uis.menus[ MENU_MYMUSIC ];

	s_music.menu->callback			= UI_MyMusicCallback;
	s_music.menu->input				= NULL;

	s_music.menu->numItems			= 0;
	s_music.menu->selectedItem		= 0;

	s_music.Repeat.generic.type		= MENU_CONTROL_TEXT;
	s_music.Repeat.generic.flags	= 0;
	s_music.Repeat.generic.x		= 35;
	s_music.Repeat.generic.y		= 70;
	s_music.Repeat.generic.id		= ID_REPEAT;
	s_music.Repeat.text				= "Toggle Loop";
	s_music.Repeat.size				= GR_FONT_SMALL;
	s_music.Repeat.color			= RGB(255, 255, 255);

	s_music.GoBack.generic.type		= MENU_CONTROL_TEXT;
	s_music.GoBack.generic.flags	= 0;
	s_music.GoBack.generic.x		= 35;
	s_music.GoBack.generic.y		= 100;
	s_music.GoBack.generic.id		= ID_GOBACK;
	s_music.GoBack.text				= "Go Back";
	s_music.GoBack.size				= GR_FONT_SMALL;
	s_music.GoBack.color			= RGB(255, 255, 255);

	s_music.DirView.generic.type	= MENU_CONTROL_DIRVIEW;
	s_music.DirView.generic.flags	= 0;
	s_music.DirView.generic.x		= 200;
	s_music.DirView.generic.y		= 40;
	s_music.DirView.generic.id		= ID_DIRVIEW;
	s_music.DirView.width			= 400;
	s_music.DirView.height			= 200;
	s_music.DirView.color			= RGB(81,112, 164);
	s_music.DirView.filter			= "mp3 m3u";	// only show .mp3 and .m3u files

	s_music.SeekBar.generic.type	= MENU_CONTROL_SLIDER;
	s_music.SeekBar.generic.flags	= CFL_SL_HORIZONTAL | CFL_INACTIVE; // this is read only for now
	s_music.SeekBar.generic.x		= 230;
	s_music.SeekBar.generic.y		= 330;
	s_music.SeekBar.generic.id		= ID_SEEKBAR;
	s_music.SeekBar.barColor		= RGBA( 82, 117, 168, 40 );
	s_music.SeekBar.width			= 150;
	s_music.SeekBar.height			= 10;

	nPos_x = 210;
	nPos_y = 370;

	s_music.Prev.generic.type		= MENU_CONTROL_BUTTON;
	s_music.Prev.generic.flags		= CFL_OWNERDRAW;
	s_music.Prev.generic.x			= nPos_x;
	s_music.Prev.generic.y			= nPos_y;
	s_music.Prev.generic.id			= ID_PREV;

	nPos_x += 40;

	s_music.Play.generic.type		= MENU_CONTROL_BUTTON;
	s_music.Play.generic.flags		= CFL_OWNERDRAW;
	s_music.Play.generic.x			= nPos_x;
	s_music.Play.generic.y			= nPos_y;
	s_music.Play.generic.id			= ID_PLAY;

	nPos_x += 40;

	s_music.Pause.generic.type		= MENU_CONTROL_BUTTON;
	s_music.Pause.generic.flags		= CFL_OWNERDRAW;
	s_music.Pause.generic.x			= nPos_x;
	s_music.Pause.generic.y			= nPos_y;
	s_music.Pause.generic.id		= ID_PAUSE;

	nPos_x += 40;

	s_music.Stop.generic.type		= MENU_CONTROL_BUTTON;
	s_music.Stop.generic.flags		= CFL_OWNERDRAW;
	s_music.Stop.generic.x			= nPos_x;
	s_music.Stop.generic.y			= nPos_y;
	s_music.Stop.generic.id			= ID_STOP;

	nPos_x += 40;

	s_music.Next.generic.type		= MENU_CONTROL_BUTTON;
	s_music.Next.generic.flags		= CFL_OWNERDRAW;
	s_music.Next.generic.x			= nPos_x;
	s_music.Next.generic.y			= nPos_y;
	s_music.Next.generic.id			= ID_NEXT;

	s_music.VolumeBar.generic.type	= MENU_CONTROL_SLIDER;
	s_music.VolumeBar.generic.flags	= CFL_SL_HORIZONTAL;
	s_music.VolumeBar.generic.x		= 425;
	s_music.VolumeBar.generic.y		= 375;
	s_music.VolumeBar.generic.id	= ID_VOLUMEBAR;
	s_music.VolumeBar.barColor		= RGBA( 82, 117, 168, 40 );
	s_music.VolumeBar.width			= 100;
	s_music.VolumeBar.height		= 10;

	UI_AddItemToMenu( s_music.menu, &s_music.Repeat );
	UI_AddItemToMenu( s_music.menu, &s_music.GoBack );
	UI_AddItemToMenu( s_music.menu, &s_music.DirView );
	UI_AddItemToMenu( s_music.menu, &s_music.SeekBar );
	UI_AddItemToMenu( s_music.menu, &s_music.Prev );
	UI_AddItemToMenu( s_music.menu, &s_music.Play );
	UI_AddItemToMenu( s_music.menu, &s_music.Pause );
	UI_AddItemToMenu( s_music.menu, &s_music.Stop );
	UI_AddItemToMenu( s_music.menu, &s_music.Next );
	UI_AddItemToMenu( s_music.menu, &s_music.VolumeBar );

	// setup volume slider
	UI_Slider_SetBounds( &s_music.VolumeBar, 0, 0x3FFF );
	UI_Slider_SetPos( &s_music.VolumeBar, MP3_GetVolume() );
	UI_Slider_SetStepSize( &s_music.VolumeBar, 0x3FF );
}

//
// UI_MyMusicCallback
//

int UI_MyMusicCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	const fileInfo_t	*pFileInfo;
	const char			*pDir;
	char				strBuf[256];

	switch( nMsg )
	{
		case MSG_CONTROL:
			switch( sParam )
			{
				case ID_GOBACK:
					UI_SetActiveMenu(MENU_MAIN);
					return 1;

				case ID_REPEAT:
					MP3_SetLooping( !MP3_GetLooping() );
					UI_Refresh();
					return 1;

				case ID_DIRVIEW:
					switch( fParam )
					{
						case NOT_DV_CLICKED_ENTRY:
							// build path
							pDir		= UI_DirView_GetDir( &s_music.DirView );
							pFileInfo	= UI_DirView_GetSelected( &s_music.DirView );

							strcpy( strBuf, pDir );
							strcat( strBuf, pFileInfo->name );

							MP3_Play( strBuf );
							return 1;
					}
					return 1;

				case ID_PREV:
					MP3_PrevTrack();
					return 1;

				case ID_PLAY:
					MP3_Play(NULL);
					return 1;

				case ID_PAUSE:
					MP3_SetPause( !(MP3_GetStatus() == MP3_PAUSED) );
					UI_Refresh();
					return 1;

				case ID_STOP:
					MP3_Stop();
					return 1;

				case ID_NEXT:
					MP3_NextTrack();
					return 1;

				case ID_VOLUMEBAR:
					switch( fParam )
					{
						case NOT_SL_POS_CHANGED:
							MP3_SetVolume( UI_Slider_GetPos( &s_music.VolumeBar) );
							return 1;
					}
					return 1;
			}
			return 1;

		case MSG_DRAW:
			UI_MyMusicDraw();
			return 1;

		// ownerdraw
		case MSG_DRAWITEM:
			switch( sParam )
			{
				case ID_PREV:
					UI_MyMusicDrawPrev();
					return 1;

				// draw play button
				case ID_PLAY:
					UI_MyMusicDrawPlay();
					return 1;

				case ID_PAUSE:
					UI_MyMusicDrawPause();
					return 1;

				case ID_STOP:
					UI_MyMusicDrawStop();
					return 1;

				case ID_NEXT:
					UI_MyMusicDrawNext();
					return 1;
			}
			return 1;
	}

	return 0;
}


//
// UI_MyMusicDraw
//

void UI_MyMusicDraw( void )
{
	int nMaxWidth, nIndex;
	const char *pStr;
	char strName[256], strTime[256];
	int nMins, nSecs;

	// draw sidebar
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_DrawRoundRect( 200, 250, 400, 155 );
	GR_SetBlendMode( GR_BLEND_NONE );

	// update seekbar position
	UI_Slider_SetBounds( &s_music.SeekBar, 0, MP3_GetTrackLength() + 1 );
	UI_Slider_SetPos( &s_music.SeekBar, MP3_GetCurrentTime() );

	// draw controls
	UI_DrawMenu( s_music.menu );

	GR_SetFontColor( RGB(255, 255, 255) );
	GR_DrawTextExt( 210, 25, "Select an MP3 or M3U File", GR_FONT_SMALL );

	GR_DrawTextExt( 210, 260, "Time Display", GR_FONT_SMALL );

	pStr = MP3_GetStatus() ? MP3_GetTrackName() : "Nothing";
	strcpy( strName, pStr );

	// truncate string if it's too long to fit on the screen
	nMaxWidth	= 165;
	nIndex		= strlen(strName) - 1;

	while( GR_GetStringWidth( strName, GR_FONT_SMALL ) > nMaxWidth ) {
		strName[ nIndex ] = 0;

		if( nIndex > 1 ) {
			strName[ nIndex - 1 ] = '.';
			strName[ nIndex - 2 ] = '.';
		}

		nIndex--;
	}

	GR_SetFontColor( RGBA(81, 112, 164, 64) );
	GR_DrawTextExt( 415, 275, strName, GR_FONT_SMALL );

	// draw play time
	nSecs = MP3_GetCurrentTime();

	nMins = nSecs / 60;
	nSecs = nSecs % 60;

	sprintf( strTime, "%02i:%02i", nMins, nSecs );

	GR_DrawTextExt( 215, 280, strTime, GR_FONT_BIG );

	GR_SetFontColor( RGB(255, 255, 255) );
	GR_DrawTextExt( 410, 260, "Currently Playing", GR_FONT_SMALL );
	GR_DrawTextExt( 410, 300, "Status", GR_FONT_SMALL );
	GR_DrawTextExt( 410, 355, "Volume", GR_FONT_SMALL );

	switch( MP3_GetStatus() )
	{
		case MP3_STOPPED:
			pStr = "Playback Stopped";
			break;

		case MP3_PLAYING:
			pStr = "Playback Started";
			break;

		case MP3_PAUSED:
			pStr = "Playback Paused";
			break;
	}

	GR_SetFontColor( RGBA(81, 112, 164, 64) );
	GR_DrawTextExt( 415, 315, pStr, GR_FONT_SMALL );

	pStr = MP3_GetLooping() ? "Repeat On" : "Repeat Off";
	GR_DrawTextExt( 415, 330, pStr, GR_FONT_SMALL );
}

//
// UI_MyMusicDrawPrev - Draws previous button
//

void UI_MyMusicDrawPrev( void )
{
	int old;
	point_t coords[3];
	menuButton_t *pBtn = &s_music.Prev;
	int pos = pBtn->generic.x;

	if( UI_GetSelectedItem( pBtn->generic.parent ) == pBtn )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(81, 112, 164) );

	GR_SetAlpha( 0.5f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	GR_DrawFillRect( pos, pBtn->generic.y, 3, 20 );
	pos += 5;

	coords[0].x = pos;
	coords[0].y = pBtn->generic.y + 20 / 2;
	coords[1].x = pos + 15;
	coords[1].y = pBtn->generic.y;
	coords[2].x = pos + 15;
	coords[2].y = pBtn->generic.y + 20;

	GR_DrawTriangle( coords[0], coords[1], coords[2] );
	GR_SetBlendMode( old );
}

//
// UI_MyMusicDrawPlay - Draws play button
//

void UI_MyMusicDrawPlay( void )
{
	int old;
	point_t coords[3];
	menuButton_t *pBtn = &s_music.Play;

	if( UI_GetSelectedItem( pBtn->generic.parent ) == pBtn )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(81, 112, 164) );

	GR_SetAlpha( 0.5f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	coords[0].x	= pBtn->generic.x;
	coords[0].y = pBtn->generic.y;
	coords[1].x = pBtn->generic.x + 20;
	coords[1].y = pBtn->generic.y + 20 / 2;
	coords[2].x = pBtn->generic.x;
	coords[2].y = pBtn->generic.y + 20;

	GR_DrawTriangle( coords[0], coords[1], coords[2] );
	GR_SetBlendMode( old );

}

//
// UI_MyMusicDrawPause - Draws pause button
//

void UI_MyMusicDrawPause( void )
{
	int old;
	menuButton_t *pBtn = &s_music.Pause;

	if( UI_GetSelectedItem( pBtn->generic.parent ) == pBtn )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(81, 112, 164) );

	GR_SetAlpha( 0.5f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	GR_DrawFillRect( pBtn->generic.x, pBtn->generic.y, 7, 20 );
	GR_DrawFillRect( pBtn->generic.x + 13, pBtn->generic.y, 7, 20 );

	GR_SetBlendMode( old );
}

//
// UI_MyMusicDrawStop - Draws stop button
//

void UI_MyMusicDrawStop( void )
{
	int old;
	menuButton_t *pBtn = &s_music.Stop;

	if( UI_GetSelectedItem( pBtn->generic.parent ) == pBtn )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(81, 112, 164) );

	GR_SetAlpha( 0.5f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	GR_DrawFillRect( pBtn->generic.x, pBtn->generic.y, 20, 20 );

	GR_SetBlendMode( old );
}

//
// UI_MyMusicDrawNext - Draws next button
//

void UI_MyMusicDrawNext( void )
{
	int old;
	point_t coords[3];
	menuButton_t *pBtn = &s_music.Next;
	int pos = pBtn->generic.x;

	if( UI_GetSelectedItem( pBtn->generic.parent ) == pBtn )
		GR_SetDrawColor( RGB(0, 255, 0) );
	else
		GR_SetDrawColor( RGB(81, 112, 164) );

	GR_SetAlpha( 0.5f );
	old = GR_SetBlendMode( GR_BLEND_CONSTANT );

	coords[0].x	= pos;
	coords[0].y = pBtn->generic.y;
	coords[1].x = pos + 15;
	coords[1].y = pBtn->generic.y + 20 / 2;
	coords[2].x = pos;
	coords[2].y = pBtn->generic.y + 20;

	GR_DrawTriangle( coords[0], coords[1], coords[2] );

	pos += 17;
	GR_DrawFillRect( pos, pBtn->generic.y, 3, 20 );
	GR_SetBlendMode( old );
}
