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
# Date: 11-29-2005
#
# File: MP3 playback
#
*/

#ifndef _MP3_H
#define	_MP3_H

#include <include/sample.h>
#include <libsjpcm/sjpcm.h>
#include <libmad/include/mad.h>

//
// The MP3_* functions can be called from the UI to control
// MP3 playback.
//

int MP3_Play( const char *pFileName );


// these should only be called from the mp3 playback thread
//

void _mp3_playback( const char *pFileName );
int _mp3_decode( unsigned char const *start, unsigned long length );
enum mad_flow _mp3_input( void *data, struct mad_stream *stream );
enum mad_flow _mp3_output( void *data, struct mad_header const *header, struct mad_pcm *pcm );
enum mad_flow _mp3_error( void *data, struct mad_stream *stream, struct mad_frame *frame );

struct audio_dither  {
	mad_fixed_t error[3];
	mad_fixed_t random;
};

inline unsigned long prng( unsigned long state );
inline short audio_linear_dither( unsigned int bits, mad_fixed_t sample, struct audio_dither *dither );

#endif
