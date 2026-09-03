---
title: Sparse Table
documentation_of: //blueberry/data-structure/sparse-table.hpp
---

静的な冪等半群の区間積を扱うSparse Table。演算は結合的かつ冪等
（`op(x, x) == x`）でなければならない。min/max/gcdなどに利用できる。

## 計算量

- 構築: $O(N\log N)$
- `product`: $O(1)$
- メモリ: $O(N\log N)$

空区間の積は定義せず、`left < right` が必要である。
