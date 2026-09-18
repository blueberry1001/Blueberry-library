# Blueberry Library

[![verify](https://github.com/blueberry1001/Blueberry-library/actions/workflows/verify.yml/badge.svg)](https://github.com/blueberry1001/Blueberry-library/actions/workflows/verify.yml)
[![documentation](https://img.shields.io/badge/docs-GitHub%20Pages-0969da?logo=github)](https://blueberry1001.github.io/Blueberry-library/)

C++20向けの競技プログラミング用ライブラリです。`blueberry/` 以下のヘッダは、
[Library Checker](https://judge.yosupo.jp/) のテストケースを利用して
[verification-helper](https://github.com/online-judge-tools/verification-helper) で継続的に検証します。

## 使い方

リポジトリのルートをインクルードパスに追加し、必要なヘッダだけを読み込みます。

```cpp
#include "blueberry/data-structure/rollback-union-find.hpp"

blueberry::RollbackUnionFind uf(n);
int saved = uf.state();
uf.merge(u, v);
uf.rollback(saved);
```

まとめて読み込む場合は `#include "blueberry/all.hpp"` を使用できます（FPSを含むためACLが必要）。
提出用の単一ファイルは `oj-bundle main.cpp -I .` で生成できます。

## 収録ライブラリ

| 分類 | ライブラリ |
| --- | --- |
| Data Structure | Sparse Table / Disjoint Sparse Table / Sqrt Tree, Rollback Union Find, Li Chao Tree / Dynamic Li Chao Tree, Wavelet Matrix / Weighted Wavelet Matrix, Offline / Dynamic Fenwick Tree 2D, Dynamic Fenwick Tree, Potential Union Find, Ordered Set / Multiset, Implicit Treap, Persistent Segment Tree, Segment Tree Beats, Binary Trie / Persistent Binary Trie, Aggregate Queue / Deque, Persistent Queue, Static Range Distinct, Rectangle Union Area, Static Range Inversions, Static Range Mode |
| Graph | Dijkstra, Lowest Common Ancestor, Heavy-Light Decomposition, Low Link, Rerooting DP, Hopcroft–Karp, Biconnected Components, Eulerian Trail, Cartesian Tree, Cycle Detection, Topological Sort, Tree Diameter, Triangle Enumeration, Assignment, Complement Components, Count Spanning Trees, Minimum Spanning Forest, Rooted Tree Isomorphism, Clique Enumeration, Maximum Independent Set |
| Math | Formal Power Series, Fraction, Prime Sieve, Factorize, Modular Square Root, Linear Recurrence, Bitwise Convolution, Subset Convolution, Matrix, Divisor Convolution, Enumerate Quotients, Kth Root Integer, Discrete Logarithm, Longest Increasing Subsequence, Polynomial Taylor Shift, Polynomial Product, Stirling Numbers Second Kind, Montmort Numbers, Multipoint Evaluation, Polynomial Interpolation, Sample Point Shift |
| String | Manacher, Aho–Corasick, Eertree, Lyndon Factorization, Prefix Function / KMP, Count Subsequences, Longest Common Substring |

標準のDSU・Fenwick Tree・Segment Tree・Z AlgorithmはACLを第一候補とし、既存のBlueberry版は互換用に保持します。
ドキュメントの検索欄では日本語の用途や `LCA`, `HLD`, `FPS`, `BIT` でも検索できます（`/` で検索へ移動）。

[操作から探す](https://blueberry1001.github.io/Blueberry-library/operations.html)では、配列・木・二次元・集合などの対象から、知りたい値・更新方法・入力条件を絞り込みます。
更新なしでも動的構造を候補に含め、SegTreeの境界探索・最初の該当位置・区間affine/代入・chmin/chmaxも探せます。
静的な問い合わせの到着順と、値の更新の有無は別の条件として扱います。
[Library Checker対応表](https://blueberry1001.github.io/Blueberry-library/library-checker.html)は全公開問題のスナップショットを掲載し、
実装とverifyあり／専用verifyなし／ACL推奨／未対応を区別します。リポジトリ内の[チェックリスト](docs/library-checker-checklist.md)も同じデータから生成します。

## 問題から探す

| やりたいこと | 選ぶもの・最初に使う操作 |
| --- | --- |
| 更新のない区間min/max/gcd | [Sparse Table](docs/data-structure/sparse-table.md): `prod(l,r)` |
| 併合を巻き戻す | [Rollback UF](docs/data-structure/rollback-union-find.md): `state()`, `rollback(state)` |
| 直線・線分の最小値 | [Li Chao Tree](docs/data-structure/li-chao-tree.md): `add_line`, `add_segment`, `query` |
| 未知の座標で直線・線分の最小値 | [Dynamic Li Chao Tree](docs/data-structure/dynamic-li-chao-tree.md): 領域指定 → `add_line`, `query` |
| 区間k番目・値の頻度 | [Wavelet Matrix](docs/data-structure/wavelet-matrix.md): `kth_smallest`, `range_freq` |
| 点加算・長方形内の重みの和 | [Offline Fenwick 2D](docs/data-structure/offline-fenwick-tree-2d.md): 座標登録 → `add`, `sum` |
| 未知の更新点に対する点加算・長方形和 | [Dynamic Fenwick 2D](docs/data-structure/dynamic-fenwick-tree-2d.md): `add`, `pref`, `sum` |
| 巨大な添字の疎な配列 | [Dynamic Fenwick](docs/data-structure/dynamic-fenwick-tree.md): `add`, `sum`, `lower_bound` |
| 非負重みの最短路・経路復元 | [Dijkstra](docs/graph/dijkstra.md): `dijkstra`, `path_to` |
| 共通祖先・木の距離 | [LCA](docs/graph/lowest-common-ancestor.md): doubling / RMQ版を選択 |
| 木のパス・部分木クエリ | [HLD](docs/graph/heavy-light-decomposition.md) + ACL segtree |
| 橋・関節点・二辺連結成分 | [Low Link](docs/graph/low-link.md): `bridges`, `is_articulation`, `groups` |
| すべての頂点を根にした木DP | [Rerooting DP](docs/graph/rerooting.md): `rerooting` |
| 形式的冪級数の逆元・log・exp・sqrt | [FPS](docs/math/formal-power-series.md) + ACL modint |
| 全中心の最長回文 | [Manacher](docs/string/manacher.md): `manacher` |
| 差分制約・群のポテンシャルと矛盾判定 | [Potential UF](docs/data-structure/potential-union-find.md): `merge`, `diff` |
| 集合のオンライン更新と順位 | [Ordered Set](docs/data-structure/ordered-set.md): `insert`, `erase`, `rank`, `kth` |
| 重複を含む集合の順位・区間集約 | [Ordered Multiset](docs/data-structure/ordered-multiset.md): `count`, `kth`, `prod` |
| 動的な列の挿入・削除・反転・区間作用 | [Implicit Treap](docs/data-structure/implicit-treap.md): `insert`, `erase`, `reverse`, `apply` |
| 過去の配列を残して一点更新 | [Persistent Segment Tree](docs/data-structure/persistent-segment-tree.md): `set(version,p,x)`, `prod(version,l,r)` |
| 最大二部マッチング・最小頂点被覆 | [Hopcroft–Karp](docs/graph/hopcroft-karp.md): `pairs`, `min_vertex_cover` |
| 二重頂点連結成分・block-cut forest | [Biconnected Components](docs/graph/biconnected-components.md): `groups`, `block_cut_tree` |
| 全辺を一度ずつ通る経路 | [Eulerian Trail](docs/graph/eulerian-trail.md): `exists`, `vertices`, `edges` |
| 64bit素数判定・素因数分解 | [Factorize](docs/math/factorize.md): `is_prime`, `factorize` |
| 素数modの平方根 | [Mod Sqrt](docs/math/mod-sqrt.md): `mod_sqrt` |
| 漸化式の推定と遠い項 | [Linear Recurrence](docs/math/linear-recurrence.md): `berlekamp_massey`, `linear_recurrence_kth` |
| 分数の厳密な比較・四則演算 | [Fraction](docs/math/fraction.md): `num`, `den`, `+`, `-`, `*`, `/` |
| 複数パターンの出現数 | [Aho–Corasick](docs/string/aho-corasick.md): `add` → `build` → `count` |
| 異なる回文とその出現数 | [Eertree](docs/string/eertree.md): `add`, `suffix`, `count` |
| 辞書順の非増加Lyndon分解 | [Lyndon Factorization](docs/string/lyndon-factorization.md): `lyndon_factorization` |
| 更新のない列の非可換な区間積 | [Disjoint Sparse Table](docs/data-structure/disjoint-sparse-table.md): `prod(l,r)` |
| 静的モノイド区間積をO(1)で取得 | [Sqrt Tree](docs/data-structure/sqrt-tree.md): `prod(l,r)`、構築O(N log log N) |
| 過去の集合へ分岐してXOR最小値・順位 | [Persistent Binary Trie](docs/data-structure/persistent-binary-trie.md): `insert(v,x)`, `xor_min(v,x)`, `kth(v,k)` |
| 固定値列の重みを更新し、添字区間×値域の和 | [Weighted Wavelet Matrix](docs/data-structure/weighted-wavelet-matrix.md): `add(p,w)`, `sum(l,r,low,upper)` |
| 配列から最小ヒープ順の木を構築 | [Cartesian Tree](docs/graph/cartesian-tree.md): `cartesian_tree` |
| 有向・無向グラフの閉路復元 | [Cycle Detection](docs/graph/cycle-detection.md): `find_cycle<Directed>` |
| DAGの処理順と閉路の有無 | [Topological Sort](docs/graph/topological-sort.md): `topological_sort` |
| 非負重みの木の最長経路 | [Tree Diameter](docs/graph/tree-diameter.md): `tree_diameter` |
| 単純無向グラフの三角形 | [Triangle Enumeration](docs/graph/triangle-enumeration.md): `enumerate_triangles` |
| 添字のgcd・lcmによる畳み込み | [Divisor Convolution](docs/math/divisor-convolution.md): `gcd_convolution`, `lcm_convolution` |
| 整数除算の異なる商 | [Enumerate Quotients](docs/math/enumerate-quotients.md): `enumerate_quotients` |
| 整数のk乗根の切り捨て | [Kth Root Integer](docs/math/kth-root-integer.md): `kth_root_integer` |
| 累乗の合同式から指数を求める | [Discrete Logarithm](docs/math/discrete-log.md): `discrete_log` |
| 最長増加部分列の復元 | [LIS](docs/math/longest-increasing-subsequence.md): `longest_increasing_subsequence` |
| 接頭辞関数・パターンの全出現位置 | [Prefix Function / KMP](docs/string/prefix-function.md): `prefix_function`, `kmp_search` |
| 静的配列の区間転倒数 | [Static Range Inversions](docs/data-structure/static-range-inversions.md): `static_range_inversions` |
| 静的配列の区間最頻値 | [Static Range Mode](docs/data-structure/static-range-mode.md): `static_range_mode` |
| 根付き部分木の同型分類 | [Rooted Tree Isomorphism](docs/graph/rooted-tree-isomorphism.md): `rooted_tree_isomorphism` |
| すべての非空クリーク | [Clique Enumeration](docs/graph/clique-enumeration.md): `enumerate_cliques` |
| 40頂点以下の最大独立集合 | [Maximum Independent Set](docs/graph/maximum-independent-set.md): `maximum_independent_set` |
| 多項式の多点評価 | [Multipoint Evaluation](docs/math/multipoint-evaluation.md): `multipoint_evaluation` |
| 点の値から多項式を復元 | [Polynomial Interpolation](docs/math/polynomial-interpolation.md): `polynomial_interpolation` |
| 連続する標本点のシフト | [Sample Point Shift](docs/math/sample-point-shift.md): `sample_point_shift` |
| 2文字列の最長共通部分文字列 | [Longest Common Substring](docs/string/longest-common-substring.md): `longest_common_substring` |
| SCC・2-SAT・最大流・最小費用流・畳み込み・suffix array | ACL: [公式リファレンス](https://atcoder.github.io/ac-library/production/document_ja/) |

## サポート範囲

カタログにある `blueberry/<category>/*.hpp` と一括入口 `all.hpp` が検証対象です。
各ページには実行可能な最小例、全公開操作の説明、型や境界条件を載せています。

未検証だった旧ルート直下ヘッダ6個と `fps.hpp` は、2026-09-17 の整備で削除しました。
オンライン版・多重集合・分数には検証付きの新しい実装があります。
旧ファイル名、include、APIの対応は[旧版からの移行](https://blueberry1001.github.io/Blueberry-library/migration.html)を参照してください。
特に旧 `implicit_treap.hpp` はキー順多重集合なので、新しい `OrderedMultiset` が移行先です。
新 `ImplicitTreap` は位置で扱う動的列です。

詳細な説明・計算量・検証コードは[ドキュメント](https://blueberry1001.github.io/Blueberry-library/)に掲載します。
ACLとの役割分担や実装・採用基準は[実装方針](IMPLEMENTATION_POLICY.md)にまとめています。

形式的冪級数は `#include "blueberry/math/formal-power-series.hpp"` で読み込みます。
`atcoder::modint998244353` などのACL `static_modint`を係数型に指定し、`inv`・`log`・
`exp`・`sqrt`・`pow`を利用できます。一般的な多項式積はACLの`convolution`を使い、
FPS固有のNewton反復だけをBlueberry側で提供します。
FPSを使う場合は、ACLの `atcoder/` ヘッダをインクルードパスに配置してください。
GitHub Actionsでは公式ACLを取得してから検証します。

LCA が大量に必要な場合は `LowestCommonAncestorRMQ`（Euler tour + Sparse Table、クエリ O(1)）を、
部分木・パスクエリには `HeavyLightDecomposition` を利用できます。後者は ACL の `segtree` / `lazy_segtree`
を分解配列の区間に適用する設計です。

区間積の新しい呼び出しは `SparseTable::prod` / `SegmentTree::prod`（全体は `all_prod`）を推奨します。
既存の `product` / `all_product` も互換性のため残しています。Fenwick Tree のprefix取得には短い
`pref`、Union Find の成分サイズには `comp_size` も使えます。

## ローカルでの検証

WindowsではWSL2のUbuntuから同じ検証手順を実行できます。
GCC/ClangとPython仮想環境の準備・実行方法は
[`docs/development/windows.md`](docs/development/windows.md)を参照してください。

Python 3.9以上とC++20対応のGCCまたはClangが必要です。Fraction の独立参照テスト・比較 benchmark には
Boost ヘッダ（Ubuntu: `libboost-dev`）も使います。本体は Boost に依存しません。初回だけ `make setup` を実行すると、
verification-helperとACLを準備します。

操作フィルターと問題一覧フィルターのJavaScript単体テストにはNode.jsを使います。
未導入なら当該テストはskipと表示されます（C++ライブラリの利用には不要）。

```console
make setup
make check
make verify
make docs
```

ライブラリやverifyの追加後は `python3 scripts/library_checker_coverage.py` で対応表を再生成してください。
`make test` 内の `--check` が登録・更新漏れを検出します。新しい公開問題を取り込む場合のみ、公式問題リポジトリを更新して
`python3 scripts/library_checker_coverage.py --refresh --upstream /path/to/library-checker-problems` を実行します。
専用verifyがまだない実装やACL案内は `data/library-checker-mappings.json` にAPIと前提を確認して登録します。
verifyの `PROBLEM` / `IGNORE` は無条件のトップレベル定義を使ってください。対応表はC++プリプロセッサの一般的な条件評価を行いません。

Fastest 提出の調査には `scripts/fetch_lc_submissions.py` を推奨します（下記benchmark節）。
先行PRの `scripts/fetch_lc_fastest.py` は既存CLI・出力形式の互換用として残しています。

```console
python3 scripts/fetch_lc_fastest.py exp_of_formal_power_series --limit 10 \
  --out-dir .verification/lc/exp --save-source
```

各コマンドの役割は次の通りです。

| コマンド | 内容 |
| --- | --- |
| `make check` | Pythonテスト、ドキュメント例、compile test、random testをまとめて実行 |
| `make include-test` | 公開カタログの全ヘッダと `all.hpp` を、1個ずつ単独includeしてコンパイル |
| `make verify-compile` | `verify/**/*.test.cpp` を公式ケースの取得なしで全件コンパイル |
| `make random-test` | `tests/random/**/*.cpp` をコンパイルし、既定でseed 1から20回実行 |
| `make verify` | Library Checker公式ケースをchecker付きで実行し、3回の中央値を記録 |

全公式検証の時間上限は既定で3600秒です。ケース生成や低速なファイルシステムで不足する場合は、
`make verify VERIFY_TIMEOUT=7200` のように指定できます。個々の解答のTLE設定は変更しません。

コンパイラと標準は、例えば
`make compile-test CXX=clang++ CXX_STANDARD=gnu++23` のように切り替えられます。
ACLを別の場所に置く場合は `ACL_ROOT=/path/to/ac-library` を指定してください。

random testはseedを `argv[1]` と `BLUEBERRY_RANDOM_SEED` の両方で受け取れます。
失敗時に表示されたseedは
`RANDOM_SEED=123 RANDOM_RUNS=1 make random-test` で再現できます。配置規約と最小例は
[`tests/random/README.md`](tests/random/README.md)にあります。

`make verify` は `verify/**/*.test.cpp` を検出し、指定されたLibrary Checkerの公式テストケースを
ダウンロードしてローカル実行します。verification-helperの標準仕様はYosupoへ自動提出する方式ではなく、
同じテストデータとcheckerを使ってACかどうかを判定する方式です。

## CIで行う検証

pushとpull requestでは、成功済みmainからの変更と、変更前後の推移的なinclude依存を調べ、
影響するLibrary Checker公式ケースだけを1回実行します。C++の変更時は従来どおり
GCC/Clang × GNU C++20/23の4環境で全ヘッダ・全verifyのコンパイルとrandom testを行います。
文書/UIだけの変更では重いC++検証を省き、Python/UIテスト、文書形式、変更文書のC++例、
生成サイトを検査します。基準や依存解析が不明な場合、検証harness・未知のファイル変更は全件へ戻します。
直前のmain CIが失敗・キャンセルされていても、その未検証の変更を次の検証へ引き継ぎます。

手動の `workflow_dispatch` は全件3回です。ローカル `make verify` も全件・既定3回のままです。
既存のrequired check名と公開条件を維持し、対象外を成功済みとは表示しません。
選択理由は `ci-plan`、結果と計測ログは `verification-metrics` artifactに保存します。
詳細は [変更範囲に応じたCI](docs/development/change-aware-ci.md) を参照してください。

性能調査は`make benchmark`で決定的なmicrobenchmarkを反復実行できます。Library Checkerの
Fastest収集、compiler option matrix、計測条件と現在のStatic RMQ調査結果は
[`benchmark/README.md`](benchmark/README.md)と[`benchmark/REPORT.md`](benchmark/REPORT.md)を参照してください。

## Yosupoへの提出

`online-judge-tools` のLibrary Checker向けアダプタは、現在テストケースの取得とローカル検証を
提供している一方、提出APIには対応していません。そのためGitHub Actionsからの自動提出は行わず、
必要な場合は `oj-bundle` で生成した単一ファイルを [Library Checker](https://judge.yosupo.jp/) の
提出画面から提出してください。

## ライブラリを追加する流れ

1. `blueberry/<category>/name.hpp` に自己完結したヘッダを追加する。
2. `docs/<category>/name.md` に仕様と計算量を書く。
3. `verify/<category>/name.test.cpp` に `PROBLEM` と検証コードを書く。
4. `make verify` と `make docs` を実行する。
5. pushするとGitHub Actionsが再検証し、`main` ならGitHub Pagesも更新する。

新しい実装はカテゴリ別ヘッダへ追加し、カタログ・API文書・公式verify・境界テストを揃えます。
