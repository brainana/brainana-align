"""Physical registration regions; no cropping, zeroing, or altered image taps."""
from pathlib import Path
import hashlib
import nibabel as nib
import numpy as np

def validate_windows(windows):
    if not isinstance(windows,list) or len(windows)>3:raise ValueError('Invalid optimization windows')
    for w in windows:
        if not isinstance(w,dict):raise ValueError('Invalid optimization window')
        axes=w.get('axes',[0,1,2]);low=w.get('min');high=w.get('max')
        if not isinstance(axes,list) or not axes or len(set(axes))!=len(axes) or any(type(a)!=int or a not in [0,1,2] for a in axes):raise ValueError('Invalid physical window axes')
        if np.shape(low)!=(3,) or np.shape(high)!=(3,) or not np.isfinite([low,high]).all() or any(low[a]>high[a] for a in axes):raise ValueError('Invalid physical window bounds')
    return windows

def prepare(request,fixed,moving,orientation,cancelled):
    spec=request.get('sampling')
    if spec is None:return None,None
    if spec.get('version')!=1:raise ValueError('Unsupported registration sampling format')
    windows={side:validate_windows(spec.get(side+'_windows',[])) for side in ['fixed','moving']}
    result={};metadata=dict(version=1,fixed_windows=windows['fixed'],moving_windows=windows['moving'])
    for side,image in [('fixed',fixed),('moving',moving)]:
        excluded=None;file=spec.get(side+'_excluded')
        if file:
            values=np.fromfile(file,dtype=np.uint8)
            if values.size!=np.prod(image.shape) or np.any(values>1):raise ValueError('Invalid registration exclusion mask')
            excluded=values.reshape(image.shape,order='F')
            metadata[side+'_exclusion']=dict(voxels=int(np.count_nonzero(excluded)),sha256=hashlib.sha256(values).hexdigest())
        selected=None if excluded is None else (excluded==0).astype(np.uint8)
        if side=='fixed' and windows[side]:
            if selected is None:selected=np.ones(image.shape,dtype=np.uint8,order='F')
            xy=np.indices(image.shape[:2],dtype=np.float64)
            for z in range(image.shape[2]):
                if cancelled():raise InterruptedError('Cancelled')
                world=[image.affine[a,0]*xy[0]+image.affine[a,1]*xy[1]+image.affine[a,2]*z+image.affine[a,3] for a in range(3)]
                for w in windows[side]:
                    for a in w.get('axes',[0,1,2]):selected[:,:,z]&=(world[a]>=w['min'][a])&(world[a]<=w['max'][a])
        if side=='moving' and selected is not None:selected=nib.orientations.apply_orientation(selected,np.asarray(orientation))
        if selected is not None:
            if not selected.any():raise ValueError('Registration region or exclusion mask contains no sample centers. Enlarge the region or adjust the mask.')
            result[side]=selected
        metadata[side+'_included_voxels']=None if selected is None else int(np.count_nonzero(selected))
    result['moving_windows']=windows['moving']
    return result,metadata
