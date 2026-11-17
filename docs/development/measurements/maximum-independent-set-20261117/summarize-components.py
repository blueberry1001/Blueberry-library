from pathlib import Path
import json,statistics,sys
name=sys.argv[1] if len(sys.argv)>1 else 'results-components.json'
data=json.loads(Path(name).read_text())
samples=[x for x in data['samples'] if x.get('rep',-1)>=0 and x.get('status','ok')=='ok']
for key in sorted(set((x['compiler'],x['n'],x['count'],x['shape']) for x in samples)):
    rows=[x for x in samples if (x['compiler'],x['n'],x['count'],x['shape'])==key]
    variants=list(dict.fromkeys(x['variant'] for x in rows))
    print(*key,*(f"{v}={statistics.median(x['milliseconds'] for x in rows if x['variant']==v):.5f}" for v in variants))
