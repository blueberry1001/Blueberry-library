---
title: Bitwise Convolution
documentation_of: //blueberry/math/bitwise-convolution.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

添字のAND・OR・XORを組合せ演算とする畳み込みです。入力は同じ長さNで、N=0またはN=2^Kです。Tは可換環を要求し、AND/ORは整数型にも対応します。XORはさらにT(N)が可逆である必要があるため、N>1の通常の整数型や標数2では使えません。

最悪時間O(N(1+K))、入力コピーを含む追加メモリO(N+1)です。空入力同士は空、N=1では要素同士の積を返します。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`bitwise_convolution_and`](#and) | O(N(1+K)) |
| [`bitwise_convolution_or`](#or) | O(N(1+K)) |
| [`bitwise_convolution_xor`](#xor) | O(N(1+K)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 通常の多項式畳み込みを扱うACL `convolution` とは別の演算です。係数にはACL modintを利用できます。法が奇数ならXORの可逆性条件を満たします。
- 不足分の自動ゼロ埋めはしません。
- Tはコピー・`T{}`（ゼロ）・`+`・`-`・`+=`・`-=`・`*=` を使える型です。XORはさらに `T(1)`、`T(N)` と除算を使います。N>1の整数型では `1/N` が切り捨てられます。
- 係数演算・コピーをO(1)とします。全中間和・差・積が型の範囲に収まることは呼出側の責任です。浮動小数点は丸め誤差を含みます。
- 入力は値渡しで、通常呼出しではコピー、`std::move` を渡せば格納領域を利用できます。返り値は独立したvectorで、内部参照を公開しません。移動した入力の内容は未規定です。
- 公開変換APIはなく、用途ごとの3関数を提供します。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/bitwise-convolution.hpp"
int main() {
  using Mint = atcoder::modint998244353;
  std::vector<Mint> a{1, 2}, b{3, 4};
  auto c = blueberry::bitwise_convolution_and(a, b);
  assert(c[0] == 13 && c[1] == 8);
  c = blueberry::bitwise_convolution_or(a, b);
  assert(c[0] == 3 && c[1] == 18);
  c = blueberry::bitwise_convolution_xor(a, b);
  assert(c[0] == 11 && c[1] == 10);
}
```
{% endraw %}

<details class="api-operation" id="and" markdown="1">
<summary><code>vector&lt;T&gt; bitwise_convolution_and(vector&lt;T&gt; a, vector&lt;T&gt; b)</code> — O(N(1+K))</summary>

`c[s] = Σ a[i] b[j] (i & j = s)` を返します。上位集合 zeta 変換、各点の積、Möbius 逆変換を行います。

{% raw %}
```cpp
auto c = blueberry::bitwise_convolution_and(a, b);
```
{% endraw %}

注意点: a,b は同長の2冪長、または両空。係数の除算はありません。
</details>

<details class="api-operation" id="or" markdown="1">
<summary><code>vector&lt;T&gt; bitwise_convolution_or(vector&lt;T&gt; a, vector&lt;T&gt; b)</code> — O(N(1+K))</summary>

`c[s] = Σ a[i] b[j] (i | j = s)` を返します。下位集合 zeta 変換を用います。

{% raw %}
```cpp
auto c = blueberry::bitwise_convolution_or(a, b);
```
{% endraw %}

注意点: OR と排他的な部分集合畳み込みは異なります。i,j が重なっていても和に含みます。
</details>

<details class="api-operation" id="xor" markdown="1">
<summary><code>vector&lt;T&gt; bitwise_convolution_xor(vector&lt;T&gt; a, vector&lt;T&gt; b)</code> — O(N(1+K))</summary>

`c[s] = Σ a[i] b[j] (i ^ j = s)` を返します。Walsh–Hadamard 変換後に N の逆元を掛けます。

{% raw %}
```cpp
auto c = blueberry::bitwise_convolution_xor(a, b);
```
{% endraw %}

注意点: T(N) の逆元が必須。通常の整数除算や N>1 の標数2には対応しません。空入力は除算を実行せず空を返します。
</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [Library Checker AND](https://judge.yosupo.jp/problem/bitwise_and_convolution)、[XOR](https://judge.yosupo.jp/problem/bitwise_xor_convolution): 対応する verify/math/bitwise-*-convolution.test.cpp。
- OR の公式単独問題はありません。`verify/math/bitwise-or-via-and.test.cpp` は bit 補集合による De Morgan 則で公式 AND 入力を OR に還元します。OR の直接定義は O(N²) oracle でも検証します。
- [Fastest AND 400554](https://judge.yosupo.jp/submission/400554)、[XOR 400553](https://judge.yosupo.jp/submission/400553): 同系統の変換に SIMD・Montgomery 演算・専用 I/O を組合せています。公開コードを転記せず、標準的な変換を独立実装しています。
- `tests/random/algebra-expansion.cpp`: 固定 seed の全 i,j 列挙、空・長さ1・符号付き係数・異なる法を検証。

</details>
