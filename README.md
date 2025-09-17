# audio-processor

This C++ project provides functionality to record audio, visualize recordings in real time, and process audio using techniques like equalization.

- [PortAudio](http://www.portaudio.com/) for cross-platform audio input/output.
- [libsndfile](https://github.com/libsndfile/libsndfile) for writing audio data to WAV files.
- [vcpkg](https://github.com/microsoft/vcpkg) for managing dependencies on Windows.

## Audio Visualization

The audio signal is transformed into the frequency domain using Hanning windowing and a Fast Fourier Transform (FFT), and resulting frequency components are displayed in the terminal as scaled horizontal bars in your terminal.

## Audio Processing

Equalization is performed in the frequency domain by adjusting the amplitudes of specific frequency bands, and the modified signal is reconstructed in the time domain using an inverse FFT. Equalization settings are specified in config.h.

## Dependencies
Install dependencies via vcpkg:

```cmd
vcpkg install portaudio
vcpkg install libsndfile
```

## Build
```cmd
cmake -S . -B -DCMAKE_TOOLCHAIN_FILE=[insert-vcpkg-root]/buildsystems/vcpkg.cmake
cmake --build .
```

## Run
For recording and visualizing audio, run the record_and_visualize.exe executable

```cmd
./audio-processor/build/Release/record_and_visualize.exe
```

For applying audio equalization, run the process_audio.exe executable.

```cmd
./audio-processor/build/Release/process_audio.exe
```