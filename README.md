# audio-processor

This is a C++ audio processing project that records audio from your microphone and writes it to a WAV file. It uses:

- [PortAudio](http://www.portaudio.com/) for cross-platform audio input/output.
- [libsndfile](https://github.com/libsndfile/libsndfile) for writing audio data to WAV files.
- [vcpkg](https://github.com/microsoft/vcpkg) for managing dependencies on Windows.

## Dependencies
Install dependencies via vcpkg:

```cmd
vcpkg install portaudio
vcpkg install libsndfile