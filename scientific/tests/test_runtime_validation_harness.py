"""Exercise clean Windows keys and timeout cleanup with a real child process."""
import ast,os,shutil,subprocess,sys,tempfile,time
from pathlib import Path
source=Path(__file__).with_name('validate_runtime.py')
module=ast.parse(source.read_text())
functions=ast.Module(body=[n for n in module.body if isinstance(n,ast.FunctionDef) and n.name in ['clean','frozen_run']],type_ignores=[])
with tempfile.TemporaryDirectory(prefix='align-harness-') as temporary:
    scope=dict(os=os,shutil=shutil,subprocess=subprocess,sys=sys,Path=Path,ROOT=Path(temporary))
    exec(compile(functions,str(source),'exec'),scope)
    before=dict(os.environ)
    try:
        os.environ['SYSTEMROOT']=os.environ.get('SYSTEMROOT','C:/Windows')
        os.environ['TEMP']=temporary
        os.environ['PYTHONHOME']='must-not-inherit'
        env=scope['clean']()
        assert env['SYSTEMROOT']==os.environ['SYSTEMROOT'] and env['TEMP']==temporary
        assert not any(k.upper()=='PYTHONHOME' for k in env)
        started=time.monotonic()
        try:
            scope['frozen_run']([sys.executable,'-c','import time;print("started",flush=True);time.sleep(30)'],timeout=.3)
            raise AssertionError('Slow child must fail')
        except RuntimeError as error:
            assert 'timed out' in str(error)
        assert time.monotonic()-started<20
        assert (Path(temporary)/'artifacts/validation/frozen-timeout/stdout.txt').is_file()
    finally:
        os.environ.clear();os.environ.update(before)
print('Runtime harness: uppercase OS keys retained, Python home excluded, real timed-out child stopped and evidence preserved.')
