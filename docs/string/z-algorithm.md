---
title: Z Algorithm
documentation_of: //blueberry/string/z-algorithm.hpp
---

各位置 `i` について、列全体と `i` から始まる接尾辞の最長共通接頭辞長を求める。
文字列だけでなく、`size()`・添字アクセス・等値比較を備えた列に使用できる。

## 計算量

- 時間: $O(N)$
- メモリ: $O(N)$

空列には空列を返す。空でなければ `z[0] == N` となる。
