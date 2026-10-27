# The following macros to describe the C compiler, and options
# to the C compiler for use on the host systsem.
#
CC = gcc

JIP_HOME = $(PWD)/../..
MACHINE = `uname -s`-`uname -m`
NIFTI    = $(JIP_HOME)/nifticlib-2.0.0
FFTW     = $(JIP_HOME)/fftw-3.2.2/$(MACHINE)
# libz.a may already exist on the default seach path of many unix-type installations.
ZLIB     = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)

# Define where the X11 library can be found. (You may need to change this on
# your host system if you get complaints about not being able to find libX11).
X11INCLUDE = /usr/X11R6/include
X11LIBDIR = /usr/X11R6/lib

OBJECTS = display-Xwindows.o common-display.o common-X11.o common-actions.o common-io.o display-io.o display-actions.o display-util.o display-time-model.o display-glm.o display-rem.o display-srtm.o glm.o rem.o srtm.o

CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I$(JIP_HOME)/include -I$(JIP_HOME)/jip-display/common -I$(JIP_HOME)/jip-glm -I$(JIP_HOME)/jip-rem -I$(JIP_HOME)/jip-srtm -I$(FFTW)/include -I${NIFTI}/include -g
CWARN = -Wall
LIBFLAGS = -L$(JIP_HOME)/lib/$(MACHINE) -L$(FFTW)/lib -L$(NIFTI)/lib/$(MACHINE) -L$(ZLIB) -L$(X11LIBDIR) 
LIBRARIES = -lfftw3 -lUTIL -lIO -lJIPFIT -lX11 -lniftiio -lznz -lz -lm

display : $(OBJECTS)
	$(CC) -o jip-display $(OBJECTS) $(LIBFLAGS) $(LIBRARIES)

display-io.o : ../display-io.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-io.c

display-util.o : ../display-util.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) -I../../lib/lmmin ../display-util.c

common-display.o : ../common/common-display.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../common/common-display.c

common-X11.o : ../common/common-X11.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../common/common-X11.c

common-actions.o : ../common/common-actions.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../common/common-actions.c

common-io.o : ../common/common-io.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../common/common-io.c

display-actions.o : ../display-actions.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-actions.c

display-Xwindows.o : ../display-Xwindows.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-Xwindows.c

display-time-model.o : ../display-time-model.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-time-model.c

display-glm.o : ../display-glm.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-glm.c

display-rem.o : ../display-rem.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-rem.c

display-srtm.o : ../display-srtm.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-srtm.c

glm.o	: ../../jip-glm/glm.c ../../jip-glm/glm.h
	$(CC) -c $(CFLAGS) $(CWARN) ../../jip-glm/glm.c

rem.o	: ../../jip-rem/rem.c ../../jip-rem/rem.h
	$(CC) -c $(CFLAGS) $(CWARN) ../../jip-rem/rem.c

srtm.o	: ../../jip-srtm/srtm.c ../../jip-srtm/srtm.h
	$(CC) -c $(CFLAGS) $(CWARN) ../../jip-srtm/srtm.c

clean	:
	rm jip-display *.o
