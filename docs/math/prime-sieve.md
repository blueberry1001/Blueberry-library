---
title: Prime Sieve
documentation_of: //blueberry/math/prime-sieve.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

Eratosthenesの篩で上限L以下の素数を前計算します。判定表 O(L) bitに加え、素数列 O(π(L)) 個のintを保持します。大量の小さい整数の判定・列挙向けです。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/math/prime-sieve.hpp"
int main() {
  blueberry::PrimeSieve sieve(10);
  assert(sieve.is_prime(7));
  assert(!sieve.is_prime(1));
  assert((sieve.primes() == std::vector<int>{2, 3, 5, 7}));
  assert(sieve.limit() == 10);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `PrimeSieve sieve(limit)` | O(L log log L) | [開く](#construct) |
| `bool sieve.is_prime(x) const` | O(1) | [開く](#is-prime) |
| `const vector<int>& sieve.primes() const` | O(1)（参照取得） | [開く](#primes) |
| `int sieve.limit() const` | O(1) | [開く](#limit) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>PrimeSieve sieve(limit)</code> — O(L log log L)</summary>

limit以下の整数を前計算します。0と1は素数ではありません。

{% raw %}
```cpp
blueberry::PrimeSieve sieve(100);
```
{% endraw %}

注意点: 0<=limit<INT_MAX、かつ確保可能なメモリ量が必要です。limit+1のオーバーフローを避けます。非常に小さいLは定数時間です。

</details>

<details class="api-operation" id="is-prime" markdown="1">
<summary><code>bool sieve.is_prime(x) const</code> — O(1)</summary>

xが素数かを返します。

{% raw %}
```cpp
bool prime = sieve.is_prime(7);
```
{% endraw %}

注意点: 0<=x<=limit()。上限を超える整数は判定できません。

</details>

<details class="api-operation" id="primes" markdown="1">
<summary><code>const vector&lt;int&gt;&amp; sieve.primes() const</code> — O(1)（参照取得）</summary>

昇順の素数列をconst参照で返します。

{% raw %}
```cpp
const auto& primes = sieve.primes();
```
{% endraw %}

注意点: autoでコピーすると O(π(L)) です。参照は元オブジェクトより長く保持しないでください。

</details>

<details class="api-operation" id="limit" markdown="1">
<summary><code>int sieve.limit() const</code> — O(1)</summary>

指定した上限を返します。

{% raw %}
```cpp
int upper = sieve.limit();
```
{% endraw %}

注意点: 配列長ではなく、判定可能な最大値です。端点を含みます。

</details>
