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
#include <libhdd.h>

#define ID_SAVE			1
#define	ID_LOAD			2
#define ID_NETWORK		3
#define ID_POWEROFF		14
#define ID_GOBACK		4

#define ID_SCR_MODE		5
#define ID_SCR_HOR		6
#define ID_SCR_VER		7

#define ID_TBN_CACHING	8
#define ID_TBN_PATH		9

#define ID_USBD_CUSTOM	10
#define ID_USBD_PATH	11

#define ID_LOG_ENABLE	12
#define ID_LOG_PATH		13

typedef struct {
	menuFramework_t	*menu;

	menuText_t		Save;
	menuText_t		Load;
	menuText_t		Network;
	menuText_t		PowerOff;
	menuText_t		GoBack;

	menuCombo_t		scrMode;
	menuSlider_t	scrHor;
	menuSlider_t	scrVer;

	menuRadio_t		tbnCaching;
	menuText_t		tbnPath;

	menuRadio_t		usbdCustom;
	menuText_t		usbdPath;

	menuRadio_t		logEnable;
	menuText_t		logPath;

} uiOptionsMenu_t;

static uiOptionsMenu_t	s_options;

//
// UI_InitOptionsMenu - Initializes options menu controls
//

void UI_InitOptionsMenu( void )
{
	int nValue;

	s_options.menu						= &uis.menus[ MENU_OPTIONS ];

	s_options.menu->callback			= UI_OptionsCallback;
	s_options.menu->input				= NULL;
	s_options.menu->numItems			= 0;
	s_options.menu->selectedItem		= 0;

	s_options.Save.generic.type			= MENU_CONTROL_TEXT;
	s_options.Save.generic.flags		= MC_Available(0) ? 0 : CFL_INACTIVE;
	s_options.Save.generic.x			= 35;
	s_options.Save.generic.y			= 70;
	s_options.Save.generic.id			= ID_SAVE;
	s_options.Save.text					= "Save Config";
	s_options.Save.color				= RGB(255, 255, 255);
	s_options.Save.size					= GR_FONT_SMALL;

	s_options.Load.generic.type			= MENU_CONTROL_TEXT;
	s_options.Load.generic.flags		= MC_Available(0) ? 0 : CFL_INACTIVE;
	s_options.Load.generic.x			= 35;
	s_options.Load.generic.y			= 90;
	s_options.Load.generic.id			= ID_LOAD;
	s_options.Load.text					= "Load Config";
	s_options.Load.color				= RGB(255, 255, 255);
	s_options.Load.size					= GR_FONT_SMALL;

	s_options.Network.generic.type		= MENU_CONTROL_TEXT;
	s_options.Network.generic.flags		= 0;
	s_options.Network.generic.x			= 35;
	s_options.Network.generic.y			= 110;
	s_options.Network.generic.id		= ID_NETWORK;
	s_options.Network.text				= "Network";
	s_options.Network.color				= RGB(255, 255, 255);
	s_options.Network.size				= GR_FONT_SMALL;

	s_options.PowerOff.generic.type		= MENU_CONTROL_TEXT;
	s_options.PowerOff.generic.flags	= 0;
	s_options.PowerOff.generic.x		= 35;
	s_options.PowerOff.generic.y		= 130;
	s_options.PowerOff.generic.id		= ID_POWEROFF;
	s_options.PowerOff.text				= "PowerOff PS2";
	s_options.PowerOff.color			= RGB(255, 255, 255);
	s_options.PowerOff.size				= GR_FONT_SMALL;

	s_options.GoBack.generic.type		= MENU_CONTROL_TEXT;
	s_options.GoBack.generic.flags		= 0;
	s_options.GoBack.generic.x			= 35;
	s_options.GoBack.generic.y			= 170;
	s_options.GoBack.generic.id			= ID_GOBACK;
	s_options.GoBack.text				= "Go Back";
	s_options.GoBack.color				= RGB(255, 255, 255);
	s_options.GoBack.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_options.menu, &s_options.Save );
	UI_AddItemToMenu( s_options.menu, &s_options.Load );
	UI_AddItemToMenu( s_options.menu, &s_options.Network );
	UI_AddItemToMenu( s_options.menu, &s_options.PowerOff );
	UI_AddItemToMenu( s_options.menu, &s_options.GoBack );

	// General Settings
	s_options.scrMode.generic.type		= MENU_CONTROL_COMBO;
	s_options.scrMode.generic.flags		= 0;
	s_options.scrMode.generic.x			= 215;
	s_options.scrMode.generic.y			= 85;
	s_options.scrMode.generic.id		= ID_SCR_MODE;
	s_options.scrMode.width				= 325;
	s_options.scrMode.text				= "Screen Graphics Mode";
	s_options.scrMode.color				= RGB(255, 255, 255);

	UI_AddItemToMenu( s_options.menu, &s_options.scrMode );

	UI_Combo_AddString( &s_options.scrMode, "<AUTO>" );
	UI_Combo_AddString( &s_options.scrMode, "<NTSC>" );
	UI_Combo_AddString( &s_options.scrMode, "<PAL>"  );

	nValue = SC_GetValueForKey_Int( "scr_mode", NULL );

	if( nValue == GR_MODE_AUTO )		UI_Combo_Select( &s_options.scrMode, 0 );
	else if( nValue == GR_MODE_NTSC )	UI_Combo_Select( &s_options.scrMode, 1 );
	else if( nValue == GR_MODE_PAL )	UI_Combo_Select( &s_options.scrMode, 2 );

	s_options.scrHor.generic.type		= MENU_CONTROL_SLIDER;
	s_options.scrHor.generic.flags		= CFL_SL_HORIZONTAL;
	s_options.scrHor.generic.x			= 490;
	s_options.scrHor.generic.y			= 105;
	s_options.scrHor.generic.id			= ID_SCR_HOR;
	s_options.scrHor.width				= 100;
	s_options.scrHor.height				= 10;
	s_options.scrHor.barColor			= RGBA( 82, 117, 168, 40 );
	
	UI_AddItemToMenu( s_options.menu, &s_options.scrHor );

	UI_Slider_SetBounds( &s_options.scrHor, -250, 250 );
	UI_Slider_SetPos( &s_options.scrHor, SC_GetValueForKey_Int( "scr_adjust_x", NULL ) );
	UI_Slider_SetStepSize( &s_options.scrHor, 3 );

	s_options.scrVer.generic.type		= MENU_CONTROL_SLIDER;
	s_options.scrVer.generic.flags		= CFL_SL_HORIZONTAL;
	s_options.scrVer.generic.x			= 490;
	s_options.scrVer.generic.y			= 125;
	s_options.scrVer.generic.id			= ID_SCR_VER;
	s_options.scrVer.width				= 100;
	s_options.scrVer.height				= 10;
	s_options.scrVer.barColor			= RGBA( 82, 117, 168, 40 );
	
	UI_AddItemToMenu( s_options.menu, &s_options.scrVer );

	UI_Slider_SetBounds( &s_options.scrVer, -250, 250 );
	UI_Slider_SetPos( &s_options.scrVer, SC_GetValueForKey_Int( "scr_adjust_y", NULL ) );
	UI_Slider_SetStepSize( &s_options.scrVer, 3 );

	s_options.tbnCaching.generic.type	= MENU_CONTROL_RADIO;
	s_options.tbnCaching.generic.flags	= 0;
	s_options.tbnCaching.generic.x		= 215;
	s_options.tbnCaching.generic.y		= 160;
	s_options.tbnCaching.generic.id		= ID_TBN_CACHING;
	s_options.tbnCaching.width			= 322;
	s_options.tbnCaching.text			= "Thumbnail HDD Caching";
	s_options.tbnCaching.color			= RGB(255, 255, 255);

	UI_Radio_SetState( &s_options.tbnCaching, SC_GetValueForKey_Int( "tbn_caching", NULL ) );
	UI_AddItemToMenu( s_options.menu, &s_options.tbnCaching );

	s_options.tbnPath.generic.type		= MENU_CONTROL_TEXT;
	s_options.tbnPath.generic.flags		= 0;
	s_options.tbnPath.generic.x			= 215;
	s_options.tbnPath.generic.y			= 180;
	s_options.tbnPath.generic.id		= ID_TBN_PATH;
	s_options.tbnPath.text				= "Thumbnail Cache Path";
	s_options.tbnPath.color				= RGB(255, 255, 255);
	s_options.tbnPath.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_options.menu, &s_options.tbnPath );

	s_options.usbdCustom.generic.type	= MENU_CONTROL_RADIO;
	s_options.usbdCustom.generic.flags	= 0;
	s_options.usbdCustom.generic.x		= 215;
	s_options.usbdCustom.generic.y		= 215;
	s_options.usbdCustom.generic.id		= ID_USBD_CUSTOM;
	s_options.usbdCustom.width			= 322;
	s_options.usbdCustom.text			= "Custom USBD Module";
	s_options.usbdCustom.color			= RGB(255, 255, 255);

	UI_Radio_SetState( &s_options.usbdCustom, SC_GetValueForKey_Int( "usbd_irx_custom", NULL ) );
	UI_AddItemToMenu( s_options.menu, &s_options.usbdCustom );

	s_options.usbdPath.generic.type		= MENU_CONTROL_TEXT;
	s_options.usbdPath.generic.flags	= 0;
	s_options.usbdPath.generic.x		= 215;
	s_options.usbdPath.generic.y		= 235;
	s_options.usbdPath.generic.id		= ID_USBD_PATH;
	s_options.usbdPath.text				= "USBD Module Path";
	s_options.usbdPath.color			= RGB(255, 255, 255);
	s_options.usbdPath.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_options.menu, &s_options.usbdPath );

	s_options.logEnable.generic.type	= MENU_CONTROL_RADIO;
	s_options.logEnable.generic.flags	= 0;
	s_options.logEnable.generic.x		= 215;
	s_options.logEnable.generic.y		= 270;
	s_options.logEnable.generic.id		= ID_LOG_ENABLE;
	s_options.logEnable.width			= 322;
	s_options.logEnable.text			= "Enable Logging";
	s_options.logEnable.color			= RGB(255, 255, 255);

	UI_Radio_SetState( &s_options.logEnable, SC_GetValueForKey_Int( "log_enable", NULL ) );
	UI_AddItemToMenu( s_options.menu, &s_options.logEnable );

	s_options.logPath.generic.type		= MENU_CONTROL_TEXT;
	s_options.logPath.generic.flags		= 0;
	s_options.logPath.generic.x			= 215;
	s_options.logPath.generic.y			= 290;
	s_options.logPath.generic.id		= ID_LOG_PATH;
	s_options.logPath.text				= "Log File Path";
	s_options.logPath.color				= RGB(255, 255, 255);
	s_options.logPath.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_options.menu, &s_options.logPath );
}


//
// UI_OptionsCallback
//

int UI_OptionsCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	const char	*pStr;
	int			nValue;

	switch( nMsg )
	{
		case MSG_DRAW:
			UI_OptionsDraw();
			return 1;

		case MSG_CLOSE:
			UI_Combo_Clean( &s_options.scrMode );
			return 1;

		case MSG_CONTROL:
			switch( sParam )
			{

				case ID_SAVE:
					SC_SaveConfig();
					return 1;
					
				case ID_LOAD:
					UI_OptionsLoad();
					return 1;

				case ID_NETWORK:
					UI_SetActiveMenu(MENU_NETWORK);
					return 1;

				case ID_POWEROFF:
					hddPowerOff();
					return 1;

				case ID_GOBACK:
					UI_SetActiveMenu(MENU_MAIN);
					return 1;

				case ID_SCR_MODE:
					pStr = UI_Combo_GetSelected( &s_options.scrMode );
					
					if( !strcmp( pStr, "<AUTO>" ) )
						SC_SetValueForKey_Int( "scr_mode", GR_MODE_AUTO );
					else if( !strcmp( pStr, "<NTSC>" ) )
						SC_SetValueForKey_Int( "scr_mode", GR_MODE_NTSC );
					else if( !strcmp( pStr, "<PAL>" ) )
						SC_SetValueForKey_Int( "scr_mode", GR_MODE_PAL );

					return 1;

				case ID_SCR_HOR:
					nValue = UI_Slider_GetPos( &s_options.scrHor );
					SC_SetValueForKey_Int( "scr_adjust_x", nValue );

					GR_SetScreenAdjust( SC_GetValueForKey_Int( "scr_adjust_x", NULL ),
										SC_GetValueForKey_Int( "scr_adjust_y", NULL ) );
					return 1;

				case ID_SCR_VER:
					nValue = UI_Slider_GetPos( &s_options.scrVer );
					SC_SetValueForKey_Int( "scr_adjust_y", nValue );

					GR_SetScreenAdjust( SC_GetValueForKey_Int( "scr_adjust_x", NULL ),
										SC_GetValueForKey_Int( "scr_adjust_y", NULL ) );
					return 1;

				case ID_TBN_CACHING:
					nValue = UI_Radio_GetState( &s_options.tbnCaching );
					SC_SetValueForKey_Int( "tbn_caching", nValue );
					return 1;

				case ID_TBN_PATH:
					UI_OpenEditMenu( "Enter new Thumbnail Cache Path", "tbn_cache_path", MENU_OPTIONS );
					return 1;

				case ID_USBD_CUSTOM:
					nValue = UI_Radio_GetState( &s_options.usbdCustom );
					SC_SetValueForKey_Int( "usbd_irx_custom", nValue );
					return 1;

				case ID_USBD_PATH:
					UI_OpenEditMenu( "Enter USBD.IRX Path", "usbd_irx_path", MENU_OPTIONS );
					return 1;

				case ID_LOG_ENABLE:
					nValue = UI_Radio_GetState( &s_options.logEnable );
					SC_SetValueForKey_Int( "log_enable", nValue );
					return 1;

				case ID_LOG_PATH:
					UI_OpenEditMenu( "Enter Log File Path", "log_path", MENU_OPTIONS );
					return 1;

			}
			return 1;
	}

	return 0;
}

//
// UI_OptionsDraw
//

void UI_OptionsDraw( void )
{
	// draw background
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_DrawRoundRect( 175, 25, 450, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	UI_DrawMenu( s_options.menu );

	GR_SetFontColor( RGB(0, 255, 0) );
	GR_DrawTextExt( 200, 50, "General Settings", GR_FONT_SMALL );

	GR_SetFontColor( RGB(255, 255, 255) );
	GR_DrawTextExt( 215, 105, "Horizonal Offset", GR_FONT_SMALL );
	GR_DrawTextExt( 215, 125, "Vertical Offset", GR_FONT_SMALL );

}

//
// UI_OptionsLoad - Restores settings from MC
//

void UI_OptionsLoad( void )
{
	// clean up current settings
	SC_Clean();

	// reload settings from MC
	SC_LoadConfig();

	// this will re-init all controls
	UI_InitOptionsMenu();

	// reset screen adjustment
	GR_SetScreenAdjust( SC_GetValueForKey_Int( "scr_adjust_x", NULL ),
						SC_GetValueForKey_Int( "scr_adjust_y", NULL ) );
	
	// refresh the screen
	UI_Refresh();
}

//////////////////////////////////////////////////////////////////////////7
// NETWORK OPTIONS MENU

#define ID_NET_GOBACK		1
#define ID_NET_ENABLE		2
#define ID_NET_IP			3
#define ID_NET_MASK			4
#define ID_NET_GW			5
#define ID_NET_FTP_ENABLE	6
#define ID_NET_FTP_PORT		7
#define ID_NET_FTP_ANON		8
#define ID_NET_FTP_LOGIN	9
#define ID_NET_FTP_PASSW	10

typedef struct {
	menuFramework_t *menu;

	menuText_t		GoBack;

	menuRadio_t		netEnable;
	menuText_t		net_ip;
	menuText_t		net_mask;
	menuText_t		net_gw;

	menuRadio_t		ftpEnable;
	menuText_t		ftp_port;
	menuRadio_t		ftp_anon;
	menuText_t		ftp_login;
	menuText_t		ftp_passw;

} uiNetworkMenu_t;

static uiNetworkMenu_t s_network;

//
// UI_InitNetworkMenu - Initializes network options menu
//

void UI_InitNetworkMenu( void )
{
	s_network.menu						= &uis.menus[ MENU_NETWORK ];

	s_network.menu->callback			= UI_NetworkCallback;
	s_network.menu->input				= NULL;
	s_network.menu->numItems			= 0;
	s_network.menu->selectedItem		= 0;

	s_network.GoBack.generic.type		= MENU_CONTROL_TEXT;
	s_network.GoBack.generic.flags		= 0;
	s_network.GoBack.generic.x			= 35;
	s_network.GoBack.generic.y			= 70;
	s_network.GoBack.generic.id			= ID_NET_GOBACK;
	s_network.GoBack.text				= "Go Back";
	s_network.GoBack.color				= RGB(255, 255, 255);
	s_network.GoBack.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.GoBack );

	s_network.netEnable.generic.type	= MENU_CONTROL_RADIO;
	s_network.netEnable.generic.flags	= 0;
	s_network.netEnable.generic.x		= 215;
	s_network.netEnable.generic.y		= 85;
	s_network.netEnable.generic.id		= ID_NET_ENABLE;
	s_network.netEnable.width			= 322;
	s_network.netEnable.text			= "Enable Network";
	s_network.netEnable.color			= RGB(255, 255, 255);

	UI_Radio_SetState( &s_network.netEnable, SC_GetValueForKey_Int( "net_enable", NULL ) );
	UI_AddItemToMenu( s_network.menu, &s_network.netEnable );

	s_network.net_ip.generic.type		= MENU_CONTROL_TEXT;
	s_network.net_ip.generic.flags		= 0;
	s_network.net_ip.generic.x			= 225;
	s_network.net_ip.generic.y			= 105;
	s_network.net_ip.generic.id			= ID_NET_IP;
	s_network.net_ip.text				= "IP Address";
	s_network.net_ip.color				= RGB(255, 255, 255);
	s_network.net_ip.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.net_ip );

	s_network.net_mask.generic.type		= MENU_CONTROL_TEXT;
	s_network.net_mask.generic.flags	= 0;
	s_network.net_mask.generic.x		= 225;
	s_network.net_mask.generic.y		= 125;
	s_network.net_mask.generic.id		= ID_NET_MASK;
	s_network.net_mask.text				= "Subnet Mask";
	s_network.net_mask.color			= RGB(255, 255, 255);
	s_network.net_mask.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.net_mask );

	s_network.net_gw.generic.type		= MENU_CONTROL_TEXT;
	s_network.net_gw.generic.flags		= 0;
	s_network.net_gw.generic.x			= 225;
	s_network.net_gw.generic.y			= 145;
	s_network.net_gw.generic.id			= ID_NET_GW;
	s_network.net_gw.text				= "Gateway IP";
	s_network.net_gw.color				= RGB(255, 255, 255);
	s_network.net_gw.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.net_gw );

	s_network.ftpEnable.generic.type	= MENU_CONTROL_RADIO;
	s_network.ftpEnable.generic.flags	= 0;
	s_network.ftpEnable.generic.x		= 215;
	s_network.ftpEnable.generic.y		= 180;
	s_network.ftpEnable.generic.id		= ID_NET_FTP_ENABLE;
	s_network.ftpEnable.width			= 322;
	s_network.ftpEnable.text			= "Enable FTP Server";
	s_network.ftpEnable.color			= RGB(255, 255, 255);

	UI_Radio_SetState( &s_network.ftpEnable, SC_GetValueForKey_Int( "ftp_daemon", NULL ) );
	UI_AddItemToMenu( s_network.menu, &s_network.ftpEnable );

	s_network.ftp_port.generic.type		= MENU_CONTROL_TEXT;
	s_network.ftp_port.generic.flags	= 0;
	s_network.ftp_port.generic.x		= 225;
	s_network.ftp_port.generic.y		= 200;
	s_network.ftp_port.generic.id		= ID_NET_FTP_PORT;
	s_network.ftp_port.text				= "FTP Port";
	s_network.ftp_port.color			= RGB(255, 255, 255);
	s_network.ftp_port.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.ftp_port );

	s_network.ftp_anon.generic.type		= MENU_CONTROL_RADIO;
	s_network.ftp_anon.generic.flags	= 0;
	s_network.ftp_anon.generic.x		= 225;
	s_network.ftp_anon.generic.y		= 220;
	s_network.ftp_anon.generic.id		= ID_NET_FTP_ANON;
	s_network.ftp_anon.width			= 312;
	s_network.ftp_anon.text				= "Anonymous Login";
	s_network.ftp_anon.color			= RGB(255, 255, 255);

	UI_Radio_SetState( &s_network.ftp_anon, SC_GetValueForKey_Int( "ftp_anonymous", NULL ) );
	UI_AddItemToMenu( s_network.menu, &s_network.ftp_anon );

	s_network.ftp_login.generic.type	= MENU_CONTROL_TEXT;
	s_network.ftp_login.generic.flags	= 0;
	s_network.ftp_login.generic.x		= 225;
	s_network.ftp_login.generic.y		= 240;
	s_network.ftp_login.generic.id		= ID_NET_FTP_LOGIN;
	s_network.ftp_login.text			= "Login Name";
	s_network.ftp_login.color			= RGB(255, 255, 255);
	s_network.ftp_login.size			= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.ftp_login );

	s_network.ftp_passw.generic.type	= MENU_CONTROL_TEXT;
	s_network.ftp_passw.generic.flags	= 0;
	s_network.ftp_passw.generic.x		= 225;
	s_network.ftp_passw.generic.y		= 260;
	s_network.ftp_passw.generic.id		= ID_NET_FTP_PASSW;
	s_network.ftp_passw.text			= "Login Password";
	s_network.ftp_passw.color			= RGB(255, 255, 255);
	s_network.ftp_passw.size			= GR_FONT_SMALL;

	UI_AddItemToMenu( s_network.menu, &s_network.ftp_passw );

}

//
// UI_NetworkCallback
//

int UI_NetworkCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	int nValue;

	switch( nMsg )
	{
		case MSG_DRAW:
			UI_NetworkDraw();
			return 1;

		case MSG_CONTROL:
			switch( sParam )
			{
				case ID_NET_GOBACK:
					UI_SetActiveMenu(MENU_OPTIONS);
					return 1;

				case ID_NET_ENABLE:
					nValue = UI_Radio_GetState( &s_network.netEnable );
					SC_SetValueForKey_Int( "net_enable", nValue );
					return 1;

				case ID_NET_IP:
					UI_OpenEditMenu( "Enter IP Address", "net_ip", MENU_NETWORK );
					return 1;

				case ID_NET_MASK:
					UI_OpenEditMenu( "Enter Subnet Mask", "net_netmask", MENU_NETWORK );
					return 1;

				case ID_NET_GW:
					UI_OpenEditMenu( "Enter Gateway IP", "net_gateway", MENU_NETWORK );
					return 1;

				case ID_NET_FTP_ENABLE:
					nValue = UI_Radio_GetState( &s_network.ftpEnable );
					SC_SetValueForKey_Int( "ftp_daemon", nValue );
					return 1;

				case ID_NET_FTP_PORT:
					UI_OpenEditMenu( "Enter FTP Port", "ftp_port", MENU_NETWORK );
					return 1;

				case ID_NET_FTP_ANON:
					nValue = UI_Radio_GetState( &s_network.ftp_anon );
					SC_SetValueForKey_Int( "ftp_anonymous", nValue );
					return 1;

				case ID_NET_FTP_LOGIN:
					UI_OpenEditMenu( "Enter FTP Login Name", "ftp_login", MENU_NETWORK );
					return 1;

				case ID_NET_FTP_PASSW:
					UI_OpenEditMenu( "Enter FTP Login Password", "ftp_password", MENU_NETWORK );
					return 1;

			}
			return 1;
	}
	return 0;
}

//
// UI_NetworkDraw
//

void UI_NetworkDraw( void )
{
	// draw background
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_DrawRoundRect( 175, 25, 450, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	UI_DrawMenu( s_network.menu );

	GR_SetFontColor( RGB(0, 255, 0) );
	GR_DrawTextExt( 200, 50, "Network Configuration", GR_FONT_SMALL );

	GR_SetFontColor( RGB(255, 255, 255) );
	GR_DrawTextExt( 350, 105, SC_GetValueForKey_Str( "net_ip", NULL ), GR_FONT_SMALL );
	GR_DrawTextExt( 350, 125, SC_GetValueForKey_Str( "net_netmask", NULL ), GR_FONT_SMALL );
	GR_DrawTextExt( 350, 145, SC_GetValueForKey_Str( "net_gateway", NULL ), GR_FONT_SMALL );

	GR_DrawTextExt( 350, 200, SC_GetValueForKey_Str( "ftp_port", NULL ), GR_FONT_SMALL );

}

///////////////////////////////////////////////////////////////////////////
// EDIT MENU

#define ID_EDIT_GOBACK		1
#define ID_EDIT_EDITFIELD	2

typedef struct {
	menuFramework_t *menu;

	char			strEdit[256];
	char			strHeadline[256];
	int				nGoBackId;

	menuText_t		GoBack;
	menuEditfield_t	EditField;;
} uiEditMenu_t;

static uiEditMenu_t s_edit;

//
// UI_OpenEditMenu - This is called from the Options and Network Options
//					 menus.
//

void UI_OpenEditMenu( const char *pHeadline, const char *pEditValue, int GoBackId )
{
	if( !pHeadline || !pEditValue )
		return;

	strcpy( s_edit.strHeadline, pHeadline );
	strcpy( s_edit.strEdit, pEditValue );

	s_edit.nGoBackId = GoBackId;

	UI_SetActiveMenu( MENU_OPTIONS_EDIT );
}

//
// UI_InitEditMenu - Initializes options edit menu
//

void UI_InitEditMenu( void )
{
	const char *pStr;

	s_edit.menu						= &uis.menus[ MENU_OPTIONS_EDIT ];

	s_edit.menu->callback			= UI_EditCallback;
	s_edit.menu->input				= NULL;
	s_edit.menu->numItems			= 0;
	s_edit.menu->selectedItem		= 1; // select editfield control

	s_edit.GoBack.generic.type		= MENU_CONTROL_TEXT;
	s_edit.GoBack.generic.flags		= 0;
	s_edit.GoBack.generic.x			= 35;
	s_edit.GoBack.generic.y			= 70;
	s_edit.GoBack.generic.id		= ID_EDIT_GOBACK;
	s_edit.GoBack.text				= "Go Back";
	s_edit.GoBack.color				= RGB(255, 255, 255);
	s_edit.GoBack.size				= GR_FONT_SMALL;

	UI_AddItemToMenu( s_edit.menu, &s_edit.GoBack );

	s_edit.EditField.generic.type	= MENU_CONTROL_EDITFIELD;
	s_edit.EditField.generic.flags	= CFL_EF_NOBKG;
	s_edit.EditField.generic.x		= 185;
	s_edit.EditField.generic.y		= 80;
	s_edit.EditField.generic.id		= ID_EDIT_EDITFIELD;
	s_edit.EditField.width			= 400;
	s_edit.EditField.height			= 200;

	UI_AddItemToMenu( s_edit.menu, &s_edit.EditField );

	pStr = SC_GetValueForKey_Str( s_edit.strEdit, NULL );

	if( pStr )
		UI_Editfield_SetString( &s_edit.EditField, pStr );
}

//
// UI_EditCallback
//

int UI_EditCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam )
{
	const char *pStr;

	switch( nMsg )
	{
		case MSG_DRAW:
			UI_EditDraw();
			return 1;

		case MSG_CONTROL:
			switch( sParam )
			{
				case ID_EDIT_GOBACK:
					UI_SetActiveMenu( s_edit.nGoBackId );
					break;

				case ID_EDIT_EDITFIELD:
					switch( fParam )
					{
						case NOT_EF_CLICKED_CANCEL:
							UI_SetActiveMenu( s_edit.nGoBackId );
							return 1;

						case NOT_EF_CLICKED_OK:
							pStr = UI_Editfield_GetString( &s_edit.EditField );

							if( pStr )
								SC_SetValueForKey_Str( s_edit.strEdit, pStr );

							UI_SetActiveMenu( s_edit.nGoBackId );
							return 1;
					}
					return 1;
			}
			return 1;
	}

	return 0;
}

//
// UI_EditDraw
//

void UI_EditDraw( void )
{
	// draw background
	GR_SetDrawColor( RGB(81, 112, 164) );
	GR_SetAlpha( 0.25f );
	GR_SetBlendMode( GR_BLEND_CONSTANT );
	GR_DrawRoundRect( 25, 25, 128, 384 );
	GR_DrawRoundRect( 175, 25, 450, 384 );
	GR_SetBlendMode( GR_BLEND_NONE );

	UI_DrawMenu( s_edit.menu );

	GR_SetFontColor( RGB(0, 255, 0) );
	GR_DrawTextExt( 200, 50, s_edit.strHeadline, GR_FONT_SMALL );
}
