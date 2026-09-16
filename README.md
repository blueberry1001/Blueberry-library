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
| Data Structure | Sparse Table, Rollback Union Find, Li Chao Tree, Wavelet Matrix, Offline Fenwick Tree 2D, Potential Union Find, Ordered Set, Persistent Segment Tree |
| Graph | Dijkstra, Lowest Common Ancestor, Heavy-Light Decomposition, Low Link, Rerooting DP, Hopcroft–Karp, Biconnected Components, Eulerian Trail |
| Math | Formal Power Series, Prime Sieve, Factorize, Modular Square Root, Linear Recurrence |
| String | Manacher, Aho–Corasick, Eertree, Lyndon Factorization |

標準のDSU・Fenwick Tree・Segment Tree・Z AlgorithmはACLを第一候補とし、既存のBlueberry版は互換用に保持します。
ドキュメントの検索欄では日本語の用途や `LCA`, `HLD`, `FPS`, `BIT` でも検索できます（`/` で検索へ移動）。

## 問題から探す

| やりたいこと | 選ぶもの・最初に使う操作 |
| --- | --- |
| 更新のない区間min/max/gcd | [Sparse Table](docs/data-structure/sparse-table.md): `prod(l,r)` |
| 併合を巻き戻す | [Rollback UF](docs/data-structure/rollback-union-find.md): `state()`, `rollback(state)` |
| 直線・線分の最小値 | [Li Chao Tree](docs/data-structure/li-chao-tree.md): `add_line`, `add_segment`, `query` |
| 区間k番目・値の頻度 | [Wavelet Matrix](docs/data-structure/wavelet-matrix.md): `kth_smallest`, `range_freq` |
| 点加算・長方形内の重みの和 | [Offline Fenwick 2D](docs/data-structure/offline-fenwick-tree-2d.md): 座標登録 → `add`, `sum` |
| 非負重みの最短路・経路復元 | [Dijkstra](docs/graph/dijkstra.md): `dijkstra`, `path_to` |
| 共通祖先・木の距離 | [LCA](docs/graph/lowest-common-ancestor.md): doubling / RMQ版を選択 |
| 木のパス・部分木クエリ | [HLD](docs/graph/heavy-light-decomposition.md) + ACL segtree |
| 橋・関節点・二辺連結成分 | [Low Link](docs/graph/low-link.md): `bridges`, `is_articulation`, `groups` |
| すべての頂点を根にした木DP | [Rerooting DP](docs/graph/rerooting.md): `rerooting` |
| 形式的冪級数の逆元・log・exp・sqrt | [FPS](docs/math/formal-power-series.md) + ACL modint |
| 全中心の最長回文 | [Manacher](docs/string/manacher.md): `manacher` |
| 差分制約・群のポテンシャルと矛盾判定 | [Potential UF](docs/data-structure/potential-union-find.md): `merge`, `diff` |
| 集合のオンライン更新と順位 | [Ordered Set](docs/data-structure/ordered-set.md): `insert`, `erase`, `rank`, `kth` |
| 過去の配列を残して一点更新 | [Persistent Segment Tree](docs/data-structure/persistent-segment-tree.md): `set(version,p,x)`, `prod(version,l,r)` |
| 最大二部マッチング・最小頂点被覆 | [Hopcroft–Karp](docs/graph/hopcroft-karp.md): `pairs`, `min_vertex_cover` |
| 二重頂点連結成分・block-cut forest | [Biconnected Components](docs/graph/biconnected-components.md): `groups`, `block_cut_tree` |
| 全辺を一度ずつ通る経路 | [Eulerian Trail](docs/graph/eulerian-trail.md): `exists`, `vertices`, `edges` |
| 64bit素数判定・素因数分解 | [Factorize](docs/math/factorize.md): `is_prime`, `factorize` |
| 素数modの平方根 | [Mod Sqrt](docs/math/mod-sqrt.md): `mod_sqrt` |
| 漸化式の推定と遠い項 | [Linear Recurrence](docs/math/linear-recurrence.md): `berlekamp_massey`, `linear_recurrence_kth` |
| 複数パターンの出現数 | [Aho–Corasick](docs/string/aho-corasick.md): `add` → `build` → `count` |
| 異なる回文とその出現数 | [Eertree](docs/string/eertree.md): `add`, `suffix`, `count` |
| 辞書順の非増加Lyndon分解 | [Lyndon Factorization](docs/string/lyndon-factorization.md): `lyndon_factorization` |
| SCC・2-SAT・最大流・最小費用流・畳み込み・suffix array | ACL: [公式リファレンス](https://atcoder.github.io/ac-library/production/document_ja/) |

## サポート範囲

カタログにある `blueberry/<category>/*.hpp` と互換入口 `fps.hpp` が検証対象です。
各ページには実行可能な最小例、全公開操作の説明、型や境界条件を載せています。

次の `blueberry/` 直下ファイルは**未検証の歴史的スニペット**で、新規利用を推奨しません。
削除やAPI変更はせずに残します。`all.hpp` にも含めません。

| 旧ファイル | 新規利用での選択肢・制限 |
| --- | --- |
| `ConvexHulltrick.hpp` | [Li Chao Tree](docs/data-structure/li-chao-tree.md)（問い合わせるx座標を事前登録） |
| `DynamicFenwickTree2D.hpp` | [Offline Fenwick 2D](docs/data-structure/offline-fenwick-tree-2d.md)（更新座標の事前登録が必要） |
| `RollbackUnionFind.hpp` | [Rollback Union Find](docs/data-structure/rollback-union-find.md) |
| `Graph.hpp` | 最短路は[Dijkstra](docs/graph/dijkstra.md)、SCC等はACL。全APIを置換するものではありません |
| `implicit_treap.hpp` | 名前と異なり値をキーにする木。集合用途は[Ordered Set](docs/data-structure/ordered-set.md)。列のreverse/lazy操作や多重集合の代替ではありません |
| `fraction.hpp` | 型範囲外の演算を保証しません。検証済み代替は未収録 |

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

Python 3.8以上とC++20対応のGCCまたはClangが必要です。初回だけ `make setup` を実行すると、
verification-helperとACLを準備します。

```console
make setup
make check
make verify
make docs
```

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
| `make include-test` | 公開カタログの全ヘッダと互換入口を、1個ずつ単独includeしてコンパイル |
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

pushとpull requestでは、GCC/Clang × GNU C++20/23の4環境で、全公開ヘッダの単体include、
全verifyコードのコンパイル、共通random testを実行します。これにすべて通った後、GCC・GNU C++20で
Pythonテスト、ドキュメント例、Library Checker公式ケース、生成サイトを検証します。
各compile jobの結果はGitHub ActionsのSummaryに件数付きで表示され、Library Checkerの詳細な
実行結果と計測ログは `verification-metrics` artifactに保存されます。

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

既存のルート直下ヘッダは旧構成との互換性のため残しており、順次、新構成への移行と
検証コードの追加を進めます。
