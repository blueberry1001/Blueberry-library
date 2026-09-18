---
title: Taylor Shift
documentation_of: //blueberry/math/taylor-shift.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

N=f.size()。昇べき順の係数から f(x+c) の係数を同じ長さで返す。空入力は空、末尾のゼロも保持する。入力を変更しない。

N <= p、p=Mint::mod() は素数。2N-1 以上の最小の2冪が p-1 を割ること。Mint は ACL static_modint。階乗と逆階乗を用いた1回のACL畳み込みで計算する。 メモリは O(N)。以下は最悪計算量（償却ではない）。返り値は独立した所有ベクトルで、既存の参照を無効化しない。法演算を除く長さは利用可能なメモリに収まる必要がある。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/taylor-shift.hpp"
#include <atcoder/modint>
using Mint = atcoder::modint998244353;
int main() {
  auto a = blueberry::taylor_shift(std::vector<Mint>{1, 2, 3}, Mint(2));
  assert((a == std::vector<Mint>{17, 14, 3}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<Mint> taylor_shift(const std::vector<Mint>& f, Mint c)` | O(N log(N+1)) | [開く](#compute) |

<details class="api-operation" id="compute" markdown="1">
<summary><code>std::vector&lt;Mint&gt; taylor_shift(const std::vector&lt;Mint&gt;& f, Mint c)</code> — O(N log(N+1))</summary>

N=f.size()。昇べき順の係数から f(x+c) の係数を同じ長さで返す。空入力は空、末尾のゼロも保持する。入力を変更しない。

{% raw %}
```cpp
// Mint は最小使用例の型（必要な場合）。
auto a = blueberry::taylor_shift(std::vector<Mint>{1, 2, 3}, Mint(2));
  assert((a == std::vector<Mint>{17, 14, 3}));
```
{% endraw %}

注意点: N <= p、p=Mint::mod() は素数。2N-1 以上の最小の2冪が p-1 を割ること。Mint は ACL static_modint。階乗と逆階乗を用いた1回のACL畳み込みで計算する。 入力範囲外は契約違反。法を超える整数としての個数は保持しない。

</details>

## 出典・検証

[公式問題](https://judge.yosupo.jp/problem/polynomial_taylor_shift)、[アルゴリズム比較資料](https://raw.githubusercontent.com/NyaanNyaan/library/master/fps/taylor-shift.hpp)を参照し、コードは独立に実装した。候補比較・追加資料・生ログは [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-math.md)。乱数seed付きの愚直比較は `tests/random/math_batch_three.cpp`。
