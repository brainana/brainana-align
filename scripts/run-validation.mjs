import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { spawn } from 'node:child_process'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
const pkg=JSON.parse(fs.readFileSync(path.join(root,'package.json'),'utf8'))
const output=process.argv[2]?path.resolve(process.argv[2]):path.join(root,'validation-results')
fs.mkdirSync(output,{recursive:true})
const priorPath=path.join(output,'results.json')
const only=new Set((process.env.BRAINANA_VALIDATION_ONLY || '').split(',').filter(Boolean))
const results=only.size && fs.existsSync(priorPath) ? JSON.parse(fs.readFileSync(priorPath,'utf8')).results.filter(r=>!only.has(r.name)) : []
for(const name of ['build',...Object.keys(pkg.scripts).filter(n=>n.startsWith('test:'))]) {
  if (only.size && !only.has(name)) continue
  if(['test:browser','test:browser-matrix'].includes(name) && process.env.BRAINANA_VALIDATION_SKIP_BROWSER==='1') {
    results.push({name,status:'blocked',reason:process.env.BRAINANA_VALIDATION_BROWSER_BLOCK_REASON || 'Browser automation skipped explicitly by BRAINANA_VALIDATION_SKIP_BROWSER; no graphical assertions counted as passed. See retained browser launch attempt logs.'})
    continue
  }
  const start=Date.now(),log=path.join(output,name.replaceAll(':','-')+'.log')
  const stream=fs.createWriteStream(log)
  const result=await new Promise(resolve=>{
    const child=spawn('npm',['run',name],{cwd:root,env:process.env,stdio:['ignore','pipe','pipe']})
    child.stdout.pipe(stream,{end:false});child.stderr.pipe(stream,{end:false})
    let timedOut=false
    const timer=setTimeout(()=>{timedOut=true;child.kill('SIGTERM')},180000)
    child.on('error',error=>{clearTimeout(timer);resolve({name,status:'failed',error:String(error)})})
    child.on('close',(code,signal)=>{clearTimeout(timer);resolve({name,status:code===0?'passed':'failed',exitCode:code,signal,timedOut,durationMs:Date.now()-start,log:path.basename(log)})})
  })
  await new Promise(resolve=>stream.end(resolve));
  if(result.status==='failed' && fs.readFileSync(log,'utf8').includes('listen EPERM: operation not permitted')){result.status='blocked';result.reason='The execution sandbox denies local socket creation; the command exited before integration assertions. See the retained log.'}
  results.push(result);console.log(`${name}: ${result.status}`)
}
const report={version:pkg.version,generatedAt:new Date().toISOString(),platform:process.platform,architecture:process.arch,node:process.version,results}
fs.writeFileSync(path.join(output,'results.json'),JSON.stringify(report,null,2)+'\n')
if(results.some(r=>r.status==='failed'))process.exitCode=1
console.log(`${results.filter(r=>r.status==='passed').length} passed; ${results.filter(r=>r.status==='failed').length} failed; ${results.filter(r=>r.status==='blocked').length} blocked`)
