cmake_minimum_required(VERSION 3.20)
project(hum2sol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hum2sol
    src/main.cpp
    src/audio/wav_reader.cpp
    src/dsp/yin.cpp
    src/music/pitch.cpp
    src/music/solfege.cpp
    src/music/segment.cpp
    src/music/smooth.cpp
)

target_include_directories(hum2sol PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

if (MSVC)
    target_compile_options(hum2sol PRIVATE /W4 /permissive-)
else()
    target_compile_options(hum2sol PRIVATE -Wall -Wextra -Wpedantic)
endif()
