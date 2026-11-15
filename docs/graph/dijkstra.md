---
title: Dijkstra
documentation_of: //blueberry/graph/dijkstra.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要

非負辺重みのグラフに対する単一始点最短路です。`infinity`は正の値とし、最短距離が`infinity`以上の頂点は未到達として扱います。

Vを頂点数、Eを辺数とすると、計算時間はO(V+E log(E+1))、メモリはO(V+E)です。
`dijkstra`は全頂点、`dijkstra_path`は指定した1頂点までの最短路を返します。
辺を構築後に変更しない場合は、連続配置する`WeightedCsrGraph`も使えます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`WeightedEdge<Cost>{to, cost}`](#edge) | O(1) |
| [`edge.to`](#edge-to) | O(1) access |
| [`edge.cost`](#edge-cost) | O(1) access |
| [`ShortestPathResult<Cost>{distance, parent, infinity}`](#result) | O(V) copy / O(1) move |
| [`ShortestPathResult<Cost>::distance`](#distance) | O(1) access |
| [`ShortestPathResult<Cost>::parent`](#parent) | O(1) access |
| [`ShortestPathResult<Cost>::infinity`](#infinity) | O(1) access |
| [`result.path_to(target)`](#path-to) | O(V) |
| [`dijkstra(graph, source, infinity)`](#dijkstra) | O(V + E log(E+1)) |
| [`dijkstra(csr, source, infinity)`](#dijkstra-csr) | O(V + E log(E+1)) |
| [`dijkstra_path(graph, source, target, infinity)`](#dijkstra-path) | O(V + E log(E+1)) |
| [`dijkstra_path(csr, source, target, infinity)`](#dijkstra-path-csr) | O(V + E log(E+1)) |

<details class="extra-operations" markdown="1">
<summary>CSRの構築と参照</summary>

| 操作 | 計算量 |
| --- | --- |
| [`WeightedCsrGraph<Cost>()`](#csr-default) | O(1) |
| [`WeightedCsrGraph<Cost>(n, edges)`](#csr-build) | O(V+E) |
| [`WeightedCsrGraph<Cost>(other)`](#csr-copy) | O(V+E) |
| [`WeightedCsrGraph<Cost>(std::move(other))`](#csr-move) | O(1) |
| [`graph = other`](#csr-copy-assign) | O(V+E+V_old+E_old) |
| [`graph = std::move(other)`](#csr-move-assign) | O(V_old+E_old) |
| [`graph.size()`](#csr-size) | O(1) |
| [`graph[v]`](#csr-adjacency) | O(1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- boolを除く64bit以下の標準整数型では、頂点数が256以下なら二分ヒープ、それ以外は距離をradix heapで管理し、同距離は頂点番号の小さい順に取り出します。それ以外の型は二分ヒープを使います。radixのbit幅は最大64で、表の計算量では定数として扱います。
- 単純グラフでは通常のO((V+E)log V)と書けますが、平行辺を許すためキュー長はO(E)です。
- 辺コストは`Cost{}`以上とします。標準整数型の距離は加算前に`infinity`で打ち切り、符号付き・符号なし双方のオーバーフローを防ぎます。
- 浮動小数点・独自型では、加算結果が型に収まり、全順序比較ができることが必要です（NaN不可）。
- `WeightedGraph<Cost>`は`vector<vector<WeightedEdge<Cost>>>`の別名です。通常のvectorと同じ構築・更新・参照無効化規則を持ち、辺の`to`は0-indexedの頂点番号です。
- `WeightedCsrGraph<Cost>`は辺を連続配列に保存する静的グラフです。構築後に辺を追加できません。同じ始点の辺を入力順に格納するため、同じ順で挿入した`WeightedGraph`と距離・親の選択が一致します。CSR構築は入力辺列を別途保持したままO(V+E)メモリを使い、一時的にO(V)の作業領域も必要です。
- 代入の計算量ではV・Eを代入元、V_old・E_oldを代入前の代入先の頂点数・辺数とします。

</details>

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
  std::vector<std::tuple<int, int, int>> edges{{0, 1, 2}, {1, 2, 3}};
  blueberry::WeightedCsrGraph<int> csr(3, edges);
  assert(blueberry::dijkstra_path(csr, 0, 2).first == 5);
}
```
{% endraw %}

<details class="api-operation" id="csr-default" markdown="1">
<summary><code>WeightedCsrGraph&lt;Cost&gt;()</code> — O(1)</summary>

頂点も辺もないグラフを作ります。

{% raw %}
```cpp
blueberry::WeightedCsrGraph<int> graph;
assert(graph.size() == 0);
```
{% endraw %}

注意点: 空グラフには有効な始点がないため、`dijkstra`には渡せません。

</details>

<details class="api-operation" id="csr-build" markdown="1">
<summary><code>WeightedCsrGraph&lt;Cost&gt;(int n, const vector&lt;tuple&lt;int,int,Cost&gt;&gt;&amp; edges)</code> — O(V+E)</summary>

頂点数`n`と、`(from, to, cost)`の辺列からグラフを構築します。同じ始点の辺は入力順です。

{% raw %}
```cpp
std::vector<std::tuple<int, int, int>> edges{{0, 1, 2}, {1, 2, 3}};
blueberry::WeightedCsrGraph<int> graph(3, edges);
assert(blueberry::dijkstra(graph, 0).distance[2] == 5);
```
{% endraw %}

注意点: `n >= 0`、両端点は`[0,n)`です。辺列は空でもよく、自己ループ・平行辺を保持します。
`Cost`は値初期化、コピー構築、コピー代入が必要で、最短路で使う場合は重み・演算について`dijkstra`と同じ前提を守ってください。
グラフは入力から独立して所有し、入力辺列の変更や破棄の影響を受けません。領域はO(V+E)、構築中の一時領域はO(V)で、呼び出し元の入力辺列のO(E)は別に必要です。

</details>

<details class="api-operation" id="csr-copy" markdown="1">
<summary><code>WeightedCsrGraph&lt;Cost&gt;(const WeightedCsrGraph&amp; other)</code> — O(V+E)</summary>

頂点と辺を独立にコピーします。

{% raw %}
```cpp
auto copy = graph;
```
{% endraw %}

注意点: `Cost`のコピーが必要です。要素の参照はコピー元と共有しません。

</details>

<details class="api-operation" id="csr-move" markdown="1">
<summary><code>WeightedCsrGraph&lt;Cost&gt;(WeightedCsrGraph&amp;&amp; other)</code> — O(1)</summary>

所有する配列を移動します。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元は破棄または代入して再利用できます。移動前に得たspanは使用しないでください。

</details>

<details class="api-operation" id="csr-copy-assign" markdown="1">
<summary><code>graph = other</code> — O(V+E+V_old+E_old)</summary>

辺と頂点をコピーで置き換え、代入先への参照を返します。

{% raw %}
```cpp
blueberry::WeightedCsrGraph<int> copy;
copy = graph;
```
{% endraw %}

注意点: コピーの構築に失敗した場合、代入先は変更されません。コピー中は代入元と同じ大きさのO(V+E)の追加メモリを使います。成功後は代入先から以前に得たspanは無効になります。自己代入では変更しません。

</details>

<details class="api-operation" id="csr-move-assign" markdown="1">
<summary><code>graph = std::move(other)</code> — O(V_old+E_old)</summary>

所有する配列を移動して置き換え、代入先への参照を返します。

{% raw %}
```cpp
blueberry::WeightedCsrGraph<int> moved;
moved = std::move(copy);
```
{% endraw %}

注意点: 代入先の旧要素の破棄に比例する場合があります。移動元・代入先から以前に得たspanは使用しないでください。自己移動代入では変更しません。

</details>

<details class="api-operation" id="csr-size" markdown="1">
<summary><code>int graph.size() const</code> — O(1)</summary>

頂点数を返します。空グラフでは0です。

{% raw %}
```cpp
assert(graph.size() == 3);
```
{% endraw %}

注意点: 辺の数ではありません。

</details>

<details class="api-operation" id="csr-adjacency" markdown="1">
<summary><code>span&lt;const WeightedEdge&lt;Cost&gt;&gt; graph[int v] const</code> — O(1)</summary>

頂点`v`から出る辺を入力順の読み取り専用spanとして返します。

{% raw %}
```cpp
for (const auto& edge : graph[0]) { (void)edge; }
```
{% endraw %}

注意点: `v`は`[0,graph.size())`です。出辺がなければ空spanです。spanはグラフを所有せず、破棄・代入・移動の後は使用できません。

</details>

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

注意点: `target`は`[0,V)`内です。始点自身には始点1個のvector、未到達・打ち切り対象には空vectorです。
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

<details class="api-operation" id="dijkstra-path" markdown="1">
<summary><code>dijkstra_path(graph, source, target, infinity = numeric_limits&lt;Cost&gt;::max())</code> — O(V + E log(E+1))</summary>

`source`から`target`への距離と経路を`pair&lt;Cost, vector&lt;int&gt;&gt;`で返します。
対象頂点の最短距離が確定した時点で探索を終えます。

{% raw %}
```cpp
auto [distance, path] = blueberry::dijkstra_path(graph, 0, 2, 1'000'000'000);
```
{% endraw %}

注意点: `source`と`target`は`[0,V)`内、`infinity`は正で、辺重み・型の前提は`dijkstra`と同じです。
到達不能または最短距離が`infinity`以上なら`{infinity, {}}`、始点と終点が同じなら`{Cost{}, {source}}`を返します。
経路は両端を含む独立したvectorで、同距離の経路は`dijkstra(...).path_to(target)`と同じです。
始点と終点が同じ場合はO(1)、それ以外の計算量とメモリは最悪の場合の値です。

</details>

<details class="api-operation" id="dijkstra-csr" markdown="1">
<summary><code>dijkstra(csr, source, infinity = numeric_limits&lt;Cost&gt;::max())</code> — O(V + E log(E+1))</summary>

`WeightedCsrGraph`から全頂点の最短距離と親を返します。

{% raw %}
```cpp
auto result = blueberry::dijkstra(graph, 0);
```
{% endraw %}

注意点: `dijkstra(WeightedGraph, ...)`と同じ辺重み、型、頂点範囲、打ち切りの前提です。結果はグラフとは独立し、同じ辺順の`WeightedGraph`と親の選択も一致します。
CSR版はグラフ型を推論して呼び出します。`dijkstra<int>(csr, ...)`のように`Cost`を明示しません。

</details>

<details class="api-operation" id="dijkstra-path-csr" markdown="1">
<summary><code>dijkstra_path(csr, source, target, infinity = numeric_limits&lt;Cost&gt;::max())</code> — O(V + E log(E+1))</summary>

`WeightedCsrGraph`で単一終点の距離と経路を返します。

{% raw %}
```cpp
auto [distance, path] = blueberry::dijkstra_path(graph, 0, 2);
```
{% endraw %}

注意点: `dijkstra_path(WeightedGraph, ...)`と同じ前提・返り値・同距離の親選択を持ちます。始点と終点が等しい場合はO(1)です。
CSR版はグラフ型を推論して呼び出し、`Cost`を明示しません。

</details>

## 検証

`shortest_path` の公式ケースに加え、`tests/random/dijkstra.cpp` でFloyd–Warshallとの比較、
経路復元、単一終点版と全頂点版の一致、CSRと従来の隣接リストの辺順・距離・親の一致、符号付き・符号なし整数の上限、独自の打ち切り値を検証します。
整数キューの設計は[Nyaanのradix heapによるDijkstra](https://nyaannyaan.github.io/library/shortest-path/dijkstra-radix-heap.hpp.html)を参考に、同距離の頂点順序を維持する形で独立に実装しました。
