import datetime,hashlib,json,pathlib,subprocess,time
root=pathlib.Path('/workspace/Blueberry-library');out=root/'.verification/consolidated-library-2026-10-03/whole-check';out.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
git=lambda *args:subprocess.check_output(['git',*args],cwd=root,text=True).strip()
assert not git('diff','--name-only','HEAD')
paths=[p for p in git('ls-files').splitlines() if p.startswith(('blueberry/','tests/','verify/','scripts/','.verify-helper/docs/static/_data/')) or p in ['Makefile','README.md'] or (p.startswith('docs/') and not p.startswith('docs/development/'))]
(out/'runner.py').write_bytes(pathlib.Path(__file__).read_bytes())
r={'started_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'revision':git('rev-parse','HEAD'),'tree':git('rev-parse','HEAD^{tree}'),'branch':git('branch','--show-current'),'source_sha256':{p:sha(root/p) for p in paths},'original_full_official_report_sha256':sha(root/'.verification/current.json'),'command':['make','check','CXX=g++','CXX_STANDARD=gnu++20','COMPILE_JOBS=2'],'scope':'New complete local check on integrated code; not full official verification or publication.'}
(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
log=out/'make-check.log';start=time.monotonic()
with log.open('w') as s:result=subprocess.run(r['command'],cwd=root,stdout=s,stderr=subprocess.STDOUT)
r.update(returncode=result.returncode,elapsed_seconds=time.monotonic()-start,log=str(log.relative_to(root)),log_sha256=sha(log),source_unchanged=all(sha(root/p)==h for p,h in r['source_sha256'].items()),original_full_report_unchanged=sha(root/'.verification/current.json')==r['original_full_official_report_sha256'],finished_at=datetime.datetime.now(datetime.timezone.utc).isoformat())
(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
print('integrated make check',r['returncode'],round(r['elapsed_seconds'],2),flush=True)
assert result.returncode==0 and r['source_unchanged'] and r['original_full_report_unchanged']
