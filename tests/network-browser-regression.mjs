import assert from 'node:assert/strict'
import {Readable} from 'node:stream'
import {createNetworkBrowser,parseSshConfig} from '../packages/core-server/networkBrowser.mjs'
let closed=0,connected=0,fail=false
class Client {
 constructor(options){assert.equal(options.host,'host.example');assert.equal(options.username,'user');assert.equal(options.password,'secret')}
 async connect(){connected++;if(fail)throw Error('Authentication failed')}
 async close(){closed++}
 async realpath(p){return p==='.'?'/home/user':p}
 async list(){return [{name:'image.nii.gz',type:'file'},{name:'folder',type:'directory'},{name:'secret.txt',type:'file'},{name:'..',type:'directory'}]}
 async stat(){return {isFile:true,size:4}}
 createReadStream(){return Readable.from([Buffer.from('data')])}
}
const service=createNetworkBrowser({Client,readJson:async req=>req.body,json:(res,status,body)=>{res.status=status;res.body=body},isAllowed:name=>name.endsWith('.nii.gz')})
const invoke=async(action,{method,token,body,path='.'}={})=>{const req={method:method||(['connect','disconnect'].includes(action)?'POST':'GET'),headers:token?{'x-brainana-remote-token':token}:{},body,aborted:false};const res={destroyed:false};await service.handle(req,res,new URL(`http://localhost/api/network/${action}?path=${encodeURIComponent(path)}`));return res}
try{
 assert.equal((await invoke('connect',{method:'GET'})).status,405)
 assert.equal((await invoke('list')).status,401)
 await assert.rejects(invoke('connect',{body:{host:'bad host',username:'user'}}),/valid hostname/)
 const result=await invoke('connect',{body:{host:'host.example',username:'user',password:'secret'}});const token=result.body.token;assert.equal(token.length,64);assert.equal(result.body.path,'/home/user');assert.equal(JSON.stringify(result.body).includes('secret'),false)
 const list=await invoke('list',{token});assert.equal(list.body.entries.length,2);assert.equal(list.body.entries[0].directory,true);assert.equal(list.body.entries[1].path,'/home/user/image.nii.gz')
 assert.equal((await invoke('list',{token:'forged'})).status,401)
 assert.equal((await invoke('file',{token,path:'/home/user/secret.txt'})).status,415)
 await invoke('disconnect',{token});assert.equal(closed,1);assert.equal((await invoke('list',{token})).status,401)
 fail=true;await assert.rejects(invoke('connect',{body:{host:'host.example',username:'user',password:'secret'}}),/Authentication failed/);assert.equal(closed,2)
 fail=false;for(let i=0;i<4;i++)await invoke('connect',{body:{host:'host.example',username:'user',password:'secret'}});await assert.rejects(invoke('connect',{body:{host:'host.example',username:'user',password:'secret'}}),/Close another network/);
 const profiles=parseSshConfig('Host lab\r\n HostName host.example\r\n User user\r\n Port 2222\r\nHost *\r\n User ignored');assert.equal(profiles[0].hostName,'host.example');assert.equal(profiles[0].port,2222)
}finally{service.close()}
console.log('Network browser: bounded authenticated connections, fixed methods, token isolation, image filtering, absolute directory navigation, disconnect invalidation, failed-auth cleanup and SSH profiles passed.')
