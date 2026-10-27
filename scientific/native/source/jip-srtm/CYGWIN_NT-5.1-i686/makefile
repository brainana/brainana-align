JIP_HOME = $(PWD)/../..
MACHINE = `uname -s`-`uname -m`
NIFTI = $(JIP_HOME)/nifticlib-2.0.0
CC = gcc

SRTM = srtm-glm.o srtm.o
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include -I${NIFTI}/include
CWARN = -Wall
LIBFLAGS = -L$(JIP_HOME)/lib/$(MACHINE) -L$(NIFTI)/lib/$(MACHINE)
LIBRARIES = -lJIPFIT -lIO -lUTIL -lniftiio -lznz -lz -lm

srtm:   ${SRTM}
	${CC} -o jip-srtm ${SRTM} ${LIBFLAGS} ${LIBRARIES}

srtm-glm.o: ../srtm-glm.c ../srtm.c ../srtm.h ../srtm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../srtm-glm.c

srtm.o:  ../srtm.c ../srtm.h ../srtm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../srtm.c

clean:
	rm srtm-glm *.o
