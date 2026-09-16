---
title: Low Link
documentation_of: //blueberry/graph/low-link.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

無向グラフの橋・関節点・二辺連結成分を一度に求めます。ACLの有向グラフ用SCCとは異なる機能です。
頂点数を N、辺数を M とし、構築時間・構築時メモリ・保持メモリはいずれも O(N+M) です。
DFSと成分探索は反復処理なので、長いパスでも再帰スタックを消費しません。

頂点は `[0,N)`、辺IDは入力vectorの添字 `[0,M)`。`0 <= N <= INT_MAX`、`M <= INT_MAX` とします。
非連結グラフ、孤立頂点、自己ループ、多重辺に対応します。`N=0` では辺も空にしてください。
自己ループは橋ではなく、同じ両端を結ぶ複数の辺も互いに迂回できるため橋ではありません。
グラフは構築後に変更できません。入力vectorへの参照を保持しないため、構築後に入力を破棄・変更できます。
不正な添字・サイズは前提違反で、debug buildではassertで検出します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/graph/low-link.hpp"
int main() {
  std::vector<std::pair<int, int>> edges{{0, 1}, {0, 1}, {1, 2}, {2, 2}};
  blueberry::LowLink graph(4, edges);
  assert(graph.size() == 4);
  assert(!graph.is_bridge(0) && graph.is_bridge(2));
  assert(graph.is_articulation(1));
  assert(graph.bridges() == std::vector<int>{2});
  assert(graph.articulation_points() == std::vector<int>{1});
  assert(graph.component(0) == graph.component(1));
  assert(graph.component(1) != graph.component(2));
  assert((graph.groups() == std::vector<std::vector<int>>{{0, 1}, {2}, {3}}));
  assert(blueberry::LowLink(0, {}).groups().empty());
}
```
{% endraw %}

## 操作一覧

計算量は最悪計算量です。`bridges()` と `articulation_points()` は内部vectorへのconst参照を返し、
要素をコピーしません。参照はオブジェクトの破棄・代入・移動まで有効です。`groups()` は独立した値を返します。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LowLink graph(n, edges)` | O(N+M) | [開く](#construct) |
| `int graph.size()` | O(1) | [開く](#size) |
| `bool graph.is_bridge(edge_id)` | O(1) | [開く](#is-bridge) |
| `bool graph.is_articulation(v)` | O(1) | [開く](#is-articulation) |
| `const vector<int>& graph.bridges()` | O(1) | [開く](#bridges) |
| `const vector<int>& graph.articulation_points()` | O(1) | [開く](#articulation-points) |
| `int graph.component(v)` | O(1) | [開く](#component) |
| `vector<vector<int>> graph.groups()` | O(N) | [開く](#groups) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>LowLink graph(int n, const vector&lt;pair&lt;int,int&gt;&gt;&amp; edges)</code> — O(N+M)</summary>

各 `edges[id] = {u,v}` を無向辺として読み取り、橋・関節点と二辺連結成分を前計算します。
辺IDは入力順を保持します。`n=0` と空の `edges` も有効です。

{% raw %}
```cpp
blueberry::LowLink graph(3, {{0, 1}, {1, 2}});
```
{% endraw %}

注意点: `n>=0`、全辺の両端が `[0,n)` である必要があります。構築中も入力を変更しないでください。
入力の所有権は移動しません。整数演算は頂点・辺ID・訪問順だけで、指定した上限内で扱えます。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int graph.size()</code> — O(1)</summary>

頂点数を返します。空グラフでは0です。

{% raw %}
```cpp
int n = graph.size();
```
{% endraw %}

注意点: 辺数や二辺連結成分数ではありません。

</details>

<details class="api-operation" id="is-bridge" markdown="1">
<summary><code>bool graph.is_bridge(int edge_id)</code> — O(1)</summary>

指定した1本の辺を除去したとき、連結成分数が増えるならtrueを返します。

{% raw %}
```cpp
bool bridge = graph.is_bridge(2);  // 最小使用例の辺1--2
```
{% endraw %}

注意点: `edge_id` は `[0,M)`。多重辺は別々のIDを持ち、自己ループはfalseです。

</details>

<details class="api-operation" id="is-articulation" markdown="1">
<summary><code>bool graph.is_articulation(int v)</code> — O(1)</summary>

頂点vとその接続辺を除去したとき、グラフ全体の連結成分数が増えるならtrueを返します。

{% raw %}
```cpp
bool articulation = graph.is_articulation(1);
```
{% endraw %}

注意点: `v` は `[0,N)`。孤立頂点や、二頂点一本の辺の両端は関節点ではありません。

</details>

<details class="api-operation" id="bridges" markdown="1">
<summary><code>const vector&lt;int&gt;&amp; graph.bridges()</code> — O(1)</summary>

橋の辺IDを昇順に格納したvectorへのconst参照を返します。橋がなければ空です。

{% raw %}
```cpp
for (int id : graph.bridges()) {
  auto [u, v] = edges[id];  // 元の入力から両端を取得
  (void)u; (void)v;
}
```
{% endraw %}

注意点: 頂点の組ではなく辺IDです。列挙には橋の本数に比例する時間がかかります。
返された参照をオブジェクトの破棄・代入・移動後に使わないでください。

</details>

<details class="api-operation" id="articulation-points" markdown="1">
<summary><code>const vector&lt;int&gt;&amp; graph.articulation_points()</code> — O(1)</summary>

関節点の頂点IDを昇順に格納したvectorへのconst参照を返します。関節点がなければ空です。

{% raw %}
```cpp
const auto& vertices = graph.articulation_points();
```
{% endraw %}

注意点: 列挙には関節点数に比例する時間がかかります。参照はオブジェクトの破棄・代入・移動まで有効です。

</details>

<details class="api-operation" id="component" markdown="1">
<summary><code>int graph.component(int v)</code> — O(1)</summary>

vが属する二辺連結成分のIDを返します。すべての橋を取り除いた後で到達し合える頂点は同じIDです。
成分IDは `[0,K)` で、Kは成分数です。成分内の最小頂点が小さい順にIDを割り当てます。

{% raw %}
```cpp
bool same = graph.component(0) == graph.component(1);
```
{% endraw %}

注意点: `v` は `[0,N)`。通常の連結成分とは異なり、橋の両端は別成分です。
この値を使って橋で結ばれた成分の森を構築できます。

</details>

<details class="api-operation" id="groups" markdown="1">
<summary><code>vector&lt;vector&lt;int&gt;&gt; graph.groups()</code> — O(N)</summary>

成分IDごとの頂点列を返します。各列は頂点番号の昇順です。全頂点がちょうど1回現れ、空成分は含みません。
空グラフの返り値は空vectorです。

{% raw %}
```cpp
auto components = graph.groups();
```
{% endraw %}

注意点: 呼び出すたびに O(N) の時間と追加メモリで結果を構築します。戻り値は独立して保持・変更できます。

</details>

## 出典・検証

[ei1333 LowLink](https://github.com/ei1333/library/blob/master/graph/others/low-link.hpp) の
DFS lowlinkと多重辺の扱い、[KACTL BiconnectedComponents](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/BiconnectedComponents.h)
の辺IDによる親辺識別を比較し、標準的なlowlinkの漸化式から独立に実装しました。コードの転用はしていません。
本実装は再帰・グラフ継承を使わず、入力順IDを持つ反復DFSと、橋を除いた反復探索を採用しています。
ACLのSCCやDSUを再実装するものではありません。

公式問題は [Two Edge Connected Components](https://judge.yosupo.jp/problem/two_edge_connected_components)。
`tests/random/low-link.cpp` では小さい多重グラフから各辺・各頂点を取り除いた愚直探索と比較し、
空入力、孤立点、自己ループ、多重辺、非連結、20万頂点の鎖も検証します。
