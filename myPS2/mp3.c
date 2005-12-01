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

static short	left[TICK]	 __attribute__((aligned (64)));
static short	right[TICK]  __attribute__((aligned (64)));
static int		iSamples;

//
// MP3_Play - Creates the MP3 playback thread.
//
//			  Returns 1 on success, otherwise 0.
//

int MP3_Play( const char *pFileName )
{
	_mp3_playback( pFileName );
	return 1;
}

//////////////////////////////////////////////////////////////

struct buffer {
	unsigned char const *start;
	unsigned long length;
};


//
// _mp3_playback - MP3 thread entry function.
//
//				   Loads file into EE memory, then starts
//				   decoding.
//

void _mp3_playback( const char *pFileName )
{
	FHANDLE fHandle;
	int		nFileSize;
	u8		*pBuffer;

	if( !pFileName )
		return;

	fHandle = FileOpen( pFileName, O_RDONLY );

	if( fHandle.fh < 0 ) {
#ifdef _DEBUG
		printf("MP3_Play : Couldn't open file %s\n", pFilename);
#endif
		return;
	}

	nFileSize = FileSeek( fHandle, 0, SEEK_END );
	FileSeek( fHandle, 0, SEEK_SET );

	if( !nFileSize ) {
#ifdef _DEBUG
		printf("MP3_Play : Filesize is 0\n");
#endif
		FileClose(fHandle);
		return;
	}

	pBuffer = (u8*) malloc( nFileSize );

	if( pBuffer == NULL ) {
#ifdef _DEBUG
		printf("MP3_Play : malloc() failed for pBuffer!\n");
#endif
		return;
	}

	FileRead( fHandle, pBuffer, nFileSize );
	FileClose(fHandle);

	// setup sjPCM stuff
	SjPCM_Init(0);
	SjPCM_Clearbuff();
	SjPCM_Setvol(0x3fff);
	SjPCM_Play();

	// setup callbacks and start decoding
	_mp3_decode( pBuffer, nFileSize );

	// done playing mp3
	SjPCM_Clearbuff();

	free(pBuffer);
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
