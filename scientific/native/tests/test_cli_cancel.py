from platform_tools import executable
from pathlib import Path
import argparse,json,math,os,queue,signal,struct,subprocess,threading,time
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True);a=p.parse_args()
R=Path(__file__).resolve().parents[1];O=R/'validation'/('cli-cancel-'+a.build_dir.name);O.mkdir(parents=True,exist_ok=True)
side=32;h=bytearray(352);struct.pack_into('<i',h,0,348);struct.pack_into('<8h',h,40,3,side,side,side,1,1,1,1)
struct.pack_into('<2h',h,70,16,32);struct.pack_into('<8f',h,76,1,1,1,1,1,0,0,0);struct.pack_into('<f',h,108,352);h[123]=2
struct.pack_into('<2h',h,252,0,1);struct.pack_into('<12f',h,280,1,0,0,0,0,1,0,0,0,0,1,0);h[344:348]=b'n+1\0'
values=[1+80*math.exp(-((x-15)**2+(y-14)**2+(z-13)**2)/30) for z in range(side) for y in range(side) for x in range(side)]
(O/'image.nii').write_bytes(h+struct.pack('<'+str(len(values))+'f',*values))
sentinel=b'existing accepted output must survive cancellation';(O/'previous.nii').write_bytes(sentinel);(O/'previous.jipcp').write_bytes(sentinel)
command=[str(executable(a.build_dir.resolve(),'jip-register')),'--fixed','image.nii','--moving','image.nii','--grid','1','--metric','nc','--output','previous.nii','--checkpoint','previous.jipcp']
process=subprocess.Popen(command,cwd=O,stdout=subprocess.PIPE,stderr=subprocess.PIPE,creationflags=subprocess.CREATE_NEW_PROCESS_GROUP if os.name=='nt' else 0)
chunks=queue.Queue()
def reader():
    while True:
        block=process.stdout.read1(65536);chunks.put(block)
        if not block:break
thread=threading.Thread(target=reader,daemon=True);thread.start()
captured=b'';deadline=time.monotonic()+60
try:
    while b'Registration is fitting coefficients.' not in captured:
        assert time.monotonic()<deadline,'runner never reached fitting'
        try:block=chunks.get(timeout=1)
        except queue.Empty:continue
        assert block,'runner ended before cancellation';captured+=block
    process.send_signal(signal.CTRL_BREAK_EVENT if os.name=='nt' else signal.SIGINT)
    process.wait(timeout=10);thread.join(timeout=2);err=process.stderr.read()
    while not chunks.empty():captured+=chunks.get_nowait()
    assert process.returncode==130,(process.returncode,captured,err)
    assert b'accepted result preserved' in err,(captured,err)
    assert (O/'previous.nii').read_bytes()==sentinel and (O/'previous.jipcp').read_bytes()==sentinel
    assert not list(O.glob('*.tmp'))
    (O/'run.log').write_bytes(captured+err)
    (O/'results.json').write_text(json.dumps({'status':'PASS','signal':'CTRL_BREAK_EVENT' if os.name=='nt' else 'SIGINT','during':'coefficient fitting','exit_code':process.returncode,'existing_outputs':'unchanged','temporary_files':0},indent=2)+'\n')
finally:
    if process.poll() is None:process.kill();process.wait()
    thread.join(timeout=2);process.stdout.close();process.stderr.close()
print('CLI_CANCEL_RESULT interrupted active fitting; exit 130; previous outputs unchanged')
