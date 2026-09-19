# 固定木での動的構造と固定構造の比較

辺を変更できるデータ構造も固定された木の操作に使用できる。同じ木・初期値・操作列を使い、その追加機能のコストを比較する。
**辺の変更を含むワークロードは対象外**。link/cut の性能比較や動的構造の不要性を主張する測定ではない。

| 問題 | 動的構造 | 固定構造 |
| --- | --- | --- |
| 頂点の係数を置換し根の木 DP を取得 | DynamicTopTree | StaticTopTree |
| 頂点代入・頂点パス和 | LinkCutTree | HLD + ACL segtree |
| 固定根の部分木加算・部分木和 | EulerTourTree | Euler 順序 + ACL lazy_segtree |

## 条件と再現方法

- N=1024 / 16384、各20,000操作、seed=20260919。頂点 v>0 の親を [0,v) から一様に選ぶ random recursive tree。根は0で固定。パス・星など別の形の結果は測っていない。
- 木 DP は `dp[v] = a[v] * sum(dp[child]) + b[v]`。毎操作で1頂点の係数を置換し、`all_prod().b`（動的版は `all_prod(0).b`）を読む。uint64_t の演算で法2^64の意味を持つ。
- パス和と部分木操作は更新50%・問い合わせ50%を独立に抽選。頂点は一様乱数、初期値0..100、更新値-10..10。long long の範囲に収まる。
- DynamicTopTree / LCT / ETT の構築時間には全初期辺の link を含む。固定構造側も木の前処理と ACL 構造の構築を含む。ETT の部分木操作が内部で cut/link を使う費用は操作時間に含まれるが、論理的な木の形は不変。
- 同一実行ファイル、GCC、`-std=gnu++20 -O2 -DNDEBUG`。入力生成・JSON出力を除き、構築・操作時間を別々に測る。
- 5回、別プロセス、動的版と固定版の実行順序を交互にする。ウォームアップ・CPU固定なし。全反復で入力hash・結果checksumの一致を確認し、測定中のソース変更を拒否する。

[比較ソース](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmarks/dynamic-tree-comparison.cpp)、
[再現スクリプト](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmarks/run-dynamic-tree-comparison.py)。
リポジトリ直下で `python3 benchmarks/run-dynamic-tree-comparison.py --output .build/dynamic-tree-remeasurement`。
既存サンプルの上書きは禁止するため、再測定には新しい出力先を指定する。

## 結果

2026-09-19、Intel Core i7-14650HX / WSL2 Linux / GCC 13.3。
ほかの担当の重い処理を止めた順次測定。各欄は **中央値 / 最小値、単位 ms**、各入力5回。

| 問題 | N | 実装 | 構築 ms | 20,000操作 ms |
| --- | ---: | --- | ---: | ---: |
| 木 DP | 1024 | DynamicTopTree | 0.421 / 0.415 | 7.563 / 7.030 |
| 木 DP | 1024 | StaticTopTree | 0.184 / 0.140 | 1.607 / 1.264 |
| 木 DP | 16384 | DynamicTopTree | 9.287 / 8.760 | 11.554 / 10.772 |
| 木 DP | 16384 | StaticTopTree | 2.527 / 2.252 | 2.534 / 2.449 |
| パス和 | 1024 | LinkCutTree | 0.223 / 0.213 | 6.755 / 6.695 |
| パス和 | 1024 | HLD + ACL segtree | 0.047 / 0.042 | 0.971 / 0.905 |
| パス和 | 16384 | LinkCutTree | 4.421 / 4.230 | 9.202 / 8.885 |
| パス和 | 16384 | HLD + ACL segtree | 0.764 / 0.736 | 1.417 / 1.357 |
| 部分木加算・和 | 1024 | EulerTourTree | 0.631 / 0.568 | 20.059 / 19.859 |
| 部分木加算・和 | 1024 | Euler 順序 + ACL lazy_segtree | 0.034 / 0.032 | 1.354 / 1.250 |
| 部分木加算・和 | 16384 | EulerTourTree | 14.159 / 13.345 | 30.138 / 28.950 |
| 部分木加算・和 | 16384 | Euler 順序 + ACL lazy_segtree | 0.657 / 0.587 | 1.860 / 1.644 |

全60サンプルで入力hash・結果checksumが一致し、計測中のソース変更がないことを確認した。
事前に N=1/2/37、各3000操作でも3組の結果を照合した。
この固定木ワークロードでは、固定構造の方が構築・操作とも速かった。動的構造は同じ操作を処理できるが、辺変更などを可能にする内部処理の費用がある。
操作検索では固定木にも動的構造を候補に含めつつ、辺変更の必要性とこの費用を区別すると選びやすい。
木の形・更新率・問い合わせ分布・代数を変えた場合の倍率や、辺変更操作の性能に一般化しない。メモリ使用量は実測していない。

[全60サンプル](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-tree-comparison/samples.jsonl)、
[集計 JSON](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-tree-comparison/summary.json)、
[環境・ソース SHA256](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-tree-comparison/environment.json)、
[コンパイルログ](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-tree-comparison/compile.log) を保存した。
