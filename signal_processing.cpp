#include "signal_processing.h"
#include <cmath>
#include <iostream>
#include "config.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// adjust the bass, mid, and treble of the recording
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

// apply Hann window to block of audio samples
CArray hann_window(const float* input, unsigned long framesPerBuffer, int input_channels) {
    CArray windowed(framesPerBuffer);
    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float coeff = 0.5f * (1.0f - std::cos((2 * M_PI * i) / (framesPerBuffer - 1)));
        float sample = input[i * input_channels];
        windowed[i] = Complex(sample * coeff, 0.0);
    }
    return windowed;
}

// compute magnitudes of first half of the FFT frequency spectrum
std::vector<float> get_magnitudes(const CArray& frequencyDomain) {
    std::vector<float> magnitudes(frequencyDomain.size() / 2);
    for (size_t i = 0; i < magnitudes.size(); i++) {
        magnitudes[i] = std::abs(frequencyDomain[i]);
    }
    return magnitudes;
}

// print a representation of the audio spectrum
void visualize_audio(const std::vector<float>& magnitudes) {
    int step = magnitudes.size() / NUM_BARS;

    for (int i = 0; i < NUM_BARS; i++) {
        float avg = 0.0f;
        for (int j = 0; j < step; j++) avg += magnitudes[i * step + j];
        avg /= step;

        float db = 20 * log10(avg + 1e-9f); // avoid log(0)
        int height = (int)((db + 60) / 2);
        if (height < 0) height = 0;
        if (height > MAX_BAR_HEIGHT) height = MAX_BAR_HEIGHT;

        std::cout << std::string(height, '*') << "\n";
    }
}