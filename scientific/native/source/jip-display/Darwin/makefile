#
# The following macros to describe the C compiler, and options
# to the C compiler for use on the host systsem.
#
CC = cc

#
# Define where the X11 library can be found. (You may need to change this on
# your host system if you get complaints about not being able to find libX11).
#
JIP_HOME = ../..
MACHINE = `uname -s`
NIFTI    = $(JIP_HOME)/nifticlib-2.0.0
FFTW     = $(JIP_HOME)/fftw-3.2.2/$(MACHINE)
ZLIB     = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)


X11LIBDIR = /usr/X11R6/lib
#
# Define the compiler flags.
#
X11INCLUDE = /usr/X11R6/include
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I$(X11INCLUDE) -I.. -I$(JIP_HOME)/include -I$(JIP_HOME)/jip-display/common -I$(JIP_HOME)/jip-glm -I$(JIP_HOME)/jip-rem -I$(JIP_HOME)/jip-srtm -I$(FFTW)/include -I${NIFTI}/include -g


CWARN = -Wall

OBJECTS = display-Xwindows.o common-display.o common-X11.o common-actions.o common-io.o display-io.o display-actions.o display-util.o display-time-model.o display-glm.o display-rem.o display-srtm.o glm.o rem.o srtm.o

LIBRARIES  = $(FFTW)/lib/libfftw3.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libIO.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libUTIL.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libJIPFIT.a \
	     $(NIFTI)/lib/$(MACHINE)/libniftiio.a \
	     $(NIFTI)/lib/$(MACHINE)/libznz.a \
	     $(ZLIB)/libz.a

display : $(OBJECTS)
	$(CC) -o jip-display $(OBJECTS) $(LIBRARIES) -L$(X11LIBDIR) -lX11 -lm

display-io.o : ../display-io.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../display-io.c

display-util.o : ../display-util.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -I../../lib/lmmin  -c ../display-util.c

common-display.o : ../common/common-display.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../common/common-display.c

common-X11.o : ../common/common-X11.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../common/common-X11.c

common-actions.o : ../common/common-actions.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../common/common-actions.c

common-io.o : ../common/common-io.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../common/common-io.c

display-actions.o : ../display-actions.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../display-actions.c

display-Xwindows.o : ../display-Xwindows.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../display-Xwindows.c

display-time-model.o : ../display-time-model.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../display-time-model.c

display-glm.o : ../display-glm.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../display-glm.c

display-rem.o : ../display-rem.c ../display.h ../display-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c ../display-rem.c

display-srtm.o : ../display-srtm.c ../display.h ../display-prototypes.h
	$(CC) -c $(CFLAGS) $(CWARN) ../display-srtm.c

glm.o	: ../../jip-glm/glm.c ../../jip-glm/glm.h
	$(CC) $(CFLAGS) $(CWARN) -c ../../jip-glm/glm.c

rem.o	: ../../jip-rem/rem.c ../../jip-rem/rem.h
	$(CC) $(CFLAGS) $(CWARN) -c ../../jip-rem/rem.c

srtm.o	: ../../jip-srtm/srtm.c ../../jip-srtm/srtm.h
	$(CC) -c $(CFLAGS) $(CWARN) ../../jip-srtm/srtm.c

clean	:
	rm jip-display *.o
