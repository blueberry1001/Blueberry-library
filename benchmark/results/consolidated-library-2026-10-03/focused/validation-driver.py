#!/usr/bin/env python3
"""Bounded merged-source integration verification; offline and one compiler at a time."""
import hashlib,json,os,platform,resource,shutil,subprocess,time
from pathlib import Path
from datetime import datetime,timezone
R=Path('/workspace/Blueberry-library');O=R/'.verification/consolidated-library-2026-10-03/focused'
REV='a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c'
SMOKE=Path('/tmp/blueberry-setup/integration_all_api_smoke.cpp')
TESTS=['dominator-tree','general-matching','convex-hull','static-top-tree','dynamic-fenwick-get','persistent-segment-get','furthest-pair']
MODES=[('g++','gnu++20'),('g++','gnu++23'),('clang++','gnu++20'),('clang++','gnu++23')]
HEADERS=['graph/dominator-tree','graph/general-matching','geometry/convex-hull','geometry/furthest-pair','graph/static-top-tree','data-structure/dynamic-fenwick-tree','data-structure/persistent-segment-tree']
def require(ok,msg):
 if not ok:raise RuntimeError(msg)
def sha(p):
 h=hashlib.sha256()
 with Path(p).open('rb') as f:
  for b in iter(lambda:f.read(1048576),b''):h.update(b)
 return h.hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=R).decode().strip()
def ref(p):
 p=Path(p);return {'path':str(p.relative_to(R)) if p.is_relative_to(R) else str(p),'sha256':sha(p),'bytes':p.stat().st_size}
def flags(c,s):return [c,'-std='+s,'-O2','-pipe','-Wall','-Wextra','-Wshadow','-Werror','-I',str(R),'-isystem',str(R/'.deps/ac-library')]
def save():(O/'report.json').write_text(json.dumps(report,indent=2)+'\n')
def sources_unchanged():
 for name,h in report['source_sha256'].items():require(sha(R/name)==h,'Source changed: '+name)
 require(sha(R/'.verification/current.json')==report['preserved_full_report_sha256'],'Preserved full report changed')
def run(label,cmd,env=None,source=None,timeout=240):
 sources_unchanged();row={'label':label,'command':cmd,'status':'running'}
 if env:row['environment']={k:env[k] for k in ['BLUEBERRY_RANDOM_SEED','ASAN_OPTIONS','UBSAN_OPTIONS'] if k in env}
 report['steps'].append(row);save();log=O/('%03d.log'%len(report['steps']));start=time.monotonic()
 try:
  result=subprocess.run(cmd,cwd=R,env=env,input=source,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=timeout,check=False)
  log.write_text(result.stdout);row.update(returncode=result.returncode,status='passed' if result.returncode==0 else 'failed')
 except subprocess.TimeoutExpired as e:
  output=e.stdout or b'';output=output.decode(errors='replace') if isinstance(output,bytes) else output
  log.write_text(output+'\nTIMEOUT\n');row.update(returncode=124,status='failed')
 row.update(elapsed_seconds=time.monotonic()-start,log=str(log.relative_to(R)),log_sha256=sha(log));save()
 print(row['status'].upper()+' '+label,flush=True);require(row['status']=='passed','Failed stage: '+label)
def binary(p):report['binary_sha256'][str(p.relative_to(R))]=sha(p);save()
require(git('rev-parse','HEAD')==REV,'Expected merge-freeze revision changed')
O.mkdir(parents=True,exist_ok=False);resource.setrlimit(resource.RLIMIT_CORE,(0,0))
sources=sorted(str(p.relative_to(R)) for p in (R/'blueberry').rglob('*.hpp'))
sources += ['tests/random/'+name+'.cpp' for name in TESTS]+['docs/'+name+'.md' for name in HEADERS]
report={'started_at':datetime.now(timezone.utc).isoformat(),'succeeded':False,'revision':REV,'branch':git('branch','--show-current'),'source_sha256':{name:sha(R/name) for name in sources},'driver_sha256':sha(__file__),'combined_smoke_sha256':sha(SMOKE),'compiler_versions':{c:subprocess.check_output([c,'--version'],text=True).splitlines()[0] for c in ['g++','clang++']},'platform':platform.platform(),'preserved_full_report_sha256':sha(R/'.verification/current.json'),'scope':'Merged all.hpp integration and selected Clang GNU23 random/sanitizer checks; no official rerun and no full-official/publication pass claim.','full_official_verification':False,'steps':[],'binary_sha256':{}}
try:
 require('14.' in report['compiler_versions']['g++'] and '19.' in report['compiler_versions']['clang++'],'Expected GCC14 and Clang19')
 shutil.copyfile(__file__,O/'validation-driver.py');shutil.copyfile(SMOKE,O/'combined-smoke.cpp')
 for name in sources:
  p=O/'source-snapshot'/name;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(R/name,p)
 save()
 for c,s in MODES:
  for release in [False,True]:
   mode=c+'-'+s+('-release' if release else '-assert');f=flags(c,s)+(['-DNDEBUG'] if release else [])
   for count in [1,2]:
    code='#include "blueberry/all.hpp"\n'*count+'int main() {}\n'
    run('all.hpp include '+mode+' count='+str(count),f+['-x','c++','-','-fsyntax-only'],source=code)
   p=O/('combined-'+mode);run('combined compile '+mode,f+[str(O/'combined-smoke.cpp'),'-o',str(p)]);binary(p)
   run('combined execute '+mode,[str(p)])
 for name in TESTS:
  p=O/('random-'+name);run('random compile '+name,flags('clang++','gnu++23')+['tests/random/'+name+'.cpp','-o',str(p)]);binary(p)
  for seed in range(1,21):run('random '+name+' seed='+str(seed),[str(p),str(seed)],env=dict(os.environ,BLUEBERRY_RANDOM_SEED=str(seed)))
 for name,source in [('combined',str(O/'combined-smoke.cpp')),('dynamic-fenwick-get','tests/random/dynamic-fenwick-get.cpp'),('persistent-segment-get','tests/random/persistent-segment-get.cpp')]:
  p=O/('sanitized-'+name);f=[('-O1' if x=='-O2' else x) for x in flags('clang++','gnu++23')]
  run('sanitizer compile '+name,f+['-g','-fno-omit-frame-pointer','-fsanitize=address,undefined','-fno-sanitize-recover=all',source,'-o',str(p)]);binary(p)
  run('sanitizer execute '+name+' seed=20261003',[str(p),'20261003'],env=dict(os.environ,BLUEBERRY_RANDOM_SEED='20261003',ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
 sources_unchanged();require(len(report['steps'])==185,'Unexpected final step count')
 for name,h in report['binary_sha256'].items():require(sha(R/name)==h,'Compiled binary changed: '+name)
 report['succeeded']=True
except BaseException as e:report['failure']=repr(e);raise
finally:
 report['finished_at']=datetime.now(timezone.utc).isoformat();save()
print('PASS integration185: '+str(O/'report.json'),flush=True)
