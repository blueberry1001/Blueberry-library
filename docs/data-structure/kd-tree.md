---
title: KD Tree
documentation_of: //blueberry/data-structure/kd-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

事前登録した2次元点の点代入と矩形集約を行います。`KDTree<Coord, S, op, e>` のopは可換・結合的、e()は左右単位元です。構築後の座標追加・移動はできず、同じ座標も入力順の異なるIDとして保持します。

Nは点数。構築は平均O(N log(N+1))、点更新はO(log(N+1))、矩形集約は最悪O(sqrt(N)+1)、メモリはO(1+N)です。逆元のない可換モノイドや、固定点の代入が必要な場合に選びます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`KDTree`](#constructor) | 平均 O(N log(N+1)) |
| [`KDTree`](#copy-constructor) | O(1 + N) |
| [`operator=`](#copy-assignment) | O(1 + N) + D |
| [`KDTree`](#move-constructor) | O(1) |
| [`operator=`](#move-assignment) | O(1 + D) |
| [`size`](#size) | O(1) |
| [`S`](#get) | O(1) |
| [`set`](#set) | O(log(N+1)) |
| [`S`](#prod) | O(sqrt(N)+1) |
| [`S`](#all-prod) | O(1) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `0 <= N <= INT_MAX/2`。Coordの `<` は全順序（NaN不可）、Sはコピー構築・代入可能であることが必要です。空集合・空矩形はe()です。
- 座標の加減算はしません。集約のオーバーフロー・演算の閉性は利用者が保証してください。比較・op・e・コピー・破棄はO(1)とします。
- 軸を交互に変えた中央値分割で平衡木を作り、構築計算量はstd::nth_elementの平均線形選択に基づきます。最近傍や未登録点の動的挿入にO(log N)を主張する構造ではありません。
- 値・座標は所有し、入力の変更・破棄に依存しません。返り値はコピーです。Dは代入先の旧保持要素数です。
- ACLに多次元KD-treeはありません。既存2D Fenwickや永続矩形和と比較して選んでください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/data-structure/kd-tree.hpp"
long long op(long long a,long long b){return a+b;}
long long e(){return 0;}
int main(){
  blueberry::KDTree<int,long long,op,e> t({{1,2},{1,2},{4,5}}, {3,7,9});
  assert(t.size()==3 && t.prod(0,0,2,3)==10);
  t.set(1,2);
  assert(t.get(1)==2 && t.all_prod()==14);
  assert(t.prod(0,0,0,4)==0);
}
```
{% endraw %}

<details class="api-operation" id="constructor" markdown="1">
<summary><code>KDTree(const std::vector&lt;std::pair&lt;Coord,Coord&gt;&gt;&amp; points, const std::vector&lt;S&gt;&amp; values)</code> — 平均 O(N log(N+1))</summary>

入力順 ID と値で構築する。

{% raw %}
```cpp
blueberry::KDTree<int,long long,op,e> t({{1,2}}, {3});
```
{% endraw %}

注意点: 両配列の長さは一致。空の配列2個も有効。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>KDTree copy(const KDTree&amp; other)</code> — O(1 + N)</summary>

全データを独立に複製する。コピー元の更新・破棄はコピー先に影響しない。

{% raw %}
```cpp
KDTree copy(other);
```
{% endraw %}

注意点: N はコピー元の要素数。追加メモリは元の構造と同じオーダー。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>KDTree&amp; operator=(const KDTree&amp; other)</code> — O(1 + N) + D</summary>

元データを破棄・置換し、自身の参照を返す。

{% raw %}
```cpp
object = other;
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己コピー代入は有効。既存の内部参照は無効化される。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>KDTree moved(KDTree&amp;&amp; other)</code> — O(1)</summary>

所有権を移動する。

{% raw %}
```cpp
KDTree moved(std::move(other));
```
{% endraw %}

注意点: <utility> が必要。移動元は再代入または破棄のみとする。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>KDTree&amp; operator=(KDTree&amp;&amp; other)</code> — O(1 + D)</summary>

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

登録済みの点数を返す。

{% raw %}
```cpp
assert(t.size()==1);
```
{% endraw %}

注意点: 重複座標も各1点。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>S get(int p) const</code> — O(1)</summary>

入力順 ID の値を返す。

{% raw %}
```cpp
assert(t.get(0)==3);
```
{% endraw %}

注意点: 0 <= p < N。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void set(int p, const S&amp; value)</code> — O(log(N+1))</summary>

登録点 p の値を置き換える。

{% raw %}
```cpp
t.set(0,5);
```
{% endraw %}

注意点: 座標は変わらない。点加算は set(p,get(p)+x) で表せる。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S prod(Coord xl, Coord yl, Coord xr, Coord yr) const</code> — O(sqrt(N)+1)</summary>

半開矩形 [xl,xr) × [yl,yr) の点値を集約する。

{% raw %}
```cpp
assert(t.prod(0,0,2,3)==5);
```
{% endraw %}

注意点: xl <= xr、yl <= yr。境界は上端を含まない。空矩形は e()。可換性が必要。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>S all_prod() const</code> — O(1)</summary>

全点の集約を返す。

{% raw %}
```cpp
assert(t.all_prod()==5);
```
{% endraw %}

注意点: 空の点集合では e()。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Bentley の KD-tree 原論文](https://dl.acm.org/doi/10.1145/361002.361007) と [maspypy の矩形作用 KD-tree](https://maspypy.github.io/library/ds/kdtree/dual_kdtree_monoid.hpp) を参考に固定点・点代入・矩形集約へ用途を絞り独立実装。
- 公式 `point_add_rectangle_sum` は未来の点を重み0で事前登録し `verify/data-structure/kd-tree.test.cpp` で検証。
- ランダムテストで重複座標・空矩形・負座標・更新を愚直比較する。

</details>
