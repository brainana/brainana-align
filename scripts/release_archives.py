"""Shared verified release archives and inventories."""
import hashlib, json, os, stat, zipfile, shutil

def digest(path):
    h = hashlib.sha256()
    if path.is_symlink():
        return hashlib.sha256(os.readlink(path).encode()).hexdigest()
    with path.open('rb') as handle:
        for block in iter(lambda: handle.read(1024*1024), b''):
            h.update(block)
    return h.hexdigest()

def files(folder):
    return sorted(p for p in folder.rglob('*') if p.is_file() or p.is_symlink())

def inventory(folder):
    links = [{'path':p.relative_to(folder).as_posix(),'target':os.readlink(p)} for p in files(folder) if p.is_symlink()]
    (folder/'SYMLINKS.json').write_text(json.dumps(links,indent=2)+'\n')
    listing = sorted(set([p.relative_to(folder).as_posix() for p in files(folder)] + ['RELEASE_CONTENTS.txt','SHA256SUMS.txt']))
    (folder/'RELEASE_CONTENTS.txt').write_text('\n'.join(listing)+'\n')
    paths = [p for p in files(folder) if not p.is_symlink() and (p.name != 'SHA256SUMS.txt' or p.parent != folder)]
    (folder/'SHA256SUMS.txt').write_text(''.join(f'{digest(p)}  {p.relative_to(folder).as_posix()}\n' for p in paths))

def archive(folder, target):
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for p in files(folder):
            if p.is_symlink():
                info = zipfile.ZipInfo(p.relative_to(folder).as_posix())
                info.create_system = 3
                info.external_attr = (stat.S_IFLNK | 0o777) << 16
                z.writestr(info, os.readlink(p).encode())
            else:
                z.write(p, p.relative_to(folder).as_posix())
    with zipfile.ZipFile(target) as z:
        bad = z.testzip()
        if bad:
            raise RuntimeError(f'ZIP integrity failure: {bad}')
        for line in (folder/'SHA256SUMS.txt').read_text().splitlines():
            sha, name = line.split('  ', 1)
            if hashlib.sha256(z.read(name)).hexdigest() != sha:
                raise RuntimeError(f'Archive checksum failure: {name}')
        for link in json.loads((folder/'SYMLINKS.json').read_text()):
            info = z.getinfo(link['path'])
            if not stat.S_ISLNK(info.external_attr >> 16) or z.read(link['path']).decode() != link['target']:
                raise RuntimeError(f'Archive symbolic link failure: {link["path"]}')
    print(f'Verified {target.name}: {target.stat().st_size} bytes')



def link_or_copy(src,dst):
    """Stage verified immutable payloads without duplicating them on the same disk."""
    try: os.link(src,dst)
    except OSError: shutil.copy2(src,dst)
    return dst
