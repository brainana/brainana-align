import path from 'node:path'
// @electron/asar traverses its header using the host's path separator.
export const asarEntry=(parts,pathApi=path)=>pathApi.join(...parts)
