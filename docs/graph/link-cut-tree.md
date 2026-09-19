---
title: Link-Cut Tree
documentation_of: //blueberry/graph/link-cut-tree.hpp
---

## 概要・前提

`blueberry::LinkCutTree<S, op, e>` は辺の接続・切断と頂点値のパス積を扱う動的森です。
Splay tree で preferred path を管理し、正逆両方向の積を保持するため非可換モノイドにも対応します。
ACL には動的森がないため独立実装です。部分木集約・パス一括更新はこの API には含みません。

`op(S,S)` は結合的、`e()` は両側単位元。演算・S のコピーと破棄を O(1) とした計算量です。
N は固定頂点数、格納メモリと一時スタックは O(N)。操作の O(log N) は償却であり、単発最悪 O(N) です。
値・集約値のオーバーフローは呼び出し側で防いでください。辺を切っても頂点番号は変化しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/link-cut-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  blueberry::LinkCutTree<long long, op, e> t({1, 2, 3});
  assert(t.link(0, 1));
  assert(t.link(1, 2));
  assert(t.prod(0, 2) == 6);
  t.set(1, 10);
  assert(t.prod(2, 0) == 14);
  assert(t.cut(0, 1));
  assert(!t.same(0, 2));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LinkCutTree(const vector<S>& values = {})` | O(N) | [開く](#construct) |
| `void evert(int v)` | 償却 O(log(N+1)) | [開く](#evert) |
| `int leader(int v)` | 償却 O(log(N+1)) | [開く](#leader) |
| `int size() const` | O(1) | [開く](#size) |
| `bool same(int u, int v)` | 償却 O(log(N+1)) | [開く](#same) |
| `bool link(int u, int v)` | 償却 O(log(N+1)) | [開く](#link) |
| `bool cut(int u, int v)` | 償却 O(log(N+1)) | [開く](#cut) |
| `S get(int v) const` | O(1) | [開く](#get) |
| `void set(int v, const S& x)` | 償却 O(log(N+1)) | [開く](#set) |
| `S prod(int u, int v)` | 償却 O(log(N+1)) | [開く](#prod) |
| `LinkCutTree(const LinkCutTree& other)` | O(N) | [開く](#copy) |
| `LinkCutTree& operator=(const LinkCutTree& other)` | O(N+D) | [開く](#copy-assign) |
| `LinkCutTree(LinkCutTree&& other)` | O(1) | [開く](#move) |
| `LinkCutTree& operator=(LinkCutTree&& other)` | O(D) | [開く](#move-assign) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>LinkCutTree(const vector<S>& values = {})</code> — O(N)</summary>

N 個の孤立点を構築します。空配列・引数省略も可能です。

{% raw %}
```cpp
blueberry::LinkCutTree<long long, op, e> t({1, 2, 3});
```
{% endraw %}

注意点: N <= INT_MAX。値はコピーされ、元配列の寿命に依存しません。

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

注意点: 0 <= u,v < N。内部の preferred path は変化します。

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

<details class="api-operation" id="get" markdown="1">
<summary><code>S get(int v) const</code> — O(1)</summary>

頂点の現在値をコピーして返します。

{% raw %}
```cpp
auto x = t.get(0);
```
{% endraw %}

注意点: 0 <= v < N。参照を返さないため、更新による参照無効化はありません。

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

<details class="api-operation" id="prod" markdown="1">
<summary><code>S prod(int u, int v)</code> — 償却 O(log(N+1))</summary>

u から v のパスを両端込みで順に op した値を返します。

{% raw %}
```cpp
auto sum = t.prod(0, 1);
```
{% endraw %}

注意点: 連結であることが前提です。u == v なら get(u)。非可換演算では逆方向の値は異なります。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>LinkCutTree(const LinkCutTree& other)</code> — O(N)</summary>

独立な森林をコピーします。

{% raw %}
```cpp
auto copy = t;
```
{% endraw %}

注意点: 頂点番号は同じですが、その後の辺・値更新は独立です。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>LinkCutTree& operator=(const LinkCutTree& other)</code> — O(N+D)</summary>

other の森林で置き換え、自身への参照を返します。

{% raw %}
```cpp
copy = t;
```
{% endraw %}

注意点: N はコピー元、D はコピー先の旧頂点数。自己代入可能。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>LinkCutTree(LinkCutTree&& other)</code> — O(1)</summary>

vector の所有権を移動します。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元は破棄・再代入のみを保証します。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>LinkCutTree& operator=(LinkCutTree&& other)</code> — O(D)</summary>

旧格納値を破棄して所有権を移動します。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: D は移動先の旧頂点数。移動元は破棄・再代入のみを保証します。

</details>

## 出典・検証

- [Splay preferred path の説明（ei1333）](https://ei1333.hateblo.jp/entry/2018/05/29/011140)
- [Harvard CS224 講義資料](https://people.seas.harvard.edu/~cs224/spring17/lec/lec26.pdf)
- [Dynamic Tree Vertex Add Path Sum](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum)
- [Dynamic Tree Vertex Set Path Composite](https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite)

ランダムテストでは非可換な文字列連結を使い、逆向きパス、閉路になる link、存在しない cut、独立コピーを BFS と照合します。

<details class="api-operation" id="evert" markdown="1">
<summary><code>void evert(int v)</code> — 償却 O(log(N+1))</summary>

v を所属する木の根にします。辺集合と値は変わりません。

{% raw %}
```cpp
t.evert(1);
assert(t.leader(1) == 1);
```
{% endraw %}

注意点: 0 <= v < N。prod(u,v)、cut(u,v)、link(u,v) も内部で evert(u) を行います。失敗して false を返す link/cut でも根は変わることがあります。set/get/same/leader は根を変更しません。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int leader(int v)</code> — 償却 O(log(N+1))</summary>

現在の根の頂点番号を返します。成分の固定 ID ではありません。

{% raw %}
```cpp
auto root = t.leader(0);
```
{% endraw %}

注意点: 0 <= v < N。成功した link(u,v) は u 側の木を v 側に接続し、根は元の v 側の根です。prod(u,v) 後は u が根です。成功した cut(u,v) は u と v をそれぞれの木の根にします。根の意味を固定したい処理では evert を明示してください。

</details>
