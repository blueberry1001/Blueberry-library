---
title: Dijkstra
documentation_of: //blueberry/graph/dijkstra.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

非負重みグラフの単一始点最短路です。Vは頂点数、Eは格納した有向辺数。古い候補もキューに保持する実装で時間 O(V + E log(E+2))、追加メモリ O(V+E)。Costの演算は O(1) とします。

## 最小使用例

```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/dijkstra.hpp"
int main() {
  blueberry::WeightedGraph<long long> g(4);
  g[0].push_back({1, 2});
  g[1].push_back({2, 3});
  g[0].push_back({2, 9});
  auto result = blueberry::dijkstra(g, 0, 1LL << 60);
  assert(result.distance[2] == 5);
  assert((result.path_to(2) == std::vector<int>{0, 1, 2}));
  assert(result.path_to(3).empty());
}
```

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `WeightedGraph<Cost> g(n); g[u].push_back({v, w})` | 構築 O(V) / 辺追加 償却 O(1) | [開く](#graph) |
| `auto result = blueberry::dijkstra(g, source, infinity)` | O(V + E log(E+2)) | [開く](#dijkstra) |
| `result.distance[v]; result.parent[v]; result.infinity` | O(1) | [開く](#distance) |
| `vector<int> result.path_to(target) const` | O(経路頂点数) / 到達不能 O(1) | [開く](#path-to) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="graph" markdown="1">
<summary><code>WeightedGraph&lt;Cost&gt; g(n); g[u].push_back({v, w})</code> — 構築 O(V) / 辺追加 償却 O(1)</summary>

二重vectorの隣接リストです。辺型WeightedEdgeの公開フィールドはtoとcost。

```cpp
blueberry::WeightedGraph<long long> g(3);
g[0].push_back({1, 4});
g[1].push_back({0, 4});
```

注意点: 有向辺を1本追加します。無向辺は逆向きも追加します。端点は[0,V)、重みは非負。

</details>

<details class="api-operation" id="dijkstra" markdown="1">
<summary><code>auto result = blueberry::dijkstra(g, source, infinity)</code> — O(V + E log(E+2))</summary>

距離・親・infinityを持つShortestPathResultを返します。infinity省略時はCostの最大値です。

```cpp
auto result = blueberry::dijkstra(g, 0, 1LL << 60);
```

注意点: 有効な始点が必要です。到達距離はinfinity未満。current_distance+costがオーバーフローしない型と入力を使ってください。負辺・NaNは不可。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>result.distance[v]; result.parent[v]; result.infinity</code> — O(1)</summary>

distanceは最短距離、parentは直前頂点。始点と到達不能頂点のparentは-1です。

```cpp
bool reachable = result.distance[2] != result.infinity;
```

注意点: 到達不能判定はdistance[v]==infinityです。公開vectorを変更するとpath_toの前提を壊す場合があります。

</details>

<details class="api-operation" id="path-to" markdown="1">
<summary><code>vector&lt;int&gt; result.path_to(target) const</code> — O(経路頂点数) / 到達不能 O(1)</summary>

始点からtargetまでの頂点列を返します。到達不能は空、始点自身は1要素です。

```cpp
auto path = result.path_to(2);
```

注意点: targetは有効な頂点。最短路が複数あるときの選択順は保証しません。平行辺の辺IDは返しません。

</details>
