#!/bin/tcsh
# From OS: source build-jiplibs.tcsh

source ../define-jip.tcsh 1
set OS = `uname -s`
set ARCH = `uname -m`

echo "---------- definitions ----------"
echo "JIP_HOME = $JIP_HOME"
echo "OS       = $OS"
echo "ARCH     = $ARCH"
echo "---------------------------------"

echo "---------------------------------"
echo "make jip libraries"
echo "---------------------------------"
if ( $OS == Darwin ) then
   cd $JIP_HOME/lib/$OS
   make clean
   make util; make io; make jipfit; /bin/rm *.o
else
   if ( !(-f $JIP_HOME/lib/$OS-$ARCH) ) then
      mkdir $JIP_HOME/lib/$OS-$ARCH
      cp $JIP_HOME/lib/Linux-x86_64/makefile $JIP_HOME/lib/$OS-$ARCH
   endif
   cd $JIP_HOME/lib/$OS-$ARCH
   make clean
   make util; make io; make jipfit; /bin/rm *.o
endif

cd $JIP_HOME/build-jip
