import assert from 'node:assert/strict'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import crypto from 'node:crypto'
import {execFileSync,spawnSync} from 'node:child_process'
import {fileURLToPath} from 'node:url'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..'),scratch=fs.mkdtempSync(path.join(os.tmpdir(),'align-publication-'))
const hash=b=>crypto.createHash('sha256').update(b).digest('hex')
try{
 for(const kind of ['valid','missing','hash','commit','platform']){
  const dir=path.join(scratch,kind);fs.mkdirSync(dir)
  const meta={app:'Brainana Align',version:'0.17.10'};fs.writeFileSync(path.join(dir,'VERSION.json'),JSON.stringify(meta));fs.writeFileSync(path.join(dir,'README.md'),'public source');fs.writeFileSync(path.join(dir,'.gitignore'),'.env\nrelease-output/\n')
  fs.writeFileSync(path.join(dir,'.env'),'PRIVATE_FIXTURE_MUST_NOT_EXPORT=1')
  const git=args=>execFileSync('git',args,{cwd:dir,stdio:'pipe'});git(['init','-b','main']);git(['add','VERSION.json','README.md','.gitignore']);git(['-c','user.name=Test','-c','user.email=test@example.invalid','commit','-m','fixture'])
  const commit=git(['rev-parse','HEAD']).toString().trim(),out=path.join(dir,'release-output');fs.mkdirSync(out)
  const platforms=[['darwin','arm64'],['darwin','x64'],['windows','x64'],['linux','x64']]
  for(const [index,[platform,arch]] of platforms.entries()){
   if(kind==='missing'&&index===3)continue
   const name=`brainana-align-0.17.10-${platform}-${arch}.zip`,data=Buffer.from(`synthetic fixture ${platform}/${arch}`);fs.writeFileSync(path.join(out,name),data)
   const record={version:meta.version,sourceCommit:kind==='commit'?'wrong':commit,platform,architecture:kind==='platform'&&index===3?'arm64':arch,assets:[{file:name,bytes:data.length,sha256:kind==='hash'?'0'.repeat(64):hash(data)}]}
   fs.writeFileSync(path.join(out,`assets-${platform}-${arch}.json`),JSON.stringify(record))
  }
  const result=spawnSync(process.env.CI_PYTHON||(process.platform==='win32'?'python':'python3'),[path.join(root,'scripts/ci/assemble-release.py')],{cwd:dir,encoding:'utf8'})
  if(kind==='valid'){
   assert.equal(result.status,0,result.stderr)
   const source=path.join(out,'brainana-align-0.17.10-source.zip')
   const listing=execFileSync('git',['archive','--format=tar','HEAD'],{cwd:dir});assert(!listing.includes(Buffer.from('PRIVATE_FIXTURE_MUST_NOT_EXPORT')))
   assert(fs.existsSync(source));assert(fs.existsSync(path.join(out,'brainana-align-0.17.10-full-release.zip')))
   const manifest=JSON.parse(fs.readFileSync(path.join(out,'RELEASE_ASSETS.json')));assert.equal(manifest.commit,commit);assert.equal(manifest.platforms.length,4)
  }else{assert.notEqual(result.status,0,kind);assert(!fs.existsSync(path.join(out,'RELEASE_ASSETS.json')),kind+' must block publication assembly')}
 }
 const cwd=path.join(scratch,'valid')
 for(const [tag,event,pass] of [['v0.17.10','push',true],['v0.17.11','push',false],['main','workflow_dispatch',true]]){
  const result=spawnSync(process.execPath,[path.join(root,'scripts/ci/check-tag.mjs')],{cwd,encoding:'utf8',env:{...process.env,RELEASE_TAG:tag,GITHUB_EVENT_NAME:event,GITHUB_OUTPUT:''}});assert.equal(result.status===0,pass,tag)
 }
 const ci=fs.readFileSync(path.join(root,'.github/workflows/release.yml'),'utf8');assert(ci.includes('needs: [package, browsers]'));assert(ci.includes('needs: aggregate'));assert(ci.includes("github.event_name != 'workflow_dispatch'"));assert(!ci.includes('pull_request_target'))
 console.log('Publication regression passed: four-platform/commit/hash gates, exact-commit source, ignored private fixture, manual no-publish route, tag matching and final publish dependency.')
}finally{fs.rmSync(scratch,{recursive:true,force:true})}
