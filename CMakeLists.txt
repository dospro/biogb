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
        src/macros.h
        src/MemoryMap.h
        src/MemoryMap.cpp
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
        #src/Memory/Rom.cpp src/Memory/Rom.h src/Memory/Ram.cpp src/Memory/Ram.h src/Memory/MemoryController.cpp src/Memory/MemoryController.h src/Memory/MBC1.cpp src/Memory/MBC1.h src/Memory/RomOnly.cpp src/Memory/RomOnly.h src/Memory/MBC5.cpp src/Memory/MBC5.h)
        src/Memory/RomLoader.cpp
        src/Memory/RomLoader.h src/Exceptions.h)
add_executable(biogb ${SOURCE_FILES})
target_link_libraries(biogb ${SDL2_LIBRARY})

#-fsanitize=address -fstack-protector-all -g -ggdb