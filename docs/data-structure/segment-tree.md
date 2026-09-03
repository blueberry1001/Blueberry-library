---
title: Segment Tree
documentation_of: //blueberry/data-structure/segment-tree.hpp
---

モノイドの一点更新・区間積を扱うSegment Tree。演算は結合則を満たし、与える
`identity` は単位元でなければならない。可換性は不要。

## 計算量

- 構築: $O(N)$
- `set`, `product`: $O(\log N)$
- `get`, `all_product`: $O(1)$

`product(left, right)` は添字の昇順で `[left, right)` の要素を畳み込む。
