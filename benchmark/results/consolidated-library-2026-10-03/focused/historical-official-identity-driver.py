#!/usr/bin/env python3
"""Record historical official evidence and exact local include identity; never claim a rerun."""
import hashlib,json,re,subprocess
from pathlib import Path
R=Path('/workspace/Blueberry-library');O=R/'.verification/consolidated-library-2026-10-03/focused';ACL=R/'.deps/ac-library'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def ref(p):
 p=Path(p);return {'path':str(p.relative_to(R)),'sha256':sha(p),'bytes':p.stat().st_size}
def git(*args,cwd=R):return subprocess.check_output(['git',*args],cwd=cwd)
def closure(start):
 seen={};external={};todo=[(R/start,False)]
 while todo:
  p,is_external=todo.pop();name=str(p.relative_to(ACL if is_external else R));mapping=external if is_external else seen
  if name in mapping:continue
  mapping[name]=sha(p)
  for inc in re.findall(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]',p.read_text(),re.M):
   if inc.startswith('blueberry/'):todo.append((R/inc,False))
   elif inc.startswith('atcoder/'):todo.append((ACL/inc,True))
 return seen,external
G=R/'.verification/integration-graph/official/current.json';ST=R/'.verification/static-tree-string-tuning/official-resume/report.json';DF=R/'.verification/point-access-tuning/dynamic-fenwick/final/report.json';PS=R/'.verification/point-access-tuning/persistent-segment/final/report.json';FP=R/'.verification/furthest-pair-resume/final/report.json'
selected=[(G,'verify/graph/dominator-tree.test.cpp'),(G,'verify/graph/general-matching.test.cpp'),(ST,'verify/graph/static-top-tree.test.cpp'),(DF,'verify/data-structure/dynamic-point-add-range-sum.test.cpp'),(PS,'verify/data-structure/persistent-rectangle-sum.test.cpp'),(PS,'verify/data-structure/persistent-point-set-range-composite.test.cpp'),(FP,'verify/geometry/furthest-pair.test.cpp'),(FP,'verify/geometry/static-convex-hull.test.cpp')]
rows=[]
for report_path,target in selected:
 prior=read(report_path);project,external=closure(target)
 if report_path==G:
  row=next(x for x in prior['results'] if x['path']==target)
  hashes=read(R/'benchmark/results/repertoire-integration/validation.json')['header_sha256'].copy();hashes[target]=row['verifier_sha256'];repeats=row['repeat_count']
 else:
  official = prior['official'] if isinstance(prior['official'],list) else [prior['official']]
  row=next(x for x in official if x['target']==target);hashes=prior['source_sha256'];repeats=row['repeats']
 assert row['status']=='passed' and repeats==3 and len(row['runs'])==3,target
 binding=[]
 for name,current in project.items():
  assert name in hashes and current==hashes[name],target+': changed/unbound project dependency '+name
  binding.append({'path':name,'sha256':current,'matches_historical_record':True})
 acl_record={'current_include_closure_sha256':external,'historical_revision':prior.get('acl_revision'),'historical_external_identity_proved':not external}
 if external and prior.get('acl_revision'):
  rev=prior['acl_revision'];assert git('rev-parse','HEAD',cwd=ACL).decode().strip()==rev
  for name,h in external.items():assert hashlib.sha256(git('show',rev+':'+name,cwd=ACL)).hexdigest()==h,name
  acl_record['historical_external_identity_proved']=True
 elif external:
  acl_record['limitation']='Prior focused record lacks a contemporaneous ACL revision/content binding. Current external hashes are recorded only; no exact historical external-identity claim.'
 rows.append({'target':target,'historical_report':ref(report_path),'historical_recorded_revision':prior['revision'],'historical_status':'passed','historical_case_count':row['case_count'],'historical_repeats':repeats,'historical_dataset_sha256':row['dataset_sha256'],'current_project_include_closure':binding,'all_project_dependencies_identical':True,'acl':acl_record,'new_official_execution':False})
result={'revision':git('rev-parse','HEAD').decode().strip(),'targets':rows,'all_project_include_closures_identical':True,'official_tests_executed_in_integration':0,'full_official_verification':False,'policy':'Preserve prior successful scoped official results as historical evidence. Exact source matches justify bounded new integration checks, not relabeling old AC as a new full or merged official run. External-binding limitations remain explicit. AOJ was not retried.'}
with (O/'historical-official-identity.json').open('x') as f:f.write(json.dumps(result,indent=2)+'\n')
print(json.dumps({'report':ref(O/'historical-official-identity.json'),'targets':len(rows),'current_project_files':sum(len(r['current_project_include_closure']) for r in rows),'external_binding_gaps':[r['target'] for r in rows if not r['acl']['historical_external_identity_proved']]},indent=2))
