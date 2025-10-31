cmake_minimum_required(VERSION 4.0)
project(biogb)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_definitions(-DLINUX)
set(GRAPHICS_IMPLEMENTATION src/imp/video/sdl_display.h src/imp/video/sdl_display.cpp)
set(AUDIO_IMPLEMENTATION src/imp/audio/sdl_audio.h src/imp/audio/sdl_audio.cpp)
set(SOURCE_FILES
        ${GRAPHICS_IMPLEMENTATION}
        ${AUDIO_IMPLEMENTATION}
        src/main.cpp
        src/memory/memory_map.cpp
        src/CPU/cpu.cpp
        src/CPU/opcodes.cpp
        src/CPU/interpreter.cpp
        src/display.cpp
        src/input.cpp
        src/sound/sound.cpp
        src/sound/sound_channel.cpp
        src/sound/sound_channel_1.cpp
        src/sound/sound_channel_3.cpp
        src/sound/sound_channel_2.cpp
        src/sound/sound_channel_4.cpp
        src/timer.cpp
        src/memory/rom_loader.cpp)
add_executable(biogb ${SOURCE_FILES})
target_link_libraries(biogb ${SDL2_LIBRARY})

#-fsanitize=address -fstack-protector-all -g -ggdb