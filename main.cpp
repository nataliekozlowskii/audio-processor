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

struct UserData {
    SNDFILE* file;
    SF_INFO sfinfo;
};

// define equalization settings
struct EQSettings {
    float bass_gain   = 1.5f;  // boost bass
    float mid_gain    = 1.0f;  // keep same
    float treble_gain = 0.5f;  // cut treble
};


// audio equalization (adjust volume of different frequencies)
void apply_eq(CArray& spectrum, double sampleRate, const EQSettings& eq) {
    size_t N = spectrum.size();

    for (size_t i = 0; i < N; i++) {
        double freq = (i * sampleRate) / N;

        if (freq < 250.0) {
            spectrum[i] *= eq.bass_gain;
        } else if (freq < 4000.0) {
            spectrum[i] *= eq.mid_gain;
        } else {
            spectrum[i] *= eq.treble_gain;
        }
    }
}

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

// Cooley-Tukey FFT implementation (time -> frequency domain)
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

// inverse FFT implementation (frequency -> time domain)
void inverse_fft(CArray& signal) {
    // conjugate complex numbers
    for (auto& x : signal) {
        x = std::conj(x);
    }

    // forward FFT
    fft(signal);

    // conjugate again and scale by 1/N
    for (auto& x : signal) {
        x = std::conj(x) / static_cast<double>(signal.size());
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