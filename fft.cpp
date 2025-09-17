#include "fft.h"
#include <algorithm>
#include "config.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// bit reversal
int reverse_bits(int forward_int, int log2n) {
    int reversed = 0;
    for (int i = 0; i < log2n; i++) {
        if ((forward_int >> i) & 1) {
            reversed |= (1 << (log2n - 1 - i));
        }
    }
    return reversed;
}

// Cooley-Tukey FFT implementation (time -> frequency domain)
void fft(CArray& signal){
    const size_t total_size = signal.size();

    int log2n = 0;
    while ((1 << log2n) < total_size) {
        log2n++;
    }

    // bit reversal
    for (int i = 0; i < total_size; i++) {
        int reversed_i = reverse_bits(i, log2n);
        if (i < reversed_i) {
            std::swap(signal[i], signal[reversed_i]);
        }
    }

    // iterative butterfly operations
    for (int len = 2; len <= total_size; len <<= 1) {
        double angle = -2 * M_PI / len;
        std::complex<double> wlen(std::cos(angle), std::sin(angle));
            
        for (int i = 0; i < total_size; i += len) {
            std::complex<double> twiddle_factor(1);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> even_part = signal[i + j];
                std::complex<double> odd_part_twiddled = (signal[i + j + len / 2] 
                                                          * twiddle_factor);
                signal[i + j] = even_part + odd_part_twiddled;
                signal[i + j + len / 2] = even_part - odd_part_twiddled;
                twiddle_factor *= wlen;
            }
        }
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