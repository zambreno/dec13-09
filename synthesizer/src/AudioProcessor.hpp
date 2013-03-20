#pragma once

#include <stdio.h>
#include <math.h>
#include <iostream>
#include "portaudio.h"
#include "Module.hpp"

//NUM-SECONDS is simply a playback mechanism
#define NUM_SECONDS   (1)

//Sample rate of the soundcard
#define SAMPLE_RATE   (32000)

//Audio buffer size
#define FRAMES_PER_BUFFER  (256)

//Power of wavetable size (wavetable size = 2 ^ POWER)
#define POWER (4)
#define TABLE_SIZE   (1<<POWER)

//Pi, for generation of sine table
#ifndef M_PI
#define M_PI  (3.14159265)
#endif


namespace chip
{

    class AudioProcessor
    {
        public:
            AudioProcessor();
            ~AudioProcessor();
            void mainLoop();
            
            //void generateWavetables();
            
            /* This routine will be called by the PortAudio engine when audio is needed.
            ** It may called at interrupt level on some machines so don't do anything
            ** that could mess up the system like calling malloc() or free().
            */
            static int paCallback( const void *inputBuffer, void *outputBuffer,
                                   unsigned long framesPerBuffer,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags,
                                   void *userData );
            
        private:
            Module modules[5];
            Wavetables wavetables[4];
    };

} 

