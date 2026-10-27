#!/bin/tcsh
# From OS: source build-nifti.tcsh

source ../define-jip.tcsh 1
set OS = `uname -s`
set ARCH = `uname -m`

echo "---------- definitions ----------"
echo "JIP_HOME = $JIP_HOME"
echo "OS       = $OS"
echo "ARCH     = $ARCH"
echo "---------------------------------"

# nifti
echo "---------------------------------"
echo "Create NIFTI c libraries"
echo "---------------------------------"
cd $JIP_HOME/nifticlib-2.0.0
make clean
if ( $OS == Darwin ) then
   make ARCH=Darwin
   mv lib/*.a lib/Darwin
else
   make
   if ( !(-f $OS-$ARCH) ) then
      mkdir $OS-$ARCH
   endif
   mv lib/*.a lib/$OS-$ARCH
endif
make clean

cd $JIP_HOME/build-jip

