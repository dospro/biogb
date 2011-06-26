SRCDIR = src
SOURCES = ${SRCDIR}/main.cpp \
	${SRCDIR}/cGfx.cpp \
	${SRCDIR}/cInput.cpp \
	${SRCDIR}/cLibGfx.cpp \
	${SRCDIR}/cNet.cpp \
	${SRCDIR}/cSound.cpp \
	${SRCDIR}/cCpu.cpp 
	
OBJS    = ${SOURCES:.c=.o}
HDRS	= cSound.h tables.h cCpu.h cGfx.h cInput.h cLibGfx.h cNet.h

COPTIONS  =-DLINUX -DUSE_SDL -g `sdl-config --cflags`
#COPTIONS  =-DLINUX -DUSE_ALLEGRO -lalleg
LOPTIONS   =`sdl-config --libs`
CC      = g++
PACKAGE = biogb

all : ${OBJS}
	${CC} -o ${PACKAGE} ${OBJS} ${COPTIONS} ${LOPTIONS}

.c.o:
	${CC} ${COPTIONS} ${LOPTIONS} -c $<
