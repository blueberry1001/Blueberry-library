---
title: 約数・倍数変換と GCD / LCM 畳み込み
documentation_of: //blueberry/math/divisor-convolution.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

添字の約数・倍数変換とgcd/lcm畳み込みです。Nはvectorの長さで、添字1以上を扱います。ゼータ変換には可換な加算モノイド、逆変換には加法可換群、畳み込みには環を要求します。

各操作は最悪O(N log log(N+2))時間、補助メモリO(N)です。畳み込みは入力コピーと結果を含めてもO(N)メモリです。空入力は空のままです。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`divisor_zeta`](#divisor_zeta) | O(N log log(N+2)) |
| [`divisor_mobius`](#divisor_mobius) | O(N log log(N+2)) |
| [`multiple_zeta`](#multiple_zeta) | O(N log log(N+2)) |
| [`multiple_mobius`](#multiple_mobius) | O(N log log(N+2)) |
| [`gcd_convolution`](#gcd_convolution) | O(N log log(N+2)) |
| [`lcm_convolution`](#lcm_convolution) | O(N log log(N+2)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLにはない添字の約数・倍数変換です。補助メモリは篩に使います。
- 添字0は変換では保持し、畳み込みでは無視して結果を `T{}` にします。
- `T{}` は零元、Tは必要な `+=`、`-=`、`*=` とコピーを持つ型です。Tの演算をO(1)として数えます。
- 整数では中間和・差・積も型の範囲内とします。modintの使用を推奨します。
- 変換は要素だけを書き換え、vectorの参照・iteratorを無効化しません。
- 畳み込みの引数は値渡しで、通常の呼び出しでは元vectorを変更せず、独立したvectorを返します。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/divisor-convolution.hpp"
int main() {
  std::vector<int> a{0, 1, 2};
  assert((blueberry::gcd_convolution(a, a) == std::vector<int>{0, 5, 4}));
  assert((blueberry::lcm_convolution(a, a) == std::vector<int>{0, 1, 8}));
}
```
{% endraw %}

<details class="library-notes" markdown="1">
<summary>操作例の前提</summary>

## 操作の補足 {#操作の補足}

- すべて `template<class T>` の関数。

</details>

<details class="api-operation" id="divisor_zeta" markdown="1">
<summary><code>void divisor_zeta(std::vector&lt;T&gt;&amp; a)</code> — O(N log log(N+2))</summary>

注意点:

a[i] を正の約数 d|i の a[d] の和へ置き換える。 空vectorも許可する。数値の範囲と代数の前提は概要に従う。

{% raw %}
```cpp
std::vector<int> a{0, 1, 2, 3};
blueberry::divisor_zeta(a);
assert((a == std::vector<int>{0, 1, 3, 4}));
```
{% endraw %}

</details>

<details class="api-operation" id="divisor_mobius" markdown="1">
<summary><code>void divisor_mobius(std::vector&lt;T&gt;&amp; a)</code> — O(N log log(N+2))</summary>

注意点:

約数ゼータ変換の逆変換。 空vectorも許可する。数値の範囲と代数の前提は概要に従う。

{% raw %}
```cpp
std::vector<int> a{0, 1, 3, 4};
blueberry::divisor_mobius(a);
assert((a == std::vector<int>{0, 1, 2, 3}));
```
{% endraw %}

</details>

<details class="api-operation" id="multiple_zeta" markdown="1">
<summary><code>void multiple_zeta(std::vector&lt;T&gt;&amp; a)</code> — O(N log log(N+2))</summary>

注意点:

a[i] を N 未満の正の倍数 j の a[j] の和へ置き換える。 空vectorも許可する。数値の範囲と代数の前提は概要に従う。

{% raw %}
```cpp
std::vector<int> a{0, 1, 2, 3};
blueberry::multiple_zeta(a);
assert((a == std::vector<int>{0, 6, 2, 3}));
```
{% endraw %}

</details>

<details class="api-operation" id="multiple_mobius" markdown="1">
<summary><code>void multiple_mobius(std::vector&lt;T&gt;&amp; a)</code> — O(N log log(N+2))</summary>

注意点:

倍数ゼータ変換の逆変換。 空vectorも許可する。数値の範囲と代数の前提は概要に従う。

{% raw %}
```cpp
std::vector<int> a{0, 6, 2, 3};
blueberry::multiple_mobius(a);
assert((a == std::vector<int>{0, 1, 2, 3}));
```
{% endraw %}

</details>

<details class="api-operation" id="gcd_convolution" markdown="1">
<summary><code>std::vector&lt;T&gt; gcd_convolution(std::vector&lt;T&gt; a, std::vector&lt;T&gt; b)</code> — O(N log log(N+2))</summary>

注意点:

返り値 c[k] = sum(a[i]*b[j] : gcd(i,j)=k, 1<=i,j<N)。同じ長さの入力が必要。 空vectorも許可する。数値の範囲と代数の前提は概要に従う。

{% raw %}
```cpp
std::vector<int> a{0, 1, 2};
auto c = blueberry::gcd_convolution(a, a);
assert((c == std::vector<int>{0, 5, 4}));
```
{% endraw %}

</details>

<details class="api-operation" id="lcm_convolution" markdown="1">
<summary><code>std::vector&lt;T&gt; lcm_convolution(std::vector&lt;T&gt; a, std::vector&lt;T&gt; b)</code> — O(N log log(N+2))</summary>

注意点:

返り値 c[k] = sum(a[i]*b[j] : lcm(i,j)=k, 1<=i,j<N)。k>=N の項は切り捨てる。同じ長さの入力が必要。 空vectorも許可する。数値の範囲と代数の前提は概要に従う。

{% raw %}
```cpp
std::vector<int> a{0, 1, 2};
auto c = blueberry::lcm_convolution(a, a);
assert((c == std::vector<int>{0, 1, 8}));
```
{% endraw %}

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [Library Checker GCD](https://judge.yosupo.jp/problem/gcd_convolution)、[LCM](https://judge.yosupo.jp/problem/lcm_convolution)。
  変換の逆転・直接和・畳み込みの愚直比較を `tests/random/math-batch-two.cpp` で確認する。
  調査した素数順変換は [Nyaan](https://nyaannyaan.github.io/library/multiplicative-function/divisor-multiple-transform.hpp.html)、[suisen](https://suisen-cp.github.io/cp-library-cpp/library/convolution/gcd_convolution.hpp.html)。
  コードの転記はせず、篩と更新方向を共通化して独立実装した。
  [候補測定]({{ '/docs/development/math-batch-two.html' | relative_url }})。

</details>
