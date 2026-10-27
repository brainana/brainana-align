"""Optional, configurable N4 preprocessing companion; independent of JIP math.

Requires numpy, nibabel and the focused native ITK library. The C engine does not load these packages.
Defaults are engineering starting presets, not a validated macaque-wide optimum.
"""
from dataclasses import asdict, dataclass, replace
from pathlib import Path
import argparse
import hashlib
import json
import math
import os
import tempfile
import time

import nibabel as nib
import numpy as np


@dataclass(frozen=True)
class N4Settings:
    schema_version: int = 1
    preset: str = 'macaque-functional'
    estimation_spacing_mm: float = 2.0
    initial_spline_spacing_mm: float = 80.0
    iterations: tuple = (50, 30, 20)
    convergence: float = 0.0001
    histogram_bins: int = 200
    bias_field_fwhm: float = 0.15
    wiener_noise: float = 0.01
    threads: int = 4

    @classmethod
    def for_preset(cls, name):
        if name == 'macaque-anatomical':
            return cls(preset=name, estimation_spacing_mm=1., iterations=(50,50,30,20))
        if name == 'macaque-functional':
            return cls()
        raise ValueError('Unknown N4 preset')

    def validate(self):
        if type(self.schema_version) is not int or self.schema_version != 1:
            raise ValueError('Unsupported N4 settings schema')
        if not isinstance(self.preset,str):
            raise ValueError('Preset label must be text')
        for name in ['estimation_spacing_mm','initial_spline_spacing_mm','convergence','bias_field_fwhm','wiener_noise']:
            value=getattr(self,name)
            if isinstance(value,bool) or not isinstance(value,(float,int)) or not math.isfinite(value) or value<=0:
                raise ValueError(name+' must be finite and positive')
        if not 1<=len(self.iterations)<=6 or any(type(x) is not int or x<1 or x>1000 for x in self.iterations):
            raise ValueError('Supply 1–6 fitting levels, each with 1–1000 iterations')
        if type(self.histogram_bins) is not int or not 16<=self.histogram_bins<=4096:
            raise ValueError('Histogram bins must be 16–4096')
        if type(self.threads) is not int or not 1<=self.threads<=128:
            raise ValueError('Thread count must be 1–128')
        if self.convergence>1 or self.bias_field_fwhm>1 or self.wiener_noise>1:
            raise ValueError('Convergence, FWHM and Wiener noise must be <=1')
        if not .05<=self.estimation_spacing_mm<=100 or not 1<=self.initial_spline_spacing_mm<=1000:
            raise ValueError('Estimation spacing must be 0.05–100 mm and initial spline spacing 1–1000 mm')
        return self

    def to_dict(self):
        result=asdict(self);result['iterations']=list(self.iterations)
        return result

    @classmethod
    def from_dict(cls, data):
        data=dict(data)
        if 'iterations' in data:data['iterations']=tuple(data['iterations'])
        return cls(**data).validate()


def correct(image, settings, mask_image=None, progress=None, cancelled=None):
    """Estimate N4 natively; preserve the input grid and all source values."""
    from brainana_registration import native_itk
    settings.validate();started=time.monotonic();mask=None
    if mask_image is not None:
        if mask_image.shape!=image.shape or not np.allclose(mask_image.affine,image.affine,atol=1e-5,rtol=0):raise ValueError('Estimation mask must use the same native grid and physical geometry')
        mask=mask_image.get_fdata(dtype=np.float32)
        if not np.isfinite(mask).all() or not np.all((mask==0)|(mask==1)):raise ValueError('Estimation mask must contain only binary 0/1 values')
    corrected,bias,selection,meta=native_itk.n4(image,settings.to_dict(),mask,progress or (lambda event:None),cancelled or (lambda:False))
    meta.update(settings=settings.to_dict(),library='ITK native',preset_validation='preliminary engineering defaults; not optimized across macaque datasets',elapsed_seconds=time.monotonic()-started,geometry='Native grid unchanged; no registration performed',application='Input divided by multiplicative bias field over the native grid; zeros remain zero. Mask restricts estimation only. No intensity range rescaling.')
    return corrected,bias,selection,meta


def atomic_json(path, data):
    path=Path(path)
    temporary=None
    try:
        with tempfile.NamedTemporaryFile(mode='w',dir=path.parent,delete=False,suffix='.json') as stream:
            temporary=Path(stream.name)
            json.dump(data,stream,indent=2,allow_nan=False);stream.write('\n')
        os.replace(temporary,path)
    finally:
        if temporary is not None:temporary.unlink(missing_ok=True)


def write_native(path, values, reference):
    path=Path(path)
    if not str(path).endswith(('.nii','.nii.gz')):raise ValueError('Output must end in .nii or .nii.gz')
    header=reference.header.copy();header.set_data_dtype(values.dtype)
    out=nib.Nifti1Image(values,reference.affine,header)
    out.set_qform(reference.get_qform(),int(reference.header['qform_code']))
    out.set_sform(reference.get_sform(),int(reference.header['sform_code']))
    suffix='.nii.gz' if str(path).endswith('.nii.gz') else '.nii'
    with tempfile.NamedTemporaryFile(dir=path.parent,suffix=suffix,delete=False) as stream:temporary=Path(stream.name)
    try:nib.save(out,temporary);os.replace(temporary,path)
    finally:temporary.unlink(missing_ok=True)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--field',type=Path)
    parser.add_argument('--mask',type=Path)
    parser.add_argument('--preset',choices=['macaque-anatomical','macaque-functional'])
    parser.add_argument('--settings',type=Path)
    parser.add_argument('--save-settings',type=Path)
    parser.add_argument('--estimation-spacing-mm',type=float)
    parser.add_argument('--spline-spacing-mm',type=float)
    parser.add_argument('--iterations',help='Comma-separated iteration caps, coarse to fine')
    parser.add_argument('--convergence',type=float)
    parser.add_argument('--threads',type=int)
    args=parser.parse_args()
    if args.settings and args.preset:parser.error('Choose a saved settings file or a preset')
    try:
        config=N4Settings.from_dict(json.loads(args.settings.read_text())) if args.settings else N4Settings.for_preset(args.preset or 'macaque-functional')
        changes={k:v for k,v in dict(estimation_spacing_mm=args.estimation_spacing_mm,
            initial_spline_spacing_mm=args.spline_spacing_mm,convergence=args.convergence,threads=args.threads).items() if v is not None}
        if args.iterations:changes['iterations']=tuple(int(x) for x in args.iterations.split(','))
        config=replace(config,**changes).validate()
        sources=[x.resolve() for x in [args.input,args.mask,args.settings] if x is not None]
        destinations=[args.output,args.output.with_name(args.output.name+'.json')]+([args.field] if args.field else [])+([args.save_settings] if args.save_settings else [])
        resolved=[x.resolve() for x in destinations]
        if len(set(resolved))!=len(resolved) or any(x in sources for x in resolved):raise ValueError('Input, mask, settings and output paths must be distinct')
        for x in destinations:
            if x.exists() and any(os.path.samefile(x,y) for y in sources):raise ValueError('Output aliases an input file')
        image=nib.load(args.input)
        result,field,mask,meta=correct(image,config,nib.load(args.mask) if args.mask else None)
        meta.update(input=str(args.input.resolve()),input_sha256=hashlib.sha256(args.input.read_bytes()).hexdigest())
        if args.mask:meta.update(mask_input=str(args.mask.resolve()),mask_sha256=hashlib.sha256(args.mask.read_bytes()).hexdigest())
        write_native(args.output,result,image)
        if args.field:write_native(args.field,field,image)
        atomic_json(args.output.with_name(args.output.name+'.json'),meta)
        if args.save_settings:atomic_json(args.save_settings,config.to_dict())
        print(json.dumps(meta,indent=2))
    except (ValueError,TypeError,OSError,RuntimeError,nib.filebasedimages.ImageFileError) as error:
        parser.exit(1,str(error)+'\n')


if __name__=='__main__':main()
