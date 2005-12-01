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
# File: UI Header File.
#
*/


#ifndef _UI_H
#define _UI_H

#define MYPS2_VERSION	"0.3" 

#include <gr.h>
#include <libpad.h>
#include <misc.h>
#include <file.h>
#include <imgscale.h>
#include <gamepad.h>
#include <sysconf.h>
#include <include/cdvd.h>

#include <libjpg/include/libjpg.h>

// rgb image data
extern u32 font_arial_w;
extern u32 font_arial_h;
extern u32 font_arial[];
extern u32 font_arial_psm;
extern u32 font_arial_tc[];

extern u32 img_aqua_w;
extern u32 img_aqua_h;
extern u32 img_aqua[];
extern u32 img_aqua_psm;

extern u32 img_folder_w;
extern u32 img_folder_h;
extern u32 img_folder[];
extern u32 img_folder_psm;

extern u32 img_error_w;
extern u32 img_error_h;
extern u32 img_error[];
extern u32 img_error_psm;

extern u32 img_logo_w;
extern u32 img_logo_h;
extern u32 img_logo[];
extern u32 img_logo_psm;

//
// ui_menu.c
//

#define MENU_MAX_ITEMS	64

typedef struct menuFramework_s menuFramework_t;

struct menuFramework_s {
	int	numItems;						// Number of items in menu
	void *items[MENU_MAX_ITEMS];		// Items
	int selectedItem;					// Item that currently has the focus

	int (*callback)( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
	void (*input)( u32 buttons );		// optional input handler

	menuFramework_t	*parent;			// valid for popup menus, otherwise NULL
};

typedef enum {
	MSG_CONTROL,
	MSG_DRAW,
	MSG_CLOSE,
	MSG_DIALOGEXIT
} callbackMsg_t;


enum {
	MENU_CREATEPART,

	MENU_MAIN,
	MENU_PICVIEW,
	MENU_MANAGER,
	MENU_ELFLOADER,
	MENU_OPTIONS,
	MENU_NETWORK,
	MENU_OPTIONS_EDIT,

	MENU_POPUP_THUMB,

	MENU_NUM_MENUS
};

typedef struct {
	IMG_HANDLE		hBackground;
	IMG_HANDLE		hFontmap;
	u32				*fontCoords;
	menuFramework_t	*active;

	menuFramework_t	menus[MENU_NUM_MENUS];
} uiStatic_t;

extern uiStatic_t uis;

//
// Menu Item Definitions
//

// this structure is shared by all menu items and holds
// common attributes.
typedef struct {
	int				type;
	int				x;
	int				y;
	int				flags;
	int				id;
	menuFramework_t *parent;
} menuCommon_t;

typedef struct {
	menuCommon_t	generic;
	const char		*text;
	int				size;
	int				color;
} menuText_t;

typedef struct {
	menuCommon_t	generic;
	IMG_HANDLE		hImage;
	int				width;
	int				height;
	int				frameColor;
} menuBitmap_t;

typedef struct {
	menuCommon_t	generic;
	int				direction;
	int				width;
	int				height;
	int				color;
} menuTriBtn_t;

typedef struct {
	menuCommon_t	generic;
	int				width;
	int				height;
	int				color;			// color of background rectangle

	char			*filter;		// only display files with matching extension

	fileInfo_t		*pEntries;
	int				numEntries;		// number of entries in dirview
	int				selEntry;		// selected entry
	char			absPath[256];	// absolute path

	int				visEntry;		// where to start drawing
	int				numDraw;		// how many items fit on the screen at a time 

} menuDirView_t;

typedef struct {
	menuCommon_t	generic;
	int				width;
	int				height;

	int				barColor;
	int				textColor;
	int				frameColor;

	u64				mins;
	u64				maxs;
	u64				pos;
} menuProgress_t;

typedef struct {
	menuCommon_t	generic;
	int				width;
	int				height;
	int				color;

	char			string[256];	// input string
	int				caretPos;		// current offset in string

	int				chrIndex;		// currently selected Character

} menuEditfield_t;

typedef struct {
	menuCommon_t	generic;
	int				width;
	int				height;

	int				barColor;

	s64				mins;
	s64				maxs;
	s64				pos;
	u64				stepSize;
} menuSlider_t;

typedef struct {
	menuCommon_t	generic;
	int				width;
	int				state;
	const char		*text;
	int				color;
} menuRadio_t;

typedef struct comboEntry_s comboEntry_t;

struct comboEntry_s {
	char			*string;

	comboEntry_t	*next;
	comboEntry_t	*prev;
};

typedef struct {
	menuCommon_t	generic;
	int				width;
	const char		*text;
	int				color;

	int				selected;
	int				numEntries;
	comboEntry_t	*pEntries;
} menuCombo_t;


#define UI_DIRVIEW_MARGIN	20		// 20 pixel margin
#define UI_EDIT_MARGIN		20

typedef enum {
	MENU_CONTROL_TEXT,			// Text Control
	MENU_CONTROL_BITMAP,		// Bitmap Control
	MENU_CONTROL_TRIBUTTON,		// Triangle Button Control
	MENU_CONTROL_DIRVIEW,		// Directory List View Control
	MENU_CONTROL_PROGRESS,		// Progress Bar Control
	MENU_CONTROL_EDITFIELD,		// Editfield with a keyboard mask
	MENU_CONTROL_SLIDER,		// A movable Slider Control
	MENU_CONTROL_RADIO,			// Radio Button Control
	MENU_CONTROL_COMBO			// Combo dropdown like Control

} menuContols_e;

#define CFL_FRAME			0x00000001
#define CFL_INACTIVE		0x00000002
#define CFL_CENTER_JUSTIFY	0x00000004
#define CFL_INVISIBLE		0x00000008
#define CFL_FORCECOLOR		0x00000010

// Slider Flags
//
#define CFL_SL_VERTICAL		0x00000020
#define CFL_SL_HORIZONTAL	0x00000040

// Editfield Flags
//
#define	CFL_EF_NOBKG		0x00000020		// No background

//
// Notification Messages
//

#define	NOT_CLICKED					0x00000001

// Directory View Notifications
//
#define NOT_DV_CHANGED_DIR			0x00000002
#define NOT_DV_CLICKED_ENTRY		0x00000004
#define NOT_DV_MARKED_ENTRY			0x00000008
#define NOT_DV_UNMARKED_ENTRY		0x00000010
#define NOT_DV_SEL_CHANGED			0x00000020

// Edit Field Notifications
//
#define NOT_EF_CLICKED_OK			0x00000002
#define NOT_EF_CLICKED_CANCEL		0x00000004

// Slider Notifications
//
#define NOT_SL_POS_CHANGED			0x00000002

// Radiobutton Notifications
//
#define NOT_RB_FLIPPED				0x00000002

// Combobox Notifications
//
#define NOT_CB_SEL_CHANGED			0x00000002

typedef enum {
	TRI_UP,
	TRI_RIGHT,
	TRI_DOWN,
	TRI_LEFT
} triBtnDir_e;

typedef enum {
	RB_STATE_DISABLED,
	RB_STATE_ENABLED
} rbState_e;

void UI_Thread( void );
void UI_Init( void );
int UI_AddItemToMenu( menuFramework_t *menu, void *item );
void UI_DelItemFromMenu( menuFramework_t *menu, void *item );
void *UI_GetSelectedItem( menuFramework_t *menu );
void UI_SelectItemById( menuFramework_t *menu, int id );
void UI_DrawTextControl( menuText_t *t );
void UI_DrawBitmapControl( menuBitmap_t *b );
void UI_DrawMenu( menuFramework_t *menu );
void UI_Refresh( void );
void UI_SetActiveMenu( int menuId );
void UI_PopupOpen( int popupId, menuFramework_t *parent );
void UI_PopupClose( menuFramework_t *popup );
void UI_GamepadInput( u32 buttons );
void UI_DefaultInputHandler( menuFramework_t *menu, u32 buttons );

void UI_InitDirViewControl( menuDirView_t *pDirView );
int UI_InputDirViewControl( menuDirView_t *pDirView, menuFramework_t *menu, u32 buttons );
void UI_DrawDirViewControl( menuDirView_t *pDirView );
void UI_DirView_AddFile( menuDirView_t *pDirView, const fileInfo_t *pInfo );
void UI_DirView_SetDir( menuDirView_t *pDirView, const char *pDir );
void UI_DirView_SetCursor( menuDirView_t *pDirView, int nCursor );
void UI_DirView_Clear( menuDirView_t *pDirView );
const char *UI_DirView_GetDir( const menuDirView_t *pDirView );
int UI_DirView_MarkedCount( const menuDirView_t *pDirView );
const fileInfo_t *UI_DirView_GetMarked( const menuDirView_t *pDirView, int n );
const fileInfo_t *UI_DirView_GetSelected( const menuDirView_t *pDirView );
void UI_DirView_Refresh( menuDirView_t *pDirView );

void UI_InitProgressControl( menuProgress_t *pProgress );
void UI_DrawProgressControl( menuProgress_t *pProgress );
void UI_Progress_SetBounds( menuProgress_t *pProgress, u64 mins, u64 maxs );
void UI_Progress_SetPosition( menuProgress_t *pProgress, u64 pos );
u64 UI_Progress_GetPosition( const menuProgress_t *pProgress );

void UI_InitEditControl( menuEditfield_t *pEditfield );
int UI_InputEditControl( menuEditfield_t *pEditfield, menuFramework_t *menu, u32 buttons );
void UI_DrawEditControl( menuEditfield_t *pEditfield );
void UI_Editfield_AddCharAt( menuEditfield_t *pEditfield, char c, int pos );
void UI_Editfield_DelCharAt( menuEditfield_t *pEditfield, int pos );
void UI_Editfield_SetString( menuEditfield_t *pEditfield, const char *pStr );
const char *UI_Editfield_GetString( const menuEditfield_t *pEditfield );
void UI_Editfield_Clear( menuEditfield_t *pEditfield );

void UI_InitSliderControl( menuSlider_t *pSlider );
int UI_InputSliderControl( menuSlider_t *pSlider, menuFramework_t *menu, u32 buttons );
void UI_DrawSliderControl( menuSlider_t *pSlider );
void UI_Slider_SetBounds( menuSlider_t *pSlider, s64 mins, s64 maxs );
void UI_Slider_SetPos( menuSlider_t *pSlider, s64 pos );
s64 UI_Slider_GetPos( const menuSlider_t *pSlider );
void UI_Slider_SetStepSize( menuSlider_t *pSlider, u64 stepSize );
u64 UI_Slider_GetStepSize( const menuSlider_t *pSlider );

void UI_InitRadioControl( menuRadio_t *pRadio );
void UI_DrawRadioControl( menuRadio_t *pRadio );
int UI_InputRadioControl( menuRadio_t *pRadio, menuFramework_t *menu, u32 buttons );
void UI_Radio_SetState( menuRadio_t *pRadio, int state );
int UI_Radio_GetState( const menuRadio_t *pRadio );

void UI_InitComboControl( menuCombo_t *pCombo );
void UI_DrawComboControl( menuCombo_t *pCombo );
int UI_InputComboControl( menuCombo_t *pCombo, menuFramework_t *menu, u32 buttons );
void UI_Combo_AddString( menuCombo_t *pCombo, const char *pStr );
void UI_Combo_Clean( menuCombo_t *pCombo );
const char *UI_Combo_GetSelected( const menuCombo_t *pCombo );
int UI_Combo_Select( menuCombo_t *pCombo, int nIndex );
int UI_Combo_SelectByName( menuCombo_t *pCombo, const char *pName );

//
// ui_main.c
//

void UI_InitMainMenu( void );
int UI_MainCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_MainDraw( void );

//
// ui_picview.c
//

typedef struct {
	u8		magic[2];		// should be 'TC'
	u32		size;			// stores the size of the original image thumbnail was
							// generated from.
	u8		width;			// width of thumbnail
	u8		height;			// height of thumbnail
} TBN_HEADER;

void UI_InitPicViewMenu( void );
int UI_PicViewCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_PicViewDraw( void );
void UI_UpdateImageGrid( void );
void UI_PicViewSetDir( const char *path );
void UI_PicViewPrevPage( void );
void UI_PicViewNextPage( void );
int UI_SortByName( const void *a, const void *b );
int UI_SortBySize( const void *a, const void *b );
int UI_SortByType( const void *a, const void *b );
int UI_CreateThumbnail( const fileInfo_t *pFile, const char *pCurrentDir, IMG_HANDLE *pHandle );
int UI_TbnCacheLoad( const fileInfo_t *pFile, const char *pCurrentDir, IMG_HANDLE *pHandle );
int UI_TbnCacheSave( const fileInfo_t *pFile, const char *pCurrentDir, int nWidth, int nHeight, void *pRGBData );
void UI_TbnPrecacheDir( void );

void UI_InitPopupThumb( void );
int UI_PopupThumbCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );

void UI_InitDetailView( const fileInfo_t *pImage, const char *pCurrentDir, IMG_HANDLE *pHandle );
void UI_DrawDetailView( void );
void UI_DetailViewInput( u32 buttons );

//
// ui_manager.c
//

void UI_InitManagerMenu( void );
int UI_ManagerCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_ManagerDraw( void );
void UI_ManagerStartCopying( void );
void UI_ManagerCopyDir( const char *path_in, const char *path_out );
void UI_ManagerCopyFile( const char *file_in, const char *file_out );
void UI_ManagerDeleteFiles( void );

//
// ui_elfloader.c
//

typedef struct {
	char	filePath[256];
	char	fileDesc[128];
} descEntry_t;

#define MAX_DESC_ENTRIES 32

void UI_InitElfLoaderMenu( void );
int UI_ElfLoaderCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_ElfLoaderDraw( void );
int UI_LoadDescriptions( descEntry_t *descTable, int maxItems );
const descEntry_t *UI_GetDescByName( const char *name );
int UI_AddDescription( const char *strFile, const char *strDesc );

//
// ui_install.c
//

void UI_InitCreatePart( void );
int UI_CreatePartCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_CreatePartDraw( void );

//
// ui_options.c
//

void UI_InitOptionsMenu( void );
int UI_OptionsCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_OptionsDraw( void );
void UI_OptionsLoad( void );
void UI_InitNetworkMenu( void );
int UI_NetworkCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_NetworkDraw( void );
void UI_OpenEditMenu( const char *pHeadline, const char *pEditValue, int GoBackId );
void UI_InitEditMenu( void );
int UI_EditCallback( menuFramework_t *pMenu, int nMsg, unsigned int fParam, unsigned long sParam );
void UI_EditDraw( void );

#endif
