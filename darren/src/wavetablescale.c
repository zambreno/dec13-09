/** @file paex_sine.c
	@ingroup examples_src
	@brief Play a sine wave for several seconds.
	@author Ross Bencina <rossb@audiomulch.com>
    @author Phil Burk <philburk@softsynth.com>
*/
/*
 * $Id: paex_sine.c 1752 2011-09-08 03:21:55Z philburk $
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com/
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */
 
#include <stdio.h>
#include <math.h>
#include "portaudio.h"

//NUM-SECONDS is simply a playback mechanism
#define NUM_SECONDS   (1)

//Sample rate of the soundcard
#define SAMPLE_RATE   (32000)

//Audio buffer size
#define FRAMES_PER_BUFFER  (1024)

//Power of wavetable size (wavetable size = 2 ^ POWER)
#define POWER (4)
#define TABLE_SIZE   ((int)pow(2,POWER))

//Pi, for generation of sine table
#ifndef M_PI
#define M_PI  (3.14159265)
#endif

//Note struct
typedef struct
{
    float sine[TABLE_SIZE];
    unsigned short phase;
    char message[20];
    int frequency;
}
paTestData;





//Phase stepsize calculation from frequency
unsigned int stepsize(int freq){
	//Maximum value of phase scale (2^4 in this case)
	float phasescale = 0xFFFF;
	float step;
	
	//Our equation!
	step = (freq*phasescale)/SAMPLE_RATE;
	return (unsigned short)step;
}



/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->sine[(data->phase)>>(16-POWER)];  /* Advance Phase */
        data->phase += stepsize(data->frequency);
    }
    
    return paContinue;
}


/*
 * This routine is called by portaudio when playback is done.
 */
static void StreamFinished( void* userData )
{
   paTestData *data = (paTestData *) userData;
}

/*******************************************************************/
int main(void);
int main(void)
{
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    paTestData data;
    int i;

    
    /* initialise sinusoidal wavetable */
    for( i=0; i<TABLE_SIZE; i++ )
    {
        data.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. );
    }
    data.phase = 0;
    
    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      goto error;
    }
    outputParameters.channelCount = 1;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;


	//Open Audio Stream
    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              patestCallback,
              &data );
    if( err != paNoError ) goto error;

    sprintf( data.message, "No Message" );
    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    for(i=0;i<8;i++){
    	switch(i){
    		case 0:
    			data.frequency = 261;
    			break;
    		case 1:
    			data.frequency = 293;
    			break;
    		case 2:
    			data.frequency = 329;
    			break;
	    	case 3:
   	 			data.frequency = 349;
   	 			break;
   		 	case 4:
    			data.frequency = 391;
    			break;
   		 	case 5:
    			data.frequency = 440;
    			break;
    		case 6:
    			data.frequency = 493;
    			break;
    		case 7:
    			data.frequency = 523;
    			break; 
    	}   	
    	Pa_Sleep( NUM_SECONDS * 100 );  
    }
    
    
    //Stop stream        
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
	
	
	//Close stream
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
	
	//Terminate Portaudio
    Pa_Terminate();
    
    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
