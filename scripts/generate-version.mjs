import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const here = path.dirname(fileURLToPath(import.meta.url))
const root = path.resolve(here, '..')
const source = path.join(root, 'apps/align')
const metadata = JSON.parse(fs.readFileSync(path.join(root, 'VERSION.json'), 'utf8'))
for (const file of [path.join(source,'VERSION.json'),path.join(root,'Documentation/VERSION.json')]) fs.writeFileSync(file, JSON.stringify(metadata,null,2)+'\n')
const builderPath=path.join(source,'electron-builder.yml')
if (fs.existsSync(builderPath)) {
  let config=fs.readFileSync(builderPath,'utf8')
  config=config.replace(/bundleVersion: '[^']*'/, `bundleVersion: '${metadata.bundleVersion}'`).replace(/bundleShortVersion: '[^']*'/, `bundleShortVersion: '${metadata.version.split('-')[0]}'`).replace(/minimumSystemVersion: '[^']*'/, `minimumSystemVersion: '${metadata.minimumMacOS}'`).replace(/BrainanaReleaseVersion: .*/, `BrainanaReleaseVersion: ${metadata.version}`).replace(/BrainanaBuildID: .*/, `BrainanaBuildID: ${metadata.buildId}`)
  fs.writeFileSync(builderPath,config)
}
const required = ['app','version','buildId','bundleVersion','bundleIdentifier','minimumMacOS','behaviorReference','readableSourceBase']
for (const key of required) {
  if (typeof metadata[key] !== 'string' || !metadata[key].trim()) throw new Error(`VERSION.json missing ${key}`)
}
const js = `// Generated from VERSION.json. Do not edit.\nexport const VERSION=${JSON.stringify(metadata.version)}\nexport const BUILD_ID=${JSON.stringify(metadata.buildId)}\nexport const BEHAVIOR_REFERENCE=${JSON.stringify(metadata.behaviorReference)}\nexport const SOURCE_BASE=${JSON.stringify(metadata.readableSourceBase)}\n`
fs.writeFileSync(path.join(source, 'version.mjs'), js)
fs.writeFileSync(path.join(source, 'src', 'version.ts'), js)
const env = `# Generated from VERSION.json. Do not edit.\nVERSION=${JSON.stringify(metadata.version)}\nBUILD_ID=${JSON.stringify(metadata.buildId)}\n`
fs.writeFileSync(path.join(source, 'version.env'), env)
const pkgPath = path.join(source, 'package.json')
const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'))
pkg.version = metadata.version
fs.writeFileSync(pkgPath, JSON.stringify(pkg, null, 2) + '\n')
const rootPkgPath = path.join(root, 'package.json')
const rootPkg = JSON.parse(fs.readFileSync(rootPkgPath, 'utf8'))
rootPkg.version = metadata.version
fs.writeFileSync(rootPkgPath, JSON.stringify(rootPkg, null, 2) + '\n')
const lockPath = path.join(root, 'package-lock.json')
const lock = JSON.parse(fs.readFileSync(lockPath, 'utf8'))
lock.version = metadata.version
if (lock.packages?.['']) lock.packages[''].version = metadata.version
for (const entry of Object.keys(lock.packages || {})) {
  if (entry === 'apps/align' || entry.startsWith('packages/')) lock.packages[entry].version = metadata.version
}
for (const name of fs.readdirSync(path.join(root,'packages'))) {
  const file = path.join(root,'packages',name,'package.json')
  const shared = JSON.parse(fs.readFileSync(file,'utf8'))
  shared.version = metadata.version
  fs.writeFileSync(file, JSON.stringify(shared,null,2)+'\n')
}
fs.writeFileSync(lockPath, JSON.stringify(lock, null, 2) + '\n')
const launcherTemplate = path.join(root, 'packaging', 'templates', 'brainana-align-launcher.in')
fs.copyFileSync(launcherTemplate, path.join(root, 'packaging', 'brainana-align-launcher'))
fs.chmodSync(path.join(root, 'packaging', 'brainana-align-launcher'), 0o755)
const plist = `<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n<plist version="1.0">\n<dict>\n\t<key>CFBundleDisplayName</key>\n\t<string>${metadata.app}</string>\n\t<key>CFBundleExecutable</key>\n\t<string>brainana-align-launcher</string>\n\t<key>CFBundleIdentifier</key>\n\t<string>${metadata.bundleIdentifier}</string>\n\t<key>CFBundleName</key>\n\t<string>${metadata.app}</string>\n\t<key>CFBundlePackageType</key>\n\t<string>APPL</string>\n\t<key>CFBundleShortVersionString</key>\n\t<string>${metadata.version.split('-')[0]}</string>\n\t<key>BrainanaReleaseVersion</key>\n\t<string>${metadata.version}</string>\n\t<key>CFBundleVersion</key>\n\t<string>${metadata.bundleVersion}</string>\n\t<key>LSMinimumSystemVersion</key>\n\t<string>${metadata.minimumMacOS}</string>\n</dict>\n</plist>\n`
fs.mkdirSync(path.join(root, 'generated'), {recursive:true})
fs.writeFileSync(path.join(root, 'generated', 'Info.plist'), plist)
fs.writeFileSync(path.join(root, 'generated', 'VERSION.json'), JSON.stringify(metadata, null, 2) + '\n')
console.log(`${metadata.version} (${metadata.buildId})`)
