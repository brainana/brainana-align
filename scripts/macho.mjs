// Structural validation of the exact binary shipped; a checksum alone can
// faithfully identify a truncated executable. Cryptographic signing is separate.
export function inspectMachO(binary, expectedCpu) {
  if (binary.length < 32 || binary.readUInt32LE(0) !== 0xfeedfacf) throw new Error('Expected a little-endian 64-bit Mach-O executable')
  const cpu = binary.readUInt32LE(4)
  if (expectedCpu !== undefined && cpu !== expectedCpu) throw new Error('Mach-O CPU architecture does not match package directory')
  const commands = binary.readUInt32LE(16), commandBytes = binary.readUInt32LE(20)
  if (32 + commandBytes > binary.length) throw new Error('Mach-O load commands are truncated')
  let offset = 32, minimumMacOS = null, signature = false
  const bounds = (start, size, label) => {
    if (BigInt(start) + BigInt(size) > BigInt(binary.length)) throw new Error(`${label} points past end of Mach-O file (truncated executable)`)
  }
  for (let i = 0; i < commands; i++) {
    if (offset + 8 > 32 + commandBytes) throw new Error('Mach-O command header is truncated')
    const command = binary.readUInt32LE(offset), size = binary.readUInt32LE(offset+4)
    if (size < 8 || offset + size > 32 + commandBytes) throw new Error('Mach-O load command has an invalid size')
    if (command === 0x19) {
      if (size < 72) throw new Error('Invalid LC_SEGMENT_64')
      bounds(binary.readBigUInt64LE(offset+40), binary.readBigUInt64LE(offset+48), 'Mach-O segment')
    }
    if ([0x1d,0x26,0x29,0x80000033,0x80000034].includes(command)) {
      if (size < 16) throw new Error('Invalid Mach-O linkedit command')
      bounds(binary.readUInt32LE(offset+8),binary.readUInt32LE(offset+12),'Mach-O linkedit/signature')
      if(command===0x1d) signature = true
    }
    if (command === 0x2) {
      if(size<24) throw new Error('Invalid Mach-O symbol command')
      bounds(binary.readUInt32LE(offset+8),binary.readUInt32LE(offset+12)*16,'Mach-O symbols')
      bounds(binary.readUInt32LE(offset+16),binary.readUInt32LE(offset+20),'Mach-O strings')
    }
    if (command === 0x32 || command === 0x24) {
      if(size<(command===0x32?24:16)) throw new Error('Invalid macOS deployment command')
      minimumMacOS = binary.readUInt32LE(offset+(command===0x32?12:8))
    }
    offset += size
  }
  if(offset !== 32+commandBytes) throw new Error('Mach-O load-command inventory is inconsistent')
  if(minimumMacOS===null) throw new Error('Mach-O deployment target is missing')
  return {cpu,minimumMacOS,signature,bytes:binary.length}
}
