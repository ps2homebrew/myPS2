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
# File: File I/O
#		This is just a wrapper around fio and fileXio to have one set of  
#		functions for cd and hdd.
#
#		Supports :	- CD/DVD
#					- Hard Disk
#					- Memory Card
#					- USB Mass		
*/

#include <file.h>

//
// FileOpen - Opens a file and returns a handle to it
//

FHANDLE FileOpen( const char *filename, int mode )
{
	FHANDLE handle;
	int		c;
	char	*ptr;

	memset( &handle, -1, sizeof(handle) );

	if(!filename)
		return handle;

	if( (ptr = strchr( filename, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("FileOpen : Invalid Path (ptr = NULL)\n");
#endif
		return handle;
	}

	c = ptr - filename;

	// file is on cd/dvd
	if( !strncmp( filename, "cdfs:", c ) ) {
		CDVD_FlushCache();

		handle.fh = fioOpen( filename, mode );
		handle.dt = DT_CD;
	}
	else if( !strncmp( filename, "pfs0:", c ) ) {
		handle.fh = fileXioOpen( filename, mode, 0 );
		handle.dt = DT_HDD;
	}
	else if( !strncmp( filename, "mc0:", c ) || !strncmp( filename, "mc1:", c ) ) {
		handle.fh = fioOpen( filename, mode );
		handle.dt = DT_MC;
	}
	else if( !strncmp( filename, "mass:", c ) ) {
		handle.fh = fioOpen( filename, mode );
		handle.dt = DT_USB;
	}

	return handle;
}

//
// FileClose - Closes an open file
//

int FileClose( FHANDLE handle )
{
	int ret = 0;

	if( handle.dt == DT_CD ) {
		ret = fioClose( handle.fh );
	}
	else if( handle.dt == DT_HDD ) {
		ret = fileXioClose( handle.fh );
	}
	else if( handle.dt == DT_MC ) {
		ret = fioClose( handle.fh );
	}
	else if( handle.dt == DT_USB ) {
		ret = fioClose( handle.fh );
	}

	return ret;
}

//
// FileRead - Reads from an open file
//

int FileRead( FHANDLE handle, void *buffer, int size )
{
	int ret = 0;

	if( handle.dt == DT_CD ) {
		ret = fioRead( handle.fh, buffer, size );
	}
	else if( handle.dt == DT_HDD ) {
		ret = fileXioRead( handle.fh, buffer, size );
	}
	else if( handle.dt == DT_MC ) {
		ret = fioRead( handle.fh, buffer, size );
	}
	else if( handle.dt == DT_USB ) {
		ret = fioRead( handle.fh, buffer, size );
	}

	return ret;
}

//
// FileWrite - Writes to an open file
//

int FileWrite( FHANDLE handle, void *buffer, int size )
{
	int ret = 0;

	if( handle.dt == DT_CD ) {
		ret = fioWrite( handle.fh, buffer, size );
	}
	else if( handle.dt == DT_HDD ) {
		ret = fileXioWrite( handle.fh, buffer, size );
	}
	else if( handle.dt == DT_MC ) {
		ret = fioWrite( handle.fh, buffer, size );
	}
	else if( handle.dt == DT_USB ) {
		ret = fioWrite( handle.fh, buffer, size );
	}

	return ret;
}

//
// FileSeek - Sets position in an open file
//

int FileSeek( FHANDLE handle, long offset, int whence )
{
	int ret = 0;

	if( handle.dt == DT_CD ) {
		ret = fioLseek( handle.fh, offset, whence );
	}
	else if( handle.dt == DT_HDD ) {
		ret = fileXioLseek( handle.fh, offset, whence );
	}
	else if( handle.dt == DT_MC ) {
		ret = fioLseek( handle.fh, offset, whence );
	}
	else if( handle.dt == DT_USB ) {
		ret = fioLseek( handle.fh, offset, whence );
	}

	return ret;
}

//
// FileGets - Reads characters until num characters have been read
//			  or a newline or eof is reached.
//			  Newline character is included in returned string.
//
//			  The character sequence carriage-return linefeed is
//			  translated into linefeed.
//

char *FileGets( char *string, int num, FHANDLE fh )
{
	char	c;
	int		ret	= 0;
	int		i	= 0;

	while( i < num ) {
		ret = FileRead( fh, &c, 1 );

		if( ret < 1 )
			break;

		string[i] = c;
		i++;

		if( c == '\n' )
			break;
	}

	if( (i >= 2 ) && string[ i - 2 ] == '\r' ) {
		string[ i - 2 ] = '\n';
		string[ i - 1 ] = '\0';
	}
	else {
		string[i] = '\0';
	}

	if( ret < 1 )
		return NULL;

	return string;
}

//
// FileGetc - Reads the next character from an open file
//

int FileGetc( FHANDLE fh )
{
	char c;

	FileRead( fh, &c, 1 );

	return c;
}

//
// FileMkdir - Creates a new directory
//

int FileMkdir( const char *path )
{
	char *ptr;
	int c;

	if( (ptr = strchr( path, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("FileMkdir : Invalid Path (ptr = NULL)\n");
#endif
		return -1;
	}

	c = ptr - path;

	if( !strncmp( path, "pfs0:", c ) ) {
		return fileXioMkdir( path, O_RDWR | O_CREAT );
	}
	else if( !strncmp( path, "mc0:", c ) || !strncmp( path, "mc1:", c ) ) {
		return fioMkdir( path );
	}
	else if( !strncmp( path, "mass:", c ) ) {
		return fioMkdir( path );
	}

	return -1;
}

//
// FileRemove - Deletes a file
//

int FileRemove( const char *file )
{
	char *ptr;
	int c;

	if( (ptr = strchr( file, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("FileRemove : Invalid Path (ptr = NULL)\n");
#endif
		return -1;
	}

	c = ptr - file;

	if( !strncmp( file, "pfs0:", c ) ) {
		return fileXioRemove( file );
	}
	else if( !strncmp( file, "mc0:", c ) || !strncmp( file, "mc1:", c ) ) {
		// there's a bug in the fio stuff that creates a new directory
		// whenever a file is removed with fioRemove.
		c = fioRemove( file );
		fioRmdir( file );

		return c;
	}
	else if( !strncmp( file, "mass:", c ) ) {
		char strDir[256];

		c = fioRemove( file );

		// you MUST call fioRmdir with device number for USB device.
		strcpy( strDir, "mass0" );
		strcat( strDir, ptr );

		// need to remove trailing '/' or fioRmdir won't work
		if( strDir[ strlen(strDir) - 1 ] == '/' )
			strDir[ strlen(strDir) - 1 ] = 0x0;

		fioRmdir( strDir );
		return c;
	}

	return -1;
}

//
// FileRmdir - Removes an empty directory
//

int FileRmdir( const char *path )
{
	char *ptr;
	int c;

	if( (ptr = strchr( path, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("FileRmdir : Invalid Path (ptr = NULL)\n");
#endif
		return -1;
	}

	c = ptr - path;

	if( !strncmp( path, "pfs0:", c ) ) {
		return fileXioRmdir( path );
	}
	else if( !strncmp( path, "mc0:", c ) || !strncmp( path, "mc1:", c ) ) {
		return fioRmdir( path );
	}
	else if( !strncmp( path, "mass:", c ) ) {
		char strDir[256];

		// you MUST call fioRmdir with device number for USB device.
		strcpy( strDir, "mass0" );
		strcat( strDir, ptr );

		// need to remove trailing '/' or fioRmdir won't work
		if( strDir[ strlen(strDir) - 1 ] == '/' )
			strDir[ strlen(strDir) - 1 ] = 0x0;

		return fioRmdir( strDir );
	}

	return -1;
}

//
// FileRename - Renames a file or directory
//

int FileRename( const char *src, const char *dst )
{
	char *ptr;
	int c;

	if( (ptr = strchr( src, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("FileRename : Invalid Path (ptr = NULL)\n");
#endif
		return -1;
	}

	c = ptr - src;

	if( !strncmp( src, "pfs0:", c ) ) 
	{
		return fileXioRename( src, dst );
	}
	else if( !strncmp( src, "mc0:", c ) || !strncmp( src, "mc1:", c ) )
	{
		// Note : mcRename doesn't work
		// TODO : mc renaming not implemented yet
		printf("FileRename : Not implemented for Memory Card!\n");
		return -1;
	}
	else if( !strncmp( src, "mass:", c ) ) 
	{
		// TODO : usb renaming not implemented yet
		printf("FileRename : Not implemented for USB!\n");
		return -1;
	}

	return -1;
}

//
// DirGetContents - Retrieves a list of all items in a directory.
//
// filter can contain a list of whitespace delimited extensions
// such as "jpg bmp gif" to only return files with those
// extensions.
//
// Directories are always returned even if a filter is specified.
// The function returns the number of valid entries in the fileInfo
// array.
//

int DirGetContents( const char *path, const char *filter, fileInfo_t *fileInfo, int maxItems )
{
	int		c, i;
	char	*ptr;
	int		numRead;
	int		index	= 0;

	if( !path || !fileInfo || !maxItems )
		return 0;

	if( (ptr = strchr( path, '/' )) == NULL ) {
#ifdef _DEBUG
		printf("DirGetContents : Invalid Path (ptr = NULL)\n");
#endif
		return 0;
	}

	c = ptr - path;

	// try to read in dir from cd/dvd
	if( !strncmp( path, "cdfs:/", c ) ) 
	{
		// just make sure we are initialized
		CD_Init();

		struct TocEntry *tocEntries = (struct TocEntry*) malloc( sizeof(struct TocEntry) * maxItems );
		
		if( !tocEntries )
		{
#ifdef _DEBUG
			printf("DirGetContents : malloc() failed (tocEntries = NULL)\n");
#endif
			return 0;
		}

		CDVD_FlushCache();
		numRead = CDVD_GetDir( ptr, NULL, CDVD_GET_FILES_AND_DIRS, tocEntries, maxItems, NULL );
		CDVD_Stop();

#ifdef _DEBUG
		printf("CDVD_GetDir read %i items\n", numRead);
#endif

		index	= 0;
		ptr		= NULL;

		if( filter )
			ptr = (char*) malloc( strlen(filter) + 1 );

		for( i = 0; i < numRead; i++ )
		{
			// just in case
			if( index >= maxItems )
				break;

			// check for filters
			c = 1;

			if( filter && !(tocEntries[i].fileProperties & FLAG_DIRECTORY) ) 
			{
				strcpy( ptr, filter );

				c = 0;
				char *token = strtok( ptr, " " );

				while( token ) {
					// found matching extension
					if( CmpFileExtension( tocEntries[i].filename, token ) ) {
						c = 1;
						break;
					}

					token = strtok( NULL, " " );
				}
			}

			if( c == 1 )
			{
				strncpy( fileInfo[index].name, tocEntries[i].filename, sizeof(fileInfo[index].name) );

				fileInfo[index].size	= tocEntries[i].fileSize;
				fileInfo[index].flags	= tocEntries[i].fileProperties;

				index++;
			}
		}

		if( ptr )
			free(ptr);

		free(tocEntries);
	}
	else if( !strncmp( path, "pfs0:/", c ) )
	{
		// try to read in dir from hdd
		int hDir = fileXioDopen( path );
		int nRet;
		iox_dirent_t dirEntry;

		index	= 0;
		ptr		= NULL;

		if( filter )
			ptr = (char*) malloc( strlen(filter) + 1 );

		do {
			if( !(nRet = fileXioDread( hDir, &dirEntry )) )
				break;

			if(!strcmp( dirEntry.name, "." ))
				continue;

			// if browsing partition root, ignore ".."
			if(!strcmp( dirEntry.name, ".." ) && !strcmp( path, "pfs0:/" ))
				continue;

			if( index >= maxItems )
				break;

			if( FIO_S_ISDIR(dirEntry.stat.mode) )
				fileInfo[index].flags = FLAG_DIRECTORY;
			else
				fileInfo[index].flags = 0;

			// check for filters
			c = 1;

			if( filter && !(fileInfo[index].flags & FLAG_DIRECTORY) )
			{
				strcpy( ptr, filter );

				c = 0;
				char *token = strtok( ptr, " " );

				while( token ) {
					// found matching extension
					if( CmpFileExtension( dirEntry.name, token ) ) {
						c = 1;
						break;
					}

					token = strtok( NULL, " " );
				}
			}
			
			if( c == 1 )
			{
				strncpy( fileInfo[index].name, dirEntry.name, sizeof(fileInfo[index].name) );
				fileInfo[index].size = dirEntry.stat.size;

				index++;
			}

		} while( nRet > 0 );

		if( ptr )
			free(ptr);

		fileXioDclose( hDir );
	}
	else if( !strncmp( path, "mc0:/", c ) || !strncmp( path, "mc1:/", c ) )
	{
		// try to read in dir from memory card
		int		nPort;
		char	mcPath[256];
		mcTable mcEntries[MAX_DIR_FILES] __attribute__((aligned(64)));

		if( !strncmp( path, "mc0:/", c ) )
			nPort = 0;
		else
			nPort = 1;

		strcpy( mcPath, ptr );
		strcat( mcPath, "*" );

		mcGetDir( nPort, 0, mcPath, 0, MAX_DIR_FILES, mcEntries );
		mcSync( 0, NULL, &numRead );

		index	= 0;
		ptr		= NULL;

		if( filter )
			ptr = (char*) malloc( strlen(filter) + 1 );

		for( i = 0; i < numRead; i++ )
		{
			if( index >= maxItems )
				break;

			if( !strcmp( mcEntries[i].name, "." ) )
				continue;

			if( mcEntries[i].attrFile & MC_ATTR_SUBDIR )
				fileInfo[index].flags = FLAG_DIRECTORY;
			else
				fileInfo[index].flags = 0;

			// check for filters
			c = 1;

			if( filter && !(mcEntries[i].attrFile & MC_ATTR_SUBDIR) )
			{
				strcpy( ptr, filter );

				c = 0;
				char *token = strtok( ptr, " " );

				while( token ) {
					// found matching extension
					if( CmpFileExtension( mcEntries[i].name, token ) ) {
						c = 1;
						break;
					}

					token = strtok( NULL, " " );
				}
			}

			if( c == 1 )
			{
				strncpy( fileInfo[index].name, mcEntries[i].name, sizeof(fileInfo[index].name) );

				fileInfo[index].size	= mcEntries[i].fileSizeByte;

				index++;
			}
		}

		if( ptr )
			free(ptr);
	}
	else if( !strncmp( path, "mass:/", c ) ) 
	{
		// try to read in dir from USB device
		int nRet;
		fat_dir_record dirEntry;

		// returns number of entries in directory
		nRet = usb_mass_getFirstDirentry( ptr, &dirEntry );

		index	= 0;
		ptr		= NULL;

		if( filter )
			ptr = (char*) malloc( strlen(filter) + 1 );

		// loop through all entries in directory
		while( nRet > 0 ) {

			if(!strcmp( dirEntry.name, "." )) {
				nRet = usb_mass_getNextDirentry(&dirEntry);
				continue;
			}

			// ignore volume label
			if( dirEntry.attr & USB_VOLUME ) {
				nRet = usb_mass_getNextDirentry(&dirEntry);
				continue;
			}

			if( index >= maxItems )
				break;

			if( dirEntry.attr & USB_DIRECTORY )
				fileInfo[index].flags = FLAG_DIRECTORY;
			else
				fileInfo[index].flags = 0;

			// check for filters
			c = 1;

			if( filter && !(fileInfo[index].flags & FLAG_DIRECTORY) )
			{
				strcpy( ptr, filter );

				c = 0;
				char *token = strtok( ptr, " " );

				while( token ) {
					// found matching extension
					if( CmpFileExtension( dirEntry.name, token ) ) {
						c = 1;
						break;
					}

					token = strtok( NULL, " " );
				}
			}
			
			if( c == 1 )
			{
				strncpy( fileInfo[index].name, dirEntry.name, sizeof(fileInfo[index].name) );
				fileInfo[index].size = dirEntry.size;

				index++;
			}

			nRet = usb_mass_getNextDirentry( &dirEntry );
		}

		if( ptr )
			free(ptr);
	}

	return index;
}

//
// CmpFileExtension - Returns 1 if extensions match, otherwise 0.
//
// Comparison is performed case insensitive.
//

int CmpFileExtension( const char *filename, const char *ext )
{
	char *p;

	if( !filename || !ext )
		return 0;

	if( (p = strrchr( filename, '.' )) == NULL ) {
		return 0;
	}

	// skip '.'
	p++;

	if( !stricmp( p, ext ) )
		return 1;

	return 0;
}

//
// StripFileExt - Strips file extension from a filename
//				  Returns pointer to dst string
//

char *StripFileExt( char *dst, const char *src )
{
	int	l;
	char *p; 
	
	if( (p = strrchr( src, '.' )) == NULL ) {
		*dst = 0;
		return dst;
	}

	l = strlen(src) - strlen(p);
	strncpy( dst, src, l );
	*(dst + l) = 0;

	return dst;
}

//
// StripFileSpec - Removes the trailing file name from a path
//				   Returns pointer to dst string
//

char *StripFileSpec( char *dst, const char *src )
{
	int	l;
	char *p; 
	
	if( (p = strrchr( src, '/' )) == NULL ) {
		if( (p = strrchr( src, ':' )) == NULL ) {
			*dst = 0;
			return dst;
		}
	}

	p++;
	l = strlen(src) - strlen(p);
	strncpy( dst, src, l );
	*(dst + l) = 0;

	return dst;
}

//
// DirGetSize - Returns the cumulative size of all files in a directory 
//				and its sub-directories.
//				u64 reserved must be 0.
//

u64 DirGetSize( const char *path, u64 reserved )
{
	int	i, numFiles;
	fileInfo_t *pFileInfo;
	u64	size;
	char new_path[256];

	size = reserved;

	pFileInfo = (fileInfo_t*) malloc( sizeof(fileInfo_t) * MAX_DIR_FILES );
	if( !pFileInfo ) {
#ifdef _DEBUG
		printf("DirGetSize : malloc() failed.\n");
#endif
		return 0;
	}

	numFiles = DirGetContents( path, NULL, pFileInfo, MAX_DIR_FILES );

	for( i = 0; i < numFiles; i++ )
	{
		if( !strcmp( pFileInfo[i].name, "." ) || !strcmp( pFileInfo[i].name, ".." ) )
			continue;

		if( pFileInfo[i].flags & FLAG_DIRECTORY )
		{
			// build new path
			strcpy( new_path, path );
			strcat( new_path, pFileInfo[i].name );
			strcat( new_path, "/" );

			size = DirGetSize( new_path, size );
		}
		else
		{
			size += pFileInfo[i].size;
		}
	}

	free(pFileInfo);

	return size;
}

//
// DirRemove - Recursively deletes a directory and all of its content
//

void DirRemove( const char *path )
{
	int i;
	int numFiles;
	fileInfo_t *pFileInfo;
	char new_path[256];

	pFileInfo = (fileInfo_t*) malloc( sizeof(fileInfo_t) * MAX_DIR_FILES );
	if( !pFileInfo ) {
#ifdef _DEBUG
		printf("DirRemove : malloc() failed.\n");
#endif
		return;
	}

	numFiles = DirGetContents( path, NULL, pFileInfo, MAX_DIR_FILES );

	for( i = 0; i < numFiles; i++ )
	{
		if( !strcmp( pFileInfo[i].name, "." ) || !strcmp( pFileInfo[i].name, ".." ) )
			continue;

		if( pFileInfo[i].flags & FLAG_DIRECTORY )
		{
			// build new path
			strcpy( new_path, path );
			strcat( new_path, pFileInfo[i].name );
			strcat( new_path, "/" );

			DirRemove( new_path );
		}
		else
		{
			strcpy( new_path, path );
			strcat( new_path, pFileInfo[i].name );

			FileRemove( new_path );
		}
	}

	// all files have been removed so we can delete the directory now
	FileRmdir( path );

	free(pFileInfo);
}

//
// DirCreate - Creates new directory
//

void DirCreate( const char *path )
{
	char strBase[256];
	char *ptr;
	int c;

	if( (ptr = strchr( path, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("DirCreate : Invalid Path (ptr = NULL)\n");
#endif
		return;
	}

	// skip ':'
	ptr++;

	// first slash is optional
	if( *ptr == '/' )
		ptr++;

	c = ptr - path;

	strncpy( strBase, path, c );

	while(*ptr) {
		strBase[c++]	= *ptr;
		strBase[c]		= 0;
		
		if( *ptr == '/' )
			FileMkdir( strBase );

		ptr++;	
	}
}
