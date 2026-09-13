---
title: Prime Sieve
documentation_of: //blueberry/math/prime-sieve.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

Eratosthenesの篩で上限以下の素数を前計算します。上限を $N$ とすると構築は
$O(N\log\log N)$、メモリは $O(N)$、判定と上限取得は $O(1)$ です。素数一覧の取得は
列挙した個数を $P$ として参照を返し、列挙自体は構築時に完了しています。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/prime-sieve.hpp"
int main() {
  blueberry::PrimeSieve sieve(10);
  assert(sieve.is_prime(7));
  assert(!sieve.is_prime(9));
  assert(sieve.primes().size() == 4);
  assert(sieve.limit() == 10);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `PrimeSieve sieve(limit)` | O(N log log N) | [開く](#construct) |
| `bool sieve.is_prime(x) const` | O(1) | [開く](#is-prime) |
| `const vector<int>& sieve.primes() const` | O(1) access | [開く](#primes) |
| `int sieve.limit() const` | O(1) | [開く](#limit) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>PrimeSieve sieve(limit)</code> — O(N log log N)</summary>

`0..limit` の素数判定表と素数一覧を構築します。

{% raw %}
```cpp
blueberry::PrimeSieve sieve(1'000'000);
```
{% endraw %}

注意点: `limit` は0以上。大きな上限では `vector<bool>` と素数一覧のメモリを確保します。

</details>

<details class="api-operation" id="is-prime" markdown="1">
<summary><code>bool sieve.is_prime(x) const</code> — O(1)</summary>

`x` が素数かどうかを返します。

{% raw %}
```cpp
if (sieve.is_prime(97)) { /* prime */ }
```
{% endraw %}

注意点: `x` は `[0,limit]`。範囲外はassertで停止します。

</details>

<details class="api-operation" id="primes" markdown="1">
<summary><code>const vector&lt;int&gt;&amp; sieve.primes() const</code> — O(1) access</summary>

昇順の素数一覧へのconst参照を返します。

{% raw %}
```cpp
for (int p : sieve.primes()) { /* use p */ }
```
{% endraw %}

注意点: 参照先は `sieve` の生存中だけ有効です。返り値のvectorは変更できません。

</details>

<details class="api-operation" id="limit" markdown="1">
<summary><code>int sieve.limit() const</code> — O(1)</summary>

判定表の上限 $N$ を返します。

{% raw %}
```cpp
int n = sieve.limit();
```
{% endraw %}

注意点: コンストラクタ引数と同じ値で、構築後は変化しません。

</details>
