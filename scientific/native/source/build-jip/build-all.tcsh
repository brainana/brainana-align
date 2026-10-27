#!/bin/tcsh
# From OS: source build-all.tcsh

source ../define-jip.tcsh 1

cd $JIP_HOME/build-jip
source build-zlib.tcsh

cd $JIP_HOME/build-jip
source build-nifti.tcsh

cd $JIP_HOME/build-jip
source build-fftw.tcsh

cd $JIP_HOME/build-jip
source build-jiplibs.tcsh

cd $JIP_HOME/build-jip
source build-jip.tcsh

cd $JIP_HOME/build-jip
