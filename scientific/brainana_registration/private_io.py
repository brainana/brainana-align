"""Atomic private worker results."""
from pathlib import Path
import tempfile,json,os

def atomic_json(path,value):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    handle,name=tempfile.mkstemp(prefix=path.name+'.',suffix='.tmp',dir=path.parent)
    try:
        with os.fdopen(handle,'w') as stream:json.dump(value,stream,indent=2,allow_nan=False);stream.write('\n')
        os.replace(name,path)
    finally:Path(name).unlink(missing_ok=True)
