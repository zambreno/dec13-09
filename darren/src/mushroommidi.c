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
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"
#include "portmidi.h"

//Sample rate of the soundcard
#define SAMPLE_RATE   (32000)

//Audio buffer size
#define FRAMES_PER_BUFFER  (256)

//Number of Poly Voices per module
#define NUM_POLYVOICES (127)

//Power of wavetable size (wavetable size = 2 ^ POWER)
#define POWER (4)
#define TABLE_SIZE  (1<<POWER)
#define NUM_SECONDS (1)

//Note struct
typedef struct
{
	unsigned short phase;
	int frequency;
	int isActive;
	int note;
}
polyVoice;

//Wave tables - tri is triangle, sq1 is a 50% duty cycle, sq2 is a 25%, nse is a noise
volatile short tri[TABLE_SIZE];
volatile short sq1[TABLE_SIZE];
volatile short sq2[TABLE_SIZE];
volatile short nse[TABLE_SIZE];


//Initialize Polyvoices
void polyVoice_init(polyVoice module[]){
	int i;
	for (i=0;i<NUM_POLYVOICES;i++){
		module[i].phase=0;
		module[i].frequency=0;
		module[i].isActive=0;
		module[i].note=0;
	}
}



//Generate Waves
void wavetablegen(void){

    int i;

    //Divide TABLE_SIZE into four regions
    int quarter = TABLE_SIZE/4;
    int half = TABLE_SIZE/2;
    int three_fourths = 3*quarter;

    //Cycle through the entirety of TABLE_SIZE and generate triangle, and square waves
    //The triangle wave statement takes the current index of the for loop, casts it to a float (to do division), and scales it to do the correct math in triangle wave generation
	for (i=0;i<TABLE_SIZE;i++){
		//First half of the wave
    		if (i<half){
    			tri[i] = -32768 + (((float)i/quarter) * 32768);
    			sq1[i] = -32768;
    			sq2[i] = -32768;
    		}
    		//Third quarter of the wave
    		else if (i<three_fourths){
    			tri[i] = 32767 - ((((float)i-half)/quarter) * 32768);
    			sq1[i] = 32767;
    			sq2[i] = -32768;
    		}
    		//Fourth quarter of the wave
    		else {
    			tri[i] = 32768 - ((((float)i-half)/quarter) * 32768);
    			sq1[i] = 32767;
    			sq2[i] = 32767;
    		}
    		nse[i] = rand() % 32768 - 16384;
    	}
}


//Phase stepsize calculation from frequency
unsigned int stepsize(int freq){
	//Maximum value of phase scale (16^4 in this case)
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
	polyVoice *data = (polyVoice*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    for( i=0; i<framesPerBuffer; i++ )
    {
    	//Lookup the wave value of the current phase (Only uses the top 16-POWER bits of phase, to allow for table sizes smaller than the phase register)
        *out++ = ( (float)sq2[(data->phase)>>(16-POWER)]) / 32768;  
        
        /* Advance Phase */
        data->phase += stepsize(data->frequency);
    }

    return paContinue;
}


/*
 * This routine is called by portaudio when playback is done.
 */
static void StreamFinished( void* userData )
{
   polyVoice *data = (polyVoice *) userData;
}

/*******************************************************************/
int main(void);
int main(void)
{
	wavetablegen();
	polyVoice module1[NUM_POLYVOICES];
	polyVoice_init(module1);


	PaStreamParameters outputParameters;
	PaStream *stream;
	PaError err;

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
              &module1[0]);
	if( err != paNoError ) goto error;

	err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
	if( err != paNoError ) goto error;

	//Start Stream
    	err = Pa_StartStream( stream );
   	if( err != paNoError ) goto error;
	int i;
	for (i=0;i<100;i++){
		module1[0].frequency = 500 + 10*i;
		module1[0].note = 60;
		module1[0].isActive = 1;
		Pa_Sleep(10);
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
