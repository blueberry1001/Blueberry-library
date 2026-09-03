---
title: Disjoint Set Union
documentation_of: //blueberry/data-structure/disjoint-set-union.hpp
---

素集合データ構造。頂点集合を互いに素な連結成分として管理する。

## 計算量

- `merge`, `leader`, `same`, `component_size`: 償却 $O(\alpha(N))$
- `groups`: $O(N\alpha(N))$

経路圧縮とunion by sizeを使用する。rollbackが必要な場合は
`RollbackUnionFind`を使用する。
