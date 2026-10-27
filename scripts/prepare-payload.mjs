import fs from 'node:fs'
import path from 'node:path'
export function preparePayload(root, target, nodes=[]) {
  // Recreate the payload so stale Node binaries cannot leak into a slim build.
  fs.rmSync(target,{recursive:true,force:true})
  const contents=path.join(target,'Contents'),runtime=path.join(contents,'Resources/runtime')
  fs.mkdirSync(path.join(contents,'MacOS'),{recursive:true});fs.mkdirSync(runtime,{recursive:true})
  fs.copyFileSync(path.join(root,'packaging/brainana-align-launcher'),path.join(contents,'MacOS/brainana-align-launcher'))
  fs.chmodSync(path.join(contents,'MacOS/brainana-align-launcher'),0o755)
  for(const name of ['version.mjs','version.env'])fs.copyFileSync(path.join(root,'apps/align',name),path.join(runtime,name))
  for(const name of ['platformCore.mjs','sftpClient.mjs','replaceWithBackup.mjs','runtime.mjs','networkBrowser.mjs','networkSftpClient.mjs','knownHosts.mjs'])fs.copyFileSync(path.join(root,'packages/core-server',name),path.join(runtime,name))
  for(const name of ['ssh2','asn1','bcrypt-pbkdf','safer-buffer','tweetnacl','streamsearch']) {const source=path.join(root,'node_modules',name);if(fs.existsSync(source))fs.cpSync(source,path.join(runtime,'node_modules',name),{recursive:true,filter:filename=>!filename.endsWith('.node')})}
  fs.copyFileSync(path.join(root,'packaging/runtime-command.sh'),path.join(runtime,'runtime-command.sh'))
  const cli=fs.readFileSync(path.join(root,'apps/align/server.mjs'),'utf8').replace('@brainana/core-server/runtime.mjs','./runtime.mjs')
  fs.writeFileSync(path.join(runtime,'server.mjs'),cli)
  fs.cpSync(path.join(root,'apps/align/dist'),path.join(runtime,'dist'),{recursive:true})
  for(const [arch,input] of nodes){const dest=path.join(runtime,'darwin-'+arch,'node');fs.mkdirSync(path.dirname(dest),{recursive:true});fs.copyFileSync(input,dest);fs.chmodSync(dest,0o755)}
  if(nodes.length)fs.copyFileSync(path.join(root,'packaging/NODE-LICENSE.txt'),path.join(runtime,'NODE-LICENSE.txt'))
}
