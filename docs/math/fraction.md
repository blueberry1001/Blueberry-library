---
title: Fraction
documentation_of: //blueberry/math/fraction.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

正確な比較と四則演算を行う `blueberry::Fraction`。ACL にない有理数の値型です。
常に既約で、分子は `int64_t` 全域、分母は `1..INT64_MAX`、ゼロは `0/1` です。
2個の整数だけを保持し、メモリ O(1)。分子・分母の直接書き換えはできません。

**入力と演算結果を約分・符号正規化した後に、この範囲に収まることが前提です。**
ゼロ除算、表現できない結果は assert で検出します。assert 無効時も許される入力にはなりません。
無限大、NaN、多倍長分数、浮動小数への変換は提供しません。
GNU/Clang の `__int128` を意図的に使用し、64bit の積と和を広い中間型で計算します。
有効な値同士の比較は常に安全で、四則演算の途中が int64 を超えても約分後に収まれば使えます。

B は正規化する整数の絶対値の最大（最低2）です。固定幅整数演算を O(1) として、
Euclid の互除法を使う構築・四則演算は O(log B)、比較は O(1)。文字列出力は桁数に比例します。
stream 出力を除く全演算は constexpr。コピー・移動は独立した値で、参照無効化はありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <limits>
#include "blueberry/math/fraction.hpp"
using blueberry::Fraction;

int main() {
  Fraction a(2, 3), b(3, 4);
  assert(a + b == Fraction(17, 12));
  assert(a * b == Fraction(1, 2));
  assert(a < b);
  a /= b;
  assert(a.num() == 8 && a.den() == 9);
  constexpr Fraction normalized(6, -8);
  static_assert(normalized == Fraction(-3, 4));
  const auto m = std::numeric_limits<std::int64_t>::min();
  assert(Fraction(m, m) == 1);
  assert(Fraction(m, 1) - Fraction(m, 1) == 0);
}
```
{% endraw %}

## 操作一覧

以下の断片では `using blueberry::Fraction;` を前提とします。
整数は `Fraction(n)` に変換されるため、`a + 2`、`2 * a`、`a == 0` も使えます。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Fraction a(int64_t n = 0, int64_t d = 1)` | O(log B) | [開く](#construct) |
| `Fraction a(const Fraction& b)` | O(1) | [開く](#copy) |
| `Fraction a(Fraction&& b)` | O(1) | [開く](#move) |
| `Fraction& a.operator=(const Fraction& b)` | O(1) | [開く](#copy-assign) |
| `Fraction& a.operator=(Fraction&& b)` | O(1) | [開く](#move-assign) |
| `int64_t a.num() const` | O(1) | [開く](#num) |
| `int64_t a.den() const` | O(1) | [開く](#den) |
| `Fraction +a` | O(1) | [開く](#unary-plus) |
| `Fraction -a` | O(1) | [開く](#unary-minus) |
| `Fraction a + b` | O(log B) | [開く](#add) |
| `Fraction a - b` | O(log B) | [開く](#subtract) |
| `Fraction a * b` | O(log B) | [開く](#multiply) |
| `Fraction a / b` | O(log B) | [開く](#divide) |
| `Fraction& a += b` | O(log B) | [開く](#add-assign) |
| `Fraction& a -= b` | O(log B) | [開く](#subtract-assign) |
| `Fraction& a *= b` | O(log B) | [開く](#multiply-assign) |
| `Fraction& a /= b` | O(log B) | [開く](#divide-assign) |
| `bool a == b` | O(1) | [開く](#equal) |
| `bool a != b` | O(1) | [開く](#not-equal) |
| `bool a < b` | O(1) | [開く](#less) |
| `bool a > b` | O(1) | [開く](#greater) |
| `bool a <= b` | O(1) | [開く](#less-equal) |
| `bool a >= b` | O(1) | [開く](#greater-equal) |
| `ostream& out << a` | O(log B) | [開く](#stream) |
| `a.~Fraction()`（通常は自動） | O(1) | [開く](#destruct) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>Fraction a(int64_t n = 0, int64_t d = 1)</code> — O(log B)</summary>

既約化した n/d を作ります。引数なしは 0/1、引数1個なら整数です。整数からの暗黙変換も可能です。

{% raw %}
```cpp
Fraction a(6, -8); assert(a == Fraction(-3, 4));
Fraction zero; assert(zero.num() == 0 && zero.den() == 1);
```
{% endraw %}

注意点: d != 0。符号と約分を処理した後の値が保存範囲内であること。Fraction(2, INT64_MIN) は表現可能ですが Fraction(1, INT64_MIN) は前提違反です。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>Fraction a(const Fraction&amp; b)</code> — O(1)</summary>

値を独立にコピーします。

{% raw %}
```cpp
Fraction b(1, 3); Fraction a(b); a += 1;
assert(b == Fraction(1, 3) && a == Fraction(4, 3));
```
{% endraw %}

注意点: 所有する外部資源や共有状態はありません。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>Fraction a(Fraction&amp;&amp; b)</code> — O(1)</summary>

2個の整数をコピーして移動します。移動元も同じ値のまま使えます。

{% raw %}
```cpp
Fraction b(1, 3); Fraction a(std::move(b));
assert(a == b);
```
{% endraw %}

注意点: std::move には <utility> を include してください。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>Fraction&amp; a.operator=(const Fraction&amp; b)</code> — O(1)</summary>

a を b の値で置き換え、a 自身への参照を返します。

{% raw %}
```cpp
Fraction a, b(2, 3); a = b; assert(a == b);
```
{% endraw %}

注意点: 自己代入可能。返却参照は a の寿命まで有効です。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>Fraction&amp; a.operator=(Fraction&amp;&amp; b)</code> — O(1)</summary>

a を b の値で置き換えます。移動元も同じ値を保持します。

{% raw %}
```cpp
Fraction a, b(2, 3); a = std::move(b); assert(a == b);
```
{% endraw %}

注意点: 自己移動代入可能。返却参照は a の寿命まで有効。<utility> が必要です。

</details>

<details class="api-operation" id="num" markdown="1">
<summary><code>int64_t a.num() const</code> — O(1)</summary>

正規化後の分子を値で返します。

{% raw %}
```cpp
assert(Fraction(-6, 8).num() == -3);
```
{% endraw %}

注意点: 読み取り専用です。値の変更は Fraction の構築・代入で行います。

</details>

<details class="api-operation" id="den" markdown="1">
<summary><code>int64_t a.den() const</code> — O(1)</summary>

正規化後の正の分母を値で返します。

{% raw %}
```cpp
assert(Fraction(0, -8).den() == 1);
```
{% endraw %}

注意点: 常に 1 以上。ゼロの分母は 1 です。

</details>

<details class="api-operation" id="unary-plus" markdown="1">
<summary><code>Fraction +a</code> — O(1)</summary>

a と等しい値を返します。

{% raw %}
```cpp
Fraction a(-2, 3); assert(+a == a);
```
{% endraw %}

注意点: 元の値は変わりません。

</details>

<details class="api-operation" id="unary-minus" markdown="1">
<summary><code>Fraction -a</code> — O(1)</summary>

符号を反転した既約分数を返します。

{% raw %}
```cpp
assert(-Fraction(-2, 3) == Fraction(2, 3));
```
{% endraw %}

注意点: a.num() != INT64_MIN が必要です。INT64_MIN の符号反転は保存範囲を超えます。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>Fraction a + b</code> — O(log B)</summary>

正確な和を既約化して返します。

{% raw %}
```cpp
assert(Fraction(1, 6) + Fraction(1, 3) == Fraction(1, 2));
```
{% endraw %}

注意点: 最終的な既約分子・分母が保存範囲に収まること。中間値は 128bit で計算します。

</details>

<details class="api-operation" id="subtract" markdown="1">
<summary><code>Fraction a - b</code> — O(log B)</summary>

正確な差を既約化して返します。

{% raw %}
```cpp
assert(Fraction(1, 6) - Fraction(1, 3) == Fraction(-1, 6));
```
{% endraw %}

注意点: 最終的な既約分子・分母が保存範囲に収まること。

</details>

<details class="api-operation" id="multiply" markdown="1">
<summary><code>Fraction a * b</code> — O(log B)</summary>

正確な積を既約化して返します。

{% raw %}
```cpp
assert(Fraction(2, 3) * Fraction(3, 4) == Fraction(1, 2));
```
{% endraw %}

注意点: 約分前の積が int64 を超えても、約分後の値が保存範囲に収まれば利用できます。

</details>

<details class="api-operation" id="divide" markdown="1">
<summary><code>Fraction a / b</code> — O(log B)</summary>

正確な商を既約化して返します。

{% raw %}
```cpp
assert(Fraction(2, 3) / Fraction(-4, 5) == Fraction(-5, 6));
```
{% endraw %}

注意点: b.num() != 0。最終的な既約分子・分母が保存範囲に収まること。

</details>

<details class="api-operation" id="add-assign" markdown="1">
<summary><code>Fraction&amp; a += b</code> — O(log B)</summary>

a = a + b を行い、a 自身への参照を返します。

{% raw %}
```cpp
Fraction a(1, 3); a += a; assert(a == Fraction(2, 3));
```
{% endraw %}

注意点: 加算の前提を満たすこと。返却参照は a の寿命まで有効です。

</details>

<details class="api-operation" id="subtract-assign" markdown="1">
<summary><code>Fraction&amp; a -= b</code> — O(log B)</summary>

a = a - b を行い、a 自身への参照を返します。

{% raw %}
```cpp
Fraction a(1, 3); a -= a; assert(a == 0);
```
{% endraw %}

注意点: 減算の前提を満たすこと。自己参照も可能です。

</details>

<details class="api-operation" id="multiply-assign" markdown="1">
<summary><code>Fraction&amp; a *= b</code> — O(log B)</summary>

a = a * b を行い、a 自身への参照を返します。

{% raw %}
```cpp
Fraction a(2, 3); a *= a; assert(a == Fraction(4, 9));
```
{% endraw %}

注意点: 乗算の前提を満たすこと。返却参照は a の寿命まで有効です。

</details>

<details class="api-operation" id="divide-assign" markdown="1">
<summary><code>Fraction&amp; a /= b</code> — O(log B)</summary>

a = a / b を行い、a 自身への参照を返します。

{% raw %}
```cpp
Fraction a(2, 3); a /= a; assert(a == 1);
```
{% endraw %}

注意点: 除算の前提を満たすこと。0 /= 0 は前提違反です。

</details>

<details class="api-operation" id="equal" markdown="1">
<summary><code>bool a == b</code> — O(1)</summary>

同じ有理数か判定します。

{% raw %}
```cpp
assert(Fraction(2, 4) == Fraction(1, 2));
```
{% endraw %}

注意点: 既約分子・正の分母を直接比較します。浮動小数は使いません。

</details>

<details class="api-operation" id="not-equal" markdown="1">
<summary><code>bool a != b</code> — O(1)</summary>

異なる有理数か判定します。

{% raw %}
```cpp
assert(Fraction(1, 3) != Fraction(1, 2));
```
{% endraw %}

注意点: 整数との比較も可能です。

</details>

<details class="api-operation" id="less" markdown="1">
<summary><code>bool a &lt; b</code> — O(1)</summary>

a が b より小さいか判定します。

{% raw %}
```cpp
assert(Fraction(-1, 3) < Fraction(1, 2));
```
{% endraw %}

注意点: 128bit の交差積で比較し、すべての有効な値で overflow しません。

</details>

<details class="api-operation" id="greater" markdown="1">
<summary><code>bool a &gt; b</code> — O(1)</summary>

a が b より大きいか判定します。

{% raw %}
```cpp
assert(Fraction(2, 3) > Fraction(1, 2));
```
{% endraw %}

注意点: 誤差を含まない厳密な順序です。

</details>

<details class="api-operation" id="less-equal" markdown="1">
<summary><code>bool a &lt;= b</code> — O(1)</summary>

a が b 以下か判定します。

{% raw %}
```cpp
assert(Fraction(2, 4) <= Fraction(1, 2));
```
{% endraw %}

注意点: すべての有効な値を比較できます。

</details>

<details class="api-operation" id="greater-equal" markdown="1">
<summary><code>bool a &gt;= b</code> — O(1)</summary>

a が b 以上か判定します。

{% raw %}
```cpp
assert(Fraction(2, 4) >= Fraction(1, 2));
```
{% endraw %}

注意点: すべての有効な値を比較できます。

</details>

<details class="api-operation" id="stream" markdown="1">
<summary><code>ostream&amp; out &lt;&lt; a</code> — O(log B)</summary>

分母 1 なら整数、それ以外は分子/分母を出力し、out を返します。

{% raw %}
```cpp
std::ostringstream out; out << Fraction(6, -8);
assert(out.str() == "-3/4");
```
{% endraw %}

注意点: 例は <sstream> が必要です。改行は付けません。stream の書式フラグに従い、返却参照は out の寿命まで有効です。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~Fraction()</code> — O(1)</summary>

値の寿命を終えます。動的メモリを保持しないため、追加の解放処理はありません。

{% raw %}
```cpp
{ Fraction a(1, 2); assert(a.den() == 2); } // スコープ末尾で自動破棄
```
{% endraw %}

注意点: 通常は明示的に呼ばず、自動変数やコンテナに寿命を管理させてください。

</details>

## 出典・検証

- [Boost.Rational](https://www.boost.org/doc/libs/latest/libs/rational/rational.html) の既約値型と演算範囲を比較。コードの転載はせず、128bit 中間値と Euclid の互除法で実装しています。
- [Library Checker: Sort Points by Argument](https://judge.yosupo.jp/problem/sort_points_by_argument) で、半平面ごとの有理数比較による厳密な偏角ソートを検証します。これは比較・構築の公式検証であり、四則演算全体を検証する問題ではありません。
- `tests/random/fraction.cpp` は Boost.Multiprecision の無制限整数を独立 oracle とし、四則・複合代入・比較・正規化・出力・コピー/移動を比較します。INT64_MIN/MAX、相殺、負の分母、ゼロを含みます。
- 旧 `blueberry/fraction.hpp` の `fraction`、可変フィールド `num/den`、`raw_assign` は廃止。
  新パス・`Fraction`・`num()/den()` と正規化する構築・代入へ移行してください。
