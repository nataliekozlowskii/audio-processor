#include "portaudio.h"
#include <cstdio>
#include <sndfile.h>
#include <iostream>
#include <string>
#include <cmath>
#include "fft.h"
#include "signal_processing.h"
#include "config.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Complex = std::complex<double>;
using CArray = std::vector<Complex>;

struct UserData {
    SNDFILE* file;
    SF_INFO sfinfo;
};

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

        // --- Audio Visualization ---
        // apply hann window
        CArray windowed_signal = hann_window(in, framesPerBuffer);

        // FFT (time -> frequency domain)
        fft(windowed_signal);

        // get magnitudes
        std::vector<float> magnitudes = get_magnitudes(windowed_signal);

        // visualize
        visualize_audio(magnitudes);

        // --- Audio Processing ---
        // apply audio equalization
        EQSettings eq;
        apply_eq(windowed_signal, user_data->sfinfo.samplerate, eq);

        // inverse FFT (frequency -> time domain)
        inverse_fft(windowed_signal);

        // convert complex sample data into real array of floats
        std::vector<float> eq_output(framesPerBuffer);
        for (size_t i = 0; i < framesPerBuffer; i++) {
            eq_output[i] = static_cast<float>(windowed_signal[i].real());
        }

        // write frames to file
        sf_writef_float(user_data->file, eq_output.data(), framesPerBuffer);
    }
    
    return paContinue;
}

int main() {
    PaError err;

    // output WAV file setup
    UserData user_data;
    user_data.sfinfo.samplerate = SAMPLE_RATE;
    user_data.sfinfo.channels = INPUT_CHANNELS;
    user_data.sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    user_data.file = sf_open(RECORDING_FILENAME, SFM_WRITE, &user_data.sfinfo);
    if (!user_data.file) {
        printf("Error: could not open recording file.\n");
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
                               INPUT_CHANNELS,
                               0, // no output (writing to file)
                               paFloat32, // sample format
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               callback,
                               &user_data);

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

    // record for the given number of seconds
    printf("Recording input for %d seconds\n", SECONDS);
    Pa_Sleep(SECONDS * 1000);

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