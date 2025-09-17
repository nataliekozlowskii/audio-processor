#ifndef CONFIG_H
#define CONFIG_H

// equalization settings
struct EQSettings {
    float bass_gain   = 1.5f;  // boost bass
    float mid_gain    = 1.0f;  // keep mids
    float treble_gain = 0.5f;  // cut treble
};

// audio recording parameters
const int SAMPLE_RATE       = 44100;
const int INPUT_CHANNELS    = 1;     // mono
const int SECONDS           = 15;    // recording duration
const int FRAMES_PER_BUFFER = 256;   // buffer size for PortAudio
const int FFT_SIZE          = 1024;  // must be power of 2

// file names
constexpr const char* RECORDING_FILENAME = "recording.wav";
constexpr const char* PROCESSED_FILENAME = "processed.wav";

// visualization settings
const int NUM_BARS        = 50;
const int MAX_BAR_HEIGHT  = 30;

#endif // CONFIG_H
