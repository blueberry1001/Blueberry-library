---
title: Fast Set (64-ary tree)
documentation_of: //blueberry/data-structure/fast-set.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::FastSet` は固定整数宇宙 `[0, U)` の集合。64-bit 語の非空状態を階層化し、
前後の要素を最悪 O(1 + log₆₄ U) で探す。既存 OrderedSet と異なり比較木や乱数を使わない。
重複は保持せず、rank / kth は非対応。巨大で疎な宇宙には OrderedSet / BinaryTrie を使う。
`0 <= U <= INT_MAX`。メモリは O(U / 64 + log₆₄(U + 1) + 1) 個の64-bit語。
以下 W はこの語数、H = 1 + log₆₄(U + 1)。size() は現在の要素数、universe_size() は U。
空の既定状態・ムーブ元は U=0。clear() は宇宙を維持して全要素を消す。
数値加算はしない。返り値は値で、参照の無効化はない。コピーは独立。
コピー構築は O(W)、コピー代入は両者の保持語数の和、ムーブ代入は旧保持語数の解放を含む。

insert は葉の64-bit語が空から非空へ変わるときだけ上位へ伝播します。
既に非空の語へ要素を追加しても、その語を表す上位bitは変わらないためです。
密な集合では上位への書き込みを省けますが、最悪計算量は従来どおり O(H) です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include "blueberry/data-structure/fast-set.hpp"
int main() {
  blueberry::FastSet tree(std::string("01001"));
  assert(tree.size() == 2 && tree.universe_size() == 5);
  assert(tree.next(2) == 4 && tree.prev(3) == 1);
  assert(tree.insert(2) && !tree.insert(2));
  assert(tree.erase(1));
  assert(tree.next(5) == 5 && tree.prev(-1) == -1);
  tree.clear();
  assert(tree.empty());
}
```
{% endraw %}

## 操作一覧

W はコピー元の保持語数、V は代入先の旧保持語数。型のコピー・演算・比較は O(1) とする。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `FastSet()` | O(1) | [開く](#default) |
| `FastSet(int universe)` | O(W) | [開く](#universe) |
| `FastSet(const std::string& bits)` | O(U) | [開く](#bits) |
| `FastSet(const FastSet& other)` | O(W) | [開く](#copy-constructor) |
| `FastSet& operator=(const FastSet& other)` | O(W + V) | [開く](#copy-assignment) |
| `FastSet(FastSet&& other)` | O(1) | [開く](#move-constructor) |
| `FastSet& operator=(FastSet&& other)` | O(1 + V) | [開く](#move-assignment) |
| `int size() const` | O(1) | [開く](#size) |
| `int universe_size() const` | O(1) | [開く](#universe-size) |
| `bool empty() const` | O(1) | [開く](#empty) |
| `bool contains(int x) const` | O(1) | [開く](#contains) |
| `bool insert(int x)` | O(H) | [開く](#insert) |
| `bool erase(int x)` | O(H) | [開く](#erase) |
| `int next(int x) const` | O(H) | [開く](#next) |
| `int prev(int x) const` | O(H) | [開く](#prev) |
| `void clear()` | O(W) | [開く](#clear) |

<details class="api-operation" id="default" markdown="1">
<summary><code>FastSet()</code> — O(1)</summary>

宇宙0の空集合を作る。

{% raw %}
```cpp
blueberry::FastSet tree;
```
{% endraw %}

注意点: 要素を追加するにはサイズ付きの集合を代入する。

</details>

<details class="api-operation" id="universe" markdown="1">
<summary><code>FastSet(int universe)</code> — O(W)</summary>

指定した宇宙 U の空集合を作る。

{% raw %}
```cpp
blueberry::FastSet tree(100);
```
{% endraw %}

注意点: 0 <= universe <= INT_MAX。

</details>

<details class="api-operation" id="bits" markdown="1">
<summary><code>FastSet(const std::string&amp; bits)</code> — O(U)</summary>

0/1文字列から集合を構築。bits[i]==1 の位置を登録する。

{% raw %}
```cpp
blueberry::FastSet tree(std::string("1010"));
```
{% endraw %}

注意点: 文字は 0 または 1、長さ <= INT_MAX。空文字列も有効。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>FastSet(const FastSet&amp; other)</code> — O(W)</summary>

内容を複製する。更新は互いに独立。

{% raw %}
```cpp
FastSet copy(tree);
```
{% endraw %}

注意点: 空状態のコピーも可能。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>FastSet&amp; operator=(const FastSet&amp; other)</code> — O(W + V)</summary>

代入元 N 要素で置換し、自身への参照を返す。M は置換前の保持量。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入は内容を保つ。返された参照は代入先が生存する間有効。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>FastSet(FastSet&amp;&amp; other)</code> — O(1)</summary>

所有権を移し、元を空状態にする。

{% raw %}
```cpp
FastSet moved(std::move(tree));
```
{% endraw %}

注意点: <utility> が必要。空状態の意味は概要を参照。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>FastSet&amp; operator=(FastSet&amp;&amp; other)</code> — O(1 + V)</summary>

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

現在の登録要素数を返す。

{% raw %}
```cpp
int count = tree.size();
```
{% endraw %}

注意点: 宇宙の大きさではない。空なら0。

</details>

<details class="api-operation" id="universe-size" markdown="1">
<summary><code>int universe_size() const</code> — O(1)</summary>

固定した宇宙 U を返す。

{% raw %}
```cpp
int u = tree.universe_size();
```
{% endraw %}

注意点: clear で変わらず、ムーブ元は0。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool empty() const</code> — O(1)</summary>

登録要素がなければ true。

{% raw %}
```cpp
bool none = tree.empty();
```
{% endraw %}

注意点: U>0 でも空になり得る。

</details>

<details class="api-operation" id="contains" markdown="1">
<summary><code>bool contains(int x) const</code> — O(1)</summary>

x の登録状態を返す。

{% raw %}
```cpp
bool present = tree.contains(0);
```
{% endraw %}

注意点: 0 <= x < U。U=0 では呼べない。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>bool insert(int x)</code> — O(H)</summary>

x を登録し、新規追加なら true を返す。

{% raw %}
```cpp
bool added = tree.insert(0);
```
{% endraw %}

注意点: 0 <= x < U。既に存在すれば false、重複は増えない。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>bool erase(int x)</code> — O(H)</summary>

x を削除し、存在していた場合に true。

{% raw %}
```cpp
bool removed = tree.erase(0);
```
{% endraw %}

注意点: 0 <= x < U。存在しなければ false。

</details>

<details class="api-operation" id="next" markdown="1">
<summary><code>int next(int x) const</code> — O(H)</summary>

x 以上の最小要素。なければ U。

{% raw %}
```cpp
int next = tree.next(0);
```
{% endraw %}

注意点: 全 int 入力が有効。x<0 は0に丸め、x>=U なら U。

</details>

<details class="api-operation" id="prev" markdown="1">
<summary><code>int prev(int x) const</code> — O(H)</summary>

x 以下の最大要素。なければ -1。

{% raw %}
```cpp
int previous = tree.prev(tree.universe_size());
```
{% endraw %}

注意点: 全 int 入力が有効。x>=U は U-1 に丸め、x<0 なら -1。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void clear()</code> — O(W)</summary>

全要素を消し、確保済み領域と宇宙を維持。

{% raw %}
```cpp
tree.clear();
```
{% endraw %}

注意点: 空集合でも全保持語を0にする。

</details>

## 出典・検証

[maspypy FastSet](https://maspypy.github.io/library/ds/fastset.hpp) の64分木設計を比較資料とし独立実装。
Library Checker [Predecessor Problem](https://judge.yosupo.jp/problem/predecessor_problem) と
`tests/random/specialized-structures.cpp` の std::set 比較。0/1/63/64/65/4095/4096/4097 の境界を含む。

