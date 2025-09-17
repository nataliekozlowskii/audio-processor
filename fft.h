#ifndef FFT_H
#define FFT_H

#include <vector>
#include <complex>
#include <cmath>

// type aliases
using Complex = std::complex<double>;
using CArray  = std::vector<Complex>;

// bit reversal helper function
int reverse_bits(int forward_int, int log2n);

// FFT
void fft(CArray& signal);

// IFFT
void inverse_fft(CArray& signal);

#endif