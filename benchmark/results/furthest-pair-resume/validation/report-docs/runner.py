import datetime,hashlib,json,os,pathlib,subprocess,time
root=pathlib.Path('/workspace/Blueberry-library')
out=root/'.verification/furthest-pair-resume/report-docs'
out.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(out/'runner.py').write_bytes(pathlib.Path(__file__).read_bytes())
names=['docs/development/furthest-pair.md','docs/geometry/furthest-pair.md','blueberry/geometry/furthest-pair.hpp','.verification/current.json']
r={'started_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'scope':'Regenerate and render the newly completed development report only; existing publication metrics gate remains failed. No full-check or official-test rerun.','source_sha256':{p:sha(root/p) for p in names},'stages':[]}
steps=[('generate',['python3','scripts/generate_docs.py','-j','2'],{}),('jekyll',['jekyll','build','--source','.verify-helper/markdown','--destination','.build/site','--config','.verify-helper/markdown/_config.yml,/tmp/blueberry-setup/pages-document-plugins.yml'],{'JEKYLL_ENV':'production'}),('site-preflight-without-metrics',['python3','scripts/check_site.py','--without-metrics'],{})]
for name,cmd,env in steps:
 log=out/(name+'.log');start=time.monotonic()
 with log.open('w') as s:proc=subprocess.run(cmd,cwd=root,env={**os.environ,**env},stdout=s,stderr=subprocess.STDOUT)
 r['stages'].append({'name':name,'command':cmd,'extra_env':env,'returncode':proc.returncode,'elapsed_seconds':time.monotonic()-start,'log':str(log.relative_to(root)),'log_sha256':sha(log)})
 (out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print(name,proc.returncode,flush=True)
 assert proc.returncode==0
r['sources_unchanged']=all(sha(root/p)==h for p,h in r['source_sha256'].items())
assert r['sources_unchanged']
pages=[p for p in (root/'.build/site').rglob('furthest-pair.html') if 'development' in str(p)]
assert len(pages)==1,pages
page=pages[0];text=page.read_text();assert 'b2e6e51f3dd36fb7cd7e8e5883847d02d86d2d2e0e4836a337ac97dfda9c5cef' in text and '91.37' in text and '328' in text
r['development_page']={'path':str(page.relative_to(root)),'sha256':sha(page),'frozen_source_and_key_results_present':True}
r['finished_at']=datetime.datetime.now(datetime.timezone.utc).isoformat()
(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
