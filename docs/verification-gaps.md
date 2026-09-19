# ライブラリごとのverify状況

`python3 scripts/library_checker_coverage.py` で生成。専用ドライバ未登録を先に掲載します。

ドライバの存在・include関係はACや全APIの網羅を意味しません。all.hppのA+Bテストは含めません。
問題が存在しないという断定ではありません。調査済みの候補・未検証操作・次の作業は注記を参照してください。

## Partially Retroactive Priority Queue

[blueberry/data-structure/partially-retroactive-priority-queue.hpp](../blueberry/data-structure/partially-retroactive-priority-queue.hpp) — **公式verify未登録**

- 関連ローカルテスト: [tests/random/partially-retroactive-priority-queue.cpp](../tests/random/partially-retroactive-priority-queue.cpp)
- 2026-09-19調査時のLibrary Checker公開問題スナップショットには、過去操作の編集を直接検証する問題を見つけていません。
- 通常のpriority queue問題だけでは過去操作の編集を検証できず、専用の公式verifyは未登録です。任意の過去時点の問い合わせは提供しない部分retroactive版です。
- 操作履歴を毎回再実行する愚直解で、過去への挿入・削除・不正操作拒否を比較する。
- [追跡Issue](https://github.com/blueberry1001/Blueberry-library/issues/28)

## Slope Trick（heap版）

[blueberry/data-structure/slope-trick.hpp](../blueberry/data-structure/slope-trick.hpp) — **公式verify未登録**

- 関連ローカルテスト: [tests/random/slope-tricks.cpp](../tests/random/slope-tricks.cpp)
- 2026-09-19調査時のLibrary Checker公開問題スナップショットには、heap版Slope Trickの操作を直接検証する対象を見つけていません。
- 小さい整数点での愚直比較はありますが、公式verify未登録です。ABC127 Fは候補で、shiftやmerge全体の検証にはなりません。
- 他ジャッジの適切な問題を追加調査し、問題が扱わない操作はランダム比較を維持する。
- [追跡Issue](https://github.com/blueberry1001/Blueberry-library/issues/26)

## Topological Sort

[blueberry/graph/topological-sort.hpp](../blueberry/graph/topological-sort.hpp) — **公式verify未登録**

- 関連ローカルテスト: [tests/random/graph-batch-two.cpp](../tests/random/graph-batch-two.cpp)
- 専用の公式verifyドライバは未登録です。
- 関連ローカルテストはありますが、公式ジャッジでの専用検証を表すものではありません。
- DAGの順序制約・閉路時の返り値を確かめられる問題を選び、専用ドライバを追加する。
- [追跡Issue](https://github.com/blueberry1001/Blueberry-library/issues/24)

## Aggregate Deque

[blueberry/data-structure/aggregate-deque.hpp](../blueberry/data-structure/aggregate-deque.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/aggregate-deque.test.cpp](../verify/data-structure/aggregate-deque.test.cpp) / [問題](https://judge.yosupo.jp/problem/deque_operate_all_composite)
- 関連ローカルテスト: [tests/random/operation-structures.cpp](../tests/random/operation-structures.cpp)

## Aggregate Queue

[blueberry/data-structure/aggregate-queue.hpp](../blueberry/data-structure/aggregate-queue.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/aggregate-queue.test.cpp](../verify/data-structure/aggregate-queue.test.cpp) / [問題](https://judge.yosupo.jp/problem/queue_operate_all_composite)
- 関連ローカルテスト: [tests/random/operation-structures.cpp](../tests/random/operation-structures.cpp)

## Binary Trie

[blueberry/data-structure/binary-trie.hpp](../blueberry/data-structure/binary-trie.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/binary-trie.test.cpp](../verify/data-structure/binary-trie.test.cpp) / [問題](https://judge.yosupo.jp/problem/set_xor_min)
- 関連ローカルテスト: [tests/random/operation-structures.cpp](../tests/random/operation-structures.cpp)

## Disjoint Set Union

[blueberry/data-structure/disjoint-set-union.hpp](../blueberry/data-structure/disjoint-set-union.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/unionfind.test.cpp](../verify/data-structure/unionfind.test.cpp) / [問題](https://judge.yosupo.jp/problem/unionfind)
- 関連ローカルテスト: [tests/data_structure_random_test.cpp](../tests/data_structure_random_test.cpp)

## Disjoint Sparse Table

[blueberry/data-structure/disjoint-sparse-table.hpp](../blueberry/data-structure/disjoint-sparse-table.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/disjoint-sparse-table-rmq.test.cpp](../verify/data-structure/disjoint-sparse-table-rmq.test.cpp) / [問題](https://judge.yosupo.jp/problem/staticrmq)
- 専用ドライバ: [verify/data-structure/disjoint-sparse-table.test.cpp](../verify/data-structure/disjoint-sparse-table.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_sum)
- 関連ローカルテスト: [tests/random/sequence-batch-two.cpp](../tests/random/sequence-batch-two.cpp)

## Double Ended Priority Queue

[blueberry/data-structure/double-ended-priority-queue.hpp](../blueberry/data-structure/double-ended-priority-queue.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/double-ended-priority-queue.test.cpp](../verify/data-structure/double-ended-priority-queue.test.cpp) / [問題](https://judge.yosupo.jp/problem/double_ended_priority_queue)
- 関連ローカルテスト: [tests/random/specialized-structures.cpp](../tests/random/specialized-structures.cpp)

## Dual Segment Tree

[blueberry/data-structure/dual-segment-tree.hpp](../blueberry/data-structure/dual-segment-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/dual-segment-tree.test.cpp](../verify/data-structure/dual-segment-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/range_affine_point_get)
- 関連ローカルテスト: [tests/random/specialized-structures.cpp](../tests/random/specialized-structures.cpp)

## Dynamic Fenwick Tree 2D

[blueberry/data-structure/dynamic-fenwick-tree-2d.hpp](../blueberry/data-structure/dynamic-fenwick-tree-2d.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/dynamic-point-add-rectangle-sum.test.cpp](../verify/data-structure/dynamic-point-add-rectangle-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
- 専用ドライバ: [verify/data-structure/dynamic-rectangle-sum.test.cpp](../verify/data-structure/dynamic-rectangle-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/rectangle_sum)
- 関連ローカルテスト: [tests/random/dynamic-fenwick-tree.cpp](../tests/random/dynamic-fenwick-tree.cpp)

## Dynamic Fenwick Tree

[blueberry/data-structure/dynamic-fenwick-tree.hpp](../blueberry/data-structure/dynamic-fenwick-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/dynamic-point-add-range-sum.test.cpp](../verify/data-structure/dynamic-point-add-range-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_range_sum)
- 関連ローカルテスト: [tests/random/dynamic-fenwick-tree.cpp](../tests/random/dynamic-fenwick-tree.cpp)

## Dynamic Li Chao Tree

[blueberry/data-structure/dynamic-li-chao-tree.hpp](../blueberry/data-structure/dynamic-li-chao-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/dynamic-line-add-get-min.test.cpp](../verify/data-structure/dynamic-line-add-get-min.test.cpp) / [問題](https://judge.yosupo.jp/problem/line_add_get_min)
- 専用ドライバ: [verify/data-structure/dynamic-segment-add-get-min.test.cpp](../verify/data-structure/dynamic-segment-add-get-min.test.cpp) / [問題](https://judge.yosupo.jp/problem/segment_add_get_min)
- 関連ローカルテスト: [tests/random/dynamic-li-chao-tree.cpp](../tests/random/dynamic-li-chao-tree.cpp)

## Dynamic Sqrt Tree

[blueberry/data-structure/dynamic-sqrt-tree.hpp](../blueberry/data-structure/dynamic-sqrt-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/dynamic-sqrt-tree-sum.test.cpp](../verify/data-structure/dynamic-sqrt-tree-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_range_sum)
- 専用ドライバ: [verify/data-structure/dynamic-sqrt-tree.test.cpp](../verify/data-structure/dynamic-sqrt-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_range_composite)
- 関連ローカルテスト: [tests/random/dynamic-sqrt-tree.cpp](../tests/random/dynamic-sqrt-tree.cpp)

## Fast Set（64分木）

[blueberry/data-structure/fast-set.hpp](../blueberry/data-structure/fast-set.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/fast-set.test.cpp](../verify/data-structure/fast-set.test.cpp) / [問題](https://judge.yosupo.jp/problem/predecessor_problem)
- 関連ローカルテスト: [tests/random/specialized-structures.cpp](../tests/random/specialized-structures.cpp)

## Fenwick Tree

[blueberry/data-structure/fenwick-tree.hpp](../blueberry/data-structure/fenwick-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/point-add-range-sum.test.cpp](../verify/data-structure/point-add-range-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_range_sum)
- 間接利用: [verify/data-structure/weighted-wavelet-matrix.test.cpp](../verify/data-structure/weighted-wavelet-matrix.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
- 関連ローカルテスト: [tests/data_structure_random_test.cpp](../tests/data_structure_random_test.cpp)
- 関連ローカルテスト: [tests/random/weighted-wavelet-matrix.cpp](../tests/random/weighted-wavelet-matrix.cpp)

## Implicit Treap

[blueberry/data-structure/implicit-treap.hpp](../blueberry/data-structure/implicit-treap.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/implicit-treap-affine.test.cpp](../verify/data-structure/implicit-treap-affine.test.cpp) / [問題](https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum)
- 専用ドライバ: [verify/data-structure/implicit-treap-reverse-sum.test.cpp](../verify/data-structure/implicit-treap-reverse-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/range_reverse_range_sum)
- 関連ローカルテスト: [tests/random/legacy-treap.cpp](../tests/random/legacy-treap.cpp)

## KD Tree

[blueberry/data-structure/kd-tree.hpp](../blueberry/data-structure/kd-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/kd-tree.test.cpp](../verify/data-structure/kd-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
- 関連ローカルテスト: [tests/random/static-tree-expansion.cpp](../tests/random/static-tree-expansion.cpp)

## Li Chao Tree

[blueberry/data-structure/li-chao-tree.hpp](../blueberry/data-structure/li-chao-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/line-add-get-min.test.cpp](../verify/data-structure/line-add-get-min.test.cpp) / [問題](https://judge.yosupo.jp/problem/line_add_get_min)
- 専用ドライバ: [verify/data-structure/segment-add-get-min.test.cpp](../verify/data-structure/segment-add-get-min.test.cpp) / [問題](https://judge.yosupo.jp/problem/segment_add_get_min)
- 関連ローカルテスト: [tests/random/li-chao-tree.cpp](../tests/random/li-chao-tree.cpp)

## Linear RMQ

[blueberry/data-structure/linear-rmq.hpp](../blueberry/data-structure/linear-rmq.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/linear-rmq.test.cpp](../verify/data-structure/linear-rmq.test.cpp) / [問題](https://judge.yosupo.jp/problem/staticrmq)
- 間接利用: [verify/graph/linear-lca.test.cpp](../verify/graph/linear-lca.test.cpp) / [問題](https://judge.yosupo.jp/problem/lca)
- 関連ローカルテスト: [tests/random/static-tree-expansion.cpp](../tests/random/static-tree-expansion.cpp)

## Offline Fenwick Tree 2D

[blueberry/data-structure/offline-fenwick-tree-2d.hpp](../blueberry/data-structure/offline-fenwick-tree-2d.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/point-add-rectangle-sum.test.cpp](../verify/data-structure/point-add-rectangle-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
- 関連ローカルテスト: [tests/random/offline-fenwick-tree-2d.cpp](../tests/random/offline-fenwick-tree-2d.cpp)

## Ordered Multiset

[blueberry/data-structure/ordered-multiset.hpp](../blueberry/data-structure/ordered-multiset.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/ordered-multiset-composite.test.cpp](../verify/data-structure/ordered-multiset-composite.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_range_composite)
- 専用ドライバ: [verify/data-structure/ordered-multiset-large-composite.test.cpp](../verify/data-structure/ordered-multiset-large-composite.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_range_composite_large_array)
- 専用ドライバ: [verify/data-structure/ordered-multiset-priority-queue.test.cpp](../verify/data-structure/ordered-multiset-priority-queue.test.cpp) / [問題](https://judge.yosupo.jp/problem/double_ended_priority_queue)
- 関連ローカルテスト: [tests/random/legacy-treap.cpp](../tests/random/legacy-treap.cpp)

## Ordered Set

[blueberry/data-structure/ordered-set.hpp](../blueberry/data-structure/ordered-set.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/ordered-set.test.cpp](../verify/data-structure/ordered-set.test.cpp) / [問題](https://judge.yosupo.jp/problem/ordered_set)
- 専用ドライバ: [verify/data-structure/predecessor.test.cpp](../verify/data-structure/predecessor.test.cpp) / [問題](https://judge.yosupo.jp/problem/predecessor_problem)
- 関連ローカルテスト: [tests/random/data-structure-expansion.cpp](../tests/random/data-structure-expansion.cpp)

## Persistent Array

[blueberry/data-structure/persistent-array.hpp](../blueberry/data-structure/persistent-array.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/persistent-array.test.cpp](../verify/data-structure/persistent-array.test.cpp) / [問題](https://judge.yosupo.jp/problem/persistent_queue)
- 間接利用: [verify/data-structure/online-persistent-unionfind.test.cpp](../verify/data-structure/online-persistent-unionfind.test.cpp) / [問題](https://judge.yosupo.jp/problem/persistent_unionfind)
- 関連ローカルテスト: [tests/random/persistence-and-range-union.cpp](../tests/random/persistence-and-range-union.cpp)

## Persistent Binary Trie

[blueberry/data-structure/persistent-binary-trie.hpp](../blueberry/data-structure/persistent-binary-trie.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/persistent-binary-trie.test.cpp](../verify/data-structure/persistent-binary-trie.test.cpp) / [問題](https://judge.yosupo.jp/problem/set_xor_min)
- 関連ローカルテスト: [tests/random/persistent-binary-trie.cpp](../tests/random/persistent-binary-trie.cpp)

## Persistent Queue

[blueberry/data-structure/persistent-queue.hpp](../blueberry/data-structure/persistent-queue.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/persistent-queue.test.cpp](../verify/data-structure/persistent-queue.test.cpp) / [問題](https://judge.yosupo.jp/problem/persistent_queue)
- 関連ローカルテスト: [tests/random/ds_batch_three.cpp](../tests/random/ds_batch_three.cpp)

## Persistent Segment Tree

[blueberry/data-structure/persistent-segment-tree.hpp](../blueberry/data-structure/persistent-segment-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/persistent-point-set-range-composite.test.cpp](../verify/data-structure/persistent-point-set-range-composite.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_range_composite)
- 専用ドライバ: [verify/data-structure/persistent-rectangle-sum.test.cpp](../verify/data-structure/persistent-rectangle-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/rectangle_sum)
- 関連ローカルテスト: [tests/random/data-structure-expansion.cpp](../tests/random/data-structure-expansion.cpp)

## Persistent Union Find

[blueberry/data-structure/persistent-union-find.hpp](../blueberry/data-structure/persistent-union-find.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/online-persistent-unionfind.test.cpp](../verify/data-structure/online-persistent-unionfind.test.cpp) / [問題](https://judge.yosupo.jp/problem/persistent_unionfind)
- 関連ローカルテスト: [tests/random/persistence-and-range-union.cpp](../tests/random/persistence-and-range-union.cpp)

## Potential Union Find

[blueberry/data-structure/potential-union-find.hpp](../blueberry/data-structure/potential-union-find.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/unionfind-with-potential-non-commutative-group.test.cpp](../verify/data-structure/unionfind-with-potential-non-commutative-group.test.cpp) / [問題](https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group)
- 専用ドライバ: [verify/data-structure/unionfind-with-potential.test.cpp](../verify/data-structure/unionfind-with-potential.test.cpp) / [問題](https://judge.yosupo.jp/problem/unionfind_with_potential)
- 関連ローカルテスト: [tests/random/data-structure-expansion.cpp](../tests/random/data-structure-expansion.cpp)

## Range Parallel Union Find

[blueberry/data-structure/range-parallel-union-find.hpp](../blueberry/data-structure/range-parallel-union-find.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/range-parallel-unionfind.test.cpp](../verify/data-structure/range-parallel-unionfind.test.cpp) / [問題](https://judge.yosupo.jp/problem/range_parallel_unionfind)
- 関連ローカルテスト: [tests/random/persistence-and-range-union.cpp](../tests/random/persistence-and-range-union.cpp)

## Range Sort Range Product

[blueberry/data-structure/range-sort-range-product.hpp](../blueberry/data-structure/range-sort-range-product.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/range-sort-range-product.test.cpp](../verify/data-structure/range-sort-range-product.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_range_sort_range_composite)
- 関連ローカルテスト: [tests/random/range-sort-range-product.cpp](../tests/random/range-sort-range-product.cpp)

## Rectangle Union Area

[blueberry/data-structure/rectangle-union.hpp](../blueberry/data-structure/rectangle-union.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/rectangle-union.test.cpp](../verify/data-structure/rectangle-union.test.cpp) / [問題](https://judge.yosupo.jp/problem/area_of_union_of_rectangles)
- 関連ローカルテスト: [tests/random/ds_batch_three.cpp](../tests/random/ds_batch_three.cpp)

## Rollback Union Find

[blueberry/data-structure/rollback-union-find.hpp](../blueberry/data-structure/rollback-union-find.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/persistent-unionfind.test.cpp](../verify/data-structure/persistent-unionfind.test.cpp) / [問題](https://judge.yosupo.jp/problem/persistent_unionfind)
- 関連ローカルテスト: [tests/data_structure_random_test.cpp](../tests/data_structure_random_test.cpp)

## Segment Tree Beats

[blueberry/data-structure/segment-tree-beats.hpp](../blueberry/data-structure/segment-tree-beats.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/segment-tree-beats.test.cpp](../verify/data-structure/segment-tree-beats.test.cpp) / [問題](https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum)
- 関連ローカルテスト: [tests/random/operation-structures.cpp](../tests/random/operation-structures.cpp)

## Segment Tree

[blueberry/data-structure/segment-tree.hpp](../blueberry/data-structure/segment-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/point-set-range-composite.test.cpp](../verify/data-structure/point-set-range-composite.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_range_composite)
- 関連ローカルテスト: [tests/data_structure_random_test.cpp](../tests/data_structure_random_test.cpp)

## Skew Heap

[blueberry/data-structure/skew-heap.hpp](../blueberry/data-structure/skew-heap.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/skew-heap.test.cpp](../verify/data-structure/skew-heap.test.cpp) / [問題](https://judge.yosupo.jp/problem/shortest_path)
- 関連ローカルテスト: [tests/random/specialized-structures.cpp](../tests/random/specialized-structures.cpp)
- 最短路ドライバで優先度キューとmeldを使用します。meld専用の公式問題は未登録です。
- 最短路の利用例だけでは、空ヒープ・自己meld・所有権移動・全てのmeld順を網羅しません。
- 複数ヒープをmultisetの愚直解と比較するローカルテストも併用する。
- [追跡Issue](https://github.com/blueberry1001/Blueberry-library/issues/25)

## Sparse Table

[blueberry/data-structure/sparse-table.hpp](../blueberry/data-structure/sparse-table.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/static-rmq.test.cpp](../verify/data-structure/static-rmq.test.cpp) / [問題](https://judge.yosupo.jp/problem/staticrmq)
- 間接利用: [verify/graph/lowest-common-ancestor-rmq.test.cpp](../verify/graph/lowest-common-ancestor-rmq.test.cpp) / [問題](https://judge.yosupo.jp/problem/lca)
- 間接利用: [verify/graph/lowest-common-ancestor.test.cpp](../verify/graph/lowest-common-ancestor.test.cpp) / [問題](https://judge.yosupo.jp/problem/lca)
- 関連ローカルテスト: [tests/data_structure_random_test.cpp](../tests/data_structure_random_test.cpp)
- 関連ローカルテスト: [tests/tree_random.cpp](../tests/tree_random.cpp)

## Splay Slope Trick

[blueberry/data-structure/splay-slope-trick.hpp](../blueberry/data-structure/splay-slope-trick.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/splay-slope-trick.test.cpp](../verify/data-structure/splay-slope-trick.test.cpp) / [問題](https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex)
- 関連ローカルテスト: [tests/random/slope-tricks.cpp](../tests/random/slope-tricks.cpp)

## Sqrt Tree（静的）

[blueberry/data-structure/sqrt-tree.hpp](../blueberry/data-structure/sqrt-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/sqrt-tree-rmq.test.cpp](../verify/data-structure/sqrt-tree-rmq.test.cpp) / [問題](https://judge.yosupo.jp/problem/staticrmq)
- 専用ドライバ: [verify/data-structure/sqrt-tree.test.cpp](../verify/data-structure/sqrt-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_sum)
- 関連ローカルテスト: [tests/random/sqrt-tree.cpp](../tests/random/sqrt-tree.cpp)

## Static Range Distinct

[blueberry/data-structure/static-range-distinct.hpp](../blueberry/data-structure/static-range-distinct.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/static-range-distinct.test.cpp](../verify/data-structure/static-range-distinct.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_count_distinct)
- 関連ローカルテスト: [tests/random/ds_batch_three.cpp](../tests/random/ds_batch_three.cpp)

## Static Range Inversions

[blueberry/data-structure/static-range-inversions.hpp](../blueberry/data-structure/static-range-inversions.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/static-range-inversions.test.cpp](../verify/data-structure/static-range-inversions.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_inversions_query)
- 関連ローカルテスト: [tests/random/sequence-batch-four.cpp](../tests/random/sequence-batch-four.cpp)

## Static Range LIS

[blueberry/data-structure/static-range-lis.hpp](../blueberry/data-structure/static-range-lis.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/static-range-lis.test.cpp](../verify/data-structure/static-range-lis.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_lis_query)
- 関連ローカルテスト: [tests/random/static-range-lis.cpp](../tests/random/static-range-lis.cpp)

## Static Range Mode

[blueberry/data-structure/static-range-mode.hpp](../blueberry/data-structure/static-range-mode.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/static-range-mode.test.cpp](../verify/data-structure/static-range-mode.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_mode_query)
- 関連ローカルテスト: [tests/random/sequence-batch-four.cpp](../tests/random/sequence-batch-four.cpp)

## Wavelet Matrix

[blueberry/data-structure/wavelet-matrix.hpp](../blueberry/data-structure/wavelet-matrix.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/range-kth-smallest.test.cpp](../verify/data-structure/range-kth-smallest.test.cpp) / [問題](https://judge.yosupo.jp/problem/range_kth_smallest)
- 専用ドライバ: [verify/data-structure/static-range-frequency.test.cpp](../verify/data-structure/static-range-frequency.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_frequency)
- 間接利用: [verify/data-structure/static-range-distinct.test.cpp](../verify/data-structure/static-range-distinct.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_count_distinct)
- 間接利用: [verify/data-structure/static-range-lis.test.cpp](../verify/data-structure/static-range-lis.test.cpp) / [問題](https://judge.yosupo.jp/problem/static_range_lis_query)
- 関連ローカルテスト: [tests/random/ds_batch_three.cpp](../tests/random/ds_batch_three.cpp)
- 関連ローカルテスト: [tests/random/static-range-lis.cpp](../tests/random/static-range-lis.cpp)
- 関連ローカルテスト: [tests/random/wavelet-matrix.cpp](../tests/random/wavelet-matrix.cpp)

## Weighted Wavelet Matrix

[blueberry/data-structure/weighted-wavelet-matrix.hpp](../blueberry/data-structure/weighted-wavelet-matrix.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/data-structure/weighted-wavelet-matrix.test.cpp](../verify/data-structure/weighted-wavelet-matrix.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
- 関連ローカルテスト: [tests/random/weighted-wavelet-matrix.cpp](../tests/random/weighted-wavelet-matrix.cpp)

## Assignment

[blueberry/graph/assignment.hpp](../blueberry/graph/assignment.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/assignment.test.cpp](../verify/graph/assignment.test.cpp) / [問題](https://judge.yosupo.jp/problem/assignment)
- 関連ローカルテスト: [tests/random/graph_batch_three.cpp](../tests/random/graph_batch_three.cpp)

## Biconnected Components

[blueberry/graph/biconnected-components.hpp](../blueberry/graph/biconnected-components.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/biconnected-components.test.cpp](../verify/graph/biconnected-components.test.cpp) / [問題](https://judge.yosupo.jp/problem/biconnected_components)
- 関連ローカルテスト: [tests/random/graph-expansion.cpp](../tests/random/graph-expansion.cpp)

## Cartesian Tree

[blueberry/graph/cartesian-tree.hpp](../blueberry/graph/cartesian-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/cartesian-tree.test.cpp](../verify/graph/cartesian-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/cartesian_tree)
- 関連ローカルテスト: [tests/random/sequence-batch-two.cpp](../tests/random/sequence-batch-two.cpp)

## Centroid Decomposition

[blueberry/graph/centroid-decomposition.hpp](../blueberry/graph/centroid-decomposition.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/centroid-decomposition-thirds.test.cpp](../verify/graph/centroid-decomposition-thirds.test.cpp) / [問題](https://judge.yosupo.jp/problem/frequency_table_of_tree_distance)
- 間接利用: [verify/graph/contour-add.test.cpp](../verify/graph/contour-add.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_get_range_contour_add_on_tree)
- 間接利用: [verify/graph/contour-query.test.cpp](../verify/graph/contour-query.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree)
- 関連ローカルテスト: [tests/random/static-tree-expansion.cpp](../tests/random/static-tree-expansion.cpp)

## Clique Enumeration

[blueberry/graph/clique-enumeration.hpp](../blueberry/graph/clique-enumeration.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/clique-enumeration.test.cpp](../verify/graph/clique-enumeration.test.cpp) / [問題](https://judge.yosupo.jp/problem/enumerate_cliques)
- 関連ローカルテスト: [tests/random/graph-batch-four.cpp](../tests/random/graph-batch-four.cpp)

## Complement Components

[blueberry/graph/complement-components.hpp](../blueberry/graph/complement-components.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/complement-components.test.cpp](../verify/graph/complement-components.test.cpp) / [問題](https://judge.yosupo.jp/problem/connected_components_of_complement_graph)
- 関連ローカルテスト: [tests/random/graph_batch_three.cpp](../tests/random/graph_batch_three.cpp)

## Contour Query

[blueberry/graph/contour-query.hpp](../blueberry/graph/contour-query.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/contour-add.test.cpp](../verify/graph/contour-add.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_get_range_contour_add_on_tree)
- 専用ドライバ: [verify/graph/contour-query.test.cpp](../verify/graph/contour-query.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree)
- 関連ローカルテスト: [tests/random/static-tree-expansion.cpp](../tests/random/static-tree-expansion.cpp)

## Count Spanning Trees

[blueberry/graph/count-spanning-trees.hpp](../blueberry/graph/count-spanning-trees.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/count-directed-spanning-trees.test.cpp](../verify/graph/count-directed-spanning-trees.test.cpp) / [問題](https://judge.yosupo.jp/problem/counting_spanning_tree_directed)
- 専用ドライバ: [verify/graph/count-spanning-trees.test.cpp](../verify/graph/count-spanning-trees.test.cpp) / [問題](https://judge.yosupo.jp/problem/counting_spanning_tree_undirected)
- 関連ローカルテスト: [tests/random/graph_batch_three.cpp](../tests/random/graph_batch_three.cpp)

## Cycle Detection

[blueberry/graph/cycle-detection.hpp](../blueberry/graph/cycle-detection.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/cycle-detection-directed.test.cpp](../verify/graph/cycle-detection-directed.test.cpp) / [問題](https://judge.yosupo.jp/problem/cycle_detection)
- 専用ドライバ: [verify/graph/cycle-detection-undirected.test.cpp](../verify/graph/cycle-detection-undirected.test.cpp) / [問題](https://judge.yosupo.jp/problem/cycle_detection_undirected)
- 関連ローカルテスト: [tests/random/graph-batch-two.cpp](../tests/random/graph-batch-two.cpp)

## Dijkstra

[blueberry/graph/dijkstra.hpp](../blueberry/graph/dijkstra.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/shortest-path.test.cpp](../verify/graph/shortest-path.test.cpp) / [問題](https://judge.yosupo.jp/problem/shortest_path)
- 関連ローカルテスト: [tests/random/dijkstra.cpp](../tests/random/dijkstra.cpp)

## Dynamic Top Tree

[blueberry/graph/dynamic-top-tree.hpp](../blueberry/graph/dynamic-top-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/dynamic-top-tree-subtree.test.cpp](../verify/graph/dynamic-top-tree-subtree.test.cpp) / [問題](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum)
- 専用ドライバ: [verify/graph/dynamic-top-tree.test.cpp](../verify/graph/dynamic-top-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum)
- 関連ローカルテスト: [tests/random/dynamic-top-tree.cpp](../tests/random/dynamic-top-tree.cpp)

## Euler Tour Tree

[blueberry/graph/euler-tour-tree.hpp](../blueberry/graph/euler-tour-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/euler-tour-tree-vertex-add.test.cpp](../verify/graph/euler-tour-tree-vertex-add.test.cpp) / [問題](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum)
- 専用ドライバ: [verify/graph/euler-tour-tree.test.cpp](../verify/graph/euler-tour-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/dynamic_tree_subtree_add_subtree_sum)
- 関連ローカルテスト: [tests/random/euler-tour-tree.cpp](../tests/random/euler-tour-tree.cpp)

## Eulerian Trail

[blueberry/graph/eulerian-trail.hpp](../blueberry/graph/eulerian-trail.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/eulerian-trail-directed.test.cpp](../verify/graph/eulerian-trail-directed.test.cpp) / [問題](https://judge.yosupo.jp/problem/eulerian_trail_directed)
- 専用ドライバ: [verify/graph/eulerian-trail-undirected.test.cpp](../verify/graph/eulerian-trail-undirected.test.cpp) / [問題](https://judge.yosupo.jp/problem/eulerian_trail_undirected)
- 関連ローカルテスト: [tests/random/graph-expansion.cpp](../tests/random/graph-expansion.cpp)

## Heavy Light Decomposition

[blueberry/graph/heavy-light-decomposition.hpp](../blueberry/graph/heavy-light-decomposition.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/heavy-light-decomposition-noncommutative.test.cpp](../verify/graph/heavy-light-decomposition-noncommutative.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_set_path_composite)
- 専用ドライバ: [verify/graph/heavy-light-decomposition-path.test.cpp](../verify/graph/heavy-light-decomposition-path.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_add_path_sum)
- 専用ドライバ: [verify/graph/heavy-light-decomposition-subtree.test.cpp](../verify/graph/heavy-light-decomposition-subtree.test.cpp) / [問題](https://judge.yosupo.jp/problem/vertex_add_subtree_sum)
- 専用ドライバ: [verify/graph/jump-on-tree.test.cpp](../verify/graph/jump-on-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/jump_on_tree)
- 関連ローカルテスト: [tests/tree_random.cpp](../tests/tree_random.cpp)

## Hopcroft–Karp

[blueberry/graph/hopcroft-karp.hpp](../blueberry/graph/hopcroft-karp.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/bipartite-matching.test.cpp](../verify/graph/bipartite-matching.test.cpp) / [問題](https://judge.yosupo.jp/problem/bipartitematching)
- 関連ローカルテスト: [tests/random/graph-expansion.cpp](../tests/random/graph-expansion.cpp)

## Linear LCA

[blueberry/graph/linear-lca.hpp](../blueberry/graph/linear-lca.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/linear-lca.test.cpp](../verify/graph/linear-lca.test.cpp) / [問題](https://judge.yosupo.jp/problem/lca)
- 関連ローカルテスト: [tests/random/static-tree-expansion.cpp](../tests/random/static-tree-expansion.cpp)

## Link-Cut Tree

[blueberry/graph/link-cut-tree.hpp](../blueberry/graph/link-cut-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/link-cut-tree-composite.test.cpp](../verify/graph/link-cut-tree-composite.test.cpp) / [問題](https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite)
- 専用ドライバ: [verify/graph/link-cut-tree.test.cpp](../verify/graph/link-cut-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum)
- 関連ローカルテスト: [tests/random/link-cut-tree.cpp](../tests/random/link-cut-tree.cpp)

## Low Link

[blueberry/graph/low-link.hpp](../blueberry/graph/low-link.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/two-edge-connected-components.test.cpp](../verify/graph/two-edge-connected-components.test.cpp) / [問題](https://judge.yosupo.jp/problem/two_edge_connected_components)
- 関連ローカルテスト: [tests/random/low-link.cpp](../tests/random/low-link.cpp)

## Lowest Common Ancestor

[blueberry/graph/lowest-common-ancestor.hpp](../blueberry/graph/lowest-common-ancestor.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/lowest-common-ancestor-rmq.test.cpp](../verify/graph/lowest-common-ancestor-rmq.test.cpp) / [問題](https://judge.yosupo.jp/problem/lca)
- 専用ドライバ: [verify/graph/lowest-common-ancestor.test.cpp](../verify/graph/lowest-common-ancestor.test.cpp) / [問題](https://judge.yosupo.jp/problem/lca)
- 関連ローカルテスト: [tests/tree_random.cpp](../tests/tree_random.cpp)

## Maximum Independent Set

[blueberry/graph/maximum-independent-set.hpp](../blueberry/graph/maximum-independent-set.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/maximum-independent-set.test.cpp](../verify/graph/maximum-independent-set.test.cpp) / [問題](https://judge.yosupo.jp/problem/maximum_independent_set)
- 関連ローカルテスト: [tests/random/graph-batch-four.cpp](../tests/random/graph-batch-four.cpp)

## Minimum Spanning Forest

[blueberry/graph/minimum-spanning-forest.hpp](../blueberry/graph/minimum-spanning-forest.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/minimum-spanning-forest.test.cpp](../verify/graph/minimum-spanning-forest.test.cpp) / [問題](https://judge.yosupo.jp/problem/minimum_spanning_tree)
- 関連ローカルテスト: [tests/random/graph_batch_three.cpp](../tests/random/graph_batch_three.cpp)

## Rerooting DP

[blueberry/graph/rerooting.hpp](../blueberry/graph/rerooting.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/tree-path-composite-sum.test.cpp](../verify/graph/tree-path-composite-sum.test.cpp) / [問題](https://judge.yosupo.jp/problem/tree_path_composite_sum)
- 関連ローカルテスト: [tests/random/rerooting.cpp](../tests/random/rerooting.cpp)

## Rooted Tree Isomorphism

[blueberry/graph/rooted-tree-isomorphism.hpp](../blueberry/graph/rooted-tree-isomorphism.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/rooted-tree-isomorphism.test.cpp](../verify/graph/rooted-tree-isomorphism.test.cpp) / [問題](https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification)
- 関連ローカルテスト: [tests/random/graph-batch-four.cpp](../tests/random/graph-batch-four.cpp)

## Static Top Tree

[blueberry/graph/static-top-tree.hpp](../blueberry/graph/static-top-tree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/static-top-tree.test.cpp](../verify/graph/static-top-tree.test.cpp) / [問題](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root)
- 関連ローカルテスト: [tests/random/static-top-tree.cpp](../tests/random/static-top-tree.cpp)

## Tree Diameter

[blueberry/graph/tree-diameter.hpp](../blueberry/graph/tree-diameter.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/tree-diameter.test.cpp](../verify/graph/tree-diameter.test.cpp) / [問題](https://judge.yosupo.jp/problem/tree_diameter)
- 関連ローカルテスト: [tests/random/graph-batch-two.cpp](../tests/random/graph-batch-two.cpp)

## Triangle Enumeration

[blueberry/graph/triangle-enumeration.hpp](../blueberry/graph/triangle-enumeration.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/graph/triangle-enumeration.test.cpp](../verify/graph/triangle-enumeration.test.cpp) / [問題](https://judge.yosupo.jp/problem/enumerate_triangles)
- 関連ローカルテスト: [tests/random/graph-batch-two.cpp](../tests/random/graph-batch-two.cpp)

## Bitwise Convolution

[blueberry/math/bitwise-convolution.hpp](../blueberry/math/bitwise-convolution.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/bitwise-and-convolution.test.cpp](../verify/math/bitwise-and-convolution.test.cpp) / [問題](https://judge.yosupo.jp/problem/bitwise_and_convolution)
- 専用ドライバ: [verify/math/bitwise-or-via-and.test.cpp](../verify/math/bitwise-or-via-and.test.cpp) / [問題](https://judge.yosupo.jp/problem/bitwise_and_convolution)
- 専用ドライバ: [verify/math/bitwise-xor-convolution.test.cpp](../verify/math/bitwise-xor-convolution.test.cpp) / [問題](https://judge.yosupo.jp/problem/bitwise_xor_convolution)
- 関連ローカルテスト: [tests/random/algebra-expansion.cpp](../tests/random/algebra-expansion.cpp)

## Discrete Logarithm

[blueberry/math/discrete-log.hpp](../blueberry/math/discrete-log.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/discrete-log.test.cpp](../verify/math/discrete-log.test.cpp) / [問題](https://judge.yosupo.jp/problem/discrete_logarithm_mod)
- 関連ローカルテスト: [tests/random/math-batch-two.cpp](../tests/random/math-batch-two.cpp)

## Divisor Convolution

[blueberry/math/divisor-convolution.hpp](../blueberry/math/divisor-convolution.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/gcd-convolution.test.cpp](../verify/math/gcd-convolution.test.cpp) / [問題](https://judge.yosupo.jp/problem/gcd_convolution)
- 専用ドライバ: [verify/math/lcm-convolution.test.cpp](../verify/math/lcm-convolution.test.cpp) / [問題](https://judge.yosupo.jp/problem/lcm_convolution)
- 関連ローカルテスト: [tests/random/math-batch-two.cpp](../tests/random/math-batch-two.cpp)

## Enumerate Quotients

[blueberry/math/enumerate-quotients.hpp](../blueberry/math/enumerate-quotients.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/enumerate-quotients.test.cpp](../verify/math/enumerate-quotients.test.cpp) / [問題](https://judge.yosupo.jp/problem/enumerate_quotients)
- 関連ローカルテスト: [tests/random/math-batch-two.cpp](../tests/random/math-batch-two.cpp)

## Factorize

[blueberry/math/factorize.hpp](../blueberry/math/factorize.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/factorize.test.cpp](../verify/math/factorize.test.cpp) / [問題](https://judge.yosupo.jp/problem/factorize)
- 専用ドライバ: [verify/math/primality-test.test.cpp](../verify/math/primality-test.test.cpp) / [問題](https://judge.yosupo.jp/problem/primality_test)
- 関連ローカルテスト: [tests/random/math-expansion.cpp](../tests/random/math-expansion.cpp)

## Formal Power Series

[blueberry/math/formal-power-series.hpp](../blueberry/math/formal-power-series.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/division-of-polynomials.test.cpp](../verify/math/division-of-polynomials.test.cpp) / [問題](https://judge.yosupo.jp/problem/division_of_polynomials)
- 専用ドライバ: [verify/math/exp-formal-power-series.test.cpp](../verify/math/exp-formal-power-series.test.cpp) / [問題](https://judge.yosupo.jp/problem/exp_of_formal_power_series)
- 専用ドライバ: [verify/math/inv-formal-power-series.test.cpp](../verify/math/inv-formal-power-series.test.cpp) / [問題](https://judge.yosupo.jp/problem/inv_of_formal_power_series)
- 専用ドライバ: [verify/math/log-formal-power-series.test.cpp](../verify/math/log-formal-power-series.test.cpp) / [問題](https://judge.yosupo.jp/problem/log_of_formal_power_series)
- 専用ドライバ: [verify/math/pow-formal-power-series.test.cpp](../verify/math/pow-formal-power-series.test.cpp) / [問題](https://judge.yosupo.jp/problem/pow_of_formal_power_series)
- 専用ドライバ: [verify/math/sqrt-formal-power-series.test.cpp](../verify/math/sqrt-formal-power-series.test.cpp) / [問題](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series)
- 関連ローカルテスト: [tests/random/formal-power-series.cpp](../tests/random/formal-power-series.cpp)

## Fraction

[blueberry/math/fraction.hpp](../blueberry/math/fraction.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/fraction-sort-points-by-argument.test.cpp](../verify/math/fraction-sort-points-by-argument.test.cpp) / [問題](https://judge.yosupo.jp/problem/sort_points_by_argument)
- 関連ローカルテスト: [tests/random/fraction.cpp](../tests/random/fraction.cpp)
- 関連ローカルテスト: [tests/random/slope-tricks.cpp](../tests/random/slope-tricks.cpp)

## Kth Root Integer

[blueberry/math/kth-root-integer.hpp](../blueberry/math/kth-root-integer.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/kth-root-integer.test.cpp](../verify/math/kth-root-integer.test.cpp) / [問題](https://judge.yosupo.jp/problem/kth_root_integer)
- 関連ローカルテスト: [tests/random/math-batch-two.cpp](../tests/random/math-batch-two.cpp)

## Linear Recurrence

[blueberry/math/linear-recurrence.hpp](../blueberry/math/linear-recurrence.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/find-linear-recurrence.test.cpp](../verify/math/find-linear-recurrence.test.cpp) / [問題](https://judge.yosupo.jp/problem/find_linear_recurrence)
- 専用ドライバ: [verify/math/kth-term-of-linearly-recurrent-sequence.test.cpp](../verify/math/kth-term-of-linearly-recurrent-sequence.test.cpp) / [問題](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence)
- 関連ローカルテスト: [tests/random/math-expansion.cpp](../tests/random/math-expansion.cpp)

## Longest Increasing Subsequence

[blueberry/math/longest-increasing-subsequence.hpp](../blueberry/math/longest-increasing-subsequence.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/longest-increasing-subsequence.test.cpp](../verify/math/longest-increasing-subsequence.test.cpp) / [問題](https://judge.yosupo.jp/problem/longest_increasing_subsequence)
- 関連ローカルテスト: [tests/random/sequence-batch-two.cpp](../tests/random/sequence-batch-two.cpp)

## Matrix

[blueberry/math/matrix.hpp](../blueberry/math/matrix.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/inverse-matrix.test.cpp](../verify/math/inverse-matrix.test.cpp) / [問題](https://judge.yosupo.jp/problem/inverse_matrix)
- 専用ドライバ: [verify/math/matrix-det.test.cpp](../verify/math/matrix-det.test.cpp) / [問題](https://judge.yosupo.jp/problem/matrix_det)
- 専用ドライバ: [verify/math/matrix-rank.test.cpp](../verify/math/matrix-rank.test.cpp) / [問題](https://judge.yosupo.jp/problem/matrix_rank)
- 専用ドライバ: [verify/math/system-of-linear-equations.test.cpp](../verify/math/system-of-linear-equations.test.cpp) / [問題](https://judge.yosupo.jp/problem/system_of_linear_equations)
- 間接利用: [verify/graph/count-directed-spanning-trees.test.cpp](../verify/graph/count-directed-spanning-trees.test.cpp) / [問題](https://judge.yosupo.jp/problem/counting_spanning_tree_directed)
- 間接利用: [verify/graph/count-spanning-trees.test.cpp](../verify/graph/count-spanning-trees.test.cpp) / [問題](https://judge.yosupo.jp/problem/counting_spanning_tree_undirected)
- 関連ローカルテスト: [tests/random/algebra-expansion.cpp](../tests/random/algebra-expansion.cpp)
- 関連ローカルテスト: [tests/random/graph_batch_three.cpp](../tests/random/graph_batch_three.cpp)

## Modular Square Root

[blueberry/math/mod-sqrt.hpp](../blueberry/math/mod-sqrt.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/mod-sqrt.test.cpp](../verify/math/mod-sqrt.test.cpp) / [問題](https://judge.yosupo.jp/problem/sqrt_mod)
- 関連ローカルテスト: [tests/random/math-expansion.cpp](../tests/random/math-expansion.cpp)

## Montmort Numbers

[blueberry/math/montmort.hpp](../blueberry/math/montmort.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/montmort.test.cpp](../verify/math/montmort.test.cpp) / [問題](https://judge.yosupo.jp/problem/montmort_number_mod)
- 関連ローカルテスト: [tests/random/math_batch_three.cpp](../tests/random/math_batch_three.cpp)

## Multipoint Evaluation

[blueberry/math/multipoint-evaluation.hpp](../blueberry/math/multipoint-evaluation.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/multipoint-evaluation.test.cpp](../verify/math/multipoint-evaluation.test.cpp) / [問題](https://judge.yosupo.jp/problem/multipoint_evaluation)
- 関連ローカルテスト: [tests/random/math-batch-four.cpp](../tests/random/math-batch-four.cpp)

## Polynomial Interpolation

[blueberry/math/polynomial-interpolation.hpp](../blueberry/math/polynomial-interpolation.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/polynomial-interpolation.test.cpp](../verify/math/polynomial-interpolation.test.cpp) / [問題](https://judge.yosupo.jp/problem/polynomial_interpolation)
- 関連ローカルテスト: [tests/random/math-batch-four.cpp](../tests/random/math-batch-four.cpp)

## Polynomial Product

[blueberry/math/polynomial-product.hpp](../blueberry/math/polynomial-product.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/polynomial-product.test.cpp](../verify/math/polynomial-product.test.cpp) / [問題](https://judge.yosupo.jp/problem/product_of_polynomial_sequence)
- 関連ローカルテスト: [tests/random/math_batch_three.cpp](../tests/random/math_batch_three.cpp)

## Prime Sieve

[blueberry/math/prime-sieve.hpp](../blueberry/math/prime-sieve.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/enumerate-primes.test.cpp](../verify/math/enumerate-primes.test.cpp) / [問題](https://judge.yosupo.jp/problem/enumerate_primes)

## Sample Point Shift

[blueberry/math/sample-point-shift.hpp](../blueberry/math/sample-point-shift.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/sample-point-shift.test.cpp](../verify/math/sample-point-shift.test.cpp) / [問題](https://judge.yosupo.jp/problem/shift_of_sampling_points_of_polynomial)
- 関連ローカルテスト: [tests/random/math-batch-four.cpp](../tests/random/math-batch-four.cpp)

## Stirling Numbers Second Kind

[blueberry/math/stirling-second.hpp](../blueberry/math/stirling-second.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/stirling-second.test.cpp](../verify/math/stirling-second.test.cpp) / [問題](https://judge.yosupo.jp/problem/stirling_number_of_the_second_kind)
- 関連ローカルテスト: [tests/random/math_batch_three.cpp](../tests/random/math_batch_three.cpp)

## Subset Convolution

[blueberry/math/subset-convolution.hpp](../blueberry/math/subset-convolution.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/subset-convolution.test.cpp](../verify/math/subset-convolution.test.cpp) / [問題](https://judge.yosupo.jp/problem/subset_convolution)
- 関連ローカルテスト: [tests/random/algebra-expansion.cpp](../tests/random/algebra-expansion.cpp)

## Polynomial Taylor Shift

[blueberry/math/taylor-shift.hpp](../blueberry/math/taylor-shift.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/math/taylor-shift.test.cpp](../verify/math/taylor-shift.test.cpp) / [問題](https://judge.yosupo.jp/problem/polynomial_taylor_shift)
- 関連ローカルテスト: [tests/random/math_batch_three.cpp](../tests/random/math_batch_three.cpp)

## Aho–Corasick

[blueberry/string/aho-corasick.hpp](../blueberry/string/aho-corasick.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/aho-corasick.test.cpp](../verify/string/aho-corasick.test.cpp) / [問題](https://judge.yosupo.jp/problem/aho_corasick)
- 関連ローカルテスト: [tests/random/string-expansion.cpp](../tests/random/string-expansion.cpp)

## Count Subsequences

[blueberry/string/count-subsequences.hpp](../blueberry/string/count-subsequences.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/count-subsequences.test.cpp](../verify/string/count-subsequences.test.cpp) / [問題](https://judge.yosupo.jp/problem/number_of_subsequences)
- 関連ローカルテスト: [tests/random/count_subsequences.cpp](../tests/random/count_subsequences.cpp)

## Eertree

[blueberry/string/eertree.hpp](../blueberry/string/eertree.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/eertree.test.cpp](../verify/string/eertree.test.cpp) / [問題](https://judge.yosupo.jp/problem/eertree)
- 関連ローカルテスト: [tests/random/string-expansion.cpp](../tests/random/string-expansion.cpp)

## Longest Common Substring

[blueberry/string/longest-common-substring.hpp](../blueberry/string/longest-common-substring.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/longest-common-substring.test.cpp](../verify/string/longest-common-substring.test.cpp) / [問題](https://judge.yosupo.jp/problem/longest_common_substring)
- 関連ローカルテスト: [tests/random/sequence-batch-four.cpp](../tests/random/sequence-batch-four.cpp)

## Lyndon Factorization

[blueberry/string/lyndon-factorization.hpp](../blueberry/string/lyndon-factorization.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/lyndon-factorization.test.cpp](../verify/string/lyndon-factorization.test.cpp) / [問題](https://judge.yosupo.jp/problem/lyndon_factorization)
- 関連ローカルテスト: [tests/random/string-expansion.cpp](../tests/random/string-expansion.cpp)

## Manacher

[blueberry/string/manacher.hpp](../blueberry/string/manacher.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/enumerate-palindromes.test.cpp](../verify/string/enumerate-palindromes.test.cpp) / [問題](https://judge.yosupo.jp/problem/enumerate_palindromes)
- 関連ローカルテスト: [tests/random/manacher.cpp](../tests/random/manacher.cpp)

## Prefix Function / KMP

[blueberry/string/prefix-function.hpp](../blueberry/string/prefix-function.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/kmp-search.test.cpp](../verify/string/kmp-search.test.cpp) / [問題](https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_14_B)
- 関連ローカルテスト: [tests/random/sequence-batch-two.cpp](../tests/random/sequence-batch-two.cpp)

## Suffix Automaton

[blueberry/string/suffix-automaton.hpp](../blueberry/string/suffix-automaton.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/suffix-automaton-lcs.test.cpp](../verify/string/suffix-automaton-lcs.test.cpp) / [問題](https://judge.yosupo.jp/problem/longest_common_substring)
- 専用ドライバ: [verify/string/suffix-automaton-substrings.test.cpp](../verify/string/suffix-automaton-substrings.test.cpp) / [問題](https://judge.yosupo.jp/problem/number_of_substrings)
- 関連ローカルテスト: [tests/random/suffix-automaton.cpp](../tests/random/suffix-automaton.cpp)

## Z Algorithm

[blueberry/string/z-algorithm.hpp](../blueberry/string/z-algorithm.hpp) — **専用verifyドライバあり**

- 専用ドライバ: [verify/string/z-algorithm.test.cpp](../verify/string/z-algorithm.test.cpp) / [問題](https://judge.yosupo.jp/problem/zalgorithm)
