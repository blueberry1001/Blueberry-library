---
title: Factorize
documentation_of: //blueberry/math/factorize.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

`uint64_t` 全域の素数判定と素因数分解です。ACLの公開APIにはなく、
小さい上限まで多数の問い合わせを行う `PrimeSieve` と異なり、大きな単発の整数に使います。
7基底の決定的Miller–RabinとPollard–Brent法を使い、剰余積はGCC/Clangの
`unsigned __int128` で計算します。MSVC単体は対象外です。

素数判定は64bitの剰余演算を定数時間とみなして $O(\log n)$、追加メモリ $O(1)$。
分解は確率的アルゴリズムに基づく探索で、最悪時間保証はありません。合成数の最小素因数を
$p$ とすると、rho法の典型的な期待更新回数は $O(\sqrt p)$ です。各更新は剰余演算、
128個ごとにgcdを計算します。素数判定・再帰的な分割に相当する処理・結果のsortも必要です。
素因数の個数を重複込みで $k$ とすると、追加メモリは $O(k)$、sortは $O(k\log k)$ です。

内部乱数は入力から初期化した局所状態です。同じ入力は同じ探索になり、global RNGを変更しません。
1試行の更新予算を制限して別の定数・初期値で再開しますが、敵対的入力への時間上限は保証しません。
入力を変更せず、返却vectorは自身で要素を所有します。内部 `factorize_detail` は公開APIではありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <vector>
#include "blueberry/math/factorize.hpp"
int main() {
  assert(blueberry::is_prime(1'000'000'007));
  assert(!blueberry::is_prime(1));
  assert((blueberry::factorize(360) == std::vector<std::uint64_t>{2, 2, 2, 3, 3, 5}));
  assert(blueberry::factorize(1).empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `bool is_prime(uint64_t n)` | O(log n)、0/1はO(1) | [開く](#is-prime) |
| `vector<uint64_t> factorize(uint64_t n)` | 最悪時間保証なし、各rho探索の期待更新回数O(sqrt p) | [開く](#factorize) |

<details class="api-operation" id="is-prime" markdown="1">
<summary><code>bool is_prime(uint64_t n)</code> — O(log n)</summary>

`n` が素数ならtrue。0と1はfalseです。固定の7基底を使い、`uint64_t` 全域で決定的に判定します。

{% raw %}
```cpp
bool prime = blueberry::is_prime(18'446'744'073'709'551'557ULL); // true
```
{% endraw %}

注意点: 引数は符号なし整数です。負整数を渡すとC++のunsigned変換が起きます。
判定結果に誤り確率はありませんが、暗号用途の定数時間実装ではありません。

</details>

<details class="api-operation" id="factorize" markdown="1">
<summary><code>vector&lt;uint64_t&gt; factorize(uint64_t n)</code> — 各rho探索の期待更新回数 O(sqrt p)</summary>

`n >= 1` の素因数を昇順・重複ありで返します。1の結果は空です。
同じ素因数の指数が必要なら返却値をrun-length圧縮できます。

{% raw %}
```cpp
auto factors = blueberry::factorize(1'000'000'016'000'000'063ULL);
// {1'000'000'007, 1'000'000'009}
```
{% endraw %}

注意点: 0は前提違反でassertの対象です。負整数は受け付けません。
各因数はuint64内ですが、利用者が因数を掛け戻す途中値や他の式も自身で範囲を確認してください。
実装の剰余積・加算・差はuint64全域でoverflowを避けています。探索の最悪時間は保証しません。

</details>

## 出典・検証

[NyaanのMiller–Rabin](https://github.com/NyaanNyaan/library/blob/master/prime/miller-rabin.hpp) と
[Pollard rho](https://github.com/NyaanNyaan/library/blob/master/prime/fast-factorize.hpp)、
[Fastest提出317402](https://judge.yosupo.jp/submission/317402) のアルゴリズム・積バッチgcdを調査し、
独立に実装しました。外部ソースは転記していません。Montgomeryや2系列同時更新は採用せず、
全uint64に対応する128bit剰余積と試行予算を使います。

公式問題: [Primality Test](https://judge.yosupo.jp/problem/primality_test)、
[Factorize](https://judge.yosupo.jp/problem/factorize)。公式上限は $10^{18}$ なので、
最大uint64近傍・平方・強擬素数を追加テストで補います。
