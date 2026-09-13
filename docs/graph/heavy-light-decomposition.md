---
title: Heavy-Light Decomposition
documentation_of: //blueberry/graph/heavy-light-decomposition.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

根付き木を重い辺・軽い辺に分け、頂点を一列に並べます。各部分木は連続した区間
`[in(v), out(v))` になり、任意のパスは $O(\log N)$ 個の区間に分解されます。
構築は $O(N)$ 時間・$O(N)$ メモリです。区間の和・更新には ACL の `segtree` や
`lazy_segtree` を組み合わせてください。

`path_query` は可換な演算向けで、コールバックを $O(\log N)$ 回呼びます。各コールバック内の
区間データ構造を $O(\log N)$ とすれば、パスクエリ全体は $O(\log^2 N)$ です。
`vertex=false` の場合は LCA を除くため、辺の値を子頂点の位置に置く実装にも使えます。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/heavy-light-decomposition.hpp"
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  std::vector<int> value{1, 2, 3, 4};
  blueberry::HeavyLightDecomposition hld(tree);
  int path_sum = 0;
  hld.path_query(2, 3, [&](int l, int r) {
    for (int i = l; i < r; ++i) path_sum += value[hld.vertex_at(i)];
  });
  assert(path_sum == 10);
  int subtree_sum = 0;
  hld.subtree_query(1, [&](int l, int r) {
    for (int i = l; i < r; ++i) subtree_sum += value[hld.vertex_at(i)];
  });
  assert(subtree_sum == 6);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `HeavyLightDecomposition hld(tree, root = 0)` | O(N) | [開く](#construct) |
| `int hld.size() const` | O(1) | [開く](#size) |
| `int hld.root() const` | O(1) | [開く](#root) |
| `int hld.parent(v) const` | O(1) | [開く](#parent) |
| `int hld.depth(v) const` | O(1) | [開く](#depth) |
| `int hld.head(v) const` | O(1) | [開く](#head) |
| `int hld.in(v) const` | O(1) | [開く](#in) |
| `int hld.out(v) const` | O(1) | [開く](#out) |
| `int hld.vertex_at(pos) const` | O(1) | [開く](#vertex-at) |
| `bool hld.is_ancestor(a, b) const` | O(1) | [開く](#is-ancestor) |
| `int hld.lca(u, v) const` | O(log N) | [開く](#lca) |
| `int hld.kth_ancestor(v, k) const` | O(log N) | [開く](#kth-ancestor) |
| `int hld.kth(v, k) const` | O(log N) | [開く](#kth) |
| `int hld.distance(u, v) const` | O(log N) | [開く](#distance) |
| `hld.path_query(u, v, f, vertex = true)` | O(log N) callbacks | [開く](#path-query) |
| `hld.subtree_query(v, f, vertex = true)` | O(1) callbacks | [開く](#subtree-query) |
| `hld.vertex_query(v, f)` | O(1) callbacks | [開く](#vertex-query) |

以下の操作を開くと、返り値・使用例・注意点を確認できます。断片の例は必要なヘッダと
有効な `hld` がある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>HeavyLightDecomposition hld(tree, root = 0)</code> — O(N)</summary>

重い子を先に辿る preorder と、各頂点の鎖の先頭を構築します。

{% raw %}
```cpp
blueberry::HeavyLightDecomposition hld(tree, 0);
```
{% endraw %}

注意点: 空・非連結・サイクルを含む入力は対象外です。構築後に木を変更しても分解は更新されません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int hld.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = hld.size();
```
{% endraw %}

注意点: 構築後に変化しません。

</details>

<details class="api-operation" id="root" markdown="1">
<summary><code>int hld.root() const</code> — O(1)</summary>

指定した根の頂点番号を返します。

{% raw %}
```cpp
int root = hld.root();
```
{% endraw %}

注意点: 根の変更には再構築が必要です。

</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>int hld.parent(v) const</code> — O(1)</summary>

根付き木での v の親を返します。根自身の親は根です。

{% raw %}
```cpp
int p = hld.parent(3);
```
{% endraw %}

注意点: `v` は `[0, N)` に限ります。

</details>

<details class="api-operation" id="depth" markdown="1">
<summary><code>int hld.depth(v) const</code> — O(1)</summary>

根から v までの辺数を返します。

{% raw %}
```cpp
int d = hld.depth(3);
```
{% endraw %}

注意点: 根の深さは 0 です。

</details>

<details class="api-operation" id="head" markdown="1">
<summary><code>int hld.head(v) const</code> — O(1)</summary>

v が属する重い鎖の先頭頂点を返します。

{% raw %}
```cpp
int chain_head = hld.head(3);
```
{% endraw %}

注意点: これは頂点番号であり、配列上の位置ではありません。

</details>

<details class="api-operation" id="in" markdown="1">
<summary><code>int hld.in(v) const</code> — O(1)</summary>

v の preorder 位置を返します。頂点 v の値は通常この位置に格納します。

{% raw %}
```cpp
int position = hld.in(3);
```
{% endraw %}

注意点: 位置は `[0, N)` です。分解後に並び順を変更できません。

</details>

<details class="api-operation" id="out" markdown="1">
<summary><code>int hld.out(v) const</code> — O(1)</summary>

v の部分木に対応する半開区間の右端を返します。部分木は `[in(v), out(v))` です。

{% raw %}
```cpp
int right = hld.out(1);
```
{% endraw %}

注意点: `in(v) < out(v)` で、右端は含みません。

</details>

<details class="api-operation" id="vertex-at" markdown="1">
<summary><code>int hld.vertex_at(pos) const</code> — O(1)</summary>

分解配列の位置 pos に対応する頂点番号を返します。

{% raw %}
```cpp
int vertex = hld.vertex_at(position);
```
{% endraw %}

注意点: `0 <= pos < N` が必要です。

</details>

<details class="api-operation" id="is-ancestor" markdown="1">
<summary><code>bool hld.is_ancestor(a, b) const</code> — O(1)</summary>

a が b の祖先（a 自身を含む）かを判定します。

{% raw %}
```cpp
bool ok = hld.is_ancestor(1, 3);
```
{% endraw %}

注意点: 根付き方に依存します。

</details>

<details class="api-operation" id="lca" markdown="1">
<summary><code>int hld.lca(u, v) const</code> — O(log N)</summary>

鎖を上へ辿り、u と v の最小共通祖先を返します。

{% raw %}
```cpp
int ancestor = hld.lca(2, 3);
```
{% endraw %}

注意点: 大量の LCA だけが必要なら `LowestCommonAncestorRMQ` の O(1) クエリも検討してください。

</details>

<details class="api-operation" id="kth-ancestor" markdown="1">
<summary><code>int hld.kth_ancestor(v, k) const</code> — O(log N)</summary>

v から親方向へ k 本進んだ頂点を返します。

{% raw %}
```cpp
int ancestor = hld.kth_ancestor(3, 1);
```
{% endraw %}

注意点: `0 <= k <= depth(v)` が必要です。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>int hld.kth(v, k) const</code> — O(log N)</summary>

`kth_ancestor` の短い別名です。vから親方向へ k 本進んだ頂点を返します。

{% raw %}
```cpp
int ancestor = hld.kth(3, 1);
```
{% endraw %}

注意点: `0 <= k <= depth(v)` が必要です。返り値・計算量は `kth_ancestor` と同じです。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>int hld.distance(u, v) const</code> — O(log N)</summary>

u-v 間の単純パスの辺数を返します。

{% raw %}
```cpp
int edges = hld.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離ではありません。

</details>

<details class="api-operation" id="path-query" markdown="1">
<summary><code>hld.path_query(u, v, f, vertex = true)</code> — O(log N) callbacks</summary>

u-v パスを半開区間に分解し、各区間 `[l, r)` を `f(l, r)` に渡します。`vertex=true`
なら両端を含み、`vertex=false` なら LCA を除きます。

{% raw %}
```cpp
hld.path_query(2, 3, [&](int l, int r) {
  // segtree.prod(l, r) などをここで処理する
});
```
{% endraw %}

注意点: コールバック順は未規定なので、可換な演算に限ります。順序が必要な文字列結合などは、
区間の向きを別途扱う実装を用意してください。区間データ構造が O(log N) なら全体は O(log² N) です。

</details>

<details class="api-operation" id="subtree-query" markdown="1">
<summary><code>hld.subtree_query(v, f, vertex = true)</code> — O(1) callbacks</summary>

v の部分木に対応する `[in(v), out(v))` を `f` に渡します。`vertex=false` なら v 自身を除きます。

{% raw %}
```cpp
hld.subtree_query(1, [&](int l, int r) {
  // segtree.prod(l, r) などをここで処理する
});
```
{% endraw %}

注意点: 部分木の値を分解配列上で管理する必要があります。v が葉で edge モードならコールバックは呼ばれません。

</details>

<details class="api-operation" id="vertex-query" markdown="1">
<summary><code>hld.vertex_query(v, f)</code> — O(1) callbacks</summary>

頂点 v の一点区間 `[in(v), in(v)+1)` を `f` に渡します。

{% raw %}
```cpp
hld.vertex_query(3, [&](int l, int r) { /* 一点更新 */ });
```
{% endraw %}

注意点: 区間の右端は含みません。更新の整合性は組み合わせるデータ構造側で管理します。

</details>
