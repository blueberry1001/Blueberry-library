# Library Checker / benchmark investigation

調査日: 2026-09-13 UTC

## 結論

現行ライブラリを使った提出で、明確にFastestとの差を再現できたのは **Static RMQ** である。
[Blueberry提出 401997](https://judge.yosupo.jp/submission/401997) の最大ケース時間は130 ms、
[Fastest 362370](https://judge.yosupo.jp/submission/362370) は20 msで、表示値では **6.50倍**。
最大メモリは45,019,136 B対12,595,200 Bで **3.57倍**だった。

ただし、この6.50倍をSparse Table本体だけの差と解釈してはいけない。公開ソースの特徴と
同一環境microbenchmarkから、少なくとも次が重なっている。

1. 現行実装は`vector<vector<T>>`と長さごとの`log_`を持つ通常のO(N log N) Sparse Table。
2. Fastestはbit scanを使う線形空間の`BlockedSparseTable`で、データ配置とキャッシュ効率が異なる。
3. 現行提出はiostream、Fastestは独自のbuffered C/POSIX I/Oを使用している。
4. Static RMQは最大50万個の入力・query・出力を含むため、提出時間にはI/Oも大きく入る。

Fastest側にSIMD intrinsicや`#pragma GCC target`は見つからなかった。そのため今回の主因候補は
AVX依存の特殊化より、RMQの表現とI/Oである。これはソースの特徴からの推定であり、因果関係を
確定するには同じ実行環境・同じ入出力を使った追加比較が必要である。

## Library Checkerデータ

生データとソースを含まない特徴量は
[`results/library-checker-staticrmq-2026-09-13`](results/library-checker-staticrmq-2026-09-13/)
に保存した。取得時点で両提出とも現行テストケースに対するACで、言語IDは`cpp`だった。

| 項目 | Blueberry | Fastest | 比率（Blueberry / Fastest） |
| --- | ---: | ---: | ---: |
| 最大ケース時間 | 130 ms | 20 ms | 6.50x |
| 最大メモリ | 45,019,136 B | 12,595,200 B | 3.57x |
| `max_random`群 | 120–130 ms | 17–19 ms | 約6.3–7.2x |
| `small_width_query`群 | 124–126 ms | 19–20 ms | 約6.2–6.6x |

ランダム幅と短区間の双方で差が出ており、特定ケースだけの外れ値ではない。

収集にはLibrary Checker frontendが利用する公開REST APIのGETだけを使った。
[OpenAPI定義](https://github.com/yosupo06/library-checker-judge/blob/master/restapi/openapi/openapi.yaml)と
[server実装](https://github.com/yosupo06/library-checker-judge/blob/master/restapi/handlers_submissions.go)では、
`order=+time`が最大ケース時間の昇順であることを確認できる。
[公式言語設定](https://github.com/yosupo06/library-checker-judge/blob/master/langs/langs.toml)の現行`cpp`は
GCC 15.2、C++23、`-O2 -march=native`である。ただし過去の提出が現在と同じcompiler imageで
再コンパイルされたとは限らないため、leaderboardだけから命令単位の比較は行わない。

## 同一環境microbenchmark

保存結果: [`results/local-staticrmq-2026-09-13`](results/local-staticrmq-2026-09-13/)

- CPU: AMD EPYC 9V74
- compiler: GCC 13.3.0
- affinity: CPU 0に固定
- 各対象: warmup 2回、計測11回、実行順を各周でローテーション
- N = 500,000、query = 500,000を30周、全実装のchecksum一致を検証
- `judge`: C++23、`-O2 -march=native -DEVAL -DONLINE_JUDGE`
- 入出力と乱数生成をquery時間から除外し、構築とqueryを分離

### `judge`（O2）結果

| workload / 実装 | 構築中央値 | query中央値 | 現行比 | 推定構造メモリ |
| --- | ---: | ---: | ---: | ---: |
| random / 現行Blueberry | 15.073 ms | 147.649 ms | 1.000x | 37,902,932 B |
| random / flat O(N log N) | 16.132 ms | 86.165 ms | 0.584x | 38,000,000 B |
| random / blocked O(N)参照 | 5.577 ms | 187.276 ms | 1.268x | 6,406,276 B |
| small / 現行Blueberry | 14.842 ms | 71.276 ms | 1.000x | 37,902,932 B |
| small / flat O(N log N) | 15.976 ms | 73.431 ms | 1.030x | 38,000,000 B |
| small / blocked O(N)参照 | 5.491 ms | 135.017 ms | 1.894x | 6,406,276 B |

### 読み取れること

- levelごとの領域を1配列にしたflat版はrandom queryで **約1.71倍**高速だった。計算量と
  保存要素数はほぼ同じなので、`vector<vector<T>>`の間接参照、`log_`参照、配置による
  キャッシュ差が原因候補になる。ただし短区間では差が消えたため、常に1.71倍ではない。
- 診断用blocked版は推定構造メモリを **約5.92分の1**、構築時間を **約2.70分の1**にした。
  一方でqueryは現行より遅い。この参照実装を本体への採用候補とはせず、Fastestの
  「線形空間でありながらqueryも速い」実装品質が重要だと分かる比較対象として残す。
- O3は現行random queryで155.942 msとなり、O2の147.649 msを改善しなかった。他の行にも
  ばらつきがあり、単純な`-O3`化は解決策とみなせない。
- microbenchmarkはI/Oを除外している。Fastestとの6.50倍差のうち何割がI/Oかは、この結果だけでは
  数値化できない。公式ケースのend-to-end計測とmicrobenchmarkを併記する必要がある。

## 現時点の優先順位

1. Data Structure担当では、通常Sparse Tableを維持する場合でもflat配置とlog計算方法を比較する。
2. Static RMQ専用の線形空間構造を追加するなら、genericな冪等演算用Sparse Tableとは別型も検討する。
3. official verifyのiostreamを差し替えた測定も行い、ライブラリ本体とI/Oの寄与を分離する。
4. 変更採用時は同じmanifest、CPU affinity、compiler、flags、seedで再計測する。

今回、`blueberry/`以下は変更していない。blocked/flat実装は原因切り分け専用である。

## 他ライブラリの扱い

公開ユーザー提出にはLCA、Fenwick Treeなどの過去提出もあるが、現在のBlueberryヘッダを使った
提出とは限らない。このため、それらを「現行ライブラリの倍率」として報告することは避けた。
`scripts/fetch_lc_submissions.py`へ複数の`--problem`を渡せば、今後同じ手順で候補を列挙できる。
確実に現行ヘッダをbundleした提出を作った後、ソースhash/特徴量を確認して比較対象へ追加する。

全verify targetを直列・3回ずつ実行したローカル計測では、最大ケース中央値が大きかったのは
`enumerate-primes`の4.480 s、LCAの1.088 s、次いでFPS各種の0.36--0.51 sだった。
これは同じマシン上で次の詳細調査対象を選ぶための**絶対時間によるスクリーニング**であり、
問題ごとに制約・入出力量が異なるためFastest比ではない。現行bundle提出を用意できている
Static RMQだけを、今回は確定した相対的ボトルネック（6.50x）として扱う。
