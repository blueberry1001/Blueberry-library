---
title: Lowest Common Ancestor
documentation_of: //blueberry/graph/lowest-common-ancestor.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

静的な根付き木の最小共通祖先・祖先・距離をダブリングで計算します。N>=1。連結な木の隣接リストが必要です。無向木か根から辿れる親→子方向のリストを使います。前計算・メモリ O(N log N)。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/lowest-common-ancestor.hpp"
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::LowestCommonAncestor lca(tree);
  assert(lca.lca(2, 3) == 0);
  assert(lca.depth(3) == 2);
  assert(lca.distance(2, 3) == 3);
  assert(lca.kth_ancestor(3, 1) == 1);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LowestCommonAncestor lca(tree, root = 0)` | O(N log N) | [開く](#construct) |
| `int lca.lca(u, v) const` | O(log N) | [開く](#lca) |
| `int lca.kth_ancestor(v, k) const` | O(log N) | [開く](#kth-ancestor) |
| `int lca.distance(u, v) const` | O(log N) | [開く](#distance) |
| `int lca.depth(v) const` | O(1) | [開く](#depth) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>LowestCommonAncestor lca(tree, root = 0)</code> — O(N log N)</summary>

根からの深さと2冪個先の祖先を前計算します。

{% raw %}
```cpp
blueberry::LowestCommonAncestor lca(tree, 0);
```
{% endraw %}

注意点: 空・非連結・サイクルのあるグラフは対象外。端点は[0,N)。構築後の元の木の変更は反映されません。

</details>

<details class="api-operation" id="lca" markdown="1">
<summary><code>int lca.lca(u, v) const</code> — O(log N)</summary>

最も深い共通祖先を返します。

{% raw %}
```cpp
int ancestor = lca.lca(2, 3);
```
{% endraw %}

注意点: 根の選び方に依存します。u==vならuです。

</details>

<details class="api-operation" id="kth-ancestor" markdown="1">
<summary><code>int lca.kth_ancestor(v, k) const</code> — O(log N)</summary>

vから親方向へk本進んだ頂点を返します。

{% raw %}
```cpp
int parent = lca.kth_ancestor(3, 1);
```
{% endraw %}

注意点: 0<=k<=depth(v)。根より上は指定不可。k=0ならvです。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>int lca.distance(u, v) const</code> — O(log N)</summary>

u-v間の単純パスの辺数を返します。

{% raw %}
```cpp
int edges = lca.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離ではありません。重み付きは別に根からの距離を用意してください。

</details>

<details class="api-operation" id="depth" markdown="1">
<summary><code>int lca.depth(v) const</code> — O(1)</summary>

根からvへの辺数を返します。

{% raw %}
```cpp
int d = lca.depth(3);
```
{% endraw %}

注意点: 根の深さは0。頂点番号は[0,N)です。

</details>
