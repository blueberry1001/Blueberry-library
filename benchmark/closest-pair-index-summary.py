#!/usr/bin/env python3
"""Derive the separately preserved shared-index follow-up; never pool initial samples."""
import argparse
import csv
import json
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(); p.add_argument('--experiment', default='index-layout'); args = p.parse_args()
out = ROOT / '.verification/closest-pair/benchmark' / args.experiment
raw = [json.loads(x) for x in (out / 'raw.jsonl').read_text().splitlines()]
assert len(raw) == 192 and sum(r['warmup'] for r in raw) == 48
rows = []
for compiler in ['g++', 'clang++']:
    for mode in ['release', 'assert']:
        for workload, n in [('random',20000),('random',100000),('duplicates',100000),('grid',100000),('nearline',100000),('extreme',100000)]:
            stats = {}
            for variant in ['blueberry', 'indices']:
                r = [x for x in raw if not x['warmup'] and (x['compiler'],x['mode'],x['workload'],x['n'],x['variant']) == (compiler,mode,workload,n,variant)]
                assert len(r) == 3
                v = [x['data']['full_ns'] for x in r]
                stats[variant] = dict(median=statistics.median(v), min=min(v), max=max(v))
            a,b=stats['blueberry'],stats['indices']
            separated = 'faster' if b['max']<a['min'] else 'slower' if b['min']>a['max'] else 'overlap'
            rows.append(dict(compiler=compiler,mode=mode,workload=workload,n=n,public=a,candidate=b,
                             delta_percent=(b['median']/a['median']-1)*100,sample_range=separated))
(out/'comparison.json').write_text(json.dumps(rows,indent=2)+'\n')
with (out/'comparison.csv').open('w',newline='') as f:
    fields=['compiler','mode','workload','n','delta_percent','sample_range','public_median','public_min','public_max','candidate_median','candidate_min','candidate_max']
    w=csv.DictWriter(f,fieldnames=fields,lineterminator='\n');w.writeheader()
    for r in rows:w.writerow({**{k:r[k] for k in fields if k in r},**{f'{a}_{s}':r[a][s] for a in ['public','candidate'] for s in ['median','min','max']}})
text=['# Shared y-index layout follow-up','','初回とは別の測定。公開ヘッダd949453aを固定し、x-sorted records＋共有int order/scratch案と比較した。',
      'GCC14/Clang19、GNU++20 -O2、release/assertを分離。同じ6入力・seed、warmup1＋測定3回。192profile中warmup48。I/O/sweepの再測定なし。',
      '中央値/min/maxは3標本の観測範囲。共有ホスト・CPU非固定で統計的信頼区間ではない。layout/比較式も変わるためコピー量だけへの因果帰属はしない。','',
      '| compiler/mode | 入力/N | public ms [min,max] | candidate ms [min,max] | candidate比 | 範囲 |',
      '|---|---|---:|---:|---:|---|']
for r in rows:
    def c(s):return f"{s['median']/1e6:.3f} [{s['min']/1e6:.3f},{s['max']/1e6:.3f}]"
    text.append(f"| {r['compiler']}/{r['mode']} | {r['workload']}/{r['n']} | {c(r['public'])} | {c(r['candidate'])} | {r['delta_percent']:+.2f}% | {r['sample_range']} |")
text += ['',f"中央値短縮{sum(r['delta_percent']<0 for r in rows)}/24、短縮の標本範囲分離{sum(r['sample_range']=='faster' for r in rows)}/24。遅い中央値{sum(r['delta_percent']>0 for r in rows)}/24、遅い標本範囲分離{sum(r['sample_range']=='slower' for r in rows)}/24。",'',
         'search phaseにはorder/scratchの確保・初期化を含み、prepareは座標検査・record copy・sort・duplicate scan。いずれも独立したinstrumented callで、full中央値と足し合わせない。public phasesはnull。',
         '手順は../../index-layout-plan.md、source/hash/flagsと2071case×6binary oracle＋72small profile＋6SIGABRTゲートはprepared.json。生値raw.jsonl、別診断diagnostics.jsonl、全phase範囲summary.json。','']
(out/'report.md').write_text('\n'.join(text))
print('Derived24follow-upcomparisons, original evidence untouched.')
