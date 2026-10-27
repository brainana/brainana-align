JIP_HOME = ../..
MACHINE = `uname -s`
NIFTI = $(JIP_HOME)/nifticlib-2.0.0
ZLIB  = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)
CC = cc

SRTM = srtm-glm.o srtm.o
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include -I${NIFTI}/include
CWARN = -Wall

LIBRARIES  = $(JIP_HOME)/lib/$(MACHINE)/libIO.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libUTIL.a \
	     $(JIP_HOME)/lib/$(MACHINE)/libJIPFIT.a \
	     $(NIFTI)/lib/$(MACHINE)/libniftiio.a \
	     $(NIFTI)/lib/$(MACHINE)/libznz.a \
	     $(ZLIB)/libz.a

srtm:   ${SRTM}
	${CC} -o jip-srtm ${SRTM} ${LIBFLAGS} ${LIBRARIES}

srtm-glm.o: ../srtm-glm.c ../srtm.c ../srtm.h ../srtm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../srtm-glm.c

srtm.o:  ../srtm.c ../srtm.h ../srtm-prototypes.h
	${CC} -c ${CFLAGS} ${CWARN} ../srtm.c

clean:
	rm srtm-glm *.o
