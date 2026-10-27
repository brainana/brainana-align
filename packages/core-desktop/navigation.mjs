// A server port is part of the trust boundary; other loopback origins are external.
export function isInternalUrl(appUrl,target) {
  try {
    const base=new URL(appUrl),url=new URL(target)
    return base.protocol==='http:' && base.hostname==='127.0.0.1' && url.origin===base.origin
  } catch {return false}
}
export function isExternalWebUrl(target) {
  try {const url=new URL(target);return ['http:','https:'].includes(url.protocol) && !['127.0.0.1','localhost','::1','[::1]'].includes(url.hostname)}catch{return false}
}
export const WINDOW_PREFERENCES=Object.freeze({contextIsolation:true,nodeIntegration:false,sandbox:true,webSecurity:true})
