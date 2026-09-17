---
title: 約数・倍数変換と GCD / LCM 畳み込み
documentation_of: //blueberry/math/divisor-convolution.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

ACLにはない添字の約数・倍数変換。N はvectorの長さ、添字1以上を扱う。
添字0は変換では保持し、畳み込みでは無視して結果を T{} にする。空入力は空のまま。
ゼータ変換には可換な加算モノイド、逆変換には加法可換群、畳み込みには環を要求する。
T{} は零元、T は必要な +=, -=, *= とコピーを持つこと。
整数では中間和・差・積も型の範囲内とする。modint の使用を推奨する。
各操作は最悪 O(N log log(N+2)) 時間、補助メモリ O(N)（篩）。
畳み込みは入力コピーと結果を含めても O(N) メモリ。T の演算を O(1) として数える。
変換は要素だけを書き換え、vector の参照・iteratorを無効化しない。
畳み込みの引数は値渡しで、通常の呼び出しでは元vectorを変更せず、独立したvectorを返す。

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

## 操作一覧

すべて `template<class T>` の関数。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `void divisor_zeta(std::vector<T>& a)` | O(N log log(N+2)) | [開く](#divisor_zeta) |
| `void divisor_mobius(std::vector<T>& a)` | O(N log log(N+2)) | [開く](#divisor_mobius) |
| `void multiple_zeta(std::vector<T>& a)` | O(N log log(N+2)) | [開く](#multiple_zeta) |
| `void multiple_mobius(std::vector<T>& a)` | O(N log log(N+2)) | [開く](#multiple_mobius) |
| `std::vector<T> gcd_convolution(std::vector<T> a, std::vector<T> b)` | O(N log log(N+2)) | [開く](#gcd_convolution) |
| `std::vector<T> lcm_convolution(std::vector<T> a, std::vector<T> b)` | O(N log log(N+2)) | [開く](#lcm_convolution) |

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

## 出典・検証

[Library Checker GCD](https://judge.yosupo.jp/problem/gcd_convolution)、[LCM](https://judge.yosupo.jp/problem/lcm_convolution)。
変換の逆転・直接和・畳み込みの愚直比較を `tests/random/math-batch-two.cpp` で確認する。
調査した素数順変換は [Nyaan](https://nyaannyaan.github.io/library/multiplicative-function/divisor-multiple-transform.hpp.html)、[suisen](https://suisen-cp.github.io/cp-library-cpp/library/convolution/gcd_convolution.hpp.html)。
コードの転記はせず、篩と更新方向を共通化して独立実装した。
[候補測定]({{ '/docs/development/math-batch-two.html' | relative_url }})。
