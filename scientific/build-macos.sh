#!/bin/sh
# Build the numerical engine and private worker from this Align source tree.
set -eu
align_science=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
align_arch=$(uname -m)
case "$align_arch" in arm64) align_triplet=arm-apple-darwin;; x86_64) align_triplet=x86_64-apple-darwin;; *) exit 1;; esac
mkdir -p "$align_science/.fftw-build" "$align_science/.fftw-runtime"
cd "$align_science/.fftw-build"
CFLAGS="-O2 -arch $align_arch -mmacosx-version-min=14.0" /bin/sh "$align_science/native/source/fftw-3.2.2/configure" --build="$align_triplet" --host="$align_triplet" --prefix="$align_science/.fftw-runtime" --disable-fortran --disable-dependency-tracking --enable-static --disable-shared --with-pic
make -j4
make install
cmake -S "$align_science/native/source" -B "$align_science/.native-build" -DJIP_BUILD_X11=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 -DCMAKE_OSX_ARCHITECTURES="$align_arch" -DFFTW_INCLUDE_DIR="$align_science/.fftw-runtime/include" -DFFTW_LIBRARY="$align_science/.fftw-runtime/lib/libfftw3.a"
cmake --build "$align_science/.native-build" -j4
ctest --test-dir "$align_science/.native-build" --no-tests=error --output-on-failure
cp "$align_science/.native-build/engine/libjip-engine.dylib" "$align_science/.native-build/engine/libjip-engine-repro.dylib" "$align_science/native/"
"$align_science/native-itk/build-macos.sh"
cd "$align_science/.."
python -m PyInstaller --clean --noconfirm --distpath generated --workpath scientific/.freeze scientific/Align-Registration.spec
