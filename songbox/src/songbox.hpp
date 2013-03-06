/*****************************************************************************
 * Joseph Zambreno
 * Department of Electrical and Computer Engineering
 * Iowa State University
 *****************************************************************************/


/*****************************************************************************
 * songbox.cpp - Chipophone project SongBox application. Tests SFML sound
 * generation using the SFML Sound() class.
 *
 * NOTES:
 * 3/3/13 by JAZ::Design created.
 *****************************************************************************/


#pragma once


#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string>
#include <cstring>
#include <iostream>
#include <math.h>
#include <limits.h>

#define EXEC_NAME "songbox"
#define WINDOW_TITLE "Chipophone SongBox, v-1.0a"

#define ERR_USAGE 1
#define ERR_SFML 2
#define ERR_NOFILE1 3
#define ERR_NOFILE2 4
#define ERR_NOFILE3 5
#define ERR_BADFILE1 6
#define ERR_BADFILE2 7
#define ERR_BADFILE3 8
#define ERR_BADFILE4 9
#define ERR_AUDIO 10
#define ERR_OVERBUDGET 20
#define ERR_NOMEM 50
#define ERR_UNDEFINED 100

#define FRAME_RATE 30
#define SCREEN_WIDTH_DEFAULT 640
#define SCREEN_HEIGHT_DEFAULT 480
#define SCREEN_DEPTH_DEFAULT 32
#define DEBUG_DEFAULT 0

#define FRONT_DEPTH 0
#define BACK_DEPTH 1

/* Samples in the wavetable, sampling rate, and number of concurrent keys */
#define NUM_SAMPLES 4096
#define SAMPLE_RATE 44100
#define NUM_KEYS 4


typedef enum {NOTE_A=0, NOTE_ASHARP, NOTE_B, NOTE_C, NOTE_CSHARP, NOTE_D, NOTE_DSHARP, NOTE_E, NOTE_F, NOTE_FSHARP, NOTE_G, NOTE_GSHARP, NUM_NOTES} NOTE_ENUM;
typedef enum {SIN_WAVE=0, SQUARE_WAVE, TRIANGLE_WAVE, SAWTOOTH_WAVE, NOISE_WAVE, NUM_VOICES} VOICE_ENUM;


namespace cop {

    /* Songbox configuration information */
    class Config {
        public:
            int32_t debug_level;
            uint16_t screen_width;
            uint16_t screen_height;
            uint8_t screen_depth;
            int32_t time_ms;
    };

    /* Main SongBox class */
    class SongBox {
        public:
            /* Main functions (songbox.cpp) */
            SongBox(int argc, char **argv);
            ~SongBox();
            void mainLoop();

            /* SFML functions (sfml_utils.cpp) */
            void initSFML();
            void loadWavetables();
            void processEvents();

            /* Simple utility functions (songbox.cpp) */
            void print_help();
            void raise_error(uint32_t, const char *msg);
            void printConfig();

        private:
            cop::Config myConfig;
            sf::ContextSettings mySettings;
            sf::RenderWindow myWindow;
            sf::SoundBuffer *mySoundBuffers[NUM_KEYS];
            sf::Sound mySounds[NUM_KEYS];
            sf::Clock myClock;
            sf::Int16 voice_table[NUM_VOICES][NUM_SAMPLES];
            float note_frequencies[NUM_NOTES];
            NOTE_ENUM active_notes[NUM_KEYS];
            bool notes_pressed[NUM_NOTES];
            uint8_t current_voice;
            uint8_t note_offset;
    };

} // namespace cop

