---
title: Dual Segment Tree
documentation_of: //blueberry/data-structure/dual-segment-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::DualSegmentTree<S, F, mapping, composition, id>` は範囲作用・一点取得に特化。
ACL lazy_segtree と違い区間集約 op/e を要求せず、葉の値と作用だけを保持する。
初期構築・メモリ O(N)、範囲作用・一点操作は最悪 O(log(N+1))。
`mapping(f,x)` は値への作用、`composition(f,g)` は **g の後に f**、`id()` は恒等作用。
作用の合成は結合的で mapping と整合し、可換性は不要。F の等値比較も不要。
S/F はコピー可能、既定構築は不要。N<=INT_MAX。数値演算のオーバーフローは利用側で防ぐ。
空入力・空区間に対応。get は遅延作用を伝播するため非const。get_all も内部状態を更新するが、
返す値・配列は独立したコピー。コピーした木は独立、ムーブ元は長さ0で再利用できる。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/dual-segment-tree.hpp"
struct F { long long a, b; };
long long mapping(F f, long long x) { return f.a * x + f.b; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
int main() {
  blueberry::DualSegmentTree<long long, F, mapping, composition, id> tree(std::vector<long long>{1, 2, 3});
  tree.apply(0, 3, F{2, 1});
  tree.apply(1, 3, F{3, 0});
  assert(tree.get(0) == 3 && tree.get(1) == 15);
  tree.set(2, 9);
  assert((tree.get_all() == std::vector<long long>{3, 15, 9}));
}
```
{% endraw %}

## 操作一覧

N は要素数（slope trick は保持する折れ点数）、M はコピー・結合相手または置換前の保持量。型のコピー・演算・比較は O(1) とする。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DualSegmentTree()` | O(1) | [開く](#default) |
| `DualSegmentTree(const std::vector<S>& values)` | O(N) | [開く](#vector) |
| `DualSegmentTree(int n, const S& value)` | O(N) | [開く](#fill) |
| `DualSegmentTree(const DualSegmentTree& other)` | O(N) | [開く](#copy-constructor) |
| `DualSegmentTree& operator=(const DualSegmentTree& other)` | O(N + M) | [開く](#copy-assignment) |
| `DualSegmentTree(DualSegmentTree&& other)` | O(1) | [開く](#move-constructor) |
| `DualSegmentTree& operator=(DualSegmentTree&& other)` | O(M) | [開く](#move-assignment) |
| `int size() const` | O(1) | [開く](#size) |
| `S get(int p)` | O(log(N+1)) | [開く](#get) |
| `void set(int p, const S& value)` | O(log(N+1)) | [開く](#set) |
| `void apply(int p, const F& f)` | O(log(N+1)) | [開く](#point-apply) |
| `void apply(int l, int r, const F& f)` | O(log(N+1)) | [開く](#range-apply) |
| `std::vector<S> get_all()` | O(N) | [開く](#get-all) |

<details class="api-operation" id="default" markdown="1">
<summary><code>DualSegmentTree()</code> — O(1)</summary>

長さ0の木を作る。

{% raw %}
```cpp
using Tree = blueberry::DualSegmentTree<long long, F, mapping, composition, id>;
Tree tree;
```
{% endraw %}

注意点: get/set の有効な添字はない。

</details>

<details class="api-operation" id="vector" markdown="1">
<summary><code>DualSegmentTree(const std::vector&lt;S&gt;&amp; values)</code> — O(N)</summary>

入力をコピーして構築。

{% raw %}
```cpp
Tree tree(std::vector<S>{1, 2, 3});
```
{% endraw %}

注意点: 空入力も有効。入力の寿命には依存しない。

</details>

<details class="api-operation" id="fill" markdown="1">
<summary><code>DualSegmentTree(int n, const S&amp; value)</code> — O(N)</summary>

全 n 要素が value の木。

{% raw %}
```cpp
Tree tree(10, S(0));
```
{% endraw %}

注意点: 0 <= n <= INT_MAX。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>DualSegmentTree(const DualSegmentTree&amp; other)</code> — O(N)</summary>

内容を複製する。更新は互いに独立。

{% raw %}
```cpp
DualSegmentTree copy(tree);
```
{% endraw %}

注意点: 空状態のコピーも可能。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>DualSegmentTree&amp; operator=(const DualSegmentTree&amp; other)</code> — O(N + M)</summary>

代入元 N 要素で置換し、自身への参照を返す。M は置換前の保持量。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入は内容を保つ。返された参照は代入先が生存する間有効。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>DualSegmentTree(DualSegmentTree&amp;&amp; other)</code> — O(1)</summary>

所有権を移し、元を空状態にする。

{% raw %}
```cpp
DualSegmentTree moved(std::move(tree));
```
{% endraw %}

注意点: <utility> が必要。空状態の意味は概要を参照。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>DualSegmentTree&amp; operator=(DualSegmentTree&amp;&amp; other)</code> — O(M)</summary>

古い保持要素 M 個の破棄を含めて置換し、自身への参照を返す。

{% raw %}
```cpp
moved = std::move(tree);
```
{% endraw %}

注意点: 自己ムーブ代入は内容を保つ。元は空になる。参照の寿命は代入先の寿命と同じ。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

元配列の長さを返す。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 内部のパディングは含まない。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>S get(int p)</code> — O(log(N+1))</summary>

p の現在値をコピーして返す。

{% raw %}
```cpp
auto value = tree.get(0);
```
{% endraw %}

注意点: 0 <= p < N。遅延作用を内部で伝播する。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void set(int p, const S&amp; value)</code> — O(log(N+1))</summary>

p を value で代入する。

{% raw %}
```cpp
tree.set(0, S(5));
```
{% endraw %}

注意点: 0 <= p < N。過去の作用を伝播してから置換する。

</details>

<details class="api-operation" id="point-apply" markdown="1">
<summary><code>void apply(int p, const F&amp; f)</code> — O(log(N+1))</summary>

p の現在値に f を作用させる。

{% raw %}
```cpp
tree.apply(0, F{2, 1});
```
{% endraw %}

注意点: 0 <= p < N。呼出順を保つ。

</details>

<details class="api-operation" id="range-apply" markdown="1">
<summary><code>void apply(int l, int r, const F&amp; f)</code> — O(log(N+1))</summary>

半開区間 [l,r) に f を作用させる。

{% raw %}
```cpp
tree.apply(0, tree.size(), F{2, 1});
```
{% endraw %}

注意点: 0 <= l <= r <= N。空区間は何もしない。非可換な作用でも時系列順に合成する。

</details>

<details class="api-operation" id="get-all" markdown="1">
<summary><code>std::vector&lt;S&gt; get_all()</code> — O(N)</summary>

すべての遅延作用を伝播し、現在値をコピーして返す。

{% raw %}
```cpp
auto values = tree.get_all();
```
{% endraw %}

注意点: 空なら空vector。返したvectorの変更は木に影響しない。

</details>

## 出典・検証

[maspypy Dual SegTree](https://maspypy.github.io/library/ds/segtree/dual_segtree.hpp) を設計比較資料とし独立実装。
Library Checker [Range Affine Point Get](https://judge.yosupo.jp/problem/range_affine_point_get)。
`tests/random/specialized-structures.cpp` で非可換 affine 作用・一点代入・空区間を愚直配列と比較。
