JIP_HOME = $(PWD)/../..
MACHINE = `uname -s`
NIFTI = $(JIP_HOME)/nifticlib-2.0.0
FFTW  = $(JIP_HOME)/fftw-3.2.2/$(MACHINE)
ZLIB  = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)
CC = cc

FMRI = fmri-rem.o rem.o
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include -I${NIFTI}/include -I$(FFTW)/include
CWARN = -Wall

LIBRARIES  = $(FFTW)/lib/libfftw3.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libIO.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libUTIL.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libJIPFIT.a \
	     $(NIFTI)/lib/$(MACHINE)/libniftiio.a \
	     $(NIFTI)/lib/$(MACHINE)/libznz.a \
	     $(ZLIB)/libz.a

fmri:   ${FMRI}
	${CC} -o jip-rem ${FMRI} ${LIBRARIES} -lm

fmri-rem.o: ../fmri-rem.c  ../rem.h
	${CC} -c ${CFLAGS} ${CWARN} ../fmri-rem.c

rem.o:  ../rem.c ../rem.h
	${CC} -c ${CFLAGS} ${CWARN} ../rem.c

clean:
	rm jip-rem *.o
