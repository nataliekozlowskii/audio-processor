#include "portaudio.h"
#include <cstdio>
#include <sndfile.h>

const int sample_rate = 44100;
const int input_channels = 2;
const int seconds = 5;

typedef struct
{
    SNDFILE* file;
    SF_INFO sfinfo;
}
UserData;

// callback function used to process audio
int callback(const void *inputBuffer, 
             void *outputBuffer, 
             unsigned long framesPerBuffer,
             const PaStreamCallbackTimeInfo* timeInfo,
             PaStreamCallbackFlags statusFlags,
             void *userData) {

    UserData *user_data = (UserData*)userData;
    const float *in = (const float*)inputBuffer;

    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    if (in != nullptr) {
        // write frames to file
        sf_writef_float(user_data->file, in, framesPerBuffer);
    }
    
    return paContinue;
}

int main() {
    PaError err;

    // output WAV file setup
    UserData user_data;
    user_data.sfinfo.samplerate = sample_rate;
    user_data.sfinfo.channels = input_channels;
    user_data.sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    user_data.file = sf_open("recording.wav", SFM_WRITE, &user_data.sfinfo);
    if (!user_data.file) {
        printf("Error: could not open recording.wav\n");
        return 1;
    }

    // initialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    // open audio I/O stream
    PaStream *stream;

    err = Pa_OpenDefaultStream(&stream,
                               input_channels, // stereo input
                               0, // no output (writing to file)
                               paFloat32, // sample format
                               sample_rate,
                               256, //frames per buffer (# sample frames PortAudio req from callback)
                               callback, // callback function,
                               &user_data); // pointer passed to callback

    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    // start stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    // wait for 10 seconds
    printf("Recording input for %d seconds\n", seconds);
    Pa_Sleep(seconds * 1000);

    // stop playback
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    // close the stream
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }
                               
    // terminate PortAudio
    err = Pa_Terminate();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    // close the WAV file
    sf_close(user_data.file);
}