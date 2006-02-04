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

/*
# myPS2
# Author: ntba2
# Date: 11-14-2005
#
# File: ELF loader code.
#		Based on loader code by Marcus R. Brown.
#
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <elf.h>
#include <file.h>

// loader.elf file
extern u8	*loader_elf;
extern int	size_loader_elf;

//
// CheckELFHeader - Checks for valid ELF Header.
//					Returns 1 if ELF is valid, otherwise 0.
//

int CheckELFHeader( const char *path )
{
	u8				*boot_elf	= (u8 *)0x1800000;
	elf_header_t	*eh			= (elf_header_t *)boot_elf;
	int				size;
	FHANDLE			fHandle;

	fHandle = FileOpen( path, O_RDONLY );
	if( fHandle.fh < 0 )
		return 0;

	size = FileSeek( fHandle, 0, SEEK_END );
	if( !size ) {
		FileClose( fHandle );
		return 0;
	}

	FileSeek( fHandle, 0, SEEK_SET );
	FileRead( fHandle, boot_elf, 52 );
	FileClose( fHandle );

	if( (_lw((u32)&eh->ident) != ELF_MAGIC) || eh->type != 2 )
		return 0;

	return 1;
}

//
// RunLoaderELF -  Loads LOADER.ELF from program memory and
//				   passes args of selected ELF to it.
//
//				   Returns 0 on failure, otherwise aborts
//				   myPS2 and loads new ELF file.
//

int RunLoaderELF( char *filename )
{
	u8				*boot_elf	= (u8 *)&loader_elf;
	elf_header_t	*eh			= (elf_header_t *)boot_elf;
	elf_pheader_t	*eph;
	void			*pdata;
	int				i;
	char			*argv[1];
	char			szMnt[MAX_PATH + 1];
	const char		*pPartName;

	// Load the ELF into RAM
	if( _lw((u32)&eh->ident) != ELF_MAGIC )
		return 0;

	eph = (elf_pheader_t *)(boot_elf + eh->phoff);

	// Scan through the ELF's program headers and copy them
	// into RAM, then zero out any non-loaded regions.
	for( i = 0; i < eh->phnum; i++ )
	{
		if( eph[i].type != ELF_PT_LOAD )
			continue;

		pdata = (void *)( boot_elf + eph[i].offset );
		memcpy( eph[i].vaddr, pdata, eph[i].filesz );

		if( eph[i].memsz > eph[i].filesz )
			memset( eph[i].vaddr + eph[i].filesz, 0, eph[i].memsz - eph[i].filesz );
	}

	// Let's go
	fioExit();

	SifInitRpc(0);
	SifExitRpc();
	FlushCache(0);
	FlushCache(2);

	argv[0] = filename;

	if( !strncmp( filename, "pfs", 3 ) ) {
		i = strcspn( filename, ":" );

		strncpy( szMnt, filename, i + 1);

		if( !(pPartName = HDD_GetPartition(szMnt)) )
			return 0;

		argv[1] = (char*) pPartName;
	}
	else {
		argv[1] = "";
	}

	ExecPS2( (void *)eh->entry, 0, 2, argv );

	return 1;
}

