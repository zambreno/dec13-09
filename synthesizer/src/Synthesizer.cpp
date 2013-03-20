#include "AudioProcessor.hpp"


/*
 * This function gets called whenever an error occurred while setting up
 * PortAudio.
 */
int error(int err) 
{
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}


/*
 * Sets up the PortAudio audio stream and then waits until the user presses
 * enter to terminate. During this time, audio callbacks are occurring.
 */
int main(void)
{
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    void data;
    int i;
    
    // TODO create the AudioProcessor
    
    err = Pa_Initialize();
    if( err != paNoError ) error(err);
    
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) error(err);
    
    outputParameters.channelCount = 1;       /* mono output? */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    // TODO send in the AudioProcessor's callback instead of one that doesn't exist
    err = Pa_OpenStream(
          &stream,
          NULL, /* no input */
          &outputParameters,
          SAMPLE_RATE,
          FRAMES_PER_BUFFER,
          paClipOff,      /* we won't output out of range samples so don't bother clipping them */
          patestCallback,
          &data );
          
    if( err != paNoError ) error(err);
    
    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
    if( err != paNoError ) error(err);
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) error(err);
    
    // Block the front end until someone hits enter
    // We are getting audio callbacks while this is happening
    std::cin.ignore(255, '\n');
    
    err = Pa_StopStream( stream );
    if( err != paNoError ) error(err);

    err = Pa_CloseStream( stream );
    if( err != paNoError ) error(err);

    Pa_Terminate();
}

