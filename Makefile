SRCDIR = src
SOURCES = ${SRCDIR}/main.cpp \
	${SRCDIR}/cDisplay.cpp \
	${SRCDIR}/cInput.cpp \
	${SRCDIR}/imp/video/cSDLDisplay.cpp \
	${SRCDIR}/cNet.cpp \
	${SRCDIR}/cSound.cpp \
	${SRCDIR}/cCpu.cpp \
	${SRCDIR}/MemoryMap.cpp \
	${SRCDIR}/imp/audio/cPortAudio.cpp
	
OBJS    = ${SOURCES:.c=.o}

COPTIONS  =-DLINUX -g -ggdb `sdl-config --cflags` -std=c++11
LOPTIONS   =`sdl-config --libs` -lportaudio
CC      = g++
PACKAGE = biogb

all : ${OBJS}
	${CC} -o ${PACKAGE} ${OBJS} ${COPTIONS} ${LOPTIONS}

clean:
	rm biogb
