/*
#     ___  _ _      ___
#    |    | | |    |
# ___|    |   | ___|    PS2DEV Open Source Project.
#----------------------------------------------------------
# (c) 2005 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/
#include "GUI.h"
#include "GUI_Data.h"

#include "Timer.h"
#include "CDDA.h"

#include <kernel.h>
#include <string.h>
#include <malloc.h>
#include <libpad.h>
#include <libhdd.h>
#include <loadfile.h>
#include <stdio.h>

#define STR_AVAILABLE_MEDIA "Available media: "

extern void* _gp;

static GUIContext                 s_GUICtx;
static unsigned char              s_PadBuf[   256 ] __attribute__(   (  aligned( 64 )  )   );
static unsigned char              s_Stack [ 32768 ] __attribute__(   (  aligned( 16 )  )   );
static int                        s_GUIThreadID;
static int                        s_MainThreadID;
static int                        s_Stop;
static int                        s_CDROM;
static int                        s_NoCDCheck;
static int                        s_PadSema;
static volatile unsigned long int s_Event;
static unsigned int               s_PrevBtn;
static unsigned int               s_Init;
static struct padButtonStatus     s_PadData;

static void ( *DoTimer ) ( void );

static unsigned char* const s_FSIcons[ 3 ] = {
 g_ImgFile, g_ImgFolder, g_ImgPart
};

extern void GUIStub_DrawBackground ( GSContext* );

static void GUI_Status ( char* apSts ) {

 int lLen      = strlen ( apSts );
 int lWidth    = s_GUICtx.m_pGSCtx -> TextWidth ( apSts, lLen );
 int lY        = s_GUICtx.m_pGSCtx -> m_Height - 30;
 int lCY       = lY;
 int lCH       = 26;
 int lScrWidth = s_GUICtx.m_pGSCtx -> m_Width - 24;

 lCY = lY  / 2;
 lCH = lCH / 2;

 while ( lWidth >= lScrWidth ) {

  --lLen;
  lWidth = s_GUICtx.m_pGSCtx -> TextWidth ( apSts, lLen );

 }  /* end while */

 s_GUICtx.m_pGSCtx -> CopyFBuffer (
  0, 1, lCY, s_GUICtx.m_pGSCtx -> m_Width - 10, lCH
 );

 s_GUICtx.m_pGSCtx -> SetTextColor (  GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0xFF  )  );
 s_GUICtx.m_pGSCtx -> m_Font.m_BkMode = GSBkMode_Transparent;
 s_GUICtx.m_pGSCtx -> DrawText ( 6, lY, 0, apSts, lLen );

}  /* end GUI_Status */

static void GUI_DrawDesktop ( void ) {

 s_GUICtx.m_pGSCtx -> ClearScreen (  GS_SETREG_RGBA( 0, 0, 0, 0 )  );

 GUIStub_DrawBackground ( s_GUICtx.m_pGSCtx );

 s_GUICtx.m_pGSCtx -> m_FillColor = GS_SETREG_RGBA( 0x00, 0, 0, 0x80 );
 s_GUICtx.m_pGSCtx -> m_LineColor = GS_SETREG_RGBA( 0xFF, 0, 0, 0x00 );
 s_GUICtx.m_pGSCtx -> RoundRect (
  0, s_GUICtx.m_pGSCtx -> m_Height - 32, s_GUICtx.m_pGSCtx -> m_Width - 1, s_GUICtx.m_pGSCtx -> m_Height - 2, 8
 );
 s_GUICtx.m_pGSCtx -> RoundRect ( 0,  2, s_GUICtx.m_pGSCtx -> m_Width - 1, 56, 8 );
 s_GUICtx.m_pGSCtx -> RoundRect ( 0, 62, s_GUICtx.m_pGSCtx -> m_Width - 1, s_GUICtx.m_pGSCtx -> m_Height - 38, 8 );

 s_GUICtx.m_pGSCtx -> CopyFBuffer (
  1, 0, 0, s_GUICtx.m_pGSCtx -> m_Width, s_GUICtx.m_pGSCtx -> m_Height >> 1
 );

 s_GUICtx.m_pGSCtx -> DrawText ( 6, s_GUICtx.m_DevMenu.m_StartY + 10, 0, STR_AVAILABLE_MEDIA, 0 );

}  /* end GUI_DrawDesktop */

static void GUI_Destroy ( void ) {

 void* lpPtr;

 s_Stop = 1;

 WakeupThread ( s_GUIThreadID );
 SleepThread ();

 Timer_RegisterHandler ( NULL );

 DeleteSema ( s_PadSema );

 lpPtr = s_GUICtx.m_DevMenu.m_pItems;

 while ( lpPtr ) {

  void* lpNext = (  ( GUIDeviceMenuItem* )lpPtr  ) -> m_pNext;

  free ( lpPtr );

  lpPtr = lpNext;

 }  /* end while */

 s_GUICtx.ClearFileMenu ();

}  /* end GUI_Destroy */

unsigned long int GUI_WaitEvent ( void ) {

 unsigned long int retVal;

 WaitSema ( s_PadSema );

 retVal  = s_Event;
 s_Event = 0LL;

 if (  retVal == ( PAD_SELECT | PAD_CIRCLE )  ) hddPowerOff ();

 return retVal;

}  /* end GUI_WaitEvent */

static void TimerProcRepeat ( void );

static void TimerProc ( void ) {

 static int s_Repeat = 0;

 unsigned int lCurrBtn;

 if (  padRead ( 0, 0, &s_PadData )  ) {

  lCurrBtn = 0xFFFF ^ s_PadData.btns;

  if ( lCurrBtn ) {

   if ( lCurrBtn != s_PrevBtn ) {

    s_PrevBtn  = lCurrBtn;
    s_Repeat   = 0;
    s_Event   |= lCurrBtn;
    SignalSema ( s_PadSema );

   } else if ( ++s_Repeat == 8 ) {

    s_Repeat = 0;
    DoTimer  = TimerProcRepeat;

   }  /* end if */

  } else {
doReset:
   s_PrevBtn = 0;
   s_Repeat  = 0;

  }  /* end else */

 } else goto doReset;

}  /* end TimerProc */

static void TimerProcRepeat ( void ) {

 unsigned int lCurrBtn;

 if (  padRead ( 0, 0, &s_PadData )  ) {

  lCurrBtn = 0xFFFF ^ s_PadData.btns;

  if ( lCurrBtn ) {

   if ( lCurrBtn != s_PrevBtn ) {

    s_PrevBtn  = lCurrBtn;
    s_Event   |= lCurrBtn;

    DoTimer = TimerProc;

   } else s_Event |= lCurrBtn;

   SignalSema ( s_PadSema );

  } else goto doReset;

 } else {
doReset:
  s_PrevBtn = 0;

 }  /* end else */

}  /* end TimerProcRepeat */

static void _wait_pad_ready ( void ) {

 int lState = padGetState ( 0, 0 );

 while (  lState != PAD_STATE_STABLE && lState != PAD_STATE_FINDCTP1  ) {

  GUI_Status ( "Waiting for game controller in port 0 slot 0..." );

  Timer_Wait ( 128 );

  lState = padGetState ( 0, 0 );

 }  /* end while */

}  /* end _wait_pad_ready */

static int _gui_thread ( void* apParam ) {

 while ( 1 ) {

  SleepThread ();

  if ( s_Stop ) break;

  if ( !s_NoCDCheck ) {

   if (  s_CDROM && !CDDA_IsAudioDisk ()  ) {

    s_Event |= GUI_EF_CDROM_UMOUNT;
    SignalSema ( s_PadSema );

    continue;

   } else if (  !s_CDROM && CDDA_IsAudioDisk ()  ) {

    s_Event |= GUI_EF_CDROM_MOUNT;
    SignalSema ( s_PadSema );

    continue;

   }  /* end if */

  }  /* end if */

  DoTimer ();

 }  /* end while */

 WakeupThread ( s_MainThreadID );

 return ExitDeleteThread (), 0;

}  /* end _gui_thread */

static void GUI_DrawDeviceMenu ( GUIDeviceMenu* apMenu ) {

 int                lX, lY;
 GUIDeviceMenuItem* lpItem = apMenu -> m_pItems;

 lX = apMenu -> m_StartX;
 lY = apMenu -> m_StartY;

 s_GUICtx.m_pGSCtx -> CopyFBuffer (
  0, lX, lY, s_GUICtx.m_pGSCtx -> m_Width - lX - 10, 24
 );

 if ( !lpItem ) {

  s_GUICtx.m_pGSCtx -> SetTextColor (  GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0xFF  )  );
  s_GUICtx.m_pGSCtx -> DrawText (
   s_GUICtx.m_DevMenu.m_StartX, s_GUICtx.m_DevMenu.m_StartY + 10, 0, "none", 0
  );

  return;

 }  /* end if */

 while ( lpItem ) {

  s_GUICtx.m_pGSCtx -> DrawIcon ( lX + 2, lY + 2, GSIS_48x48, lpItem -> m_pImage );

  if ( lpItem == apMenu -> m_pCurr ) {

   s_GUICtx.m_pGSCtx -> m_FillColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x60 );
   s_GUICtx.m_pGSCtx -> m_LineColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x60 );
   s_GUICtx.m_pGSCtx -> RoundRect ( lX + 4, lY + 6, lX + 50, lY + 48, 8 );

  }  /* end if */

  if ( lpItem == apMenu -> m_pSel ) {

   s_GUICtx.m_pGSCtx -> m_FillColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x80 );
   s_GUICtx.m_pGSCtx -> m_LineColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x00 );
   s_GUICtx.m_pGSCtx -> RoundRect ( lX + 4, lY + 6, lX + 50, lY + 48, 8 );

  }  /* end if */

  lX    += 52;
  lpItem = lpItem -> m_pNext;

 }  /* end while */

}  /* end GUI_DrawDeviceMenu */

static void GUI_AddDevice ( int aDev ) {

 GUIDeviceMenuItem* lpItem = ( GUIDeviceMenuItem* )calloc (  1, sizeof ( GUIDeviceMenuItem )  );

 if ( aDev == GUI_DF_CDROM ) {

  lpItem -> m_pImage = g_ImgCDROM;
  s_CDROM = 1;

 } else if ( aDev == GUI_DF_HDD )

  lpItem -> m_pImage = g_ImgHDD;

 else if ( aDev == GUI_DF_CDDA ) {

  lpItem -> m_pImage = g_ImgCDDA;
  s_CDROM = 1;

 }  /* end if */

 if ( !s_GUICtx.m_DevMenu.m_pItems ) {

  s_GUICtx.m_DevMenu.m_pItems = lpItem;
  s_GUICtx.m_DevMenu.m_pCurr  = lpItem;
  s_GUICtx.m_DevMenu.m_pSel   = lpItem;

 } else {

  s_GUICtx.m_DevMenu.m_pItems -> m_pNext = lpItem;
  lpItem -> m_pPrev = s_GUICtx.m_DevMenu.m_pItems;

 }  /* end else */

 lpItem -> m_Flags = aDev;

 GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu );

}  /* end GUI_AddDevice */

static void GUI_DelDevice ( int aDev ) {

 GUIDeviceMenuItem* lpItem = s_GUICtx.m_DevMenu.m_pItems;

 while ( lpItem ) {

  if ( aDev == lpItem -> m_Flags ) {

   if ( lpItem -> m_pPrev ) lpItem -> m_pPrev -> m_pNext = lpItem -> m_pNext;
   if ( lpItem -> m_pNext ) lpItem -> m_pNext -> m_pPrev = lpItem -> m_pPrev;

   if ( s_GUICtx.m_DevMenu.m_pCurr == lpItem ) s_GUICtx.m_DevMenu.m_pCurr = lpItem -> m_pPrev ? lpItem -> m_pPrev : lpItem -> m_pNext;
   if ( s_GUICtx.m_DevMenu.m_pSel  == lpItem ) s_GUICtx.m_DevMenu.m_pSel  = lpItem -> m_pPrev ? lpItem -> m_pPrev : lpItem -> m_pNext;

   if ( lpItem == s_GUICtx.m_DevMenu.m_pItems ) s_GUICtx.m_DevMenu.m_pItems = s_GUICtx.m_DevMenu.m_pCurr;

   free ( lpItem );

   if ( aDev == GUI_DF_CDROM || aDev == GUI_DF_CDDA ) s_CDROM = 0;

   break;

  }  /* end if */

  lpItem = lpItem -> m_pNext;

 }  /* end while */

 GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu );

}  /* end GUI_DelDevice */

static GUIDeviceMenuItem* GUI_NavigateDevice ( unsigned long int anEvent ) {

 if ( !s_GUICtx.m_DevMenu.m_pSel ) return ( GUIDeviceMenuItem* )GUI_EV_MENU_EMPTY;

 if ( anEvent & PAD_LEFT ) {

  if ( s_GUICtx.m_DevMenu.m_pSel -> m_pPrev ) {

   s_GUICtx.m_DevMenu.m_pSel = s_GUICtx.m_DevMenu.m_pSel -> m_pPrev;
   GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu );

  }  /* end if */

 } else if ( anEvent & PAD_RIGHT ) {

  if ( s_GUICtx.m_DevMenu.m_pSel -> m_pNext ) {

   s_GUICtx.m_DevMenu.m_pSel = s_GUICtx.m_DevMenu.m_pSel -> m_pNext;
   GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu );

  }  /* end if */

 } else if ( anEvent & PAD_CROSS ) {

  s_GUICtx.m_DevMenu.m_pCurr = s_GUICtx.m_DevMenu.m_pSel;
  GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu );

  return s_GUICtx.m_DevMenu.m_pCurr;

 } else if ( anEvent & PAD_DOWN ) {

  return ( GUIDeviceMenuItem* )GUI_EV_MENU_QUIT;

 }  /* end if */

 return ( GUIDeviceMenuItem* )GUI_EV_CONSUMED;

}  /* end GUI_NavigateDevice */

static void TimerHandler ( void ) {

 iWakeupThread ( s_GUIThreadID );

}  /* end TimerHandler */

static void GUI_DrawDimmedFileItem ( GUIFileMenuItem* apItem, int anY ) {

 int           i = 3, lTextWidth = s_GUICtx.m_pGSCtx -> TextWidth ( apItem -> m_pFileName, 0 );
 int           lStrLen = strlen ( apItem -> m_pFileName );
 unsigned char lBuff[ 4096 ] __attribute__(   (  aligned( 16 )  )   );

 memcpy ( lBuff, s_FSIcons[ apItem -> m_Flags & 0x0000000F ], 4096 );

 while ( i < 4096 ) {

  lBuff[ i ] >>= 1;
  i += 4;

 }  /* end while */

 SyncDCache ( lBuff, &lBuff[ 4096 ] );

 while ( lTextWidth >= s_GUICtx.m_FileMenu.m_Width ) lTextWidth = s_GUICtx.m_pGSCtx -> TextWidth ( apItem -> m_pFileName, --lStrLen );

 s_GUICtx.m_pGSCtx -> DrawIcon ( 6, anY, GSIS_32x32, lBuff );
 s_GUICtx.m_pGSCtx -> SetTextColor ( GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x60  )  );
 s_GUICtx.m_pGSCtx -> DrawText ( 40, anY + 4, 0, apItem -> m_pFileName, lStrLen  );
 s_GUICtx.m_pGSCtx -> SetTextColor ( GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0xFF  )  );

}  /* end GUI_DrawDimmedFileItem */

static void GUI_DrawFileItem ( GUIFileMenuItem* apItem, int anY ) {

 int lTextWidth = s_GUICtx.m_pGSCtx -> TextWidth ( apItem -> m_pFileName, 0 );
 int lStrLen    = strlen ( apItem -> m_pFileName );

 while ( lTextWidth >= s_GUICtx.m_FileMenu.m_Width ) lTextWidth = s_GUICtx.m_pGSCtx -> TextWidth ( apItem -> m_pFileName, --lStrLen );

 s_GUICtx.m_pGSCtx -> DrawIcon ( 6, anY, GSIS_32x32, s_FSIcons[ apItem -> m_Flags & 0x0000000F ] );
 s_GUICtx.m_pGSCtx -> DrawText ( 40, anY + 4, 0, apItem -> m_pFileName, lStrLen  );

 if ( apItem -> m_Flags & GUI_FF_SELECTED ) {

  s_GUICtx.m_pGSCtx -> m_FillColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x60 );
  s_GUICtx.m_pGSCtx -> m_LineColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0x00, 0x00 );
  s_GUICtx.m_pGSCtx -> RoundRect ( 6, anY, s_GUICtx.m_pGSCtx -> m_Width - 6, anY + 34, 4 );

 }  /* end if */

}  /* end GUI_DrawFileItem */

static void GUI_DrawFileMenu ( GUIFileMenu* apMenu ) {

 int              lY     = 1;
 int              lH     = apMenu -> m_Height - 1;
 GUIFileMenuItem* lpItem = apMenu -> m_pFirst;

 s_GUICtx.m_pGSCtx -> CopyFBuffer (
  0, 4, 33, s_GUICtx.m_pGSCtx -> m_Width - 6, ( s_GUICtx.m_pGSCtx -> m_Height >> 1 ) - 53
 );

 if ( lpItem ) {

  s_GUICtx.m_pGSCtx -> SetTextColor ( GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0xFF  )  );

  if ( lpItem != apMenu -> m_pItems )

   GUI_DrawDimmedFileItem (  lpItem, 66 + ( lpItem -> m_Y - apMenu -> m_Offset ) * 34  );

  else GUI_DrawFileItem (  lpItem, 66 + ( lpItem -> m_Y - apMenu -> m_Offset ) * 34  );

  lpItem = lpItem -> m_pNext;

  while ( lpItem && lY < lH ) {

   GUI_DrawFileItem (  lpItem, 66 + ( lpItem -> m_Y - apMenu -> m_Offset ) * 34  );

   ++lY; lpItem = lpItem -> m_pNext;

  }  /* end while */

  if ( lpItem ) {

   if ( lpItem != apMenu -> m_pLast )

    GUI_DrawDimmedFileItem (  lpItem, 66 + ( lpItem -> m_Y - apMenu -> m_Offset ) * 34  );

   else GUI_DrawFileItem (  lpItem, 66 + ( lpItem -> m_Y - apMenu -> m_Offset ) * 34 );

  }  /* end if */

 }  /* end if */

}  /* end GUI_DrawFileMenu */

static void GUI_AddFile ( char* apFileName, int aType ) {

 int              lLen   = strlen ( apFileName ) + 1;
 GUIFileMenuItem* lpItem = ( GUIFileMenuItem* )calloc (  1, sizeof ( GUIFileMenuItem )  );

 lpItem -> m_pFileName = ( char* )malloc ( lLen );
 strcpy ( lpItem -> m_pFileName, apFileName );

 lpItem -> m_Flags = aType;

 if ( s_GUICtx.m_FileMenu.m_pItems == NULL ) {

  s_GUICtx.m_FileMenu.m_pItems =
  s_GUICtx.m_FileMenu.m_pCurr  =
  s_GUICtx.m_FileMenu.m_pLast  =
  s_GUICtx.m_FileMenu.m_pFirst = lpItem;

  lpItem -> m_Y      = 0;
  lpItem -> m_Flags |= GUI_FF_SELECTED;

 } else {

  lpItem -> m_pPrev                      = s_GUICtx.m_FileMenu.m_pLast;
  s_GUICtx.m_FileMenu.m_pLast -> m_pNext = lpItem;
  lpItem -> m_Y                          = s_GUICtx.m_FileMenu.m_pLast -> m_Y + 1;
  s_GUICtx.m_FileMenu.m_pLast            = lpItem;

 }  /* end else */

}  /* end GUI_AddFile */

static void GUI_ClearFileMenu ( void ) {

 GUIFileMenuItem* lpItem = s_GUICtx.m_FileMenu.m_pItems;

 while ( lpItem ) {

  GUIFileMenuItem* lpNext = lpItem -> m_pNext;

  free ( lpItem -> m_pFileName );
  free ( lpItem );

  lpItem = lpNext;

 }  /* end while */

 s_GUICtx.m_FileMenu.m_pItems =
 s_GUICtx.m_FileMenu.m_pFirst =
 s_GUICtx.m_FileMenu.m_pLast  =
 s_GUICtx.m_FileMenu.m_pCurr  = NULL;
 s_GUICtx.m_FileMenu.m_Offset = 0;

 GUI_DrawFileMenu ( &s_GUICtx.m_FileMenu );

}  /* end GUI_ClearFileMenu */

static GUIFileMenuItem* GUI_NavigateFile ( unsigned long int anEvent ) {

 int              lIdx;
 GUIFileMenuItem* lpItem = s_GUICtx.m_FileMenu.m_pCurr;

 if (  anEvent & ( PAD_LEFT | PAD_RIGHT )  ) return ( GUIFileMenuItem* )GUI_EV_MENU_QUIT;

 if ( !lpItem ) return anEvent & PAD_TRIANGLE ? ( GUIFileMenuItem* )GUI_EV_UPLEVEL : ( GUIFileMenuItem* )GUI_EV_MENU_EMPTY;

 lIdx = lpItem -> m_Y - s_GUICtx.m_FileMenu.m_Offset;

 s_GUICtx.m_pGSCtx -> VSync ();

 if (  ( anEvent & PAD_DOWN ) && lpItem -> m_pNext  ) {

  lpItem -> m_Flags &= ~GUI_FF_SELECTED;
  s_GUICtx.m_pGSCtx -> CopyFBuffer (
   0, 1, ( 66 + lIdx * 34 ) >> 1, s_GUICtx.m_pGSCtx -> m_Width - 2, 18
  );
  GUI_DrawFileItem ( lpItem, 66 + lIdx++ * 34 );

  if ( lIdx < s_GUICtx.m_FileMenu.m_Height - 1 ||
       lpItem -> m_pNext -> m_pNext == NULL
  ) {

   lpItem -> m_pNext -> m_Flags |= GUI_FF_SELECTED;
   GUI_DrawFileItem ( lpItem -> m_pNext, 66 + lIdx * 34 );

  } else {

   ++s_GUICtx.m_FileMenu.m_Offset;
   s_GUICtx.m_FileMenu.m_pFirst = s_GUICtx.m_FileMenu.m_pFirst -> m_pNext;
   lpItem -> m_pNext -> m_Flags |= GUI_FF_SELECTED;

   GUI_DrawFileMenu ( &s_GUICtx.m_FileMenu );

  }  /* end else */

  s_GUICtx.m_FileMenu.m_pCurr = lpItem -> m_pNext;

 } else if (  ( anEvent & PAD_UP ) && lpItem -> m_pPrev  ) {

  lpItem -> m_Flags &= ~GUI_FF_SELECTED;
  s_GUICtx.m_pGSCtx -> CopyFBuffer (
   0, 1, ( 66 + lIdx * 34 ) >> 1, s_GUICtx.m_pGSCtx -> m_Width - 2, 18
  );
  GUI_DrawFileItem ( lpItem, 66 + lIdx-- * 34 );

  if ( lIdx > 0 || lpItem -> m_pPrev -> m_pPrev == NULL ) {

   lpItem -> m_pPrev -> m_Flags |= GUI_FF_SELECTED;
   GUI_DrawFileItem ( lpItem -> m_pPrev, 66 + lIdx * 34 );

  } else {

   --s_GUICtx.m_FileMenu.m_Offset;
   s_GUICtx.m_FileMenu.m_pFirst  = s_GUICtx.m_FileMenu.m_pFirst -> m_pPrev;
   lpItem -> m_pPrev -> m_Flags |= GUI_FF_SELECTED;

   GUI_DrawFileMenu ( &s_GUICtx.m_FileMenu );

  }  /* end else */

  s_GUICtx.m_FileMenu.m_pCurr = lpItem -> m_pPrev;

 } else if ( anEvent & PAD_TRIANGLE ) {

  return ( GUIFileMenuItem* )GUI_EV_UPLEVEL;

 } else if ( anEvent & PAD_CROSS ) return lpItem;

 return ( GUIFileMenuItem* )GUI_EV_CONSUMED;

}  /* end GUI_NavigateFile */

static void GUI_ActivateMenu ( int anIndex ) {

 int          lYOrg, lHOrg;
 int          lY,    lH;
 unsigned int lColor = GS_SETREG_RGBA( 0xFF, 0xFF, 0xFF, 0x00 );

 if ( anIndex == 0 ) {

  lYOrg = 62;
  lHOrg = s_GUICtx.m_pGSCtx -> m_Height - 38;

  lY = 2;
  lH = 56;

  s_GUICtx.m_pCurrentMenu = ( GUIMenu* )&s_GUICtx.m_DevMenu;
  GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu );

 } else {

  lY = 62;
  lH = s_GUICtx.m_pGSCtx -> m_Height - 38;

  lYOrg = 2;
  lHOrg = 56;

  if ( anIndex != -1 ) {

   s_GUICtx.m_pCurrentMenu = ( GUIMenu* )&s_GUICtx.m_FileMenu;
   GUI_DrawFileMenu ( &s_GUICtx.m_FileMenu );

  } else {

   s_GUICtx.m_pCurrentMenu = NULL;
   lColor                  = GS_SETREG_RGBA( 0xFF, 0x00, 0x00, 0x00 );

  }  /* end else */

 }  /* end else */

 s_GUICtx.m_pGSCtx -> m_FillColor = GS_SETREG_RGBA( 0x00, 0, 0, 0x80 );
 s_GUICtx.m_pGSCtx -> m_LineColor = GS_SETREG_RGBA( 0xFF, 0, 0, 0x00 );
 s_GUICtx.m_pGSCtx -> RoundRect (
  0, lYOrg, s_GUICtx.m_pGSCtx -> m_Width - 1, lHOrg, 8
 );
 s_GUICtx.m_pGSCtx -> m_LineColor = lColor;
 s_GUICtx.m_pGSCtx -> RoundRect (
  0, lY, s_GUICtx.m_pGSCtx -> m_Width - 1, lH, 8
 );

}  /* end GUI_ActivateMenu */

static int GUI_Run ( void** apRetVal ) {

 int retVal;

 Timer_RegisterHandler ( TimerHandler );

 while ( 1 ) {

  unsigned long int lEvent = GUI_WaitEvent ();

  if ( lEvent & GUI_EF_CDROM_MOUNT ) {

   retVal = GUI_EV_CDROM_MOUNT;
   break;

  } else if ( lEvent & GUI_EF_CDROM_UMOUNT ) {

   retVal = GUI_EV_CDROM_UMOUNT;
   break;

  } else if ( s_GUICtx.m_pCurrentMenu ) {

   void* lpResult = s_GUICtx.m_pCurrentMenu -> Navigate ( lEvent );

   if (  ( int )lpResult > GUI_EV_MENU_EMPTY  ) {

    if (  ( void* )s_GUICtx.m_pCurrentMenu == ( void* )&s_GUICtx.m_DevMenu ) {

     retVal    = GUI_EV_DEV_SELECT;
     *apRetVal = lpResult;

     break;

    } else {

     retVal    = GUI_EV_FILE_SELECT;
     *apRetVal = lpResult;

     break;

    }  /* end else */

   } else if (  ( int )lpResult == GUI_EV_UPLEVEL  ) {

    retVal = GUI_EV_UPLEVEL;

    break;

   } else if (  ( int )lpResult == GUI_EV_MENU_QUIT  ) GUI_ActivateMenu (  ( void* )s_GUICtx.m_pCurrentMenu == ( void* )&s_GUICtx.m_DevMenu ? 1 : 0  );

  }  /* end else */

 }  /* end while */

 Timer_RegisterHandler ( NULL );

 return retVal;

}  /* end GUI_Run */

static void GUI_Redraw ( void ) {

 s_GUICtx.m_pGSCtx -> m_fDblBuf = GS_ON;
 s_GUICtx.m_pGSCtx -> ClearScreen (  GS_SETREG_RGBA( 0x00, 0x00, 0x00, 0x00 )  );
 s_GUICtx.m_pGSCtx -> VSync ();
 s_GUICtx.m_pGSCtx -> InitScreen ();
 s_GUICtx.m_pGSCtx -> VSync ();

 GUI_DrawDesktop    ();
 GUI_DrawDeviceMenu ( &s_GUICtx.m_DevMenu  );
 GUI_DrawFileMenu   ( &s_GUICtx.m_FileMenu );

}  /* end GUI_Redraw */

void GUI_WaitButton ( int aButton ) {

 s_NoCDCheck = 1;

 Timer_RegisterHandler ( TimerHandler );

 while ( 1 ) {

  unsigned long int lEvent = GUI_WaitEvent ();

  if ( lEvent & aButton ) break;

 }  /* end while */

 Timer_RegisterHandler ( NULL );

 s_NoCDCheck = 0;

}  /* end GUI_WaitButton */

int GUI_ReadButtons ( void ) {

 int retVal = 0;

 if (  padRead ( 0, 0, &s_PadData )  ) retVal = 0xFFFF ^ s_PadData.btns;

 return retVal;

}  /* end GUI_CheckButton */

GUIContext* GUI_InitContext ( GSContext* apGSCtx ) {

 ee_sema_t   lSema;
 ee_thread_t lThread;

 s_MainThreadID = GetThreadId ();
 s_Stop         = 0;
 s_Event        = 0;
 s_CDROM        = 0;
 s_PrevBtn      = 0;
 s_Init         = 0;

 apGSCtx -> m_fDblBuf = GS_ON;
 apGSCtx -> InitScreen ();

 SifLoadModule ( "rom0:SIO2MAN", 0, NULL );
 SifLoadModule ( "rom0:PADMAN",  0, NULL );

 s_GUICtx.m_DevMenu.m_StartX = 6 + apGSCtx -> TextWidth ( STR_AVAILABLE_MEDIA, 0 );
 s_GUICtx.m_DevMenu.m_StartY = 4;
 s_GUICtx.m_DevMenu.m_pItems = NULL;
 s_GUICtx.m_DevMenu.m_pCurr  = NULL;
 s_GUICtx.m_DevMenu.Navigate = GUI_NavigateDevice;

 s_GUICtx.m_FileMenu.m_pItems = NULL;
 s_GUICtx.m_FileMenu.m_pCurr  = NULL;
 s_GUICtx.m_FileMenu.m_pFirst = NULL;
 s_GUICtx.m_FileMenu.m_pLast  = NULL;
 s_GUICtx.m_FileMenu.m_Width  = apGSCtx -> m_Width - 46;
 s_GUICtx.m_FileMenu.m_Height = ( apGSCtx -> m_Height - 104 ) / 34;
 s_GUICtx.m_FileMenu.m_Offset = 0;
 s_GUICtx.m_FileMenu.Navigate = GUI_NavigateFile;

 s_GUICtx.m_pCurrentMenu = NULL;

 s_GUICtx.m_pGSCtx      = apGSCtx;
 s_GUICtx.Status        = GUI_Status;
 s_GUICtx.Destroy       = GUI_Destroy;
 s_GUICtx.Run           = GUI_Run;
 s_GUICtx.AddDevice     = GUI_AddDevice;
 s_GUICtx.DelDevice     = GUI_DelDevice;
 s_GUICtx.AddFile       = GUI_AddFile;
 s_GUICtx.ClearFileMenu = GUI_ClearFileMenu;
 s_GUICtx.ActivateMenu  = GUI_ActivateMenu;
 s_GUICtx.Redraw        = GUI_Redraw;

 lSema.init_count = 0;
 lSema.max_count  = 1;
 s_PadSema = CreateSema ( &lSema );

 lThread.stack_size       = sizeof ( s_Stack );
 lThread.stack            = s_Stack;
 lThread.gp_reg           = &_gp;
 lThread.initial_priority = 1;
 lThread.func             = _gui_thread;
 StartThread (  s_GUIThreadID = CreateThread ( &lThread ), NULL  );

 GUI_Redraw ();
 GUI_Status ( "Initializing pad..." );

 padInit ( 0 );

 if (  !padPortOpen ( 0, 0, s_PadBuf )  ) {

  GUI_Status ( "Could not initialize pad, program terminated." );
  SleepThread ();

 }  /* end if */

 _wait_pad_ready ();

 padSetMainMode ( 0, 0, PAD_MMODE_DIGITAL, PAD_MMODE_LOCK );

 DoTimer = TimerProc;
 s_Init  = 1;

 return &s_GUICtx;

}  /* end GUI_InitContext */
