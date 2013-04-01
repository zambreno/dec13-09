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

//Sample rate of the soundcard
#define SAMPLE_RATE   (32000)

//Audio buffer size
#define FRAMES_PER_BUFFER  (256)

//Power of wavetable size (wavetable size = 2 ^ POWER)
#define POWER (4)
#define TABLE_SIZE  (1<<POWER)
#define NUM_SECONDS (1)

//Note struct
typedef struct
{
    unsigned short phase;
    int frequency;
}
polyVoice;


//Wave tables - tri is triangle, sq1 is a 50% duty cycle, sq2 is a 25%, nse is a noise
volatile short tri[TABLE_SIZE];
volatile short sq1[TABLE_SIZE];
volatile short sq2[TABLE_SIZE];
volatile short nse[TABLE_SIZE];

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
	
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    polyVoice data;
    int i;
	
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

    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;


	int g0 = 2*196;
	int ab0 = 2*208;
	int bb0 = 2*233;
	int c1 = 2*261;
	int d1 = 2*294;
	int eb1 = 2*311;
	int e1 = 2*329;
	int f1 = 2*349;
	int g1 = 2*391;
	int ab1 = 2*415;
	int bb1 = 2*466;
	int c2 = 2*523;
	int d2 = 2*587;
	int eb2 = 2*622;
	int e2 = 2*659;
	int f2 = 2*698;
	int g2 = 2*784;
	int ab2 = 2*830;
	int bb2 = 2*932;

	for(i=0;i<26;i++){
    	switch(i){
    		case 0:
    			data.frequency = c1;
    			break;
    		case 1:
    			data.frequency = g0;
    			break;
    		case 2:
    			data.frequency = c1;
    			break;
	    	case 3:
   	 			data.frequency = e1;
   	 			break;
   		 	case 4:
    			data.frequency = g1;
    			break;
   		 	case 5:
    			data.frequency = c2;
    			break;
    		case 6:
    			data.frequency = g1;
    			break;
    		case 7:
    			data.frequency = ab0;
    			break; 
    		case 8:
    			data.frequency = c1;
    			break;
    		case 9:
    			data.frequency = eb1;
    			break;
    		case 10:
    			data.frequency = ab1;
    			break;
	    	case 11:
   	 			data.frequency = eb1;
   	 			break;
   		 	case 12:
    			data.frequency = ab1;
    			break;
   		 	case 13:
    			data.frequency = c2;
    			break;
    		case 14:
    			data.frequency = eb2;
    			break;
    		case 15:
    			data.frequency = ab2;
    			break;
    		case 16:
    			data.frequency = eb2;
    			break;
    		case 17:
    			data.frequency = bb0;
    			break;
    		case 18:
    			data.frequency = d1;
    			break;
	    	case 19:
   	 			data.frequency = f1;
   	 			break;
   		 	case 20:
    			data.frequency = bb1;
    			break;
   		 	case 21:
    			data.frequency = f1;
    			break;
    		case 22:
    			data.frequency = bb1;
    			break;
    		case 23:
    			data.frequency = d2;
    			break; 
    		case 24:
    			data.frequency = f2;
    			break;
    		case 25:
    			data.frequency = bb2;
    			break;           			
    	} 
    	//printf("%d",i);  	
    	Pa_Sleep( NUM_SECONDS * 30 );  
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
