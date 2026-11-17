import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import time

root = Path.cwd()
record = root / 'docs/development/measurements/wavelet-20261117'
build = root / '.build/wavelet-investigation-final'
build.mkdir(parents=True, exist_ok=True)
baseline = (record / 'baseline.hpp.txt').read_text()
current = (record / 'final.hpp.txt').read_text()
shutil.copyfile(record / 'second-driver.cpp.txt', build / 'driver.cpp')
separate = '''    std::vector<std::uint64_t> bits;
    std::vector<int> prefix;
    int zeros = 0;

    int rank(int r) const {
      const int block = r / 64, offset = r % 64;
      return prefix[block] + std::popcount(bits[block] & ((std::uint64_t{1} << offset) - 1));
    }'''
interleaved = '''    struct Block { std::uint64_t bits = 0; int prefix = 0; };
    std::vector<Block> data;
    int zeros = 0;

    int rank(int r) const {
      const int block = r / 64, offset = r % 64;
      return data[block].prefix + std::popcount(data[block].bits & ((std::uint64_t{1} << offset) - 1));
    }'''
assert separate in current
candidate = current.replace(separate,interleaved)
candidate = candidate.replace('      level.bits.assign(blocks, 0);\n      level.prefix.assign(blocks, 0);','      level.data.assign(blocks, {});')
candidate = candidate.replace('level.bits[i / 64]', 'level.data[i / 64].bits')
candidate = candidate.replace('level.prefix[b] = level.prefix[b - 1] + std::popcount(level.bits[b - 1]);','level.data[b].prefix = level.data[b - 1].prefix + std::popcount(level.data[b - 1].bits);')
candidate = candidate.replace('level.bits[p / 64]', 'level.data[p / 64].bits')
assert 'level.bits' not in candidate and 'level.prefix' not in candidate
variants = {'baseline':baseline,'current':current,'interleaved':candidate}
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
version = '10.0.22621.0'
msvc = vc / 'bin/Hostx64/x64/cl.exe'
env = os.environ.copy()
env['PATH'] = str(msvc.parent) + os.pathsep + env.get('PATH','')
env['INCLUDE'] = os.pathsep.join(map(str,[vc/'include', *(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str,[vc/'lib/x64', *(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
compilers = {'msvc':(msvc,['/nologo','/std:c++20','/EHsc','/O2','/DNDEBUG']),
             'clang':(clang,['--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG'])}
out = {'status':'running','platform':platform.platform(),'cpu':platform.processor(),'samples':[],
       'notes':'Other Codex sessions may run concurrently; no CPU isolation. Measurements are not comparable to a different environment.',
       'sha256':{},'compilers':{k:{'path':str(v[0]),'flags':v[1]} for k,v in compilers.items()},'n':200000,'q':100000,'warmups':1,'runs':5}
def save(): (record/'second-results.json').write_text(json.dumps(out,indent=2)+'\n')
try:
  for name, text in variants.items():
    folder = build/name
    folder.mkdir(parents=True,exist_ok=True)
    header = folder/'wavelet-matrix.hpp'
    header.write_text(text,newline='\n')
    out['sha256'][name] = hashlib.sha256(header.read_bytes()).hexdigest()
    if name == 'interleaved': shutil.copyfile(header,record/'interleaved.hpp.txt')
    for comp,(compiler,flags) in compilers.items():
      obj=folder/(comp+'.obj'); exe=folder/(comp+'.exe')
      if comp=='msvc': cmd=[str(compiler),*flags,'/I'+str(folder),str(build/'driver.cpp'),'/Fo'+str(obj),'/Fe'+str(exe)]
      else: cmd=[str(compiler),*flags,'-I'+str(folder),str(build/'driver.cpp'),'-o',str(exe)]
      p=subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
      if p.returncode: raise RuntimeError(str((cmd,p.stdout,p.stderr)))
  for comp in compilers:
    for shape in ['full','duplicates','narrow','sorted','extreme','biased']:
      for iteration in range(-1,5):
        names=list(variants); shift=(iteration+1)%len(names); checksum=None
        for name in names[shift:]+names[:shift]:
          p=subprocess.run([str(build/name/(comp+'.exe')),'200000','100000',shape],capture_output=True,text=True,errors='replace')
          if p.returncode: raise RuntimeError(str((comp,name,shape,p.stdout,p.stderr)))
          fields=p.stdout.split()
          if checksum is None: checksum=fields[0]
          if checksum != fields[0]: raise RuntimeError(str((comp,name,shape,checksum,fields)))
          if iteration>=0:
            out['samples'].append({'compiler':comp,'shape':shape,'iteration':iteration,'variant':name,'checksum':checksum,'build_ms':float(fields[1]),'kth_ms':float(fields[2]),'range_ms':float(fields[3]),'equal_ms':float(fields[4]),'narrow_ms':float(fields[5])})
      save()
      for name in variants:
        rows=[x for x in out['samples'] if x['compiler']==comp and x['shape']==shape and x['variant']==name]
        print(comp,shape,name,*[round(statistics.median(x[k] for x in rows),3) for k in ('build_ms','kth_ms','range_ms','equal_ms','narrow_ms')],flush=True)
  out['status']='complete'
except BaseException as exc:
  out['status']='failed'; out['error']=repr(exc); save(); raise
save()
