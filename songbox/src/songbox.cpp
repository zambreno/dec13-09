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



#include "songbox.hpp"


int main(int argc, char *argv[]) {

    cop::SongBox *mySongBox = new(std::nothrow)cop::SongBox(argc,argv);


    mySongBox->initSFML();
    mySongBox->loadWavetables();

    mySongBox->mainLoop();

    delete mySongBox;
    return 0;
}

namespace cop {

    /*****************************************************************************
    * Function: SongBox::mainLoop()
    * Description: Runs the main update, drawing, and event processing loop.
    *****************************************************************************/
    void SongBox::mainLoop() {

        //NOTE: These tasks can be split into multiple threads, in that case we would
        //have to take care regarding which thread has the active window context.

        sf::Time myTime;
        while (myWindow.isOpen()) {
            processEvents();

            myWindow.display();
        }

        return;

    }


    /*****************************************************************************
    * Function: SongBox::SongBox()
    * Description: Constructor for cop::SongBox() class. Reads in and parses the
    * input command line in order to set configuration data
    *****************************************************************************/
    SongBox::SongBox(int argc, char **argv) {
        int8_t i;

        /* If the last command-line argument is -h or --help that is ok */
        if (!strncmp(argv[argc-1], "-h", 2) || !strncmp(argv[argc-1], "--help", 6)) {
            print_help();
        }

        /* Set the default values */
        myConfig.debug_level = DEBUG_DEFAULT;
        myConfig.screen_height = SCREEN_HEIGHT_DEFAULT;
        myConfig.screen_width = SCREEN_WIDTH_DEFAULT;
        myConfig.screen_depth = SCREEN_DEPTH_DEFAULT;

        for (i = 1; i < argc; i++) {
            /* Arguments must begin with '-' or '--' */
            if (argv[i][0] != '-' || strlen(argv[i]) < 2) {
                raise_error(ERR_USAGE, NULL);
            }

            /* Decode the single dash arguments */
            if (argv[i][1] != '-') {
                /* Single dash arguments should have just one character after the dash */
                if (strlen(argv[i]) > 2) {
                    raise_error(ERR_USAGE, NULL);
                }
                switch (argv[i][1]) {
                    case 'H':
                        i++;
                        sscanf(argv[i], "%hu", &myConfig.screen_height);
                        break;
                    case 'W':
                        i++;
                        sscanf(argv[i], "%hu", &myConfig.screen_width);
                        break;
                    case 'D':
                        i++;
                        sscanf(argv[i], "%d", &myConfig.debug_level);
                        break;
                    case 'h':
                        print_help();
                        break;
                    default:
                        raise_error(ERR_USAGE, NULL);
                        break;
                }
            }

            /* Decode the double dash arguments */
            else {
                if (!strncmp(argv[i], "--height", 8)) {
                    i++;
                    sscanf(argv[i], "%hu", &myConfig.screen_height);
                }
                else if (!strncmp(argv[i], "--width", 7)) {
                    i++;
                    sscanf(argv[i], "%hu", &myConfig.screen_width);
                }
                else if (!strncmp(argv[i], "--debug", 7)) {
                    i++;
                    sscanf(argv[i], "%d", &myConfig.debug_level);
                }
                else if (!strncmp(argv[i], "--help", 6)) {
                    print_help();
                }
                else {
                    raise_error(ERR_USAGE, NULL);
                }
            }
        }

        return;
    }

    /*****************************************************************************
    * Function: SongBox::~SongBox()
    * Description: Destructor for cop::SongBox() class
    *****************************************************************************/
    SongBox::~SongBox() {
    }


    /*****************************************************************************
    * Function: print_help
    * Description: Prints out the program help message.
    *****************************************************************************/
    void SongBox::print_help() {
        printf("Usage: %s [options] <file1> [<files> ...]\n\n", EXEC_NAME);
        printf("Main options:\n");

        printf("-W <value>, --width <value>        Screen pixel width ");
        printf("(default is %d)\n", SCREEN_WIDTH_DEFAULT);

        printf("-H <value>, --height <value>       Screen pixel height ");
        printf("(default is %d)\n", SCREEN_HEIGHT_DEFAULT);

        printf("-D <value>, --debug <value>        Debug level ");
        printf("(default is %d)\n", DEBUG_DEFAULT);

        printf("-h, --help                         Print this message\n");

        exit(0);
    }

    /*****************************************************************************
    * Function: raise_error
    * Description: Prints out an error message determined by
    * the condition and exits the program.
    *****************************************************************************/
    void SongBox::raise_error(uint32_t error_num, const char *msg) {

        fprintf(stderr, "\n");
        switch(error_num) {
            case ERR_SFML:
                fprintf(stderr, "Error: initSDL failed: %s\n", msg);
                break;
            case ERR_USAGE:
                fprintf(stderr, "Usage: %s [-W <n>] [-H <n>] [-D <n>] ", EXEC_NAME);
                break;
            case ERR_NOFILE1:
                fprintf(stderr, "Error: %s texture file not found\n", msg);
                break;
            case ERR_BADFILE1:
                fprintf(stderr, "Error: %s texture file is invalid\n", msg);
                break;
            case ERR_BADFILE4:
                fprintf(stderr, "Error: %s audio file is invalid\n", msg);
                break;
            case ERR_AUDIO:
                fprintf(stderr, "Error: audio problem in %s\n", msg);
                break;
            case ERR_NOMEM:
                fprintf(stderr, "Error: memory allocation problem in %s\n", msg);
                break;
            case ERR_UNDEFINED:
                default:
                fprintf(stderr, "Error: undefined error\n");
                break;
        }
        fprintf(stderr, "%s exited with error code %d\n", EXEC_NAME, error_num);
        exit(error_num);
    }


    /*****************************************************************************
    * Function: printConfig
    * Description: Prints the current configuration data.
    *****************************************************************************/
    void SongBox::printConfig() {

        if (myConfig.debug_level > 5) {
            printf("\nCurrent configuration information:\n");
            printf("\tDebug level:           %d\n", myConfig.debug_level);
            printf("\tScreen pixel width:  %d\n", myConfig.screen_width);
            printf("\tScreen pixel height: %d\n", myConfig.screen_height);

            printf("\n");


        }
    }




} //namespace cop



