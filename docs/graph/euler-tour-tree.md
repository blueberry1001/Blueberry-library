---
title: Euler Tour Tree
documentation_of: //blueberry/graph/euler-tour-tree.hpp
---

## 概要・前提

`blueberry::EulerTourTree<S, op, e, F, mapping, composition, id>` は動的森の部分木積・部分木作用を扱います。
各頂点一つと各辺二つの Euler tour 要素を splay sequence に保存します。
削除した辺ノードは再利用するため、操作回数によらず O(N) メモリです。ACL に動的森はありません。
パス積には Link-Cut Tree を使用してください。閉路を持つ一般グラフの動的連結性は対象外です。

N は固定頂点数、`N <= INT_MAX/3`。空配列も構築できます。`op` は可換モノイド、`e()` は単位元。
`mapping(f,x,count)` は count 個の頂点の集約 x に f を作用させた結果です。
`mapping(f,e(),0) == e()`、結合に対する分配性が必要です。
`composition(f,g)` は g の後に f、`id()` は恒等作用です。F の比較演算は不要です。
演算・値コピー・破棄が O(1) とした計算量を示します。操作は償却 O(log N)、単発最悪 O(N) です。
整数の積や集約値が型の範囲に収まること、演算が例外を投げないことは呼び出し側の前提です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/euler-tour-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(long long f, long long x, int n) { return x + f * n; }
long long composition(long long f, long long g) { return f + g; }
long long id() { return 0; }
int main() {
  blueberry::EulerTourTree<long long, op, e, long long, mapping, composition, id> t({1, 2, 3});
  assert(t.link(0, 1));
  assert(t.link(1, 2));
  assert(t.prod(1, 0) == 5);
  t.apply(1, 0, 10);
  assert(t.prod(0) == 26);
  assert(t.get(0) == 1);
  assert(t.cut(1, 2));
  assert(t.prod(2) == 13);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `EulerTourTree(const vector<S>& values = {})` | O(N) | [開く](#construct) |
| `int size() const` | O(1) | [開く](#size) |
| `bool same(int u, int v)` | 償却 O(log(N+1)) | [開く](#same) |
| `bool link(int u, int v)` | 償却 O(log(N+1)) | [開く](#link) |
| `bool cut(int u, int v)` | 償却 O(log(N+1)) | [開く](#cut) |
| `void set(int v, const S& x)` | 償却 O(log(N+1)) | [開く](#set) |
| `S get(int v)` | 償却 O(log(N+1)) | [開く](#get) |
| `S prod(int v, int parent = -1)` | 償却 O(log(N+1)) | [開く](#prod) |
| `void apply(int v, int parent, const F& f)` | 償却 O(log(N+1)) | [開く](#apply) |
| `EulerTourTree(const EulerTourTree& other)` | O(N) | [開く](#copy) |
| `EulerTourTree& operator=(const EulerTourTree& other)` | O(N+D) | [開く](#copy-assign) |
| `EulerTourTree(EulerTourTree&& other)` | O(1) | [開く](#move) |
| `EulerTourTree& operator=(EulerTourTree&& other)` | O(D) | [開く](#move-assign) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>EulerTourTree(const vector&lt;S&gt;&amp; values = {})</code> — O(N)</summary>

N 個の孤立点を構築します。空配列・引数省略も可能です。

{% raw %}
```cpp
blueberry::EulerTourTree<long long, op, e, long long, mapping, composition, id> t({1, 2, 3});
```
{% endraw %}

注意点: N <= INT_MAX/3。値はコピーされ、元配列の寿命に依存しません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

固定の頂点数を返します。

{% raw %}
```cpp
assert(t.size() == 3);
```
{% endraw %}

注意点: 構築後の頂点追加・削除は扱いません。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool same(int u, int v)</code> — 償却 O(log(N+1))</summary>

同一連結成分なら true。u == v も true です。

{% raw %}
```cpp
bool connected = t.same(0, 1);
```
{% endraw %}

注意点: 0 <= u,v < N。内部の splay sequence は変化します。

</details>

<details class="api-operation" id="link" markdown="1">
<summary><code>bool link(int u, int v)</code> — 償却 O(log(N+1))</summary>

異なる木を辺 (u,v) で接続し true。既に連結なら false で辺集合は変更しません。

{% raw %}
```cpp
assert(t.link(0, 1));
```
{% endraw %}

注意点: 自己辺・既存辺・閉路を作る辺も false。一般グラフの閉路を保存する機能ではありません。

</details>

<details class="api-operation" id="cut" markdown="1">
<summary><code>bool cut(int u, int v)</code> — 償却 O(log(N+1))</summary>

存在する辺 (u,v) を削除して true。辺がなければ false。

{% raw %}
```cpp
assert(t.cut(0, 1));
```
{% endraw %}

注意点: 単に連結なだけでは削除しません。頂点番号は有効である必要があります。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void set(int v, const S& x)</code> — 償却 O(log(N+1))</summary>

頂点 v の値を x で置き換えます。

{% raw %}
```cpp
t.set(0, 5);
```
{% endraw %}

注意点: 0 <= v < N。加算なら t.set(v, t.get(v) + delta)。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>S get(int v)</code> — 償却 O(log(N+1))</summary>

遅延作用を反映した現在の頂点値を返します。

{% raw %}
```cpp
auto value = t.get(0);
```
{% endraw %}

注意点: 0 <= v < N。S のコピーを返します。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S prod(int v, int parent = -1)</code> — 償却 O(log(N+1))</summary>

parent == -1 なら連結成分全体、それ以外なら辺 (v,parent) を境に v 側の部分木の積を返します。

{% raw %}
```cpp
auto component_sum = t.prod(0);
auto subtree_sum = t.prod(1, 0);
```
{% endraw %}

注意点: parent != -1 なら既存の隣接頂点を指定してください。辺は内部で一時切断・再接続され、終了時の辺集合は変わりません。

</details>

<details class="api-operation" id="apply" markdown="1">
<summary><code>void apply(int v, int parent, const F& f)</code> — 償却 O(log(N+1))</summary>

prod と同じ範囲の全頂点に f を作用させます。

{% raw %}
```cpp
t.apply(0, -1, 5);
t.apply(1, 0, 2);
```
{% endraw %}

注意点: parent == -1 は成分全体。それ以外は既存の隣接頂点が必要です。mapping / composition は例外を投げない前提です。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>EulerTourTree(const EulerTourTree& other)</code> — O(N)</summary>

独立な森林をコピーします。

{% raw %}
```cpp
auto copy = t;
```
{% endraw %}

注意点: 頂点番号は同じですが、その後の辺・値更新は独立です。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>EulerTourTree& operator=(const EulerTourTree& other)</code> — O(N+D)</summary>

other の森林で置き換え、自身への参照を返します。

{% raw %}
```cpp
copy = t;
```
{% endraw %}

注意点: N はコピー元、D はコピー先の旧頂点数。自己代入可能。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>EulerTourTree(EulerTourTree&& other)</code> — O(1)</summary>

vector の所有権を移動します。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元は破棄・再代入のみを保証します。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>EulerTourTree& operator=(EulerTourTree&& other)</code> — O(D)</summary>

旧格納値を破棄して所有権を移動します。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: D は移動先の旧頂点数。移動元は破棄・再代入のみを保証します。

</details>

## 出典・検証

- [Euler Tour Tree（sotanishy）](https://sotanishy.github.io/cp-library-cpp/tree/euler_tour_tree.hpp.html): Euler tour と部分木の切断・復元の設計を比較。コードの転載はしていません。
- [Euler Tour Tree（OI Wiki）](https://oi-wiki.org/ds/ett/): 平衡二分木上の巡回列による表現。
- [Dynamic Tree Subtree Add Subtree Sum](https://judge.yosupo.jp/problem/dynamic_tree_subtree_add_subtree_sum)
- [Dynamic Tree Vertex Add Subtree Sum](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum)

ランダム比較では森林の BFS を用いて成分・部分木・点更新を照合します。作用には加算だけでなく順序に依存する affine 作用を使い、composition の順序を検証します。

