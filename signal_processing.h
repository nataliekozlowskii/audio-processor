#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

#include "fft.h"
#include <vector>
#include "config.h"

// equalization
void apply_eq(CArray& spectrum, double sampleRate, const EQSettings& eq);

// Windowing
CArray hann_window(const float* input, unsigned long framesPerBuffer, int input_channels = 1);

// spectrum analysis
std::vector<float> get_magnitudes(const CArray& frequencyDomain);

// ASCII visualization in terminal
void visualize_audio(const std::vector<float>& magnitudes);

#endif