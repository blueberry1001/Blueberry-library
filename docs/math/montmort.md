---
title: Montmort Numbers
documentation_of: //blueberry/math/montmort.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

完全順列数D(0),...,D(n)を法modで返す。D(0)=1、D(1)=0。n=0も1要素返す。mod=1はすべて0。ACLにない任意法の数列生成で、除算や逆元を使わない。

0 <= n <= INT_MAX、1 <= mod <= UINT32_MAX。法は合成数でもよい。64bit符号なし中間積でこの全範囲のオーバーフローを防ぐ。D(i)=iD(i-1)+(-1)^iを利用する。 メモリは O(n+1)。以下は最悪計算量（償却ではない）。返り値は独立した所有ベクトルで、既存の参照を無効化しない。法演算を除く長さは利用可能なメモリに収まる必要がある。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/montmort.hpp"
int main() {
  auto a = blueberry::montmort(5, 100);
  assert((a == std::vector<std::uint32_t>{1, 0, 1, 2, 9, 44}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<std::uint32_t> montmort(int n, std::uint32_t mod)` | O(n+1) | [開く](#compute) |

<details class="api-operation" id="compute" markdown="1">
<summary><code>std::vector&lt;std::uint32_t&gt; montmort(int n, std::uint32_t mod)</code> — O(n+1)</summary>

完全順列数D(0),...,D(n)を法modで返す。D(0)=1、D(1)=0。n=0も1要素返す。mod=1はすべて0。ACLにない任意法の数列生成で、除算や逆元を使わない。

{% raw %}
```cpp
// Mint は最小使用例の型（必要な場合）。
auto a = blueberry::montmort(5, 100);
  assert((a == std::vector<std::uint32_t>{1, 0, 1, 2, 9, 44}));
```
{% endraw %}

注意点: 0 <= n <= INT_MAX、1 <= mod <= UINT32_MAX。法は合成数でもよい。64bit符号なし中間積でこの全範囲のオーバーフローを防ぐ。D(i)=iD(i-1)+(-1)^iを利用する。 入力範囲外は契約違反。法を超える整数としての個数は保持しない。

</details>

## 出典・検証

[公式問題](https://judge.yosupo.jp/problem/montmort_number_mod)、[アルゴリズム比較資料](https://raw.githubusercontent.com/ei1333/library/master/math/combinatorics/montmort.hpp)を参照し、コードは独立に実装した。候補比較・追加資料・生ログは [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-math.md)。乱数seed付きの愚直比較は `tests/random/math_batch_three.cpp`。
