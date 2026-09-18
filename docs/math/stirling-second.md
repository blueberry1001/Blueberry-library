---
title: Stirling Numbers of the Second Kind
documentation_of: //blueberry/math/stirling-second.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

n要素集合をk個の非空集合に分割する個数S(n,k)を、k=0..nの順に返す。n=0の結果は {1}。包含排除をACL畳み込みに変換し、各i^nは二分累乗で計算する。

0 <= n < p、p=Mint::mod()は素数。2n+1以上の最小の2冪がp-1を割ること。MintはACL static_modint。0^0=1の規約を利用する。 メモリは O(n+1)。以下は最悪計算量（償却ではない）。返り値は独立した所有ベクトルで、既存の参照を無効化しない。法演算を除く長さは利用可能なメモリに収まる必要がある。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/stirling-second.hpp"
#include <atcoder/modint>
using Mint = atcoder::modint998244353;
int main() {
  auto a = blueberry::stirling_second<Mint>(4);
  assert((a == std::vector<Mint>{0, 1, 7, 6, 1}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<Mint> stirling_second(int n)` | O(n log(n+2)) | [開く](#compute) |

<details class="api-operation" id="compute" markdown="1">
<summary><code>std::vector&lt;Mint&gt; stirling_second(int n)</code> — O(n log(n+2))</summary>

n要素集合をk個の非空集合に分割する個数S(n,k)を、k=0..nの順に返す。n=0の結果は {1}。包含排除をACL畳み込みに変換し、各i^nは二分累乗で計算する。

{% raw %}
```cpp
// Mint は最小使用例の型（必要な場合）。
auto a = blueberry::stirling_second<Mint>(4);
  assert((a == std::vector<Mint>{0, 1, 7, 6, 1}));
```
{% endraw %}

注意点: 0 <= n < p、p=Mint::mod()は素数。2n+1以上の最小の2冪がp-1を割ること。MintはACL static_modint。0^0=1の規約を利用する。 入力範囲外は契約違反。法を超える整数としての個数は保持しない。

</details>

## 出典・検証

[公式問題](https://judge.yosupo.jp/problem/stirling_number_of_the_second_kind)、[アルゴリズム比較資料](https://raw.githubusercontent.com/ei1333/library/master/math/fps/stirling-second.hpp)を参照し、コードは独立に実装した。候補比較・追加資料・生ログは [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-math.md)。乱数seed付きの愚直比較は `tests/random/math_batch_three.cpp`。
