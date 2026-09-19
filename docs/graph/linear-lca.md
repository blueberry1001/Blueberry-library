---
title: Linear LCA
documentation_of: //blueberry/graph/linear-lca.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

`blueberry::LinearLCA` は静的な無向木の LCA と辺数距離を O(1) で返す。N 頂点、`1 <= N <= INT_MAX/2`、隣接リストは各無向辺を両方向に1回ずつ含む連結・閉路なしの木とする。根を指定でき、頂点は `[0,N)`。

Euler tour は反復 DFS で構築し、深さ列を LinearRMQ で処理する。構築・メモリ O(N)、各操作 O(1)（word RAM、比較・整数操作 O(1)）。再帰の深さ N を使わない。既存 doubling LCA / Sparse Table LCA は変更しない。k-th ancestor は提供しない。入力木は構築後に破棄可能。辺・根の動的変更不可。距離は int に収まる。返り値は値で参照無効化なし。D は代入先の旧保持要素数。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/linear-lca.hpp"
int main() {
  std::vector<std::vector<int>> g{{1,2},{0,3},{0},{1}};
  blueberry::LinearLCA t(g);
  assert(t.size()==4 && t.lca(2,3)==0);
  assert(t.depth(3)==2 && t.distance(2,3)==3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LinearLCA(const std::vector<std::vector<int>>& tree, int root = 0)` | O(N) | [開く](#constructor) |
| `LinearLCA copy(const LinearLCA& other)` | O(N) | [開く](#copy-constructor) |
| `LinearLCA& operator=(const LinearLCA& other)` | O(N) + D | [開く](#copy-assignment) |
| `LinearLCA moved(LinearLCA&& other)` | O(1) | [開く](#move-constructor) |
| `LinearLCA& operator=(LinearLCA&& other)` | O(1 + D) | [開く](#move-assignment) |
| `int size() const` | O(1) | [開く](#size) |
| `int depth(int v) const` | O(1) | [開く](#depth) |
| `int lca(int u, int v) const` | O(1) | [開く](#lca) |
| `int distance(int u, int v) const` | O(1) | [開く](#distance) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>LinearLCA(const std::vector&lt;std::vector&lt;int&gt;&gt;&amp; tree, int root = 0)</code> — O(N)</summary>

木と根から構築する。

{% raw %}
```cpp
blueberry::LinearLCA t(g,0);
```
{% endraw %}

注意点: 非空の木、0 <= root < N。不正なグラフの受理は保証しない。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>LinearLCA copy(const LinearLCA&amp; other)</code> — O(N)</summary>

全データを独立に複製する。コピー元の更新・破棄はコピー先に影響しない。

{% raw %}
```cpp
LinearLCA copy(other);
```
{% endraw %}

注意点: N はコピー元の要素数。追加メモリは元の構造と同じオーダー。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>LinearLCA&amp; operator=(const LinearLCA&amp; other)</code> — O(N) + D</summary>

元データを破棄・置換し、自身の参照を返す。

{% raw %}
```cpp
object = other;
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己コピー代入は有効。既存の内部参照は無効化される。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>LinearLCA moved(LinearLCA&amp;&amp; other)</code> — O(1)</summary>

所有権を移動する。

{% raw %}
```cpp
LinearLCA moved(std::move(other));
```
{% endraw %}

注意点: <utility> が必要。移動元は再代入または破棄のみとする。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>LinearLCA&amp; operator=(LinearLCA&amp;&amp; other)</code> — O(1 + D)</summary>

旧データを破棄し所有権を移動、自身の参照を返す。

{% raw %}
```cpp
object = std::move(other);
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己ムーブ後も再代入または破棄のみとする。内部参照は無効化される。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

頂点数を返す。

{% raw %}
```cpp
assert(t.size()==4);
```
{% endraw %}

注意点: 木の形は固定。

</details>

<details class="api-operation" id="depth" markdown="1">
<summary><code>int depth(int v) const</code> — O(1)</summary>

根から v までの辺数を返す。

{% raw %}
```cpp
assert(t.depth(3)==2);
```
{% endraw %}

注意点: 0 <= v < N。根は0。

</details>

<details class="api-operation" id="lca" markdown="1">
<summary><code>int lca(int u, int v) const</code> — O(1)</summary>

指定根に関する最近共通祖先を返す。

{% raw %}
```cpp
assert(t.lca(1,3)==1);
```
{% endraw %}

注意点: 0 <= u,v < N。同一頂点なら自身。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>int distance(int u, int v) const</code> — O(1)</summary>

u-v パスの辺数を返す。

{% raw %}
```cpp
assert(t.distance(2,3)==3);
```
{% endraw %}

注意点: 辺重みの和ではない。同一頂点では0。

</details>

## 出典・検証

[Bender–Farach-Colton 論文](https://www3.cs.stonybrook.edu/~bender/newpub/BenderFa00-lca.pdf) の Euler tour/RMQ 帰着と既存 LowestCommonAncestorRMQ を比較。本実装の RMQ 部分は ±1 専用表ではなく LinearRMQ。公式 `lca` を `verify/graph/linear-lca.test.cpp` で検証。ランダム木・星・パスを親登りと BFS 距離で比較する。
