cmake_minimum_required(VERSION 3.3)
project(biogb)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_definitions(-DLINUX)
set(GRAPHICS_IMPLEMENTATION src/imp/video/cSDLDisplay.h src/imp/video/cSDLDisplay.cpp)
set(AUDIO_IMPLEMENTATION src/imp/audio/cSDLSound.h src/imp/audio/cSDLSound.cpp)
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
        src/sound/cSound.h
        src/sound/cSound.cpp
        src/sound/cSoundChannel.cpp
        src/sound/cSoundChannel.h
        src/sound/cSoundChannel1.cpp
        src/sound/cSoundChannel1.h
        src/sound/cSoundChannel3.cpp
        src/sound/cSoundChannel3.h
        src/sound/cSoundChannel2.cpp
        src/sound/cSoundChannel2.h
        src/sound/cSoundChannel4.cpp
        src/sound/cSoundChannel4.h
        src/cTimer.cpp
        src/cTimer.h
        src/cInterrupts.cpp
        src/cInterrupts.h
        ${GRAPHICS_IMPLEMENTATION}
        ${AUDIO_IMPLEMENTATION})
add_executable(biogb ${SOURCE_FILES})
target_link_libraries(biogb ${SDL2_LIBRARY})

#-fsanitize=address -fstack-protector-all -g -ggdb