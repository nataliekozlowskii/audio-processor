# audio-processor

This C++ project records audio from your microphone, processes it in real time, visualizes frequency data in the terminal, and allows saving recordings to WAV files. It uses:

- [PortAudio](http://www.portaudio.com/) for cross-platform audio input/output.
- [libsndfile](https://github.com/libsndfile/libsndfile) for writing audio data to WAV files.
- [vcpkg](https://github.com/microsoft/vcpkg) for managing dependencies on Windows.

## Audio Visualization

A Fast Fourier Transform with Hanning windowing is used to decompose audio into frequencies, which are then printed to the terminal as scaled horizontal bars.

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