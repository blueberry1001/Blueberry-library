---
title: Fenwick Tree
documentation_of: //blueberry/data-structure/fenwick-tree.hpp
---

一点加算と区間和を扱うFenwick Tree。区間は半開区間 `[left, right)` である。

## 計算量

- 構築: $O(N)$
- `add`, `prefix_sum`, `sum`, `lower_bound`: $O(\log N)$
- `get`: $O(\log N)$

`lower_bound(target)` は `prefix_sum(r) >= target` となる最小の `r` を返す。
この操作を使う場合、更新値は非負でなければならない。
