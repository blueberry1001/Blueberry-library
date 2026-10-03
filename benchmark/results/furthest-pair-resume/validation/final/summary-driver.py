#!/usr/bin/env python3
"""Audit completed focused evidence without rerunning tests or changing prior logs."""
import hashlib,json,re,shutil
from collections import Counter
from pathlib import Path
from datetime import datetime,timezone
R=Path('/workspace/Blueberry-library');B=R/'.verification/furthest-pair-resume';F=B/'final'
def require(c,m):
 if not c:raise RuntimeError(m)
def sha(p):
 h=hashlib.sha256()
 with Path(p).open('rb') as f:
  for chunk in iter(lambda:f.read(1048576),b''):h.update(chunk)
 return h.hexdigest()
def ref(p):
 p=Path(p);return {'path':str(p.relative_to(R)) if p.is_relative_to(R) else str(p),'sha256':sha(p),'bytes':p.stat().st_size}
def read(p):return json.loads(Path(p).read_text())
r=read(F/'report.json');require(r['succeeded'] and r.get('finished_at'),'Focused run not complete')
counts=Counter();negative=[]
for row in r['steps']:
 label=row['label'];log=R/row['log'];require(sha(log)==row['log_sha256'],'Step log changed: '+label)
 require(row['status']=='passed' and row['returncode']==row['expected_returncode'],'Unexpected step outcome: '+label)
 if 'expected_output_text' in row:require(row['expected_output_text'] in log.read_text(),'Expected diagnostic absent: '+label)
 if row['expected_returncode']!=0:negative.append({'label':label,'expected_returncode':row['expected_returncode'],'actual_returncode':row['returncode'],'log':ref(log)})
 if label.startswith('doc '):kind='documentation_compiles' if ' compile ' in label else 'documentation_runs'
 elif label.startswith('header '):kind='header_checks'
 elif label.startswith('contract type '):kind='unsupported_type_rejections'
 elif label.startswith('contract coordinate compile '):kind='coordinate_contract_compiles'
 elif label.startswith('contract coordinate assertion '):kind='coordinate_assertion_rejections'
 elif label.startswith('contract valid '):kind='valid_coordinate_controls'
 elif label.startswith('random compile '):kind='random_compiles'
 elif label.startswith('random '):kind='random_runs'
 elif label.startswith('verify compile '):kind='official_driver_compiles'
 elif label.startswith('sanitizer compile '):kind='sanitizer_compiles'
 elif label.startswith('sanitizer '):kind='sanitizer_runs'
 elif label.startswith('official '):kind='official_commands'
 else:raise RuntimeError('Unrecognized step label: '+label)
 counts[kind]+=1
expected={'documentation_compiles':4,'documentation_runs':4,'header_checks':48,'unsupported_type_rejections':24,'coordinate_contract_compiles':4,'coordinate_assertion_rejections':48,'valid_coordinate_controls':8,'random_compiles':8,'random_runs':160,'official_driver_compiles':8,'sanitizer_compiles':2,'sanitizer_runs':4,'official_commands':6}
require(dict(counts)==expected,'Unexpected executed step inventory: '+str(counts));require(len(r['steps'])==328,'Unexpected total steps')
for name,digest in r['source_sha256'].items():
 require(sha(R/name)==digest and sha(F/'source-snapshot'/name)==digest,'Source/snapshot changed: '+name)
require(sha(F/'validation-driver.py')==r['driver_sha256'],'Executed driver snapshot differs')
require(sha(R/'.verification/current.json')==r['original_full_report_sha256'],'Preserved full report changed')
for name,digest in r['binaries_sha256'].items():require(sha(R/name)==digest,'Compiled binary changed: '+name)
seeds={}
for row in r['steps']:
 if row['label'].startswith('random ') and not row['label'].startswith('random compile '):
  label,seed=row['label'].rsplit(' seed=',1);seeds.setdefault(label,[]).append(int(seed))
require(len(seeds)==8 and all(v==list(range(1,21)) for v in seeds.values()),'Random seed inventory differs')
expected_targets={'verify/geometry/furthest-pair.test.cpp':46,'verify/geometry/static-convex-hull.test.cpp':25}
require({d['target']:d['case_count'] for d in r['official']}==expected_targets,'Official inventory differs')
official=[]
for d in r['official']:
 require(d['status']=='passed' and len(d['runs'])==3,'Official repetitions incomplete')
 expected_names={Path(name).stem for name in d['files_sha256'] if name.endswith('.in')}
 for run in d['runs']:
  raw=R/run['raw_log'];norm=R/run['log'];require(sha(raw)==run['raw_log_sha256'] and sha(norm)==run['log_sha256'],'Official log changed')
  original,samples=read(raw),read(norm)
  require(len(original)==len(samples)==d['case_count'],'Official case count differs')
  require(len({x['name'] for x in samples})==len(samples) and {x['name'] for x in samples}==expected_names,'Official names differ')
  require(all(x['status']=='AC' and x['exitcode']==0 for x in samples),'Non-AC official case')
  require([{'name':x['testcase']['name'],'status':x['status'],'elapsed':x['elapsed'],'exitcode':x['exitcode'],'memory':x.get('memory')} for x in original]==samples,'Normalized official samples differ from raw')
 official.append({'target':d['target'],'case_count':d['case_count'],'repeats':3,'AC_cases':3*d['case_count'],'dataset_sha256':d['dataset_sha256'],'checker_sha256':d['checker_sha256'],'upstream_revision':d['upstream_revision'],'runs':d['runs']})
generation=read(B/'preparation/generation/report.json');require(generation['succeeded'] and generation['exit_code']==0 and generation['case_count']==46,'Official offline generation incomplete')
require(sha(R/generation['log'])==generation['log_sha256'],'Generation log changed')
output={'recorded_at':datetime.now(timezone.utc).isoformat(),'succeeded':True,'revision_recorded_at_run':r['revision'],'report':ref(F/'report.json'),'source_sha256':r['source_sha256'],'driver':ref(F/'validation-driver.py'),'counts':dict(counts),'total_passed_steps':328,'random_seeds':list(range(1,21)),'official':official,'total_official_AC':sum(d['AC_cases'] for d in official),'negative_contracts':{'expected_rejections':negative,'release_invalid_coordinate_execution':False,'core_dumps_enabled':False,'note':'Passed means observed the expected compile rejection or SIGABRT for unsupported input; these return codes are deliberately nonzero.'},'sanitizer':{'compiler':'Clang19','standard':'gnu++23','flags':['-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined','-fno-sanitize-recover=all'],'seeds':[1,20261003],'leak_detection':True},'official_preparation':{'generation':ref(B/'preparation/generation/report.json'),'cache':ref(B/'preparation/cache/report.json'),'datasets':ref(B/'preparation/cache/datasets.json')},'full_official_verification':False,'scope':'Focused furthest_pair plus convex_hull official regression only; no AOJ retry or full-repository publication gate claim.'}
with (F/'summary.json').open('x') as f:f.write(json.dumps(output,indent=2,ensure_ascii=False)+'\n')
shutil.copyfile(__file__,F/'summary-driver.py')
print(json.dumps({'summary':ref(F/'summary.json'),'report':ref(F/'report.json'),'steps':328,'official_AC':output['total_official_AC'],'source_count':len(r['source_sha256'])},indent=2))
