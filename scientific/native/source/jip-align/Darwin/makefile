# The following macros to describe the C compiler, and options
# to the C compiler for use on the host systsem.
#
JIP_HOME = $(PWD)/../..
#CC = gcc
CC = cc

#
# Define where the X11 library can be found. (You may need to change this on
# your host system if you get complaints about not being able to find libX11).
#
MACHINE = `uname -s`
SYSTEMLIBDIR = /usr/lib
X11LIBDIR = /usr/X11R6/lib
NIFTI     = $(JIP_HOME)/nifticlib-2.0.0
FFTW      = $(JIP_HOME)/fftw-3.2.2/$(MACHINE)
ZLIB      = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)
#
# Define the compiler flags.
#
X11INCLUDE = /usr/X11R6/include
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -g -I$(JIP_HOME)/include -I.. -I../../jip-display/common -I$(X11INCLUDE) -I${NIFTI}/include -I$(FFTW)/include
CWARN = -Wall
#CWARN = -Wimplicit -Wunused -Wformat

OBJECTS = align-Xwindows.o common-display.o common-X11.o common-actions.o common-io.o align-actions.o align-util.o common-align.o align-io.o align-affine.o align-nonlinear.o

LIBRARIES  = $(FFTW)/lib/libfftw3.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libIO.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libUTIL.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libJIPFIT.a \
	     $(NIFTI)/lib/$(MACHINE)/libniftiio.a \
	     $(NIFTI)/lib/$(MACHINE)/libznz.a \
	     $(ZLIB)/libz.a

jip-align : $(OBJECTS)
	$(CC) $(CFLAGS) -o jip-align  $(OBJECTS) $(LIBRARIES) -L$(X11LIBDIR) -lX11 -lm

align-Xwindows.o : $(JIP_HOME)/jip-align/align-Xwindows.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/align-Xwindows.c

align-util.o : $(JIP_HOME)/jip-align/align-util.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/align-util.c

common-align.o : $(JIP_HOME)/jip-align/common-align.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/common-align.c

align-actions.o : $(JIP_HOME)/jip-align/align-actions.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/align-actions.c

align-io.o : $(JIP_HOME)/jip-align/align-io.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/align-io.c

align-affine.o : $(JIP_HOME)/jip-align/align-affine.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/align-affine.c

align-nonlinear.o : $(JIP_HOME)/jip-align/align-nonlinear.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-align/align-nonlinear.c

common-display.o : $(JIP_HOME)/jip-display/common/common-display.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-display/common/common-display.c

common-X11.o : $(JIP_HOME)/jip-display/common/common-X11.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-display/common/common-X11.c

common-actions.o : $(JIP_HOME)/jip-display/common/common-actions.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-display/common/common-actions.c

common-io.o : $(JIP_HOME)/jip-display/common/common-io.c $(JIP_HOME)/jip-align/align.h $(JIP_HOME)/jip-align/align-prototypes.h
	$(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-display/common/common-io.c
clean:
	rm *.o jip-align
