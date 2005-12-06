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
# Date: 10-18-2005
#
# File: File I/O Header
#		This is just a wrapper for fio and fileXio to have one set of  
#		functions for cd and HDD.
#		
*/

#ifndef _FILE_H
#define _FILE_H

#include <tamtypes.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include <fileio.h>
#include <fileXio_rpc.h>
#include <include/cdvd_rpc.h>
#include <libusb/mass_rpc.h>
#include <libmc.h>
#include <misc.h>

typedef struct {
	int fh;			// file handle
	int dt;			// device type
} FHANDLE;

typedef enum {
	DT_CD,
	DT_HDD,
	DT_MC,
	DT_USB
} deviceType_e;

typedef struct {
	char	name[256];
	long	size;
	int		flags;
} fileInfo_t;

// File- function declarations
FHANDLE FileOpen( const char *filename, int mode );
int FileClose( FHANDLE handle );
int FileRead( FHANDLE handle, void *buffer, int size );
int FileWrite( FHANDLE handle, void *buffer, int size );
int FileSeek( FHANDLE handle, long offset, int whence );
char *FileGets( char *string, int num, FHANDLE fh );
int FileGetc( FHANDLE fh );
int FileMkdir( const char *path );
int FileRemove( const char *file );
int FileRmdir( const char *path );
int FileRename( const char *src, const char *dst );


#define MAX_DIR_FILES 1024

#define FLAG_DIRECTORY	0x00000002

// hack
// used by dirview control.
// this really doesn't belong in the flags field of the
// fileInfo_t structure.
#define FLAG_MARKED		0x10000000

// other
int CmpFileExtension( const char *filename, const char *ext );
char *StripFileExt( char *dst, const char *src );
char *StripFileSpec( char *dst, const char *src );

// some useful dir functions
int DirGetContents( const char *path, const char *filter, fileInfo_t *fileInfo, int maxItems );
u64 DirGetSize( const char *path, u64 reserved );
void DirRemove( const char *path );
void DirCreate( const char *path );

#endif
