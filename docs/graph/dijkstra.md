---
title: Dijkstra
documentation_of: //blueberry/graph/dijkstra.hpp
---

非負辺重みのグラフに対する単一始点最短路。`WeightedGraph<Cost>` は隣接リストで、
各辺を `{to, cost}` として追加する。

## 計算量

$O((V+E)\log V)$

返り値は各頂点への距離と親を持つ。`path_to(target)` は始点から終点までの頂点列を
返し、到達不能なら空列を返す。距離の加算が `Cost` の範囲を超えないことを前提とする。
