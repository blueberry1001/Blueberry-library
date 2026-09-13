---
title: Heavy Light Decomposition
documentation_of: //blueberry/graph/heavy-light-decomposition.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

静的な根付き木をHeavy-Light Decomposition（HLD）し、頂点・辺のパスを O(log N) 個の半開区間に、
部分木を1個の半開区間に変換します。構築時間・メモリは O(N) です。
値を保持するデータ構造ではないため、ACLの `segtree` / `lazy_segtree` / `fenwick_tree` などと組み合わせます。

N>=1 の連結な木を、無向隣接リストまたは根から辿れる親→子リストで渡します。
頂点 `v` の値は `in(v)`、親から `v` への辺の値も `in(v)` に置きます。根に対応する辺はありません。
`HeavyLightDecomposition` の短縮名として `HLD` も利用できます。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/heavy-light-decomposition.hpp"
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::HLD hld(tree);
  std::vector<int> value{10, 20, 30, 40};
  std::vector<int> base(4);
  for (int v = 0; v < 4; ++v) base[hld.in(v)] = value[v];
  int sum = 0;
  hld.vertex_query(2, 3, [&](int l, int r) {
    for (int i = l; i < r; ++i) sum += base[i];
  });
  assert(sum == 100);
  assert(hld.lca(2, 3) == 0);
  assert(hld.jump(2, 3, 2) == 1);
}
```
{% endraw %}

## 操作一覧

ここで N は頂点数です。区間callback自身の処理時間は表の計算量に含めません。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `HLD hld(tree, root = 0)` | O(N) | [開く](#construct) |
| `int hld.size() const` | O(1) | [開く](#size) |
| `int hld.root() const` | O(1) | [開く](#root) |
| `int hld.parent(v) const` | O(1) | [開く](#parent) |
| `int hld.depth(v) const` | O(1) | [開く](#depth) |
| `int hld.subtree_size(v) const` | O(1) | [開く](#subtree-size) |
| `int hld.head(v) const` | O(1) | [開く](#head) |
| `int hld.in(v) const` | O(1) | [開く](#in) |
| `int hld.out(v) const` | O(1) | [開く](#out) |
| `int hld.vertex_at(position) const` | O(1) | [開く](#vertex-at) |
| `int hld.edge_index(u, v) const` | O(1) | [開く](#edge-index) |
| `pair<int,int> hld.subtree_range(v, edge = false) const` | O(1) | [開く](#subtree-range) |
| `bool hld.is_ancestor(a, v) const` | O(1) | [開く](#is-ancestor) |
| `int hld.lca(u, v) const` | O(log N) | [開く](#lca) |
| `int hld.kth_ancestor(v, k) const` | O(log N) | [開く](#kth-ancestor) |
| `int hld.kth(v, k) const` | O(log N) | [開く](#kth) |
| `int hld.distance(u, v) const` | O(log N) | [開く](#distance) |
| `int hld.jump(u, v, k) const` | O(log N) | [開く](#jump) |
| `hld.path_query(u, v, f, edge = false) const` | O(log N) callbacks | [開く](#path-query) |
| `hld.vertex_query(u, v, f) const` | O(log N) callbacks | [開く](#vertex-path-query) |
| `hld.edge_query(u, v, f) const` | O(log N) callbacks | [開く](#edge-query) |
| `hld.vertex_query(v, f) const` | O(1) callback | [開く](#single-vertex-query) |
| `hld.subtree_query(v, f, edge = false) const` | O(1) callback | [開く](#subtree-query) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>HLD hld(tree, root = 0)</code> — O(N)</summary>

親・深さ・部分木サイズ・heavy child・Euler順を反復処理で構築します。メモリは O(N) です。

{% raw %}
```cpp
blueberry::HLD hld(tree, 0);
```
{% endraw %}

注意点: `tree` は N>=1 の連結な木、`root` は `[0,N)`。構築後の木の変更は反映されません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int hld.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = hld.size();
```
{% endraw %}

注意点: 構築後に値は変化しません。

</details>

<details class="api-operation" id="root" markdown="1">
<summary><code>int hld.root() const</code> — O(1)</summary>

構築時に指定した根を返します。

{% raw %}
```cpp
int root = hld.root();
```
{% endraw %}

注意点: 根の変更には再構築が必要です。

</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>int hld.parent(v) const</code> — O(1)</summary>

`v` の親を返します。

{% raw %}
```cpp
int p = hld.parent(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。根については根自身を返します。

</details>

<details class="api-operation" id="depth" markdown="1">
<summary><code>int hld.depth(v) const</code> — O(1)</summary>

根から `v` までの辺数を返します。

{% raw %}
```cpp
int d = hld.depth(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。根の深さは0です。

</details>

<details class="api-operation" id="subtree-size" markdown="1">
<summary><code>int hld.subtree_size(v) const</code> — O(1)</summary>

`v` 自身を含む部分木の頂点数を返します。

{% raw %}
```cpp
int count = hld.subtree_size(1);
```
{% endraw %}

注意点: 部分木は構築時の根に依存します。

</details>

<details class="api-operation" id="head" markdown="1">
<summary><code>int hld.head(v) const</code> — O(1)</summary>

`v` が属するheavy pathの、根に最も近い頂点を返します。

{% raw %}
```cpp
int chain_head = hld.head(3);
```
{% endraw %}

注意点: heavy childが同サイズの場合の選択は隣接リスト順に依存します。

</details>

<details class="api-operation" id="in" markdown="1">
<summary><code>int hld.in(v) const</code> — O(1)</summary>

`v` のHLD配列上の位置を返します。

{% raw %}
```cpp
int position = hld.in(3);
```
{% endraw %}

注意点: 返り値は `[0,N)`。同じ位置を親から `v` への辺にも使います。

</details>

<details class="api-operation" id="out" markdown="1">
<summary><code>int hld.out(v) const</code> — O(1)</summary>

`v` の部分木区間 `[in(v),out(v))` の右端を返します。

{% raw %}
```cpp
int right = hld.out(1);
```
{% endraw %}

注意点: `out(v)` 自体は区間に含まれず、値域は `[1,N]` です。

</details>

<details class="api-operation" id="vertex-at" markdown="1">
<summary><code>int hld.vertex_at(position) const</code> — O(1)</summary>

HLD配列の `position` に置かれた頂点を返します。`in` の逆写像です。

{% raw %}
```cpp
int v = hld.vertex_at(hld.in(3));
```
{% endraw %}

注意点: `position` は `[0,N)` です。

</details>

<details class="api-operation" id="edge-index" markdown="1">
<summary><code>int hld.edge_index(u, v) const</code> — O(1)</summary>

隣接する `u,v` を結ぶ辺を、深い側の頂点の `in` に写します。

{% raw %}
```cpp
int position = hld.edge_index(1, 3);
```
{% endraw %}

注意点: `u,v` は木の辺の両端でなければなりません。根に対応する辺位置はありません。

</details>

<details class="api-operation" id="subtree-range" markdown="1">
<summary><code>pair&lt;int,int&gt; hld.subtree_range(v, edge = false) const</code> — O(1)</summary>

頂点部分木なら `[in(v),out(v))`、`edge=true` なら `v` の親辺を除いた `[in(v)+1,out(v))` を返します。

{% raw %}
```cpp
auto [left, right] = hld.subtree_range(1);
```
{% endraw %}

注意点: `edge=true` の葉では空区間になります。区間は常に半開区間です。

</details>

<details class="api-operation" id="is-ancestor" markdown="1">
<summary><code>bool hld.is_ancestor(a, v) const</code> — O(1)</summary>

`a` が `v` の祖先（自身を含む）なら `true` を返します。

{% raw %}
```cpp
bool contains = hld.is_ancestor(1, 3);
```
{% endraw %}

注意点: `a,v` は `[0,N)`。関係は構築時の根に依存します。

</details>

<details class="api-operation" id="lca" markdown="1">
<summary><code>int hld.lca(u, v) const</code> — O(log N)</summary>

heavy pathを上り、`u,v` のLCAを返します。

{% raw %}
```cpp
int ancestor = hld.lca(2, 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)`。結果は構築時の根に依存します。

</details>

<details class="api-operation" id="kth-ancestor" markdown="1">
<summary><code>int hld.kth_ancestor(v, k) const</code> — O(log N)</summary>

`v` から親方向へ `k` 辺進んだ祖先を返します。

{% raw %}
```cpp
int parent = hld.kth_ancestor(3, 1);
```
{% endraw %}

注意点: `0<=k<=depth(v)`。`k==0` なら `v` です。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>int hld.kth(v, k) const</code> — O(log N)</summary>

`kth_ancestor(v, k)` の競技中向け短縮名です。

{% raw %}
```cpp
int parent = hld.kth(3, 1);
```
{% endraw %}

注意点: 引数の範囲と返り値は `kth_ancestor` と同じです。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>int hld.distance(u, v) const</code> — O(log N)</summary>

`u-v` 間の単純パスの辺数を返します。

{% raw %}
```cpp
int edges = hld.distance(2, 3);
```
{% endraw %}

注意点: 重み付き距離ではありません。最大値は `N-1` です。

</details>

<details class="api-operation" id="jump" markdown="1">
<summary><code>int hld.jump(u, v, k) const</code> — O(log N)</summary>

`u` から `v` への単純パス上で、`u` を0番目とする `k` 番目の頂点を返します。

{% raw %}
```cpp
int middle = hld.jump(2, 3, 2);
```
{% endraw %}

注意点: `0<=k<=distance(u,v)`。範囲外なら呼び出し側で判定してください。

</details>

<details class="api-operation" id="path-query" markdown="1">
<summary><code>hld.path_query(u, v, f, edge = false) const</code> — O(log N) callbacks</summary>

`u` から `v` へのパスを半開区間に分け、パス順に `f(left,right)` または
`f(left,right,reversed)` を呼びます。3引数版の `reversed=true` は `[left,right)` を右から左へ辿る区間です。
`edge=true` ではLCAの位置だけを除き、親辺を深い側の頂点位置で表します。

{% raw %}
```cpp
hld.path_query(u, v, [&](int left, int right, bool reversed) {
  // reversedに応じて正順・逆順の区間積を取得する
});
```
{% endraw %}

注意点: 2引数callbackでは向きが渡らないため、可換な集約または範囲更新に使います。callbackの参照は呼び出し中だけ使用されます。

</details>

<details class="api-operation" id="vertex-path-query" markdown="1">
<summary><code>hld.vertex_query(u, v, f) const</code> — O(log N) callbacks</summary>

`path_query(u,v,f,false)` の明示的な頂点パス版で、両端とLCAを含みます。

{% raw %}
```cpp
hld.vertex_query(2, 3, [&](int left, int right) {
  answer += seg.prod(left, right);
});
```
{% endraw %}

注意点: 非可換な積では3引数callbackを使い、`reversed` に応じた区間積を用意してください。

</details>

<details class="api-operation" id="edge-query" markdown="1">
<summary><code>hld.edge_query(u, v, f) const</code> — O(log N) callbacks</summary>

`path_query(u,v,f,true)` の辺パス版です。各辺は深い側の頂点の `in` で表します。

{% raw %}
```cpp
hld.edge_query(2, 3, [&](int left, int right) {
  answer += seg.prod(left, right);
});
```
{% endraw %}

注意点: `u==v` では空パスとなりcallbackを呼びません。非可換なら3引数callbackを使います。

</details>

<details class="api-operation" id="single-vertex-query" markdown="1">
<summary><code>hld.vertex_query(v, f) const</code> — O(1) callback</summary>

1頂点を表す `[in(v),in(v)+1)` についてcallbackを1回呼びます。

{% raw %}
```cpp
hld.vertex_query(3, [&](int left, int right) {
  seg.apply(left, right, update);
});
```
{% endraw %}

注意点: 3引数callbackを渡した場合の `reversed` は `false` です。

</details>

<details class="api-operation" id="subtree-query" markdown="1">
<summary><code>hld.subtree_query(v, f, edge = false) const</code> — O(1) callback</summary>

`subtree_range(v,edge)` が空でなければ、その区間についてcallbackを1回呼びます。

{% raw %}
```cpp
hld.subtree_query(1, [&](int left, int right) {
  answer = seg.prod(left, right);
});
```
{% endraw %}

注意点: `edge=true` の葉ではcallbackを呼びません。3引数callbackの `reversed` は `false` です。

</details>

## 出典・検証

API設計では [NyaanNyaan/library](https://github.com/NyaanNyaan/library) の頂点/辺query分離、
[ei1333/library](https://github.com/ei1333/library/blob/master/graph/tree/heavy-light-decomposition.hpp) の短い `la`・順序付き集約、
[sotanishy/cp-library-cpp](https://github.com/sotanishy/cp-library-cpp/blob/main/tree/hld.hpp) の非可換パス積を比較しました。
実装は入力グラフを書き換えず、深い木でも再帰DFSでスタックオーバーフローしない反復構築として独自に作成しています。

Library Checkerの `Vertex Add Path Sum`、`Vertex Add Subtree Sum`、`Vertex Set Path Composite`、
`Jump on Tree` で検証し、ランダム木では全操作を愚直解と比較しています。
