---
title: Bernoulli Numbers
documentation_of: //blueberry/math/bernoulli.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

ベルヌーイ数B₀,...,Bₙを列挙します。母関数x/(exp(x)-1)を使い、B₁=-1/2の規約に従います。

係数型Mintは素数法のACL `static_modint`を使います。n+1項を最悪時間O((n+1) log(n+2))、メモリO(n+1)で生成します。必要なNTT長を法がサポートすることが前提です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`a = bernoulli<Mint>(n)`](#compute) | 最悪O((n+1) log(n+2)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `0 <= n < p-1`、p=`Mint::mod()`は素数です。`Mint`はACLの`static_modint<p>`を使います。FPSの要求精度`n+1 < p`も満たす必要があります。
- n+1以上の最小の2冪Lがp-1を割る必要があります。法998244353では`n <= 8388607`です。確保する配列が利用可能なメモリに収まる必要があります。
- 係数は法pの値です。通常の整数としての値や有理数は保持しません。法演算はACLへ委譲します。
- 返り値は独立した所有ベクトルです。共有キャッシュや入力への参照を持たず、既存の参照を無効化しません。計算量は償却ではありません。
- 係数1/(i+1)!の級数を逆元にし、i!を掛けます。階乗の逆元は1回だけ求めます。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/bernoulli.hpp"
using Mint = atcoder::modint998244353;
int main() {
  auto a = blueberry::bernoulli<Mint>(4);
  assert((a == std::vector<Mint>{1, -Mint(2).inv(), Mint(6).inv(), 0, -Mint(30).inv()}));
  assert((blueberry::bernoulli<Mint>(0) == std::vector<Mint>{1}));
}
```
{% endraw %}

<details class="api-operation" id="compute" markdown="1">
<summary><code>std::vector&lt;Mint&gt; bernoulli&lt;Mint&gt;(int n)</code> — 最悪O((n+1) log(n+2))</summary>

添字0からnまでの数列を長さn+1のベクトルで返します。nは最後に求める添字です。n=0では`{1}`を返し、空ベクトルにはなりません。

{% raw %}
```cpp
// Mintは最小使用例の型です。
auto a = blueberry::bernoulli<Mint>(4);
assert(a.size() == 5 && a[0] == Mint(1));
```
{% endraw %}

注意点: 有理数の分母の逆元を法pで計算します。B₁=+1/2の規約とは符号が異なります。 共通の法・精度・NTT長の制約に従います。負のnや未対応の法・長さは契約違反です。返したベクトルの参照の寿命と無効化は`std::vector`に従います。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [Library Checker公式問題](https://judge.yosupo.jp/problem/bernoulli_number)。公式checkerで全項を検証します。
- `tests/random/fps_sequences.cpp`で境界・2冪前後と愚直な漸化式を比較します。乱数seedを指定して再現できます。
- [Nyaanの数列実装](https://nyaannyaan.github.io/library/fps/fps-famous-series.hpp)と公開Fastest提出をアルゴリズム資料として参照し、コードは独立に実装しました。NTTは既存FPSを通じてACLを利用します。ACLに同じ数列生成APIはありません。
- [性能調査と測定ログ](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/lc-expansion-fps-sequences.md)。

</details>
