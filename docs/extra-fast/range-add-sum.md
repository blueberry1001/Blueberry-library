---
title: SIMD Range Add Sum
documentation_of: //blueberry/extra-fast/range-add-sum.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::RangeAddSum`はuint64_tの区間加算・区間和を行います。
差分と添字を掛けた差分を2本の16分木で管理し、遅延伝播を省きます。
構築はO(N+1)、更新・区間和はO(log(N+1))、全体和はO(1)、メモリはO(N+1)です。
加算だけを使うときの専用構造で、一般の作用やaffine更新にはACLのlazy_segtreeを使います。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`RangeAddSum(n = 0, use_simd = true)`](#construct) | O(N+1) |
| [`RangeAddSum(values, use_simd = true)`](#construct-values) | O(N+1) |
| [`tree.size() → int`](#size) | O(1) |
| [`tree.apply(l, r, x)`](#apply) | O(log(N+1)) |
| [`tree.set(p, x)`](#set) | O(log(N+1)) |
| [`tree.get(p) → uint64_t`](#get) | O(log(N+1)) |
| [`tree.pref(r) → uint64_t`](#pref) | O(log(N+1)) |
| [`tree.prod(l, r) → uint64_t`](#prod) | O(log(N+1)) |
| [`tree.all_prod() → uint64_t`](#all-prod) | O(1) |
| [`tree.uses_simd() → bool`](#uses-simd) | O(1) |
| [`RangeAddSum::simd_available() → bool`](#simd-available) | O(1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`RangeAddSum(other)`](#copy-construct) | O(N+1) |
| [`tree = other`](#copy-assign) | O(N+V+1) |
| [`RangeAddSum(std::move(other))`](#move-construct) | O(1) |
| [`tree = std::move(other)`](#move-assign) | O(V+1) |
| [`tree.~RangeAddSum()`](#destroy) | O(N+1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 値は`std::uint64_t`で、加減算・乗算はすべてmod 2^64です。符号付きオーバーフローは使いません。
- 添字は0-indexed、区間は半開区間です。不正な添字はassertの対象で、NDEBUG時は検査しません。
- 各オブジェクトが値を所有し、コピーは独立しています。公開フィールドや要素への参照はありません。
- 移動元には破棄と再代入だけを行ってください。構築・コピーの確保失敗では標準例外を送出します。更新時の追加allocationはありません。
- Nは要素数、Vは代入前の代入先の要素数です。計算量はすべて最悪計算量です。縮小代入後の未使用capacityをC語とすると保持領域はO(N+C+1)になる場合があります。
- 通常版やACLを変更せず、個別includeする実験版です。`blueberry/all.hpp`には含まれません。
- `0 <= N <= INT_MAX`で領域を確保できることが必要です。構築中には差分用の一時vectorを使います。
- 第i要素の差分をd[i]とすると、prefixは`r * sum(d[0:r]) - sum(i*d[i], i<r)`です。
- 内部のSumSegmentTreeを2本保持します。約`2 * 16/15 * N`語と各階層の端数です。小さいNでも各木は16語以上を確保します。
- GCC/Clangのx86でAVX2を実行時検出します。非対応環境やuse_simd=falseでは通常ループを使います。全体の-mavx2は不要です。
- `BLUEBERRY_EXTRA_FAST_USE_ASM`と`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`はSumSegmentTreeへ引き継ぎます。前者はvpaddqのGNU inline asm、後者は明示AVX2経路の無効化です。設定は全翻訳単位で統一してください。
- このheader単体は約90行ですが、依存するSumSegmentTreeの実装も必要です。手書きasmそのものによる速度向上は保証しません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/range-add-sum.hpp"
int main() {
  blueberry::extra_fast::RangeAddSum tree(std::vector<std::uint64_t>{1, 2, 3});
  tree.apply(0, 2, 5);
  assert(tree.prod(0, 3) == 16);
  tree.set(1, 10);
  assert(tree.get(1) == 10);
  assert(tree.pref(2) == 16);
  assert(tree.all_prod() == 19);
  tree.apply(0, 3, std::uint64_t(0) - 1);
  assert(tree.prod(0, 3) == 16);
  assert(tree.prod(1, 1) == 0);
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>RangeAddSum(n = 0, use_simd = true)</code> — O(N+1)</summary>

長さnの0列を構築します。`0 <= n <= INT_MAX`で、空列も有効です。

{% raw %}
```cpp
blueberry::extra_fast::RangeAddSum tree(8);
```
{% endraw %}

注意点: use_simd=falseなら通常ループを選びます。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>RangeAddSum(values, use_simd = true)</code> — O(N+1)</summary>

`const std::vector<std::uint64_t>& values`をコピーして構築します。入力を保持する必要はありません。

{% raw %}
```cpp
blueberry::extra_fast::RangeAddSum tree(std::vector<std::uint64_t>{1, 2, 3});
```
{% endraw %}

注意点: 空vectorも有効です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>tree.size() → int</code> — O(1)</summary>

要素数Nを返します。

{% raw %}
```cpp
assert(tree.size() == 3);
```
{% endraw %}

注意点: 空列でも長さ0を返します。

</details>

<details class="api-operation" id="apply" markdown="1">
<summary><code>tree.apply(l, r, x)</code> — O(log(N+1))</summary>

`0 <= l <= r <= N`について[l,r)の各要素にuint64_tのxを加えます。空区間では何もしません。

{% raw %}
```cpp
tree.apply(0, 2, 5);
```
{% endraw %}

注意点: 減算は`std::uint64_t(0)-x`の加算で表せます。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>tree.set(p, x)</code> — O(log(N+1))</summary>

`0 <= p < N`の要素をuint64_tのxに置き換えます。

{% raw %}
```cpp
tree.set(1, 10);
```
{% endraw %}

注意点: 空列には呼べません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>tree.get(p) → uint64_t</code> — O(log(N+1))</summary>

`0 <= p < N`の現在の値を返します。

{% raw %}
```cpp
auto x = tree.get(1);
```
{% endraw %}

注意点: 空列には呼べません。

</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>tree.pref(r) → uint64_t</code> — O(log(N+1))</summary>

`0 <= r <= N`について[0,r)の総和を返します。

{% raw %}
```cpp
auto x = tree.pref(2);
```
{% endraw %}

注意点: r=0なら0です。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>tree.prod(l, r) → uint64_t</code> — O(log(N+1))</summary>

`0 <= l <= r <= N`について[l,r)の総和を返します。

{% raw %}
```cpp
auto x = tree.prod(0, 2);
```
{% endraw %}

注意点: 空区間では0です。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>tree.all_prod() → uint64_t</code> — O(1)</summary>

保持している全体和を返します。

{% raw %}
```cpp
auto x = tree.all_prod();
```
{% endraw %}

注意点: 空列では0です。

</details>

<details class="api-operation" id="uses-simd" markdown="1">
<summary><code>tree.uses_simd() → bool</code> — O(1)</summary>

このオブジェクトがAVX2 kernelを使うか返します。

{% raw %}
```cpp
bool enabled = tree.uses_simd();
```
{% endraw %}

注意点: CPU対応でもuse_simd=falseならfalseです。

</details>

<details class="api-operation" id="simd-available" markdown="1">
<summary><code>RangeAddSum::simd_available() → bool</code> — O(1)</summary>

実行CPUとコンパイラ設定でAVX2 kernelを利用できるか返します。

{% raw %}
```cpp
bool supported = blueberry::extra_fast::RangeAddSum::simd_available();
```
{% endraw %}

注意点: use_simdの指定とは独立です。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>RangeAddSum(other)</code> — O(N+1)</summary>

2本の木を独立にコピーします。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: コピー後の更新は相手に影響しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>tree = other</code> — O(N+V+1)</summary>

独立したコピーを代入します。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入も有効で、確保に失敗した場合は代入先を保持します。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>RangeAddSum(std::move(other))</code> — O(1)</summary>

確保領域を引き継ぎます。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元には破棄か再代入だけを行います。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>tree = std::move(other)</code> — O(V+1)</summary>

旧領域を解放して確保領域を引き継ぎます。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: 自己move代入も有効です。

</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>tree.~RangeAddSum()</code> — O(N+1)</summary>

所有領域を解放します。

{% raw %}
```cpp
{ blueberry::extra_fast::RangeAddSum temporary(4); }
```
{% endraw %}

注意点: 通常はスコープ末尾で自動的に呼び出されます。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [AOJ DSL_2_G: Range Add Query and Range Sum Query](https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_G)と、unsigned折り返しを含む固定seedランダム愚直比較。
- SIMD prefix木の由来は[SumSegmentTree]({{ '/blueberry/extra-fast/segment-tree.hpp.html' | relative_url }})を参照してください。差分2本による区間和の恒等式を独立に実装しました。
- [上位提出調査と比較測定]({{ '/docs/development/measurements/extra-fast-more/lazy-bit/README.html' | relative_url }})。

</details>
