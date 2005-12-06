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

#include <stdio.h>
#include <file.h>
#include <gr.h>
#include <scheduler.h>

#include <mp3.h>

#define TICK	800 * 3

//
// Global variables
//

// SjPCM 
static short		left[TICK]	 __attribute__((aligned (64)));
static short		right[TICK]  __attribute__((aligned (64)));
static int			iSamples;

static listEntry_t				*playList	= NULL;
static volatile  listEntry_t	*current	= NULL;

static volatile int	mp3_inited				= 0;
static volatile int	mp3_cancel_playback		= 0;
static volatile int	mp3_status				= MP3_STOPPED;
static volatile int	mp3_volume				= 0x3FFF;
static volatile int	mp3_loop				= 0;
static volatile int	mp3_tracklength			= 0;
static volatile int mp3_curtime				= 0;

static s32			mp3_wait_sema;
static s32			mp3_cancel_sema;

//
// MP3_Play - Creates the MP3 playback thread.
//			  If pFileName is NULL the current track or playlist
//			  is re-played.
//
//			  Returns 1 on success, otherwise 0.
//

int MP3_Play( const char *pFileName )
{
	listEntry_t *pEntry;

	if( !mp3_inited && (_mp3_init() < 0 ) )
		return 0;

	// make sure current playback thread is terminated before
	// creating a new one
	if( MP3_GetStatus() != MP3_STOPPED )
		MP3_Stop();

	if( pFileName )
	{
		// free old playlist
		while( playList ) {
			pEntry		= playList;
			playList	= playList->next;

			if( pEntry->filename )
				free( pEntry->filename );

			free( pEntry );
		}

		playList = NULL;

		// it's a m3u playlist
		if( CmpFileExtension( pFileName, "m3u" ) )
		{
			FHANDLE		fHandle;
			char		strLine[1024], strPath[256];
			listEntry_t	*last = NULL;
			char		*pStr;

			fHandle = FileOpen( pFileName, O_RDONLY );

			if( fHandle.fh < 0 ) {
#ifdef _DEBUG
				printf("MP3_Play : Couldn't open playlist %s\n", pFileName);
#endif
				return 0;
			}

			// get the directory path
			StripFileSpec( strPath, pFileName );

			while( FileGets( strLine, sizeof(strLine), fHandle ) )
			{
				// skip comments and empty lines
				if( strLine[0] == '#' || strLine[0] == '\n' )
					continue;

				// remove line end and trailing whitespaces from filename
				pStr = strLine + strlen(strLine) - 1;

				while( *pStr == ' ' || *pStr == '\t' || *pStr == '\n') {
					*pStr = 0;
					pStr--;
				}

				pEntry = (listEntry_t*) malloc( sizeof(listEntry_t) );
				if( pEntry == NULL ) {
#ifdef _DEBUG
					printf("MP3_Play: malloc() failed for pEntry!\n");
#endif
					return 0;
				}

				pEntry->filename = (char*) malloc( strlen(strPath) + strlen(strLine) + 1 );
				if( pEntry->filename == NULL ) {
#ifdef _DEBUG
					printf("MP3_Play: malloc() failed for pEntry->filename!\n");
#endif
					return 0;
				}

				sprintf( pEntry->filename, "%s%s", strPath, strLine );

				pEntry->next	= NULL;
				pEntry->prev	= NULL;

				if( last ) {
					pEntry->prev	= last;
					last->next		= pEntry;
				}

				last = pEntry;

				if( playList == NULL )
					playList = pEntry;
			}

			FileClose(fHandle);
		}
		else
		{
			// single mp3 file
			playList = (listEntry_t*) malloc( sizeof(listEntry_t) );
			if( playList == NULL ) {
#ifdef _DEBUG
				printf("MP3_Play: malloc() failed for playList!\n");
#endif
				return 0;
			}

			playList->filename = (char*) malloc( strlen(pFileName) + 1 );		
			if( playList->filename == NULL ) {
#ifdef _DEBUG
				printf("MP3_Play: malloc() failed for playList->filename!\n");
#endif
				free( playList );
				playList = NULL;
				return 0;
			}

			strcpy( playList->filename, pFileName );
			playList->next = playList->prev = NULL;
		}
	}

	// start mp3 playback thread
	Scheduler_BeginThread( _mp3_playback );

	return 1;
}

//
// MP3_Stop - Stops playback of MP3 and terminates
//			  mp3 thread.
//

void MP3_Stop( void )
{
	if( !mp3_inited && (_mp3_init() < 0 ) )
		return;

	if( MP3_GetStatus() == MP3_STOPPED )
		return;

	WaitSema( mp3_wait_sema );

	// signal cancellation
	mp3_cancel_playback	= 1;

	// wait until playback thread has shut itself down
	WaitSema( mp3_cancel_sema );
	mp3_cancel_playback	= 0;

	SignalSema( mp3_wait_sema );
}

//
// MP3_GetStatus - Retrieves current playback status
//

int MP3_GetStatus( void )
{
	return mp3_status;
}

//
// MP3_SetVolume - Sets the playback volume.
//				   nVolume should be between 0x0 and 0x3FFF.
//

void MP3_SetVolume( int nVolume )
{
	if( !mp3_inited && (_mp3_init() < 0 ) )
		return;

	if( nVolume < 0 )
		nVolume = 0;

	if( nVolume > 0x3FFF )
		nVolume = 0x3FFF;

	WaitSema( mp3_wait_sema );

	// can't call this from the UI thread.
	// The MP3 thread will adjust the volume.
//	SjPCM_Setvol( nVolume );

	mp3_volume = nVolume;

	SignalSema( mp3_wait_sema );
}

//
// MP3_GetVolume - Retrieves the playback volume
//

int MP3_GetVolume( void )
{
	return mp3_volume;
}

//
// MP3_SetLooping - Enables or disables looping of current
//					track or playlist
//

void MP3_SetLooping( int nValue )
{
	if( !mp3_inited && (_mp3_init() < 0 ) )
		return;

	WaitSema( mp3_wait_sema );
	mp3_loop = nValue ? 1 : 0;
	SignalSema( mp3_wait_sema );
}

//
// MP3_GetLooping - Retrieves track looping status
//

int MP3_GetLooping( void )
{
	return mp3_loop;
}

//
// MP3_GetTrackName - Returns the filename of the current track
//

const char *MP3_GetTrackName( void )
{
	const char *pStr;

	if( MP3_GetStatus() == MP3_STOPPED )
		return NULL;

	if( (!current) || (!current->filename) )
		return NULL;

	if( (pStr = strrchr( current->filename, '/' )) )
		return ++pStr;

	if( (pStr = strrchr( current->filename, ':' )) )
		return ++pStr;

	// shouldn't happen but if still here just return entire path
	return current->filename;
}

//
// MP3_GetTrackLength - Returns length of current track in seconds
//

int MP3_GetTrackLength( void )
{
	if( MP3_GetStatus() == MP3_STOPPED )
		return 0;

	return mp3_tracklength;
}

//
// MP3_GetCurrentTime - Returns current play time.
//						Ranges from 0 to mp3_tracklength.
//						This can get a little inaccurate over time.
//

int MP3_GetCurrentTime( void )
{
	if( MP3_GetStatus() == MP3_STOPPED )
		return 0;

	return mp3_curtime;
}

//
// MP3_SetPause - Pauses MP3 playback.
//				  nPause should be either 1 or 0.
//

void MP3_SetPause( int nPause )
{
	if( MP3_GetStatus() == MP3_STOPPED )
		return;

	WaitSema( mp3_wait_sema );
	mp3_status = nPause ? MP3_PAUSED : MP3_PLAYING;
	SignalSema( mp3_wait_sema );
}

//
// MP3_NextTrack - Skips current track and plays next track in
//				   the playlist. If there is no next track the
//				   function does nothing.
//

void MP3_NextTrack( void )
{
	if( !current || !current->next )
		return;

	// stop current playback thread
	MP3_Stop();

	// it's now safe to advance current pointer to next track
	// in the playlist
	current = current->next;

	// start new playback thread
	MP3_Play(NULL);
}


//
// MP3_PrevTrack - Skips current track and plays previous track
//				   in the playlist. If there is no prev track the
//				   function does nothing.
//

void MP3_PrevTrack( void )
{
	if( !current || !current->prev )
		return;

	// stop current playback thread
	MP3_Stop();

	// it's now safe to move back current pointer to prev track
	// in the playlist
	current = current->prev;

	// start new playback thread
	MP3_Play(NULL);
}

//////////////////////////////////////////////////////////////

struct buffer {
	unsigned char const *start;
	unsigned long length;
};

//
// _mp3_init - Initializes semaphores and globals
//

int _mp3_init( void )
{
	ee_sema_t sema;

	sema.init_count	= 1;
	sema.max_count	= 1;
	sema.option		= 0;

	if( (mp3_wait_sema = CreateSema(&sema)) < 0 )
		return 0;

	memset( &sema, 0, sizeof(sema) );

	sema.init_count	= 0;
	sema.max_count	= 1;
	sema.option		= 0;

	if( (mp3_cancel_sema = CreateSema(&sema)) < 0 )
		return 0;

	mp3_cancel_playback	= 0;
	mp3_status			= MP3_STOPPED;
	mp3_volume			= 0x3FFF;
	mp3_loop			= 0;
	mp3_tracklength		= 0;
	mp3_curtime			= 0;

	mp3_inited			= 1;
	return 1;
}


//
// _mp3_playback - MP3 thread entry function.
//
//				   Loads file into EE memory, then starts
//				   decoding.
//

void _mp3_playback( void )
{
	FHANDLE fHandle;
	int		nFileSize;
	u8		*pBuffer;
	int		nBitRate;
	u8		nID3Flags;
	u32		nID3Size;
	int		nIndex;

	if( !current )
		current = playList;

	while( current )
	{
		if( !current->filename )
			break;

		fHandle = FileOpen( current->filename, O_RDONLY );

		if( fHandle.fh < 0 ) {
#ifdef _DEBUG
			printf("MP3_Play : Couldn't open file %s\n", current->filename);
#endif
			break;
		}

		nFileSize = FileSeek( fHandle, 0, SEEK_END );
		FileSeek( fHandle, 0, SEEK_SET );

		if( !nFileSize ) {
#ifdef _DEBUG
			printf("MP3_Play : Filesize is 0\n");
#endif
			FileClose(fHandle);
			break;
		}

		pBuffer = (u8*) malloc( nFileSize );

		if( pBuffer == NULL ) {
#ifdef _DEBUG
			printf("MP3_Play : malloc() failed for pBuffer!\n");
#endif
			break;
		}

		FileRead( fHandle, pBuffer, nFileSize );
		FileClose(fHandle);

		// check for ID3V1 tags
		nIndex = nFileSize - 128;

		if( pBuffer[nIndex++] == 'T' && pBuffer[nIndex++] == 'A' && pBuffer[nIndex++] == 'G' ) {
#ifdef _DEBUG
			printf("MP3_Play: Stripping ID3V1 tags\n");
#endif
			// strip ID3V1 tags
			nFileSize -= 128;
		}

		// check for ID3V2 tags
		nIndex = 0;

		if( pBuffer[nIndex++] == 'I' && pBuffer[nIndex++] == 'D' && pBuffer[nIndex++] == '3' ) {
#ifdef _DEBUG
			printf("MP3_Play: Stripping ID3V2 tags\n");
#endif
			// skip the version (2 bytes)
			nIndex += 2;

			// read ID3V2 flags (1 byte)
			nID3Flags = pBuffer[nIndex++];

			// read ID3V2 size (big endian)
			nID3Size = ((int)pBuffer[nIndex] << 24) + ((int)pBuffer[nIndex + 1] << 16) + ((int) pBuffer[nIndex + 2] << 8) + pBuffer[nIndex + 3];

			// add the 10 bytes of this header
			nID3Size += 10;

			// check for optional footer (copy of 10 byte header)
			if( nID3Flags & MP3_ID3V2_FOOTER )
				nID3Size += 10;

			nFileSize -= nID3Size;
		}
		else {
			// MP3 doesn't have any ID3V2 tags
			nID3Size = 0;
		}

		// figure out bitrate
		nBitRate = _mp3_getbitrate_mem( pBuffer, nFileSize );

		// setup sjPCM stuff
		SjPCM_Init(0);
		SjPCM_Clearbuff();
		SjPCM_Setvol(mp3_volume);
		SjPCM_Play();

		// calculate track length
		mp3_tracklength = nBitRate > 0 ? (nFileSize / (nBitRate * 1024 / 8)) : 0;

		// set status to playing
		mp3_status = MP3_PLAYING;

		// setup callbacks and start decoding
		_mp3_decode( pBuffer + nID3Size, nFileSize );

		// done playing mp3
		SjPCM_Pause();
		SjPCM_Quit();
		SjPCM_Clearbuff();

		free(pBuffer);

		mp3_status	= MP3_STOPPED;

		// reset play time to 0
		mp3_curtime = 0;

		if( mp3_cancel_playback )
			break;

		// go on with next track in playlist unless track looping is enabled
		if( mp3_loop == 0 )
			current = current->next;
	}

	Scheduler_EndThread();

	// other thread can continue
	if( mp3_cancel_playback )
		SignalSema( mp3_cancel_sema );

	return;
}

//
// _mp3_decode - Called by MP3 playback thread to setup playback
//				 callbacks and then start the libmad decoder.
//

int _mp3_decode( unsigned char const *start, unsigned long length )
{
	struct buffer buffer;
	struct mad_decoder decoder;
	int result;

	// initialize our private message structure
	buffer.start  = start;
	buffer.length = length;

	// configure input, output, and error callback functions
	mad_decoder_init( &decoder, &buffer, _mp3_input, 0, 0, _mp3_output, _mp3_error, 0 );

	// start decoding
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	// release the decoder
	mad_decoder_finish(&decoder);

	return result;
}

//
// _mp3_input - This is the input callback. The purpose of this
//				callback is to (re)fill the stream buffer which
//				is to be decoded.
//

enum mad_flow _mp3_input( void *data, struct mad_stream *stream )
{
	struct buffer *buffer = data;

	if(!buffer->length)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, buffer->start, buffer->length);

	buffer->length = 0;
	
	return MAD_FLOW_CONTINUE;
}

//
// _mp3_output - This is the output callback function. It is called
//				 after each frame of MPEG audio data has been completely
//				 decoded.
//

enum mad_flow _mp3_output( void *data, struct mad_header const *header, struct mad_pcm *pcm )
{
	int							i;
	short						nSample;

	static struct audio_dither	ditherLeft;
	static struct audio_dither	ditherRight;

	static int					oldRate;
	static ls_sample_type_t		inType;
	static ls_sample_type_t		outType;

	static ls_resampler_t		*pResampler;
	static void					*pState;

	unsigned int				nSizeSample;
	unsigned int				nSizeResample;

	unsigned short				*pSampleBuf;
	unsigned short				*pResampleBuf;

	static int	lastVolume		= 0x3FFF;
	static u64	nTimer;

	// received request to shut playback thread down
	if( mp3_cancel_playback )
		return MAD_FLOW_STOP;

	// check for pause
	if( mp3_status == MP3_PAUSED )
	{
		// turn down playback volume
		SjPCM_Pause();
		lastVolume = 0;

		// should rather do this with SleepThread or a sema
		while( mp3_status == MP3_PAUSED ) {
			// still have to check for this when paused
			if( mp3_cancel_playback )
				return MAD_FLOW_STOP;

			Scheduler_YieldThread();
		}
	}

	// see MP3_SetVolume
	if( lastVolume != mp3_volume )
	{
		SjPCM_Setvol(mp3_volume);
		lastVolume = mp3_volume;
	}

	if( (nTimer + 1000) <= tnTimeMsec() ) {
		mp3_curtime++;

		nTimer = tnTimeMsec();
	}

	inType.channels		= 2;
	inType.bits			= 16;
	inType.enc			= LS_SIGNED;
	inType.be			= 0;

	outType.channels	= 2;
	outType.bits		= 16;
	outType.enc			= LS_SIGNED;
	outType.be			= 0;
	outType.rate		= 48000;

	nSizeSample = pcm->length * sizeof(unsigned short) * 2;

	pSampleBuf	= (unsigned short*) malloc( nSizeSample );

	if( pSampleBuf == NULL ) {
#ifdef _DEBUG
		printf("_mp3_output : malloc() failed for pSampleBuf!\n");
#endif
		return MAD_FLOW_BREAK;
	}

	oldRate		= inType.rate;
	inType.rate	= pcm->samplerate;

	// store in samplebuffer
	for( i = 0; i < pcm->length; i++ )
	{
		nSample = audio_linear_dither( 16, pcm->samples[0][i], &ditherLeft );
		pSampleBuf[ i * 2 + 0 ] = nSample;

		if( pcm->channels == 2 )
			nSample = audio_linear_dither( 16, pcm->samples[1][i], &ditherRight );

		pSampleBuf[ i * 2 + 1 ] = nSample;
	}

	// resample to 48000
	if( oldRate != inType.rate )
	{
		if( pResampler )
			pResampler->term(pState);

		pResampler = ls_get_resampler( inType, outType, LS_BEST );

		if( pResampler == NULL )
		{
#ifdef _DEBUG
			printf("_mp3_output : ls_get_resampler returned NULL!\n");
#endif
			return MAD_FLOW_BREAK;
		}

		pState = pResampler->init( inType, outType );
	}

	nSizeResample	= ls_resampled_size( inType, outType, nSizeSample );
	pResampleBuf	= (unsigned short*) malloc( nSizeResample );

	pResampler->resample( pState, pSampleBuf, pResampleBuf, nSizeSample, nSizeResample );

	free(pSampleBuf);

	for( i = 0; i < nSizeResample / 4; i++ )
	{
		left[ iSamples ]	= pResampleBuf[ i * 2 + 0 ];
		right[ iSamples ]	= pResampleBuf[ i * 2 + 1 ];

		if( ++iSamples == TICK )
		{
			while( SjPCM_Available() < TICK )
				Scheduler_YieldThread();

			SjPCM_Enqueue(left, right, TICK, 0);
			GR_WaitForVSync(); // FIXME
			iSamples = 0;
		}
	}

	free(pResampleBuf);

	return MAD_FLOW_CONTINUE;
}

//
// _mp3_error - Error callback function. This is called whenever a
//				decoding error occurs.
//

enum mad_flow _mp3_error( void *data, struct mad_stream *stream, struct mad_frame *frame )
{

#ifdef _DEBUG
	struct buffer *buffer = data;

	printf( "decoding error 0x%04x (%s) at byte offset %u\n",
			 stream->error, mad_stream_errorstr(stream),
			 stream->this_frame - buffer->start);
#endif

	return MAD_FLOW_CONTINUE;
}

// helper functions
//

//
// _mp3_getbitrate - Attempts to find the bitrate of an MP3 file.
//
//					 Assumes MP3 has a constant bitrate 
//					 (will not work with variable bitrates).
//
//					 See http://www.dv.co.yu/mpgscript/mpeghdr.htm
//
//					 Returns bitrate on success, otherwise -1.
//

int _mp3_getbitrate( const char *pFileName )
{
	FHANDLE			fHandle;
	int				b;
	unsigned char	swap[4];
	unsigned int	nBitRate = -1;
	unsigned int	nHeader;
	unsigned int	nMPEGVersion;
	unsigned int	nLayerDesc;
	unsigned int	nBitrateIndex;

	unsigned int	nBitrates[MP3_NUM_MPEG_VERSION][MP3_NUM_LAYERS][MP3_NUM_BITRATES] =
	{
		// MPEG Audio Version 1
		{
			// Layer 3 Bitrates
			{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 },

			// Layer 2 Bitrates
			{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },

			// Layer 1 Bitrates
			{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 }
		},

		// MPEG Audio Version 2
		{
			// Layer 3 Bitrates
			{ 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 },

			// Layer 2 Bitrates
			{ 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 },

			// Layer 1 Bitrates
			{ 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256 }
		}

	};

	fHandle = FileOpen( pFileName, O_RDONLY );
	if( fHandle.fh < 0 ) {
#ifdef _DEBUG
		printf("_mp3_getbitrate: Couldn't open file %s\n", pFileName);
#endif
		return -1;
	}

	// find the first audio frame header
	while( (b = FileGetc(fHandle)) != EOF ) {
		if( b == 0xFF )
			break;
	}

	// couldn't find anything
	if( b == EOF ) {
#ifdef _DEBUG
		printf("_mp3_getbitrate: Could not find audio frame header\n");
#endif
		FileClose(fHandle);
		return -1;
	}

	FileSeek( fHandle, -1, SEEK_CUR );
	FileRead( fHandle, &nHeader, sizeof(nHeader) );
	FileClose( fHandle );

	// header is big endian so we need to reverse the byte order
	swap[0] = nHeader & 255;
	swap[1] = (nHeader >> 8) & 255;
	swap[2] = (nHeader >> 16) & 255;
	swap[3] = (nHeader >> 24) & 255;

	nHeader = ((int)swap[0] << 24) + ((int)swap[1] << 16) + ((int)swap[2] << 8) + swap[3];

	// check for valid frame sync (bits 31-21 must be set)
	if( (nHeader & MP3_FRAME_SYNC) != MP3_FRAME_SYNC ) {
#ifdef _DEBUG
		printf("_mp3_getbitrate: Invalid frame sync in audio frame header\n");
#endif
		return -1;
	}

	// read out the mpeg version
	nMPEGVersion	= (nHeader & MP3_MPEG_VERSION) >> MP3_MPEG_VERSION_OFFSET;

	// read out the layer description
	nLayerDesc		= (nHeader & MP3_LAYER_DESC) >> MP3_LAYER_DESC_OFFSET;

	// read out the bitrate index
	nBitrateIndex	= (nHeader & MP3_BITRATE_INDEX) >> MP3_BITRATE_INDEX_OFFSET;

	// MPEG version 2 and version 2.5 use the same bitrates
	nBitRate		= nBitrates[ nMPEGVersion == MP3_MPEG_VERSION_1 ? 0 : 1 ]
							   [ nLayerDesc - 1 ]
							   [ nBitrateIndex  ];

	return nBitRate;

}

//
// _mp3_getbitrate_mem - Does the same as _mp3_getbitrate, just
//						 reads from a memory buffer not a file
//						 stream.
//

int _mp3_getbitrate_mem( const u8 *pBuffer, int nBufSize )
{
	int				i;
	unsigned int	nBitRate = -1;
	unsigned int	nHeader;
	unsigned int	nMPEGVersion;
	unsigned int	nLayerDesc;
	unsigned int	nBitrateIndex;

	unsigned int	nBitrates[MP3_NUM_MPEG_VERSION][MP3_NUM_LAYERS][MP3_NUM_BITRATES] =
	{
		// MPEG Audio Version 1
		{
			// Layer 3 Bitrates
			{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 },

			// Layer 2 Bitrates
			{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },

			// Layer 1 Bitrates
			{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 }
		},

		// MPEG Audio Version 2
		{
			// Layer 3 Bitrates
			{ 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 },

			// Layer 2 Bitrates
			{ 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 },

			// Layer 1 Bitrates
			{ 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256 }
		}

	};

	if( !pBuffer )
		return -1;

	// find the first audio frame header
	for( i = 0; i < nBufSize; i++ ) {
		if( pBuffer[i] == 0xFF )
			break;
	}

	// couldn't find anything
	if( i == nBufSize ) {
#ifdef _DEBUG
		printf("_mp3_getbitrate_mem: Could not find audio frame header\n");
#endif
		return -1;
	}

	nHeader = ((int)pBuffer[i] << 24) + ((int)pBuffer[i + 1] << 16) + ((int)pBuffer[i + 2] << 8) + pBuffer[i + 3];

	// check for valid frame sync (bits 31-21 must be set)
	if( (nHeader & MP3_FRAME_SYNC) != MP3_FRAME_SYNC ) {
#ifdef _DEBUG
		printf("_mp3_getbitrate: Invalid frame sync in audio frame header\n");
#endif
		return -1;
	}

	// read out the mpeg version
	nMPEGVersion	= (nHeader & MP3_MPEG_VERSION) >> MP3_MPEG_VERSION_OFFSET;

	// read out the layer description
	nLayerDesc		= (nHeader & MP3_LAYER_DESC) >> MP3_LAYER_DESC_OFFSET;

	// read out the bitrate index
	nBitrateIndex	= (nHeader & MP3_BITRATE_INDEX) >> MP3_BITRATE_INDEX_OFFSET;

	// MPEG version 2 and version 2.5 use the same bitrates
	nBitRate		= nBitrates[ nMPEGVersion == MP3_MPEG_VERSION_1 ? 0 : 1 ]
							   [ nLayerDesc - 1 ]
							   [ nBitrateIndex  ];

	return nBitRate;

}

inline unsigned long prng( unsigned long state )
{
	return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

inline short audio_linear_dither( unsigned int bits, mad_fixed_t sample, struct audio_dither *dither )
{
	unsigned int scalebits;
	mad_fixed_t output, mask, random;

	enum {
		MIN = -MAD_F_ONE,
		MAX =  MAD_F_ONE - 1
	};

	// noise shape
	sample += dither->error[0] - dither->error[1] + dither->error[2];

	dither->error[2] = dither->error[1];
	dither->error[1] = dither->error[0] / 2;

	// bias
	output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

	scalebits = MAD_F_FRACBITS + 1 - bits;
	mask = (1L << scalebits) - 1;

	// dither
	random  = prng(dither->random);
	output += (random & mask) - (dither->random & mask);

	dither->random = random;

	// clip
	if(output > MAX)  {
		output = MAX;
		
		if (sample > MAX)
			sample = MAX;
	}
	else if(output < MIN)  {
		output = MIN;
		
		if (sample < MIN)
			sample = MIN;
	}

	// quantize
	output &= ~mask;

	// error feedback
	dither->error[0] = sample - output;

	// scale
	return output >> scalebits;
}
