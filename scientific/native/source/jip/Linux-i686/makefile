JIP_HOME = $(PWD)/../..
MACHINE = `uname -s`-`uname -m`
NIFTI    = $(JIP_HOME)/nifticlib-2.0.0
FFTW     = $(JIP_HOME)/fftw-3.2.2/$(MACHINE)
# libz.a may already exist on the default seach path of many unix-type installations.
ZLIB     = $(JIP_HOME)/zlib-1.2.5/$(MACHINE)

JIP = jip-front.o jip-align.o jip-average.o jip-macros.o jip-magcor.o jip-math.o jip-transfilt.o jip-util.o jip-ovl.o jip-io.o jip-reorder.o jip-stats.o jip-glm.o jip-maps.o common-align.o glm.o
CC = cc
CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -std=gnu11 -fcommon -I$(JIP_HOME)/include -I$(JIP_HOME)/include/recipes -I$(JIP_HOME)/jip-glm -I$(FFTW)/include -I${NIFTI}/include -g
CWARN = -Wall
LIBFLAGS = -L$(JIP_HOME)/lib/$(MACHINE) -L$(FFTW)/lib -L$(NIFTI)/lib/$(MACHINE) -L$(ZLIB)
LIBRARIES = -lfftw3 -lIO -lUTIL -lJIPFIT -lniftiio -lznz -lz -lm

jip:    $(JIP)
	$(CC) -o jip $(JIP) $(LIBFLAGS) $(LIBRARIES)

jip-front.o:   $(JIP_HOME)/jip/jip-front.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-front.c
jip-align.o:   $(JIP_HOME)/jip/jip-align.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-align.c
jip-average.o: $(JIP_HOME)/jip/jip-average.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-average.c
jip-macros.o:  $(JIP_HOME)/jip/jip-macros.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-macros.c
jip-magcor.o:  $(JIP_HOME)/jip/jip-magcor.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-magcor.c
jip-math.o:    $(JIP_HOME)/jip/jip-math.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-math.c
jip-reorder.o: $(JIP_HOME)/jip/jip-reorder.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-reorder.c
jip-stats.o:   $(JIP_HOME)/jip/jip-stats.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-stats.c
jip-transfilt.o:$(JIP_HOME)/jip/jip-transfilt.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-transfilt.c
jip-util.o:    $(JIP_HOME)/jip/jip-util.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-util.c
jip-ovl.o:     $(JIP_HOME)/jip/jip-ovl.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-ovl.c
jip-io.o:      $(JIP_HOME)/jip/jip-io.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-io.c
jip-maps.o:    $(JIP_HOME)/jip/jip-maps.c $(JIP_HOME)/include/util.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-maps.c
jip-glm.o:     $(JIP_HOME)/jip/jip-glm.c $(JIP_HOME)/jip-glm/glm.h $(JIP_HOME)/jip/jip.h
	       $(CC) -c $(CFLAGS)  $(CWARN) $(JIP_HOME)/jip/jip-glm.c
glm.o:         $(JIP_HOME)/jip-glm/glm.c $(JIP_HOME)/jip-glm/glm.h
	       $(CC) $(CFLAGS) $(CWARN) -c $(JIP_HOME)/jip-glm/glm.c
common-align.o:   $(JIP_HOME)/jip-align/common-align.c $(JIP_HOME)/jip-align/common-align.h
	       $(CC) $(CFLAGS) $(CWARN) -I$(JIP_HOME)/jip-align -c $(JIP_HOME)/jip-align/common-align.c

clean:
	rm jip *.o
