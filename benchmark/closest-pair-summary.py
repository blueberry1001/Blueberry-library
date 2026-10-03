#!/usr/bin/env python3
"""Derive a compact report from immutable closest-pair raw records."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--experiment', default='initial')
args = parser.parse_args()
out = ROOT / '.verification/closest-pair/benchmark' / args.experiment
prepared = json.loads((out / 'prepared.json').read_text())
raw = [json.loads(x) for x in (out / 'raw.jsonl').read_text().splitlines()]
diagnostics = [json.loads(x) for x in (out / 'diagnostics.jsonl').read_text().splitlines()]
for name, expected in prepared['source_sha256'].items():
    assert hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == expected
assert len(raw) == 352 and sum(r['warmup'] for r in raw) == 88
assert len(diagnostics) == 36

def stat(rows, key):
    a = [r['data'][key] for r in rows]
    assert len(a) == 3
    return {'median': statistics.median(a), 'min': min(a), 'max': max(a)}

def group(family, compiler, mode, workload, n=None, variant=None):
    return [r for r in raw if not r['warmup'] and r['family'] == family and
            r['compiler'] == compiler and r['mode'] == mode and r['workload'] == workload and
            (n is None or r.get('n') == n) and (variant is None or r.get('variant', r.get('method')) == variant)]

def cell(s):
    return f"{s['median']/1e6:.3f} [{s['min']/1e6:.3f}, {s['max']/1e6:.3f}]"

comparisons = []
lines = ['# Closest pair — bounded performance investigation', '',
         '公開実ヘッダと、独立実装のper-node-index-vector分割統治・balanced-tree sweepを同条件で比較した。',
         '全候補は符号付き64bit座標、|c|≤2^62−1、正確な128bit距離、元添字対の辞書順最小tie、debug座標検査を揃える。',
         '元の公開資料は型・距離・tie契約が異なるため、そのままの提出時間とは比較しない。Fastest API403は既知の制約として維持し、再試行していない。', '',
         f"Production SHA256: `{prepared['source_sha256']['blueberry/geometry/closest-pair.hpp']}`。", '',
         'GCC14/Clang19、GNU++20、-O2。releaseは-DNDEBUG、assertは有効。生成seed0x43504c4f53455354。',
         '各条件warmup1回＋測定3回、候補順を循環。kernel288profile＋I/O64profile（計352、warmup88）。',
         '別途allocation18＋RSS18profile。6binary各2071caseの全対比較oracle、108small profile、9件の期待SIGABRTを準備ゲートで確認した。',
         '共有ホスト・CPU非固定・cgroup4CPU。min/maxは3標本の範囲であり、統計的信頼区間ではない。', '',
         '## 完全なアルゴリズム呼出し', '',
         '単位ms、中央値[min,max]。制御候補の%はpublic比（正は遅い）。入力生成・hashは時間外。', '',
         '| compiler/mode | 入力/N | public ms [min,max] | vectors % | sweep % |',
         '|---|---|---:|---:|---:|']
for compiler in ['g++', 'clang++']:
    for mode in ['release', 'assert']:
        for workload, n in prepared['cells']:
            base = stat(group('kernel', compiler, mode, workload, n, 'blueberry'), 'full_ns')
            effects = []
            for variant in ['vectors', 'sweep']:
                candidate = stat(group('kernel', compiler, mode, workload, n, variant), 'full_ns')
                delta = (candidate['median'] / base['median'] - 1) * 100
                separation = 'slower' if candidate['min'] > base['max'] else 'faster' if candidate['max'] < base['min'] else 'overlap'
                comparisons.append(dict(compiler=compiler, mode=mode, workload=workload, n=n,
                                        variant=variant, public=base, control=candidate,
                                        delta_percent=delta, sample_range=separation))
                effects.append(f'{delta:+.1f}% ({separation})')
            lines.append(f'| {compiler}/{mode} | {workload}/{n} | {cell(base)} | {effects[0]} | {effects[1]} |')
lines += ['', 'prepare/searchは制御候補だけの別の計測呼出しであり、fullに足し合わせない。prepareには検査・record copy・sort・duplicate scan、searchにはDC/sweep本体を含む。publicの内訳はnull。',
          'vectorsはallocationだけでなくrecords/indices配置と比較式も異なる。allocation回数だけを時間差の唯一の原因とはしない。', '', '## I/O', '',
          'publicアルゴリズムを固定し、FastIOとunsynchronizediostreamを独立に変更。単位ms。parseは入力container構築、solveは回答vector作成、formatはflushを含む。',
          'end-to-endは同一実行内のparse＋solve＋formatで、process startupを含まない。', '',
          '| compiler/mode | 入力 | FastIO parse/solve/format/end-to-end中央値 | iostream parse/solve/format/end-to-end中央値 |',
          '|---|---|---:|---:|']
for compiler in ['g++', 'clang++']:
    for mode in ['release', 'assert']:
        for workload in ['random', 'tiny']:
            cells = []
            for method in ['fast', 'iostream']:
                rows = group('io', compiler, mode, workload, variant=method)
                cells.append(' / '.join(f"{stat(rows,k)['median']/1e6:.3f}" for k in ['parse_ns','solve_ns','format_ns','end_to_end_ns']))
            lines.append(f'| {compiler}/{mode} | {workload} | {cells[0]} | {cells[1]} |')
lines += ['', '## メモリ診断', '',
          '別のfresh processでGCCを使用。allocationはfull呼出し中の回数/requested bytesで、計測用newを使った時間は性能集計から除外。',
          'RSSは別のuninstrumented診断のwhole-process high-waterで入力生成・runtime・allocatorを含む。staged callを実行しない。通常sampleのRSSは比較に使わない。', '',
          '| mode | 入力 | variant | allocation calls | requested bytes | peak RSS KiB |', '|---|---|---|---:|---:|---:|']
for r in [x for x in diagnostics if x['allocation']]:
    other = next(x for x in diagnostics if not x['allocation'] and all(x[k] == r[k] for k in ['mode','workload','variant']))
    lines.append(f"| {r['mode']} | {r['workload']} | {r['variant']} | {r['data']['allocation_calls']} | {r['data']['allocation_bytes']} | {other['data']['rss_kib']} |")
lines += ['', '## 再現・調査資料', '',
          '`benchmark/closest-pair.py`のprepare/measure/diagnostics/summarizeを、新しいexperiment名で順に実行する。source /tmp/blueberry-setup/env.shが必要。既存experimentは上書きしない。',
          '入力・source・binary・compiler flags・環境はprepared.json、全生値はraw.jsonl/diagnostics.jsonl、phaseのmedian/min/maxはsummary.json、全48対比較はcomparison.json/csv。',
          '詳しい事前計画は../../benchmark-plan.md。公式29caseの生成・58hash照合は../../datasets/report.json。公式checkerはany-tieを認めるため、lexicographic契約は独立oracleで確認する。',
          '公開資料: [LC pinned DC](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/closest_pair/sol/correct.cpp), [KACTL CC0 sweep](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/ClosestPair.h), [maspypy DC/randomized grid](https://github.com/maspypy/library/blob/main/geo/closest_pair.hpp)。',
          'KACTL/maspypyは取得2026-10-03のsource hashを../../research/metadata.jsonに保存。アルゴリズム研究のみでソースコードをコピーしていない。ランダムgridは期待計算量や契約が異なるため測定代替案には採用していない。', '']
(out / 'comparison.json').write_text(json.dumps(comparisons, indent=2) + '\n')
with (out / 'comparison.csv').open('w', newline='') as stream:
    fields = ['compiler','mode','workload','n','variant','delta_percent','sample_range','public_median','public_min','public_max','control_median','control_min','control_max']
    writer = csv.DictWriter(stream, fieldnames=fields, lineterminator='\n'); writer.writeheader()
    for row in comparisons:
        writer.writerow({**{k:row[k] for k in fields if k in row}, **{f'{v}_{s}':row[v][s] for v in ['public','control'] for s in ['median','min','max']}})
(out / 'report.md').write_text('\n'.join(lines))
print(f'Report and48comparisons derived from352profiles; no extra measurements.')
