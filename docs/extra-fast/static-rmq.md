---
title: SIMD Static Range Min
documentation_of: //blueberry/extra-fast/static-rmq.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::StaticRangeMin<T>` は、変更しない32bit整数列の区間最小値を求めます。
Tは `std::int32_t` または `std::uint32_t`。16要素ごとのprefix/suffix最小値と、
ブロック最小値のSparse Tableを持ちます。AVX2対応CPUでは短区間と表の構築にSIMDを使います。
最小値だけを返し、最小位置や同値要素の選択位置は返しません。

要素数をN、ブロック数をM = ceil(N/16)、K = N + M log₂(M+1) + 1とします。
構築・メモリはO(K)、問い合わせは最悪O(1)です。同一ブロックでは最大16要素を調べます。
既存の比較器付き・左端argmin対応のLinearRMQ、任意演算のSparseTableを置き換えるものではありません。

## 操作一覧

以下では `RMQ = blueberry::extra_fast::StaticRangeMin<T>` とします。

| 操作 | 計算量 |
| --- | --- |
| [`RMQ()`](#construct-empty) | O(1) |
| [`RMQ(values, use_simd = true)`](#construct-values) | O(K) |
| [`rmq.size() → int`](#size) | O(1) |
| [`rmq.get(p) → T`](#get) | O(1) |
| [`rmq.prod(l, r) → T`](#prod) | O(1) |
| [`rmq.all_prod() → T`](#all-prod) | O(1) |
| [`rmq.uses_simd() → bool`](#uses-simd) | O(1) |
| [`RMQ::simd_available() → bool`](#simd-available) | O(1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`StaticRangeMin<T = std::int32_t>`](#type) | 型 |
| [`RMQ(other)`](#copy-construct) | O(K) |
| [`RMQ(std::move(other))`](#move-construct) | O(1) |
| [`rmq = other → RMQ&`](#copy-assign) | O(K + Kold) |
| [`rmq = std::move(other) → RMQ&`](#move-assign) | O(Kold + 1) |
| [`~RMQ()`](#destruct) | O(K) |

Koldは代入前の対象オブジェクトの記憶量です。破棄の計算量はvectorの標準的な上界を記載しています。

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- C++20以降。`values.size() <= INT_MAX` が必要です。範囲外の添字はassertの対象で、assert無効時も呼び出してはいけません。
- 0-indexed、半開区間 `[l, r)`。空入力を許可し、空区間の答えは `std::numeric_limits<T>::max()` です。最大値自体も通常の入力値として扱います。
- signed/unsignedの順序を型に合わせて選びます。加減算をせず比較するため、`INT32_MIN`、`INT32_MAX`、`UINT32_MAX` も値のオーバーフローなしに扱えます。
- 入力を所有し、構築後は変更しません。返り値は値なので参照の寿命や無効化を意識する必要はありません。
- GCC/Clangのx86では実行時にAVX2対応を検出します。それ以外は通常のループに切り替えます。`-mavx2` や `-march=native` をプログラム全体へ指定した場合、他のコードまで非対応CPUで動くことは保証しません。
- `use_simd=false` または `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` 定義で明示的SIMD経路を止めます。コンパイラ自身の自動ベクトル化までは止めません。
- `BLUEBERRY_EXTRA_FAST_USE_ASM` 定義時はSIMDの最小値演算を手書きの `vpminsd` / `vpminud` に替えます。AT&T/Intelの両構文に対応します。未定義時はintrinsicsを使います。
- macroはincludeより前に指定し、同じプログラムの全翻訳単位で設定をそろえてください。非対応CPUではasm設定でも通常のループへ切り替えます。asmが常に速いという保証はありません。
- メモリは元の値・prefix・suffixに3N要素、上位表に各段の有効長だけを保持します。入力コピー中の一時領域は別です。moveした入力vectorに余剰capacityがある場合はそれも引き継ぎ、余剰をCとすると実際の保持領域はO(K + C)です。確保失敗は標準コンテナの例外として伝わります。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>
#include "blueberry/extra-fast/static-rmq.hpp"

int main() {
  using RMQ = blueberry::extra_fast::StaticRangeMin<std::int32_t>;
  RMQ rmq(std::vector<std::int32_t>{8, -3, 5, -3, 9});
  assert(rmq.size() == 5);
  assert(rmq.get(2) == 5);
  assert(rmq.prod(1, 4) == -3);
  assert(rmq.all_prod() == -3);
  assert(rmq.prod(3, 3) == std::numeric_limits<std::int32_t>::max());
  RMQ empty;
  assert(empty.size() == 0);
  assert(empty.all_prod() == std::numeric_limits<std::int32_t>::max());
}
```
{% endraw %}

<details class="api-operation" id="type" markdown="1">
<summary><code>template&lt;class T = std::int32_t&gt; class StaticRangeMin</code> — 実行時の追加コストO(1)</summary>

Tに応じてsigned/unsignedの比較命令をコンパイル時に選びます。関数ポインタや型消去は使いません。

{% raw %}
```cpp
using SignedRMQ = blueberry::extra_fast::StaticRangeMin<>;
using UnsignedRMQ = blueberry::extra_fast::StaticRangeMin<std::uint32_t>;
```
{% endraw %}

注意点: Tは `std::int32_t` / `std::uint32_t` のみです。それ以外はstatic_assertで拒否します。公開フィールドはありません。

</details>

<details class="api-operation" id="construct-empty" markdown="1">
<summary><code>StaticRangeMin()</code> — O(1)</summary>

要素数0のオブジェクトを作ります。

{% raw %}
```cpp
blueberry::extra_fast::StaticRangeMin<> rmq;
assert(rmq.size() == 0);
```
{% endraw %}

注意点: `prod(0, 0)` と `all_prod()` はTの最大値。SIMD使用状態はfalseで、`get()` は呼べません。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>explicit StaticRangeMin(std::vector&lt;T&gt; values, bool use_simd = true)</code> — O(K)</summary>

値を所有して前計算します。lvalueのvectorはコピーし、`std::move` したvectorは所有権を移します。
use_simdはSIMD使用の許可で、trueでもCPUが非対応なら通常のループを使います。

{% raw %}
```cpp
std::vector<std::int32_t> values{4, -1, 2};
blueberry::extra_fast::StaticRangeMin<> copied(values);
blueberry::extra_fast::StaticRangeMin<> owned(std::move(values), false);
```
{% endraw %}

注意点: 空入力にも対応します。lvalueのコピーO(N)も構築のO(K)に含みます。元配列の後の変更はオブジェクトへ影響しません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int rmq.size() const</code> — O(1)</summary>

要素数Nを返します。

{% raw %}
```cpp
int n = rmq.size();
```
{% endraw %}

注意点: 空入力・移動元は0。構築後の要素数は代入するまで変わりません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T rmq.get(int p) const</code> — O(1)</summary>

要素pの値を返します。

{% raw %}
```cpp
auto value = rmq.get(0);
```
{% endraw %}

注意点: `0 <= p < size()`。空オブジェクトには呼べません。値で返し、内部の参照を公開しません。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>T rmq.prod(int l, int r) const</code> — O(1)</summary>

`[l, r)` の最小値を返します。問い合わせはオンラインで任意の順番に呼べます。

{% raw %}
```cpp
auto minimum = rmq.prod(0, rmq.size());
assert(rmq.prod(0, 0) == std::numeric_limits<std::int32_t>::max());
```
{% endraw %}

注意点: `0 <= l <= r <= size()`。l=rならTの最大値。最小位置・同値の位置を求める操作ではありません。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>T rmq.all_prod() const</code> — O(1)</summary>

全体の最小値を返します。構築時の結果を保持しています。

{% raw %}
```cpp
assert(rmq.all_prod() == rmq.prod(0, rmq.size()));
```
{% endraw %}

注意点: 空入力・移動元はTの最大値です。

</details>

<details class="api-operation" id="uses-simd" markdown="1">
<summary><code>bool rmq.uses_simd() const</code> — O(1)</summary>

このオブジェクトで明示的SIMD経路の使用を許可しているか返します。

{% raw %}
```cpp
bool enabled = rmq.uses_simd();
```
{% endraw %}

注意点: trueでも全操作がSIMDを使うわけではありません。特に長区間クエリはprefix/suffixと表をscalarで参照します。既定構築・移動元はfalseです。

</details>

<details class="api-operation" id="simd-available" markdown="1">
<summary><code>static bool StaticRangeMin&lt;T&gt;::simd_available()</code> — O(1)</summary>

現在のビルドとCPUで明示的AVX2経路が使用可能か返します。

{% raw %}
```cpp
bool available = blueberry::extra_fast::StaticRangeMin<>::simd_available();
```
{% endraw %}

注意点: `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` 定義時はfalseです。CPU対応を調べるためにAVX2命令を実行することはありません。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>StaticRangeMin(const StaticRangeMin&amp; other)</code> — O(K)</summary>

配列・前計算・SIMD設定を独立したオブジェクトへコピーします。

{% raw %}
```cpp
auto copied = rmq;
```
{% endraw %}

注意点: コピー後にどちらかを代入・破棄しても他方へ影響しません。空入力もコピーできます。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>StaticRangeMin(StaticRangeMin&amp;&amp; other) noexcept</code> — O(1)</summary>

前計算済みの領域を移動します。移動元は既定構築と同じ空の状態になります。

{% raw %}
```cpp
auto moved = std::move(copied);
assert(copied.size() == 0);
```
{% endraw %}

注意点: 移動元も `size()`、空区間クエリ、代入、破棄が有効です。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>StaticRangeMin&amp; rmq.operator=(const StaticRangeMin&amp; other)</code> — O(K + Kold)</summary>

otherをコピーして元の内容を置き換え、自分への参照を返します。確保が失敗した場合は元の内容を保持します。

{% raw %}
```cpp
blueberry::extra_fast::StaticRangeMin<> assigned;
assigned = rmq;
```
{% endraw %}

注意点: 自己代入はO(1)です。代入中は新旧両方の前計算領域を保持します。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>StaticRangeMin&amp; rmq.operator=(StaticRangeMin&amp;&amp; other) noexcept</code> — O(Kold + 1)</summary>

otherの領域を引き継ぎ、対象の元の領域を解放し、自分への参照を返します。

{% raw %}
```cpp
assigned = std::move(moved);
assert(moved.size() == 0);
```
{% endraw %}

注意点: 異なるオブジェクトから移動した場合、移動元は空になります。自己移動代入はO(1)で元の内容を保ちます。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~StaticRangeMin()</code> — O(K)</summary>

所有する配列と前計算領域を解放します。

{% raw %}
```cpp
{
  blueberry::extra_fast::StaticRangeMin<> temporary;
} // 解放
```
{% endraw %}

注意点: 入力vectorをlvalueで渡した場合、その元配列は破棄しません。Tは整数なので要素ごとのユーザー定義破棄処理はありません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Library Checker staticrmq](https://judge.yosupo.jp/problem/staticrmq) を `verify/extra-fast/static-rmq.test.cpp` で検証します。
- [提出402567](https://judge.yosupo.jp/submission/402567)、[提出362370](https://judge.yosupo.jp/submission/362370) から、固定幅ブロック・prefix/suffix・ブロック上のSparse Table・SIMD minを調査しました。第三者ソースを転載せず独立実装しています。
- [既存のソース調査](../../docs/development/fastest-rmq-source-study.md) は方式と公開時間の違いを説明しています。公開時間を同条件の性能比較として扱いません。
- `tests/random/extra-fast-rmq.cpp` でsigned/unsigned全域、空区間、ブロック境界、同値・単調・乱数、コピー・移動を愚直解と比較します。seedはコマンドラインで固定でき、失敗時に入力と期待値を表示します。
- 同じ入力・CPU・コンパイラでscalar/intrinsics/asmを比較し、速度の採否は測定記録に基づきます。
- [今回の比較測定]({{ '/docs/development/measurements/extra-fast-expansion/README.html' | relative_url }})に、構築・問い合わせ・メモリの結果と採用判断を記録しています。

</details>
