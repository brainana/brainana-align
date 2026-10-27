"""Catch cross-language identifier mistakes even on CPU-only build hosts.

The optional real Metal compiler check is distinct from this portable source
check; neither is a substitute for executing registration on a physical GPU.
"""
from pathlib import Path
import os,re,shutil,subprocess,tempfile,unittest

ROOT=Path(__file__).resolve().parents[1]
HEADER=ROOT/'source/engine/accel_kernel.h'

def portable_identifiers(source):
    source=re.sub(r'/\*.*?\*/|//[^\n]*','',source,flags=re.S)
    # Native floating-point types are deliberately absent from the shared
    # integer kernel. These tokens cannot be used as local integer names in MSL.
    reserved=re.findall(r'\b(?:half|float|double)(?:[234](?:x[234])?)?\b',source)
    if reserved:raise ValueError('Metal scalar/vector type used in portable integer kernel: '+', '.join(sorted(set(reserved))))

class MetalSourceTests(unittest.TestCase):
    def test_portable_source_avoids_metal_scalar_type_names(self):
        portable_identifiers(HEADER.read_text())

    def test_original_reported_identifier_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'half'):
            portable_identifiers('jip_u64 q=x>>n, r=x&mask, half=1;')

    def test_native_metal_compiler_when_installed(self):
        if not shutil.which('xcrun'):self.skipTest('Apple Metal compiler is unavailable on this host')
        result=subprocess.run(['xcrun','--find','metal'],capture_output=True,text=True)
        if result.returncode:self.skipTest('Apple Metal compiler is not installed/configured')
        compiler=result.stdout.strip()
        version=subprocess.run([compiler,'--version'],capture_output=True,text=True)
        if version.returncode:self.skipTest('Apple Metal toolchain component is unavailable')
        host=(ROOT/'source/engine/accel_metal.mm').read_text()
        suffix=re.search(r'stringByAppendingString:@"(.*?)"\]',host,re.S).group(1)
        suffix=suffix.replace(r'\n','\n').replace(r'\"','"')
        with tempfile.TemporaryDirectory(prefix='jip-metal-source-') as folder:
            path=Path(folder)/'interpolation.metal';path.write_text(HEADER.read_text()+suffix)
            result=subprocess.run([compiler,'-std=metal3.0','-c',str(path),'-o',str(Path(folder)/'interpolation.air')],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)

if __name__=='__main__':unittest.main()
