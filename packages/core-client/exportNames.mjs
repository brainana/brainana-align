// Shared by the browser save dialog and the native shell. A volume and its
// metadata retain the same stem when the user renames the selected output.
export function exportFilename(value) {
  if (typeof value !== 'string' || !value.trim() || value.length > 240 || /[\\/:*?"<>|\x00-\x1f]/.test(value) || /^\.+$/.test(value.trim()) || /[. ]$/.test(value)) throw Error('Enter a filename without folder separators or reserved characters.')
  return value.trim()
}
const stem = name => name.replace(/\.nii\.gz$|\.[^.]+$/i, '')
export function exportFilenames(suggested, selected) {
  if (!Array.isArray(suggested) || !suggested.length || suggested.length > 16) throw Error('Invalid export file group')
  suggested.forEach(exportFilename)
  let first = exportFilename(selected)
  const extension = suggested[0].match(/\.nii\.gz$|\.(?:json|txt)$/i)?.[0]
  if (extension && !first.toLowerCase().endsWith(extension.toLowerCase())) first = first.replace(extension.toLowerCase() === '.nii.gz' ? /\.nii$/i : /$^/, '') + extension
  const original = stem(suggested[0]), renamed = stem(first)
  const names = [first, ...suggested.slice(1).map(name => name.startsWith(original + '.') ? renamed + name.slice(original.length) : first === suggested[0] ? name : renamed + '_' + name)]
  names.forEach(exportFilename)
  if (new Set(names.map(name => name.toLowerCase())).size !== names.length) throw Error('Output filenames must be distinct')
  return names
}
