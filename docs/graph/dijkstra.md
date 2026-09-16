---
title: Dijkstra
documentation_of: //blueberry/graph/dijkstra.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

非負辺重みのグラフに対する単一始点最短路です。頂点数を $V$、辺数を $E$ とすると、
二分ヒープを使って計算量は $O(V+E\log(E+1))$、メモリは $O(V+E)$ です。
単純グラフでは通常の $O((V+E)\log V)$ と書けますが、平行辺を許すためヒープ長は $O(E)$ です。
辺コストは `Cost{}` 以上、`infinity` は正の値が必要です。
標準整数型の距離は加算前に `infinity` で打ち切るため、符号付き・符号なし双方のオーバーフローを防ぎます。
最短距離が `infinity` 以上の頂点は未到達と同じ扱いです。
浮動小数点・独自型では従来通り加算結果が型に収まり、全順序比較ができることが必要です（NaN不可）。
`WeightedGraph<Cost>` は `vector<vector<WeightedEdge<Cost>>>` の別名です。
通常のvectorと同じ構築・更新・参照無効化規則を持ち、辺の `to` は0-indexedの頂点番号です。

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
| `edge.to` | O(1) access | [開く](#edge-to) |
| `edge.cost` | O(1) access | [開く](#edge-cost) |
| `ShortestPathResult<Cost>{distance, parent, infinity}` | O(V) copy / O(1) move | [開く](#result) |
| `ShortestPathResult<Cost>::distance` | O(1) access | [開く](#distance) |
| `ShortestPathResult<Cost>::parent` | O(1) access | [開く](#parent) |
| `ShortestPathResult<Cost>::infinity` | O(1) access | [開く](#infinity) |
| `vector<int> result.path_to(target)` | O(V) | [開く](#path-to) |
| `dijkstra(graph, source, infinity)` | O(V + E log(E+1)) | [開く](#dijkstra) |

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

<details class="api-operation" id="edge-to" markdown="1">
<summary><code>int edge.to</code> — O(1) access</summary>

辺の終点を保持する公開フィールドです。

{% raw %}
```cpp
blueberry::WeightedEdge<int> edge{2, 7};
assert(edge.to == 2);
```
{% endraw %}

注意点: `dijkstra` に渡すときは `[0,V)` 内にしてください。無向辺は両方向を隣接リストへ追加します。

</details>

<details class="api-operation" id="edge-cost" markdown="1">
<summary><code>Cost edge.cost</code> — O(1) access</summary>

辺の非負の重みを保持する公開フィールドです。

{% raw %}
```cpp
blueberry::WeightedEdge<int> edge{2, 7};
edge.cost = 0;
```
{% endraw %}

注意点: `Cost{}` が零である必要があります。負の値やNaNを渡せません。

</details>

<details class="api-operation" id="result" markdown="1">
<summary><code>ShortestPathResult&lt;Cost&gt;{distance, parent, infinity}</code> — O(V) copy / O(1) move</summary>

距離vector、親vector、打ち切り値の順に集成体初期化できます。通常は `dijkstra` の返り値を使います。

{% raw %}
```cpp
blueberry::ShortestPathResult<int> result{{0, 3}, {-1, 0}, 100};
assert(result.path_to(1) == std::vector<int>({0, 1}));
```
{% endraw %}

注意点: vectorの長さは同じで、到達可能な頂点の親を辿ると有限回で `-1` に到達する必要があります。
手作業で構築・変更した結果の整合性は呼び出し側の責任です。各vectorは独立した所有値です。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>result.distance[v]</code> — O(1) access</summary>

始点から各頂点への最短距離を格納する公開vectorです。

{% raw %}
```cpp
int distance = result.distance[2];
```
{% endraw %}

注意点: 到達不能または距離が `infinity` 以上の頂点は `result.infinity` です。添字は `[0,V)` です。
結果は入力グラフに依存しない値です。vectorを利用者側で再確保すると、その要素への参照は無効化されます。

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

注意点: 正の値が必要です。取得したい有限距離より大きい値を指定してください。
標準整数型ではこの値以上の距離を未到達として打ち切ります。

</details>

<details class="api-operation" id="path-to" markdown="1">
<summary><code>vector&lt;int&gt; result.path_to(target)</code> — O(V)</summary>

`parent` を辿り、始点から `target` までの頂点列を返します。未到達なら空vectorです。

{% raw %}
```cpp
auto path = result.path_to(2);
```
{% endraw %}

注意点: 始点自身には始点1個のvector、未到達・打ち切り対象には空vectorです。
パス長が $V$ に比例するため、全頂点で呼ぶ場合は合計計算量に注意してください。
公開の `parent` を変更して循環などを作らないでください。返したvectorは独立した値です。

</details>

<details class="api-operation" id="dijkstra" markdown="1">
<summary><code>dijkstra(graph, source, infinity = numeric_limits&lt;Cost&gt;::max())</code> — O(V + E log(E+1))</summary>

始点からの距離・最短路木・未到達値を `ShortestPathResult` として返します。

{% raw %}
```cpp
auto result = blueberry::dijkstra(graph, 0, 1'000'000'000);
```
{% endraw %}

注意点: 負辺には使えません。`source` は `[0,V)`、辺の終点も有効な頂点である必要があります。
空グラフは不可、単頂点・自己ループ・平行辺・重み0は有効です。
`Cost` はコピー、比較、加算、`Cost{}` に対応する型です。入力グラフは変更しません。

</details>

## 検証

`shortest_path` の公式ケースに加え、`tests/random/dijkstra.cpp` でFloyd–Warshallとの比較、
経路復元、符号付き・符号なし整数の上限、独自の打ち切り値を検証します。
