import datetime,hashlib,json,os,pathlib,subprocess,time
root=pathlib.Path('/workspace/Blueberry-library');base=root/'.verification/consolidated-library-2026-10-03';out=base/'docs';out.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(out/'runner.py').write_bytes(pathlib.Path(__file__).read_bytes())
whole=json.loads((base/'whole-check/report.json').read_text());assert whole['returncode']==0 and whole['source_unchanged']
dev='docs/development/consolidated-library-2026-10-03.md'
sources={**whole['source_sha256'],dev:sha(root/dev)}
assert all(sha(root/p)==h for p,h in sources.items())
r={'started_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'revision':subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),'scope':'Generate integrated documentation, preserve failed metrics publication gate, then render and check site structure only. Not a deployment or full official verification.','source_sha256':sources,'original_full_report_sha256':sha(root/'.verification/current.json'),'stages':[]}
steps=[('make-docs',['make','docs'],{},2),('jekyll',['jekyll','build','--source','.verify-helper/markdown','--destination','.build/site','--config','.verify-helper/markdown/_config.yml,/tmp/blueberry-setup/pages-document-plugins.yml'],{'JEKYLL_ENV':'production'},0),('site-preflight-without-metrics',['python3','scripts/check_site.py','--without-metrics'],{},0)]
for name,cmd,env,expected in steps:
 log=out/(name+'.log');start=time.monotonic()
 with log.open('w') as s:proc=subprocess.run(cmd,cwd=root,env={**os.environ,**env},stdout=s,stderr=subprocess.STDOUT)
 r['stages'].append({'name':name,'command':cmd,'extra_env':env,'returncode':proc.returncode,'expected_returncode':expected,'status':'passed' if proc.returncode==0 else 'blocked_or_failed','elapsed_seconds':time.monotonic()-start,'log':str(log.relative_to(root)),'log_sha256':sha(log)})
 (out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print(name,proc.returncode,flush=True)
 assert proc.returncode==expected
 if name=='make-docs':assert 'do not publish unsupported or failed measurements' in log.read_text()
r['sources_unchanged']=all(sha(root/p)==h for p,h in sources.items());assert r['sources_unchanged']
r['original_full_report_unchanged']=sha(root/'.verification/current.json')==r['original_full_report_sha256'];assert r['original_full_report_unchanged']
page=root/'.build/site/docs/development/consolidated-library-2026-10-03.html';text=page.read_text();assert 'a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c' in text and '185' in text and '54' in text
r['rendered_report']={'path':str(page.relative_to(root)),'sha256':sha(page),'exact_merge_and_final_test_counts_present':True}
r['finished_at']=datetime.datetime.now(datetime.timezone.utc).isoformat();(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
