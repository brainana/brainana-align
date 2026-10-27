import assert from 'node:assert/strict'
import {fitNonlinearSpline,createPointMapping,splineBound,splineDiagnostics,validateSpline,resampleSpline} from '../packages/imaging-math/nonlinearSpline'
import {splineDisplacementField,displacementReferenceGrid} from '../packages/imaging-math/splineFields'
import {applyMat4,multiplyMat4,rigidDelta,type Vec3} from '../packages/imaging-math/rigid'
import {invertAffine,applyAffine,type RawNifti} from '../packages/imaging-math/roiWarp'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {runIntensityRegistration,makeIntensityScorer} from '../packages/imaging-math/intensityRegistration'
import type {FitState} from '../apps/align/src/appTypes'
import * as nifti from 'nifti-reader-js'
const points:Vec3[]=[];for(const x of [-10,0,10])for(const y of [-10,0,10])for(const z of [-10,0,10])points.push([x,y,z])
const dist=(a:Vec3,b:Vec3)=>Math.hypot(...a.map((v,i)=>v-b[i]))
const targets=points.map(([x,y,z])=>[x+.4*Math.sin(y/8)*Math.cos(z/9),y+.3*Math.sin(z/7)*Math.cos(x/9),z+.3*Math.sin(x/8)*Math.cos(y/7)] as Vec3)
const result=fitNonlinearSpline(points,targets,0),s=result.spline,map=createPointMapping(result.matrix,s)
assert(result.rms<1e-7,`Exact spline recovery: ${result.rms}`);assert(result.affineRms>.05);assert(s.limitingFactor===1)
for(let i=0;i<points.length;i++){assert(dist(map.forward(points[i]),targets[i])<1e-7);assert(dist(map.inverse(targets[i]),points[i])<1e-6)}
for(const p of [[3,4,5],[30,-40,60],[300,-500,1000]] as Vec3[])assert(dist(map.inverse(map.forward(p)),p)<1e-6,'Inverse must be of the same nonlinear map')
const smooth=fitNonlinearSpline(points,targets,.1);assert(smooth.rms>result.rms);assert(smooth.rms<smooth.affineRms)
const affine=[[0,-1.7,.2,50],[.8,0,.1,-20],[.1,.2,1.3,80],[0,0,0,1]]
const linear=fitNonlinearSpline(points,points.map(p=>applyMat4(affine,p)),.01);assert(linear.rms<1e-7);assert(splineBound(linear.spline)<1e-7)
const moved=multiplyMat4(rigidDelta([2,-1,3,15,0,90],[0,0,0]),result.matrix),manual=createPointMapping(moved,s)
for(const p of points){assert(dist(manual.inverse(manual.forward(p)),p)<1e-6,'Manual adjustment inverse');assert(dist(manual.forward(p),applyMat4(rigidDelta([2,-1,3,15,0,90],[0,0,0]),map.forward(p)))<1e-7,'Manual adjustment must act after the full nonlinear map')}
const reverse=fitNonlinearSpline(targets,points,0);assert(reverse.rms<1e-7)
assert.throws(()=>fitNonlinearSpline(points.slice(0,6),targets.slice(0,6)),/12/)
assert.throws(()=>fitNonlinearSpline([...points.slice(0,26),points[0]],targets),/coincident/)
assert.throws(()=>fitNonlinearSpline(points.map(p=>[p[0],p[1],0]),targets),/coplanar|degenerate|collinear/)
assert.throws(()=>fitNonlinearSpline(points,points.map(([x,y,z])=>[-x,y,z])),/mirrors/)
assert.throws(()=>fitNonlinearSpline(points,targets,-1),/smoothing/)
const severe=points.map(p=>[...p] as Vec3);severe[13][0]+=100
const limited=fitNonlinearSpline(points,severe,0);assert(limited.spline.limitingFactor<1);assert(limited.rms>1);assert(splineBound(limited.spline)<=.6500000001);assert(splineDiagnostics(limited.spline,limited.matrix).globalJacobianLowerBound>0)
const bad=structuredClone(s);bad.weights[0][0]+=1e6;assert.throws(()=>validateSpline(bad),/safety bound/)
const fit:FitState={direction:'ct-mri',model:'spline',nonlinear:s,parameterCount:points.length*3,matrix:result.matrix,inverse:result.inverse,baseMatrix:result.matrix,landmarkMatrix:result.matrix,rms:result.rms,manual:[0,0,0,0,0,0],proposal:null,landmarkSnapshot:points.map((ct,i)=>({id:i+1,ct,mri:targets[i],enabled:true})),landmarksChanged:false,fittedAt:'test',diagnostics:result.diagnostics}
const payload=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:fit.landmarkSnapshot,selectedId:1,nextId:28,direction:'ct-mri',landmarkModel:'spline',splineSmoothing:0,fit,optimizationWindows:{mri:{},ct:{}}})
assert.equal(payload.schemaVersion,13);const restored=parseSessionPayload(JSON.parse(JSON.stringify(payload))).fit!;assert(dist(createPointMapping(restored.matrix,restored.nonlinear).forward(points[0]),targets[0])<1e-7)
const tampered=structuredClone(payload);tampered.fit!.nonlinear!.weights[0][0]+=1e6;assert.throws(()=>parseSessionPayload(tampered),/safety/)
const artifacts=createRegistrationArtifacts({appVersion:'test',fit,landmarks:fit.landmarkSnapshot});assert.equal(artifacts.length,1,'Never export misleading affine TXT matrices for a nonlinear warp');const metadata=JSON.parse(await artifacts[0].blob.text());assert.equal(metadata.forward_matrix,null);assert.equal(metadata.inverse_matrix,null);assert.equal(metadata.nonlinear.kernel,'3d-polyharmonic-r')
fit.proposal={matrix:fit.matrix,params:[],scoreBefore:.1,scoreAfter:0,kind:'spline',previous:null};assert.throws(()=>createRegistrationArtifacts({appVersion:'test',fit,landmarks:fit.landmarkSnapshot}),/Accept or reject/);assert.equal(parseSessionPayload({...payload,fit}).fit!.proposal!.kind,'spline');fit.proposal=null
const dims:[number,number,number]=[17,17,17],geometry=[[.8,-.3,.1,-5],[.2,1.2,.1,-10],[.1,.2,1.5,-12],[0,0,0,1]]
const image:RawNifti={dims,affine:geometry,pixDims:[.83,1.25,1.51],values:Float32Array.from({length:17**3},(_,i)=>{const x=i%17,y=Math.floor(i/17)%17,z=Math.floor(i/289);return 1+100*Math.exp(-((x-5)**2+(y-8)**2+(z-6)**2)/20)+40*Math.exp(-((x-11)**2+(y-5)**2+(z-10)**2)/12)}),frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0}
const coarse=displacementReferenceGrid(image,2);assert(coarse.values.length===0);assert(dist(applyAffine(coarse.affine,coarse.dims.map(v=>v-1) as Vec3),applyAffine(image.affine,[16,16,16]))<1e-10);assert.throws(()=>displacementReferenceGrid({...image,dims:[1000,1000,1000]},.2),/coarser/);
const identical=fitNonlinearSpline(points,points,.01),resampled=await resampleSpline(image,image,identical.matrix,identical.spline)
assert(Math.max(...resampled.map((v,i)=>Math.abs(v-image.values[i])))<1e-4,'Anisotropic oblique resampling applies each image affine once')
const warpedSource={...image,values:Float32Array.from(image.values,(_,i)=>{const q=map.forward(applyAffine(geometry,[i%17,Math.floor(i/17)%17,Math.floor(i/289)]));return 100+q[0]+.5*q[1]-.3*q[2]})};const warped=await resampleSpline(warpedSource,image,result.matrix,s);let squared=0,beforeSquared=0,count=0;for(let z=2;z<15;z++)for(let y=2;y<15;y++)for(let x=2;x<15;x++){const i=x+17*(y+17*z),q=applyAffine(geometry,[x,y,z]),expected=100+q[0]+.5*q[1]-.3*q[2];squared+=(warped[i]-expected)**2;beforeSquared+=(warpedSource.values[i]-expected)**2;count++}assert(Math.sqrt(squared/count)<.025);assert(squared<beforeSquared*.2,'Actual nonlinear resampling improves a known physical-space intensity image')
for(const direction of ['forward','inverse'] as const){const bytes=await splineDisplacementField(image,moved,s,direction),buffer=bytes.buffer as ArrayBuffer,hdr=nifti.readHeader(buffer)!;assert.equal(hdr.intent_code,1006);assert.deepEqual(hdr.dims.slice(0,6),[5,17,17,17,1,3]);assert.equal(hdr.sform_code,2);assert.equal(hdr.qform_code,0);assert.equal(hdr.xyzt_units,2);const data=new Float32Array(nifti.readImage(hdr,buffer));const i=8+17*(8+17*8),p=applyAffine(geometry,[8,8,8]),mapped=manual[direction](p);for(let a=0;a<3;a++)assert(Math.abs(data[a*17**3+i]-(mapped[a]-p[a]))<1e-5)}
const score=makeIntensityScorer(image,image,'normalized-correlation',32,[],[],18000,identical.matrix,identical.spline);assert(score(identical.matrix)>.9999)
const refined=await runIntensityRegistration({fixed:image,moving:image,operation:'local',start:identical.matrix,nonlinear:identical.spline,metric:'normalized-correlation',bins:32,fixedWindows:[],movingWindows:[],settings:{model:'rigid',enabledParameters:[],rotationRanges:[1,1,1] as any,translationLimitMm:1,scaleLimit:.2,shearLimit:.2,candidatesToRefine:1,shrinkFactors:[1],sweepsPerLevel:[1],refinement:{translationStepMm:.5,rotationStepDegrees:.5}}},()=>{})
assert(refined.scoreAfter>.999)
console.log('Nonlinear spline: exact/nonlinear/affine/smoothed recovery, same-map inverse, post-warp manual composition, reverse fit, degenerate/reflected rejection, global deformation bound, schema-4 sessions/proposals/tamper rejection, honest JSON export, oblique resampling, RAS forward/inverse vector NIfTI bytes and local refinement passed.')
