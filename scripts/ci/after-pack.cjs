const path=require('node:path');const {execFileSync}=require('node:child_process')
module.exports=async function(context){
 if(context.electronPlatformName!=='darwin')return
 const app=path.join(context.appOutDir,context.packager.appInfo.productFilename+'.app')
 execFileSync('/usr/bin/codesign',['--force','--deep','--sign','-',app],{stdio:'inherit'})
}
