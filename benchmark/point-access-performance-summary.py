#!/usr/bin/env python3
"""Derive one final-code batch report; leave all raw/frozen artifacts untouched."""
import argparse
import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent / 'results/point-access-performance'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--experiment', choices=['dynamic-fenwick', 'persistent-segment'], default='dynamic-fenwick')
    args = parser.parse_args()
    rows, preparations, diagnostics, starts, finishes = [], [], [], [], []
    profile_count = warmups = 0
    for mode in ['release', 'assert']:
        directory = ROOT / f'{args.experiment}-{mode}'
        preparations.append(json.loads((directory/'prepared.json').read_text()))
        result = json.loads((directory/'results.json').read_text())
        rows += [dict(row, mode=mode) for row in result['summary']]
        starts.append(result['started_at']); finishes.append(result['finished_at'])
        raw = json.loads((directory/'raw.json').read_text())
        profile_count += len(raw); warmups += sum(row['warmup'] for row in raw)
        diagnostics += [dict(row, mode=mode) for row in json.loads((directory/'diagnostics.json').read_text())['rows']]
    def key(row): return tuple(row[k] for k in ['mode', 'shape', 'scalar', 'compiler'])
    paired = []
    for old in rows:
        if old['implementation'] != 'baseline': continue
        new = next(row for row in rows if key(row)==key(old) and row['implementation']=='candidate')
        paired.append((old,new))
    def change(old,new,metric): return 100*(new['metrics'][metric]['median']/old['metrics'][metric]['median']-1)
    def fmt(row,metric):
        m=row['metrics'][metric];return f"{m['median']/1e6:.6f} [{m['min']/1e6:.6f},{m['max']/1e6:.6f}]"
    table=[]
    for old,new in paired:
        for metric in old['metrics']:
            a,b=old['metrics'][metric],new['metrics'][metric]
            table.append(dict(mode=old['mode'],shape=old['shape'],scalar=old['scalar'],compiler=old['compiler'],metric=metric,
                baseline_median_ns=a['median'],baseline_min_ns=a['min'],baseline_max_ns=a['max'],
                candidate_median_ns=b['median'],candidate_min_ns=b['min'],candidate_max_ns=b['max'],delta_percent=change(old,new,metric)))
    with (ROOT/f'{args.experiment}-stage-comparisons.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(table[0]),lineterminator="\n");writer.writeheader();writer.writerows(table)
    header_hash=preparations[0]['production_header_sha256']
    assert all(p['production_header_sha256']==header_hash for p in preparations)
    lines=[f'# {args.experiment}: final production header の測定',
        '',f'基準は `ca8a4df35269a6980cf0b9612878e420744fc860`。候補はproduction targetから直接保存したSHA256 `{header_hash}` のheader。生成文字列から再構成した候補ではない。他の4headerは基準と同一。実行直前にもlive production headerの同一性を確認する。',
        '',f'{profile_count} profiles（測定{profile_count-warmups}、warmup{warmups}）、別の{len(diagnostics)} allocation/arithmetic diagnosticsを保存。GCC14/Clang19 × gnu++20/O2 × release/assert、有効な小ケースgate後、他workerのcompile/testが停止した窓で交互に5回計測。開始 `{min(starts)}`、終了 `{max(finishes)}`。共有ホスト・CPU非固定・周波数変動の制約は残る。',
        '', '各query stageには同じchecksum/hash処理を含む。入力生成、起動、出力、破棄は除外。getter単独latencyやI/O/end-to-end速度の測定ではない。全標本と異常時保存方針、seed、phase範囲、再実行方法は [plan.md](plan.md) を参照。過去surveyの結果と統合しない。',
        '', '## get / mixed の結果']
    for metric in ['get_ns','mixed_ns']:
        changes=[change(a,b,metric) for a,b in paired]
        separated=sum(b['metrics'][metric]['max']<a['metrics'][metric]['min'] for a,b in paired)
        lines += ['',f'{metric}: {sum(x<0 for x in changes)}/{len(changes)}条件で中央値短縮。変化率は{min(changes):+.1f}%〜{max(changes):+.1f}%。{separated}/{len(changes)}条件で候補最大値が基準最小値を下回る。以下はmsの中央値[min,max]、変化は候補/基準−1。',
                  '', '|mode|compiler|shape / scalar|baseline|candidate|変化|','|---|---|---|---:|---:|---:|']
        for old,new in paired:
            lines.append(f"|{old['mode']}|{old['compiler']}|{old['shape']} / {old['scalar']}|{fmt(old,metric)}|{fmt(new,metric)}|{change(old,new,metric):+.1f}%|")
    lines += ['', '## 変更していないstageとmemory', '',
        '以下はbuild/init/add/rangeのうち中央値が5%以上遅くなった行を機械的に列挙したもの。微小なconstructor時間の割合も含むため絶対値を見る。これらの負値をget最適化の効果として数えず、正値も除外しない。全stageの数値は同名stage-comparisons.csvに保持する。',
        '', '|mode / compiler|shape / scalar|stage|baseline ms[min,max]|candidate ms[min,max]|変化|', '|---|---|---|---:|---:|---:|']
    for old,new in paired:
        for metric in old['metrics']:
            if metric in ['get_ns','mixed_ns'] or change(old,new,metric)<5: continue
            lines.append(f"|{old['mode']} / {old['compiler']}|{old['shape']} / {old['scalar']}|{metric}|{fmt(old,metric)}|{fmt(new,metric)}|{change(old,new,metric):+.1f}%|")
    zero=True;equal=True
    for old in diagnostics:
        if old['implementation']!='baseline':continue
        new=next(r for r in diagnostics if key(r)==key(old) and r['implementation']=='candidate')
        zero &= old['get_allocations']==new['get_allocations']==0
        equal &= all(old[k]==new[k] for k in old if k.endswith('_allocations') or k.endswith('_allocated_bytes'))
    lines += ['',f'別診断: get中allocationが双方0 = {zero}。対応stageのallocation呼出数・要求byte数が双方一致 = {equal}。要求byte数は累積でpeak-live容量ではない。RSSは入力・allocator保持・runtime・全stageを含むprocess peakで、get固有のmemory削減とは扱わない。算術/monoid callback数・導出したvalue_at回数は各diagnostics.jsonを参照。']
    if args.experiment=='dynamic-fenwick':
        lines += ['', 'mixedは40,000add/160,000get。各add直後に同座標のgetを行い、残りは既知座標と新規座標を混ぜる。新しい座標へのmap挿入/拡張を含むので、純粋なgetの改善率とは分けて解釈する。全値・混合checksumの愚直oracleはsmall profileで検証した。', '', '局所ブロックを既存の+=で蓄積し最後に1回binary−を使うため、Tの代入・−=を追加要求しない。数学的な可換加法群の結果を保つが、浮動小数点の結合順やcallback回数の同一性は保証しない。最終採否・commitは親担当のproduction validationと合わせて判断する。']
    (ROOT/f'{args.experiment}-final-code-report.md').write_text('\n'.join(lines)+'\n')
    print(f'Wrote {args.experiment} derived report/CSV; raw evidence unchanged.')


if __name__=='__main__':main()
