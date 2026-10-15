---
title: Dynamic Top Tree
documentation_of: //blueberry/graph/dynamic-top-tree.hpp
---

## 概要

辺のlink/cutと全木・部分木のDPを扱います。頂点数は固定で、初期状態は孤立点です。`compress`は上から下への結合的な結合、`rake`は同一親への寄与の可換・結合的な結合とします。Pointの逆元は不要ですが、同じ木を表す分解・再結合・根変更で整合するDP代数が必要です。

Nを頂点数とすると、構築O(N)、メモリO(N)、get/sizeはO(1)です。動的操作は償却O(log²(N+1))、単発最悪O(N)となり得ます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`DynamicTopTree`](#construct) | O(N) |
| [`size`](#size) | O(1) |
| [`V`](#get) | O(1) |
| [`set`](#set) | 償却 O(log²(N+1)) |
| [`same`](#same) | 償却 O(log²(N+1)) |
| [`link`](#link) | 償却 O(log²(N+1)) |
| [`cut`](#cut) | 償却 O(log²(N+1)) |
| [`evert`](#evert) | 償却 O(log²(N+1)) |
| [`leader`](#leader) | 償却 O(log²(N+1)) |
| [`Path`](#all-prod) | 償却 O(log²(N+1)) |
| [`Path`](#prod) | 償却 O(log²(N+1)) |
| [`Path`](#subtree-prod) | 償却 O(log²(N+1)) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`DynamicTopTree`（コピー構築）](#copy) | O(N) |
| [`operator=`（コピー代入）](#copy-assign) | O(N+D) |
| [`DynamicTopTree`（移動構築）](#move) | O(1) |
| [`operator=`（移動代入）](#move-assign) | O(D) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 型は`DynamicTopTree<V, Path, Point, add_vertex, add_edge, compress, rake, point_identity>`です。`0 <= N <= INT_MAX`とし、PathとVはコピー可能、Pointはデフォルト構築・コピー可能にしてください。
- `add_vertex(value, point_identity())`が単独頂点を表し、`add_edge`は完成したpath clusterを親へのPointへ変換します。
- 例えば互換なclusterでは`add_edge(compress(add_vertex(v,L),P)) == add_edge(add_vertex(v,rake(L,add_edge(P))))`が必要です。一頂点のclusterはpathの向きに依存しないことを仮定し、パス反転時はcompressの順を反転します。
- 葉と枝の違いを扱える情報や、辺を分割して作る辺頂点などはVに含めます。頂点値は一度だけ加算されます。
- callbackは外部の可変状態に依存させず、例外を投げないでください。オーバーフローは呼び出し側で防ぎます。計算量はcallback・コピー・破棄がO(1)の場合です。
- preferred pathと各頂点のlight clusterをそれぞれsplay treeで保持します。light nodeは再利用し、操作回数でメモリが増え続けません。既存LCTのラッパーではなく、light clusterの所有・結合を持つ独立実装です。
- 競プロで広義のTop Treeと呼ばれる二階splay方式です。論文の狭義の辺クラスタ（境界頂点を共有する二境界クラスタ）モデルそのものとは異なります。ACLに同等機能はありません。

### 計算量の保証 {#計算量の保証}

- preferred-path accessの辺変更回数は一連の操作で償却O(log N)です。その一回につきlight splayの挿入・削除が加わり、それぞれ償却O(log N)なので、動的操作の上界は償却O(log²(N+1))となります。
- 二階のpotentialを合わせた、より強いO(log N)の解析は本実装の保証として採用していません。

</details>

## 最小使用例

`dp[v] = value[v] * product(1 + dp[child])` を保持します。
0 を含む積でも逆元を使わずに扱えます。Path は heavy child の dp を変数とした affine 関数です。

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/dynamic-top-tree.hpp"
struct Path { long long a, b; };
Path vertex(long long value, long long light) { long long x=value*light; return {x,x}; }
long long edge(Path p) { return 1+p.b; }
Path compress(Path x, Path y) { return {x.a*y.a,x.a*y.b+x.b}; }
long long rake(long long x, long long y) { return x*y; }
long long identity() { return 1; }
using Tree=blueberry::DynamicTopTree<long long,Path,long long,vertex,edge,compress,rake,identity>;
int main() {
  Tree t({1,2,3});
  assert(t.link(0,1));
  assert(t.link(1,2));
  assert(t.all_prod(0).b==9);
  assert(t.all_prod(1).b==16);
  assert(t.subtree_prod(1,0).b==8);
  assert(t.cut(1,2));
  assert(t.all_prod(0).b==3);
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>DynamicTopTree(const vector&lt;V&gt;&amp; values = {})</code> — O(N)</summary>

孤立点を構築します。空入力・省略も可能です。

{% raw %}
```cpp
Tree t({1, 2, 3});
```
{% endraw %}

注意点: 値はコピーします。元配列の寿命に依存しません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

固定頂点数を返します。

{% raw %}
```cpp
assert(t.size() == 3);
```
{% endraw %}

注意点: 頂点自体の増減はありません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>V get(int v) const</code> — O(1)</summary>

入力値の現在値をコピーします。

{% raw %}
```cpp
auto value = t.get(0);
```
{% endraw %}

注意点: 0 <= v < N。木 DP の答えではありません。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void set(int v, const V& x)</code> — 償却 O(log²(N+1))</summary>

頂点値を代入し集約を再計算します。

{% raw %}
```cpp
t.set(0, 5);
```
{% endraw %}

注意点: 0 <= v < N。根は変わりません。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool same(int u, int v)</code> — 償却 O(log²(N+1))</summary>

同じ成分なら true を返します。

{% raw %}
```cpp
bool connected=t.same(0, 1);
```
{% endraw %}

注意点: 0 <= u,v < N。内部の preferred path は変わります。

</details>

<details class="api-operation" id="link" markdown="1">
<summary><code>bool link(int u, int v)</code> — 償却 O(log²(N+1))</summary>

異なる木を辺で接続して true。既に連結なら false。

{% raw %}
```cpp
assert(t.link(0, 1));
```
{% endraw %}

注意点: 自己辺や閉路は保存しません。内部で evert(u) を行い、成功後は v 側の元の根が根です。失敗しても根は変わる場合があります。

</details>

<details class="api-operation" id="cut" markdown="1">
<summary><code>bool cut(int u, int v)</code> — 償却 O(log²(N+1))</summary>

既存辺を削除して true。辺がないと false。

{% raw %}
```cpp
assert(t.cut(0, 1));
```
{% endraw %}

注意点: 内部で evert(u)。成功後は u,v が各成分の根です。失敗しても辺集合は変わりませんが根は変わる場合があります。

</details>

<details class="api-operation" id="evert" markdown="1">
<summary><code>void evert(int v)</code> — 償却 O(log²(N+1))</summary>

v を成分の根にします。

{% raw %}
```cpp
t.evert(1);
```
{% endraw %}

注意点: 0 <= v < N。値・辺集合は変えません。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int leader(int v)</code> — 償却 O(log²(N+1))</summary>

現在の根の頂点番号を返します。

{% raw %}
```cpp
auto root=t.leader(0);
```
{% endraw %}

注意点: 成分の固定 ID ではありません。leader 自体は根を変えません。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>Path all_prod(int root)</code> — 償却 O(log²(N+1))</summary>

root を根とする成分の DP cluster を返します。根から他の全枝を light として rake し、add_vertex した一頂点 path cluster に正規化します。

{% raw %}
```cpp
auto answer=t.all_prod(0).b;
```
{% endraw %}

注意点: root が成分の根になります。前回の preferred path の下端に結果の境界が依存しません。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>Path prod(int u, int v)</code> — 償却 O(log²(N+1))</summary>

u→v を path 境界とし、パス外の枝も rake した成分全体の cluster を返します。

{% raw %}
```cpp
auto cluster=t.prod(0, 1);
```
{% endraw %}

注意点: 連結が前提。LCT のパスだけの積とは異なります。u が根になります。u==v は全枝を rake した一頂点境界です。

</details>

<details class="api-operation" id="subtree-prod" markdown="1">
<summary><code>Path subtree_prod(int v, int parent = -1)</code> — 償却 O(log²(N+1))</summary>

parent==-1 なら all_prod(v)。それ以外は辺 (v,parent) を除いた v 側を v 根とする cluster です。

{% raw %}
```cpp
auto subtree=t.subtree_prod(1, 0).b;
```
{% endraw %}

注意点: parent は隣接頂点。一時 cut/link 後に元の辺集合へ戻します。結果は all_prod 同様の一頂点境界。根の保持は保証しません。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>DynamicTopTree(const DynamicTopTree& other)</code> — O(N)</summary>

独立な森林・DP をコピーします。

{% raw %}
```cpp
auto copy=t;
```
{% endraw %}

注意点: light node の index も所有配列内なので他方と共有しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>DynamicTopTree& operator=(const DynamicTopTree& other)</code> — O(N+D)</summary>

コピーして自身への参照を返します。

{% raw %}
```cpp
copy=t;
```
{% endraw %}

注意点: N はコピー元、D はコピー先旧頂点数。自己代入可能。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>DynamicTopTree(DynamicTopTree&& other)</code> — O(1)</summary>

所有配列を移動します。

{% raw %}
```cpp
auto moved=std::move(copy);
```
{% endraw %}

注意点: 移動元は破棄・再代入のみ保証します。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>DynamicTopTree& operator=(DynamicTopTree&& other)</code> — O(D)</summary>

旧値を破棄して所有配列を移動します。

{% raw %}
```cpp
copy=std::move(moved);
```
{% endraw %}

注意点: D は移動先旧頂点数。移動元は破棄・再代入のみ保証します。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Luzhiled Top Tree](https://ei1333.github.io/library/structure/dynamic-tree/top-tree.hpp.html): 二階 splay と light cluster handle の移動を比較。ここでは index 所有・再利用 pool を用い独立に実装。
- [ABC351 G 公式解説](https://atcoder.jp/contests/abc351/editorial/9899): 広義・狭義 Top Tree の区別と rake/compress 代数。
- [Maintaining Information in Fully-Dynamic Trees with Top Trees](https://arxiv.org/abs/cs/0310065): 狭義の境界クラスタモデルとの比較。
- [Point Set Tree Path Composite Sum](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum)
- [Dynamic Tree Vertex Add Subtree Sum](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum)

ランダム森林上の link/cut、全根・部分木 DP、非可換 compress、0 を含む逆元のない rake を愚直 DP と照合します。

</details>
