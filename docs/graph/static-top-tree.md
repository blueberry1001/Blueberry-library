---
title: Static Top Tree
documentation_of: //blueberry/graph/static-top-tree.hpp
---

## 概要・前提

`blueberry::StaticTopTree<V, Path, Point, add_vertex, add_edge, compress, rake, point_identity>` は固定された根付き木の DP を頂点更新後 O(log N) で再計算します。
ACL の区間データ構造では木の分岐の DP をそのまま扱えないため追加しています。動的な辺の変更は別の Dynamic Top Tree の用途です。

heavy path の直列結合を compress、light subtree の並列結合を rake として二分式木にします。
元の部分木サイズによる weighted median で構築するため、同じ重さの葉数で分割する方法と異なり、式木の高さは O(log N) です。
格納量 O(N)、構築 O(N log(N+1)) 以下、set は最悪 O(log(N+1))。callback・値コピー・破棄が O(1) の場合です。
Path / Point はデフォルト構築・コピー可能、V はコピー可能としてください。空木は不可。算術のオーバーフローは呼び出し側で防ぎます。

### callback の意味

| callback | 意味 |
| --- | --- |
| `Path add_vertex(V value, Point light)` | 頂点値と light child 側の集約から、一頂点の path cluster を作る |
| `Point add_edge(Path path)` | 完成した light subtree の path cluster を親側への寄与へ変換する |
| `Path compress(Path top, Path bottom)` | 上から下の順に path cluster を結合する。結合的であること |
| `Point rake(Point a, Point b)` | 同一親へ接続される light subtree の寄与を結合する。可換・結合的であること |
| `Point point_identity()` | light subtree がない場合の単位元 |

同じ木を表す結合順序で同じ答えになる木 DP の代数を与えてください。単に型が一致するだけでは十分ではありません。
重い子の選択は内部処理なので、light / heavy の選択で答えが変わらないことも必要です。
葉と非葉で DP が異なる場合や辺の値を扱う場合は、その情報を V に含められます。
各頂点は一度だけ add_vertex に含まれ、境界の頂点を重複して加算しません。

## 最小使用例

次は `dp[v] = a[v] * sum(dp[child]) + b[v]` を扱います。
heavy child の答えを変数とした affine 関数を Path に保存し、末尾は 0 を代入します。

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/static-top-tree.hpp"
struct Affine { long long a, b; };
Affine vertex(Affine v, long long light) { return {v.a, v.a * light + v.b}; }
long long edge(Affine p) { return p.b; }
Affine compress(Affine x, Affine y) { return {x.a * y.a, x.a * y.b + x.b}; }
long long rake(long long x, long long y) { return x + y; }
long long identity() { return 0; }
using Tree = blueberry::StaticTopTree<Affine, Affine, long long, vertex, edge, compress, rake, identity>;
int main() {
  std::vector<std::vector<int>> graph{{1, 2}, {0}, {0}};
  std::vector<Affine> values{{2, 1}, {1, 3}, {1, 4}};
  Tree t(graph, values);
  assert(t.all_prod().b == 15);
  t.set(1, {1, 5});
  assert(t.all_prod().b == 19);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `StaticTopTree(const vector<vector<int>>& tree, const vector<V>& values, int root = 0)` | O(N log(N+1)) | [開く](#construct) |
| `int size() const` | O(1) | [開く](#size) |
| `V get(int v) const` | O(1) | [開く](#get) |
| `void set(int v, const V& x)` | O(log(N+1)) | [開く](#set) |
| `Path all_prod() const` | O(1) | [開く](#all-prod) |
| `StaticTopTree(const StaticTopTree& other)` | O(N) | [開く](#copy) |
| `StaticTopTree& operator=(const StaticTopTree& other)` | O(N+D) | [開く](#copy-assign) |
| `StaticTopTree(StaticTopTree&& other)` | O(1) | [開く](#move) |
| `StaticTopTree& operator=(StaticTopTree&& other)` | O(D) | [開く](#move-assign) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>StaticTopTree(const vector&lt;vector&lt;int&gt;&gt;&amp; tree, const vector&lt;V&gt;&amp; values, int root = 0)</code> — O(N log(N+1))</summary>

無向連結木と値から固定根の rake/compress 木を構築します。

{% raw %}
```cpp
Tree t(graph, values, 0);
```
{% endraw %}

注意点: 1 <= N <= INT_MAX/4、tree.size()==values.size()。重辺・自己辺・閉路は禁止。入力はコピーされ寿命に依存しません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

頂点数を返します。

{% raw %}
```cpp
assert(t.size() == 3);
```
{% endraw %}

注意点: 構造は固定です。辺の追加・削除や reroot は扱いません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>V get(int v) const</code> — O(1)</summary>

頂点の現在値をコピーして返します。

{% raw %}
```cpp
auto value = t.get(0);
```
{% endraw %}

注意点: 0 <= v < N。集約値ではなく入力値です。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void set(int v, const V& x)</code> — O(log(N+1))</summary>

頂点値を代入し、依存する cluster を再評価します。

{% raw %}
```cpp
t.set(0, {1, 5});
```
{% endraw %}

注意点: 0 <= v < N。callback が外部の可変状態を読む設計は避けてください。更新対象以外を変えると保存集約と不整合になります。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>Path all_prod() const</code> — O(1)</summary>

固定根から始まる木全体の path cluster をコピーして返します。

{% raw %}
```cpp
auto result = t.all_prod();
```
{% endraw %}

注意点: 最終的な答えの取り出し方は Path の設計によります。例では result.b。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>StaticTopTree(const StaticTopTree& other)</code> — O(N)</summary>

独立な構造・値をコピーします。

{% raw %}
```cpp
auto copy = t;
```
{% endraw %}

注意点: 以降の set は他方に影響しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>StaticTopTree& operator=(const StaticTopTree& other)</code> — O(N+D)</summary>

コピーして自身への参照を返します。

{% raw %}
```cpp
copy = t;
```
{% endraw %}

注意点: N はコピー元、D はコピー先の旧頂点数。自己代入可能。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>StaticTopTree(StaticTopTree&& other)</code> — O(1)</summary>

格納配列を移動します。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元は破棄・再代入のみ保証します。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>StaticTopTree& operator=(StaticTopTree&& other)</code> — O(D)</summary>

旧値を破棄して格納配列を移動します。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: D は移動先の旧頂点数。移動元は破棄・再代入のみ保証します。

</details>

## 出典・検証

- [ABC351 G 公式解説](https://atcoder.jp/contests/abc351/editorial/9899): rake/compress による固定木の DP。
- [Nyaan Static Top Tree](https://nyaannyaan.github.io/library/tree/static-top-tree-vertex-based.hpp.html): weighted balancing と cluster API を比較。
- [Point Set Tree Path Composite Sum (Fixed Root)](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root): 頂点値と辺の affine 関数の更新。

ランダム DP 比較に加え、10 万頂点の鎖・星・二分木で再帰の深さと更新 callback 数の境界を検査します。
