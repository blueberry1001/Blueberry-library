---
title: Polynomial Product
documentation_of: //blueberry/math/polynomial-product.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

K個の多項式を昇べき順の係数列で渡し、積を返します。Mintは素数法のACL `static_modint` を使い、必要なNTT長を法がサポートすることが前提です。空の因子リストは{1}、空の因子がある場合は空のゼロ多項式です。

Dを非空因子の次数（末尾ゼロを含む長さ-1）の総和として、最悪時間O((D+K) log(D+2) log(K+2))、メモリO(D+K)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`polynomial_product`](#compute) | O((D+K) log(D+2) log(K+2)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- p=Mint::mod()として、D+1以上の最小の2冪がp-1を割ることが必要です。長さは利用可能なメモリに収まる必要があります。
- 末尾ゼロを削除しません。隣接ペアを反復して掛ける平衡積木でACL畳み込みを使います。
- 値渡しなのでlvalueはコピー、`std::move` なら所有権を移します。返り値は独立した所有ベクトルで、既存の参照を無効化しません。計算量は償却ではありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/polynomial-product.hpp"
#include <atcoder/modint>
using Mint = atcoder::modint998244353;
int main() {
  auto a = blueberry::polynomial_product<Mint>({{1, 2}, {3, 4}});
  assert((a == std::vector<Mint>{3, 10, 8}));
}
```
{% endraw %}

<details class="api-operation" id="compute" markdown="1">
<summary><code>std::vector&lt;Mint&gt; polynomial_product(std::vector&lt;std::vector&lt;Mint&gt;&gt; factors)</code> — O((D+K) log(D+2) log(K+2))</summary>

K個の多項式を昇べき順の係数列で渡し、積を返す。Dは非空因子の次数（末尾ゼロを含む長さ-1）の総和。空の因子リストは {1}、空の因子がある場合は空のゼロ多項式。末尾ゼロを削除しない。

{% raw %}
```cpp
// Mint は最小使用例の型（必要な場合）。
auto a = blueberry::polynomial_product<Mint>({{1, 2}, {3, 4}});
  assert((a == std::vector<Mint>{3, 10, 8}));
```
{% endraw %}

注意点: MintはACL static_modint、法pは素数。D+1以上の最小の2冪がp-1を割ること。値渡しなのでlvalueはコピー、std::moveなら所有権を移す。隣接ペアを反復して掛ける平衡積木でACL畳み込みを利用する。 入力範囲外は契約違反。法を超える整数としての個数は保持しない。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [公式問題](https://judge.yosupo.jp/problem/product_of_polynomial_sequence)、[アルゴリズム比較資料](https://raw.githubusercontent.com/NyaanNyaan/library/master/fps/fps-utility.hpp)を参照し、コードは独立に実装した。候補比較・追加資料・生ログは [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-math.md)。乱数seed付きの愚直比較は `tests/random/math_batch_three.cpp`。

</details>
