"""Find CMake executables in single- or multi-configuration native builds."""
from pathlib import Path
import sys


def executable(build,name):
    suffix='.exe' if sys.platform=='win32' else ''
    for directory in ['engine','engine/Release','engine/RelWithDebInfo','engine/Debug']:
        path=Path(build)/directory/(name+suffix)
        if path.is_file():return path
    raise FileNotFoundError('Build the native test executable: '+name)
