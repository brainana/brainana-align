#!/bin/tcsh
# From OS: source build-zlib.tcsh

source ../define-jip.tcsh 1
set OS = `uname -s`
set ARCH = `uname -m`

echo "---------- definitions ----------"
echo "JIP_HOME = $JIP_HOME"
echo "OS       = $OS"
echo "ARCH     = $ARCH"
echo "---------------------------------"

# zlib
echo "---------------------------------"
echo "Create zlib"
echo "---------------------------------"
cd $JIP_HOME/zlib-1.2.5
make distclean
if ( $OS == Darwin ) then
   setenv CFLAGS "-arch i386 -arch x86_64 -arch ppc"; ./configure
   make
   mv libz.a Darwin
else
   ./configure
   make
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
   endif
   mv libz.a $OS-$ARCH
endif
make distclean

cd $JIP_HOME/build-jip
