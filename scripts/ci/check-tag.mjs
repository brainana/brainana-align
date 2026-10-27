import fs from 'node:fs'
const version=JSON.parse(fs.readFileSync('VERSION.json','utf8')).version
const tag=process.env.RELEASE_TAG||process.env.GITHUB_REF_NAME
if(process.env.GITHUB_EVENT_NAME!=='workflow_dispatch'&&tag&&tag!=='v'+version)throw Error(`Release tag ${tag} must equal v${version}; set VERSION.json and run npm run version:generate before committing.`)
if(process.env.GITHUB_OUTPUT)fs.appendFileSync(process.env.GITHUB_OUTPUT,`version=${version}\n`)
console.log(version)
