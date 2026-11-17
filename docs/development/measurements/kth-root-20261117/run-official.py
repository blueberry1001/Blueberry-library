import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

root = Path.cwd()
here = root / '.build/kth-root-research'
work = here / 'official-work'
source = root / '.verification/library-checker-problems/number_theory/kth_root_integer'
common = root / '.verification/library-checker-problems/common'
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
msvc = vc / 'bin/Hostx64/x64/cl.exe'
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(msvc.parent) + os.pathsep + env.get('PATH', '')
env['INCLUDE'] = os.pathsep.join(map(str, [vc/'include', *(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str, [vc/'lib/x64', *(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
for d in ['gen','inputs','outputs','baseline/blueberry/math','candidate/blueberry/math']:
  (work/d).mkdir(parents=True,exist_ok=True)
(work/'params.h').write_text('constexpr int T_MAX = 500000;\n')
for p in (source/'gen').glob('*.cpp'): shutil.copyfile(p,work/'gen'/p.name)
shutil.copyfile(here/'baseline.hpp',work/'baseline/blueberry/math/kth-root-integer.hpp')
shutil.copyfile(here/'candidate.hpp',work/'candidate/blueberry/math/kth-root-integer.hpp')
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
result={'status':'running','problem_git_commit':'73876f1cbcf082d2b22722924b924d3d65a2f00e','inputs':[],'builds':[],'samples':[], 'warning':'Windows Clang; load from other processes may interfere. Full wall-clock includes process startup, I/O and teardown.'}
def save(): (here/'official-results.json').write_text(json.dumps(result,indent=2)+'\n')
flags=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG']
def build(src,output,extra=[]):
  cmd=flags+extra+[str(src),'-o',str(output)]
  p=subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
  result['builds'].append({'src':str(src),'sha256':sha(src),'command':cmd,'exit':p.returncode,'output':p.stdout+p.stderr});save()
  if p.returncode: raise RuntimeError('build failed '+str(src))
try:
  example=work/'inputs/example_00.in';shutil.copyfile(source/'gen/example_00.in',example)
  result['inputs'].append({'path':str(example),'sha256':sha(example),'bytes':example.stat().st_size,'source':'official example'})
  for name in ['all_k2','all_k3','all_k3_2','near_border','near_border_2','random']:
    generator=work/'gen'/f'{name}.cpp';exe=work/f'gen-{name}.exe'
    build(generator,exe,['-I'+str(common)])
    for seed in range(3 if name.startswith('near_border') else 2):
      path=work/'inputs'/f'{name}-{seed}.in'
      with path.open('wb') as output:
        p=subprocess.run([str(exe),str(seed)],env=env,stdout=output,stderr=subprocess.PIPE,timeout=300)
      if p.returncode: raise RuntimeError(f'generator {name}-{seed} failed: {p.stderr!r}')
      result['inputs'].append({'path':str(path),'sha256':sha(path),'bytes':path.stat().st_size,'generator_sha256':sha(generator),'seed':seed});save()
  correct=work/'correct.exe'
  build(source/'sol/correct.cpp',correct)
  for entry in result['inputs']:
    inp=Path(entry['path']);answer=work/'outputs'/f'{inp.stem}.ans'
    with inp.open('rb') as f, answer.open('wb') as o:
      p=subprocess.run([str(correct)],env=env,stdin=f,stdout=o,stderr=subprocess.PIPE,timeout=300)
    if p.returncode: raise RuntimeError(f'correct {inp.name} failed: {p.stderr!r}')
    entry['answer_sha256']=sha(answer);save()
  exes={}
  for name in ['baseline','candidate','nyaan']:
    exe=work/f'{name}.exe'
    driver=here/('official-nyaan.cpp' if name=='nyaan' else 'official.cpp')
    build(driver,exe,['-I'+str(here),'-I'+str(work/name)] if name!='nyaan' else ['-I'+str(here)])
    exes[name]=exe
  for entry in result['inputs']:
    inp=Path(entry['path'])
    for rep in range(-1,3):
      names=list(exes);names=names[rep%len(names):]+names[:rep%len(names)]
      for name in names:
        out=work/'outputs'/f'{inp.stem}-{name}.out'
        with inp.open('rb') as f,out.open('wb') as o:
          start=time.perf_counter();p=subprocess.run([str(exes[name])],env=env,stdin=f,stdout=o,stderr=subprocess.PIPE,timeout=300);elapsed=time.perf_counter()-start
        row={'input':inp.name,'variant':name,'rep':rep,'elapsed_s':elapsed,'exit':p.returncode,'sha256':sha(out)}
        result['samples'].append(row);save()
        if p.returncode or row['sha256']!=entry['answer_sha256']: raise RuntimeError('wrong output: '+str(row))
    print(inp.name,'passed',flush=True)
  result['status']='complete'
except BaseException as error:
  result['status']='failed';result['error']=repr(error);save();raise
save()
