---
title: Lowest Common Ancestor
documentation_of: //blueberry/graph/lowest-common-ancestor.hpp
---

ダブリングによる根付き木の最小共通祖先。入力する隣接リストは連結な木でなければならない。

## 計算量

- 前計算: $O(N\log N)$
- `lca`, `kth_ancestor`: $O(\log N)$
- `distance`, `depth`: それぞれ $O(\log N)$, $O(1)$

`distance(u, v)` は辺数を返す。重み付き距離は扱わない。
