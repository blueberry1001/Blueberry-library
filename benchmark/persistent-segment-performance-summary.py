#!/usr/bin/env python3
"""Generate only the new Persistent report using the frozen shared summarizer."""
import csv
import json
from pathlib import Path
import runpy
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE / 'results/point-access-performance'


def main():
    shared = HERE / 'point-access-performance-summary.py'
    previous = sys.argv
    try:
        sys.argv = [str(shared), '--experiment', 'persistent-segment']
        runpy.run_path(str(shared), run_name='__main__')
    finally:
        sys.argv = previous
    path = ROOT / 'persistent-segment-final-code-report.md'
    text = path.read_text().replace('[plan.md](plan.md)', '[persistent-segment-plan.md](persistent-segment-plan.md)')
    text = text.replace('build/init/add/rangeのうち', 'build/update/rangeのうち')
    text += '''

## Persistent batch固有の条件

準備時のcheckout HEADは先行Dynamic batchの `5182af3cdb4f05a24ae8978b0599d67c38f1eb70`。この未commitのPersistent production headerを別SHAで固定して直接保存した。他4headerは双方ca8基準のままなので、先行Dynamic最適化は今回の比較に混在しない。共通C++/runner/summary generatorも先行commitから変更していない。

N100,000、version生成50,000（set/apply）、point get200,000、range25,000、mixed50,000（10,000set/40,000get）。dense-sequential / dense-branch / implicit-identity sparse-branch、uint64加算 / 非可換affineを分離。小ケースではmixedを含めた全versionを独立vectorで再構築し、全point/all_prodとrangeを検証する。O(log N)と値返却・任意の過去version・index/version assertを維持する。identityとのcallback回数そのものはAPI保証ではない。

releaseのuint64診断値（200,000get）。assert・affineの個別値は各diagnostics.jsonに保存する。

|history|baseline get op calls|candidate get op calls|baseline get allocations|candidate get allocations|
|---|---:|---:|---:|---:|
'''
    rows = json.loads((ROOT/'persistent-segment-release/diagnostics.json').read_text())['rows']
    for old in rows:
        if old['scalar'] != 'u64' or old['implementation'] != 'baseline': continue
        new = next(r for r in rows if (r['shape'], r['scalar'], r['implementation']) == (old['shape'], old['scalar'], 'candidate'))
        text += f"|{old['shape']}|{old['get_combines']}|{new['get_combines']}|{old['get_allocations']}|{new['get_allocations']}|\n"
    text += '\n最終採否・commitは独立production validationと合わせて親担当が判断する。今回の計測は既存公式全体gateの別blockerを解消したという意味ではない。\n'
    text += """

## 小さい差と主測定の負のcontrol

主測定のClang/assert/sparse-branch/affine mixedは中央値25.386919→25.293043ms（−0.3698%）、基準範囲[24.033506,27.018043]ms、候補[20.909264,29.681568]msで重なる。これは観測上のほぼ同率で、確立した速度優位ではない。24/24という中央値の符号だけからmixed全般の高速化を保証しない。

主測定で標本範囲が分離した遅いrange controlは、Clang/assert/dense-sequential/u64 +10.2051%、GCC/release/sparse-branch/u64 +7.2715%、Clang/release/dense-sequential/u64 +5.5427%。これらも全て保持する。最大のbuild増加+36.676%とupdate増加+14.817%（Clang/assert/dense-sequential/u64）は元の標本範囲が重なっていた。原因を単にnoiseと断定せず、変更していないstageの改善もget最適化の効果とは数えない。
"""
    focused_path = ROOT/'persistent-segment-recheck/results.json'
    if focused_path.exists():
        focused = json.loads(focused_path.read_text())
        text += """

## 別保存の同一binary再確認

上記controlとget outlierを理由に、計測前に選んだ4cellを同一binary・入力・flagsでwarmup1回+9回交互実行した。80 profiles（測定72/warmup8）は主測定288とは別保存し、標本をpool・置換・除外していない。理由、source/binary/gate/hash、環境は [再確認prepared.json](persistent-segment-recheck/prepared.json)、全値は [再確認CSV](persistent-segment-recheck/stage-comparisons.csv) を参照。主測定と合わせて368 profiles（測定312/warmup56）だが、異なる反復数の結果を一つの中央値にまとめない。

3つの元の分離したrange増加は同程度には再現せず、順に−1.1180%、−8.1999%、+0.1610%となり標本範囲が重なった。一方、Clang/assert/dense-branch rangeは元+7.8194%から再確認+5.7993%で、範囲は重なるものの小さい増加が残る。GCC/release/sparse-branch constructorは+10.56%が残るが、絶対値は1.278→1.413µs。これらを隠さず、全操作に一律の速度改善があるとは主張しない。getは再確認4/4で範囲が分離して短縮、mixedは4/4で中央値が短縮したがsparseの範囲は重なる。追加の再確認や最適化候補は行わない。

以下は全stageを別々に記録した中央値[min,max]のms表。変化は候補/基準−1。

|cell / stage|主baseline|主candidate|主変化|再baseline|再candidate|再変化|
|---|---:|---:|---:|---:|---:|---:|
"""
        originals = {}
        for mode in ['release', 'assert']:
            originals[mode] = json.loads((ROOT/f'persistent-segment-{mode}/results.json').read_text())['summary']
        csv_rows = []
        def fmt(metric):
            return f"{metric['median']/1e6:.6f} [{metric['min']/1e6:.6f},{metric['max']/1e6:.6f}]"
        for old in focused['summary']:
            if old['implementation'] != 'baseline': continue
            identity = (old['mode'], old['shape'], old['scalar'], old['compiler'])
            new = next(r for r in focused['summary'] if (r['mode'],r['shape'],r['scalar'],r['compiler'])==identity and r['implementation']=='candidate')
            a = next(r for r in originals[old['mode']] if (r['shape'],r['scalar'],r['compiler'],r['implementation'])==(old['shape'],old['scalar'],old['compiler'],'baseline'))
            b = next(r for r in originals[old['mode']] if (r['shape'],r['scalar'],r['compiler'],r['implementation'])==(old['shape'],old['scalar'],old['compiler'],'candidate'))
            for metric in old['metrics']:
                before,after = a['metrics'][metric],b['metrics'][metric]
                re_before,re_after = old['metrics'][metric],new['metrics'][metric]
                delta = 100*(after['median']/before['median']-1)
                re_delta = 100*(re_after['median']/re_before['median']-1)
                label = f"{old['mode']} / {old['compiler']} / {old['shape']} / {metric}"
                text += f"|{label}|{fmt(before)}|{fmt(after)}|{delta:+.2f}%|{fmt(re_before)}|{fmt(re_after)}|{re_delta:+.2f}%|\n"
                row = dict(mode=old['mode'],compiler=old['compiler'],shape=old['shape'],scalar=old['scalar'],metric=metric,
                           original_delta_percent=delta,recheck_delta_percent=re_delta)
                for prefix,values in [('original_baseline',before),('original_candidate',after),('recheck_baseline',re_before),('recheck_candidate',re_after)]:
                    for statistic,value in values.items(): row[prefix+'_'+statistic+'_ns']=value
                csv_rows.append(row)
        with (ROOT/'persistent-segment-recheck/stage-comparisons.csv').open('w',newline='') as f:
            writer=csv.DictWriter(f,fieldnames=list(csv_rows[0]),lineterminator='\n')
            writer.writeheader();writer.writerows(csv_rows)
    path.write_text(text)
    print('Added Persistent-only plan/context/operation table; old batch artifacts unchanged.')


if __name__ == '__main__': main()
