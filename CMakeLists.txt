cmake_minimum_required(VERSION 3.3)
project(biogb)
find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lportaudio")
add_definitions(-DLINUX)
set(GRAPHICS_IMPLEMENTATION src/imp/video/cSDLDisplay.h src/imp/video/cSDLDisplay.cpp)
set(AUDIO_IMPLEMENTATION src/imp/audio/cPortAudio.h src/imp/audio/cPortAudio.cpp)
set(SOURCE_FILES
        src/main.cpp
        src/tables.h
        src/macros.h
        src/cMemory.h
        src/cMemory.cpp
        src/cCpu.h
        src/cCpu.cpp
        src/cDisplay.h
        src/cDisplay.cpp
        src/cInput.h
        src/cInput.cpp
        src/cSound.h
        src/cSound.cpp
        src/cSoundChannel.cpp
        src/cSoundChannel.h
        src/cSoundChannel1.cpp
        src/cSoundChannel1.h
        src/cSoundChannel3.cpp
        src/cSoundChannel3.h
        ${GRAPHICS_IMPLEMENTATION}
        ${AUDIO_IMPLEMENTATION} src/cSoundChannel2.cpp src/cSoundChannel2.h src/cSoundChannel4.cpp src/cSoundChannel4.h)
add_executable(biogb ${SOURCE_FILES})
target_link_libraries(biogb ${SDL_LIBRARY})
#-fsanitize=address -fstack-protector-all -g -ggdb