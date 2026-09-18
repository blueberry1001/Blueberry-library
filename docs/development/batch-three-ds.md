---
title: Batch three data structure adoption report
---

## 採用内容と比較

2026-09-18。コードは既存実装を転記せず独立に実装した。

- PersistentQueue: [Nyaanのdoubling](https://nyaannyaan.github.io/library/data-structure/persistent-queue.hpp.html) と [Library Checker公式解の遅延stream](https://github.com/yosupo06/library-checker-problems/blob/master/data_structure/persistent_queue/sol/correct.cpp) を調査。前者のO(Q log Q)メモリ、後者のshared_ptrと評価状態に対し、親とskip link各1本を格納するO(Q)メモリを採用した。隣接する2本のskip長が同じ場合に合成する。skip長は2の冪から1を引いた値になり、祖先探索はO(log Q)。push/popはvector再確保込み償却O(1)。先頭参照は最悪O(log Q)。版0が空、操作が新しい版IDを返す。
- StaticRangeDistinct: [suisenのoffline Fenwick](https://suisen-cp.github.io/cp-library-cpp/test/src/range_query/static%20range_count_distinct/static_range_count_distinct.test.cpp) と [anqooqieのwavelet変換](https://anqooqie.github.io/proconlib/tests/range_count_distinct.test.cpp) を調査。同値要素の直前位置がl未満である要素を[l,r)で数える。既存WaveletMatrixを利用し、クエリ順を制限しないAPIを採用。構築O(N log N)、問い合わせO(log N)。offline用途ではFenwick候補のほうが今回高速だった。
- Rectangle union: [sotanishyの最小被覆数/長さ方式](https://sotanishy.github.io/cp-library-cpp/misc/rectangle_union.hpp.html) と [spaghetti-sourceのBentley走査](https://github.com/spaghetti-source/algorithm/blob/master/geometry/rectangle_union.cc) を調査。走査で区間の被覆数と被覆長を保つ専用木を実装し、O(N log N)時間・O(N)メモリ。座標の差を取る前に面積型へ変換する。

## 再現可能な測定

[比較コード](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-ds.cpp)、[生ログ](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-ds-results.txt)。WSL2 x86_64、GCC 13.3.0、`-std=gnu++20 -O2 -I. -I.deps/ac-library`、seed3103、同一プロセスで3回。構築を含む時間、単位ms。全候補のchecksum一致をassertした。並列作業中のローカル測定であり、微小差の一般化や異環境との比較は行わない。

| 処理 | 候補 | 入力 | min | median |
| --- | --- | --- | ---: | ---: |
| queue | 採用skip | 100000 push後に全pop/front | 8.388 | 9.642 |
| queue | 固定17段doubling | 同じ値と経路 | 6.560 | 7.124 |
| distinct | 採用Wavelet | N=Q=100000、値域10000 | 48.396 | 48.755 |
| distinct | offline ACL Fenwick | 同じ配列と区間 | 10.981 | 11.236 |
| rectangle | 採用被覆木 | N=2000、座標0..99999 | 1.633 | 1.702 |
| rectangle | x帯ごとにactive y区間をsort/merge | 同じ矩形集合 | 91.502 | 92.797 |

queue代案は線形経路専用の比較ハーネスであり、汎用APIの版管理コストを含まない。採用根拠は時間の勝利ではなくO(Q)空間と簡単な版管理である。doublingは17個のint/要素、skip実装はint値の場合Nodeが16 bytes/要素と版が8 bytes/更新（vector容量の余裕は別）。distinctのoffline候補は値域を使ったlast配列を使用し、採用実装の一般の比較可能なTとは前提も異なる。入力を事前収集できる用途ではoffline候補が適する。rectangleの代案はO(N^2 log N)で、大規模入力への採用対象ではない。全候補は独立実装。

## 検証

公式IDは `persistent_queue`（Q<=500000）、`static_range_count_distinct`（N,Q<=500000）、`area_of_union_of_rectangles`（N<=500000、座標<=10^9）。各公式driverを追加した。実行ログは `.verification/batch-three-ds` と `.build/batch-three-ds-verify.log` に保存する。

`tests/random/ds_batch_three.cpp` はseed引数から再現可能。20seedで過去版分岐をdequeコピーと比較、10000要素のpush/pop経路、空配列、全区間のset比較、矩形の整数格子全セル比較、負座標・退化矩形・広い座標差を確認した。GCC 13.3 / Clang 18のgnu++20/23全4構成でseed23（コピー・移動の確認も含む）を実行して成功した。公式検証は1反復でqueue 14件、rectangle 21件、distinct 11件、計46件全AC。統合担当が全体検証も行う。
