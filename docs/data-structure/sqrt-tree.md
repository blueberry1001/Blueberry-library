---
title: Sqrt Tree（静的）
documentation_of: //blueberry/data-structure/sqrt-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

更新のない配列の区間モノイド積を最悪O(1)で返します。`SqrtTree<S, op, e>` のopは結合的、e()は左右単位元で、可換性・冪等性は不要です。左から右への順序を保ち、空配列・空区間も扱います。

Nは要素数、L=1+log log(N+2)。構築・メモリはO(N L)、prodのop呼出しは最大2回です。一点更新が必要なら [Dynamic Sqrt Tree]({{ '/blueberry/data-structure/dynamic-sqrt-tree.hpp.html' | relative_url }})、更新が多い場合はACL segtreeと比較してください。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`Tree tree`](#default-constructor) | O(1) |
| [`Tree`](#constructor) | O(N L) |
| [`tree.size`](#size) | O(1) |
| [`tree.get`](#get) | O(1) |
| [`tree.prod`](#prod) | O(1) |
| [`tree.all_prod`](#all-prod) | O(1) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACL segtreeの区間積O(log N)を前計算と追加メモリで高速化する、静的用途に特化した版です。Disjoint Sparse Tableの構築・メモリはO(N log N)ですが、実測の優劣は型・N・クエリ分布に依存し、常にDSTより速いとは限りません。
- `0 <= N <= INT_MAX`。Sはコピー可能であればよく、既定構築は不要です。op/e/コピーをO(1)とする計算量で、和・積のオーバーフローは呼び出し側で防いでください。
- 入力をコピーするため構築後に破棄して構いません。返り値はすべて値で、参照の寿命・無効化はありません。
- 各層にN要素のprefix/suffixと、必要な層だけに2冪へ切り上げた長さのブロック間積領域を持ちます。存在しない末尾要素や、両端ブロックだけで処理できるブロック間積は前計算しません。
- 静的・動的版の構築・メモリはいずれもO(N L)です。用途の違いは次のとおりです。

| 版 | 区間積 | 一点更新 | 用途 |
| --- | --- | --- | --- |
| `SqrtTree` | O(1)、`op` は最大2回 | 非対応 | 構築後に変更しない配列 |
| `DynamicSqrtTree` | O(1) | O(√N) | 更新が少なく区間積が多い配列 |

- ACL segtreeは更新・区間積ともO(log N)です。

## 操作の補足 {#操作の補足}

- 以下 `Tree = blueberry::SqrtTree<S, op, e>`。

</details>

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

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

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

</details>
