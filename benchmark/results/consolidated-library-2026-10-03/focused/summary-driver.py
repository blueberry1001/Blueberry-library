#!/usr/bin/env python3
import hashlib,json,re,shutil
from collections import Counter
from pathlib import Path
from datetime import datetime,timezone
R=Path('/workspace/Blueberry-library');F=R/'.verification/consolidated-library-2026-10-03/focused'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def ref(p):
 p=Path(p);return {'path':str(p.relative_to(R)),'sha256':sha(p),'bytes':p.stat().st_size}
r=json.loads((F/'report.json').read_text());assert r['succeeded'] and r['finished_at'];assert r['revision']=='a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c'
c=Counter();seeds={}
for row in r['steps']:
 assert row['status']=='passed' and row['returncode']==0,row['label'];assert sha(R/row['log'])==row['log_sha256'],row['log']
 s=row['label']
 if s.startswith('all.hpp include '):kind='all_hpp_standalone_duplicate_checks'
 elif s.startswith('combined compile '):kind='combined_api_compiles'
 elif s.startswith('combined execute '):kind='combined_api_runs'
 elif s.startswith('random compile '):kind='random_compiles'
 elif s.startswith('random '):
  kind='random_runs';target,seed=s.rsplit(' seed=',1);seeds.setdefault(target,[]).append(int(seed))
 elif s.startswith('sanitizer compile '):kind='sanitizer_compiles'
 elif s.startswith('sanitizer execute '):kind='sanitizer_runs'
 else:raise RuntimeError(s)
 c[kind]+=1
assert dict(c)=={'all_hpp_standalone_duplicate_checks':16,'combined_api_compiles':8,'combined_api_runs':8,'random_compiles':7,'random_runs':140,'sanitizer_compiles':3,'sanitizer_runs':3}
assert len(r['steps'])==185 and len(seeds)==7 and all(v==list(range(1,21)) for v in seeds.values())
for name,h in r['source_sha256'].items():assert sha(R/name)==h and sha(F/'source-snapshot'/name)==h,name
for name,h in r['binary_sha256'].items():assert sha(R/name)==h,name
assert sha(F/'validation-driver.py')==r['driver_sha256'] and sha(F/'combined-smoke.cpp')==r['combined_smoke_sha256']
assert sha(R/'.verification/current.json')==r['preserved_full_report_sha256']
identity=json.loads((F/'historical-official-identity.json').read_text());assert identity['revision']==r['revision'] and identity['all_project_include_closures_identical']
output={'recorded_at':datetime.now(timezone.utc).isoformat(),'revision':r['revision'],'succeeded':True,'report':ref(F/'report.json'),'passed_steps':len(r['steps']),'counts':dict(c),'source_sha256':r['source_sha256'],'source_snapshots_match_current':True,'compiler_versions':r['compiler_versions'],'driver':ref(F/'validation-driver.py'),'combined_smoke':ref(F/'combined-smoke.cpp'),'random_seeds':list(range(1,21)),'random_scope':list(seeds),'sanitizer':{'scope':['combined all.hpp smoke','dynamic-fenwick-get','persistent-segment-get'],'compiler':'clang++ GNU23','seed':20261003,'address_sanitizer':True,'undefined_behavior_sanitizer':True,'leak_detection':True},'historical_official_identity':ref(F/'historical-official-identity.json'),'official_tests_executed':0,'full_official_verification':False,'notes':['No prior test or performance evidence was overwritten.','The eight combined-smoke executions retain explicit runtime checks under -DNDEBUG.','Historical official source identity is separate from a new official run; the PST point-set-composite prior record lacks a contemporaneous external ACL identity binding.','Whole GCC20 make-check and documentation/site results are owned by the root integration task.']}
with (F/'summary.json').open('x') as f:f.write(json.dumps(output,indent=2)+'\n')
shutil.copyfile(__file__,F/'summary-driver.py');shutil.copyfile('/tmp/blueberry-setup/audit_consolidated_official_identity.py',F/'historical-official-identity-driver.py')
print(json.dumps({'report':ref(F/'report.json'),'summary':ref(F/'summary.json'),'source_count':len(r['source_sha256']),'logs':len(r['steps']),'log_bytes':sum((R/x['log']).stat().st_size for x in r['steps'])},indent=2))
