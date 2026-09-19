---
title: Double-Ended Priority Queue
documentation_of: //blueberry/data-structure/double-ended-priority-queue.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::DoubleEndedPriorityQueue<T, Compare = std::less<T>>` は配列上の min-max heap。
現在の最小・最大の両方を O(1) で参照できる。push / pop_min / pop_max は比較 O(log(N+1))、
push は vector 再確保を含めて償却 O(log(N+1))、取り出しは最悪 O(log(N+1))。
メモリは保持容量 C に対して O(C)、通常の増加利用では O(N)（clear 後も容量は残る）。
既存 OrderedMultiset より機能を限定し、木のノードや順序統計・集約を持たない。
Compare は厳密弱順序で、std::less なら通常の最小・最大。逆順なら両者の意味も逆になる。
T はコピー・交換可能、既定構築は不要。重複を保持する。要素値の算術はしない。
min/max は空で呼べない。返す const 参照は破棄・代入または次の変更操作まで有効とする。
コピーは独立、ムーブ元は空。比較器・要素の操作は例外を投げないことを前提とする。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/double-ended-priority-queue.hpp"
int main() {
  blueberry::DoubleEndedPriorityQueue<int> tree(std::vector<int>{4, 1, 7, 1});
  assert(tree.min() == 1 && tree.max() == 7);
  tree.pop_min(); tree.pop_max(); tree.push(9);
  assert(tree.size() == 3 && tree.min() == 1 && tree.max() == 9);
}
```
{% endraw %}

## 操作一覧

N は要素数（slope trick は保持する折れ点数）、M はコピー・結合相手または置換前の保持量。型のコピー・演算・比較は O(1) とする。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DoubleEndedPriorityQueue(Compare compare = {})` | O(1) | [開く](#default) |
| `DoubleEndedPriorityQueue(const std::vector<T>& values, Compare compare = {})` | O(N) | [開く](#vector) |
| `DoubleEndedPriorityQueue(const DoubleEndedPriorityQueue& other)` | O(N) | [開く](#copy-constructor) |
| `DoubleEndedPriorityQueue& operator=(const DoubleEndedPriorityQueue& other)` | O(N + M) | [開く](#copy-assignment) |
| `DoubleEndedPriorityQueue(DoubleEndedPriorityQueue&& other)` | O(1) | [開く](#move-constructor) |
| `DoubleEndedPriorityQueue& operator=(DoubleEndedPriorityQueue&& other)` | O(M) | [開く](#move-assignment) |
| `std::size_t size() const` | O(1) | [開く](#size) |
| `bool empty() const` | O(1) | [開く](#empty) |
| `const T& min() const` | O(1) | [開く](#min) |
| `const T& max() const` | O(1) | [開く](#max) |
| `void push(const T& value)` | O(log(N+1)) 償却 | [開く](#push) |
| `void pop_min()` | O(log(N+1)) | [開く](#pop-min) |
| `void pop_max()` | O(log(N+1)) | [開く](#pop-max) |
| `void clear()` | O(N) | [開く](#clear) |
| `void reserve(std::size_t capacity)` | O(N) | [開く](#reserve) |

<details class="api-operation" id="default" markdown="1">
<summary><code>DoubleEndedPriorityQueue(Compare compare = {})</code> — O(1)</summary>

比較器を保持した空heapを作る。

{% raw %}
```cpp
blueberry::DoubleEndedPriorityQueue<int> tree;
```
{% endraw %}

注意点: 比較器は以後同じ順序を与えること。

</details>

<details class="api-operation" id="vector" markdown="1">
<summary><code>DoubleEndedPriorityQueue(const std::vector&lt;T&gt;&amp; values, Compare compare = {})</code> — O(N)</summary>

配列をコピーし、bottom-upでheap化する。

{% raw %}
```cpp
blueberry::DoubleEndedPriorityQueue<int> tree(std::vector<int>{3, 1, 2});
```
{% endraw %}

注意点: 空入力・重複も有効。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>DoubleEndedPriorityQueue(const DoubleEndedPriorityQueue&amp; other)</code> — O(N)</summary>

内容を複製する。更新は互いに独立。

{% raw %}
```cpp
DoubleEndedPriorityQueue copy(tree);
```
{% endraw %}

注意点: 空状態のコピーも可能。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>DoubleEndedPriorityQueue&amp; operator=(const DoubleEndedPriorityQueue&amp; other)</code> — O(N + M)</summary>

代入元 N 要素で置換し、自身への参照を返す。M は置換前の保持量。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入は内容を保つ。返された参照は代入先が生存する間有効。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>DoubleEndedPriorityQueue(DoubleEndedPriorityQueue&amp;&amp; other)</code> — O(1)</summary>

所有権を移し、元を空状態にする。

{% raw %}
```cpp
DoubleEndedPriorityQueue moved(std::move(tree));
```
{% endraw %}

注意点: <utility> が必要。空状態の意味は概要を参照。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>DoubleEndedPriorityQueue&amp; operator=(DoubleEndedPriorityQueue&amp;&amp; other)</code> — O(M)</summary>

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

重複込みの要素数。

{% raw %}
```cpp
auto n = tree.size();
```
{% endraw %}

注意点: 空なら0。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool empty() const</code> — O(1)</summary>

空なら true。

{% raw %}
```cpp
bool none = tree.empty();
```
{% endraw %}

注意点: clear/move後も利用可能。

</details>

<details class="api-operation" id="min" markdown="1">
<summary><code>const T&amp; min() const</code> — O(1)</summary>

Compare順の最小要素を参照。

{% raw %}
```cpp
auto value = tree.min();
```
{% endraw %}

注意点: 非空。保持参照は次の変更操作で無効になり得る。

</details>

<details class="api-operation" id="max" markdown="1">
<summary><code>const T&amp; max() const</code> — O(1)</summary>

Compare順の最大要素を参照。

{% raw %}
```cpp
auto value = tree.max();
```
{% endraw %}

注意点: 非空。1要素では min と同じ要素。

</details>

<details class="api-operation" id="push" markdown="1">
<summary><code>void push(const T&amp; value)</code> — O(log(N+1)) 償却</summary>

要素を1個追加。

{% raw %}
```cpp
tree.push(4);
```
{% endraw %}

注意点: vector再確保を含む単発の最悪は O(N)。

</details>

<details class="api-operation" id="pop-min" markdown="1">
<summary><code>void pop_min()</code> — O(log(N+1))</summary>

最小要素を1個削除。返り値はない。

{% raw %}
```cpp
tree.pop_min();
```
{% endraw %}

注意点: 非空。値が必要なら先に min をコピーする。

</details>

<details class="api-operation" id="pop-max" markdown="1">
<summary><code>void pop_max()</code> — O(log(N+1))</summary>

最大要素を1個削除。返り値はない。

{% raw %}
```cpp
tree.pop_max();
```
{% endraw %}

注意点: 非空。重複のうち1個だけ消す。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void clear()</code> — O(N)</summary>

全要素を破棄し容量を保持。

{% raw %}
```cpp
tree.clear();
```
{% endraw %}

注意点: 空でも有効。

</details>

<details class="api-operation" id="reserve" markdown="1">
<summary><code>void reserve(std::size_t capacity)</code> — O(N)</summary>

必要に応じて容量を確保する。

{% raw %}
```cpp
tree.reserve(1000);
```
{% endraw %}

注意点: 既存容量以下なら O(1)。容量以上のメモリを確保できること。参照が無効化され得る。

</details>

## 出典・検証

[Sartaj Sahni: Double-Ended Priority Queues](https://www.cise.ufl.edu/~sahni/dsaaj/enrich/c13/double.htm)
に挙げられた heap 系設計を比較し、min/max の深さを交互に持つ min-max heap を独立実装。
Library Checker [Double-Ended Priority Queue](https://judge.yosupo.jp/problem/double_ended_priority_queue)。
`tests/random/specialized-structures.cpp` で std::multiset、重複、逆順比較器、bulk構築と比較。
