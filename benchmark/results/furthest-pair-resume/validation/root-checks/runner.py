import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time

root=Path('/workspace/Blueberry-library')
family='furthest-pair'
out=root/'.verification/furthest-pair-resume'/('root-checks' if len(sys.argv)<2 else sys.argv[1])
out.mkdir(parents=True,exist_ok=False)
sources=['blueberry/geometry/furthest-pair.hpp','blueberry/geometry/convex-hull.hpp','blueberry/all.hpp','tests/random/furthest-pair.cpp','tests/random/convex-hull.cpp','docs/geometry/furthest-pair.md','docs/geometry/convex-hull.md','verify/geometry/furthest-pair.test.cpp','verify/geometry/static-convex-hull.test.cpp','.verify-helper/docs/static/_data/libraries.yml','.verify-helper/docs/static/_data/library_checker.json','docs/library-checker-checklist.md','README.md']
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
sources.extend(['.verify-helper/docs/static/_data/library_verification.json', 'docs/verification-gaps.md'])
(out/'runner.py').write_bytes(Path(__file__).read_bytes())
current=root/'.verification/current.json'
report={'started_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'family':family,'head_before':subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),'source_sha256':{p:digest(root/p) for p in sources},'full_official_verification':False,'preserved_full_official_attempt_sha256':digest(current),'stages':[]}
stages=[('make-check',['make','check','CXX=g++','CXX_STANDARD=gnu++20','COMPILE_JOBS=2'],{},0),('docs-publication-gate',['make','docs'],{},2),('jekyll',['jekyll','build','--source','.verify-helper/markdown','--destination','.build/site','--config','.verify-helper/markdown/_config.yml,/tmp/blueberry-setup/pages-document-plugins.yml'],{'JEKYLL_ENV':'production'},0),('site-preflight-without-metrics',['python3','scripts/check_site.py','--without-metrics'],{},0)]
for name,command,additions,expected in stages:
 log=out/(name+'.log');start=time.monotonic()
 with log.open('w') as stream:
  result=subprocess.run(command,cwd=root,env={**os.environ,**additions},stdout=stream,stderr=subprocess.STDOUT)
 row={'name':name,'command':command,'extra_env':additions,'exit_code':result.returncode,'seconds':time.monotonic()-start,'log':str(log.relative_to(root)),'log_sha256':digest(log),'status':'passed' if result.returncode==0 else 'blocked_or_failed'}
 report['stages'].append(row);(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
 print(name,result.returncode,round(row['seconds'],2),flush=True)
 if result.returncode!=expected:raise SystemExit('Unexpected stage result; preserved log must be reviewed')
report['source_unchanged']=all(digest(root/p)==h for p,h in report['source_sha256'].items())
report['preserved_full_official_attempt_unchanged']=digest(current)==report['preserved_full_official_attempt_sha256']
report['finished_at']=datetime.datetime.now(datetime.timezone.utc).isoformat()
(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
assert report['source_unchanged'] and report['preserved_full_official_attempt_unchanged']
