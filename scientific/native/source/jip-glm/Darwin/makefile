JIP_HOME = ../..
MACHINE = `uname -s`
NIFTI = $(JIP_HOME)/nifticlib-2.0.0
ZLIB  = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)
CC = cc

FMRI = fmri-glm.o glm.o
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include -I${NIFTI}/include
CWARN = -Wall

LIBRARIES  = $(JIP_HOME)/lib/$(MACHINE)/libIO.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libUTIL.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libJIPFIT.a \
	     $(NIFTI)/lib/$(MACHINE)/libniftiio.a \
	     $(NIFTI)/lib/$(MACHINE)/libznz.a \
	     $(ZLIB)/libz.a

fmri:   ${FMRI}
	${CC} -o jip-glm ${CWARN} ${FMRI} ${LIBRARIES} -lm

fmri-glm.o: ../fmri-glm.c ../glm.c ../glm.h ../glm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../fmri-glm.c

glm.o:  ../glm.c ../glm.h ../glm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../glm.c

clean:
	rm jip-glm *.o
