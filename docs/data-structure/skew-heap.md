---
title: Skew Heap
documentation_of: //blueberry/data-structure/skew-heap.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::SkewHeap<T, Compare = std::less<T>>` は所有権付き meldable heap。
std::priority_queue にない heap 同士の結合を償却 O(log(N+M+1)) で行う。
push/pop も償却 O(log(N+1))、top O(1)。単発の push/pop/meld は最悪 O(N+M)。
ノード数 N に対しメモリ O(N)。meld と破棄は再帰を使わず、偏った木でも call stack を消費しない。
Compare=less なら最小を返す。比較器は厳密弱順序で、meldする2個の比較器は同じ順序を与えること。
コピー構築・コピー代入は delete。ムーブ元と meld 元は空になり再利用可能。
T はコピー可能、既定構築不要。比較器・値の操作は例外を投げない前提。算術はしない。
キー一括加算・decrease-key・永続化は提供しない。top参照は次の変更または破棄まで有効。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/skew-heap.hpp"
int main() {
  blueberry::SkewHeap<int> tree, other;
  tree.push(4); tree.push(1); other.push(3);
  tree.meld(other);
  assert(other.empty() && tree.size() == 3 && tree.top() == 1);
  tree.pop(); assert(tree.top() == 3);
  tree.meld(tree); assert(tree.size() == 2);
}
```
{% endraw %}

## 操作一覧

N は要素数（slope trick は保持する折れ点数）、M はコピー・結合相手または置換前の保持量。型のコピー・演算・比較は O(1) とする。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SkewHeap(Compare compare = {})` | O(1) | [開く](#default) |
| `SkewHeap(SkewHeap&& other)` | O(1) | [開く](#move-constructor) |
| `SkewHeap& operator=(SkewHeap&& other)` | O(M) | [開く](#move-assignment) |
| `~SkewHeap()` | O(N) | [開く](#destructor) |
| `std::size_t size() const` | O(1) | [開く](#size) |
| `bool empty() const` | O(1) | [開く](#empty) |
| `const T& top() const` | O(1) | [開く](#top) |
| `void push(const T& value)` | O(log(N+1)) 償却 | [開く](#push) |
| `void pop()` | O(log(N+1)) 償却 | [開く](#pop) |
| `void meld(SkewHeap& other)` | O(log(N+M+1)) 償却 | [開く](#meld) |
| `void clear()` | O(N) | [開く](#clear) |

<details class="api-operation" id="default" markdown="1">
<summary><code>SkewHeap(Compare compare = {})</code> — O(1)</summary>

空heapを作る。

{% raw %}
```cpp
blueberry::SkewHeap<int> tree;
```
{% endraw %}

注意点: 比較器を保持する。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>SkewHeap(SkewHeap&amp;&amp; other)</code> — O(1)</summary>

所有権を移し、元を空状態にする。

{% raw %}
```cpp
SkewHeap moved(std::move(tree));
```
{% endraw %}

注意点: <utility> が必要。空状態の意味は概要を参照。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>SkewHeap&amp; operator=(SkewHeap&amp;&amp; other)</code> — O(M)</summary>

古い保持要素 M 個の破棄を含めて置換し、自身への参照を返す。

{% raw %}
```cpp
moved = std::move(tree);
```
{% endraw %}

注意点: 自己ムーブ代入は内容を保つ。元は空になる。参照の寿命は代入先の寿命と同じ。

</details>

<details class="api-operation" id="destructor" markdown="1">
<summary><code>~SkewHeap()</code> — O(N)</summary>

所有ノードをすべて解放する。

{% raw %}
```cpp
{ blueberry::SkewHeap<int> temporary; temporary.push(1); }
```
{% endraw %}

注意点: 明示的にdestructorを呼ばず通常の寿命管理を使う。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>std::size_t size() const</code> — O(1)</summary>

要素数を返す。

{% raw %}
```cpp
auto n = tree.size();
```
{% endraw %}

注意点: 重複を含む。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool empty() const</code> — O(1)</summary>

空なら true。

{% raw %}
```cpp
bool none = tree.empty();
```
{% endraw %}

注意点: ムーブ元・meld元にも使える。

</details>

<details class="api-operation" id="top" markdown="1">
<summary><code>const T&amp; top() const</code> — O(1)</summary>

Compare順の最小要素。

{% raw %}
```cpp
auto value = tree.top();
```
{% endraw %}

注意点: 非空。変更後の参照保持をしない。

</details>

<details class="api-operation" id="push" markdown="1">
<summary><code>void push(const T&amp; value)</code> — O(log(N+1)) 償却</summary>

ノードを1個確保して挿入。

{% raw %}
```cpp
tree.push(5);
```
{% endraw %}

注意点: 単発最悪 O(N)。値・比較器は例外を投げないこと。

</details>

<details class="api-operation" id="pop" markdown="1">
<summary><code>void pop()</code> — O(log(N+1)) 償却</summary>

先頭1個を削除してノードを解放。

{% raw %}
```cpp
tree.pop();
```
{% endraw %}

注意点: 非空。単発最悪 O(N)。

</details>

<details class="api-operation" id="meld" markdown="1">
<summary><code>void meld(SkewHeap&amp; other)</code> — O(log(N+M+1)) 償却</summary>

other の全要素を移して結合し、other を空にする。

{% raw %}
```cpp
tree.meld(other);
```
{% endraw %}

注意点: 自己meldは何もしない O(1)。異なる順序の比較器を結合しない。単発最悪 O(N+M)。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void clear()</code> — O(N)</summary>

全ノードを非再帰で解放する。

{% raw %}
```cpp
tree.clear();
```
{% endraw %}

注意点: 空でも有効。メモリは保持しない。

</details>

## 出典・検証

[Luzhiled Skew Heap](https://ei1333.github.io/library/structure/heap/skew-heap.hpp.html) を設計比較資料とし、
所有権管理と非再帰meldを独立実装。Library Checkerに直接の meldable-heap 問題はない。
[Shortest Path](https://judge.yosupo.jp/problem/shortest_path) のdriverで Dijkstra の緩和候補を別heapに作り、
実際に meld して利用する（すべてのmeldパターンの検証ではない）。
`tests/random/specialized-structures.cpp` は複数heapの meld/move/self-meld を std::multiset と比較し、深いheapの破棄も確認。
