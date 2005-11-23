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
# File: System Config Header file
#
*/

#ifndef _SYSCONF_H
#define _SYSCONF_H

typedef struct {

	// screen
	int		scr_adjust_x;
	int		scr_adjust_y;
	int		scr_mode;

	// Thumbnail Caching
	int		tbnCaching;
	char	tbnCachePath[256];

} sysConf_t;

// so it can be accessed from everywhere
extern sysConf_t sysConf;

int SysConfLoad( void );
int SysConfSave( void );
void SysConfDefault( void );

#endif
