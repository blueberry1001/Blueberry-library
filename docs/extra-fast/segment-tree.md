---
title: SIMD Sum Segment Tree
documentation_of: //blueberry/extra-fast/segment-tree.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::SumSegmentTree` は、`uint64_t` の点加算・点代入と区間和に特化した16分木です。
AVX2対応のx86 CPUでは、部分和の更新をSIMD命令でまとめて実行します。
既存のSegment TreeやACLを使い続けながら、用途が合う場合だけ個別に読み込めます。

要素数をN、木の段数をH = max(1, ceil(log₁₆(max(1, N)))) とすると、構築O(N + 1)、
加算・代入O(16H)、区間和O(H)、メモリO(N + 1)です。16は固定なので更新もO(log(N + 1))です。
値・演算が固定される代わりに、二分木より木の段数と区間和の読み取り回数を減らします。
AVX2が使えない環境でも同じAPIで動作します。速度はCPU・入力分布・コンパイラに依存します。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`SumSegmentTree(n, use_simd = true)`](#construct-size) | O(N + 1) |
| [`SumSegmentTree(a, use_simd = true)`](#construct-values) | O(N + 1) |
| [`tree.add(p, x)`](#add) | O(16H) |
| [`tree.set(p, x)`](#set) | O(16H) |
| [`tree.get(p) → uint64_t`](#get) | O(1) |
| [`tree.pref(r) → uint64_t`](#pref) | O(H) |
| [`tree.prod(l, r) → uint64_t`](#prod) | O(H) |
| [`tree.all_prod() → uint64_t`](#all-prod) | O(1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`tree.size() → int`](#size) | O(1) |
| [`tree.uses_simd() → bool`](#uses-simd) | O(1) |
| [`SumSegmentTree::simd_available() → bool`](#simd-available) | O(1) |
| [`SumSegmentTree(other)`](#copy-construct) | O(N + 1) |
| [`tree = other`](#copy-assign) | O(N + M + 1) |
| [`SumSegmentTree(std::move(other))`](#move-construct) | O(1) |
| [`tree = std::move(other)`](#move-assign) | O(M + 1) |
| [`tree.~SumSegmentTree()`](#destroy) | O(N + 1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- C++20。公開型は `blueberry::extra_fast::SumSegmentTree`。公開フィールドはありません。
  `blueberry/all.hpp` には含まれません。
- 添字は `int`、0-indexed、区間は半開区間です。`0 <= N <= INT_MAX` かつメモリに収まることが必要です。
  不正な添字はassertの対象で、`NDEBUG`時には検査しません。
- 演算は `uint64_t` の加減算、単位元は0です。**2⁶⁴を法とする和**を返し、あふれはunsignedの規則で折り返します。
  通常の非負整数の和が必要なら、答えが `UINT64_MAX` 以下に収まることを確認してください。
  min、最大値、関数合成、任意モノイド、遅延伝播、境界探索には対応しません。
- 内部の各16分木ノードに子の累積和を持ちます。第i段の保存セル数は
  `16 * ceil(N / 16^(i+1))` で、根まで合計します。空の場合も16セル（128 bytes）を確保します。
  このほかvector、最大8段のoffset、AVX2版では共有2 KiBのマスク表を使います。
  小さいNでは通常の二分木より余白が多くなります。
- GCC / Clangのx86では実行CPUのAVX2対応を検出し、対応関数だけを `target("avx2")` でコンパイルします。
  全体への `-mavx2` や `-march=native` は不要です。それ以外の環境は通常のループで更新します。
  実行中にCPUの機能が変化しないことを前提とします。
- 既定のSIMD加算はintrinsicで記述しています。`BLUEBERRY_EXTRA_FAST_USE_ASM` をinclude前に定義すると、
  同じ加算をGNU inline asmの `vpaddq` で記述した実験版に切り替えます。`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` はSIMDの検出を無効にします。
  マクロはプログラム内の全翻訳単位で統一してください。手書きasm自体がintrinsicより速いとは保証しません。
- 更新は確保済み領域を書き換えるだけで、追加のallocationはありません。値は木が所有し、`get`などは値で返します。
  返り値の参照寿命・無効化を気にせず使えます。異なるインスタンスの状態は独立しています。
- 計算量は最悪時間です。Mは代入前の代入先の要素数です。copy/move後も値とSIMD選択を引き継ぎます。
  move元には破棄と再代入だけを行ってください。allocationが失敗すると標準コンテナと同様に例外が送出されます。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/segment-tree.hpp"
int main() {
  blueberry::extra_fast::SumSegmentTree tree(std::vector<std::uint64_t>{1, 2, 3});
  assert(tree.prod(0, 3) == 6);
  tree.add(1, 5);
  tree.set(2, 10);
  assert(tree.get(1) == 7);
  assert(tree.pref(2) == 8);
  assert(tree.all_prod() == 18);
  assert(tree.prod(2, 2) == 0);
  blueberry::extra_fast::SumSegmentTree empty(0);
  assert(empty.all_prod() == 0);
}
```
{% endraw %}

<details class="api-operation" id="construct-size" markdown="1">
<summary><code>SumSegmentTree(int n, bool use_simd = true)</code> — O(N + 1)</summary>

長さn、全要素0の木を構築します。`use_simd=false` なら、対応CPUでも通常のループを選びます。

{% raw %}
```cpp
blueberry::extra_fast::SumSegmentTree tree(100);
blueberry::extra_fast::SumSegmentTree scalar(100, false);
```
{% endraw %}

注意点: nは非負。n=0も構築できます。`use_simd=true` でも非対応CPUでは通常のループを選びます。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>SumSegmentTree(const std::vector&lt;std::uint64_t&gt;&amp; a, bool use_simd = true)</code> — O(N + 1)</summary>

配列aをコピーして構築します。元配列を変更・破棄しても木は影響を受けません。

{% raw %}
```cpp
std::vector<std::uint64_t> a{10, 20};
blueberry::extra_fast::SumSegmentTree tree(a);
```
{% endraw %}

注意点: `a.size() <= INT_MAX` が必要です。空配列にも対応します。初期値の総和も2⁶⁴で折り返します。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void tree.add(int p, std::uint64_t x)</code> — O(16H)</summary>

要素pにxを加えます。SIMD版は各段で4本の256-bitベクトルを更新します。

{% raw %}
```cpp
tree.add(0, 7);
tree.add(0, std::uint64_t(0) - 7); // mod 2^64で7を引く
```
{% endraw %}

注意点: `0 <= p < size()`。空の木には呼べません。値と累積和は2⁶⁴で折り返します。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void tree.set(int p, std::uint64_t x)</code> — O(16H)</summary>

要素pをxに置き換えます。現在値との差を内部で加算するため、値が小さくなる代入にも対応します。

{% raw %}
```cpp
tree.set(0, 3);
```
{% endraw %}

注意点: `0 <= p < size()`。差分のunsigned演算による折り返しは意図した動作です。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>std::uint64_t tree.get(int p) const</code> — O(1)</summary>

要素pの現在値を返します。

{% raw %}
```cpp
std::uint64_t x = tree.get(0);
```
{% endraw %}

注意点: `0 <= p < size()`。値で返すため、後の更新による無効化はありません。

</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>std::uint64_t tree.pref(int r) const</code> — O(H)</summary>

`[0, r)` の和を返します。

{% raw %}
```cpp
assert(tree.pref(0) == 0);
auto total = tree.pref(tree.size());
```
{% endraw %}

注意点: `0 <= r <= size()`。空区間は0。答えは2⁶⁴を法とする和です。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>std::uint64_t tree.prod(int l, int r) const</code> — O(H)</summary>

`[l, r)` の和を返します。左右の経路が合流した後の計算を省きます。

{% raw %}
```cpp
assert(tree.prod(0, 0) == 0);
auto sum = tree.prod(0, tree.size());
```
{% endraw %}

注意点: `0 <= l <= r <= size()`。l=rなら0。空の木では `(0, 0)` が有効です。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>std::uint64_t tree.all_prod() const</code> — O(1)</summary>

全要素の和を返します。

{% raw %}
```cpp
assert(tree.all_prod() == tree.pref(tree.size()));
```
{% endraw %}

注意点: 空なら0。答えは2⁶⁴を法とする和です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

要素数Nを返します。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 空なら0。内部のpaddingは含めません。

</details>

<details class="api-operation" id="uses-simd" markdown="1">
<summary><code>bool tree.uses_simd() const</code> — O(1)</summary>

この木が更新にAVX2を使用するかを返します。

{% raw %}
```cpp
blueberry::extra_fast::SumSegmentTree tree(1, false);
assert(!tree.uses_simd());
```
{% endraw %}

注意点: コンストラクタの指定と実行CPUの機能で決まります。クエリはどちらも同じスカラー処理です。

</details>

<details class="api-operation" id="simd-available" markdown="1">
<summary><code>static bool SumSegmentTree::simd_available()</code> — O(1)</summary>

このビルド・実行CPUでAVX2の更新を選択できるかを返します。

{% raw %}
```cpp
bool available = blueberry::extra_fast::SumSegmentTree::simd_available();
```
{% endraw %}

注意点: x86のGCC/Clang以外や強制スカラーマクロ指定時はfalseです。検出結果によらず構築できます。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>SumSegmentTree(const SumSegmentTree&amp; other)</code> — O(N + 1)</summary>

otherと同じ値・SIMD選択を持つ独立したコピーを作ります。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: 元とコピーの更新は互いに影響しません。記憶領域も複製します。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>SumSegmentTree&amp; tree.operator=(const SumSegmentTree&amp; other)</code> — O(N + M + 1)</summary>

otherの値とSIMD選択をコピーし、`*this` への参照を返します。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入も可能です。既存の代入先の値は失われます。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>SumSegmentTree(SumSegmentTree&amp;&amp; other)</code> — O(1)</summary>

otherの記憶領域を引き継ぎます。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: move元には破棄と再代入だけを行ってください。使用例では `<utility>` をincludeします。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>SumSegmentTree&amp; tree.operator=(SumSegmentTree&amp;&amp; other)</code> — O(M + 1)</summary>

既存領域を解放してotherの記憶領域を引き継ぎ、`*this` への参照を返します。

{% raw %}
```cpp
moved = std::move(tree);
```
{% endraw %}

注意点: move元には破棄と再代入だけを行います。Mは代入前の代入先の要素数です。

</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>tree.~SumSegmentTree()</code> — O(N + 1)</summary>

所有する配列を解放します。

{% raw %}
```cpp
{ blueberry::extra_fast::SumSegmentTree temporary(16); }
```
{% endraw %}

注意点: 通常はスコープ終了時に自動で呼ばれます。明示的に破棄した後に操作してはいけません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・測定</summary>

## 出典・検証 {#出典検証}

- Library Checker [Point Add Range Sum](https://judge.yosupo.jp/problem/point_add_range_sum) の
  [258491](https://judge.yosupo.jp/submission/258491)・[403014](https://judge.yosupo.jp/submission/403014)で、
  多分木・部分和・SIMDによる更新の設計を調査しました。公開コードを転載せず独立に実装しています。
- [最速提出の調査記録]({{ '/docs/development/extra-fast-research.html' | relative_url }})では、
  SIMDによる木の高速化と、手書きasmによるI/O・先読み・固定制約を区別しています。
- 公式verifyは `verify/extra-fast/point-add-range-sum.test.cpp`。
  境界値・16の冪の前後・unsignedの折り返し・copy/move・SIMD/通常版の一致をランダム比較します。
- [比較測定]({{ '/docs/development/measurements/extra-fast/README.html' | relative_url }})に、
  ACL segtree/Fenwick、互換SegmentTree、SIMD asm/intrinsic、通常ループの同条件の結果を保存します。
  最速提出の表示時間はローカルの測定値と直接比較しません。

</details>
