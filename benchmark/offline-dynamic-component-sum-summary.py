#!/usr/bin/env python3
"""Compact derived comparisons, retaining all phases and slower cells."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--experiment',default='ordered-generation');args=p.parse_args()
out=ROOT/'.verification/offline-dynamic-connectivity/performance'/args.experiment
s=json.loads((out/'summary.json').read_text());raw=[json.loads(x) for x in (out/'raw.jsonl').read_text().splitlines()]
diag=[json.loads(x) for x in (out/'diagnostics.jsonl').read_text().splitlines()]
prepared=json.loads((out/'prepared.json').read_text())
assert len(raw)==144 and sum(x['warmup'] for x in raw)==36 and len(diag)==16

def get(family,c,m,w,v):return next(x['metrics'] for x in s if (x['family'],x['compiler'],x['mode'],x['workload'],x['variant'])==(family,c,m,w,v))
def comparison(a,b):return dict(public=a,buckets=b,delta_percent=(a['median']/b['median']-1)*100,
                              sample_range='faster' if a['max']<b['min'] else 'slower' if a['min']>b['max'] else 'overlap')
def cell(a):return f"{a['median']/1e6:.3f} [{a['min']/1e6:.3f},{a['max']/1e6:.3f}]"
rows=[]
lines=['# OfflineDynamicComponentSum storage comparison','',
       'CSRを維持する。churnでは全4compiler/modeでfull中央値が短縮して標本範囲も分離し、全4入力でRSSが小さい。一方、longではsolve中央値が全4条件で増加し、GCC/assertのfull遅延は標本範囲も分離した。一律の高速化は主張しない。','',
       '実ヘッダSHA256: `'+prepared['source_sha256']['blueberry/graph/offline-dynamic-component-sum.hpp']+'`。対照は同じヘッダからbucket格納部分とclass名だけを変更。control.diffで全差分を確認できる。',
       '同一long long入力、同一event ID/order、query-time compression、集約undo、RollbackUnionFindを使用。GCC14/Clang19、GNU++20、-O2、release/assert。',
       '各条件warmup1＋測定3。kernel128＋release I/O16profile中warmup36。別allocation8＋uninstrumented RSS8profile。共有ホスト・CPU非固定・4CPU quota。標本範囲は信頼区間ではない。','',
       '## Kernel','',
       '単位ms、中央値[min,max]。%はCSR/publicのnested buckets比、正は遅い。',
       '| compiler/mode | 入力 | public full | buckets full | full %/範囲 | solve % | registration % |',
       '|---|---|---:|---:|---:|---:|---:|']
for c in ['g++','clang++']:
 for m in ['release','assert']:
  for w in ['long','churn','updates','cyclic']:
   a,b=get('kernel',c,m,w,'public'),get('kernel',c,m,w,'buckets')
   metrics={k:comparison(a[k],b[k]) for k in a}
   for k,v in metrics.items():rows.append(dict(compiler=c,mode=m,workload=w,metric=k,**v))
   f=metrics['full_ns']
   lines.append(f"| {c}/{m} | {w} | {cell(a['full_ns'])} | {cell(b['full_ns'])} | {f['delta_percent']:+.2f}%/{f['sample_range']} | {metrics['solve_ns']['delta_percent']:+.2f}% | {metrics['registration_ns']['delta_percent']:+.2f}% |")
lines+=['','constructorは初期値copy、registrationは全操作登録、solveは区間抽出・bucket構築・DFS・回答作成を含む。fullはrecorder破棄まで。すべて同じ呼出し内の計測で、内部工程の独立分離を主張しない。',
        'registrationは実装差のないcontrol。GCC/release cyclicのregistration増加がfull中央値を逆転させた事実も保存する。constructor/registrationを含む全64phase比較と範囲はcomparison.jsonにあり、変動を都合よく除外しない。','',
        '## I/O','',
        '公開APIを固定し、FastIOとunsynchronizediostreamだけを変更。churn N=Q=300000、releaseのみ。parseは入力vector作成、algorithmはconstructor/registration/solve/destruction、formatはflushを含む。process起動は時間外。','',
        '| compiler | FastIO parse/algorithm/format/end-to-end ms | iostream parse/algorithm/format/end-to-end ms | FastIO end-to-end %/範囲 |','|---|---:|---:|---:|']
io=[]
for c in ['g++','clang++']:
 a,b=get('io',c,'release','churn','fast'),get('io',c,'release','churn','iostream')
 effects={k:comparison(a[k],b[k]) for k in a}
 for effect in effects.values():effect['fast']=effect.pop('public');effect['iostream']=effect.pop('buckets')
 io.append(dict(compiler=c,metrics=effects))
 text=lambda r:' / '.join(f"{r[k]['median']/1e6:.3f}" for k in ['parse_ns','algorithm_ns','format_ns','end_to_end_ns'])
 e=effects['end_to_end_ns'];lines.append(f"| {c} | {text(a)} | {text(b)} | {e['delta_percent']:+.2f}%/{e['sample_range']} |")
lines+=['','## Memory','',
        '各条件1回の独立したGCC/release診断。allocationはfull API lifetimeの回数/requested bytesで、instrumented時間は集計しない。RSSは入力・runtime・allocatorを含むfresh processのhigh-water。bucketのみの値ではない。','',
        '| 入力 | public allocations / bytes / RSS KiB | buckets allocations / bytes / RSS KiB |','|---|---:|---:|']
memory=[]
for w in ['long','churn','updates','cyclic']:
 row={'workload':w}
 for v in ['public','buckets']:
  a=next(x['data'] for x in diag if x['workload']==w and x['variant']==v and x['allocation'])
  b=next(x['data'] for x in diag if x['workload']==w and x['variant']==v and not x['allocation'])
  row[v]={k:a[k] for k in ['allocation_calls','allocation_bytes']};row[v]['rss_kib']=b['rss_kib']
 memory.append(row)
 text=lambda v:f"{row[v]['allocation_calls']} / {row[v]['allocation_bytes']} / {row[v]['rss_kib']}"
 lines.append(f"| {w} | {text('public')} | {text('buckets')} |")
lines+=['','5binary各217 BFS program、非default/非inverse型、4large入力の全回答要素一致、40small cross-compiler profilesを準備ゲートで確認。',
        '初回prepareはRNG引数の評価順によりGCC/Clang入力hashが異なり中止。元の失敗証拠を維持し、明示的local変数で生成順を固定したordered-generation実験だけを測定した。API本体の不具合ではなく、失敗回の性能値は存在しない。',
        '再現はbenchmark/offline-dynamic-component-sum.pyのprepare/measure/diagnostics/summarize。計画、source/header/control/binary hashとflagsはplan.md/prepared.json、全生値はraw.jsonl/diagnostics.jsonl。元のdatasetと公式API検証は別記録であり、この性能比較を公式ACとして数えない。','']
(out/'comparison.json').write_text(json.dumps(rows,indent=2)+'\n')
(out/'review-summary.json').write_text(json.dumps(dict(kernel=rows,io=io,memory=memory,
    source_sha256=prepared['source_sha256'],raw_sha256=hashlib.sha256((out/'raw.jsonl').read_bytes()).hexdigest()),indent=2)+'\n')
(out/'report.md').write_text('\n'.join(lines))
print('Derived64phase comparisons and compact report; no new measurements.')
