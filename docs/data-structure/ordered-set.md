---
title: Ordered Set
documentation_of: //blueberry/data-structure/ordered-set.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

オンラインで未知のキーを追加・削除できる順序集合。ACL にない順位・k 番目・前後検索を提供する。`OrderedSet<Key,Compare=std::less<Key>>`。重複は保持しない。旧 implicit_treap とは独立した API。

Compare は strict weak ordering。等価は `!comp(a,b) && !comp(b,a)`。Key に算術演算・既定コンストラクタ・operator== は不要で、コピー構築とコピー/ムーブ代入が可能であること。Compare もコピー可能で、検索中に順序が変化しないこと。

独立した64ビット擬似乱数列を使う treap。N を現在の要素数、M を過去の最大要素数とするとメモリ O(M)、ノード番号は int に収まる必要がある。削除ノードを再利用するが容量と削除済みキーの格納領域は解放しない。通常操作の期待計算量は O(log(N+1))、配列拡張を伴う insert と erase の free-list 拡張は償却を含む。最悪の木の高さ・再帰スタックは O(N)、固定 seed に対する敵対入力で最悪保証はない。

検索結果は optional のコピー。参照・iterator・公開フィールドは提供せず、更新による結果の無効化はない。整数の最小値・最大値や文字列を扱え、座標圧縮や入力先読みを必要としない。以下の大小はすべて Compare による順序。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/ordered-set.hpp"
int main() {
  blueberry::OrderedSet<int> s;
  assert(s.insert(8) && s.insert(3));
  assert(!s.insert(3));
  assert(s.kth(0) == 3 && s.rank(8) == 1);
  assert(s.floor(7) == 3 && s.lower_bound(7) == 8);
  assert(s.erase(3) && !s.kth(1));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `OrderedSet(Compare compare={})` | O(1) | [開く](#construct) |
| `int size() const` | O(1) | [開く](#size) |
| `bool empty() const` | O(1) | [開く](#empty) |
| `bool insert(const Key& x)` | 期待・償却 O(log(N+1)) | [開く](#insert) |
| `bool erase(const Key& x)` | 期待・償却 O(log(N+1)) | [開く](#erase) |
| `bool contains(const Key& x) const` | 期待 O(log(N+1)) | [開く](#contains) |
| `int rank(const Key& x) const` | 期待 O(log(N+1)) | [開く](#rank) |
| `std::optional<Key> kth(int k) const` | 期待 O(log(N+1)) | [開く](#kth) |
| `std::optional<Key> lower_bound(const Key& x) const` | 期待 O(log(N+1)) | [開く](#lower-bound) |
| `std::optional<Key> upper_bound(const Key& x) const` | 期待 O(log(N+1)) | [開く](#upper-bound) |
| `std::optional<Key> floor(const Key& x) const` | 期待 O(log(N+1)) | [開く](#floor) |
| `OrderedSet(const OrderedSet&); operator=(const OrderedSet&)` | O(M) | [開く](#copy) |
| `OrderedSet(OrderedSet&&); operator=(OrderedSet&&)` | O(1) ※代入先解放 O(M) | [開く](#move) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>OrderedSet(Compare compare={})</code> — O(1)</summary>

空集合を作る。比較関数を値で保持する。

{% raw %}
```cpp
blueberry::OrderedSet<int, std::greater<int>> descending;
```
{% endraw %}

注意点: greater の場合、kth(0) は数値最大。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

現在の異なるキー数。

{% raw %}
```cpp
assert(s.size() == 1);
```
{% endraw %}

注意点: int に収まる要素数を扱う。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool empty() const</code> — O(1)</summary>

空集合なら true。

{% raw %}
```cpp
assert(!s.empty());
```
{% endraw %}

注意点: 初期状態と全要素削除後は true。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>bool insert(const Key&amp; x)</code> — 期待・償却 O(log(N+1))</summary>

x と等価なキーがなければ追加して true。既存なら false で元のキーを保つ。

{% raw %}
```cpp
assert(s.insert(10));
```
{% endraw %}

注意点: 追加時にキーをコピーする。最悪は O(M+1)（配列拡張を含む）。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>bool erase(const Key&amp; x)</code> — 期待・償却 O(log(N+1))</summary>

等価なキーを削除して true。存在しなければ false。

{% raw %}
```cpp
assert(!s.erase(100));
```
{% endraw %}

注意点: 削除スロットを再利用する。最悪 O(M+1)。

</details>

<details class="api-operation" id="contains" markdown="1">
<summary><code>bool contains(const Key&amp; x) const</code> — 期待 O(log(N+1))</summary>

x と等価なキーが存在するかを返す。

{% raw %}
```cpp
assert(s.contains(8));
```
{% endraw %}

注意点: 空なら false。最悪 O(N)。

</details>

<details class="api-operation" id="rank" markdown="1">
<summary><code>int rank(const Key&amp; x) const</code> — 期待 O(log(N+1))</summary>

x より前にあるキー数（order_of_key 相当）。

{% raw %}
```cpp
assert(s.rank(8) == 0);
```
{% endraw %}

注意点: x が存在しなくても使える。空なら 0。最悪 O(N)。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>std::optional&lt;Key&gt; kth(int k) const</code> — 期待 O(log(N+1))</summary>

0 始まりの k 番目をコピーして返す。

{% raw %}
```cpp
assert(s.kth(0) == 8);
assert(!s.kth(-1));
```
{% endraw %}

注意点: k<0 または k>=size() は nullopt。最悪 O(N)。

</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>std::optional&lt;Key&gt; lower_bound(const Key&amp; x) const</code> — 期待 O(log(N+1))</summary>

x 以上の最初のキー。

{% raw %}
```cpp
assert(s.lower_bound(7) == 8);
```
{% endraw %}

注意点: 存在しなければ nullopt。最悪 O(N)。

</details>

<details class="api-operation" id="upper-bound" markdown="1">
<summary><code>std::optional&lt;Key&gt; upper_bound(const Key&amp; x) const</code> — 期待 O(log(N+1))</summary>

x より大きい最初のキー。

{% raw %}
```cpp
assert(s.upper_bound(8) == 10);
```
{% endraw %}

注意点: 存在しなければ nullopt。最悪 O(N)。

</details>

<details class="api-operation" id="floor" markdown="1">
<summary><code>std::optional&lt;Key&gt; floor(const Key&amp; x) const</code> — 期待 O(log(N+1))</summary>

x 以下の最後のキー。

{% raw %}
```cpp
assert(s.floor(9) == 8);
```
{% endraw %}

注意点: 存在しなければ nullopt。greater では数値 x 以上のうち最小になる。最悪 O(N)。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>OrderedSet(const OrderedSet&amp;); operator=(const OrderedSet&amp;)</code> — O(M)</summary>

コピーはノードと乱数状態を複製し、元の集合と独立する。

{% raw %}
```cpp
auto copy = s;
copy.erase(8);
assert(s.contains(8));
```
{% endraw %}

注意点: 代入は古い格納領域の破棄も必要。Key/Compare がコピー可能であること。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>OrderedSet(OrderedSet&amp;&amp;); operator=(OrderedSet&amp;&amp;)</code> — O(1) ※代入先解放 O(M)</summary>

格納領域を移す。移動元は空集合として再利用できる。Compare はコピーする。

{% raw %}
```cpp
auto moved = std::move(s);
assert(s.empty());
s.insert(4);
```
{% endraw %}

注意点: 自己ムーブ代入は無操作。Compare のコピーに要する時間は別。

</details>

## 出典・検証

[公式 Ordered Set](https://judge.yosupo.jp/problem/ordered_set) と std::set による固定 seed の独立比較で検証。昇順・降順、重複、削除、整数端点、文字列、コピー・ムーブを確認する。

treap の考え方を [KACTL](https://github.com/kth-competitive-programming/kactl/blob/main/content/data-structures/Treap.h)、ノードプールを [suisen](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/datastructure/bbst/implicit_treap.hpp) と比較し、順序集合として独立実装した。
Fastest [403024](https://judge.yosupo.jp/submission/403024) は30ビット整数の radix 木、[400970](https://judge.yosupo.jp/submission/400970) はクエリを先読みして座標圧縮する。本 API は比較可能な汎用キーとオンライン更新を維持する。プール・ポインタ候補の同条件測定は `benchmark/data-structure-expansion.cpp` と `benchmark/results/lc-expansion-ds/` を参照。
