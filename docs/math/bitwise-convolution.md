---
title: Bitwise Convolution
documentation_of: //blueberry/math/bitwise-convolution.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

添字の AND・OR・XOR を組合せ演算とする畳み込みを計算します。通常の多項式畳み込みを扱う ACL `convolution` とは別の演算です。係数には ACL modint を利用できます。

入力は同じ長さ N で、N=0 または N=2^K を要求します。不足分の自動ゼロ埋めはしません。空入力同士は空、N=1 では要素同士の積を返します。入力は値渡しで、通常呼出しではコピー、`std::move` を渡せば格納領域を利用できます。返り値は独立した vector で、内部参照を公開しません。移動した入力の内容は未規定です。

T は可換環として、コピー・`T{}`（ゼロ）・`+`・`-`・`+=`・`-=`・`*=` を使える型です。AND/OR は除算不要で、整数型も使えます。XOR はさらに `T(1)`、`T(N)` と除算を使い、**T(N) が可逆**でなければなりません。法が奇数の modint はこの条件を満たします。N>1 の通常の整数型では `1/N` が切り捨てられるため XOR API に使えません。標数2でも N>1 の XOR は使えません。

計算量は最悪 O(N(1+K))、入力のコピーを含む追加メモリ O(N+1)。係数演算・コピーを O(1) とします。全中間和・差・積が型の範囲に収まることは呼出側の責任です。浮動小数点は丸め誤差を含みます。公開変換APIはなく、用途ごとの3関数を提供します。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<T> bitwise_convolution_and(vector<T> a, vector<T> b)` | O(N(1+K)) | [開く](#and) |
| `vector<T> bitwise_convolution_or(vector<T> a, vector<T> b)` | O(N(1+K)) | [開く](#or) |
| `vector<T> bitwise_convolution_xor(vector<T> a, vector<T> b)` | O(N(1+K)) | [開く](#xor) |

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

## 出典・検証

- [Library Checker AND](https://judge.yosupo.jp/problem/bitwise_and_convolution)、[XOR](https://judge.yosupo.jp/problem/bitwise_xor_convolution): 対応する verify/math/bitwise-*-convolution.test.cpp。
- OR の公式単独問題はありません。`verify/math/bitwise-or-via-and.test.cpp` は bit 補集合による De Morgan 則で公式 AND 入力を OR に還元します。OR の直接定義は O(N²) oracle でも検証します。
- [Fastest AND 400554](https://judge.yosupo.jp/submission/400554)、[XOR 400553](https://judge.yosupo.jp/submission/400553): 同系統の変換に SIMD・Montgomery 演算・専用 I/O を組合せています。公開コードを転記せず、標準的な変換を独立実装しています。
- `tests/random/algebra-expansion.cpp`: 固定 seed の全 i,j 列挙、空・長さ1・符号付き係数・異なる法を検証。
