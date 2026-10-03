#!/usr/bin/env python3
"""Regenerate the bounded survey's derived tables; never changes raw measurements."""
import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent / 'results/representative-performance'
rows = []
for directory in sorted(ROOT.glob('*')):
    if directory.name == 'focused-recheck' or not (directory / 'results.json').exists(): continue
    data = json.loads((directory / 'results.json').read_text())
    for row in data['summary']:
        row = dict(row, experiment=directory.name, mode=data['mode'])
        rows.append(row)
focused = json.loads((ROOT / 'focused-recheck/results.json').read_text())['summary']


def key(row): return tuple(row[k] for k in ['family', 'shape', 'scalar', 'mode', 'compiler'])
def pairs(data):
    for a in data:
        if a['implementation'] not in ['baseline', 'acl']: continue
        subject = 'candidate' if a['implementation'] == 'baseline' else 'blueberry'
        b = next(r for r in data if key(r) == key(a) and r['implementation'] == subject)
        yield a, b

def change(a, b, metric): return 100 * (b['metrics'][metric]['median'] / a['metrics'][metric]['median'] - 1)
def value(row, metric):
    m = row['metrics'][metric]
    return f"{m['median']/1e6:.6f} [{m['min']/1e6:.6f}, {m['max']/1e6:.6f}]"

def write_csv(name, data):
    output = []
    for a, b in pairs(data):
        for metric in a['metrics']:
            old, new = a['metrics'][metric], b['metrics'][metric]
            # A zero initialization time on SegmentTree means not applicable.
            delta = 100 * (new['median']/old['median'] - 1) if old['median'] else None
            output.append(dict(experiment=a.get('experiment', 'focused-recheck'), family=a['family'],
                shape=a['shape'], scalar=a['scalar'], mode=a['mode'], compiler=a['compiler'], metric=metric,
                reference=a['implementation'], subject=b['implementation'],
                reference_median_ns=old['median'], reference_min_ns=old['min'], reference_max_ns=old['max'],
                subject_median_ns=new['median'], subject_min_ns=new['min'], subject_max_ns=new['max'],
                delta_percent=delta))
    with (ROOT/name).open('w', newline='') as f:
        writer=csv.DictWriter(f, fieldnames=list(output[0]), lineterminator="\n"); writer.writeheader(); writer.writerows(output)

write_csv('stage-comparisons.csv', rows)
write_csv('focused-stage-comparisons.csv', focused)
text = ['''# 代表的データ構造の測定 shortlist（未採用）

結論: `DynamicFenwickTree::get` の局所ブロック差分と `PersistentSegmentTree::get` の片側下降を、次回の実装候補として残す。今回変更したものはベンチマークと証拠だけで、production header は基準 `ca8a4df35269a6980cf0b9612878e420744fc860` のまま。dense DSU/Fenwick/SegmentTree の書き換えは提案しない。

- DynamicFenwick の get: 全24条件で中央値が短縮。release 59.4–91.6%、assert 有効 56.0–91.6%。全24条件で候補の最大値が基準の最小値を下回った。
- PersistentSegmentTree の get: 全24条件で中央値が短縮。release 19.5–56.6%、assert 有効 25.0–57.7%。23/24条件で候補最大値が基準最小値を下回った。
- Persistent の mixed（20% set / 80% get）: 全24条件で中央値が短縮。release 6.6–49.8%、assert 有効 7.6–48.1%。17/24条件で標本範囲が分離した。全 workload の一律高速化を意味しない。

## 実行条件と証拠

主測定は5回+warmup1回を実装/コンパイラ順を交互にして実行し、864 profiles（720測定+144warmup）を保存。同じ frozen binary による局所再確認は9回+warmup1回、140 profiles（126測定+14warmup）を別保存した。合計1004 profiles（846測定+158warmup）。別の48 allocation/arithmetic diagnostics は時間を採否に使っていない。全実装の入力/結果 checksum は一致した。主測定区間は2026-10-03 07:19:39–07:21:33 UTC、再確認は07:24:10–07:24:25 UTC。

GCC14.2 / Clang19.1.7、`-std=gnu++20 -O2 -Wall -Wextra`、release のみ `-DNDEBUG`。`-march=native` は不使用。Xeon Platinum8573C の共有ホストで、cgroup `cpu.max=400000 100000`（4 CPU相当）、cpuset `0-4`、memory.max16GiB。CPU固定はしていない。他workerのcompile/testを止めた窓で直列測定したが、他tenant・周波数変動まで管理したものではない。詳細は [host-limits.json](host-limits.json)、各 experiment の prepared/results/quiet-processes ファイルを参照。

28 binary が警告なしでcompile成功。最終runnerによる192の小ケースprofileで愚直oracle/checksum一致（失敗証拠保存を強化する前の192も別名で保持、計384実行）。[contract-checks](contract-checks/) は GCC/Clang の厳密compileと Clang UBSan を基準/候補双方で実施し、12組が成功。nonassignable additive value、signed座標境界、空/単一/非2冪の木、疎なidentity root、分岐version、非可換monoid等を確認した。これはproduction全体の公式judge gateを置き換えない。

準備時runner、更新前runner、二段のrunner-amendment、各prepared.jsonを保存している。候補生成/C++/header/binaryはその間変更せず、自己hash確認・小ケースgate・異常出力/不一致rowの保存を測定前に強化した。過去のarchive/report、production header、認証情報は変更していない。この性能調査ではネットワーク再試行やuploadは行っていない。

[plan.md](plan.md) に再実行コマンド・seed・stage定義・適用範囲を記載。全stageの正確なns値は [stage-comparisons.csv](stage-comparisons.csv)、再確認は [focused-stage-comparisons.csv](focused-stage-comparisons.csv)。各raw.jsonにはwarmupも含む全標本を保持。以下はms表記の中央値 [最小, 最大]、変化率は候補/基準−1（denseだけBlueberry/ACL−1）。入力生成・起動・出力・破棄は計時外、queryのchecksum/hash処理は計時内で双方同一。I/O・end-to-end改善率ではない。

## point get の全条件

DynamicFenwick はdomain2^40−1、初期add30,000、追加add50,000、get200,000、range25,000。broad/hotspot/2冪境界、uint64とmod998を分離。getだけを変更し、mixed操作列は測っていない。除去ブロックを `+=` で蓄積し最後に1回 `-` を行うため、Tの代入や `-=` を追加要求しない。除去部分は元のpref(p)の先頭部分列なので、元のsigned中間値が有効なら新しいoverflowを導入しない。浮動小数点の結合順・丸めは変わり得る。算術callbackの回数そのものはAPI保証ではない。

Persistent はN100,000、version生成50,000、get200,000、range25,000、mixed50,000。初期配列あり/implicit identity、直列/分岐version、uint64加算/非可換affineを分離。index/version assert・値返却・O(log N)を維持し、prod/set/applyには手を加えない。
''']
for family in ['dynamic-fenwick', 'persistent-segment']:
    text += [f'### {family}\n', '|mode|compiler|shape / scalar|baseline ms [min,max]|candidate ms [min,max]|変化|', '|---|---|---|---:|---:|---:|']
    for a,b in pairs(rows):
        if a['family'] != family: continue
        text.append(f"|{a['mode']}|{a['compiler']}|{a['shape']} / {a['scalar']}|{value(a,'get_ns')}|{value(b,'get_ns')}|{change(a,b,'get_ns'):+.1f}%|")
    text.append('')
text += ['''## 変更していないstage・dense controlの注意点

初回の Persistent GCC release / dense-sequential update は uint64で+14.5%、affineで+16.4%遅かった。Dynamic Clang assert / boundary uint64 range も+14.9%。これらを除外せず、同じbinaryで再確認したところ+2.8%、+4.6%、+4.0%に縮小し、各標本範囲は重なった。差の原因を断定せず、変更していないstageの短縮を候補の効果として数えない。初回と再確認は統合せず保持する。

最も大きなdense outlierはGCC release / random SegmentTree get。初回Blueberry 0.687180ms [0.205381,0.736424]、ACL0.207561ms [0.181799,0.209061]、+231.1%（差0.479619ms / 200,000get）。getはBlueberryのbuild+update+range+get各中央値の合計55.723745ms中約1.23%を占めた。これは各中央値の和による参考割合で、独立したend-to-end計時ではない。初回の範囲は端でわずかに重なり、他コンパイラ/モードで同規模の差は出なかった。9回再確認のGCC releaseは+2.0%、他3条件は−1.3%、−0.3%、−6.2%で全範囲が重なった。3.3倍という差は再現せず、原因未確定のままdense実装を書き換えない。

再確認表（denseはACL→Blueberry、それ以外はbaseline→candidate）:

|対象stage|mode / compiler|scalar|基準 ms [min,max]|比較先 ms [min,max]|変化|
|---|---|---|---:|---:|---:|''']
for a,b in pairs(focused):
    metric = 'get_ns' if a['family']=='segment-tree' else 'update_ns' if a['family']=='persistent-segment' else 'range_ns'
    text.append(f"|{a['family']} {metric}|{a['mode']} / {a['compiler']}|{a['scalar']}|{value(a,metric)}|{value(b,metric)}|{change(a,b,metric):+.1f}%|")
text += ['''
Dense controlはuint64の共通操作部分に限定。DSUはmerge戻り値/代表元の違いを比較せずsameとcomponent sizeを消費。Fenwickは両方zero構築+同じN回addで、BlueberryのO(N)配列constructor対ACL反復addの不公平な構築比較をしていない。point get対ACL sum(p,p+1)は値が等しい操作比較であり、ACLに同名APIがあるという意味ではない。SegmentTreeのget参照返却の契約差は即座の値消費で限定し（all_prodはdense測定対象外）、generic monoid全般の性能へ外挿しない。既存のdense get短絡・bit_floor・no_unique_addressは再実装していない。

## 別計測の操作・allocation diagnostics

Dynamic getで呼ぶvalue_atの回数は各query座標からloop回数を正確に集計したもの（実際のbucket probeや衝突回数ではない）。`+=`と`-`、Persistentのmonoid opは別instrumented buildで計数した。以下のu64表の削減はmod998/affineでも対応する同一入力条件で確認している。

|family / shape|point reads|before|after|数える対象|
|---|---:|---:|---:|---|''']
for family in ['dynamic-fenwick','persistent-segment']:
    diagnostic=json.loads((ROOT/f'{family}-release/diagnostics.json').read_text())['rows']
    for a in diagnostic:
        if a['implementation']!='baseline' or a['scalar']!='u64':continue
        b=next(r for r in diagnostic if (r['shape'],r['scalar'],r['implementation'])==(a['shape'],a['scalar'],'candidate'))
        if family=='dynamic-fenwick': old,new,label=a['baseline_get_lookup_calls'],b['candidate_get_lookup_calls'],'value_at calls'
        else: old,new,label=a['get_combines'],b['get_combines'],'monoid op calls'
        text.append(f"|{family} / {a['shape']}|200000|{old}|{new}|{label}|")
text += ['''
全48 diagnosticsでget中のnew/new[]は基準/候補とも0。build/init/update/mixed等のallocation呼出数・要求byte数も対応する基準/候補で一致した。get時allocation削減による効果とは主張しない。Persistentはget中の恒等元とのop呼出しが0になり、Dynamicはhash lookupと加算回数を減らしている。stageごとの詳細は各diagnostics.jsonに保存。

RSSはLinux process peakで、入力・allocator保持・runtime・全stageを含む。要求byte数は累積でpeak-live容量ではなく、RSS差をget固有のmemory削減として扱わない。dense allocation診断は未測定で0ではない。I/O処理は今回の対象外。

## 採否と次の範囲

二候補とも「測定済みの有力候補」として残し、今回のsurveyでは採用しない。実装に進める場合は別の変更単位でproduction差分・公開docs・必要なunit/random/official検証をレビューする。元のofficial全体gateの既知blockerをこの局所測定で解消したとは記載しない。dense controlから新たな最適化候補を追加せず、このbounded batchを終了する。
''']
(ROOT/'measured-shortlist.md').write_text('\n'.join(text))
print('Wrote measured-shortlist.md and two derived stage CSVs; raw files unchanged.')
