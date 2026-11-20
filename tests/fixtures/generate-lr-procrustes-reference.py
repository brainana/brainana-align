"""Independent NumPy SVD oracle, not the app's quaternion/dual-branch solver.
Fixtures are committed; tests never regenerate them from the candidate."""
from pathlib import Path
import json
import numpy as np

rng=np.random.default_rng(184)
cases=[]
for model in ['rigid','similarity']:
    for reflected in [False,True]:
        for i in range(4):
            x=rng.normal(size=(12,3))*np.array([8,19,11])+[1000,-700,400]
            q,_=np.linalg.qr(rng.normal(size=(3,3)))
            if np.linalg.det(q)<0:q[:,0]*=-1
            if reflected:q[:,0]*=-1
            y=x@(q*(1.18 if model=='similarity' else 1)).T+[13,-8,2]+rng.normal(scale=.035,size=x.shape)
            xc=x-x.mean(axis=0);yc=y-y.mean(axis=0)
            u,_,vt=np.linalg.svd(xc.T@yc)
            expected={}
            for allow in [False,True]:
                rotation=vt.T@u.T
                if not allow and np.linalg.det(rotation)<0:
                    vt_fixed=vt.copy();vt_fixed[-1]*=-1;rotation=vt_fixed.T@u.T
                factor=float(np.sum((xc@rotation.T)*yc)/np.sum(xc*xc)) if model=='similarity' else 1.
                linear=rotation*factor
                matrix=np.eye(4);matrix[:3,:3]=linear;matrix[:3,3]=y.mean(axis=0)-linear@x.mean(axis=0)
                rms=float(np.sqrt(np.mean(np.sum((x@linear.T+matrix[:3,3]-y)**2,axis=1))))
                expected['allow' if allow else 'proper']={'matrix':matrix.tolist(),'rms':rms,'reflected':bool(np.linalg.det(linear)<0)}
            cases.append({'model':model,'source':x.tolist(),'target':y.tolist(),'expected':expected})
out=Path(__file__).with_name('lr-procrustes-reference.json')
out.write_text(json.dumps({'oracle':'Centered NumPy SVD orthogonal Procrustes with determinant-constrained comparison and positive uniform scale','numpy':np.__version__,'seed':184,'cases':cases},indent=2)+'\n')
print(len(cases),'independent noisy SVD references written')
