cmake_minimum_required(VERSION 3.15)

project(audio_processor)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(portaudio CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)

add_executable(audio_processor main.cpp)

target_link_libraries(audio_processor PRIVATE portaudio SndFile::sndfile)