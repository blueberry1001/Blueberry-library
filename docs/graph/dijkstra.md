---
title: Dijkstra
documentation_of: //blueberry/graph/dijkstra.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

非負辺重みのグラフに対する単一始点最短路です。頂点数を $V$、辺数を $E$ とすると、
二分ヒープを使って計算量は $O((V+E)\log V)$、メモリは $O(V+E)$ です。辺コストは
`Cost{}` 以上で、距離の加算が `Cost` の範囲を超えないことを前提とします。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/dijkstra.hpp"
int main() {
  blueberry::WeightedGraph<int> graph(3);
  graph[0].push_back({1, 2});
  graph[1].push_back({2, 3});
  auto result = blueberry::dijkstra(graph, 0);
  assert(result.distance[2] == 5);
  assert(result.path_to(2) == std::vector<int>({0, 1, 2}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `WeightedEdge<Cost>{to, cost}` | O(1) | [開く](#edge) |
| `ShortestPathResult<Cost>::distance` | O(1) access | [開く](#distance) |
| `ShortestPathResult<Cost>::parent` | O(1) access | [開く](#parent) |
| `ShortestPathResult<Cost>::infinity` | O(1) access | [開く](#infinity) |
| `vector<int> result.path_to(target)` | O(V) | [開く](#path-to) |
| `dijkstra(graph, source, infinity)` | O((V+E) log V) | [開く](#dijkstra) |

<details class="api-operation" id="edge" markdown="1">
<summary><code>WeightedEdge&lt;Cost&gt;{to, cost}</code> — O(1)</summary>

隣接リストの一辺を表し、`to` が終点、`cost` が重みです。

{% raw %}
```cpp
blueberry::WeightedEdge<int> edge{2, 7};
```
{% endraw %}

注意点: `to` はグラフの頂点範囲内、`cost` は非負でなければなりません。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>result.distance[v]</code> — O(1) access</summary>

始点から各頂点への最短距離を格納する公開vectorです。

{% raw %}
```cpp
int distance = result.distance[2];
```
{% endraw %}

注意点: 到達不能な頂点は `result.infinity` です。添字は `[0,V)` です。

</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>result.parent[v]</code> — O(1) access</summary>

最短路木での直前の頂点を格納します。始点や到達不能頂点は `-1` です。

{% raw %}
```cpp
int previous = result.parent[2];
```
{% endraw %}

注意点: 同じ最短距離を持つ経路の選択は入力順などに依存します。

</details>

<details class="api-operation" id="infinity" markdown="1">
<summary><code>result.infinity</code> — O(1) access</summary>

到達不能を表す値を返します。既定値は `numeric_limits&lt;Cost&gt;::max()` です。

{% raw %}
```cpp
if (result.distance[v] == result.infinity) { /* unreachable */ }
```
{% endraw %}

注意点: 実際の距離がこの値にならないようにしてください。必要なら十分大きく安全な値を指定します。

</details>

<details class="api-operation" id="path-to" markdown="1">
<summary><code>vector&lt;int&gt; result.path_to(target)</code> — O(V)</summary>

`parent` を辿り、始点から `target` までの頂点列を返します。未到達なら空vectorです。

{% raw %}
```cpp
auto path = result.path_to(2);
```
{% endraw %}

注意点: パス長が $V$ に比例するため、全頂点で呼ぶ場合は合計計算量に注意してください。

</details>

<details class="api-operation" id="dijkstra" markdown="1">
<summary><code>dijkstra(graph, source, infinity)</code> — O((V+E) log V)</summary>

始点からの距離・最短路木・未到達値を `ShortestPathResult` として返します。

{% raw %}
```cpp
auto result = blueberry::dijkstra(graph, 0, 1'000'000'000);
```
{% endraw %}

注意点: 負辺には使えません。`source` は `[0,V)`、辺の終点も有効な頂点である必要があります。

</details>
