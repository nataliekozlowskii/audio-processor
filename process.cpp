#include <sndfile.h>
#include <iostream>
#include <vector>
#include "fft.h"
#include "signal_processing.h"
#include "config.h"

int main() {

    // Open input WAV
    SF_INFO sfinfo_in;
    SNDFILE* infile = sf_open(RECORDING_FILENAME, SFM_READ, &sfinfo_in);
    if (!infile) {
        std::cerr << "Error: could not open input file\n";
        return 1;
    }

    // open output WAV
    SF_INFO sfinfo_out = sfinfo_in;
    SNDFILE* outfile = sf_open(PROCESSED_FILENAME, SFM_WRITE, &sfinfo_out);
    if (!outfile) {
        std::cerr << "Error: could not open output file\n";
        sf_close(infile);
        return 1;
    }

    // default eq settings
    EQSettings eq;

    std::vector<float> buffer(FRAMES_PER_BUFFER * sfinfo_in.channels);

    while (true) {
        sf_count_t frames_read = sf_readf_float(infile, buffer.data(), FRAMES_PER_BUFFER);
        if (frames_read == 0) break;

        // apply Hann window
        CArray windowed = hann_window(buffer.data(), frames_read, sfinfo_in.channels);

        // FFT -> EQ -> iFFT
        fft(windowed);
        apply_eq(windowed, sfinfo_in.samplerate, eq);
        inverse_fft(windowed);

        // convert back to real samples
        std::vector<float> processed(frames_read * sfinfo_in.channels);
        for (size_t i = 0; i < frames_read; i++) {
            processed[i] = static_cast<float>(windowed[i].real());
        }

        // write output to file
        sf_writef_float(outfile, processed.data(), frames_read);
    }

    sf_close(infile);
    sf_close(outfile);

    std::cout << "Processing done. Saved to " << PROCESSED_FILENAME << std::endl;
    return 0;
}
