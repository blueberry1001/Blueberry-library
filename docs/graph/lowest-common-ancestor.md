---
title: Lowest Common Ancestor
documentation_of: //blueberry/graph/lowest-common-ancestor.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

根付き木の LCA（最小共通祖先）を二つの方式で提供します。`LowestCommonAncestor` は
ダブリングで、k-th parent も取得できます。`LowestCommonAncestorRMQ` は Euler tour と
Sparse Table を使い、前計算後の LCA・距離を $O(1)$ で返します。どちらも $N\ge1$ の
連結な木（無向隣接リスト、または根から辿れる親→子リスト）を受け取ります。

ダブリング版は前計算 $O(N\log N)$・メモリ $O(N\log N)$、RMQ 版も Euler tour の長さ
$2N-1$ に対して前計算 $O(N\log N)$・メモリ $O(N\log N)$ です。頂点番号は `[0, N)`、
根の深さは 0 です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/lowest-common-ancestor.hpp"
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::LowestCommonAncestor doubling(tree);
  blueberry::LowestCommonAncestorRMQ rmq(tree);
  assert(doubling.lca(2, 3) == 0);
  assert(doubling.kth_ancestor(3, 1) == 1);
  assert(rmq.lca(2, 3) == 0);
  assert(rmq.distance(2, 3) == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LowestCommonAncestor lca(tree, root = 0)` | O(N log N) | [開く](#doubling-construct) |
| `int lca.lca(u, v) const` | O(log N) | [開く](#doubling-lca) |
| `int lca.kth_ancestor(v, k) const` | O(log N) | [開く](#doubling-kth-ancestor) |
| `int lca.kth(v, k) const` | O(log N) | [開く](#doubling-kth) |
| `int lca.distance(u, v) const` | O(log N) | [開く](#doubling-distance) |
| `int lca.depth(v) const` | O(1) | [開く](#doubling-depth) |
| `int lca.size() const` | O(1) | [開く](#doubling-size) |
| `LowestCommonAncestorRMQ lca(tree, root = 0)` | O(N log N) | [開く](#rmq-construct) |
| `int lca.lca(u, v) const` | O(1) | [開く](#rmq-lca) |
| `int lca.distance(u, v) const` | O(1) | [開く](#rmq-distance) |
| `int lca.depth(v) const` | O(1) | [開く](#rmq-depth) |
| `int lca.size() const` | O(1) | [開く](#rmq-size) |

以下の操作を開くと、返り値・使用例・注意点を確認できます。断片の例は必要なヘッダと
有効な `lca` がある前提です。

<details class="api-operation" id="doubling-construct" markdown="1">
<summary><code>LowestCommonAncestor lca(tree, root = 0)</code> — O(N log N)</summary>

根からの深さと $2^j$ 個先の祖先を前計算します。

{% raw %}
```cpp
blueberry::LowestCommonAncestor lca(tree, 0);
```
{% endraw %}

注意点: 空・非連結・サイクルを含む入力は対象外です。構築後の元の木の変更は反映されません。

</details>

<details class="api-operation" id="doubling-lca" markdown="1">
<summary><code>int lca.lca(u, v) const</code> — O(log N)</summary>

u と v の最も深い共通祖先を返します。

{% raw %}
```cpp
int ancestor = lca.lca(2, 3);
```
{% endraw %}

注意点: 結果は選んだ根に依存します。`u == v` なら u を返します。

</details>

<details class="api-operation" id="doubling-kth-ancestor" markdown="1">
<summary><code>int lca.kth_ancestor(v, k) const</code> — O(log N)</summary>

v から親方向へ k 本進んだ頂点を返します。

{% raw %}
```cpp
int parent = lca.kth_ancestor(3, 1);
```
{% endraw %}

注意点: `0 <= k <= depth(v)` が必要です。根より上は指定できず、`k == 0` は v です。

</details>

<details class="api-operation" id="doubling-kth" markdown="1">
<summary><code>int lca.kth(v, k) const</code> — O(log N)</summary>

`kth_ancestor` の短い別名です。vから親方向へ k 本進んだ頂点を返します。

{% raw %}
```cpp
int ancestor = lca.kth(3, 1);
```
{% endraw %}

注意点: `0 <= k <= depth(v)` が必要です。返り値・計算量は `kth_ancestor` と同じです。

</details>

<details class="api-operation" id="doubling-distance" markdown="1">
<summary><code>int lca.distance(u, v) const</code> — O(log N)</summary>

u-v 間の単純パスの辺数を返します。

{% raw %}
```cpp
int edges = lca.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離ではありません。重み付きの場合は別途、根からの距離を管理してください。

</details>

<details class="api-operation" id="doubling-depth" markdown="1">
<summary><code>int lca.depth(v) const</code> — O(1)</summary>

根から v までの辺数を返します。

{% raw %}
```cpp
int d = lca.depth(3);
```
{% endraw %}

注意点: 頂点番号は `[0, N)`、根の深さは 0 です。

</details>

<details class="api-operation" id="doubling-size" markdown="1">
<summary><code>int lca.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = lca.size();
```
{% endraw %}

注意点: 構築後に変化しません。

</details>

<details class="api-operation" id="rmq-construct" markdown="1">
<summary><code>LowestCommonAncestorRMQ lca(tree, root = 0)</code> — O(N log N)</summary>

Euler tour の各頂点の深さを Sparse Table に格納します。メモリは $O(N\log N)$ です。

{% raw %}
```cpp
blueberry::LowestCommonAncestorRMQ lca(tree, 0);
```
{% endraw %}

注意点: LCA を大量に問い合わせる静的な木向けです。構築後の木の変更には対応しません。

</details>

<details class="api-operation" id="rmq-lca" markdown="1">
<summary><code>int lca.lca(u, v) const</code> — O(1)</summary>

Euler tour 上の `first[u]..first[v]` の最小深さを RMQ し、LCA を返します。

{% raw %}
```cpp
blueberry::LowestCommonAncestorRMQ lca(tree);
int ancestor = lca.lca(2, 3);
```
{% endraw %}

注意点: 時間計算量は O(1) ですが、Sparse Table の前計算・メモリは O(N log N) です。根の選択に依存します。

</details>

<details class="api-operation" id="rmq-distance" markdown="1">
<summary><code>int lca.distance(u, v) const</code> — O(1)</summary>

RMQ 版の LCA と深さから u-v 間の辺数を計算します。

{% raw %}
```cpp
int edges = lca.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離は返しません。

</details>

<details class="api-operation" id="rmq-depth" markdown="1">
<summary><code>int lca.depth(v) const</code> — O(1)</summary>

根から v までの辺数を返します。

{% raw %}
```cpp
int d = lca.depth(3);
```
{% endraw %}

注意点: 頂点番号は `[0, N)` です。

</details>

<details class="api-operation" id="rmq-size" markdown="1">
<summary><code>int lca.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = lca.size();
```
{% endraw %}

注意点: 構築後に変化しません。

</details>
