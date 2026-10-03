#!/usr/bin/env python3
"""Generate the Japanese development report from both immutable experiments."""
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[1]
BASE = ROOT / "benchmark/results/furthest-pair-resume"


def read(path):
    return json.loads((BASE / path).read_text())


def main():
    initial = read("initial/prepared.json")
    final = read("final/prepared.json")
    first_meta = read("initial/measurement/metadata.json")
    last_meta = read("final/measurement/metadata.json")
    comparisons = read("final/measurement/comparisons.json")
    allocations = read("final/diagnostics/allocation.json")
    lines = ["# furthest_pair: 実装比較と最終コードの測定", "",
        "座標を持つ凸包と元添字の復元を使う設計を維持し、要素数2以下・全点同一の場合の早期 return を追加した。添字を sort する案と座標/添字を一緒に sort する案も比較したが、通常入力での遅い例が多く採用しなかった。", "",
        "最終の実 header 比較では全点同一を91.37–96.85%短縮（8/8セルでsample range分離）、tinyの中央値を11.82–27.87%短縮（分離は2/4セル）した。末尾のみ異なる入力の変化は−0.04〜+8.68%で全てrangeが重なる。通常入力を含む33/76セルでは中央値が増加し、全てrangeが重なった。したがって、一般入力での一律な速度向上は主張しない。", "",
        "2026-10-03時点の `wip/furthest-pair` のローカル WIP 検証で、`improve/point-access-performance`（2958f619）とは独立して進めた。merge・公開は行っていない。Fastest取得403と既存AOJ公式gate403は未解決で、全体の公開gateが完了したという報告ではない。今回の数値はローカルの同一条件比較に限る。", "",
        "早期 return の利点と、全点同一か調べた末に異なる点が見つかる場合の追加走査を分けて評価する。全体の最悪計算量は O(N log(N+1)) 時間・O(N) 補助記憶のまま。これは今回の入力・型・コンパイラでの比較であり、すべての入力で速い、または Library Checker 最速という主張ではない。", "",
        "## 契約・出典・保存範囲", "",
        "測定対象は変更しない `vector<pair<long long,long long>>` への参照を受け取り、元配列の正規化した異なる添字 i<j を返す。N<2 は{-1,-1}、同距離の候補は任意。座標の絶対値は2^62-1以下、N<=INT_MAX、外積・距離二乗は符号付き128bitで計算する。比較は添字そのものではなく距離二乗と返値条件で行う。公開 API の小さい符号付き整数型への対応は別の正当性検証対象であり、今回の性能測定は long long に限る。", "",
        "[調査ノート](../../benchmark/results/furthest-pair-resume/research-notes.md)に cached primary source の違いを記録した。KACTL は構築済み凸包から座標を返す O(H) 手続き、maspypy/公式解は添字凸包と重複した凸包配列を使い、公式解は N>=2・座標±10^9 の問題契約を前提とする。広い座標・空入力・元添字復元まで揃えずに、それらを同じ end-to-end API として直接測定していない。他人のソースは候補へコピーしていない。", "",
        "旧 WIP の `benchmark/furthest-pair.cpp/.py` と `benchmark/results/furthest-pair/` は変更していない。旧4バイナリの93ケース検証は準備記録で、旧時点で性能測定や公式 AC が完了していたことを意味しない。Fastest API の既存403ログを保存し、この性能調査では再試行していない。", "",
        f"初期 public header SHA256 は `{initial['production_header_sha256']}`、採用後は `{final['production_header_sha256']}`。最終比較は旧 header の保存コピーと採用後の実ファイルを直接含める別バイナリを作り、共通 harness・convex_hull・FastIO を同じにした。初期 wrapper を最終実装そのものとして扱っていない。", "",
        "## 測定方法", "",
        "GCC14/Clang19、gnu++20、-O2 -Wall -Wextra。release は -DNDEBUG、assert は有効。-march=native は使わない。各セルは warmup1回と測定5回、候補順を交互にし、別プロセスで実行した。中央値・最小・最大と全 warmup を保存し、異なる compiler/mode の値を混ぜない。sample range の非重複は記述的な指標で、統計的な信頼区間ではない。", "",
        "N=2,048/200,000で random、重複、全点同一、共線、shuffleした放物線、sort済み、逆順、座標上限に近いrandom、末尾のみ異なる同値列を使った。N=200,000の H はそれぞれ30,4,1,2,200000,30,30,30,2。tiny は空・1点・2点・正方形・同値3点・共線3点の計100,000呼出し、総点数216,665。seedと具体的な生成規則は [計画](../../benchmark/results/furthest-pair-resume/plan.md) と harness にある。", "",
        "入力生成・答え領域の確保・objective検証・H計算は full timer の外に置く。full は入力コピー、構築、calipers、添字復元、内部領域解放を含む。構築/calipers/復元の stage は独立した分解実装の別呼出しで、public関数内の計時ではない。stage を足して full と一致すると解釈しない。tiny/fast-cases の stage、添字/record候補の復元は null のまま保存する。", "",
        f"初期は {first_meta['kernel_profiles']} kernel + {first_meta['io_profiles']} I/O = {first_meta['kernel_profiles']+first_meta['io_profiles']} profiles（warmup {first_meta['warmup_profiles']}）。最終は public関数の paired {last_meta['profiles']} profiles（warmup {last_meta['warmup_profiles']}）。初期と最終の生データを混ぜて中央値を作らない。", "",
        "CPUはXeon Platinum8573C、cgroup quota400000/100000=4CPU、cpuset0-4、メモリ上限16GiB。作業者間で compile/test を止めた quiet window を使用し、引数を含まないプロセス一覧と環境を保存した。共有ホストでCPU固定はしておらず、外部負荷や実行ばらつきが消えるとは保証できない。", "",
        "## 初期の設計比較", "",
        "76セル（19入力×4compiler/mode）の full中央値で、添字案は17セルのみ速く、32セルは全サンプルが public より遅い。record案は7セルのみ速く、27セルは全サンプルが遅い。最大中央値増加は添字+55.7%、record+52.3%。添字案は random の要求メモリを6,400,016→1,600,004bytesへ減らすが、間接参照を伴う sort/calipers の時間上の代償がある。record案は9,600,024bytesへ増える。", "",
        "fast-cases wrapper は全点同一で90.3–97.1%短縮し、8/8セルのsample rangeが分離した。tinyは17.2–29.9%短縮、3/4セルで分離。末尾だけ異なる入力は−0.65〜+13.47%で全セルが重なる。通常入力の最も大きい中央値増加は Clang release/duplicates/N2048 の+26.21%で、rangeは重なる。遅い中央値を削除せず、重なりをもってコストが存在しないとはしない。実際に全点同一判定は、末尾で不一致となる場合に O(N) の追加走査になる。", "",
        "初期の全708 stage比較は [CSV](../../benchmark/results/furthest-pair-resume/initial/stage-comparisons.csv)、全セルの表は [比較レポート](../../benchmark/results/furthest-pair-resume/initial/comparison-report.md)。元ログに対応しない成功値や0の埋め合わせはない。", "",
        "## 採用後の実 header と保存 baseline の比較", "",
        "以下は最終 paired測定のみ。負の変化率は短縮、正は増加。", "",
        "| Compiler | Mode | 全点同一の変化率(N2048 / N200000) | tinyの変化率 | 末尾不一致(N2048 / N200000) | 全19セルでrange分離:速い/遅い |",
        "|---|---|---:|---:|---:|---:|"]
    for compiler in ["g++", "clang++"]:
        for mode in ["release", "assert"]:
            selected = [r for r in comparisons if r['compiler'] == compiler and r['mode'] == mode]
            def values(workload):
                return " / ".join(f"{r['delta_percent']:+.2f}%" for r in sorted(selected, key=lambda r:r['size']) if r['workload'] == workload)
            lines.append(f"| {compiler} | {mode} | {values('all-same')} | {values('tiny')} | {values('late-different')} | {sum(r['separated_faster'] for r in selected)}/{sum(r['separated_slower'] for r in selected)} |")
    lines += ["", "全セルの中央値[min,max]と変化率は [最終CSV](../../benchmark/results/furthest-pair-resume/final/measurement/comparisons.csv) に保存した。最終 paired測定で遅くなった中央値を以下にすべて残す。sample range が重なる行も省かない。", "",
        "| Compiler | Mode | 入力 | N | Before ms [min,max] | After ms [min,max] | 変化率 | 遅いrangeが分離 |",
        "|---|---|---|---:|---:|---:|---:|---|"]
    for row in comparisons:
        if row['delta_percent'] > 0:
            lines.append(f"| {row['compiler']} | {row['mode']} | {row['workload']} | {row['size']} | {row['baseline_median_ns']/1e6:.6f} [{row['baseline_min_ns']/1e6:.6f},{row['baseline_max_ns']/1e6:.6f}] | {row['final_median_ns']/1e6:.6f} [{row['final_min_ns']/1e6:.6f},{row['final_max_ns']/1e6:.6f}] | {row['delta_percent']:+.2f}% | {'yes' if row['separated_slower'] else 'no'} |")
    lines += ["", "## I/O、allocation、RSS", "",
        "I/Oは初期比較でrandom/tinyを全候補・両compiler/mode・FastIO/iostreamについて測定した。parse/solve/formatとプロセス込みelapsedを分ける。外側elapsedはshell/process起動、pipe、format後のobjective確認も含む。すべての入力を保持してphase分離するため、公式のstreaming verifierと同じメモリ消費とは主張しない。最終header確認ではI/O方式を変更していないため、同じI/O比較を繰り返していない。", "",
        "初期56 allocation profiles +48独立RSS profiles、最終12 allocation profilesを時間測定とは分離した。allocation要求回数/bytesはfull呼出し中のみで、入力・答え領域・stage・metadataを除く。bytesは累積要求量でpeak-liveではない。RSSは小さいshell下のLinux getrusageで、入力・runtime・allocatorが保持した領域を含む全プロセスhigh water。各セル1回の診断値から精密なメモリ差を断定しない。", "",
        "| Mode | 入力 | Before calls / bytes | After calls / bytes |",
        "|---|---|---:|---:|"]
    for mode in ["release", "assert"]:
        for workload in ["all-same", "tiny", "late-different"]:
            before = next(r for r in allocations if r['mode'] == mode and r['workload'] == workload and r['variant'] == 'baseline')
            after = next(r for r in allocations if r['mode'] == mode and r['workload'] == workload and r['variant'] == 'final')
            lines.append(f"| {mode} | {workload} | {before['allocation_calls']} / {before['allocation_bytes']} | {after['allocation_calls']} / {after['allocation_bytes']} |")
    lines += ["", "## 再現・検証との関係", "",
        "初期6バイナリは各1,258ケースのall-pairs比較、計240小さいprofileの比較を通過し、12のinvalid singletonは実際にSIGABRTした。最終12バイナリも同じ1,258ケースを各々検証し、120 paired小profileを照合、assert版6バイナリのinvalid singletonをSIGABRTとして検証した。これはbenchmark自身のgateであり、別担当の広い型・境界・公式judge・sanitizer・統合テストに代わるものではない。", "",
        "別担当の最終 focused検証は同じb2e6 headerに対して328 expected-outcome stepsが成功（未対応型のcompile拒否24件・SIGABRT48件を含む）。公式furthest_pair46ケース×3=138AC、convex_hull25ケース×3=75AC、計213AC、Clang ASan/UBSan/leakを含む4runが成功した。保存summaryは [validation/final/summary.json](../../benchmark/results/furthest-pair-resume/validation/final/summary.json)。これはこの2targetのoffline公式データ検証であり、既存AOJ403を含む全repository公式gateの成功ではない。", "",
        "独立したint256 oracle/UBSan検証も最終実headerで197,120ケースと624,344 support invariant checksを通過した。[証明ノート](../../benchmark/results/furthest-pair-resume/validation/review/proof.md) と [最終source review](../../benchmark/results/furthest-pair-resume/validation/review-final/source-review.json) に根拠を保存する。6つの大きな公式rawログはローカル保存を維持し、path/hashをmanifestで追跡する。rawログ自体の追跡追加やarchive作成はしていない。", "",
        "rootの `make check` は成功（97 unittest、107 library docsと111 executable examples、108 headers、145 verify drivers、52 random targets×20 seeds）。`make docs` は既存の `ValueError(\"do not publish unsupported or failed measurements\")` gateでexit2、Jekyllと`--without-metrics` site preflightは成功した。計測gateを迂回した公開や全公式verifyの再実行はしていない。", "",
        "初期runnerのnegative gateは任意のnonzeroを受け入れるが、保存された12結果は全て−6と確認済み。初期small gateでJSON/schema failureが起きた場合は失敗command/outputが保存され、それ以前の未保存のsmall行は残らない可能性がある。今回のsmall240行は全成功し保存済み。測定raw、warmup、失敗出力の保存と、snapshot/hash変更時の停止を設けた。既存result directoryへ上書きして再測定しない。", "",
        "```sh", "source /tmp/blueberry-setup/env.sh", "ulimit -c 0",
        "# 初期は新しい --experiment 名で作成（既存 initial は上書き不可）",
        "python3 benchmark/furthest-pair-resume.py prepare --experiment fresh-name",
        "# quiet windowを確保してから run / diagnostics",
        "python3 benchmark/furthest-pair-resume.py run --experiment fresh-name",
        "python3 benchmark/furthest-pair-resume.py diagnostics --experiment fresh-name",
        "# 最終 paired実験の実際のcommand/flags/hashは final/prepared.json を参照",
        "python3 benchmark/furthest-pair-resume-summary.py --experiment initial",
        "python3 benchmark/furthest-pair-report.py", "```", "",
        "最終runnerは `benchmark/furthest-pair-final.py`。準備済み `final/` とbinaryを置換せず、保存snapshotのhashを検査する。初期/finalのrunner本体、共通harness、header snapshot、compile/check出力、全raw、環境・プロセス一覧を新しいresults配下に保持した。第三者のraw sourceや実行binaryをtracked sourceへ取り込まない。", ""]
    path = ROOT / "docs/development/furthest-pair.md"
    path.write_text("\n".join(lines))
    print(path)


if __name__ == "__main__":
    main()
