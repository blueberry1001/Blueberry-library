---
title: Rollback Union Find
documentation_of: //blueberry/data-structure/rollback-union-find.hpp
---

変更履歴を巻き戻せるUnion Find。経路圧縮を行わず、union by sizeを使用する。

## 計算量

- `merge`, `leader`, `same`, `component_size`: $O(\log N)$
- `state`, `snapshot`: $O(1)$
- `undo`: $O(1)$
- `rollback`: 取り消すmerge回数に比例

`state()` の返り値を保存し、`rollback(state)` に渡すことで任意の保存地点へ戻せる。
同一連結成分への `merge` も履歴を1件消費するため、探索木上で扱いやすい。
