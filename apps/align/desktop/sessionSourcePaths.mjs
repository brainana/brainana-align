import fs from 'node:fs/promises'
import path from 'node:path'

export const IMAGE_EXTENSIONS=['nii','nii.gz','hdr','img','img.gz','head','brik','brik.gz','mgh','mgz','nrrd','nhdr','mif','mha','mhd','raw','v','v16','vmr','npy','npz','fib','src']
export const supportedImage=name=>typeof name==='string'&&IMAGE_EXTENSIONS.some(ext=>name.toLowerCase().endsWith('.'+ext))

function stringEnd(text,start){for(let i=start+1;i<text.length;i++){if(text[i]==='\\')i++;else if(text[i]==='"')return i+1}return -1}
function valueEnd(text,start){
 if(text[start]==='"')return stringEnd(text,start)
 if(text[start]!=='{'&&text[start]!=='['){const end=text.slice(start).search(/[,}\]]/);return end<0?-1:start+end}
 let depth=0
 for(let i=start;i<text.length;i++){if(text[i]==='"'){i=stringEnd(text,i)-1;if(i<0)return -1}else if(text[i]==='{'||text[i]==='[')depth++;else if(text[i]==='}'||text[i]===']'){if(--depth===0)return i+1}}
 return -1
}
/** Image references are near the beginning of our JSON, before potentially large
 * nonlinear fields. Read only this bounded prefix, never a second full session. */
export function sessionHeader(text){
 text=text.replace(/^\uFEFF/,'');let at=0;const header={}
 const whitespace=()=>{while(/\s/.test(text[at]??'')&&at<text.length)at++}
 whitespace();if(text[at++]!=='{')throw Error('Invalid session JSON')
 while(at<text.length){whitespace();if(text[at]==='}')break;if(text[at++]!== '"')throw Error('Invalid session header');const begin=at-1,end=stringEnd(text,begin);if(end<0)break;const key=JSON.parse(text.slice(begin,end));at=end;whitespace();if(text[at++]!==':')throw Error('Invalid session header');whitespace();const stop=valueEnd(text,at);if(stop<0)break
  if(['application','registrationPairing','images'].includes(key))header[key]=JSON.parse(text.slice(at,stop))
  if(key==='images'){if(header.application!=='Brainana Align')throw Error('Not an Align session');return header}
  at=stop;whitespace();if(text[at]!==',')break;at++
 }
 throw Error('Image references are unavailable in the session header')
}
export async function readSessionHeader(filename){
 if(typeof filename!=='string'||!path.isAbsolute(filename)||!filename.toLowerCase().endsWith('.json')||filename.includes('\0'))throw Error('Select a saved Align JSON session')
 const file=await fs.open(filename,'r')
 try{const stat=await file.stat();if(!stat.isFile())throw Error('Select a saved session file');const bytes=Buffer.alloc(Math.min(stat.size,2*1024**2));let read=0;while(read<bytes.length){const {bytesRead}=await file.read(bytes,read,bytes.length-read,read);if(!bytesRead)break;read+=bytesRead}return sessionHeader(bytes.subarray(0,read).toString('utf8'))}finally{await file.close()}
}
async function readable(filename){try{if(!supportedImage(filename))return false;const s=await fs.stat(filename);await fs.access(filename,fs.constants?.R_OK??4);return s.isFile()}catch{return false}}
export async function findSessionSources(sessionPath,signature){
 if(!signature)return []
 const names=Array.isArray(signature.sourceFiles)&&signature.sourceFiles.length?signature.sourceFiles:[signature.name]
 if(names.length>128||names.some(n=>typeof n!=='string'||!supportedImage(n)))throw Error('Invalid image references in session')
 const files=[]
 for(let i=0;i<names.length;i++){
  const saved=signature.sourceLocations?.[i],leaf=names[i].split(/[\\/]/).pop()
  const candidates=[...(typeof saved==='string'&&path.isAbsolute(saved)&&!saved.includes('\0')?[saved]:[]),path.join(path.dirname(sessionPath),leaf)]
  const found=[];for(const candidate of candidates)if(await readable(candidate)){found.push(candidate);break}
  if(!found.length)return null
  files.push(found[0])
 }
 return files
}
