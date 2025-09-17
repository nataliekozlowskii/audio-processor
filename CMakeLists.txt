cmake_minimum_required(VERSION 3.15)

project(audio_processor)

find_package(portaudio CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)

# record and visualize executable
add_executable(record_and_visualize
    record_and_visualize.cpp
    fft.cpp
    signal_processing.cpp
    config.cpp
)

target_link_libraries(record_and_visualize PRIVATE portaudio SndFile::sndfile)
target_include_directories(record_and_visualize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# audio processing executible
add_executable(process_audio
    process.cpp
    fft.cpp
    signal_processing.cpp
    config.cpp
)
target_link_libraries(process_audio PRIVATE portaudio SndFile::sndfile)
target_include_directories(process_audio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})