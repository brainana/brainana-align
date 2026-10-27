JIP_HOME = $(PWD)/../..
CC = cc
CFLAGS   = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I${JIP_HOME}/include
CWARN    = -Wall
NIFTI    = ${JIP_HOME}/nifticlib-2.0.0

jipfit:  jip-fit.o lmmin.o lm_eval.o
	ar rv libJIPFIT.a lmmin.o lm_eval.o jip-fit.o
	ranlib libJIPFIT.a
	rm *.o
jip-fit.o: ${JIP_HOME}/lib/jip-fit.c ${INCLUDES}  ${JIP_HOME}/lib/lmmin/lmmin.h
	${CC} -c ${CFLAGS} ${CWARN} ${JIP_HOME}/lib/jip-fit.c -I${JIP_HOME}/lib/lmmin
lmmin.o: ${JIP_HOME}/lib/lmmin/lmmin.c ${JIP_HOME}/lib/lmmin/lmmin.h
	${CC} -c ${JIP_HOME}/lib/lmmin/lmmin.c ${CWARN} -I${JIP_HOME}/lib/lmmin
lm_eval.o: ${JIP_HOME}/lib/lmmin/lm_eval.c ${JIP_HOME}/lib/lmmin/lmmin.h ${JIP_HOME}/lib/lmmin/lm_eval.h
	${CC} -c ${JIP_HOME}/lib/lmmin/lm_eval.c ${CWARN} -I${JIP_HOME}/lib/lmmin

util:  util.o
	ar rv libUTIL.a util.o
	ranlib libUTIL.a
	rm *.o
util.o: ${JIP_HOME}/lib/util.c ${JIP_HOME}/include/util.h ${NIFTI}/include/nifti1.h
	${CC} -c ${JIP_HOME}/lib/util.c ${CFLAGS} ${CWARN} -I${NIFTI}/include

io:  io.o
	ar rv libIO.a io.o
	ranlib libIO.a
	rm *.o
io.o: ${JIP_HOME}/lib/io.c ${JIP_HOME}/include/util.h ${NIFTI}/include/nifti1.h
	${CC} -c ${JIP_HOME}/lib/io.c ${CFLAGS} ${CWARN} -I${NIFTI}/include

niftiio: nifti1_io.o
	ar rv libNIFTIIO.a nifti1_io.o
	ranlib libNIFTIIO.a
	rm *.o
nifti1_io.o: ${NIFTI}/niftilib/nifti1_io.c ${NIFTI}/niftilib/nifti1_io.h
	${CC} -c ${NIFTI}/niftilib/nifti1_io.c ${CFLAGS} ${CWARN} -I${NIFTI}/include -I${NIFTI}/niftilib
all:	util.o util io.o io jip-fit.o lmmin.o lm_eval.o jipfit
clean:
	rm *.o lib*.a
