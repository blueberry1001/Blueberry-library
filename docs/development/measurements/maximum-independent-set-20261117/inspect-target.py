import gzip
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

here = Path(__file__).resolve().parent
build = here/'build'/'assembly'
build.mkdir(parents=True,exist_ok=True)
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
msvc = vc/'bin/Hostx64/x64/cl.exe'
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(msvc.parent) + os.pathsep + env.get('PATH','')
env['INCLUDE'] = os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
sha = lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
results = {'status':'running','driver_sha256':sha(here/'assembly.cpp'),'sources':{},'rows':[]}
def save(): (here/'assembly-target-results.json').write_text(json.dumps(results,indent=2)+'\n',encoding='utf-8')
try:
    for variant in ('components','components-popcnt-cached'):
        folder=build/variant;folder.mkdir(exist_ok=True)
        header=here/(variant+'.hpp');shutil.copyfile(header,folder/'candidate.hpp')
        results['sources'][variant]=sha(header)
        for mode in ('clang-portable',):
            output=folder/(mode+'.s')
            if mode == 'msvc-portable':
                cmd=[str(msvc),'/nologo','/std:c++20','/EHsc','/O2','/DNDEBUG','/c','/FAs','/Fa'+str(output),'/Fo'+str(folder/(mode+'.obj')),'/I'+str(folder),str(here/'assembly.cpp')]
            else:
                cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-std=gnu++20','-O2','-DNDEBUG','-S','-masm=intel','-I'+str(folder),str(here/'assembly.cpp'),'-o',str(output)]
                if mode == 'clang-popcnt': cmd.insert(-2,'-mpopcnt')
            p=subprocess.run(cmd,cwd=folder,env=env,capture_output=True,text=True,errors='replace')
            row={'variant':variant,'mode':mode,'command':cmd,'exit':p.returncode,'output':p.stdout+p.stderr}
            results['rows'].append(row);save()
            if p.returncode: raise RuntimeError(f'assembly failed: {variant} {mode}')
            data=output.read_bytes()
            text=data.decode('utf-8',errors='replace')
            row['raw_sha256']=hashlib.sha256(data).hexdigest()
            row['instruction_matches']={ins:len(re.findall(r'(?im)^\s*(?:rep\s+)?'+ins+r'\s',text)) for ins in ('popcnt','bsf','tzcnt','bsr','lzcnt')}
            row['packed_file']=f'{variant}-{mode}.s.gz'
            with (here/row['packed_file']).open('wb') as target:
                with gzip.GzipFile(filename='',mode='wb',fileobj=target,mtime=0) as stream: stream.write(data)
            row['gzip_sha256']=sha(here/row['packed_file']);save()
            print(variant,mode,row['instruction_matches'],flush=True)
    results['status']='complete'
except BaseException as exc:
    results['status']='failed';results['error']=repr(exc);save();raise
save()
