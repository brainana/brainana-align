JIP_HOME = $(PWD)/../..
MACHINE = `uname -s`-`uname -m`
NIFTI = $(JIP_HOME)/nifticlib-2.0.0
CC = gcc

FMRI = fmri-glm.o glm.o
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include -I${NIFTI}/include
CWARN = -Wall
LIBFLAGS = -L$(JIP_HOME)/lib/$(MACHINE) -L$(NIFTI)/lib/$(MACHINE)
LIBRARIES = -lJIPFIT -lIO -lUTIL -lniftiio -lznz -lz -lm

fmri:   ${FMRI}
	${CC} -o jip-glm ${FMRI} ${LIBFLAGS} ${LIBRARIES}

fmri-glm.o: ../fmri-glm.c ../glm.c ../glm.h ../glm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../fmri-glm.c

glm.o:  ../glm.c ../glm.h ../glm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../glm.c

clean:
	rm jip-glm *.o
