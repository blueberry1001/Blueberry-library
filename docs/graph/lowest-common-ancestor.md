---
title: Lowest Common Ancestor
documentation_of: //blueberry/graph/lowest-common-ancestor.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

静的な根付き木の最小共通祖先（LCA）・深さ・辺数距離を求めます。
`LowestCommonAncestor` はダブリングで、前計算・メモリ O(N log N)、各クエリ O(log N) です。
`LowestCommonAncestorRMQ` は Euler Tour と `SparseTable` により、前計算・メモリ O(N log N)、
LCA・距離クエリ O(1) です。N>=1 の連結な木を、無向隣接リストまたは根から辿れる親→子リストで渡します。

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
  assert(doubling.kth(3, 1) == 1);
  assert(doubling.distance(2, 3) == 3);
  assert(rmq.lca(2, 3) == 0);
  assert(rmq.distance(2, 3) == 3);
}
```
{% endraw %}

## 操作一覧

ここで N は頂点数です。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LowestCommonAncestor lca(tree, root = 0)` | O(N log N) | [開く](#construct-doubling) |
| `int lca.lca(u, v) const` | O(log N) | [開く](#lca-doubling) |
| `int lca.kth_ancestor(v, k) const` | O(log N) | [開く](#kth-ancestor) |
| `int lca.kth(v, k) const` | O(log N) | [開く](#kth) |
| `int lca.distance(u, v) const` | O(log N) | [開く](#distance-doubling) |
| `int lca.depth(v) const` | O(1) | [開く](#depth-doubling) |
| `int lca.size() const` | O(1) | [開く](#size-doubling) |
| `LowestCommonAncestorRMQ lca(tree, root = 0)` | O(N log N) | [開く](#construct-rmq) |
| `int lca.lca(u, v) const` | O(1) | [開く](#lca-rmq) |
| `int lca.distance(u, v) const` | O(1) | [開く](#distance-rmq) |
| `int lca.depth(v) const` | O(1) | [開く](#depth-rmq) |
| `int lca.size() const` | O(1) | [開く](#size-rmq) |

<details class="api-operation" id="construct-doubling" markdown="1">
<summary><code>LowestCommonAncestor lca(tree, root = 0)</code> — O(N log N)</summary>

根からの深さと2冪個先の祖先を前計算します。メモリは O(N log N) です。

{% raw %}
```cpp
blueberry::LowestCommonAncestor lca(tree, 0);
```
{% endraw %}

注意点: `tree` は N>=1 の連結な木、`root` は `[0,N)`。構築後の木の変更は反映されません。

</details>

<details class="api-operation" id="lca-doubling" markdown="1">
<summary><code>int lca.lca(u, v) const</code> — O(log N)</summary>

`u` と `v` の最も深い共通祖先を返します。

{% raw %}
```cpp
int ancestor = lca.lca(2, 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)`。結果は構築時の根に依存し、`u==v` なら `u` です。

</details>

<details class="api-operation" id="kth-ancestor" markdown="1">
<summary><code>int lca.kth_ancestor(v, k) const</code> — O(log N)</summary>

`v` から親方向へ `k` 辺進んだ祖先を返します。

{% raw %}
```cpp
int parent = lca.kth_ancestor(3, 1);
```
{% endraw %}

注意点: `v` は `[0,N)`、`0<=k<=depth(v)`。`k==0` なら `v` です。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>int lca.kth(v, k) const</code> — O(log N)</summary>

`kth_ancestor(v, k)` の競技中向け短縮名です。

{% raw %}
```cpp
int parent = lca.kth(3, 1);
```
{% endraw %}

注意点: 引数の範囲と返り値は `kth_ancestor` と同じです。

</details>

<details class="api-operation" id="distance-doubling" markdown="1">
<summary><code>int lca.distance(u, v) const</code> — O(log N)</summary>

`u-v` 間の単純パスの辺数を返します。

{% raw %}
```cpp
int edges = lca.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離ではありません。最大値は `N-1` で `int` に収まります。

</details>

<details class="api-operation" id="depth-doubling" markdown="1">
<summary><code>int lca.depth(v) const</code> — O(1)</summary>

根から `v` までの辺数を返します。

{% raw %}
```cpp
int d = lca.depth(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。根の深さは0です。

</details>

<details class="api-operation" id="size-doubling" markdown="1">
<summary><code>int lca.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = lca.size();
```
{% endraw %}

注意点: 構築後に値は変化しません。

</details>

<details class="api-operation" id="construct-rmq" markdown="1">
<summary><code>LowestCommonAncestorRMQ lca(tree, root = 0)</code> — O(N log N)</summary>

長さ `2N-1` の Euler Tour と、深さ最小値を取る Sparse Table を構築します。メモリは O(N log N) です。

{% raw %}
```cpp
blueberry::LowestCommonAncestorRMQ lca(tree, 0);
```
{% endraw %}

注意点: `tree` と `root` の前提はダブリング版と同じです。祖先ジャンプはダブリング版またはHLDを使います。

</details>

<details class="api-operation" id="lca-rmq" markdown="1">
<summary><code>int lca.lca(u, v) const</code> — O(1)</summary>

Euler Tour上の初回出現区間に対するRMQでLCAを返します。

{% raw %}
```cpp
int ancestor = lca.lca(2, 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)`。結果は構築時の根に依存します。

</details>

<details class="api-operation" id="distance-rmq" markdown="1">
<summary><code>int lca.distance(u, v) const</code> — O(1)</summary>

O(1) LCAを用いて `u-v` 間の辺数を返します。

{% raw %}
```cpp
int edges = lca.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離ではありません。最大値は `N-1` です。

</details>

<details class="api-operation" id="depth-rmq" markdown="1">
<summary><code>int lca.depth(v) const</code> — O(1)</summary>

根から `v` までの辺数を返します。

{% raw %}
```cpp
int d = lca.depth(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。根の深さは0です。

</details>

<details class="api-operation" id="size-rmq" markdown="1">
<summary><code>int lca.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = lca.size();
```
{% endraw %}

注意点: 構築後に値は変化しません。

</details>

## 出典・検証

Euler Tour上で隣接要素の深さ差が1になる標準的なLCA→RMQ帰着を、既存の
`blueberry::SparseTable` と組み合わせて独自実装しています。
Library Checker `Lowest Common Ancestor` でダブリング版・RMQ版を検証し、ランダム木では愚直LCAと比較しています。
