# Library Checker 全問題チェックリスト

このファイルは `python3 scripts/library_checker_coverage.py` で生成します。直接編集しません。

公式公開問題一覧: 2026-09-18T02:28:49.842610+00:00 / **253 問題**。
[公式一覧API](https://v3.api.judge.yosupo.jp/problems) / [カテゴリの固定リビジョン](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/categories.toml)

チェック済みは対応するローカル実装があることを示します。verifyありはドライバの存在であり、ACや現在の実装での成功を意味しません。
実行結果は各verifyページと[測定一覧](https://blueberry1001.github.io/Blueberry-library/benchmarks.html)で確認してください。
ACL推奨は外部ライブラリです。Blueberryの実装数・チェック済み数には含めません。

実装＋verifyあり: 94 / 実装あり・専用verifyなし: 0 / ACL推奨: 9 / 補助verifyのみ: 1 / 未対応: 149

## Sample

- [ ] [A + B](https://judge.yosupo.jp/problem/aplusb) (`aplusb`): 未対応
- [ ] [Many A + B](https://judge.yosupo.jp/problem/many_aplusb) (`many_aplusb`): 補助verifyのみ — [verify](../verify/utility/all-headers.test.cpp)
- [ ] [Many A + B (128 bit)](https://judge.yosupo.jp/problem/many_aplusb_128bit) (`many_aplusb_128bit`): 未対応

## Data Structure

- [x] [Area of Union of Rectangles](https://judge.yosupo.jp/problem/area_of_union_of_rectangles) (`area_of_union_of_rectangles`): 実装＋verifyあり — [Rectangle Union Area](../blueberry/data-structure/rectangle-union.hpp), [verify](../verify/data-structure/rectangle-union.test.cpp)
- [ ] [Associative Array](https://judge.yosupo.jp/problem/associative_array) (`associative_array`): 未対応
- [x] [Deque Operate All Composite](https://judge.yosupo.jp/problem/deque_operate_all_composite) (`deque_operate_all_composite`): 実装＋verifyあり — [Aggregate Deque](../blueberry/data-structure/aggregate-deque.hpp), [verify](../verify/data-structure/aggregate-deque.test.cpp)
- [x] [Double-Ended Priority Queue](https://judge.yosupo.jp/problem/double_ended_priority_queue) (`double_ended_priority_queue`): 実装＋verifyあり — [Ordered Multiset](../blueberry/data-structure/ordered-multiset.hpp), [verify](../verify/data-structure/ordered-multiset-priority-queue.test.cpp)
- [ ] [Dynamic Point Rectangle Affine Rectangle Sum](https://judge.yosupo.jp/problem/dynamic_point_set_rectangle_affine_rectangle_sum) (`dynamic_point_set_rectangle_affine_rectangle_sum`): 未対応
- [x] [Dynamic Sequence Range Affine Range Sum](https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum) (`dynamic_sequence_range_affine_range_sum`): 実装＋verifyあり — [Implicit Treap](../blueberry/data-structure/implicit-treap.hpp), [verify](../verify/data-structure/implicit-treap-affine.test.cpp)
- [x] [Line Add Get Min](https://judge.yosupo.jp/problem/line_add_get_min) (`line_add_get_min`): 実装＋verifyあり — [Dynamic Li Chao Tree](../blueberry/data-structure/dynamic-li-chao-tree.hpp), [Li Chao Tree](../blueberry/data-structure/li-chao-tree.hpp), [verify](../verify/data-structure/dynamic-line-add-get-min.test.cpp), [verify](../verify/data-structure/line-add-get-min.test.cpp)
- [x] [Ordered Set](https://judge.yosupo.jp/problem/ordered_set) (`ordered_set`): 実装＋verifyあり — [Ordered Set](../blueberry/data-structure/ordered-set.hpp), [verify](../verify/data-structure/ordered-set.test.cpp)
- [x] [Persistent Queue](https://judge.yosupo.jp/problem/persistent_queue) (`persistent_queue`): 実装＋verifyあり — [Persistent Queue](../blueberry/data-structure/persistent-queue.hpp), [verify](../verify/data-structure/persistent-queue.test.cpp)
- [ ] [Persistent Range Affine Range Sum](https://judge.yosupo.jp/problem/persistent_range_affine_range_sum) (`persistent_range_affine_range_sum`): 未対応
- [x] [Persistent Unionfind](https://judge.yosupo.jp/problem/persistent_unionfind) (`persistent_unionfind`): 実装＋verifyあり — [Rollback Union Find](../blueberry/data-structure/rollback-union-find.hpp), [verify](../verify/data-structure/persistent-unionfind.test.cpp)
- [x] [Point Add Range Sum](https://judge.yosupo.jp/problem/point_add_range_sum) (`point_add_range_sum`): 実装＋verifyあり — [Dynamic Fenwick Tree](../blueberry/data-structure/dynamic-fenwick-tree.hpp), [Fenwick Tree](../blueberry/data-structure/fenwick-tree.hpp), [verify](../verify/data-structure/dynamic-point-add-range-sum.test.cpp), [verify](../verify/data-structure/point-add-range-sum.test.cpp)
- [x] [Point Add Rectangle Sum](https://judge.yosupo.jp/problem/point_add_rectangle_sum) (`point_add_rectangle_sum`): 実装＋verifyあり — [Dynamic Fenwick Tree 2D](../blueberry/data-structure/dynamic-fenwick-tree-2d.hpp), [Fenwick Tree](../blueberry/data-structure/fenwick-tree.hpp), [Offline Fenwick Tree 2D](../blueberry/data-structure/offline-fenwick-tree-2d.hpp), [Weighted Wavelet Matrix](../blueberry/data-structure/weighted-wavelet-matrix.hpp), [verify](../verify/data-structure/dynamic-point-add-rectangle-sum.test.cpp), [verify](../verify/data-structure/point-add-rectangle-sum.test.cpp), [verify](../verify/data-structure/weighted-wavelet-matrix.test.cpp)
- [x] [Point Set Range Composite](https://judge.yosupo.jp/problem/point_set_range_composite) (`point_set_range_composite`): 実装＋verifyあり — [Ordered Multiset](../blueberry/data-structure/ordered-multiset.hpp), [Persistent Segment Tree](../blueberry/data-structure/persistent-segment-tree.hpp), [Segment Tree](../blueberry/data-structure/segment-tree.hpp), [verify](../verify/data-structure/ordered-multiset-composite.test.cpp), [verify](../verify/data-structure/persistent-point-set-range-composite.test.cpp), [verify](../verify/data-structure/point-set-range-composite.test.cpp)
- [x] [Point Set Range Composite (Large Array)](https://judge.yosupo.jp/problem/point_set_range_composite_large_array) (`point_set_range_composite_large_array`): 実装＋verifyあり — [Ordered Multiset](../blueberry/data-structure/ordered-multiset.hpp), [verify](../verify/data-structure/ordered-multiset-large-composite.test.cpp) — 64bit座標をキーに更新済みの関数のみ保存する。更新は同座標をeraseしてinsert、問い合わせはprod(rank(l),rank(r))。未更新点の恒等関数を省略する専用verifyを追加。
- [ ] [Point Set Range Frequency](https://judge.yosupo.jp/problem/point_set_range_frequency) (`point_set_range_frequency`): 未対応
- [ ] [Point Set Range Sort Range Composite](https://judge.yosupo.jp/problem/point_set_range_sort_range_composite) (`point_set_range_sort_range_composite`): 未対応
- [x] [Predecessor Problem](https://judge.yosupo.jp/problem/predecessor_problem) (`predecessor_problem`): 実装＋verifyあり — [Ordered Set](../blueberry/data-structure/ordered-set.hpp), [verify](../verify/data-structure/predecessor.test.cpp) — insert/erase/contains/lower_bound/floorを専用verifyで検証。
- [x] [Queue Operate All Composite](https://judge.yosupo.jp/problem/queue_operate_all_composite) (`queue_operate_all_composite`): 実装＋verifyあり — [Aggregate Queue](../blueberry/data-structure/aggregate-queue.hpp), [verify](../verify/data-structure/aggregate-queue.test.cpp)
- [ ] [Range Affine Point Get](https://judge.yosupo.jp/problem/range_affine_point_get) (`range_affine_point_get`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html) — ACL lazy_segtreeのapply(l,r,f)とget(p)で対応する。
- [ ] [Range Affine Range Sum](https://judge.yosupo.jp/problem/range_affine_range_sum) (`range_affine_range_sum`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html) — 区間長をSに含めるACL lazy_segtreeで対応する。固定長配列では動的列Treapよりこちらを第一候補にする。
- [ ] [Range Affine Range Sum (Large Array)](https://judge.yosupo.jp/problem/range_affine_range_sum_large_array) (`range_affine_range_sum_large_array`): 未対応
- [x] [Range Chmin Chmax Add Range Sum](https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum) (`range_chmin_chmax_add_range_sum`): 実装＋verifyあり — [Segment Tree Beats](../blueberry/data-structure/segment-tree-beats.hpp), [verify](../verify/data-structure/segment-tree-beats.test.cpp)
- [x] [Range Kth Smallest](https://judge.yosupo.jp/problem/range_kth_smallest) (`range_kth_smallest`): 実装＋verifyあり — [Wavelet Matrix](../blueberry/data-structure/wavelet-matrix.hpp), [verify](../verify/data-structure/range-kth-smallest.test.cpp)
- [ ] [Range Linear Add Range Min](https://judge.yosupo.jp/problem/range_linear_add_range_min) (`range_linear_add_range_min`): 未対応
- [ ] [Range Parallel Unionfind](https://judge.yosupo.jp/problem/range_parallel_unionfind) (`range_parallel_unionfind`): 未対応
- [x] [Range Reverse Range Sum](https://judge.yosupo.jp/problem/range_reverse_range_sum) (`range_reverse_range_sum`): 実装＋verifyあり — [Implicit Treap](../blueberry/data-structure/implicit-treap.hpp), [verify](../verify/data-structure/implicit-treap-reverse-sum.test.cpp)
- [ ] [Range Set Range Composite](https://judge.yosupo.jp/problem/range_set_range_composite) (`range_set_range_composite`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html) — ACL lazy_segtreeに区間長と同一関数の反復合成を持つ作用を定義する。二分累乗で反復合成する実装はmappingがO(log N)、更新がO(log² N)。a=1も二分累乗なら安全に扱える。専用verify未追加。
- [ ] [Rectangle Add Point Get](https://judge.yosupo.jp/problem/rectangle_add_point_get) (`rectangle_add_point_get`): 未対応
- [x] [Rectangle Sum](https://judge.yosupo.jp/problem/rectangle_sum) (`rectangle_sum`): 実装＋verifyあり — [Dynamic Fenwick Tree 2D](../blueberry/data-structure/dynamic-fenwick-tree-2d.hpp), [Persistent Segment Tree](../blueberry/data-structure/persistent-segment-tree.hpp), [verify](../verify/data-structure/dynamic-rectangle-sum.test.cpp), [verify](../verify/data-structure/persistent-rectangle-sum.test.cpp)
- [x] [Segment Add Get Min](https://judge.yosupo.jp/problem/segment_add_get_min) (`segment_add_get_min`): 実装＋verifyあり — [Dynamic Li Chao Tree](../blueberry/data-structure/dynamic-li-chao-tree.hpp), [Li Chao Tree](../blueberry/data-structure/li-chao-tree.hpp), [verify](../verify/data-structure/dynamic-segment-add-get-min.test.cpp), [verify](../verify/data-structure/segment-add-get-min.test.cpp)
- [x] [Set Xor-Min](https://judge.yosupo.jp/problem/set_xor_min) (`set_xor_min`): 実装＋verifyあり — [Binary Trie](../blueberry/data-structure/binary-trie.hpp), [Persistent Binary Trie](../blueberry/data-structure/persistent-binary-trie.hpp), [verify](../verify/data-structure/binary-trie.test.cpp), [verify](../verify/data-structure/persistent-binary-trie.test.cpp)
- [x] [Static Range Count Distinct](https://judge.yosupo.jp/problem/static_range_count_distinct) (`static_range_count_distinct`): 実装＋verifyあり — [Static Range Distinct](../blueberry/data-structure/static-range-distinct.hpp), [Wavelet Matrix](../blueberry/data-structure/wavelet-matrix.hpp), [verify](../verify/data-structure/static-range-distinct.test.cpp)
- [x] [Static Range Frequency](https://judge.yosupo.jp/problem/static_range_frequency) (`static_range_frequency`): 実装＋verifyあり — [Wavelet Matrix](../blueberry/data-structure/wavelet-matrix.hpp), [verify](../verify/data-structure/static-range-frequency.test.cpp)
- [x] [Static Range Inversions Query](https://judge.yosupo.jp/problem/static_range_inversions_query) (`static_range_inversions_query`): 実装＋verifyあり — [Static Range Inversions](../blueberry/data-structure/static-range-inversions.hpp), [verify](../verify/data-structure/static-range-inversions.test.cpp)
- [ ] [Static Range LIS Query](https://judge.yosupo.jp/problem/static_range_lis_query) (`static_range_lis_query`): 未対応
- [x] [Static Range Mode Query](https://judge.yosupo.jp/problem/static_range_mode_query) (`static_range_mode_query`): 実装＋verifyあり — [Static Range Mode](../blueberry/data-structure/static-range-mode.hpp), [verify](../verify/data-structure/static-range-mode.test.cpp)
- [x] [Static Range Sum](https://judge.yosupo.jp/problem/static_range_sum) (`static_range_sum`): 実装＋verifyあり — [Disjoint Sparse Table](../blueberry/data-structure/disjoint-sparse-table.hpp), [Fenwick Tree](../blueberry/data-structure/fenwick-tree.hpp), [Sqrt Tree](../blueberry/data-structure/sqrt-tree.hpp), [verify](../verify/data-structure/disjoint-sparse-table.test.cpp), [verify](../verify/data-structure/sqrt-tree.test.cpp) — Disjoint Sparse Tableの非冪等な区間積を専用verifyで検証。静的な和だけなら通常の累積和でもO(N)構築・O(1)問い合わせにできる。
- [ ] [Static Rectangle Add Rectangle Sum](https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum) (`static_rectangle_add_rectangle_sum`): 未対応
- [x] [Static RMQ](https://judge.yosupo.jp/problem/staticrmq) (`staticrmq`): 実装＋verifyあり — [Sparse Table](../blueberry/data-structure/sparse-table.hpp), [Sqrt Tree](../blueberry/data-structure/sqrt-tree.hpp), [verify](../verify/data-structure/sqrt-tree-rmq.test.cpp), [verify](../verify/data-structure/static-rmq.test.cpp)
- [x] [Unionfind](https://judge.yosupo.jp/problem/unionfind) (`unionfind`): 実装＋verifyあり — [Disjoint Set Union](../blueberry/data-structure/disjoint-set-union.hpp), [verify](../verify/data-structure/unionfind.test.cpp)
- [x] [Unionfind with Potential](https://judge.yosupo.jp/problem/unionfind_with_potential) (`unionfind_with_potential`): 実装＋verifyあり — [Potential Union Find](../blueberry/data-structure/potential-union-find.hpp), [verify](../verify/data-structure/unionfind-with-potential.test.cpp)
- [x] [Unionfind with Potential (Non-Commutative Group)](https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group) (`unionfind_with_potential_non_commutative_group`): 実装＋verifyあり — [Potential Union Find](../blueberry/data-structure/potential-union-find.hpp), [verify](../verify/data-structure/unionfind-with-potential-non-commutative-group.test.cpp)

## Graph

- [x] [Assignment Problem](https://judge.yosupo.jp/problem/assignment) (`assignment`): 実装＋verifyあり — [Assignment](../blueberry/graph/assignment.hpp), [verify](../verify/graph/assignment.test.cpp)
- [x] [Biconnected Components](https://judge.yosupo.jp/problem/biconnected_components) (`biconnected_components`): 実装＋verifyあり — [Biconnected Components](../blueberry/graph/biconnected-components.hpp), [verify](../verify/graph/biconnected-components.test.cpp)
- [ ] [Edge Coloring of Bipartite Graph](https://judge.yosupo.jp/problem/bipartite_edge_coloring) (`bipartite_edge_coloring`): 未対応
- [x] [Matching on Bipartite Graph](https://judge.yosupo.jp/problem/bipartitematching) (`bipartitematching`): 実装＋verifyあり — [Hopcroft–Karp](../blueberry/graph/hopcroft-karp.hpp), [verify](../verify/graph/bipartite-matching.test.cpp)
- [ ] [Chordal Graph Recognition](https://judge.yosupo.jp/problem/chordal_graph_recognition) (`chordal_graph_recognition`): 未対応
- [ ] [Chromatic Number](https://judge.yosupo.jp/problem/chromatic_number) (`chromatic_number`): 未対応
- [ ] [Chromatic Polynomial](https://judge.yosupo.jp/problem/chromatic_polynomial) (`chromatic_polynomial`): 未対応
- [x] [Connected Components of Complement Graph](https://judge.yosupo.jp/problem/connected_components_of_complement_graph) (`connected_components_of_complement_graph`): 実装＋verifyあり — [Complement Components](../blueberry/graph/complement-components.hpp), [verify](../verify/graph/complement-components.test.cpp)
- [ ] [Counting $C _ 4$'s](https://judge.yosupo.jp/problem/counting_c4) (`counting_c4`): 未対応
- [ ] [Counting Eulerian Circuits](https://judge.yosupo.jp/problem/counting_eulerian_circuits) (`counting_eulerian_circuits`): 未対応
- [x] [Counting Spanning Trees (Directed)](https://judge.yosupo.jp/problem/counting_spanning_tree_directed) (`counting_spanning_tree_directed`): 実装＋verifyあり — [Count Spanning Trees](../blueberry/graph/count-spanning-trees.hpp), [Matrix](../blueberry/math/matrix.hpp), [verify](../verify/graph/count-directed-spanning-trees.test.cpp)
- [x] [Counting Spanning Trees (Undirected)](https://judge.yosupo.jp/problem/counting_spanning_tree_undirected) (`counting_spanning_tree_undirected`): 実装＋verifyあり — [Count Spanning Trees](../blueberry/graph/count-spanning-trees.hpp), [Matrix](../blueberry/math/matrix.hpp), [verify](../verify/graph/count-spanning-trees.test.cpp)
- [x] [Cycle Detection (Directed)](https://judge.yosupo.jp/problem/cycle_detection) (`cycle_detection`): 実装＋verifyあり — [Cycle Detection](../blueberry/graph/cycle-detection.hpp), [verify](../verify/graph/cycle-detection-directed.test.cpp)
- [x] [Cycle Detection (Undirected)](https://judge.yosupo.jp/problem/cycle_detection_undirected) (`cycle_detection_undirected`): 実装＋verifyあり — [Cycle Detection](../blueberry/graph/cycle-detection.hpp), [verify](../verify/graph/cycle-detection-undirected.test.cpp)
- [ ] [Directed MST](https://judge.yosupo.jp/problem/directedmst) (`directedmst`): 未対応
- [ ] [Dominator Tree](https://judge.yosupo.jp/problem/dominatortree) (`dominatortree`): 未対応
- [ ] [Dynamic Graph Vertex Add Component Sum](https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum) (`dynamic_graph_vertex_add_component_sum`): 未対応
- [x] [Enumerate Cliques](https://judge.yosupo.jp/problem/enumerate_cliques) (`enumerate_cliques`): 実装＋verifyあり — [Clique Enumeration](../blueberry/graph/clique-enumeration.hpp), [verify](../verify/graph/clique-enumeration.test.cpp)
- [x] [Enumerate Triangles](https://judge.yosupo.jp/problem/enumerate_triangles) (`enumerate_triangles`): 実装＋verifyあり — [Triangle Enumeration](../blueberry/graph/triangle-enumeration.hpp), [verify](../verify/graph/triangle-enumeration.test.cpp)
- [x] [Eulerian Trail (Directed)](https://judge.yosupo.jp/problem/eulerian_trail_directed) (`eulerian_trail_directed`): 実装＋verifyあり — [Eulerian Trail](../blueberry/graph/eulerian-trail.hpp), [verify](../verify/graph/eulerian-trail-directed.test.cpp)
- [x] [Eulerian Trail (Undirected)](https://judge.yosupo.jp/problem/eulerian_trail_undirected) (`eulerian_trail_undirected`): 実装＋verifyあり — [Eulerian Trail](../blueberry/graph/eulerian-trail.hpp), [verify](../verify/graph/eulerian-trail-undirected.test.cpp)
- [ ] [Matching on General Graph](https://judge.yosupo.jp/problem/general_matching) (`general_matching`): 未対応
- [ ] [General Weighted Matching](https://judge.yosupo.jp/problem/general_weighted_matching) (`general_weighted_matching`): 未対応
- [ ] [Global Minimum Cut of Dynamic Star Augmented Graph](https://judge.yosupo.jp/problem/global_minimum_cut_of_dynamic_star_augmented_graph) (`global_minimum_cut_of_dynamic_star_augmented_graph`): 未対応
- [ ] [Strongly Connected Components (Incremental)](https://judge.yosupo.jp/problem/incremental_scc) (`incremental_scc`): 未対応
- [ ] [K-Shortest Walk](https://judge.yosupo.jp/problem/k_shortest_walk) (`k_shortest_walk`): 未対応
- [x] [Maximum Independent Set](https://judge.yosupo.jp/problem/maximum_independent_set) (`maximum_independent_set`): 実装＋verifyあり — [Maximum Independent Set](../blueberry/graph/maximum-independent-set.hpp), [verify](../verify/graph/maximum-independent-set.test.cpp)
- [ ] [Minimum Cost b-flow](https://judge.yosupo.jp/problem/min_cost_b_flow) (`min_cost_b_flow`): 未対応
- [ ] [Minimum Diameter Spanning Tree](https://judge.yosupo.jp/problem/minimum_diameter_spanning_tree) (`minimum_diameter_spanning_tree`): 未対応
- [x] [Minimum Spanning Tree](https://judge.yosupo.jp/problem/minimum_spanning_tree) (`minimum_spanning_tree`): 実装＋verifyあり — [Minimum Spanning Forest](../blueberry/graph/minimum-spanning-forest.hpp), [verify](../verify/graph/minimum-spanning-forest.test.cpp)
- [ ] [Strongly Connected Components](https://judge.yosupo.jp/problem/scc) (`scc`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/scc.html) — atcoder::scc_graphを利用する。
- [x] [Shortest Path](https://judge.yosupo.jp/problem/shortest_path) (`shortest_path`): 実装＋verifyあり — [Dijkstra](../blueberry/graph/dijkstra.hpp), [verify](../verify/graph/shortest-path.test.cpp)
- [ ] [st-Numbering](https://judge.yosupo.jp/problem/st_numbering) (`st_numbering`): 未対応
- [ ] [Three-Edge-Connected Components](https://judge.yosupo.jp/problem/three_edge_connected_components) (`three_edge_connected_components`): 未対応
- [ ] [Tree Decomposition (Width 2)](https://judge.yosupo.jp/problem/tree_decomposition_width_2) (`tree_decomposition_width_2`): 未対応
- [x] [Two-Edge-Connected Components](https://judge.yosupo.jp/problem/two_edge_connected_components) (`two_edge_connected_components`): 実装＋verifyあり — [Low Link](../blueberry/graph/low-link.hpp), [verify](../verify/graph/two-edge-connected-components.test.cpp)

## Tree

- [x] [Cartesian Tree](https://judge.yosupo.jp/problem/cartesian_tree) (`cartesian_tree`): 実装＋verifyあり — [Cartesian Tree](../blueberry/graph/cartesian-tree.hpp), [verify](../verify/graph/cartesian-tree.test.cpp)
- [ ] [Common Interval Decomposition Tree](https://judge.yosupo.jp/problem/common_interval_decomposition_tree) (`common_interval_decomposition_tree`): 未対応
- [ ] [Dynamic Tree Subtree Add Subtree Sum](https://judge.yosupo.jp/problem/dynamic_tree_subtree_add_subtree_sum) (`dynamic_tree_subtree_add_subtree_sum`): 未対応
- [ ] [Dynamic Tree Vertex Add Path Sum](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum) (`dynamic_tree_vertex_add_path_sum`): 未対応
- [ ] [Dynamic Tree Vertex Add Subtree Sum](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum) (`dynamic_tree_vertex_add_subtree_sum`): 未対応
- [ ] [Dynamic Tree Vertex Set Path Composite](https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite) (`dynamic_tree_vertex_set_path_composite`): 未対応
- [ ] [Frequency Table of Tree Distance](https://judge.yosupo.jp/problem/frequency_table_of_tree_distance) (`frequency_table_of_tree_distance`): 未対応
- [x] [Jump on Tree](https://judge.yosupo.jp/problem/jump_on_tree) (`jump_on_tree`): 実装＋verifyあり — [Heavy Light Decomposition](../blueberry/graph/heavy-light-decomposition.hpp), [verify](../verify/graph/jump-on-tree.test.cpp)
- [x] [Lowest Common Ancestor](https://judge.yosupo.jp/problem/lca) (`lca`): 実装＋verifyあり — [Sparse Table](../blueberry/data-structure/sparse-table.hpp), [Lowest Common Ancestor](../blueberry/graph/lowest-common-ancestor.hpp), [verify](../verify/graph/lowest-common-ancestor-rmq.test.cpp), [verify](../verify/graph/lowest-common-ancestor.test.cpp)
- [ ] [Point Set Tree Path Composite Sum](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum) (`point_set_tree_path_composite_sum`): 未対応
- [ ] [Point Set Tree Path Composite Sum (Fixed Root)](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root) (`point_set_tree_path_composite_sum_fixed_root`): 未対応
- [x] [Rooted Tree Isomorphism Classification](https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification) (`rooted_tree_isomorphism_classification`): 実装＋verifyあり — [Rooted Tree Isomorphism](../blueberry/graph/rooted-tree-isomorphism.hpp), [verify](../verify/graph/rooted-tree-isomorphism.test.cpp)
- [ ] [Rooted Tree Topological Order with Minimum Inversions](https://judge.yosupo.jp/problem/rooted_tree_topological_order_with_minimum_inversions) (`rooted_tree_topological_order_with_minimum_inversions`): 未対応
- [x] [Tree Diameter](https://judge.yosupo.jp/problem/tree_diameter) (`tree_diameter`): 実装＋verifyあり — [Tree Diameter](../blueberry/graph/tree-diameter.hpp), [verify](../verify/graph/tree-diameter.test.cpp)
- [x] [Tree Path Composite Sum](https://judge.yosupo.jp/problem/tree_path_composite_sum) (`tree_path_composite_sum`): 実装＋verifyあり — [Rerooting DP](../blueberry/graph/rerooting.hpp), [verify](../verify/graph/tree-path-composite-sum.test.cpp)
- [x] [Vertex Add Path Sum](https://judge.yosupo.jp/problem/vertex_add_path_sum) (`vertex_add_path_sum`): 実装＋verifyあり — [Heavy Light Decomposition](../blueberry/graph/heavy-light-decomposition.hpp), [verify](../verify/graph/heavy-light-decomposition-path.test.cpp)
- [ ] [Vertex Add Range Contour Sum on Tree](https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree) (`vertex_add_range_contour_sum_on_tree`): 未対応
- [x] [Vertex Add Subtree Sum](https://judge.yosupo.jp/problem/vertex_add_subtree_sum) (`vertex_add_subtree_sum`): 実装＋verifyあり — [Heavy Light Decomposition](../blueberry/graph/heavy-light-decomposition.hpp), [verify](../verify/graph/heavy-light-decomposition-subtree.test.cpp)
- [ ] [Vertex Get Range Contour Add on Tree](https://judge.yosupo.jp/problem/vertex_get_range_contour_add_on_tree) (`vertex_get_range_contour_add_on_tree`): 未対応
- [x] [Vertex Set Path Composite](https://judge.yosupo.jp/problem/vertex_set_path_composite) (`vertex_set_path_composite`): 実装＋verifyあり — [Heavy Light Decomposition](../blueberry/graph/heavy-light-decomposition.hpp), [verify](../verify/graph/heavy-light-decomposition-noncommutative.test.cpp)

## Convolution

- [x] [Bitwise And Convolution](https://judge.yosupo.jp/problem/bitwise_and_convolution) (`bitwise_and_convolution`): 実装＋verifyあり — [Bitwise Convolution](../blueberry/math/bitwise-convolution.hpp), [verify](../verify/math/bitwise-and-convolution.test.cpp), [verify](../verify/math/bitwise-or-via-and.test.cpp)
- [x] [Bitwise Xor Convolution](https://judge.yosupo.jp/problem/bitwise_xor_convolution) (`bitwise_xor_convolution`): 実装＋verifyあり — [Bitwise Convolution](../blueberry/math/bitwise-convolution.hpp), [verify](../verify/math/bitwise-xor-convolution.test.cpp)
- [ ] [Convolution](https://judge.yosupo.jp/problem/convolution_mod) (`convolution_mod`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/convolution.html) — atcoder::convolutionを利用する。Blueberry自作NTTは追加しない。
- [ ] [Convolution (Mod 1,000,000,007)](https://judge.yosupo.jp/problem/convolution_mod_1000000007) (`convolution_mod_1000000007`): 未対応
- [ ] [Convolution (Mod 2^64)](https://judge.yosupo.jp/problem/convolution_mod_2_64) (`convolution_mod_2_64`): 未対応
- [ ] [Convolution (Large)](https://judge.yosupo.jp/problem/convolution_mod_large) (`convolution_mod_large`): 未対応
- [x] [Gcd Convolution](https://judge.yosupo.jp/problem/gcd_convolution) (`gcd_convolution`): 実装＋verifyあり — [Divisor Convolution](../blueberry/math/divisor-convolution.hpp), [verify](../verify/math/gcd-convolution.test.cpp)
- [x] [Lcm Convolution](https://judge.yosupo.jp/problem/lcm_convolution) (`lcm_convolution`): 実装＋verifyあり — [Divisor Convolution](../blueberry/math/divisor-convolution.hpp), [verify](../verify/math/lcm-convolution.test.cpp)
- [ ] [Min Plus Convolution (Concave and Arbitrary)](https://judge.yosupo.jp/problem/min_plus_convolution_concave_arbitrary) (`min_plus_convolution_concave_arbitrary`): 未対応
- [ ] [Min Plus Convolution (Convex and Arbitrary)](https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary) (`min_plus_convolution_convex_arbitrary`): 未対応
- [ ] [Min Plus Convolution (Convex and Convex)](https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex) (`min_plus_convolution_convex_convex`): 未対応
- [ ] [Convolution on the Multiplicative Monoid of $\\mathbb{Z} / 2^N\\mathbb{Z}$](https://judge.yosupo.jp/problem/mul_mod2n_convolution) (`mul_mod2n_convolution`): 未対応
- [ ] [Convolution on the Multiplicative Monoid of $\\mathbb{Z} / P\\mathbb{Z}$](https://judge.yosupo.jp/problem/mul_modp_convolution) (`mul_modp_convolution`): 未対応
- [ ] [Multidimensional Convolution (Truncated)](https://judge.yosupo.jp/problem/multivariate_convolution) (`multivariate_convolution`): 未対応
- [ ] [Multidimensional Convolution (Circular)](https://judge.yosupo.jp/problem/multivariate_convolution_cyclic) (`multivariate_convolution_cyclic`): 未対応

## Number Theory

- [ ] [Bernoulli Number](https://judge.yosupo.jp/problem/bernoulli_number) (`bernoulli_number`): 未対応
- [ ] [Counting Primes](https://judge.yosupo.jp/problem/counting_primes) (`counting_primes`): 未対応
- [ ] [Counting Square-free Integers](https://judge.yosupo.jp/problem/counting_squarefrees) (`counting_squarefrees`): 未対応
- [x] [Discrete Logarithm](https://judge.yosupo.jp/problem/discrete_logarithm_mod) (`discrete_logarithm_mod`): 実装＋verifyあり — [Discrete Logarithm](../blueberry/math/discrete-log.hpp), [verify](../verify/math/discrete-log.test.cpp)
- [x] [Enumerate Primes](https://judge.yosupo.jp/problem/enumerate_primes) (`enumerate_primes`): 実装＋verifyあり — [Prime Sieve](../blueberry/math/prime-sieve.hpp), [verify](../verify/math/enumerate-primes.test.cpp)
- [x] [Enumerate Quotients](https://judge.yosupo.jp/problem/enumerate_quotients) (`enumerate_quotients`): 実装＋verifyあり — [Enumerate Quotients](../blueberry/math/enumerate-quotients.hpp), [verify](../verify/math/enumerate-quotients.test.cpp)
- [x] [Factorize](https://judge.yosupo.jp/problem/factorize) (`factorize`): 実装＋verifyあり — [Factorize](../blueberry/math/factorize.hpp), [verify](../verify/math/factorize.test.cpp)
- [ ] [Gcd of Gaussian Integers](https://judge.yosupo.jp/problem/gcd_of_gaussian_integers) (`gcd_of_gaussian_integers`): 未対応
- [x] [Kth Root (Integer)](https://judge.yosupo.jp/problem/kth_root_integer) (`kth_root_integer`): 実装＋verifyあり — [Kth Root Integer](../blueberry/math/kth-root-integer.hpp), [verify](../verify/math/kth-root-integer.test.cpp)
- [ ] [Kth Root (Mod)](https://judge.yosupo.jp/problem/kth_root_mod) (`kth_root_mod`): 未対応
- [ ] [Min of Mod of Linear](https://judge.yosupo.jp/problem/min_of_mod_of_linear) (`min_of_mod_of_linear`): 未対応
- [ ] [Nim Product ($\\mathbb{F}_{2^{64}}$)](https://judge.yosupo.jp/problem/nim_product_64) (`nim_product_64`): 未対応
- [x] [Primality Test](https://judge.yosupo.jp/problem/primality_test) (`primality_test`): 実装＋verifyあり — [Factorize](../blueberry/math/factorize.hpp), [verify](../verify/math/primality-test.test.cpp)
- [ ] [Primitive Root](https://judge.yosupo.jp/problem/primitive_root) (`primitive_root`): 未対応
- [ ] [Rational Approximation](https://judge.yosupo.jp/problem/rational_approximation) (`rational_approximation`): 未対応
- [x] [Sqrt Mod](https://judge.yosupo.jp/problem/sqrt_mod) (`sqrt_mod`): 実装＋verifyあり — [Modular Square Root](../blueberry/math/mod-sqrt.hpp), [verify](../verify/math/mod-sqrt.test.cpp)
- [ ] [Stern–Brocot Tree](https://judge.yosupo.jp/problem/stern_brocot_tree) (`stern_brocot_tree`): 未対応
- [ ] [Sum of Floor of Linear](https://judge.yosupo.jp/problem/sum_of_floor_of_linear) (`sum_of_floor_of_linear`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/math.html) — atcoder::floor_sumを利用する。引数の整数範囲を満たすこと。
- [ ] [Sum of Multiplicative Function(Large)](https://judge.yosupo.jp/problem/sum_of_multiplicative_function_large) (`sum_of_multiplicative_function_large`): 未対応
- [ ] [Sum of Totient Function](https://judge.yosupo.jp/problem/sum_of_totient_function) (`sum_of_totient_function`): 未対応
- [ ] [Tetration Mod](https://judge.yosupo.jp/problem/tetration_mod) (`tetration_mod`): 未対応
- [ ] [Represent A Number As Two Square Sum](https://judge.yosupo.jp/problem/two_square_sum) (`two_square_sum`): 未対応

## Polynomial

- [ ] [Composition of Formal Power Series](https://judge.yosupo.jp/problem/composition_of_formal_power_series) (`composition_of_formal_power_series`): 未対応
- [ ] [Composition of Formal Power Series (Large)](https://judge.yosupo.jp/problem/composition_of_formal_power_series_large) (`composition_of_formal_power_series_large`): 未対応
- [ ] [Compositional Inverse of Formal Power Series](https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series) (`compositional_inverse_of_formal_power_series`): 未対応
- [ ] [Compositional Inverse of Formal Power Series (Large)](https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series_large) (`compositional_inverse_of_formal_power_series_large`): 未対応
- [ ] [Conversion from Monomial Basis to Newton Basis](https://judge.yosupo.jp/problem/conversion_from_monomial_basis_to_newton_basis) (`conversion_from_monomial_basis_to_newton_basis`): 未対応
- [x] [Division of Polynomials](https://judge.yosupo.jp/problem/division_of_polynomials) (`division_of_polynomials`): 実装＋verifyあり — [Formal Power Series](../blueberry/math/formal-power-series.hpp), [verify](../verify/math/division-of-polynomials.test.cpp)
- [x] [Exp of Formal Power Series](https://judge.yosupo.jp/problem/exp_of_formal_power_series) (`exp_of_formal_power_series`): 実装＋verifyあり — [Formal Power Series](../blueberry/math/formal-power-series.hpp), [verify](../verify/math/exp-formal-power-series.test.cpp)
- [ ] [Exp of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse) (`exp_of_formal_power_series_sparse`): 未対応
- [ ] [Factorization of Polynomial (Mod)](https://judge.yosupo.jp/problem/factorization_of_polynomials) (`factorization_of_polynomials`): 未対応
- [x] [Inv of Formal Power Series](https://judge.yosupo.jp/problem/inv_of_formal_power_series) (`inv_of_formal_power_series`): 実装＋verifyあり — [Formal Power Series](../blueberry/math/formal-power-series.hpp), [verify](../verify/math/inv-formal-power-series.test.cpp)
- [ ] [Inv of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/inv_of_formal_power_series_sparse) (`inv_of_formal_power_series_sparse`): 未対応
- [ ] [Inv of Polynomials](https://judge.yosupo.jp/problem/inv_of_polynomials) (`inv_of_polynomials`): 未対応
- [x] [Log of Formal Power Series](https://judge.yosupo.jp/problem/log_of_formal_power_series) (`log_of_formal_power_series`): 実装＋verifyあり — [Formal Power Series](../blueberry/math/formal-power-series.hpp), [verify](../verify/math/log-formal-power-series.test.cpp)
- [ ] [Log of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/log_of_formal_power_series_sparse) (`log_of_formal_power_series_sparse`): 未対応
- [x] [Multipoint Evaluation](https://judge.yosupo.jp/problem/multipoint_evaluation) (`multipoint_evaluation`): 実装＋verifyあり — [Multipoint Evaluation](../blueberry/math/multipoint-evaluation.hpp), [verify](../verify/math/multipoint-evaluation.test.cpp)
- [ ] [Multipoint Evaluation (Geometric Sequence)](https://judge.yosupo.jp/problem/multipoint_evaluation_on_geometric_sequence) (`multipoint_evaluation_on_geometric_sequence`): 未対応
- [x] [Polynomial Interpolation](https://judge.yosupo.jp/problem/polynomial_interpolation) (`polynomial_interpolation`): 実装＋verifyあり — [Polynomial Interpolation](../blueberry/math/polynomial-interpolation.hpp), [verify](../verify/math/polynomial-interpolation.test.cpp)
- [ ] [Polynomial Interpolation (Geometric Sequence)](https://judge.yosupo.jp/problem/polynomial_interpolation_on_geometric_sequence) (`polynomial_interpolation_on_geometric_sequence`): 未対応
- [ ] [Polynomial Root Finding (Mod 998244353)](https://judge.yosupo.jp/problem/polynomial_root_finding) (`polynomial_root_finding`): 未対応
- [x] [Polynomial Taylor Shift](https://judge.yosupo.jp/problem/polynomial_taylor_shift) (`polynomial_taylor_shift`): 実装＋verifyあり — [Polynomial Taylor Shift](../blueberry/math/taylor-shift.hpp), [verify](../verify/math/taylor-shift.test.cpp)
- [x] [Pow of Formal Power Series](https://judge.yosupo.jp/problem/pow_of_formal_power_series) (`pow_of_formal_power_series`): 実装＋verifyあり — [Formal Power Series](../blueberry/math/formal-power-series.hpp), [verify](../verify/math/pow-formal-power-series.test.cpp)
- [ ] [Pow of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/pow_of_formal_power_series_sparse) (`pow_of_formal_power_series_sparse`): 未対応
- [x] [Product of Polynomial Sequence](https://judge.yosupo.jp/problem/product_of_polynomial_sequence) (`product_of_polynomial_sequence`): 実装＋verifyあり — [Polynomial Product](../blueberry/math/polynomial-product.hpp), [verify](../verify/math/polynomial-product.test.cpp)
- [x] [Shift of Sampling Points of Polynomial](https://judge.yosupo.jp/problem/shift_of_sampling_points_of_polynomial) (`shift_of_sampling_points_of_polynomial`): 実装＋verifyあり — [Sample Point Shift](../blueberry/math/sample-point-shift.hpp), [verify](../verify/math/sample-point-shift.test.cpp)
- [x] [Sqrt of Formal Power Series](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series) (`sqrt_of_formal_power_series`): 実装＋verifyあり — [Formal Power Series](../blueberry/math/formal-power-series.hpp), [verify](../verify/math/sqrt-formal-power-series.test.cpp)
- [ ] [Sqrt of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series_sparse) (`sqrt_of_formal_power_series_sparse`): 未対応

## Set Power Series

- [ ] [Exp of Set Power Series](https://judge.yosupo.jp/problem/exp_of_set_power_series) (`exp_of_set_power_series`): 未対応
- [ ] [Polynomial Composite Set Power Series](https://judge.yosupo.jp/problem/polynomial_composite_set_power_series) (`polynomial_composite_set_power_series`): 未対応
- [ ] [Power Projection of Set Power Series](https://judge.yosupo.jp/problem/power_projection_of_set_power_series) (`power_projection_of_set_power_series`): 未対応
- [x] [Subset Convolution](https://judge.yosupo.jp/problem/subset_convolution) (`subset_convolution`): 実装＋verifyあり — [Subset Convolution](../blueberry/math/subset-convolution.hpp), [verify](../verify/math/subset-convolution.test.cpp)

## Enumerative Combinatorics

- [ ] [Bell Number](https://judge.yosupo.jp/problem/bell_number) (`bell_number`): 未対応
- [ ] [Binomial Coefficient](https://judge.yosupo.jp/problem/binomial_coefficient) (`binomial_coefficient`): 未対応
- [ ] [Binomial Coefficient (Prime Mod)](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) (`binomial_coefficient_prime_mod`): 未対応
- [ ] [Factorial](https://judge.yosupo.jp/problem/factorial) (`factorial`): 未対応
- [ ] [Many Factorials](https://judge.yosupo.jp/problem/many_factorials) (`many_factorials`): 未対応
- [x] [Montmort Number](https://judge.yosupo.jp/problem/montmort_number_mod) (`montmort_number_mod`): 実装＋verifyあり — [Montmort Numbers](../blueberry/math/montmort.hpp), [verify](../verify/math/montmort.test.cpp)
- [ ] [Number of Increasing Sequences Between Two Sequences](https://judge.yosupo.jp/problem/number_of_increasing_sequences_between_two_sequences) (`number_of_increasing_sequences_between_two_sequences`): 未対応
- [x] [Number of Subsequences ](https://judge.yosupo.jp/problem/number_of_subsequences) (`number_of_subsequences`): 実装＋verifyあり — [Count Subsequences](../blueberry/string/count-subsequences.hpp), [verify](../verify/string/count-subsequences.test.cpp)
- [ ] [Partition Function](https://judge.yosupo.jp/problem/partition_function) (`partition_function`): 未対応
- [ ] [$q$-Binomial Coefficient (Prime Mod)](https://judge.yosupo.jp/problem/q_binomial_coefficient_prime_mod) (`q_binomial_coefficient_prime_mod`): 未対応
- [ ] [$\\#_p$ Subset Sum](https://judge.yosupo.jp/problem/sharp_p_subset_sum) (`sharp_p_subset_sum`): 未対応
- [ ] [Stirling Number of the First Kind](https://judge.yosupo.jp/problem/stirling_number_of_the_first_kind) (`stirling_number_of_the_first_kind`): 未対応
- [ ] [Stirling Number of the First Kind (Fixed K)](https://judge.yosupo.jp/problem/stirling_number_of_the_first_kind_fixed_k) (`stirling_number_of_the_first_kind_fixed_k`): 未対応
- [ ] [Stirling Number of the First Kind (Small p, Large n)](https://judge.yosupo.jp/problem/stirling_number_of_the_first_kind_small_p_large_n) (`stirling_number_of_the_first_kind_small_p_large_n`): 未対応
- [x] [Stirling Number of the Second Kind](https://judge.yosupo.jp/problem/stirling_number_of_the_second_kind) (`stirling_number_of_the_second_kind`): 実装＋verifyあり — [Stirling Numbers Second Kind](../blueberry/math/stirling-second.hpp), [verify](../verify/math/stirling-second.test.cpp)
- [ ] [Stirling Number of the Second Kind (Fixed K)](https://judge.yosupo.jp/problem/stirling_number_of_the_second_kind_fixed_k) (`stirling_number_of_the_second_kind_fixed_k`): 未対応
- [ ] [Stirling Number of the Second Kind (Small p, Large n)](https://judge.yosupo.jp/problem/stirling_number_of_the_second_kind_small_p_large_n) (`stirling_number_of_the_second_kind_small_p_large_n`): 未対応

## Linear Algebra

- [ ] [Adjugate Matrix](https://judge.yosupo.jp/problem/adjugate_matrix) (`adjugate_matrix`): 未対応
- [ ] [Characteristic Polynomial](https://judge.yosupo.jp/problem/characteristic_polynomial) (`characteristic_polynomial`): 未対応
- [ ] [Hafnian of Matrix](https://judge.yosupo.jp/problem/hafnian_of_matrix) (`hafnian_of_matrix`): 未対応
- [ ] [Intersection of $\\mathbb{F}_{2}$ vector spaces](https://judge.yosupo.jp/problem/intersection_of_f2_vector_spaces) (`intersection_of_f2_vector_spaces`): 未対応
- [x] [Inverse Matrix](https://judge.yosupo.jp/problem/inverse_matrix) (`inverse_matrix`): 実装＋verifyあり — [Matrix](../blueberry/math/matrix.hpp), [verify](../verify/math/inverse-matrix.test.cpp)
- [ ] [Inverse Matrix (Mod 2)](https://judge.yosupo.jp/problem/inverse_matrix_mod_2) (`inverse_matrix_mod_2`): 未対応
- [x] [Determinant of Matrix](https://judge.yosupo.jp/problem/matrix_det) (`matrix_det`): 実装＋verifyあり — [Matrix](../blueberry/math/matrix.hpp), [verify](../verify/math/matrix-det.test.cpp)
- [ ] [Determinant of Matrix (Arbitrary Mod)](https://judge.yosupo.jp/problem/matrix_det_arbitrary_mod) (`matrix_det_arbitrary_mod`): 未対応
- [ ] [Determinant of Matrix (Mod 2)](https://judge.yosupo.jp/problem/matrix_det_mod_2) (`matrix_det_mod_2`): 未対応
- [ ] [Matrix Product](https://judge.yosupo.jp/problem/matrix_product) (`matrix_product`): 未対応
- [ ] [Matrix Product (Mod 2)](https://judge.yosupo.jp/problem/matrix_product_mod_2) (`matrix_product_mod_2`): 未対応
- [x] [Rank of Matrix](https://judge.yosupo.jp/problem/matrix_rank) (`matrix_rank`): 実装＋verifyあり — [Matrix](../blueberry/math/matrix.hpp), [verify](../verify/math/matrix-rank.test.cpp)
- [ ] [Rank of Matrix (Mod 2)](https://judge.yosupo.jp/problem/matrix_rank_mod_2) (`matrix_rank_mod_2`): 未対応
- [ ] [Pow of Matrix](https://judge.yosupo.jp/problem/pow_of_matrix) (`pow_of_matrix`): 未対応
- [ ] [Determinant of Sparse Matrix](https://judge.yosupo.jp/problem/sparse_matrix_det) (`sparse_matrix_det`): 未対応
- [x] [System of Linear Equations](https://judge.yosupo.jp/problem/system_of_linear_equations) (`system_of_linear_equations`): 実装＋verifyあり — [Matrix](../blueberry/math/matrix.hpp), [verify](../verify/math/system-of-linear-equations.test.cpp)
- [ ] [System of Linear Equations (Mod 2)](https://judge.yosupo.jp/problem/system_of_linear_equations_mod_2) (`system_of_linear_equations_mod_2`): 未対応

## String

- [x] [Eertree](https://judge.yosupo.jp/problem/eertree) (`eertree`): 実装＋verifyあり — [Eertree](../blueberry/string/eertree.hpp), [verify](../verify/string/eertree.test.cpp)
- [x] [Enumerate Palindromes](https://judge.yosupo.jp/problem/enumerate_palindromes) (`enumerate_palindromes`): 実装＋verifyあり — [Manacher](../blueberry/string/manacher.hpp), [verify](../verify/string/enumerate-palindromes.test.cpp)
- [x] [Longest Common Substring](https://judge.yosupo.jp/problem/longest_common_substring) (`longest_common_substring`): 実装＋verifyあり — [Longest Common Substring](../blueberry/string/longest-common-substring.hpp), [verify](../verify/string/longest-common-substring.test.cpp)
- [x] [Lyndon Factorization](https://judge.yosupo.jp/problem/lyndon_factorization) (`lyndon_factorization`): 実装＋verifyあり — [Lyndon Factorization](../blueberry/string/lyndon-factorization.hpp), [verify](../verify/string/lyndon-factorization.test.cpp)
- [ ] [Number of Substrings](https://judge.yosupo.jp/problem/number_of_substrings) (`number_of_substrings`): 未対応
- [ ] [Palindromes in Deque](https://judge.yosupo.jp/problem/palindromes_in_deque) (`palindromes_in_deque`): 未対応
- [ ] [Prefix-Substring LCS](https://judge.yosupo.jp/problem/prefix_substring_lcs) (`prefix_substring_lcs`): 未対応
- [ ] [Run Enumerate](https://judge.yosupo.jp/problem/runenumerate) (`runenumerate`): 未対応
- [ ] [Suffix Array](https://judge.yosupo.jp/problem/suffixarray) (`suffixarray`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/string.html) — atcoder::suffix_arrayを利用する。
- [ ] [Wildcard Pattern Matching](https://judge.yosupo.jp/problem/wildcard_pattern_matching) (`wildcard_pattern_matching`): 未対応
- [x] [Z Algorithm](https://judge.yosupo.jp/problem/zalgorithm) (`zalgorithm`): 実装＋verifyあり — [Z Algorithm](../blueberry/string/z-algorithm.hpp), [verify](../verify/string/z-algorithm.test.cpp)

## Geometry

- [ ] [Closest Pair of Points](https://judge.yosupo.jp/problem/closest_pair) (`closest_pair`): 未対応
- [ ] [Convex Layers](https://judge.yosupo.jp/problem/convex_layers) (`convex_layers`): 未対応
- [ ] [Count Points in Triangles](https://judge.yosupo.jp/problem/count_points_in_triangle) (`count_points_in_triangle`): 未対応
- [ ] [Euclidean MST](https://judge.yosupo.jp/problem/euclidean_mst) (`euclidean_mst`): 未対応
- [ ] [Furthest Pair of Points](https://judge.yosupo.jp/problem/furthest_pair) (`furthest_pair`): 未対応
- [ ] [Manhattan MST](https://judge.yosupo.jp/problem/manhattanmst) (`manhattanmst`): 未対応
- [x] [Sort Points by Argument](https://judge.yosupo.jp/problem/sort_points_by_argument) (`sort_points_by_argument`): 実装＋verifyあり — [Fraction](../blueberry/math/fraction.hpp), [verify](../verify/math/fraction-sort-points-by-argument.test.cpp)
- [ ] [Static Convex Hull](https://judge.yosupo.jp/problem/static_convex_hull) (`static_convex_hull`): 未対応

## Big Integer

- [ ] [Addition of Big Integers](https://judge.yosupo.jp/problem/addition_of_big_integers) (`addition_of_big_integers`): 未対応
- [ ] [Addition of Hex Big Integers](https://judge.yosupo.jp/problem/addition_of_hex_big_integers) (`addition_of_hex_big_integers`): 未対応
- [ ] [Division of Big Integers](https://judge.yosupo.jp/problem/division_of_big_integers) (`division_of_big_integers`): 未対応
- [ ] [Division of Hex Big Integers](https://judge.yosupo.jp/problem/division_of_hex_big_integers) (`division_of_hex_big_integers`): 未対応
- [ ] [Multiplication of Big Integers](https://judge.yosupo.jp/problem/multiplication_of_big_integers) (`multiplication_of_big_integers`): 未対応
- [ ] [Multiplication of Hex Big Integers](https://judge.yosupo.jp/problem/multiplication_of_hex_big_integers) (`multiplication_of_hex_big_integers`): 未対応

## Other

- [ ] [Consecutive Terms of Linear Recurrent Sequence](https://judge.yosupo.jp/problem/consecutive_terms_of_linear_recurrent_sequence) (`consecutive_terms_of_linear_recurrent_sequence`): 未対応
- [x] [Find Linear Recurrence](https://judge.yosupo.jp/problem/find_linear_recurrence) (`find_linear_recurrence`): 実装＋verifyあり — [Linear Recurrence](../blueberry/math/linear-recurrence.hpp), [verify](../verify/math/find-linear-recurrence.test.cpp)
- [x] [Kth term of Linearly Recurrent Sequence](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence) (`kth_term_of_linearly_recurrent_sequence`): 実装＋verifyあり — [Linear Recurrence](../blueberry/math/linear-recurrence.hpp), [verify](../verify/math/kth-term-of-linearly-recurrent-sequence.test.cpp)
- [x] [Longest Increasing Subsequence](https://judge.yosupo.jp/problem/longest_increasing_subsequence) (`longest_increasing_subsequence`): 実装＋verifyあり — [Longest Increasing Subsequence](../blueberry/math/longest-increasing-subsequence.hpp), [verify](../verify/math/longest-increasing-subsequence.test.cpp)
- [ ] [$\\sum_{i=0}^{n-1} r^i i^d$](https://judge.yosupo.jp/problem/sum_of_exponential_times_polynomial) (`sum_of_exponential_times_polynomial`): 未対応
- [ ] [$\\sum_{i=0}^{\\infty} r^i i^d$](https://judge.yosupo.jp/problem/sum_of_exponential_times_polynomial_limit) (`sum_of_exponential_times_polynomial_limit`): 未対応
- [ ] [2 Sat](https://judge.yosupo.jp/problem/two_sat) (`two_sat`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/twosat.html) — atcoder::two_satを利用する。

## Uncategorized

- [x] [Aho Corasick](https://judge.yosupo.jp/problem/aho_corasick) (`aho_corasick`): 実装＋verifyあり — [Aho–Corasick](../blueberry/string/aho-corasick.hpp), [verify](../verify/string/aho-corasick.test.cpp)
- [ ] [All Furthest Neighbors of a Convex Polygon](https://judge.yosupo.jp/problem/all_furthest_neighbors) (`all_furthest_neighbors`): 未対応
- [ ] [Convolution ($\\mathbb{F}_{2^{64}}$)](https://judge.yosupo.jp/problem/convolution_F_2_64) (`convolution_F_2_64`): 未対応
- [ ] [Deque](https://judge.yosupo.jp/problem/deque) (`deque`): 未対応
- [ ] [Dirichlet Convolution and Prefix Sums](https://judge.yosupo.jp/problem/dirichlet_convolution_and_prefix_sums) (`dirichlet_convolution_and_prefix_sums`): 未対応
- [ ] [Dirichlet Inverse and Prefix Sums](https://judge.yosupo.jp/problem/dirichlet_inverse_and_prefix_sums) (`dirichlet_inverse_and_prefix_sums`): 未対応
- [ ] [Discrete Logarithm (Fixed Mod)](https://judge.yosupo.jp/problem/discrete_logarithm_fixed_mod) (`discrete_logarithm_fixed_mod`): 未対応
- [ ] [Incremental Minimum Spanning Forest](https://judge.yosupo.jp/problem/incremental_minimum_spanning_forest) (`incremental_minimum_spanning_forest`): 未対応
- [ ] [Inv of Bivariate Formal Power Series](https://judge.yosupo.jp/problem/inv_of_formal_power_series_2d) (`inv_of_formal_power_series_2d`): 未対応
- [ ] [Log of Set Power Series](https://judge.yosupo.jp/problem/log_of_set_power_series) (`log_of_set_power_series`): 未対応
- [ ] [Majority Voting](https://judge.yosupo.jp/problem/majority_voting) (`majority_voting`): 未対応
- [ ] [Minimum Enclosing Circle](https://judge.yosupo.jp/problem/minimum_enclosing_circle) (`minimum_enclosing_circle`): 未対応
- [ ] [Minimum Steiner Tree](https://judge.yosupo.jp/problem/minimum_steiner_tree) (`minimum_steiner_tree`): 未対応
- [ ] [Pfaffian of Matrix](https://judge.yosupo.jp/problem/pfaffian_of_matrix) (`pfaffian_of_matrix`): 未対応
- [ ] [Prefix Sum of Polynomial](https://judge.yosupo.jp/problem/prefix_sum_of_polynomial) (`prefix_sum_of_polynomial`): 未対応
- [ ] [Range Add Range Min](https://judge.yosupo.jp/problem/range_add_range_min) (`range_add_range_min`): ACL推奨 — [ACL](https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html) — ACL lazy_segtreeの区間加算・最小値モノイドで対応する。
- [ ] [Static Range Sum with Upper Bound](https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound) (`static_range_sum_with_upper_bound`): 未対応
- [ ] [Sum of Multiplicative Function](https://judge.yosupo.jp/problem/sum_of_multiplicative_function) (`sum_of_multiplicative_function`): 未対応
