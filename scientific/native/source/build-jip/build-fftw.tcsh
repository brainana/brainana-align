#!/bin/tcsh
# From OS: source build-fftw.tcsh

source ../define-jip.tcsh 1
set OS = `uname -s`
set ARCH = `uname -m`

echo "---------- definitions ----------"
echo "JIP_HOME = $JIP_HOME"
echo "OS       = $OS"
echo "ARCH     = $ARCH"
echo "---------------------------------"

# fftw
echo "---------------------------------"
echo "Create FFTW libraries"
echo "---------------------------------"
cd $JIP_HOME/fftw-3.2.2
make distclean
if ( $OS == Darwin ) then
   setenv CC "gcc -arch i386 -arch x86_64 -arch ppc"
   setenv CXX "g++ -arch i386 -arch x86_64 -arch ppc"
   setenv CPP "gcc -E"
   setenv CXXCPP "g++ -E"  
   ./configure --prefix=$JIP_HOME/fftw-3.2.2/Darwin
   make install
else
   if ( !(-f $JIP_HOME/fftw-3.2.2/$OS-$ARCH) ) then
      mkdir $JIP_HOME/fftw-3.2.2/$OS-$ARCH
   endif
   ./configure --prefix=$JIP_HOME/fftw-3.2.2/$OS-$ARCH
   make install
endif
make distclean

cd $JIP_HOME/build-jip
