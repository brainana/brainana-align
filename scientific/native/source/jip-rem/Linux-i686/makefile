JIP_HOME = $(PWD)/../..
MACHINE  = `uname -s`-`uname -m`
NIFTI    = $(JIP_HOME)/nifticlib-2.0.0
FFTW     = $(JIP_HOME)/fftw-3.2.2/$(MACHINE)
ZLIB     = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)
CC = gcc

FMRI = fmri-rem.o rem.o
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include -I${NIFTI}/include -I$(FFTW)/include
CWARN = -Wall
LIBFLAGS = -L$(JIP_HOME)/lib/$(MACHINE) -L$(NIFTI)/lib/$(MACHINE) -L$(FFTW)/lib -L$(ZLIB)
LIBRARIES = -lfftw3 -lJIPFIT -lIO -lUTIL -lniftiio -lznz -lz -lm

fmri:   ${FMRI}
	${CC} -o jip-rem ${FMRI} ${LIBFLAGS} ${LIBRARIES}

fmri-rem.o: ../fmri-rem.c  ../rem.h
	${CC} -c ${CFLAGS} ${CWARN} ../fmri-rem.c

rem.o:  ../rem.c ../rem.h
	${CC} -c ${CFLAGS} ${CWARN} ../rem.c

clean:
	rm jip-rem *.o
