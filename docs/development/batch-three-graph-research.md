---
title: Graph additions batch three — research and candidate measurements
---

## 調査と採用

2026-09-18 に 4 ヘッダ、5 個の公式問題用 driver を追加した。
既存 API の変更はなく、共通 DSU は ACL、行列式は既存 Matrix を利用する。
公開実装はアルゴリズムと API の比較資料として読み、コードを転載せず独立に実装した。

| 対象 | 比較した一次資料・候補 | 採用判断 |
| --- | --- | --- |
| 割当 | [KACTL](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/WeightedMatching.h) の矩形最短増加路、[SciPy](https://github.com/scipy/scipy/blob/main/scipy/optimize/rectangular_lsap/rectangular_lsap.cpp) の rectangular LSAP | 整数の Hungarian 法で N≤M の矩形を扱う。外部実装の転写はせず行・列ポテンシャルを逐次更新する。O(N²M)、補助 O(N+M)。部分集合 DP は小規模比較用。 |
| 補グラフ成分 | [公式解](https://github.com/yosupo06/library-checker-problems/blob/master/graph/connected_components_of_complement_graph/sol/correct.cpp) の未訪問配列圧縮、[NetworkX](https://networkx.org/documentation/stable/_modules/networkx/algorithms/operators/unary.html) の補辺明示生成 | 未訪問頂点の単方向連結リストで O(N+M)、補助 O(N)。補辺 O(N²) を保存せず、毎探索で新規配列も確保しない。 |
| 全域木個数 | [maspypy](https://maspypy.github.io/library/graph/count/count_spanning_tree.hpp) の dense/sparse Matrix-Tree、[公式有向解](https://github.com/yosupo06/library-checker-problems/blob/master/graph/counting_spanning_tree_directed/sol/correct.cpp) の行列式 | 決定的な既存 Matrix の消去を再利用。O(N³+M)、補助 O(N²)。巨大疎行列向けの乱択 blackbox 法は今回対象外。部分集合の符号付き行列式 DP と比較。 |
| 最小全域森 | [ei1333](https://ei1333.github.io/algorithm/kruskal.html) の Kruskal、[公式解](https://github.com/yosupo06/library-checker-problems/blob/master/graph/minimum_spanning_tree/sol/correct.cpp) の priority queue Prim | ACL dsu を使う Kruskal を採用。入力辺リストをそのまま扱え、非連結・負辺も同一 API。O(N+M log(M+1))、補助 O(N+M)。 |

KACTL は CC0、SciPy はソース記載の BSD 条項を確認した。他資料を含め直接コードを取り込んでいないため、第三者コードのライセンスをこの実装へ移植していない。

## 同一条件の候補測定

[独立比較プログラム](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-graph.cpp)、
[全試行ログ](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-graph-results.csv)、
[実行環境](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-graph-environment.txt) を保存した。

GCC 13.3.0、`-std=gnu++20 -O2 -DNDEBUG -I. -isystem .deps/ac-library`、
WSL2 Ubuntu、Intel Core i7-14650HX、seed 20260918、同一生成入力を各候補 3 回。
入力生成は計測外、各候補の必要な補助構築は計測内。全候補で返り値 checksum が一致した。
共有マシン上の測定であり、別環境や公式 judge の最大ケース時間とは比較しない。

| 入力 | 候補 | 中央値 ms | 最小 ms |
| --- | --- | ---: | ---: |
| 割当 18×18 | Hungarian | 0.006225 | 0.003527 |
| 同上 | 部分集合 DP | 13.6022 | 13.2008 |
| 補グラフ N=3000、生成辺18000 | 連結リスト | 0.046622 | 0.042902 |
| 同上 | 密隣接行列探索 | 6.90002 | 6.89957 |
| 全域木 N=18、生成辺100 | 消去法 | 0.003671 | 0.002983 |
| 同上 | 部分集合行列式 DP | 7.6993 | 7.29471 |
| 最小森 N=30000、生成辺120000 | Kruskal | 9.92713 | 9.30487 |
| 同上 | Prim | 23.937 | 21.5586 |

指数時間候補との比較は小入力での独立検算と計算量差の確認であり、
最速の Hungarian 実装や消去実装との定数倍比較ではない。
サブミリ秒の絶対値や比率は安定した性能保証として扱わない。
今回の疎グラフでは Kruskal の定数倍も小さかったが、全密度での優位性は主張しない。

## 検証内容

`tests/random/graph_batch_three.cpp` は seed を引数で受け取り、割当を順列全列挙、
全域木と森を辺部分集合全列挙、補グラフ成分を密行列＋DSU と比較する。
失敗時に seed、trial、入力、および費用・個数比較の expected/actual を出す。
空入力、1 頂点、自己ループ、多重辺、非連結、負の費用、矩形割当を含む。
公式 driver は `assignment`、`connected_components_of_complement_graph`、
`counting_spanning_tree_directed`、`counting_spanning_tree_undirected`、`minimum_spanning_tree`。
統合検証の結果と実行回数はバッチ全体の検証記録を参照する。
