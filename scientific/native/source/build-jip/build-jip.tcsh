#!/bin/tcsh
# From OS: source build-jip.tcsh

source ../define-jip.tcsh 1
set OS = `uname -s`
set ARCH = `uname -m`

echo "---------- definitions ----------"
echo "JIP_HOME = $JIP_HOME"
echo "OS       = $OS"
echo "ARCH     = $ARCH"
echo "---------------------------------"

# jip
echo "---------------------------------"
echo "make jip executables"
echo "---------------------------------"

echo "---------------------------------"
echo "make jip"
echo "---------------------------------"
cd $JIP_HOME/jip
if ( $OS == Darwin ) then
   cd Darwin
   pwd
   make clean; make; /bin/rm *.o
else
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
      cp Linux-x86_64/makefile $OS-$ARCH
   endif
   cd $OS-$ARCH
   pwd
   make clean; make; /bin/rm *.o
endif

# jip-display
echo "--------------------------------------------"
echo "make jip-display"
echo "--------------------------------------------"
cd $JIP_HOME/jip-display
if ( $OS == Darwin ) then
   cd Darwin
   pwd
   make clean; make; /bin/rm *.o
else
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
      cp Linux-x86_64/makefile $OS-$ARCH
   endif
   cd $OS-$ARCH
   pwd
   make clean; make; /bin/rm *.o
endif

# jip-align
echo "--------------------------------------------"
echo "make jip-align"
echo "--------------------------------------------"
cd $JIP_HOME/jip-align
if ( $OS == Darwin ) then
   cd Darwin
   pwd
   make clean; make; /bin/rm *.o
else
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
      cp Linux-x86_64/makefile $OS-$ARCH
   endif
   cd $OS-$ARCH
   pwd
   make clean; make; /bin/rm *.o
endif

# jip-glm
echo "--------------------------------------------"
echo "make jip-glm"
echo "--------------------------------------------"
cd $JIP_HOME/jip-glm
if ( $OS == Darwin ) then
   cd Darwin
   pwd
   make clean; make; /bin/rm *.o
else
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
      cp Linux-x86_64/makefile $OS-$ARCH
   endif
   cd $OS-$ARCH
   pwd
   make clean; make; /bin/rm *.o
endif

# jip-srtm
echo "--------------------------------------------"
echo "make jip-srtm"
echo "--------------------------------------------"
cd $JIP_HOME/jip-srtm
if ( $OS == Darwin ) then
   cd Darwin
   pwd
   make clean; make; /bin/rm *.o
else
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
      cp Linux-x86_64/makefile $OS-$ARCH
   endif
   cd $OS-$ARCH
   pwd
   make clean; make; /bin/rm *.o
endif

# jip-rem
echo "--------------------------------------------"
echo "make jip-rem"
echo "--------------------------------------------"
cd $JIP_HOME/jip-rem
if ( $OS == Darwin ) then
   cd Darwin
   pwd
   make clean; make; /bin/rm *.o
else
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
      cp Linux-x86_64/makefile $OS-$ARCH
   endif
   cd $OS-$ARCH
   pwd
   make clean; make; /bin/rm *.o
endif

cd $JIP_HOME/build-jip
