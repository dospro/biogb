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
* RTC emulation for games using timers
* Input through hardcoded keyboard keys.

### Supported but broken or temporally removed

* **Save states**: This was supported previously, but the main redesign 
broke this feature. I'm also looking for a way to implement this with
smaller save files.
* **Gameshark / Gamegenie**: There was some rudimentary support of this 
device, but it was buggy and also broke after redesign.
* **Serial IO**: It was supported using SDL_Net, and it actually
worked, but was pretty inaccurate and buggy, so I removed support until 
more research is done for a more accurate implementation.

### Future support

* More configurable options.
* A nice GUI
* Be able to use a boot rom.

## Portability 

biogb has been tested on Windows, Linux and MacOS. At some point I was able to 
compile a binary for Dreamcast using KOS.

## Compatibility

As a matter of fact, game compatibility has been rather high even before the 
general cleanup. As of now, I haven't found any game that doesn't work with 
biogb. Yes, some test roms don't pass, but most of them are related to cycle
 accuracy, which mostly won't affect commercial games.

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

* You need a C++ compiler that supports c++23.

* The emulation code is mostly independent of SDL2. You can use any other library on top of biogb.
Look at the `main.cpp` file to see how to use biogb.


## History

26/12/2025
After years of silence, I finally worked on a bunch of updates. The most obvious
one is the update to C++23 standard. This helped me clean and refactor more parts
of the emulator. And with that came a bunch of bug fixes and improvements. Let's
list some of these achievements:

* BioGB now uses less memory!. It seems I have some bigger arrays than I needed.
* I found out that the frame-limiter code was super inaccurate, so I've rewritten it. 
Voilà, the experience is now way smoother. Even sound got better.
* I also found a little bug in MBC1 which prevented games like Lion King from working.
* I also included a lot of performance optimizations.
* There were some other minor bugs in some opcodes that I've fixed as well.
* Oh, another alluring one is the new color correction algorithm. It's way more accurate now.
* Also, the sound mixing algorithm was improved. It is a bit louder and more accurate now.
* I also fixed the RTC emulation, which was completely broken.
* And last but not least, SGB border emulation was added!

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
come from. I want biogb's code to be as self-explainable as possible, so 
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

Because it's a very portable, well-engineered library for abstracting
graphics, sound, input and the main window.

There was some time when you could conditionally compile biogb to use Allegro
but that was removed recently. To be honest, Allegro worked really good. 
It even outperformed SDL1.2, but I just wanted to maintain one library not
two. I found SDL easier to work with, so I stuck to it.

4. How accurate is biogb emulation?

TL;DR: pretty inaccurate. Long answer: I believe Game Boy was well-designed 
so games wouldn't need to rely on obscure, glitchy behaviors (like on the 
NES, for example). The result is that most commercial games work fine 
without emulating those obscure behaviors. Nevertheless, biogb will keep 
improving on accuracy.
