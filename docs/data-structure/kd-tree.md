---
title: KD Tree
documentation_of: //blueberry/data-structure/kd-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::KDTree<Coord, S, op, e>` は事前登録した2次元点の点代入・矩形集約を行う。構築後に新しい座標を追加したり、点の位置を移動したりしない。同じ座標も入力順の異なる ID として保持する。

`0 <= N <= INT_MAX/2`。Coord は `<` による全順序（NaN 不可）、S はコピー構築・代入可能。op は可換・結合的、e は左右単位元。空集合・空矩形は e()。座標の加減算はしない。集約のオーバーフロー・演算の閉性は利用者が保証する。

交互の軸で中央値分割する平衡 KD-tree。構築は std::nth_element の平均線形選択に基づき平均 O(N log(N+1))、メモリ O(1+N)。点更新 O(log(N+1))、2次元の軸平行矩形集約は最悪 O(sqrt(N)+1)。比較・op・e・コピー・破棄は O(1) とする。最近傍や未登録点の動的挿入に O(log N) を主張する構造ではない。値・座標は所有し、入力の変更・破棄に依存しない。返り値はコピー。D は代入先の旧保持要素数。

既存2D Fenwick/永続矩形和と比較し、逆元がない可換モノイドや固定点の代入が必要な場合に選ぶ。ACL に多次元 KD-tree はない。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `KDTree(const std::vector<std::pair<Coord,Coord>>& points, const std::vector<S>& values)` | 平均 O(N log(N+1)) | [開く](#constructor) |
| `KDTree copy(const KDTree& other)` | O(1 + N) | [開く](#copy-constructor) |
| `KDTree& operator=(const KDTree& other)` | O(1 + N) + D | [開く](#copy-assignment) |
| `KDTree moved(KDTree&& other)` | O(1) | [開く](#move-constructor) |
| `KDTree& operator=(KDTree&& other)` | O(1 + D) | [開く](#move-assignment) |
| `int size() const` | O(1) | [開く](#size) |
| `S get(int p) const` | O(1) | [開く](#get) |
| `void set(int p, const S& value)` | O(log(N+1)) | [開く](#set) |
| `S prod(Coord xl, Coord yl, Coord xr, Coord yr) const` | O(sqrt(N)+1) | [開く](#prod) |
| `S all_prod() const` | O(1) | [開く](#all-prod) |

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

## 出典・検証

[Bentley の KD-tree 原論文](https://dl.acm.org/doi/10.1145/361002.361007) と [maspypy の矩形作用 KD-tree](https://maspypy.github.io/library/ds/kdtree/dual_kdtree_monoid.hpp) を参考に固定点・点代入・矩形集約へ用途を絞り独立実装。公式 `point_add_rectangle_sum` は未来の点を重み0で事前登録し `verify/data-structure/kd-tree.test.cpp` で検証。ランダムテストで重複座標・空矩形・負座標・更新を愚直比較する。
