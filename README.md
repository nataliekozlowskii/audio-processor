# audio-processor

This C++ project records audio from your microphone, processes it in real time, applies equalization, visualizes frequency data in the terminal, and allows saving recordings to WAV files. It uses:

- [PortAudio](http://www.portaudio.com/) for cross-platform audio input/output.
- [libsndfile](https://github.com/libsndfile/libsndfile) for writing audio data to WAV files.
- [vcpkg](https://github.com/microsoft/vcpkg) for managing dependencies on Windows.

## Audio Visualization

The audio signal is transformed into the frequency domain using Hanning windowing and a Fast Fourier Transform (FFT). The resulting frequency components are displayed in the terminal as scaled horizontal bars.

## Audio Processing

Equalization is performed in the frequency domain by adjusting the amplitudes of specific frequency bands. The modified signal is then reconstructed in the time domain using an inverse FFT.

## Dependencies
Install dependencies via vcpkg:

```cmd
vcpkg install portaudio
vcpkg install libsndfile
```

## Build
```cmd
mkdir build
cd build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[insert-vcpkg-root]/buildsystems/vcpkg.cmake
cmake --build .
```

## Run
```cmd
./audio-processor
```