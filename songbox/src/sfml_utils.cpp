/*****************************************************************************
 * Joseph Zambreno
 * Department of Electrical and Computer Engineering
 * Iowa State University
 *****************************************************************************/
/*****************************************************************************
 * sfml_utils.cpp - graphics and audio functionality using the SFML-2.0
 * library with OpenGL.
 *
 *
 * NOTES:
 * 3/3/13 by JAZ::Design created.
 *****************************************************************************/

#include "songbox.hpp"


namespace cop {

    /*****************************************************************************
    * Function: SongBox::processEvents
    * Description: Processes keyboard and other SFML events.
    *****************************************************************************/
    void SongBox::processEvents() {

        sf::Event event;
        NOTE_ENUM note_type = NUM_NOTES;

        while (myWindow.pollEvent(event)) {
            if ((event.type == sf::Event::KeyPressed) || (event.type == sf::Event::KeyReleased)) {
                switch (event.key.code) {
                    case sf::Keyboard::A:
                        note_type = NOTE_A;
                        break;
                    case sf::Keyboard::W:
                        note_type = NOTE_ASHARP;
                        break;
                    case sf::Keyboard::S:
                        note_type = NOTE_B;
                        break;
                    case sf::Keyboard::D:
                        note_type = NOTE_C;
                        break;
                    case sf::Keyboard::R:
                        note_type =  NOTE_CSHARP;
                        break;
                    case sf::Keyboard::F:
                        note_type = NOTE_D;
                        break;
                    case sf::Keyboard::T:
                        note_type = NOTE_DSHARP;
                        break;
                    case sf::Keyboard::G:
                        note_type = NOTE_E;
                        break;
                    case sf::Keyboard::H:
                        note_type = NOTE_F;
                        break;
                    case sf::Keyboard::U:
                        note_type = NOTE_FSHARP;
                        break;
                     case sf::Keyboard::J:
                        note_type = NOTE_G;
                        break;
                    case sf::Keyboard::I:
                        note_type = NOTE_GSHARP;
                        break;
                    case sf::Keyboard::Space:
                        current_voice++;
                        current_voice %= NUM_VOICES;
                        break;
                    case sf::Keyboard::Up:
                        if ((event.type == sf::Event::KeyPressed) && (note_offset > 0)) {
                            note_offset += 5;
                            for (uint16_t i = 0; i < NUM_NOTES; i++) {
                                note_frequencies[i] = pow(2,(note_offset+i-49)/12.0) * 440.0;
                            }
                        }
                        break;
                    case sf::Keyboard::Down:
                        if ((event.type == sf::Event::KeyPressed) && (note_offset < 88)) {
                            note_offset -= 5;
                            for (uint16_t i = 0; i < NUM_NOTES; i++) {
                                note_frequencies[i] = pow(2,(note_offset+i-49)/12.0) * 440.0;
                            }
                        }
                        break;
                    case sf::Keyboard::Escape:
                    case sf::Keyboard::Q:
                        myWindow.close();
                        break;
                    default:
                        break;
                }

                if (note_type != NUM_NOTES) {

                    /* Only generate a new sound if we have a new note */
                    if ((event.type == sf::Event::KeyPressed) && notes_pressed[note_type] == false) {

                        sf::Int16 *note_table;

                        for (int8_t i = 0; i < NUM_KEYS; i++) {
                            // We've found a free Sound()
                            if (active_notes[i] == NUM_NOTES) {
                                uint32_t step_size = (uint32_t)floor(NUM_SAMPLES*note_frequencies[note_type]/SAMPLE_RATE);
                                uint32_t note_table_length = NUM_SAMPLES / step_size;

                                note_table = new sf::Int16[note_table_length];
                                for (uint32_t j = 0; j < note_table_length; j++) {
                                    note_table[j] = voice_table[current_voice][(j*step_size)%NUM_SAMPLES];
                                }

                                mySoundBuffers[i] = new sf::SoundBuffer;
                                mySoundBuffers[i]->loadFromSamples(note_table, note_table_length, 1, SAMPLE_RATE);
                                mySounds[i].setBuffer(*mySoundBuffers[i]);
                                mySounds[i].setLoop(true);
                                mySounds[i].play();
                                active_notes[i] = note_type;
                                notes_pressed[note_type] = true;
                                /* loadFromSamples apparently copies the samples over, so we can delete the note_table */
                                delete note_table;
                                break;
                            }
                        }
                    }
                    if ((event.type == sf::Event::KeyReleased) && notes_pressed[note_type] == true) {
                        for (int8_t i = 0; i < NUM_KEYS; i++) {
                            if (active_notes[i] == note_type) {
                                mySounds[i].stop();
                                mySounds[i].setLoop(false);
                                active_notes[i] = NUM_NOTES;
                                notes_pressed[note_type] = false;

                                /* I don't think we need the original note_table pointer at this point, see above */
                                //sf::Int16 *note_table = const_cast<sf::Int16 *>(mySoundBuffers[i]->getSamples());
                                delete mySoundBuffers[i];
                                break;
                            }
                        }
                    }
                }

            }
        }

    }


    /*****************************************************************************
    * Function: SongBox::loadWavetables
    * Description: Generates the various wavetables for audio synthesis
    *****************************************************************************/
    void SongBox::loadWavetables() {

        /* Initialize the frequency and note information */
        note_offset = 37; // Start with A3
        current_voice = SIN_WAVE;
        for (uint16_t i = 0; i < NUM_NOTES; i++) {
            // http://en.wikipedia.org/wiki/Piano_key_frequencies
            note_frequencies[i] = pow(2,(note_offset+i-49)/12.0) * 440.0;
            notes_pressed[i] = false;
        }
        for (uint8_t i = 0; i < NUM_KEYS; i++) {
            active_notes[i] = NUM_NOTES;
        }

        /* Generate the sin_table */
        for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
            voice_table[SIN_WAVE][i] = sin(M_PI*2.0*i/NUM_SAMPLES) * SHRT_MAX/NUM_KEYS;
        }
        /* Generate the square_table */
        for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
            if (i < NUM_SAMPLES/2) {
                voice_table[SQUARE_WAVE][i] = SHRT_MAX/NUM_KEYS;
            }
            else {
                voice_table[SQUARE_WAVE][i] = -SHRT_MAX/NUM_KEYS;
            }
        }

        /* Generate the triangle_table */
        /* This is wrong and I no longer care */
        for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
            if (i < NUM_SAMPLES/4) {
                voice_table[TRIANGLE_WAVE][i] = SHRT_MAX*4*(i/NUM_SAMPLES)/NUM_KEYS;
            }
            else if (i < 3*NUM_SAMPLES/4) {
                voice_table[TRIANGLE_WAVE][i] = (SHRT_MAX-SHRT_MAX*2*(i-NUM_SAMPLES/4)/NUM_SAMPLES)/NUM_KEYS;
            }
            else {
                voice_table[TRIANGLE_WAVE][i] = (-32767+4*(i-3*NUM_SAMPLES/4)/NUM_SAMPLES)/NUM_KEYS;
            }
        }

        /* Generate the sawtooth_table */
        for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
            voice_table[SAWTOOTH_WAVE][i] = (SHRT_MIN+2*SHRT_MAX*(i/NUM_SAMPLES))/NUM_KEYS;
        }

        /* Generate the noise_table */
        /* Also not working - who knows */
        srand(0);
        for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
            voice_table[NOISE_WAVE][i] = (2*(rand()%SHRT_MAX)+SHRT_MIN)/NUM_KEYS;
        }


        if (myConfig.debug_level > 3)
            printf("done.\n");

    }


    /*****************************************************************************
    * Function: SongBox::initSFML()
    * Description: Initializes the SFML environment for handling graphics and
    * audio.
    *****************************************************************************/
    void SongBox::initSFML() {

        // Attempt to create the user-specified window
        std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
        bool foundMode = false;

        if (myConfig.debug_level > 10) {
            printf("\nAvailable video modes:\n");
        }
        for (std::size_t i = 0; i < modes.size(); i++) {
            sf::VideoMode mode = modes[i];
            if (myConfig.debug_level > 10) {
                printf("\tMode #%u: [%u,%u] - %ubpp\n", i, mode.width, mode.height, mode.bitsPerPixel);
            }
            if ((mode.width == myConfig.screen_width) && (mode.height == myConfig.screen_height)
                && (mode.bitsPerPixel == myConfig.screen_depth))
                foundMode = true;
        }

        if (foundMode == false) {
            if (myConfig.debug_level > 0) {
                printf("Warning: specified [%u, %u] resolution not available\n",
                       myConfig.screen_width, myConfig.screen_height);
                printf("Substituting with [%u, %u]\n", modes[0].width, modes[0].height);
                myConfig.screen_width = modes[0].width;
                myConfig.screen_height = modes[0].height;
                myConfig.screen_depth = modes[0].bitsPerPixel;
            }
        }

        myWindow.create(sf::VideoMode(myConfig.screen_width, myConfig.screen_height,
                        myConfig.screen_depth), WINDOW_TITLE);//, sf::Style::Fullscreen);

        mySettings = myWindow.getSettings();
        if (myConfig.debug_level > 3) {
            printf("\n%s Window configuration: \n", WINDOW_TITLE);
            printf("\tDepth bits: %u\n", mySettings.depthBits);
            printf("\tStencil bits: %u\n", mySettings.stencilBits);
            printf("\tAnti-Aliasing level: %u\n", mySettings.antialiasingLevel);
            printf("\tOpenGL v%u.%u\n\n", mySettings.majorVersion, mySettings.minorVersion);
        }

        myWindow.setVerticalSyncEnabled(true);
        //myWindow.setFramerateLimit(FRAME_RATE);


        /* Configure OpenGL default state */
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClearDepth(BACK_DEPTH);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1032.5, 1032.5, -581.0, 581.0, FRONT_DEPTH, -BACK_DEPTH);
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_TEXTURE_2D);

        //glAlphaFunc(GL_GREATER, 0);
        //glEnable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    }


} // namespace cop
