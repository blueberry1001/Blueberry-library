---
title: Contest Helpers
documentation_of: //blueberry/utility/contest-helpers.hpp
---

## 概要

最小・最大の更新、整数二分探索・丸め除算、文字列・配列・グラフの補助関数です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`chmin`](#chmin) | O(1) |
| [`chmax`](#chmax) | O(1) |
| [`floor_div`](#floor-div) | O(1) |
| [`bin_search`](#binary-search) | O(log(D+1)) |
| [`sum`](#sum) | O(N) |
| [`min_of`](#min-of) | O(N) |
| [`max_of`](#max-of) | O(N) |
| [`len`](#len) | O(1) |
| [`Fill`](#fill) | O(S) |
| [`is_palindrome`](#palindrome) | O(N) |
| [`my_distance`](#distance) | O(1) |
| [`range_sum`](#range-sum) | O(1) |
| [`compress`](#compress) | O(N log N) |
| [`divisors`](#divisors) | O(F(n) + tau(n) log tau(n)) |
| [`is_bipartite`](#bipartite) | O(V+E) |
| [`to_string128`](#to-string128) | O(L) |
| [`parse128`](#parse128) | O(L) |

<!-- library-implementation -->

## 概要・前提

競技用テンプレートから、名前空間内で使える小さな補助関数を整理しました。通常のヘッダと併用できます。N は要素数、V/E は頂点数/隣接リストの辺数です。特記しない操作の追加メモリは O(1)、返り値は値として所有します。整数の 128 bit 部分には GNU 拡張を使用します。各操作の詳細に範囲・空入力・追加メモリを示します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <limits>
#include "blueberry/utility/contest-helpers.hpp"
int main() {
  int best = 9;
  assert(blueberry::chmin(best, 3) && best == 3);
  assert(blueberry::floor_div(-7LL, 3LL) == -3);
  assert(blueberry::bin_search(0, 10, [](long long x) { return x <= 4; }) == 4);
  std::vector<long long> a{8, -2, 8};
  assert(blueberry::compress(a) == 2 && a[0] == 1 && a[1] == 0);
  const auto minimum = std::numeric_limits<__int128>::min();
  assert(blueberry::parse128(blueberry::to_string128(minimum)).value() == minimum);
}
```
{% endraw %}

<details class="api-operation" id="chmin" markdown="1">
<summary><code>bool chmin(T&amp; x, const U&amp; y)</code> — O(1)</summary>

y < x のとき x = y として true、それ以外は false を返します。

{% raw %}
```cpp
int x = 5; assert(blueberry::chmin(x, 2) && x == 2);
```
{% endraw %}

注意点: 比較と代入が可能な型を使います。変換で値が失われない組合せを指定し、signed/unsigned 混在比較に注意してください。比較・代入のコストを O(1) としています。

</details>

<details class="api-operation" id="chmax" markdown="1">
<summary><code>bool chmax(T&amp; x, const U&amp; y)</code> — O(1)</summary>

x < y のとき x = y として true、それ以外は false を返します。

{% raw %}
```cpp
int x = 5; assert(blueberry::chmax(x, 8) && x == 8);
```
{% endraw %}

注意点: chmin と同じ型条件です。同値では変更しません。

</details>

<details class="api-operation" id="floor-div" markdown="1">
<summary><code>T floor_div(T a, T b)</code> — O(1)</summary>

符号付き整数の数学的な floor(a / b) を返します。負の除数にも対応します。

{% raw %}
```cpp
assert(blueberry::floor_div(-7LL, 3LL) == -3);
assert(blueberry::floor_div(7LL, -3LL) == -3);
```
{% endraw %}

注意点: b != 0、かつ min(T) / -1 は禁止です。T は同一の符号付き整数型です。最小値の絶対値や a+b を作らないため、それ以外の端点も使えます。

</details>

<details class="api-operation" id="binary-search" markdown="1">
<summary><code>long long bin_search(long long ok, long long ng, const F&amp; pred)</code> — O(log(D+1))</summary>

述語が true の側を ok、false の側を ng とする整数二分探索です。両方向に対応し、最後の ok を返します。

{% raw %}
```cpp
assert(blueberry::bin_search(0, 10, [](long long x) { return x <= 4; }) == 4);
```
{% endraw %}

注意点: D は端点の距離、述語のコストは O(1) とします。端点は評価しない仮想番兵も可能です。内部区間で true/false の境界が単調であることが必要です。距離と中点は LLONG_MIN～LLONG_MAX でオーバーフローしません。ok == ng または隣接なら評価せず ok を返します。

</details>

<details class="api-operation" id="sum" markdown="1">
<summary><code>T sum(const std::vector&lt;T&gt;&amp; a)</code> — O(N)</summary>

T{} を初期値とする左から順の加算です。空配列は T{} を返します。

{% raw %}
```cpp
assert(blueberry::sum(std::vector<int>{1, 2, 3}) == 6);
```
{% endraw %}

注意点: T{} が加算の単位元であり、operator+= が定義されていることが必要です。operator+ は不要です。途中結果が T の範囲に収まるようにしてください。

</details>

<details class="api-operation" id="min-of" markdown="1">
<summary><code>T min_of(const std::vector&lt;T&gt;&amp; a)</code> — O(N)</summary>

最小要素のコピーを返します。

{% raw %}
```cpp
assert(blueberry::min_of(std::vector<int>{4, 1, 9}) == 1);
```
{% endraw %}

注意点: 空配列は禁止です。比較が strict weak ordering を満たす必要があります。T のコピー・比較コストを O(1) としています。

</details>

<details class="api-operation" id="max-of" markdown="1">
<summary><code>T max_of(const std::vector&lt;T&gt;&amp; a)</code> — O(N)</summary>

最大要素のコピーを返します。

{% raw %}
```cpp
assert(blueberry::max_of(std::vector<int>{4, 1, 9}) == 9);
```
{% endraw %}

注意点: 空配列は禁止です。min_of と同じ比較・コピー条件です。

</details>

<details class="api-operation" id="len" markdown="1">
<summary><code>int len(const Container&amp; a)</code> — O(1)</summary>

size() を int に変換して返します。空コンテナは 0 です。

{% raw %}
```cpp
assert(blueberry::len(std::string("abc")) == 3);
```
{% endraw %}

注意点: size() <= INT_MAX が必要です。size() 自体のコストを O(1) としています。

</details>

<details class="api-operation" id="fill" markdown="1">
<summary><code>void Fill(T (&amp;a)[N], const U&amp; value)</code> — O(S)</summary>

組込み配列の全要素を value で埋めます。多次元配列も実際の部分配列ごとに再帰します。

{% raw %}
```cpp
int a[2][3]; blueberry::Fill(a, -1); assert(a[1][2] == -1);
```
{% endraw %}

注意点: S は葉要素の総数です。各葉へ U を代入できることが必要です。std::array/vector は対象外です。途中で例外が出た場合は一部更新されます。

</details>

<details class="api-operation" id="palindrome" markdown="1">
<summary><code>bool is_palindrome(std::string_view s)</code> — O(N)</summary>

文字列の byte 列が回文なら true を返します。

{% raw %}
```cpp
assert(blueberry::is_palindrome("abba"));
```
{% endraw %}

注意点: 空文字列と 1 byte は true です。UTF-8 の Unicode 文字単位の判定ではありません。参照を保持しません。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>long double my_distance(long double x, long double y, long double xx, long double yy)</code> — O(1)</summary>

2 点のユークリッド距離を std::hypot で返します。

{% raw %}
```cpp
assert(blueberry::my_distance(0, 0, 3, 4) == 5);
```
{% endraw %}

注意点: 浮動小数点の丸めがあります。座標差が long double で表現できることを想定します。無限大・NaN は標準 hypot の規則に従います。

</details>

<details class="api-operation" id="range-sum" markdown="1">
<summary><code>__int128 range_sum(long long a, long long b)</code> — O(1)</summary>

閉区間 [a,b] の整数の和を 128 bit で返します。a > b は 0 です。

{% raw %}
```cpp
assert(blueberry::range_sum(-2, 3) == 3);
```
{% endraw %}

注意点: 全 signed 64 bit 端点で途中結果も signed 128 bit に収まります。long long へ戻す場合は返り値の範囲を確認してください。

</details>

<details class="api-operation" id="compress" markdown="1">
<summary><code>int compress(std::vector&lt;long long&gt;&amp; a)</code> — O(N log N)</summary>

要素を昇順の distinct 値の 0-based rank に置換し、distinct 数を返します。

{% raw %}
```cpp
std::vector<long long> a{7, -4, 7};
assert(blueberry::compress(a) == 2 && a[0] == 1 && a[1] == 0);
```
{% endraw %}

注意点: N <= INT_MAX が必要です。空入力は 0 を返します。配列のサイズ・capacity は変えず、元の値は失われます。参照・iterator は無効化しません。作業メモリ O(N)。

</details>

<details class="api-operation" id="divisors" markdown="1">
<summary><code>std::vector&lt;std::uint64_t&gt; divisors(std::uint64_t n)</code> — O(F(n) + tau(n) log tau(n))</summary>

既存 factorize の結果から正の約数を重複なく昇順で返します。

{% raw %}
```cpp
assert((blueberry::divisors(12) == std::vector<std::uint64_t>{1,2,3,4,6,12}));
```
{% endraw %}

注意点: n >= 1 が必要です。1 は {1} です。F(n) は既存 Pollard–Rho の因数分解コストで、期待計算量・最悪保証は factorize のページを参照してください。tau(n) は約数数です。生成積は n の約数なので uint64_t 内で収まります。作業・出力メモリ O(tau(n) + log n)。

</details>

<details class="api-operation" id="bipartite" markdown="1">
<summary><code>bool is_bipartite(const std::vector&lt;std::vector&lt;Index&gt;&gt;&amp; g)</code> — O(V+E)</summary>

無向グラフが二部グラフなら true を返します。各連結成分を反復 BFS で調べます。

{% raw %}
```cpp
std::vector<std::vector<int>> g{{1},{0,2},{1}};
assert(blueberry::is_bipartite(g));
```
{% endraw %}

注意点: V <= INT_MAX、全隣接頂点が [0,V) の整数であることが必要です。無向辺は両方向に格納します。空グラフ・孤立点は true、自己ループは false、多重辺は許可します。入力を書き換えず、再帰しません。作業メモリ O(V)。

</details>

<details class="api-operation" id="to-string128" markdown="1">
<summary><code>std::string to_string128(__int128 value)</code> — O(L)</summary>

signed 128 bit 値を符号付き十進文字列に変換します。

{% raw %}
```cpp
assert(blueberry::to_string128(-123) == "-123");
```
{% endraw %}

注意点: 最小値 -2^127 にも対応します。L は出力長で最大 40 byte、返り値が文字列を所有します。unsigned 128 bit のオーバーロードはありません。

</details>

<details class="api-operation" id="parse128" markdown="1">
<summary><code>std::optional&lt;__int128&gt; parse128(std::string_view text)</code> — O(L)</summary>

先頭の任意の +/- と 1 桁以上の ASCII 数字を読みます。不正構文・範囲外は nullopt です。

{% raw %}
```cpp
assert(blueberry::parse128("-123").value() == -123);
assert(!blueberry::parse128("12x") && !blueberry::parse128(""));
```
{% endraw %}

注意点: 空白は許可しません。範囲は [-2^127,2^127-1]、先頭ゼロと -0 は許可します。文字列を保持せず、符号を無視した旧 parsetoint128 の挙動は引き継ぎません。

</details>

## 出典・検証

元はユーザー所有の `/home/blueberry/lib/blueberrytemplate.h`。因数分解は既存 [Factorize](../math/factorize.html) に委譲します。新しい高度なアルゴリズムの重複実装は追加していません。`tests/random/contest-helpers.cpp` の境界・固定 seed 比較、GCC/Clang の include/ODR 確認を使います。採否・性能測定は [移行記録](../development/contest-template-import.html) を参照してください。
