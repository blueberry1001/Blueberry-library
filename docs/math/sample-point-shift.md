---
title: Sample Point Shift
documentation_of: //blueberry/math/sample-point-shift.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

次数 N 未満の多項式の `y[i]=f(i)` から `f(start),...,f(start+count-1)` を求めます。
ACLにはない連続点補間をACL畳み込みで実装します。
`Mint` は素数modulusのACL `static_modint`、N=`y.size()`、M=`count`、K=N+M+1 として
時間 O(K log K+log mod)、追加メモリ O(K)。計算量は最悪値で、償却ではありません。
`0 <= N,M < mod`、内部サイズ `2*N+M` がintに収まり、`bit_ceil(2*N+M)` のNTT長をサポートすることが前提です。
998244353を想定します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/sample-point-shift.hpp"
int main() {
  using mint = atcoder::modint998244353;
  std::vector<mint> y{0, 1, 4};
  auto shifted = blueberry::sample_point_shift(y, mint(3), 3);
  assert((shifted == std::vector<mint>{9, 16, 25}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<Mint> sample_point_shift(const vector<Mint>& y, Mint start, int count)` | O(K log K+log mod) | [開く](#shift) |

<details class="api-operation" id="shift" markdown="1">
<summary><code>vector&lt;Mint&gt; sample_point_shift(y, start, count)</code> — O(K log K+log mod)</summary>

ちょうど `count` 個の値を返します。開始点が既存の標本点と重なる場合も、modulusを跨ぐ場合も有効です。
空入力は零多項式とみなし、`count` 個の零を返します。`count==0` は空の結果です。

{% raw %}
```cpp
auto shifted = blueberry::sample_point_shift(std::vector<mint>{0, 1, 4}, mint(-1), 3);
assert((shifted == std::vector<mint>{1, 0, 1}));
```
{% endraw %}

注意点: 連続点の値を渡します。係数列を渡す操作ではありません。
入力は変更せず、返り値は独立した所有値です。参照失効はありません。
係数演算はmodintで行い整数オーバーフローを避けます。サイズ上限は上記の前提に従ってください。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/shift_of_sampling_points_of_polynomial) に対応。
[Luzhiled](https://ei1333.github.io/library/math/combinatorics/sample-point-shift.hpp) のLagrange重みと畳み込み、
[Nyaan](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html) の一般点評価を候補として調査しました。
数式から独立に実装しコードの転記はありません。Horner法、既存点との重複、modulus跨ぎをランダム比較します。
[追加時の調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/math-batch-four.md) に測定条件を記載します。
