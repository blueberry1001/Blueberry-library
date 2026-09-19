---
title: Dynamic Top Tree
documentation_of: //blueberry/graph/dynamic-top-tree.hpp
---

## 概要・前提

`blueberry::DynamicTopTree<V, Path, Point, add_vertex, add_edge, compress, rake, point_identity>` は
辺の link/cut と全木・部分木の DP を扱う、頂点ベースの動的 rake/compress 構造です。
preferred path の splay tree に加え、各頂点の light cluster を別の splay tree で保持します。
Point の逆元を必要としないため、通常の Link-Cut Tree のパス積より広い木 DP を扱えます。
既存 LCT のラッパーではなく、light cluster の所有・結合を持つ独立実装です。

本実装は競プロで広義の Top Tree と呼ばれる二階 splay 方式です。
論文の狭義の辺クラスタ（境界頂点を共有する二境界クラスタ）モデルそのものとは異なります。
頂点値は一度だけ加算されます。ACL に同等機能はありません。

N は固定頂点数、0 <= N <= INT_MAX。初期状態は孤立点です。メモリ O(N)。light node は再利用し、操作回数で増え続けません。
Path と V はコピー可能、Point はデフォルト構築・コピー可能にしてください。
`add_vertex(value, point_identity())` が単独頂点を表します。
compress は上→下方向の結合的な結合、rake は同一親への寄与の可換・結合的な結合です。
add_edge は完成した path cluster を親への Point に変換します。

同じ木を表す分解・再結合・根変更で整合する DP 代数が必要です。callback は外部の可変状態に依存させず、例外を投げないでください。
例えば互換な cluster では `add_edge(compress(add_vertex(v,L),P)) == add_edge(add_vertex(v,rake(L,add_edge(P))))` が必要です。
一頂点の cluster は path の向きに依存しないことを仮定します。パス反転時は compress の順を反転します。
葉と枝の違いを扱える情報、辺を分割して作る辺頂点などは V に含めます。
オーバーフローは呼び出し側で防ぎます。以下は callback・コピー・破棄が O(1) の場合です。

### 計算量の保証

構築 O(N)。get/size は O(1)、動的操作は **償却 O(log²(N+1))** と保守的に保証します。
通常の preferred-path access の辺変更回数は一連の操作で償却 O(log N)。
その一回につき light splay の挿入・削除が追加され、それぞれ償却 O(log N) なので二乗の上界を得ます。
二階の potential を合わせたより強い O(log N) の解析は本実装では保証として採用していません。
単発の最悪時間は O(N) となり得ます。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DynamicTopTree(const vector<V>& values = {})` | O(N) | [開く](#construct) |
| `int size() const` | O(1) | [開く](#size) |
| `V get(int v) const` | O(1) | [開く](#get) |
| `void set(int v, const V& x)` | 償却 O(log²(N+1)) | [開く](#set) |
| `bool same(int u, int v)` | 償却 O(log²(N+1)) | [開く](#same) |
| `bool link(int u, int v)` | 償却 O(log²(N+1)) | [開く](#link) |
| `bool cut(int u, int v)` | 償却 O(log²(N+1)) | [開く](#cut) |
| `void evert(int v)` | 償却 O(log²(N+1)) | [開く](#evert) |
| `int leader(int v)` | 償却 O(log²(N+1)) | [開く](#leader) |
| `Path all_prod(int root)` | 償却 O(log²(N+1)) | [開く](#all-prod) |
| `Path prod(int u, int v)` | 償却 O(log²(N+1)) | [開く](#prod) |
| `Path subtree_prod(int v, int parent = -1)` | 償却 O(log²(N+1)) | [開く](#subtree-prod) |
| `DynamicTopTree(const DynamicTopTree& other)` | O(N) | [開く](#copy) |
| `DynamicTopTree& operator=(const DynamicTopTree& other)` | O(N+D) | [開く](#copy-assign) |
| `DynamicTopTree(DynamicTopTree&& other)` | O(1) | [開く](#move) |
| `DynamicTopTree& operator=(DynamicTopTree&& other)` | O(D) | [開く](#move-assign) |

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

## 出典・検証

- [Luzhiled Top Tree](https://ei1333.github.io/library/structure/dynamic-tree/top-tree.hpp.html): 二階 splay と light cluster handle の移動を比較。ここでは index 所有・再利用 pool を用い独立に実装。
- [ABC351 G 公式解説](https://atcoder.jp/contests/abc351/editorial/9899): 広義・狭義 Top Tree の区別と rake/compress 代数。
- [Maintaining Information in Fully-Dynamic Trees with Top Trees](https://arxiv.org/abs/cs/0310065): 狭義の境界クラスタモデルとの比較。
- [Point Set Tree Path Composite Sum](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum)
- [Dynamic Tree Vertex Add Subtree Sum](https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum)

ランダム森林上の link/cut、全根・部分木 DP、非可換 compress、0 を含む逆元のない rake を愚直 DP と照合します。
