"""Physical coordinate helpers used by Align registration; arithmetic preserved."""
import nibabel as nib
import numpy as np

def image_like(values,reference,affine=None):
    affine=reference.affine if affine is None else affine
    header=reference.header.copy();header.set_data_dtype(values.dtype)
    out=nib.Nifti1Image(values,affine,header)
    if affine is reference.affine or np.array_equal(affine,reference.affine):
        out.set_qform(reference.get_qform(),int(reference.header['qform_code']))
        out.set_sform(reference.get_sform(),int(reference.header['sform_code']))
    else:
        out.set_qform(affine,1);out.set_sform(affine,1)
    return out

def orient_like(image,reference):
    orientation=nib.orientations.ornt_transform(nib.orientations.io_orientation(image.affine),nib.orientations.io_orientation(reference.affine))
    return image.as_reoriented(orientation),orientation.tolist()

def centered(image):
    spacing=np.linalg.norm(image.affine[:3,:3],axis=0)
    result=np.diag([*spacing,1.]);result[:3,3]=-(np.array(image.shape[:3])-1)*spacing/2
    return result

def parameters_matrix(values):
    t,p,s=np.deg2rad(values[3:6]);ct,cp,cs=np.cos([t,p,s]);st,sp,ss=np.sin([t,p,s])
    rotation=np.array([[ct*cs+st*sp*ss,-st*cp,ct*ss-st*sp*cs],
                       [st*cs-ct*sp*ss,ct*cp,st*ss+ct*sp*cs],[-cp*ss,-sp,cp*cs]])
    sx,sy,sz=values[6:9];xy,xz,yz=values[9:12]
    scale=np.array([[1/sx,xy/sx,xz/sx],[0,1/sy,yz/sy],[0,0,1/sz]])
    out=np.eye(4);out[:3,:3]=rotation@scale;out[:3,3]=-np.array(values[:3]);return out

def matrix_parameters(matrix):
    q,r=np.linalg.qr(matrix[:3,:3]);signs=np.sign(np.diag(r));q=q@np.diag(signs);r=np.diag(signs)@r
    if np.linalg.det(q)<0 or np.any(np.diag(r)<=0):raise ValueError('Initial geometry would reflect anatomy or is singular.')
    phi=np.arcsin(np.clip(-q[2,1],-1,1))
    if abs(np.cos(phi))<1e-7:theta=np.arctan2(q[1,0],q[0,0]);psi=0.
    else:theta=np.arctan2(-q[0,1],q[1,1]);psi=np.arctan2(-q[2,0],q[2,2])
    values=[*(-matrix[:3,3]),*np.rad2deg([theta,phi,psi]),*(1/np.diag(r)),r[0,1]/r[0,0],r[0,2]/r[0,0],r[1,2]/r[1,1]]
    if not np.allclose(parameters_matrix(values),matrix,atol=2e-5,rtol=1e-6):raise ValueError('Cannot represent the selected starting geometry safely.')
    return [float(x) for x in values]

def initial_parameters(fixed,moving,mode='centers',custom=None):
    if mode=='custom':return list(custom)
    matrix=centered(moving)@np.linalg.inv(moving.affine)@fixed.affine@np.linalg.inv(centered(fixed))
    if mode=='centers':matrix[:3,3]=0
    elif mode!='coordinates':raise ValueError('Unknown initial alignment mode.')
    return matrix_parameters(matrix)

def world_pull(transform):
    matrix=lambda name:np.array(transform[name],dtype=float).reshape(4,4)
    return matrix('moving_voxel_to_native_mm')@matrix('moving_centered_mm_to_voxel')@matrix('centered_pull_affine')@matrix('fixed_voxel_to_centered_mm')@np.linalg.inv(matrix('fixed_voxel_to_native_mm'))
