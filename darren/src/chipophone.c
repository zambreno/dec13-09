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
#define FRAMES_PER_BUFFER  (128)

//Number of Poly Voices per module
#define NUM_POLYVOICES (127)

//Power of wavetable size (wavetable size = 2 ^ POWER)
#define POWER (4)
#define TABLE_SIZE  (1<<POWER)
#define PHASESCALE ((1<<17)-1)
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

polyVoice module1[NUM_POLYVOICES];

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

//Midi Note to Frequency
double MtoF(int note){
	return pow(2,(((double)note - 69)/12.0))*440.0;
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
    			tri[i] = -16384 + (((float)i/quarter) * 16384);
    			sq1[i] = -16384;
    			sq2[i] = -16384;
    		}
    		//Third quarter of the wave
    		else if (i<three_fourths){
    			tri[i] = 16383 - ((((float)i-half)/quarter) * 16384);
    			sq1[i] = 16383;
    			sq2[i] = -16384;
    		}
    		//Fourth quarter of the wave
    		else {
    			tri[i] = 16384 - ((((float)i-half)/quarter) * 16384);
    			sq1[i] = 16383;
    			sq2[i] = 16383;
    		}
    		nse[i] = rand() % 16384 - 16384;
    	}
}


//Phase stepsize calculation from frequency
unsigned int stepsize(int freq){
	//Maximum value of phase scale (16^4 in this case)
	int step;

	//Our equation!
	step = (freq*PHASESCALE)/SAMPLE_RATE;
	return step;
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
    int j;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
    
    long frameVal;
    int phase_truncated = 16-POWER;

    for( i=0; i<framesPerBuffer; i++ )
    {
    	frameVal=0;
    	//Lookup the wave value of the current phase (Only uses the top 16-POWER bits of phase, to allow for table sizes smaller than the phase register)
    	for (j=0;j<NUM_POLYVOICES;j++){
    		if (data[j].isActive){
	    	    frameVal += (sq2[(data[j].phase)>>(phase_truncated)]);    
    	    	/* Advance Phase */
       	 		data[j].phase += stepsize(data[j].frequency);
       	 	}
    	}
        *out++ = (float)frameVal / 65536;
        
        
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
void doAction(PmEvent data) {
	// TODO: Put the polyvoice logic in here. :D
	int status = Pm_MessageStatus(data.message);
	int channel;
	int note = Pm_MessageData1(data.message);
	int velocity = Pm_MessageData2(data.message);
	int i;
	int j = 0;
	//printf("status:%d, byte1=%d, byte2=%d, time=%.3f\n", status, note, velocity, data.timestamp/1000.0);
	for (i=0;i<NUM_POLYVOICES;i++){
		if (module1[i].note == note){
			j=1;
			module1[i].frequency = MtoF(note);
			module1[i].note = note;
			module1[i].isActive = velocity;
			if (status==0x80){
				module1[i].isActive=0;
			}
		}
	}
	if (j!=1){
		for (i=0;i<NUM_POLYVOICES;i++){
			if(module1[i].isActive == 0){
				module1[i].frequency = MtoF(note);
				module1[i].note = note;
				module1[i].isActive =velocity;
				return;
			}
		}
	}
	return;
}


void interpretMIDI() {
	int dev = 2; // dev is the device, should always be 2		
	int i;
	PmError retval;
	const PmDeviceInfo *info;
	PmEvent msg[32];
	PortMidiStream *mstream;

	Pt_Start(1, NULL, NULL);
	retval = Pm_OpenInput(&mstream, dev, NULL, 512L, NULL, NULL);
	
	if(retval != pmNoError) {
		printf("error: %s \n", Pm_GetErrorText(retval));
	}
	else {
		while(1) {
			if(Pm_Poll(mstream)) {
				int cnt = Pm_Read(mstream, msg, 32);
				for(i=0; i<cnt; i++) {
					doAction(msg[i]);					
				}
			}
		}
	}
	Pm_Close(mstream);
	return;
}

void readMIDI() { // Reads the MIDI input stream

	int cnt;

	Pm_Initialize();
	cnt = Pm_CountDevices();
	
	if(cnt) {
		interpretMIDI();
	}
	else {
		printf("No MIDI devices found\n");
	}
	Pm_Terminate();
	return;
}
/*******************************************************************/

int main(void);
int main(void)
{
	wavetablegen();
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
              &module1);
	if( err != paNoError ) goto error;

	err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
	if( err != paNoError ) goto error;

	//Start Stream
    	err = Pa_StartStream( stream );
   	if( err != paNoError ) goto error;
   	
   	
/*	int i;
	int j;
	for (j=0;j<1;j++){
	for (i=0;i<100;i++){
		module1[0].frequency = 800 + 10*i;
		module1[0].note = 60;
		module1[0].isActive = 1;
		module1[1].frequency = 500 + 10*i;
		module1[1].note = 60;
		module1[1].isActive = 1;
		module1[2].frequency = 200 + 10*i;
		module1[2].note = 60;
		module1[2].isActive = 1;
		Pa_Sleep(10);
	}
	Pa_Sleep(50);
	}*/
	
	readMIDI();
	
	
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
