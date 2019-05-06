# BioGB

BioGB is a Game Boy/Game Boy Color emulator written in C++. It's main goals are:
1. Code should be clear and easy to read. 
2. Emulator should be as portable as possible.

## Features

### Fully supported

* Original DMG Game Boy emulation
* Game Boy Color emulation
* Full sound emulation
* Save RAM to file.
* RTC emulation for games using timers (still buggy)
* Input through hardcoded keyboard keys.

### Supported but broken or temporally removed

* **Save states**: This was supported previously but the main redesign 
broke this feature. I'm also looking for a way to implement this with
smaller save files.
* **Gameshark / Gamegenie**: There was some rudimentary support of this 
devices, but it was buggy and also broke after redesign.
* **Serial IO**: It was supported using SDL_Net and it actually
worked, but was pretty inaccurate and buggy so I removed support until 
more research is done for a more accurate implementation.

### Future support

* Super Game Boy frame emulation
* A nice GUI
* Be able to use a boot rom.

## Portability 

biogb has been tested on Windows, Linux and MacOS. At some point I was able to 
compile a binary for Dreamcast using KOS.

## Compatibility

As a matter of fact game compatibility has been rather high even before the 
general cleanup. There may be some roms that can present some glitches or even
crash. 

## Building

### Linux

Building biogb on linux is quite easy:

* You need a c++ compiler. Some linux distros come with gnu/gcc. 
Some may require you to install `build-essential` or similar package.

* You need SDL2 development libraries.

* You need at least `make`. I recommend using `cmake` instead.

* Run the following commands:

```
$ cmake .
$ make 
``` 

That's it.

### MacOS

The build steps are pretty much the same as Linux. The only main difference is the package manager:

* I recommend using Homebrew for installing SDL2 and cmake.
* You need to install Xcode so you have a toolchain to compile c++ code.

## Windows

There may be different ways to compile biogb for windows:

* Use a posix-like environment like cygwin or mingw32.
* Use cmake to generate a visual studio solution
* Create a visual studio solution manually
    * This means adding all files into a Visual Studio solution, setting SDL include and lib paths and compile.
    
## Other

biogb can be built for other platforms even if the platform is not supported by SDL2.
Here are a couple of things to take in mind when trying to build biogb for unsupported platforms.

* You need a C++ compiler that supports at least C++11. biogb uses C++14, but you can still use a compiler 
which only supports C++11. In that case you would need to manually implement a couple of things: template 
function `std::make_unique` and stl container `std::array<T, size>`.
I do plan to add those fills in the future, but for now you would have to manually implement them.

* Although biogb uses SDL2 for display, audio and input, the dependency was abstracted into components 
that you can find on the folder  `src/imp`. So if you want to build biogb using a different library than
SDL2 you can do the following:
    * Look at the SDL implementation (for example `src/imp/cSDLDisplay.h`)
    * Create a new class with your preferred display abstraction that inherits from `cDisplay`.
    * Implement the `updateScreen` method.
    * On `MemoryMap.cpp` use your new class instead of SDL2.
    * That's it.


## History

BioGB started long ago with the only goal to learn coding and how hardware 
works from the inside. In the beginning I used a weird mix of C and C++ and 
very bad coding practices. This led to a quite glitchy and buggy emulator.
So after that I decided to start a code cleaning effort. Some things I've 
done include:

* Improving general design: Remove global variables, refactor big classes and functions, etc.
* Move to a more idiomatic C++14: Use RAII pattern, smart pointers, etc.
* Reduce dependencies: Only depend on SDL2
* Refactor, refactor, refactor.

With this ongoing effort, I've been able to find and fix bugs, isolate 
different modules, and improve emulation. Here is where the two main goals
come from. I want biogb's code to be as self explainable as possible, so 
it's easy to read and maintain.


## FAQ

1. Why another Game Boy emulator?

First of all, when I first started this project, there weren't that may 
Game Boy emulators and most of them were close-source. Now, I chose 
Game Boy for more personal reason: I really like the Game Boy, so I wished
to know how that small handheld was able to run very amazing games.

2. Why use C++?

At first the project was made almost entirely on C. I even used Windows native
WinAPI to do some GUI. Some time later I discovered C++, and I used the emulator
as an excuse to learn C++. Bottom line. It worked. 

Answering the question. I used C because that was pretty much the only 
programming language I knew at the moment.

3. Why use SDL?

Because it's a very portable, well engineered library for abstracting
graphics, sound, input and the main window.

There was some time when you could conditionally compile biogb to use Allegro
but that was removed recently. To be honest, Allegro worked really good. 
It even outperformed SDL1.2, but I just wanted to maintain one library not
two. So I found SDL easier to work with, so I sicked to it.

4. How accurate is biogb emulation?

TL;DR: pretty inaccurate. Long answer: I believe Game Boy was well designed 
so games wouldn't need to rely on obscure, glitchy behaviors (like on the 
NES, for example). The result is that most commercial games work fine 
without emulating those obscure behaviors. Nevertheless, biogb will keep 
improving on accuracy.
