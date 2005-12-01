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
# Date: 10-6-2005
#
# File: module loading, other
#		initialization functions etc.
#
*/

#include <tamtypes.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libhdd.h>
#include <libmc.h>
#include <sys/fcntl.h>	// FIO_MT_RDWR
#include <sbv_patches.h>
#include <libusb/mass_rpc.h>
#include <kernel.h>

#include <gr.h>
#include <ui.h>
#include <gamepad.h>
#include <misc.h>
#include <imgscale.h>
#include <sysconf.h>
#include <mp3.h>
#include <scheduler.h>

// testing
void MP3_ThreadTEST( void )
{
	MP3_Play( "pfs0:/MP3/TEST.MP3" );
}

int main( int argc, char *argv[] )
{
	int nTexBufAddr;

	int	nScrMode, nScrVer, nScrHor;

	loadModules( argv[0] );
	dma_initialize();

	// init precision timer
	tnTimeInit();

	// init memory card
	MC_Init();

	// load system configuration from mc0
	SC_LoadConfig();
	
	// initialize network
	NET_Init( argv[0] );

	// initialize ftp daemon
	FTP_Init();

	// initialize and setup GR lib
	SC_GetValueForKey_Int( "scr_mode", &nScrMode );

	GR_Init( nScrMode, GR_PSM_32, GR_DOUBLE_BUFFERING );
	nTexBufAddr = GR_SetFrameBuffers( 0 );
	GR_SetTextureBuffer( nTexBufAddr );

	// clear so we don't get ugly flickering during loading
	GR_SetDrawFrame(0);
	GR_ClearScreen();

	GR_SetDrawFrame(1);
	GR_SetDisplayFrame(0);

	SC_GetValueForKey_Int( "scr_adjust_x", &nScrHor );
	SC_GetValueForKey_Int( "scr_adjust_y", &nScrVer );

	GR_SetScreenAdjust( nScrHor, nScrVer );

	// init hdd, usb and gamepad
	HDD_Init();
	USB_Init();
	GP_Init();

	// init the user interface
	UI_Init();

	// init the thread scheduler
	Scheduler_Init();

	// create ui thread
	Scheduler_BeginThread( UI_Thread );
	Scheduler_BeginThread( MP3_ThreadTEST );

	Scheduler_Run();

	HDD_ShutDown();
	GR_ShutDown();
	SC_Clean();
	return 0;
}

//
// loadModules - Loads all important IRX modules.
//
// CDVD module is not loaded at start up because it won't
// run if there is no disk in the drive. It is loaded later
// when it is needed. PS2IP and SMAP are loaded later after
// the config file from mc0 has been loaded.
//

void loadModules( const char *path )
{
	int			ret, irx_ret;
	const char	hddarg[]	= "-o" "\0" "4" "\0" "-n" "\0" "20";
	const char	pfsarg[]	= "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";
	int i, bootmode;

	SifInitRpc(0);

	hddPreparePoweroff();

	// install sbv patch fix
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();

	// figure out boot mode
	i = strcspn( path, ":" );

	if( !strncmp( path, "host", i ) ) {
		bootmode = BOOT_HOST;
	}
	else if( !strncmp( path, "mc", i ) ) {
		bootmode = BOOT_MC;
	}
	else if( !strncmp( path, "hdd", i ) ) {
		bootmode = BOOT_HDD;
	}
	else {
		bootmode = BOOT_CD;
	}

	// SIO stuff
	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("sifLoadModule sio failed: %d\n", ret);
#endif
		SleepThread();
	}

	// Memory Card stuff
	ret = SifLoadModule("rom0:MCMAN", 0, NULL);
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("sifLoadModule mcman failed: %d\n", ret);
#endif
		SleepThread();
	}

	// Memory Card stuff
	ret = SifLoadModule("rom0:MCSERV", 0, NULL);
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("sifLoadModule mcserv failed: %d\n", ret);
#endif
		SleepThread();
	}

	// Gamepad driver
	ret = SifLoadModule("rom0:PADMAN", 0, NULL);
	if( ret < 0 ) {
#ifdef _DEBUG_GP
		printf("sifLoadModule pad failed: %d\n", ret);
#endif
		SleepThread();
	}

	// IOMANX driver (embedded in ELF)
	ret = SifExecModuleBuffer( &iomanx_irx, size_iomanx_irx, 0, NULL, &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer iomanx failed: %d\n", ret);
#endif
		SleepThread();
	}

	// fileXio driver (embedded in ELF)
	ret = SifExecModuleBuffer( &filexio_irx, size_filexio_irx, 0, NULL, &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer filexio failed: %d\n", ret);
#endif
		SleepThread();
	}

	if( bootmode != BOOT_HOST )
	{
		// DEV9 driver (embedded in ELF)
		ret = SifExecModuleBuffer( &ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &irx_ret );
		if( ret < 0 ) {
#ifdef _DEBUG
			printf("SifExecModuleBuffer dev9 failed: %d\n", ret);
#endif
			SleepThread();
		}
	}

	// ATAD driver (embedded in ELF)
	ret = SifExecModuleBuffer( &ps2atad_irx, size_ps2atad_irx, 0, NULL, &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer ps2atad failed: %d\n", ret);
#endif
		SleepThread();
	}

	// HDD driver (embedded in ELF)
	ret = SifExecModuleBuffer( &ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, &ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer ps2hdd failed: %d\n", ret);
#endif
		SleepThread();
	}

	// PS2 file system driver (embedded in ELF)
	ret = SifExecModuleBuffer( &ps2fs_irx, size_ps2fs_irx, sizeof(pfsarg), pfsarg, &ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer ps2fs failed: %d\n", ret);
#endif
		SleepThread();
	}

	// Poweroff module (embedded in ELF)
	ret = SifExecModuleBuffer( &poweroff_irx, size_poweroff_irx, 0, NULL, &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer poweroff failed: %d\n", ret);
#endif
		SleepThread();
	}

	// LIBSD
	ret = SifLoadModule( "rom0:LIBSD", 0, NULL );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifLoadModule LIBSD failed: %d\n", ret);
#endif
		SleepThread();
	}

	// SjPCM module (embedded in ELF)
	ret = SifExecModuleBuffer( &sjpcm_irx, size_sjpcm_irx, 0, NULL, &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExexModuleBuffer sjpcm failed : %d\n", ret);
#endif
		SleepThread();
	}
}

//
// CD_Init - Loads the cdvd.irx module and initializes CDVD.
//

void CD_Init( void )
{
	static int init;
	int ret, irx_ret;

	// don't bother if already initialized
	if( init )
		return;

	// load CDVD module (embedded in ELF)
	ret = SifExecModuleBuffer( &cdvd_irx, size_cdvd_irx, 0, NULL, &irx_ret );
	if(ret < 0) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer cdvd failed: %d\n", ret);
#endif
		SleepThread();
	}

	CDVD_Init();

	init = 1;
#ifdef _DEBUG
	printf("CD_Init : CDVD initialized.\n");
#endif
}


//
// --------------------- HDD initialization and helper functions ---------------------
//

int bHddAvail = HDD_NOT_AVAIL;


//
// HDD_Init - initializes HDD and attempts to mount myPS2 partition
//

void HDD_Init( void )
{
	int				nRet;
	iox_stat_t		info;
	char			openString[256];

	nRet = hddCheckPresent();
	if( nRet == -1 )
	{
#ifdef _DEBUG
		printf("Could not find HDD.\n");
#endif
		bHddAvail = HDD_NOT_AVAIL;
		return;
	}

	nRet = hddCheckFormatted();
	if( nRet == -1 )
	{
#ifdef _DEBUG
		printf("HDD is not formatted.\n");
#endif
		bHddAvail = HDD_NOT_AVAIL;
		return;
	}

	strcpy( openString, "hdd0:+" );
	strcat( openString, PARTITION_NAME );

	// check for partition
	nRet = fileXioGetStat( openString, &info );
	if( nRet != 0 )
	{
		bHddAvail = HDD_AVAIL_NO_PART;
		return;
	}

	// attempt to mount partition
	nRet = HDD_MountPartition( "pfs0:", PARTITION_NAME );

	if( !nRet )
	{
#ifdef _DEBUG
		printf("Could not mount partition %s.\n", openString);
#endif
		// there must be some problem with the partition
		bHddAvail = HDD_NOT_AVAIL;
		return;
	}

	// everything's okay
	bHddAvail = HDD_AVAIL;
}

//
// HDD_Available - Returns availability status of HDD.
//				   Possible return values are :
//					- HDD_NOT_AVAIL
//					- HDD_AVAIL
//					- HDD_AVAIL_NO_PART
//

int HDD_Available( void )
{
	return bHddAvail;
}

//
// HDD_GetFreeSpace - Returns amount of free HDD space in megabytes
//

int HDD_GetFreeSpace( void )
{
	t_hddInfo hddInfo;

	if( HDD_Available() == HDD_NOT_AVAIL )
		return 0;

	hddGetInfo( &hddInfo );

	return hddInfo.hddFree;
}

//
// HDD_CreatePartition - Attempts to create a new partition.
//						 Returns 1 on success, otherwise 0.
//
//						 Ensures partition size is a multiple
//						 of 128 MB.
//

int HDD_CreatePartition( char *partName, int partSize )
{
	int		nRet;
	int		nMul;

	if( HDD_Available() == HDD_NOT_AVAIL )
		return 0;

	// make sure partSize is actually a multiple of 128
	// because otherwise hddMakeFilesystem will fail.
	nMul		= partSize / 128;
	partSize	= nMul * 128;

#ifdef _DEBUG
	printf("HDD_CreatePartition : Adjusted partSize to %i MB\n", partSize );
#endif

	nRet = hddMakeFilesystem( partSize, partName, FS_GROUP_COMMON );

	if( nRet < 0 )
		return 0;

	return 1;
}

//
// HDD_MountPartition - Attempts to mount a partition.
//						Returns 1 on success, otherwise 0.
//

int HDD_MountPartition( const char *mountPoint, const char *partName )
{
	int		nRet;
	char	openString[256];

	strcpy( openString, "hdd0:+" );
	strcat( openString, partName );

	nRet = fileXioMount( mountPoint, openString, FIO_MT_RDWR );

	// could not mount partition
	if( nRet < 0 )
		return 0;

	// okay, partition is now available
	bHddAvail = HDD_AVAIL;

	return 1;
}

//
// HDD_ShutDown - Unmounts myPS2 HDD partition
//

void HDD_ShutDown( void )
{
	fileXioUmount("pfs0:");
	bHddAvail = HDD_NOT_AVAIL;
}

//
// --------------------- MC initialization and helper functions ---------------------
//

int nMCInit = 0;

//
// MC_Init - initializes Memory Card
//

void MC_Init( void )
{
	int nRet;

	nRet = mcInit(MC_TYPE_MC);

	if( nRet < 0 ) {
#ifdef _DEBUG
		printf("MC_Init : failed to initialize memcard server.\n");
#endif
		nMCInit = 0;
	}

	nMCInit = 1;

	// create directory for config files
	if( !MC_Available(0) )
		return;

#ifdef _DEBUG
	printf("MC_Init : Checking for directory mc0:/SYS_CONF/MYPS2.\n");
#endif

	FileMkdir( "mc0:/SYS-CONF" );
	FileMkdir( "mc0:/SYS-CONF/MYPS2" );
}

//
// MC_Available - Returns 1 if MC in port is connected
//

int MC_Available( int nPort )
{
	int mcType, mcFree, mcFormat;
	int nRet;

	if( !nMCInit )
		return 0;

	if( nPort < 0 || nPort > 1 )
		return 0;

	mcGetInfo( nPort, 0, &mcType, &mcFree, &mcFormat );
	mcSync( 0, NULL, &nRet );

	// MC could not be found or is not properly formatted
	if( nRet <= -10 || nRet == -2 )
		return 0;

	return 1;
}

//
// --------------------- USB initialization and helper functions ---------------------
//

int nUSBInit = 0;

//
// USB_Init - Loads modules and initializes USB
//

void USB_Init( void )
{
	int nRet, irx_ret;
	char strPath[256];

	// user wants to load a custom usbd.irx
	if( SC_GetValueForKey_Int( "usbd_irx_custom", NULL ) ) {
		SC_GetValueForKey_Str( "usbd_irx_path", strPath );

		nRet = SifLoadModule( strPath, 0, NULL );
		if( nRet < 0 ) {
#ifdef _DEBUG
			printf("SifLoadModule %s failed: %d\n", strPath, nRet);
#endif
			return;
		}
	}
/*
	else {
		// load Naplink usbd module
		nRet = SifExecModuleBuffer( &npm_usbd_irx, size_npm_usbd_irx, 0, NULL, &irx_ret );
		if( nRet < 0 ) {
#ifdef _DEBUG
			printf("SifExecModuleBuffer npm_usbd failed: %d\n", nRet);
#endif
			return;
		}
	}
*/

	// load USB mass module (embedded in ELF)
	nRet = SifExecModuleBuffer( &usb_mass_irx, size_usb_mass_irx, 0, NULL, &irx_ret );
	if( nRet < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer usb_mass failed: %d\n", nRet);
#endif
		return;
	}

	// init USB RPC stuff
	nRet = usb_mass_bindRpc();
	
	if( nRet < 0 ) {
#ifdef _DEBUG
		printf("USB_Init: usb_mass_bindRpc failed : %d\n", nRet);
#endif
		return;
	}

	// everything's ok
	nUSBInit = 1;
}

//
// USB_Available - Returns 1 if USB device in either slot
//				   is connected,
//

int USB_Available( void )
{
	if( !nUSBInit )
		return 0;

	return usb_mass_getConnectState(0);
}

//
// --------------------- Net initialization and helper functions ---------------------
//

int nNetInit = 0;
int nFTPInit = 0;

//
// NET_Init - Attempts to load PS2IP and SMAP modules
//

void NET_Init( const char *path )
{
	int ret, i, bootmode, irx_ret;
	char params[256];
	char string[256];

	// Config.dat should have been loaded by now
	if( !SC_GetValueForKey_Int( "net_enable", NULL ) )
		return;

	// figure out boot mode
	i = strcspn( path, ":" );

	if( !strncmp( path, "host", i ) )
		bootmode = BOOT_HOST;
	else if( !strncmp( path, "mc", i ) )
		bootmode = BOOT_MC;
	else if( !strncmp( path, "hdd", i ) )
		bootmode = BOOT_HDD;
	else
		bootmode = BOOT_CD;

	// if booting from HOST modules should already be present
	if( bootmode == BOOT_HOST ) {
		nNetInit = 1;
		return;
	}

	ret = SifExecModuleBuffer( &ps2ip_irx, size_ps2ip_irx, 0, NULL, &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer ps2ip failed: %d\n", ret);
#endif
		return;
	}

	// prepare smap parameters
	memset( params, 0, sizeof(params) );
	i = 0;

	SC_GetValueForKey_Str( "net_ip", string );
	strncpy( &params[i], string, 15 );
	i += strlen(string) + 1;

	SC_GetValueForKey_Str( "net_netmask", string );
	strncpy( &params[i], string, 15 );
	i += strlen(string) + 1;

	SC_GetValueForKey_Str( "net_gateway", string );
	strncpy( &params[i], string, 15 );
	i += strlen( string ) + 1;

	// load SMAP module
	ret = SifExecModuleBuffer( &ps2smap_irx, size_ps2smap_irx, i, &params[0], &irx_ret );
	if( ret < 0 ) {
#ifdef _DEBUG
		printf("SifExecModuleBuffer ps2smap failed: %d\n", ret);
#endif
		return;
	}

	nNetInit = 1;
}

//
// NET_Available - Returns 1 if network was successfully initialized
//

int NET_Available( void )
{
	return nNetInit;
}

//
// FTP_Init - Attempts to load PS2FTPD module
//

void FTP_Init( void )
{
	int ret, irx_ret, i;
	char param[128];
	char list[256];

	if( !NET_Available() )
		return;

	if( !SC_GetValueForKey_Int( "ftp_daemon", NULL ) )
		return;

	i = 0;
	memset( list, 0, sizeof(list) );

	strcpy( &list[i], "-port" );
	i += strlen("-port") + 1;

	SC_GetValueForKey_Str( "ftp_port", param );
	strcpy( &list[i], param );
	i += strlen(param) + 1;

	if( SC_GetValueForKey_Int( "ftp_anonymous", NULL ) ) {
		strcpy( &list[i], "-anonymous" );
		i += strlen("-anonymous") + 1;
	}
	else {
		strcpy( &list[i], "-user" );
		i += strlen("-user") + 1;

		SC_GetValueForKey_Str( "ftp_login", param );
		strcpy( &list[i], param );
		i += strlen(param) + 1;

		strcpy( &list[i], "-pass" );
		i += strlen("-pass") + 1;

		SC_GetValueForKey_Str( "ftp_password", param );
		strcpy( &list[i], param );
		i += strlen(param) + 1;
	}

	// FIXME : fix "Null Buffer Warning" in ps2ftpd.irx
	ret = SifExecModuleBuffer( &ps2ftpd_irx, size_ps2ftpd_irx, i, &list[0], &irx_ret );
	if( ret < 0 )
	{
#ifdef _DEBUG
		printf("SifExexModuleBuffer ps2ftpd failed: %d\n", ret);
#endif
		return;
	}

	nFTPInit = 1;
}

//
// FTP_Available - Returns 1 if ftp daemon was successfully initialized
//

int FTP_Available( void )
{
	return nFTPInit;
}
