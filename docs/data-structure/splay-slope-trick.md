---
title: Splay Slope Trick (finite convex functions)
documentation_of: //blueberry/data-structure/splay-slope-trick.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::SplaySlopeTrick<T = long long>` は有限の閉区間上の一般の区分線形凸関数を保持する。
各折れ点に正の傾き変化量を持つsplay木を用い、単位傾きに展開せず任意の非負重みを扱う。
左右端の外は定義域外。全実数上の関数を扱う priority_queue SlopeTrick の置換ではない。
既定状態は空定義域、`SplaySlopeTrick(l,r,a,b)` は [l,r] 上で ax+b、l==r も有効。
K は内部折れ点数で、メモリ O(K+1)。eval/min/argmin/hinge加算/線分畳み込みは償却 O(log(K+1))、
単発のsplay操作は最悪 O(K)。shift/affine加算は O(1)。
T は正確な四則演算・比較が可能な型を使う。整数型なら from_points の各割り算が割り切れること。
有理数の傾き・座標・重みには blueberry::Fraction も使える。浮動小数点の誤差吸収や epsilon は提供しない。
座標×傾き、subtree合計、平行移動など途中の値まで型の範囲内であること。
コピー構築・コピー代入はdelete。ムーブ元・merge/convolve元は空。自己ムーブ代入は内容維持。
値・比較・演算は例外を投げない前提。返り値はコピーで外部参照を持たない。
eval/min/argmin はsplay回転するので非constだが、表す関数は変わらない。
空定義域・定義域外は optional の無値で区別し、巨大な整数を infinity として使わない。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/splay-slope-trick.hpp"
int main() {
  blueberry::SplaySlopeTrick<> tree(-10, 10);
  tree.add_abs(2, 1000000);
  tree.add_linear(3, 1);
  assert(tree.min() == 7 && tree.argmin()->first == 2);
  assert(tree.eval(3) == 1000010 && !tree.eval(11));
  tree.convolve_segment(0, 4, 0, 0);
  assert(tree.domain()->second == 14);
  assert(tree.argmin()->first == 2 && tree.argmin()->second == 6);
  tree.restrict(3, 5);
  assert(tree.min() == 7);
}
```
{% endraw %}

## 操作一覧

K は現在の内部折れ点数、N は from_points の入力点数、M は結合相手または代入先の旧内部折れ点数。型のコピー・演算・比較は O(1) とする。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SplaySlopeTrick()` | O(1) | [開く](#default) |
| `SplaySlopeTrick(const T& l, const T& r, const T& a = T(0), const T& b = T(0))` | O(1) | [開く](#linear) |
| `static SplaySlopeTrick from_points(const std::vector<std::pair<T,T>>& points)` | O(N log(N+1)) | [開く](#from-points) |
| `SplaySlopeTrick(SplaySlopeTrick&& other)` | O(1) | [開く](#move-constructor) |
| `SplaySlopeTrick& operator=(SplaySlopeTrick&& other)` | O(M) | [開く](#move-assignment) |
| `~SplaySlopeTrick()` | O(K) | [開く](#destructor) |
| `bool empty() const` | O(1) | [開く](#empty) |
| `std::size_t size() const` | O(1) | [開く](#size) |
| `std::optional<std::pair<T,T>> domain() const` | O(1) | [開く](#domain) |
| `std::optional<T> eval(const T& x)` | O(log(K+1)) 償却 | [開く](#eval) |
| `std::optional<T> min()` | O(log(K+1)) 償却 | [開く](#min) |
| `std::optional<std::pair<T,T>> argmin()` | O(log(K+1)) 償却 | [開く](#argmin) |
| `void add_const(const T& value)` | O(1) | [開く](#add-const) |
| `void add_linear(const T& a, const T& b)` | O(1) | [開く](#add-linear) |
| `void add_x_minus_a(const T& a, const T& weight = T(1))` | O(log(K+1)) 償却 | [開く](#add-right) |
| `void add_a_minus_x(const T& a, const T& weight = T(1))` | O(log(K+1)) 償却 | [開く](#add-left) |
| `void add_abs(const T& a, const T& weight = T(1))` | O(log(K+1)) 償却 | [開く](#add-abs) |
| `void shift(const T& dx, const T& dy = T(0))` | O(1) | [開く](#shift) |
| `void restrict(const T& l, const T& r)` | O(log(K+1) + D) 償却 | [開く](#restrict) |
| `void merge(SplaySlopeTrick& other)` | O((K+M+1) log(K+M+2)) 償却 | [開く](#merge) |
| `void convolve_segment(const T& l, const T& r, const T& a, const T& b)` | O(log(K+1)) 償却 | [開く](#convolve-segment) |
| `void convolve(SplaySlopeTrick& other)` | O((K+M+1) log(K+M+2)) 償却 | [開く](#convolve) |
| `void clear()` | O(K) | [開く](#clear) |

<details class="api-operation" id="default" markdown="1">
<summary><code>SplaySlopeTrick()</code> — O(1)</summary>

空定義域を作る。

{% raw %}
```cpp
blueberry::SplaySlopeTrick<> tree;
```
{% endraw %}

注意点: min/domain/eval/argminはnullopt。size=0だが、size=0の非空線形関数も存在する。

</details>

<details class="api-operation" id="linear" markdown="1">
<summary><code>SplaySlopeTrick(const T&amp; l, const T&amp; r, const T&amp; a = T(0), const T&amp; b = T(0))</code> — O(1)</summary>

閉区間[l,r]でax+bの関数。

{% raw %}
```cpp
blueberry::SplaySlopeTrick<> tree(-5, 5, 2, 3);
```
{% endraw %}

注意点: l<=r。1点定義域も有効。

</details>

<details class="api-operation" id="from-points" markdown="1">
<summary><code>static SplaySlopeTrick from_points(const std::vector&lt;std::pair&lt;T,T&gt;&gt;&amp; points)</code> — O(N log(N+1))</summary>

xの昇順の頂点列を直線補間する。

{% raw %}
```cpp
auto tree = blueberry::SplaySlopeTrick<>::from_points({{0, 3}, {1, 1}, {2, 1}, {3, 4}});
```
{% endraw %}

注意点: 空なら空定義域。xは狭義増加、隣接区間の傾きは非減少。整数型はdy/dxの完全除算が必要。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>SplaySlopeTrick(SplaySlopeTrick&amp;&amp; other)</code> — O(1)</summary>

所有権を移し、元を空状態にする。

{% raw %}
```cpp
SplaySlopeTrick moved(std::move(tree));
```
{% endraw %}

注意点: <utility> が必要。空状態の意味は概要を参照。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>SplaySlopeTrick&amp; operator=(SplaySlopeTrick&amp;&amp; other)</code> — O(M)</summary>

古い保持要素 M 個の破棄を含めて置換し、自身への参照を返す。

{% raw %}
```cpp
moved = std::move(tree);
```
{% endraw %}

注意点: 自己ムーブ代入は内容を保つ。元は空になる。参照の寿命は代入先の寿命と同じ。

</details>

<details class="api-operation" id="destructor" markdown="1">
<summary><code>~SplaySlopeTrick()</code> — O(K)</summary>

所有ノードを非再帰で解放。

{% raw %}
```cpp
{ blueberry::SplaySlopeTrick<> temporary(0, 2); temporary.add_abs(1); }
```
{% endraw %}

注意点: 通常のスコープ寿命で使い、destructorを手動で重複呼出ししない。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool empty() const</code> — O(1)</summary>

定義域が空ならtrue。

{% raw %}
```cpp
bool none = tree.empty();
```
{% endraw %}

注意点: 定数関数・1点関数は非空。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>std::size_t size() const</code> — O(1)</summary>

内部折れ点の個数K。

{% raw %}
```cpp
auto k = tree.size();
```
{% endraw %}

注意点: 傾き変化量や区間の長さではない。同じ座標の重みをまとめる。

</details>

<details class="api-operation" id="domain" markdown="1">
<summary><code>std::optional&lt;std::pair&lt;T,T&gt;&gt; domain() const</code> — O(1)</summary>

閉定義域の左右端を返す。

{% raw %}
```cpp
auto range = tree.domain();
```
{% endraw %}

注意点: 空ならnullopt。無限端点は扱わない。

</details>

<details class="api-operation" id="eval" markdown="1">
<summary><code>std::optional&lt;T&gt; eval(const T&amp; x)</code> — O(log(K+1)) 償却</summary>

定義域内でf(x)を返す。

{% raw %}
```cpp
auto value = tree.eval(0);
```
{% endraw %}

注意点: 空/定義域外ならnullopt。内部splay回転が起こる。

</details>

<details class="api-operation" id="min" markdown="1">
<summary><code>std::optional&lt;T&gt; min()</code> — O(log(K+1)) 償却</summary>

有限定義域内の最小値。

{% raw %}
```cpp
auto value = tree.min();
```
{% endraw %}

注意点: 空ならnullopt。端点での最小も含む。

</details>

<details class="api-operation" id="argmin" markdown="1">
<summary><code>std::optional&lt;std::pair&lt;T,T&gt;&gt; argmin()</code> — O(log(K+1)) 償却</summary>

最小化点の閉区間を返す。

{% raw %}
```cpp
auto minimizers = tree.argmin();
```
{% endraw %}

注意点: 空ならnullopt。定数関数なら定義域全体。

</details>

<details class="api-operation" id="add-const" markdown="1">
<summary><code>void add_const(const T&amp; value)</code> — O(1)</summary>

関数に定数を加える。

{% raw %}
```cpp
tree.add_const(3);
```
{% endraw %}

注意点: 空なら何もしない。

</details>

<details class="api-operation" id="add-linear" markdown="1">
<summary><code>void add_linear(const T&amp; a, const T&amp; b)</code> — O(1)</summary>

ax+bを加える。

{% raw %}
```cpp
tree.add_linear(-2, 3);
```
{% endraw %}

注意点: 任意の符号を許す。定義域は変わらない。

</details>

<details class="api-operation" id="add-right" markdown="1">
<summary><code>void add_x_minus_a(const T&amp; a, const T&amp; weight = T(1))</code> — O(log(K+1)) 償却</summary>

weight*max(0,x-a)を加える。

{% raw %}
```cpp
tree.add_x_minus_a(2, 1000000);
```
{% endraw %}

注意点: weight>=0。0は何もしない。重みに比例した数のノードを作らない。

</details>

<details class="api-operation" id="add-left" markdown="1">
<summary><code>void add_a_minus_x(const T&amp; a, const T&amp; weight = T(1))</code> — O(log(K+1)) 償却</summary>

weight*max(0,a-x)を加える。

{% raw %}
```cpp
tree.add_a_minus_x(2, 7);
```
{% endraw %}

注意点: weight>=0。定義域外の折れ点も正しく線形/定数部分として処理。

</details>

<details class="api-operation" id="add-abs" markdown="1">
<summary><code>void add_abs(const T&amp; a, const T&amp; weight = T(1))</code> — O(log(K+1)) 償却</summary>

weight*abs(x-a)を加える。

{% raw %}
```cpp
tree.add_abs(2, 7);
```
{% endraw %}

注意点: weight>=0。負重みは凸性を壊すため禁止。

</details>

<details class="api-operation" id="shift" markdown="1">
<summary><code>void shift(const T&amp; dx, const T&amp; dy = T(0))</code> — O(1)</summary>

f_new(x)=f_old(x-dx)+dy。定義域もdx移動。

{% raw %}
```cpp
tree.shift(2, -3);
```
{% endraw %}

注意点: 空なら何もしない。lazy offsetと積がオーバーフローしないこと。

</details>

<details class="api-operation" id="restrict" markdown="1">
<summary><code>void restrict(const T&amp; l, const T&amp; r)</code> — O(log(K+1) + D) 償却</summary>

定義域を[l,r]との共通部分へ制限。Dは削除ノード数。

{% raw %}
```cpp
tree.restrict(-2, 3);
```
{% endraw %}

注意点: 共通部分が空なら空関数。l>rも空。物理ノード解放を含むので単にO(logK)とはしない。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>void merge(SplaySlopeTrick&amp; other)</code> — O((K+M+1) log(K+M+2)) 償却</summary>

関数の点ごとの和に置換。定義域は共通部分、otherは空になる。

{% raw %}
```cpp
tree.merge(other);
```
{% endraw %}

注意点: 自分自身は禁止。片方が空なら結果も空。

</details>

<details class="api-operation" id="convolve-segment" markdown="1">
<summary><code>void convolve_segment(const T&amp; l, const T&amp; r, const T&amp; a, const T&amp; b)</code> — O(log(K+1)) 償却</summary>

[l,r]上のg(y)=ay+bとのmin-plus畳み込み。h(x)=min_z(f(z)+g(x-z))。

{% raw %}
```cpp
tree.convolve_segment(0, 3, 0, 0);
```
{% endraw %}

注意点: l<=r。定義域の両端はそれぞれl/rだけ増える。空関数は空のまま。

</details>

<details class="api-operation" id="convolve" markdown="1">
<summary><code>void convolve(SplaySlopeTrick&amp; other)</code> — O((K+M+1) log(K+M+2)) 償却</summary>

一般の凸関数otherとのmin-plus畳み込み。傾き順に線分を併合して再構築。otherは空。

{% raw %}
```cpp
tree.convolve(other);
```
{% endraw %}

注意点: 自分自身は禁止。定義域は両区間のMinkowski和。片方が空なら空。作業メモリO(K+M+1)。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void clear()</code> — O(K)</summary>

全ノードを解放し空定義域へ戻す。

{% raw %}
```cpp
tree.clear();
```
{% endraw %}

注意点: f=0にはならない。定数関数が必要なら区間付きconstructorの結果を代入する。

</details>

## 出典・検証

[maspypy Slope Trick Super](https://maspypy.github.io/library/convex/slope_trick/slope_super.hpp)
の有限定義域・重み付きsplay設計を比較資料とし、専用ノードと所有権管理で独立実装。
Library Checker [Min Plus Convolution (Convex and Convex)](https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex)
は整数格子上の畳み込みを検証する。すべての操作を公式問題で検証したとは主張しない。
`tests/random/slope-tricks.cpp` は重み付きhinge・任意線形項・平行移動・定義域制限・点ごとの和・
線分/一般畳み込み・最小点plateauを愚直列と比較し、Fractionによる非整数の座標/傾き/重みも確認する。

