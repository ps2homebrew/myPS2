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
# Date: 11-15-2005
#
# File: System Config Loading/Saving
#
*/

#include <tamtypes.h>
#include <gr.h>
#include <file.h>
#include <misc.h>
#include <sysconf.h>

// sysConf can be accessed from all other source code files.
sysConf_t sysConf;

//
// SysConfLoad - Attempts to load CONFIG.INI from mc0:/SYS-CONF/MYPS2/
//				 and initializes global sysConf variable.
//
//				 Returns 1 if CONFIG.INI was successfully loaded,
//				 otherwise 0.
//

int SysConfLoad( void )
{
	FHANDLE		fHandle;
	char		strLine[1024];
	char		strToken[128];
	char		strValue[256];
	char		*p, *pEnd;
	int			len;

	// init sysConf with default values
	SysConfDefault();

	if( !MC_Available(0) )
		return 0;

	fHandle = FileOpen( "mc0:/SYS-CONF/MYPS2/CONFIG.INI", O_RDONLY );

	if( fHandle.fh < 0 )
		return 0;

	// parse buffer line by line
	while( FileGets( strLine, sizeof(strLine), fHandle ) ) {

		// skip comments
		if( strLine[0] == '#' )
			continue;

		// skip empty lines
		if( strLine[0] == '\n' )
			continue;

		// grab token
		p = strchr( strLine, '=' );

		// invalid line, go on to next line
		if( !p ) {
#ifdef _DEBUG
			printf("SysConfLoad : Ignoring Invalid Line\n");
#endif
			continue;
		}

		len = p - strLine;

		strncpy( strToken, strLine, len );
		strToken[ len ] = 0;

		// grab value
		pEnd = strLine + strlen(strLine) - 1;

		// cut off line feed
		if( *pEnd == '\n' )
			pEnd--;

		len = pEnd - p;

		// skip '='
		p++;

		strncpy( strValue, p, len );
		strValue[ len ] = 0;

		// parse setting
		if( !strcmp( strToken, "scr_adjust_x" ) ) {
			sysConf.scr_adjust_x = atoi( strValue );
		}
		else if( !strcmp( strToken, "scr_adjust_y" ) ) {
			sysConf.scr_adjust_y = atoi( strValue );
		}
		else if( !strcmp( strToken, "scr_mode" ) ) {
			sysConf.scr_mode = atoi( strValue );
		}
		else {
#ifdef _DEBUG
			printf("SysConfLoad : Warning : Encountered unknown token\n");
#endif
		}

	}

	FileClose(fHandle);
	return 1;
}

//
// SysConfSave - Attempts to save CONFIG.INI to mc0:/SYS-CONF/MYPS2/
//
//				 Returns 1 if CONFIG.INI was successfully saved,
//				 otherwise 0.
//

int SysConfSave( void )
{
	FHANDLE		fHandle;
	char		strLine[1024];

	if( !MC_Available(0) )
		return 0;

	// make sure directories exist
	FileMkdir( "mc0:/SYS-CONF" );
	FileMkdir( "mc0:/SYS-CONF/MYPS2" );

	// just recreate the whole file
	fHandle	= FileOpen( "mc0:/SYS-CONF/MYPS2/CONFIG.INI", O_RDWR | O_CREAT | O_TRUNC );

	if( fHandle.fh < 0 )
		return 0;

	// scr_adjust_x
	sprintf( strLine, "scr_adjust_x=%i\r\n", sysConf.scr_adjust_x );
	FileWrite( fHandle, strLine, strlen(strLine) );

	// scr_adjust_y
	sprintf( strLine, "scr_adjust_y=%i\r\n", sysConf.scr_adjust_y );
	FileWrite( fHandle, strLine, strlen(strLine) );

	// scr_mode
	sprintf( strLine, "scr_mode=%i\r\n", sysConf.scr_mode );
	FileWrite( fHandle, strLine, strlen(strLine) );

	FileClose(fHandle);
	return 1;
}

//
// SysConfDefault - Initializes sysConf variable with default values.
//

void SysConfDefault( void )
{
	memset( &sysConf, 0, sizeof(sysConf) );

	sysConf.scr_adjust_x	= 0;
	sysConf.scr_adjust_y	= 0;
	sysConf.scr_mode		= GR_MODE_AUTO;
}
