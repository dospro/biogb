cmake_minimum_required(VERSION 3.3)
project(biogb)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_definitions(-DLINUX)
set(GRAPHICS_IMPLEMENTATION src/imp/video/cSDLDisplay.h src/imp/video/cSDLDisplay.cpp)
set(AUDIO_IMPLEMENTATION src/imp/audio/cSDLSound.h src/imp/audio/cSDLSound.cpp)
set(SOURCE_FILES
        ${GRAPHICS_IMPLEMENTATION}
        ${AUDIO_IMPLEMENTATION}
        src/main.cpp
        src/MemoryMap.cpp
        src/CPU/cCpu.cpp
        src/CPU/opcodes.cpp
        src/CPU/interpreter.cpp
        src/cDisplay.cpp
        src/cInput.cpp
        src/sound/cSound.cpp
        src/sound/cSoundChannel.cpp
        src/sound/cSoundChannel1.cpp
        src/sound/cSoundChannel3.cpp
        src/sound/cSoundChannel2.cpp
        src/sound/cSoundChannel4.cpp
        src/cTimer.cpp
        src/Memory/RomLoader.cpp )
add_executable(biogb ${SOURCE_FILES})
target_link_libraries(biogb ${SDL2_LIBRARY})

#-fsanitize=address -fstack-protector-all -g -ggdb