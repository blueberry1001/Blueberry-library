---
title: Dynamic Fenwick Tree
documentation_of: //blueberry/data-structure/dynamic-fenwick-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

事前の座標登録なしで、巨大な整数区間 `[0,n)` に点加算・区間和・重み付き探索を行う。
通常の密な配列には ACL `fenwick_tree` を優先する。本実装は Fenwick セルを
`unordered_map` に保存し、初期化時に `n` 個の配列を確保しない。

`DynamicFenwickTree<T, Coord = long long>` の `Coord` は符号付き整数型。
`0 <= n <= numeric_limits<Coord>::max()`。`T{}` が加算の単位元で、コピー・`+=`・二項 `-`
が必要。区間和には可換加法群を仮定する。`lower_bound` のみさらに `<` と非負な各点の値を要求する。
整数・ACL modint を使えるが、modint では重み付き探索を使わない。全中間演算を含め、`T` の
オーバーフローを呼び出し側で防ぐ。浮動小数点には演算順による誤差がある。

以下で `L = 1 + floor(log2(max(1,n)))`、`Q` はこれまで加算した異なる位置の数、
`S` は保存セル数とし、`S = O(min(n, QL))`。メモリは `O(1+S)`。
表の計算量はハッシュが十分分散する場合の平均で、更新の再ハッシュを償却する。
衝突が集中する最悪時の探索は `O(L(1+S))`、更新は再ハッシュも含め `O(L(1+S)+S²)`。
`T` のコピー・演算は `O(1)` とする。ゼロを加えた位置・ゼロに戻ったセルも保持し、削除しない。
読み取り操作はセルを挿入しない。公開フィールド・参照を返す操作はない。

旧 `DynamicFenwickTree<S,T>` からはテンプレート引数順を `T,Coord` に変更し、
`sum(r)` は `pref(r)`、`operator[](p)` は `get(p)` に移行する。
負の prefix 終端を暗黙にゼロ扱いせず、すべての添字条件を `assert` で検査する。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"
int main() {
  blueberry::DynamicFenwickTree<long long> tree(1000000000000LL);
  tree.add(3, 4);
  tree.add(999999999999LL, 7);
  assert(tree.pref(4) == 4);
  assert(tree.sum(4, tree.size()) == 7);
  assert(tree.get(3) == 4);
  assert(tree.lower_bound(5) == 999999999999LL);
  assert(tree.lower_bound(12) == tree.size());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DynamicFenwickTree<T,Coord> tree` | O(1) | [開く](#default) |
| `DynamicFenwickTree<T,Coord> tree(n)` | O(1) | [開く](#construct) |
| `DynamicFenwickTree<T,Coord> copy(other)` | O(1+S) | [開く](#copy-construct) |
| `tree = other` → tree の参照 | O(1+S+S_old) | [開く](#copy-assign) |
| `DynamicFenwickTree<T,Coord> moved(std::move(other))` | O(1) | [開く](#move-construct) |
| `tree = std::move(other)` → tree の参照 | O(1+S_old) | [開く](#move-assign) |
| `Coord tree.size() const` | O(1) | [開く](#size) |
| `void tree.add(Coord p, const T& value)` | 平均・償却 O(L) | [開く](#add) |
| `T tree.pref(Coord r) const` | 平均 O(L) | [開く](#pref) |
| `T tree.sum(Coord l, Coord r) const` | 平均 O(L) | [開く](#sum) |
| `T tree.get(Coord p) const` | 平均 O(L) | [開く](#get) |
| `Coord tree.lower_bound(T weight) const` | 平均 O(L) | [開く](#lower-bound) |
| `tree.~DynamicFenwickTree()` | O(1+S) | [開く](#destroy) |

<details class="api-operation" id="default" markdown="1">
<summary><code>DynamicFenwickTree&lt;T,Coord&gt; tree</code> — O(1)</summary>

注意点: 長さゼロの空構造を作る。`pref(0)`・`sum(0,0)` はゼロ、`lower_bound` は常にゼロ。
更新・`get` の有効な位置はない。

{% raw %}
```cpp
blueberry::DynamicFenwickTree<long long> empty;
assert(empty.size() == 0 && empty.sum(0, 0) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="construct" markdown="1">
<summary><code>DynamicFenwickTree&lt;T,Coord&gt; tree(Coord n)</code> — O(1)</summary>

注意点: `n >= 0` の全点をゼロとして作る。最大の `Coord` も有効。初期メモリは O(1)。

{% raw %}
```cpp
blueberry::DynamicFenwickTree<long long> tree(1000000000000LL);
assert(tree.pref(tree.size()) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>DynamicFenwickTree copy(const DynamicFenwickTree&amp; other)</code> — O(1+S)</summary>

注意点: サイズ・セルを独立してコピーする。追加メモリ O(1+S)。元とコピーの更新は互いに影響しない。

{% raw %}
```cpp
auto copy = tree;
assert(copy.pref(copy.size()) == tree.pref(tree.size()));
```
{% endraw %}
</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>DynamicFenwickTree&amp; tree.operator=(const DynamicFenwickTree&amp; other)</code> — O(1+S+S_old)</summary>

注意点: 元の `S_old` セルを置き換え、独立コピーを代入する。返り値は `tree` 自身の参照。
自己代入も有効。

{% raw %}
```cpp
blueberry::DynamicFenwickTree<long long> copy;
copy = tree;
```
{% endraw %}
</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>DynamicFenwickTree moved(DynamicFenwickTree&amp;&amp; other)</code> — O(1)</summary>

注意点: 所有権を移す。元はサイズゼロの空構造となり、空の問い合わせ・再代入が可能。
返された参照を保持する操作はない。例には `<utility>` を含める。

{% raw %}
```cpp
auto moved = std::move(tree);
assert(tree.size() == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>DynamicFenwickTree&amp; tree.operator=(DynamicFenwickTree&amp;&amp; other)</code> — O(1+S_old)</summary>

注意点: 元の格納領域を解放して所有権を移す。元は空、返り値は `tree` の参照。自己 move は何もしない。

{% raw %}
```cpp
blueberry::DynamicFenwickTree<long long> moved;
moved = std::move(tree);
assert(tree.size() == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>Coord tree.size() const</code> — O(1)</summary>

注意点: 位置の上限 `n` を返す。保存セル数・更新済み点数ではない。

{% raw %}
```cpp
auto n = tree.size();
assert(tree.sum(n, n) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void tree.add(Coord p, const T&amp; value)</code> — 平均・償却 O(L)</summary>

注意点: `0 <= p < n` の点に `value` を加算する。事前登録不要。負の加算も可能だが
`lower_bound` を使うときは各点の累積値を非負に保つ。最大 O(L) セルを新規保存する。
添字加算は最大 `Coord` でもオーバーフローしない。値のオーバーフローは利用側の責任。

{% raw %}
```cpp
tree.add(3, -2);
```
{% endraw %}
</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>T tree.pref(Coord r) const</code> — 平均 O(L)</summary>

注意点: `0 <= r <= n`。`[0,r)` の和を値で返す。`r=0` では `T{}`。セル挿入なし。

{% raw %}
```cpp
assert(tree.pref(0) == 0);
auto total = tree.pref(tree.size());
```
{% endraw %}
</details>

<details class="api-operation" id="sum" markdown="1">
<summary><code>T tree.sum(Coord l, Coord r) const</code> — 平均 O(L)</summary>

注意点: `0 <= l <= r <= n`。`[l,r)` の和を値で返す。空区間ではゼロ。
減算を含む中間値も `T` に収まる必要がある。

{% raw %}
```cpp
assert(tree.sum(3, 4) == tree.get(3));
```
{% endraw %}
</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T tree.get(Coord p) const</code> — 平均 O(L)</summary>

注意点: `0 <= p < n` の現在値をコピーして返す。未更新の点はゼロ。返り値の変更は構造に影響しない。

{% raw %}
```cpp
auto value = tree.get(3);
```
{% endraw %}
</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>Coord tree.lower_bound(T weight) const</code> — 平均 O(L)</summary>

注意点: 各点の値が非負のとき、`pref(p+1) >= weight` となる最小の **0-indexed 位置** `p` を返す。
存在しなければ `n`。`weight <= 0` では `0`、空構造でも `0`。`T` に整合する `<` が必要。
未保存セルの読み取りでもメモリを増やさない。条件違反の負値・順序のない群には使用しない。

{% raw %}
```cpp
blueberry::DynamicFenwickTree<long long> weights(10);
weights.add(4, 3);
assert(weights.lower_bound(1) == 4 && weights.lower_bound(4) == 10);
```
{% endraw %}
</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>tree.~DynamicFenwickTree()</code> — O(1+S)</summary>

注意点: スコープ終了時に所有するセルをすべて解放する。明示呼び出しは通常不要。

{% raw %}
```cpp
{ blueberry::DynamicFenwickTree<long long> temporary(5); temporary.add(2, 1); }
```
{% endraw %}
</details>

## 出典・検証

- [Nyaan: Dynamic Binary Indexed Tree](https://nyaannyaan.github.io/library/data-structure/dynamic-binary-indexed-tree.hpp.html): 疎な Fenwick セルと重み付き探索の設計を比較。実装は独立に記述。
- [ACL Fenwick Tree](https://atcoder.github.io/ac-library/production/document_ja/fenwicktree.html): 密な通常配列はこちらを優先。
- [Library Checker: Point Add Range Sum](https://judge.yosupo.jp/problem/point_add_range_sum): `verify/data-structure/dynamic-point-add-range-sum.test.cpp`。
- `tests/random/dynamic-fenwick-tree.cpp`: 固定 seed、愚直列挙、負の値、ゼロギャップの探索、最大整数座標、コピー・move を検証。
