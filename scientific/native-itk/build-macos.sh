#!/bin/sh
# Offline reproducible ITK build. All required upstream source is in vendor/.
set -eu
align_itk_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
align_science=$(dirname "$align_itk_root")
python - "$align_itk_root" <<'PY'
from pathlib import Path
import hashlib,json,sys,tarfile
root=Path(sys.argv[1]);pin=json.loads((root/'UPSTREAM.json').read_text());archive=root/'vendor'/pin['archive']
assert hashlib.sha256(archive.read_bytes()).hexdigest()==pin['sha256'],'ITK archive checksum mismatch'
target=root.parent/'.itk-source';target.mkdir(exist_ok=True)
if not (target/'InsightToolkit-5.4.6').is_dir():
    with tarfile.open(archive) as stream:stream.extractall(target,filter='data')
PY
# CMake caches remember absolute source paths. Recreate only our generated
# caches when rebuilding a source tree moved to another checkout.
if [ -f "$align_science/.itk-static/CMakeCache.txt" ] && ! grep -Fqx "CMAKE_HOME_DIRECTORY:INTERNAL=$align_science/.itk-source/InsightToolkit-5.4.6" "$align_science/.itk-static/CMakeCache.txt"; then
  rm -rf "$align_science/.itk-static" "$align_science/.itk-static-adapter"
fi
cmake -S "$align_science/.itk-source/InsightToolkit-5.4.6" -B "$align_science/.itk-static" -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DITK_BUILD_DEFAULT_MODULES=OFF -DITKGroup_Core=OFF -DITKGroup_Filtering=OFF -DITKGroup_IO=OFF -DITKGroup_Numerics=OFF -DITKGroup_Registration=OFF -DModule_ITKRegistrationMethodsv4=ON -DModule_ITKBiasCorrection=ON -DModule_ITKThresholding=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 -DCMAKE_OSX_ARCHITECTURES="$(uname -m)" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DITK_USE_SYSTEM_EXPAT=OFF -DITK_USE_SYSTEM_ZLIB=OFF -DITK_USE_SYSTEM_PNG=OFF
cmake --build "$align_science/.itk-static" -j4
cmake -S "$align_itk_root" -B "$align_science/.itk-static-adapter" -DCMAKE_BUILD_TYPE=Release -DITK_DIR="$align_science/.itk-static" -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 -DCMAKE_OSX_ARCHITECTURES="$(uname -m)"
cmake --build "$align_science/.itk-static-adapter" -j4
cp "$align_science/.itk-static-adapter/libalign-itk.dylib" "$align_science/native/libalign-itk.dylib"
