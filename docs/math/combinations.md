---
title: Combinations
documentation_of: //blueberry/math/combinations.hpp
---

## 概要

階乗・逆階乗を前計算し、組合せ数 C(n,k)・順列数 P(n,k) を求めます。

N は前計算上限で、Mint で 1～N がすべて可逆である必要があります。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`Combinations<Mint>`](#construct) | O(N + I) |
| [`comb.limit`](#limit) | O(1) |
| [`comb.fact`](#fact) | O(1) |
| [`comb.inv_fact`](#inv-fact) | O(1) |
| [`comb.C`](#choose) | O(1) |
| [`comb.P`](#permute) | O(1) |
| [`Combinations`（コピー構築）](#copy-construct) | O(N+1) |
| [`Combinations& operator=`（コピー代入）](#copy-assign) | O(N+Ndst+1) |
| [`Combinations`（移動構築）](#move-construct) | O(1) |
| [`Combinations& operator=`（移動代入）](#move-assign) | O(Ndst+1) |
| [`~Combinations`](#destruct) | O(N+1) |

<!-- library-implementation -->

## 概要・前提

ACL modint などの体上で、階乗・逆階乗を 1 回前計算し C(n,k)/P(n,k) を O(1) で計算します。元テンプレートの COMinit/COM/nPr を、modulus ごとの独立オブジェクトへ置き換えました。Mint は整数からの構築、乗算、inv() を持つ型です。通常は ACL static_modint を使います。

N は前計算上限、Ndst は代入先の変更前の上限、I は Mint::inv() の計算量です。計算量は Mint の構築・コピー・代入・乗算・破棄が O(1) の場合を示します。重い係数型では、それぞれの操作回数に係数型のコストを掛けてください。C/P は係数を値で返し、0を返す場合も Mint の構築コストがかかります。fact/inv_fact の返り値にもコピーコストが含まれます。

0 <= N < INT_MAX、1～N がすべて可逆であることが前提です。構築時間は O(N + I)、メモリは Mint 2(N+1) 個です。各表はオブジェクトが所有し、結果は値で返します。内部の公開フィールド・参照・iteratorはありません。表が通常の構築条件を満たすオブジェクトに対してのみ問合せを行ってください。移動元は破棄・代入・再構築のみが対象です。既定のコピー・移動操作を利用し、特別メンバを独自実装してはいません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <atcoder/modint>
#include "blueberry/math/combinations.hpp"
int main() {
  using mint = atcoder::modint998244353;
  blueberry::Combinations<mint> comb(10), zero(0);
  assert(comb.C(5, 2).val() == 10 && comb.P(5, 2).val() == 20);
  assert(comb.C(0, 0).val() == 1 && comb.C(2, 3).val() == 0);
  assert(zero.fact(0).val() == 1 && zero.inv_fact(0).val() == 1);
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>Combinations&lt;Mint&gt; comb(int limit)</code> — O(N + I)</summary>

0～limit の階乗・逆階乗を前計算します。全体に inverse は 1 回です。

{% raw %}
```cpp
blueberry::Combinations<atcoder::modint998244353> comb(10);
```
{% endraw %}

注意点: N=limit、I は Mint::inv() のコストです。0 <= limit < INT_MAX、1～limit が全て可逆であることが必要です。ACL の prime modint では limit < modulus。dynamic_modint の modulus は構築前に設定し、オブジェクト使用中は変更禁止です。メモリは Mint 2(N+1) 個。

</details>

<details class="api-operation" id="limit" markdown="1">
<summary><code>int comb.limit() const</code> — O(1)</summary>

前計算済みの最大 n を返します。

{% raw %}
```cpp
assert(comb.limit() == 10);
```
{% endraw %}

注意点: 構築時の値を返します。前計算を拡張する操作はありません。

</details>

<details class="api-operation" id="fact" markdown="1">
<summary><code>Mint comb.fact(int n) const</code> — O(1)</summary>

n! を Mint として返します。

{% raw %}
```cpp
assert(comb.fact(3).val() == 6);
```
{% endraw %}

注意点: 0 <= n <= limit が必要です。0! = 1。Mint のコピーコストを O(1) としています。

</details>

<details class="api-operation" id="inv-fact" markdown="1">
<summary><code>Mint comb.inv_fact(int n) const</code> — O(1)</summary>

n! の逆元を返します。

{% raw %}
```cpp
assert((comb.fact(3) * comb.inv_fact(3)).val() == 1);
```
{% endraw %}

注意点: 0 <= n <= limit が必要です。全ての階乗が可逆という構築条件を満たしてください。

</details>

<details class="api-operation" id="choose" markdown="1">
<summary><code>Mint comb.C(int n, int k) const</code> — O(1)</summary>

n 個から k 個を選ぶ組合せ数を返します。

{% raw %}
```cpp
assert(comb.C(5, 2).val() == 10);
assert(comb.C(0, 0).val() == 1 && comb.C(3, 4).val() == 0);
```
{% endraw %}

注意点: n<0、k<0、k>n は 0 を返し、範囲検査より先に扱います。それ以外は n <= limit が必要です。n >= modulus の Lucas 法には対応しません。Mint の構築・コピー・乗算コストを O(1) としています。重い係数型では返り値を生成するコストも含めます。

</details>

<details class="api-operation" id="permute" markdown="1">
<summary><code>Mint comb.P(int n, int k) const</code> — O(1)</summary>

n 個から順序付きで k 個を選ぶ順列数を返します。

{% raw %}
```cpp
assert(comb.P(5, 2).val() == 20);
assert(comb.P(0, 0).val() == 1 && comb.P(3, 4).val() == 0);
```
{% endraw %}

注意点: C と同じ範囲・無効 k の扱いです。k=0 は 1。Mint の構築・コピー・乗算コストを O(1) としており、重い係数型では返り値を生成するコストも含めます。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>Combinations(const Combinations&amp; other)</code> — O(N+1)</summary>

2本の前計算表を独立に複製します。

{% raw %}
```cpp
auto copy = comb;
assert(copy.C(5, 2) == comb.C(5, 2));
```
{% endraw %}

注意点: コピー元は通常の構築条件を満たす必要があります。追加メモリは Mint 2(N+1) 個。Mint のコピーが必要です。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>Combinations&amp; operator=(const Combinations&amp; other)</code> — O(N+Ndst+1)</summary>

既存の表を独立コピーに置き換え、自身への参照を返します。

{% raw %}
```cpp
auto copy = comb;
copy = comb;
```
{% endraw %}

注意点: Ndst は代入前の上限です。自身への代入も可能です。Mint のコピー・代入が必要です。既存の表を解放することがあり、その費用を計算量に含めます。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>Combinations(Combinations&amp;&amp; other)</code> — O(1)</summary>

前計算表の所有権を移します。

{% raw %}
```cpp
auto copy = comb;
auto moved = std::move(copy);
assert(moved.C(5, 2) == comb.C(5, 2));
```
{% endraw %}

注意点: 移動元に limit/fact/inv_fact/C/P を呼ばないでください。移動元は破棄、通常の構築済みオブジェクトからの代入、または再構築のみを行います。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>Combinations&amp; operator=(Combinations&amp;&amp; other)</code> — O(Ndst+1)</summary>

既存の表を破棄して所有権を移し、自身への参照を返します。

{% raw %}
```cpp
auto copy = comb;
blueberry::Combinations<atcoder::modint998244353> target(0);
target = std::move(copy);
```
{% endraw %}

注意点: Ndst は代入前の上限です。移動元の操作制限は移動構築と同じです。自己移動後も問合せをせず、通常の構築済みオブジェクトを代入してから使ってください。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~Combinations()</code> — O(N+1)</summary>

所有する2本の前計算表を解放します。

{% raw %}
```cpp
{ auto copy = comb; } // スコープ終了時に破棄
```
{% endraw %}

注意点: Mint の破棄コストを O(1) としています。移動元も破棄できます。内部への参照やiteratorは公開しません。

</details>

## 出典・検証

[Library Checker Binomial Coefficient (Prime Mod)](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) の verify driver と、Pascal 三角形による小さい素数の全数比較を用意しています。前計算アルゴリズムは元ユーザーテンプレートの用途を維持し、ACL の modint と 1 回の inverse に変更しました。性能比較・Fastest 調査の範囲は [移行記録]({{ '/docs/development/contest-template-import.html' | relative_url }}) を参照してください。
