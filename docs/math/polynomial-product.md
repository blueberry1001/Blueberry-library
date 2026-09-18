---
title: Polynomial Product
documentation_of: //blueberry/math/polynomial-product.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

K個の多項式を昇べき順の係数列で渡し、積を返す。Dは非空因子の次数（末尾ゼロを含む長さ-1）の総和。空の因子リストは {1}、空の因子がある場合は空のゼロ多項式。末尾ゼロを削除しない。

MintはACL static_modint、法pは素数。D+1以上の最小の2冪がp-1を割ること。値渡しなのでlvalueはコピー、std::moveなら所有権を移す。隣接ペアを反復して掛ける平衡積木でACL畳み込みを利用する。 メモリは O(D+K)。以下は最悪計算量（償却ではない）。返り値は独立した所有ベクトルで、既存の参照を無効化しない。法演算を除く長さは利用可能なメモリに収まる必要がある。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<Mint> polynomial_product(std::vector<std::vector<Mint>> factors)` | O((D+K) log(D+2) log(K+2)) | [開く](#compute) |

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

## 出典・検証

[公式問題](https://judge.yosupo.jp/problem/product_of_polynomial_sequence)、[アルゴリズム比較資料](https://raw.githubusercontent.com/NyaanNyaan/library/master/fps/fps-utility.hpp)を参照し、コードは独立に実装した。候補比較・追加資料・生ログは [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-math.md)。乱数seed付きの愚直比較は `tests/random/math_batch_three.cpp`。
