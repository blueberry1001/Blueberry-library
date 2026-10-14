---
title: Combinations
documentation_of: //blueberry/math/combinations.hpp
---

## 概要・前提

ACL modint などの体上で、階乗・逆階乗を 1 回前計算し C(n,k)/P(n,k) を O(1) で計算します。元テンプレートの COMinit/COM/nPr を、modulus ごとの独立オブジェクトへ置き換えました。Mint は整数からの構築、乗算、inv() を持つ型です。通常は ACL static_modint を使います。

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Combinations<Mint> comb(int limit)` | O(N + I) | [開く](#construct) |
| `int comb.limit() const` | O(1) | [開く](#limit) |
| `Mint comb.fact(int n) const` | O(1) | [開く](#fact) |
| `Mint comb.inv_fact(int n) const` | O(1) | [開く](#inv-fact) |
| `Mint comb.C(int n, int k) const` | O(1) | [開く](#choose) |
| `Mint comb.P(int n, int k) const` | O(1) | [開く](#permute) |

<!-- library-implementation -->

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

注意点: n<0、k<0、k>n は 0 を返し、範囲検査より先に扱います。それ以外は n <= limit が必要です。n >= modulus の Lucas 法には対応しません。Mint の演算コストを O(1) としています。

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

注意点: C と同じ範囲・無効 k の扱いです。k=0 は 1。

</details>

## 出典・検証

[Library Checker Binomial Coefficient (Prime Mod)](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) の verify driver と、Pascal 三角形による小さい素数の全数比較を用意しています。前計算アルゴリズムは元ユーザーテンプレートの用途を維持し、ACL の modint と 1 回の inverse に変更しました。性能比較・Fastest 調査の範囲は [移行記録](../development/contest-template-import.html) を参照してください。
