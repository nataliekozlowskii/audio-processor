#include "portaudio.h"
#include <cstdio>
#include <sndfile.h>
#include <vector>
#include <complex>
#include <iostream>
#include <string>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Complex = std::complex<double>;
using CArray = std::vector<Complex>;

const int sample_rate = 44100;
const int input_channels = 1;
const int seconds = 15;

typedef struct
{
    SNDFILE* file;
    SF_INFO sfinfo;
}
UserData;

// apply Hanning window to signal
CArray hann_window(const float* input, 
                   unsigned long framesPerBuffer){

    CArray windowed_signal(framesPerBuffer);

    int total_size = framesPerBuffer;

    for(unsigned int i = 0; i < framesPerBuffer; i++){
        float window_coeff = 0.5 * (1.0 - std::cos((2 * M_PI * i)/(total_size-1)));

        float sample = input[i*input_channels];

        // set imaginary part of the complex # to 0
        windowed_signal[i] = Complex(sample * window_coeff, 0.0f);
    }

    return windowed_signal;
}

// Cooley-Tukey FFT implementation
void fft(CArray& signal){
    const size_t total_size = signal.size();

    // recursion base case
    if (total_size <= 1) {
        return;
    }

    CArray even(total_size / 2), odd(total_size / 2);

    // split into even and odd arrays
    for(size_t i = 0; i < total_size / 2; i++){
        even[i] = signal[i*2];
        odd[i] = signal[i*2 + 1];
    }

    // recurse on both
    fft(even);
    fft(odd);

    for (size_t i  = 0; i < total_size / 2; i++) {
        Complex transformed = std::polar<double>(1.0, -2.0 * M_PI * i / total_size) * odd[i];
        signal[i] = even[i] + transformed;
        signal[i + total_size/2] = even[i] - transformed;
    }
}

// calculate the magnitudes of outputs in the frequency domain
std::vector<float> get_magnitudes(const CArray& frequencyDomain) {

    // only first half of frequency domain outputs contain unique info
    std::vector<float> magnitudes(frequencyDomain.size() / 2);

    for (size_t i = 0; i < magnitudes.size(); i++){
        magnitudes[i] = std::abs(frequencyDomain[i]);
    }

    return magnitudes;
}

void visualize_audio(const std::vector<float>& magnitudes) {
    int total_size = magnitudes.size() * 2;
    int num_bars = 50;
    int step_size = magnitudes.size() / num_bars;

    // print bars corresponding to each frequency bin
    for (int i = 0; i < num_bars; i++){
        // get the avg magnitude of this column
        float avg = 0.0f;
        for (int j = 0; j < step_size; j++) {
            avg += magnitudes[i * step_size + j];
        }
        avg /= step_size;

        // scale the magnitude to decibels
        float reference_amp = 1.0f;
        float db = 20 * log10((avg+1e-9f)/reference_amp); // add 1e-9 to avoid log(0)

        // scale such that bar height fits on screen
        int height = (int)((db + 60)/2);

        // deal with out of range heights
        if (height < 0) {
            height = 0;
        }
        else if (height > 30) {
            height = 30;
        }

        // print this column
        std::cout << std::string(height, '*') << "\n";
    }
}

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
        // apply hann window
        CArray windowed_signal = hann_window(in, framesPerBuffer);

        // run FFT
        fft(windowed_signal);

        // get magnitudes
        std::vector<float> magnitudes = get_magnitudes(windowed_signal);

        // visualize
        visualize_audio(magnitudes);

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
                               input_channels, // # input channels
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