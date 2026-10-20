---
title: SIMD Affine Lazy Segment Tree
documentation_of: //blueberry/extra-fast/affine-sum-segment-tree.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::AffineSumSegmentTree`は998244353上の区間affine更新と区間和を行います。
動的な16分木とAVX2のMontgomery演算で、複数の子と遅延作用をまとめて更新します。
構築はO(N+1)、更新・区間和はO(log(N+1))、全体和はO(1)、メモリはO(N+1)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`AffineSumSegmentTree(n = 0, use_simd = true)`](#construct) | O(N+1) |
| [`AffineSumSegmentTree(values, use_simd = true)`](#construct-values) | O(N+1) |
| [`tree.size() → int`](#size) | O(1) |
| [`tree.apply(l, r, a, b)`](#apply) | O(log(N+1)) |
| [`tree.set(p, x)`](#set) | O(log(N+1)) |
| [`tree.get(p) → uint32_t`](#get) | O(log(N+1)) |
| [`tree.prod(l, r) → uint32_t`](#prod) | O(log(N+1)) |
| [`tree.all_prod() → uint32_t`](#all-prod) | O(1) |
| [`tree.uses_simd() → bool`](#uses-simd) | O(1) |
| [`AffineSumSegmentTree::simd_available() → bool`](#simd-available) | O(1) |
| [`AffineSumSegmentTree(other)`](#copy-construct) | O(N+1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`tree = other`](#copy-assign) | O(N+V+1) |
| [`AffineSumSegmentTree(std::move(other))`](#move-construct) | O(1) |
| [`tree = std::move(other)`](#move-assign) | O(V+1) |
| [`tree.~AffineSumSegmentTree()`](#destroy) | O(N+1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- C++20、modulusは998244353固定です。公開型は`blueberry::extra_fast::AffineSumSegmentTree`で、公開フィールドはありません。
- Nは要素数、Vは代入前の代入先の要素数です。`0 <= N <= INT_MAX`で必要な領域を確保できることが必要です。計算量はすべて最悪計算量です。
- 添字は0-indexed、区間は半開区間です。不正な添字はassertの対象で、NDEBUG時は検査しません。
- 値・係数はuint32_t全域を受け取れます。内部の和は通常の剰余、遅延作用はcanonical Montgomery residueで、戻り値は[0,998244353)です。uint64_tの中間積はオーバーフローしない範囲に収まります。
- 16分木で、階層幅の除算はshift/maskを使い、各階層のslotを連続して保持します。leafにはlazyを置きません。保持量は概ね18N/15個のuint32_tと各段の端数で、構築時にもO(N+1)です。
- 符号付きの負数は引数変換でuint32_tへ折り返します。負の係数には998244352など、正規化した剰余を渡してください。
- 3配列は標準C++20のaligned allocationで32byte整列します。SIMDの各8lane全体が確保領域内にあり、入力配列側の整列やpaddingは不要です。
- 全queryを先読みする必要はなく、最大要素数・木の高さは実行時に決まります。更新は確保済み領域内で完了し、追加allocationを行いません。
- 値はオブジェクトが所有し、コピーは独立しています。参照を返す操作はありません。確保失敗では標準例外を送出します。移動元は空列となり、空列としての操作と再代入が可能です。
- queryはconstで、内部の遅延作用をquery結果に適用します。更新とqueryが同時に走る場合の同期は利用者が行います。
- GCC/Clangのx86ではAVX2を実行時検出し、対応kernelだけtarget("avx2")でコンパイルします。全体の-mavx2は不要です。それ以外やuse_simd=falseでは通常ループを使います。
- include前の`BLUEBERRY_EXTRA_FAST_USE_ASM`でMontgomery演算の偶数lane積にGNU inline asmのvpmuludqを使います。既定はintrinsicsで、Intel/AT&T両dialectを扱います。asm自体による改善は保証しません。
- `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`は明示AVX2経路を無効にします。コンパイラの自動vectorizeは禁じません。設定マクロは全翻訳単位で統一してください。
- 個別includeする実験版で、通常のACLや`blueberry/all.hpp`は変更しません。任意のmonoid/作用やmodulusにはACLのlazy_segtreeを使ってください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/affine-sum-segment-tree.hpp"
int main() {
  blueberry::extra_fast::AffineSumSegmentTree tree(std::vector<std::uint32_t>{1, 2, 3});
  tree.apply(0, 2, 2, 3);
  assert(tree.all_prod() == 15);
  tree.apply(1, 3, 0, 4);
  assert(tree.get(1) == 4);
  const auto& read_only = tree;
  assert(read_only.prod(0, 3) == 13);
  tree.set(0, 9);
  assert(tree.all_prod() == 17);
  assert(tree.prod(1, 1) == 0);
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>AffineSumSegmentTree(n = 0, use_simd = true)</code> — O(N+1)</summary>

長さnの0列を作ります。`0 <= n <= INT_MAX`で、空列も有効です。

{% raw %}
```cpp
blueberry::extra_fast::AffineSumSegmentTree tree(8);
```
{% endraw %}

注意点: use_simd=falseなら通常ループを選びます。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>AffineSumSegmentTree(values, use_simd = true)</code> — O(N+1)</summary>

`const std::vector<std::uint32_t>& values`をコピーして構築します。各値は998244353で剰余を取ります。

{% raw %}
```cpp
blueberry::extra_fast::AffineSumSegmentTree tree(std::vector<std::uint32_t>{1, 2, 3});
```
{% endraw %}

注意点: 入力の寿命に依存せず、空vectorも有効です。

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
<summary><code>tree.apply(l, r, a, b)</code> — O(log(N+1))</summary>

`0 <= l <= r <= N`について各要素xをa*x+bへ更新します。a,bはuint32_tで受け取り、998244353で剰余を取ります。空区間では何もしません。

{% raw %}
```cpp
tree.apply(0, 3, 2, 3);
tree.apply(0, 3, 5, 7);  // 元のxは10*x+22になる。
```
{% endraw %}

注意点: 新しい作用は、それまでの更新の後に適用します。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>tree.set(p, x)</code> — O(log(N+1))</summary>

`0 <= p < N`の要素をx mod 998244353に置き換えます。

{% raw %}
```cpp
tree.set(1, 10);
```
{% endraw %}

注意点: xはuint32_tです。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>tree.get(p) → uint32_t</code> — O(log(N+1))</summary>

`0 <= p < N`の現在の値を[0,998244353)で返します。

{% raw %}
```cpp
auto x = tree.get(1);
```
{% endraw %}

注意点: constから呼び出せます。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>tree.prod(l, r) → uint32_t</code> — O(log(N+1))</summary>

`0 <= l <= r <= N`について[l,r)の総和 mod 998244353を返します。空区間では0です。

{% raw %}
```cpp
const auto& read_only = tree;
auto x = read_only.prod(0, 2);
```
{% endraw %}

注意点: const操作で、遅延作用を子へ書き戻しません。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>tree.all_prod() → uint32_t</code> — O(1)</summary>

全体和 mod 998244353を返します。空列では0です。

{% raw %}
```cpp
auto x = tree.all_prod();
```
{% endraw %}

注意点: constから呼び出せます。

</details>

<details class="api-operation" id="uses-simd" markdown="1">
<summary><code>tree.uses_simd() → bool</code> — O(1)</summary>

このオブジェクトが明示AVX2 kernelを使うか返します。

{% raw %}
```cpp
bool enabled = tree.uses_simd();
```
{% endraw %}

注意点: CPU対応でもuse_simd=falseならfalseです。

</details>

<details class="api-operation" id="simd-available" markdown="1">
<summary><code>AffineSumSegmentTree::simd_available() → bool</code> — O(1)</summary>

実行CPUとコンパイラ設定で明示AVX2 kernelを利用できるか返します。

{% raw %}
```cpp
bool supported = blueberry::extra_fast::AffineSumSegmentTree::simd_available();
```
{% endraw %}

注意点: use_simdの指定とは独立です。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>AffineSumSegmentTree(other)</code> — O(N+1)</summary>

独立した3配列をコピーします。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: コピー後の更新は相手に影響しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>tree = other</code> — O(N+V+1)</summary>

独立したコピーを代入します。自己代入も有効です。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 確保失敗時は代入先を保持します。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>AffineSumSegmentTree(std::move(other))</code> — O(1)</summary>

確保領域とSIMD設定を引き継ぎます。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元は空列となり、size()、all_prod()、prod(0,0)、apply(0,0,a,b)も有効です。

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
<summary><code>tree.~AffineSumSegmentTree()</code> — O(N+1)</summary>

所有領域を解放します。

{% raw %}
```cpp
{ blueberry::extra_fast::AffineSumSegmentTree temporary(4); }
```
{% endraw %}

注意点: 通常はスコープ末尾で自動的に呼び出されます。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Library Checker Range Affine Range Sum](https://judge.yosupo.jp/problem/range_affine_range_sum)、境界と固定seedのランダム愚直比較。
- [上位提出402323](https://judge.yosupo.jp/submission/402323)と[408770](https://judge.yosupo.jp/submission/408770)からwideなSoA配置とSIMD Montgomeryの考え方を調査しました。ライセンス不明のソースはコピーせず、動的構造として独立に実装しています。
- [上位調査・比較測定・採否]({{ '/docs/development/measurements/extra-fast-more/lazy-bit/README.html' | relative_url }})。

</details>
