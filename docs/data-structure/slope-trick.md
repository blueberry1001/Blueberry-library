---
title: Slope Trick (priority queue)
documentation_of: //blueberry/data-structure/slope-trick.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::SlopeTrick<T = long long>` は実数全域で定義された、下に有界な区分線形凸関数を保持する。
2個の priority_queue で傾きの単位変化を管理し、最小値・最小点区間 O(1)、hinge/絶対値の加算 償却 O(log(N+1))。
大きな重みを回数分追加すると重みに比例して膨らむため、その用途は有限定義域の SplaySlopeTrick を検討する。
メモリ O(N)、N は傾き変化の個数（同じ座標も別に数える）。加算と減算・順序比較ができる正確な T を使い、
オーバーフローは利用側で防ぐ。数値 infinity は使わず、argmin の optional の無値が無限端点を表す。
shift(l,r) は f(x)=min(f_old(y): x-r<=y<=x-l)。prefix_min は y<=x、suffix_min は y>=x の最小化。
空heapは未定義関数ではなく定数関数。clear/move元/merge元は f(x)=0。
内部 vector の再確保により、単一の hinge 加算や merge は O(N+M) の移動を追加で要する場合がある。上記は操作列全体の償却評価。
コピーは独立。eval は非破壊だが O(N log(N+1)) のheapコピー・取り出しを行う。
単位傾き限定・全実数定義のこの型と、任意重み・有限定義域のsplay版は別の適用範囲。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/slope-trick.hpp"
int main() {
  blueberry::SlopeTrick<> tree;
  tree.add_abs(1); tree.add_abs(4); tree.add_const(2);
  assert(tree.min() == 5);
  auto [l, r] = tree.argmin();
  assert(l == 1 && r == 4 && tree.eval(0) == 7);
  tree.shift(2); assert(tree.argmin().first == 3);
  tree.prefix_min(); assert(tree.eval(100) == 5);
}
```
{% endraw %}

## 操作一覧

N は要素数（slope trick は保持する折れ点数）、M はコピー・結合相手または置換前の保持量。型のコピー・演算・比較は O(1) とする。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SlopeTrick(T constant = T(0))` | O(1) | [開く](#constructor) |
| `SlopeTrick(const SlopeTrick& other)` | O(N) | [開く](#copy-constructor) |
| `SlopeTrick& operator=(const SlopeTrick& other)` | O(N + M) | [開く](#copy-assignment) |
| `SlopeTrick(SlopeTrick&& other)` | O(1) | [開く](#move-constructor) |
| `SlopeTrick& operator=(SlopeTrick&& other)` | O(M) | [開く](#move-assignment) |
| `std::size_t size() const` | O(1) | [開く](#size) |
| `T min() const` | O(1) | [開く](#min) |
| `std::pair<std::optional<T>, std::optional<T>> argmin() const` | O(1) | [開く](#argmin) |
| `void add_const(const T& c)` | O(1) | [開く](#add-const) |
| `void add_x_minus_a(const T& a)` | 償却 O(log(N+1)) | [開く](#add-right) |
| `void add_a_minus_x(const T& a)` | 償却 O(log(N+1)) | [開く](#add-left) |
| `void add_abs(const T& a)` | 償却 O(log(N+1)) | [開く](#add-abs) |
| `void shift(const T& dx)` | O(1) | [開く](#shift) |
| `void shift(const T& l, const T& r)` | O(1) | [開く](#window) |
| `void prefix_min()` | O(K) | [開く](#prefix) |
| `void suffix_min()` | O(K) | [開く](#suffix) |
| `T eval(const T& x) const` | O(N log(N+1)) | [開く](#eval) |
| `void merge(SlopeTrick& other)` | 償却 O(min(N,M) log(N+M+1)) | [開く](#merge) |
| `void clear()` | O(N) | [開く](#clear) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>SlopeTrick(T constant = T(0))</code> — O(1)</summary>

定数関数 f(x)=constant を作る。

{% raw %}
```cpp
blueberry::SlopeTrick<> tree(3);
```
{% endraw %}

注意点: constant の符号に制限はない。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>SlopeTrick(const SlopeTrick&amp; other)</code> — O(N)</summary>

内容を複製する。更新は互いに独立。

{% raw %}
```cpp
SlopeTrick copy(tree);
```
{% endraw %}

注意点: 空状態のコピーも可能。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>SlopeTrick&amp; operator=(const SlopeTrick&amp; other)</code> — O(N + M)</summary>

代入元 N 要素で置換し、自身への参照を返す。M は置換前の保持量。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入は内容を保つ。返された参照は代入先が生存する間有効。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>SlopeTrick(SlopeTrick&amp;&amp; other)</code> — O(1)</summary>

所有権を移し、元を空状態にする。

{% raw %}
```cpp
SlopeTrick moved(std::move(tree));
```
{% endraw %}

注意点: <utility> が必要。空状態の意味は概要を参照。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>SlopeTrick&amp; operator=(SlopeTrick&amp;&amp; other)</code> — O(M)</summary>

古い保持要素 M 個の破棄を含めて置換し、自身への参照を返す。

{% raw %}
```cpp
moved = std::move(tree);
```
{% endraw %}

注意点: 自己ムーブ代入は内容を保つ。元は空になる。参照の寿命は代入先の寿命と同じ。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>std::size_t size() const</code> — O(1)</summary>

heapに保持する傾き変化の個数。

{% raw %}
```cpp
auto n = tree.size();
```
{% endraw %}

注意点: 異なる座標の個数ではない。定数関数は0。

</details>

<details class="api-operation" id="min" markdown="1">
<summary><code>T min() const</code> — O(1)</summary>

関数の最小値を返す。

{% raw %}
```cpp
auto value = tree.min();
```
{% endraw %}

注意点: 定数関数でも有効。

</details>

<details class="api-operation" id="argmin" markdown="1">
<summary><code>std::pair&lt;std::optional&lt;T&gt;, std::optional&lt;T&gt;&gt; argmin() const</code> — O(1)</summary>

最小化区間の左右端。first が nullopt なら -∞、second が nullopt なら +∞。

{% raw %}
```cpp
auto [left, right] = tree.argmin();
```
{% endraw %}

注意点: 定数関数は両側 nullopt。端点の値はコピー。

</details>

<details class="api-operation" id="add-const" markdown="1">
<summary><code>void add_const(const T&amp; c)</code> — O(1)</summary>

f(x) に c を加える。

{% raw %}
```cpp
tree.add_const(3);
```
{% endraw %}

注意点: c は負でもよい。

</details>

<details class="api-operation" id="add-right" markdown="1">
<summary><code>void add_x_minus_a(const T&amp; a)</code> — 償却 O(log(N+1))</summary>

max(0,x-a) を加える。

{% raw %}
```cpp
tree.add_x_minus_a(2);
```
{% endraw %}

注意点: 傾き変化は1。座標・最小値の加減算が型に収まること。

</details>

<details class="api-operation" id="add-left" markdown="1">
<summary><code>void add_a_minus_x(const T&amp; a)</code> — 償却 O(log(N+1))</summary>

max(0,a-x) を加える。

{% raw %}
```cpp
tree.add_a_minus_x(2);
```
{% endraw %}

注意点: 傾き変化は1。

</details>

<details class="api-operation" id="add-abs" markdown="1">
<summary><code>void add_abs(const T&amp; a)</code> — 償却 O(log(N+1))</summary>

abs(x-a) を加える。

{% raw %}
```cpp
tree.add_abs(2);
```
{% endraw %}

注意点: 2個の単位hingeを追加する。

</details>

<details class="api-operation" id="shift" markdown="1">
<summary><code>void shift(const T&amp; dx)</code> — O(1)</summary>

f(x) を f_old(x-dx) に置き換える。

{% raw %}
```cpp
tree.shift(3);
```
{% endraw %}

注意点: 正の dx はグラフを右に動かす。

</details>

<details class="api-operation" id="window" markdown="1">
<summary><code>void shift(const T&amp; l, const T&amp; r)</code> — O(1)</summary>

x-r<=y<=x-l での f_old(y) の最小値に置換。

{% raw %}
```cpp
tree.shift(-2, 3);
```
{% endraw %}

注意点: l<=r。端点offsetの加算でオーバーフローしないこと。

</details>

<details class="api-operation" id="prefix" markdown="1">
<summary><code>void prefix_min()</code> — O(K)</summary>

min_{y<=x} f_old(y) に置換。K は破棄する右heapの要素数。

{% raw %}
```cpp
tree.prefix_min();
```
{% endraw %}

注意点: 物理的な要素破棄を含み、O(1) としない。K=0なら O(1)。

</details>

<details class="api-operation" id="suffix" markdown="1">
<summary><code>void suffix_min()</code> — O(K)</summary>

min_{y>=x} f_old(y) に置換。K は破棄する左heapの要素数。

{% raw %}
```cpp
tree.suffix_min();
```
{% endraw %}

注意点: 物理的な要素破棄を含む。K=0なら O(1)。

</details>

<details class="api-operation" id="eval" markdown="1">
<summary><code>T eval(const T&amp; x) const</code> — O(N log(N+1))</summary>

f(x) を非破壊で求める。一時メモリ O(N)。

{% raw %}
```cpp
auto value = tree.eval(0);
```
{% endraw %}

注意点: 多数の一点評価にはsplay版を検討。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>void merge(SlopeTrick&amp; other)</code> — 償却 O(min(N,M) log(N+M+1))</summary>

f+=other を行い、otherを f=0 にする。大きいheap側へ小さい方を移す。

{% raw %}
```cpp
tree.merge(other);
```
{% endraw %}

注意点: 自分自身を渡せない。大小交換が内部で起こるが結果は和。両定数関数なら O(1)。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void clear()</code> — O(N)</summary>

保持要素を解放し f(x)=0 に戻す。

{% raw %}
```cpp
tree.clear();
```
{% endraw %}

注意点: 空heapでも定数項を0に戻す。

</details>

## 出典・検証

[Luzhiled Slope Trick](https://ei1333.github.io/library/structure/others/slope-trick.hpp.html)、
[Nyaan Slope Trick](https://nyaannyaan.github.io/library/data-structure/slope-trick.hpp.html) を比較し、単位傾き版を独立実装。
直接対応する Library Checker verifier は現時点でない。`tests/random/slope-tricks.cpp` で
hinge/絶対値・定数・shift・prefix/suffix最小化・merge・copy/moveを十分広い整数グリッドの愚直DPと比較。
候補問題 [ABC127 F](https://atcoder.jp/contests/abc127/tasks/abc127_f) は add_abs/add_const/min のみを扱い、
公開sampleだけではshift等を検証できないため、公式全ケースACを主張しない。



