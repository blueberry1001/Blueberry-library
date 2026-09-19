---
title: Sqrt Tree（静的）
documentation_of: //blueberry/data-structure/sqrt-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::SqrtTree<S, op, e>` は更新のない配列のモノイド区間積を最悪 O(1) で返す。
ACL segtree が必要とする O(log N) のクエリを、前計算と追加メモリで高速化する。
Disjoint Sparse Table の O(N log N) に対して、構築・メモリは O(N L)、
L = 1 + log log(N + 2)。N は要素数で `0 <= N <= INT_MAX`。
実測の優劣は要素型・N・クエリ分布に依存するため、常に DST より速いとは限らない。

`S op(S, S)` は結合的、`S e()` は左右単位元を返すこと。可換性・冪等性は不要で、
積は左から右の順序を保つ。`S` はコピー可能であればよく、既定コンストラクタは不要。
計算量は `op`・`e`・コピーが O(1) の場合。和・積のオーバーフローは呼び出し側が防ぐ。
空配列・空区間に対応し、値の更新はできない。入力配列はコピーされるため構築後に破棄してよい。
各層で N 要素の prefix・suffix と、必要な層だけに2冪に切り上げた長さのブロック間積領域を保持する。
存在しない末尾要素や、両端ブロックだけで処理できるブロック間積は前計算しない。
クエリ時の `op` 呼び出しは最大 2 回。返り値はすべて値で、参照の寿命や無効化はない。

このクラスは静的クエリに特化した版として維持する。
一点更新が必要な場合は [Dynamic Sqrt Tree]({{ '/blueberry/data-structure/dynamic-sqrt-tree.hpp.html' | relative_url }}) を使う。

| 版 | 区間積 | 一点更新 | 用途 |
| --- | --- | --- | --- |
| `SqrtTree` | O(1)、`op` は最大2回 | 非対応 | 構築後に変更しない配列 |
| `DynamicSqrtTree` | O(1) | O(√N) | 更新が少なく区間積が多い配列 |

いずれも構築・メモリは O(N L)。更新頻度が高い場合は、更新と区間積がともに O(log N) の ACL segtree と比較する。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/sqrt-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  blueberry::SqrtTree<long long, op, e> tree(std::vector<long long>{3, 1, 4, 1, 5});
  assert(tree.size() == 5);
  assert(tree.get(2) == 4);
  assert(tree.prod(1, 4) == 6);
  assert(tree.prod(5, 5) == 0);
  assert(tree.all_prod() == 14);
}
```
{% endraw %}

## 操作一覧

以下 `Tree = blueberry::SqrtTree<S, op, e>`。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Tree tree` | O(1) | [開く](#default-constructor) |
| `Tree tree(const std::vector<S>& values)` | O(N L) | [開く](#constructor) |
| `int tree.size()` | O(1) | [開く](#size) |
| `S tree.get(int p)` | O(1) | [開く](#get) |
| `S tree.prod(int l, int r)` | O(1) | [開く](#prod) |
| `S tree.all_prod()` | O(1) | [開く](#all-prod) |

<details class="api-operation" id="default-constructor" markdown="1">
<summary><code>Tree tree</code> — O(1)</summary>

空配列として構築する。`size() == 0`、`prod(0, 0)` と `all_prod()` は `e()`。

{% raw %}
```cpp
Tree empty;
assert(empty.size() == 0);
```
{% endraw %}

注意点: 空のため `get` の有効な添字はない。
</details>

<details class="api-operation" id="constructor" markdown="1">
<summary><code>Tree tree(const std::vector&lt;S&gt;&amp; values)</code> — O(N L)</summary>

`values` のコピーと静的区間積の前計算を行う。空入力も許される。

{% raw %}
```cpp
Tree tree(std::vector<S>{e(), e()});
assert(tree.size() == 2);
```
{% endraw %}

注意点: サイズは `INT_MAX` 以下。計算量は最悪で、償却ではない。構築時にも演算が
オーバーフローしないこと。暗黙のコピーは O(N L)、ムーブ後の元オブジェクトは
破棄または代入してから再利用する。
</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size()</code> — O(1)</summary>

元の配列の要素数 N を返す。内部のパディングは含まない。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 空配列では 0。
</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>S tree.get(int p)</code> — O(1)</summary>

0-indexed の要素 `values[p]` をコピーして返す。

{% raw %}
```cpp
// tree.size() > 0
S first = tree.get(0);
```
{% endraw %}

注意点: `0 <= p < N`。返した値の変更は木に影響しない。
</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S tree.prod(int l, int r)</code> — O(1)</summary>

半開区間 `[l, r)` の積を配列順に返す。`l == r` なら `e()`。

{% raw %}
```cpp
S result = tree.prod(0, tree.size());
```
{% endraw %}

注意点: `0 <= l <= r <= N`。非可換な関数合成にも使える。境界探索 `max_right` /
`min_left` や更新は非対応なので、それらが必要なら ACL segtree を選ぶ。
</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>S tree.all_prod()</code> — O(1)</summary>

配列全体の積、すなわち `prod(0, size())` を返す。

{% raw %}
```cpp
S total = tree.all_prod();
```
{% endraw %}

注意点: 空配列では `e()`。状態は変化しない。
</details>

## 出典・検証

- [CP-Algorithms: Sqrt Tree](https://cp-algorithms.com/data_structures/sqrt-tree.html)
  の層別 prefix/suffix/between と XOR による層選択を参考に、静的用途として独立実装。
  更新用の補助 Sqrt Tree は保持しない。
- [AtCoderInfo: Sqrt Tree](https://info.atcoder.jp/entry/algorithm_lectures/sqrt_tree)
  の静的・動的設計との比較。公開コードのコピーはしていない。
- Library Checker: [Static Range Sum](https://judge.yosupo.jp/problem/static_range_sum)、
  [Static RMQ](https://judge.yosupo.jp/problem/staticrmq)。
- `tests/random/sqrt-tree.cpp`: 固定 seed、空・1/2 要素・2 冪前後の境界、
  非可換 affine 合成と愚直積の比較、コピー・ムーブ先、演算回数上限を確認。
- [DST との同一環境測定]({{ '/docs/development/sqrt-tree-comparison.html' | relative_url }})。
