---
title: Linear Recurrence
documentation_of: //blueberry/math/linear-recurrence.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

有限の数列から最小次数の線形漸化式を見つけるBerlekamp–Masseyと、
与えた漸化式の $k$ 番目を求めるBostan–Moriです。添字は0から始まり、係数順は
`a[i] = c[0]*a[i-1] + c[1]*a[i-2] + ... + c[d-1]*a[i-d]` に統一します。
ACLのmodintと畳み込みを利用し、ACLにない級数の係数抽出・漸化式推定だけを実装しています。

BMの係数型は厳密な体として加減乗除・零比較が可能な型です。ACLの素数modintを推奨します。
入力長を $N$ として時間 $O(N^2)$、追加メモリ $O(N)$。
k項目の計算にはACLの `static_modint` と素数modulusが必要です。
次数を $d$、畳み込み時間を $M(d)=O(d\log d)$ として
時間 $O(M(d)\log(k+1))$、追加メモリ $O(d)$。

`k >= d > 0` では `bit_ceil(2*d+1)` が `Mint::mod()-1` を割り切る必要があります。
`modint998244353` なら `d <= 4,194,303`。`k < d` は初期値を $O(1)$ で返すため、
このNTT長制約は適用しません。すべての入力vectorは変更せず、返却値がデータを所有します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/linear-recurrence.hpp"
int main() {
  using mint = atcoder::modint998244353;
  std::vector<mint> prefix{0, 1, 1, 2, 3, 5, 8, 13};
  auto coefficients = blueberry::berlekamp_massey(prefix);
  assert((coefficients == std::vector<mint>{1, 1}));
  std::vector<mint> initial{0, 1};
  assert(blueberry::linear_recurrence_kth(initial, coefficients, 10) == mint(55));
  assert(blueberry::berlekamp_massey(std::vector<mint>{}).empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<Mint> berlekamp_massey(const vector<Mint>& sequence)` | O(N²) | [開く](#berlekamp-massey) |
| `Mint linear_recurrence_kth(const vector<Mint>& initial, const vector<Mint>& coefficients, uint64_t k)` | O(M(d) log(k+1))、k&lt;dはO(1) | [開く](#kth) |

<details class="api-operation" id="berlekamp-massey" markdown="1">
<summary><code>vector&lt;Mint&gt; berlekamp_massey(sequence)</code> — O(N²)</summary>

`sequence[d..N)` に適合する最小次数 $d$ の漸化式を返します。複数の最短解がある場合は1つを返します。
空列・全0列の結果は空です。先頭に1を付けた消去多項式ではなく、上記の漸化式係数を返します。

{% raw %}
```cpp
using mint = atcoder::modint998244353;
auto c = blueberry::berlekamp_massey(std::vector<mint>{2, 6, 18, 54});
assert((c == std::vector<mint>{3}));
```
{% endraw %}

注意点: 有限prefixに適合する式を求めるだけで、未観測の項を保証するものではありません。
真の次数が $d$ 以下と分かる列の復元には通常 $2d$ 項以上を与えます。
`sequence.size() < INT_MAX` が前提です。浮動小数の近似比較や合成数modintは対象外です。
末尾0を含む係数も次数に意味があるため、勝手に削除しないでください。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>Mint linear_recurrence_kth(initial, coefficients, uint64_t k)</code> — O(M(d) log(k+1))</summary>

`initial.size() == coefficients.size() == d` が前提です。最初の $d$ 項を初期値、
$i\ge d$ に上記の漸化式を適用した列の $a_k$ を返します。
両vectorが空なら0、`k < d` なら初期値を返します。`k` はuint64全域を扱います。

{% raw %}
```cpp
using mint = atcoder::modint998244353;
std::vector<mint> initial{3, 9, 7}, coefficients{1, 0, 0};
assert(blueberry::linear_recurrence_kth(initial, coefficients, 1'000'000'000'000ULL) == mint(7));
```
{% endraw %}

注意点: 長さ不一致はassertの対象です。末尾係数0でも初期値を省略できません。
`k >= d > 0` では上記NTT長条件と `d < INT_MAX/2` が前提で、assertでも検査します。
`Mint` がdynamic_modintの場合はコンパイルできません。負のindexはunsigned変換されるので渡さないでください。

</details>

## 出典・検証

[emthrmのBM](https://github.com/emthrm/cp-library/blob/master/include/emthrm/math/formal_power_series/berlekamp-massey.hpp)、
[NyaanのBM](https://github.com/NyaanNyaan/library/blob/master/fps/berlekamp-massey.hpp)、
[Bostan–Mori](https://github.com/emthrm/cp-library/blob/master/include/emthrm/math/formal_power_series/bostan-mori.hpp) を調査し、
係数の順序・符号を統一して独立実装しました。外部コードは転記していません。
Fastestのhalf-GCDによる高速BMや専用SIMD NTTは、実装規模と移植性を考えて今回採用していません。

公式問題: [Find Linear Recurrence](https://judge.yosupo.jp/problem/find_linear_recurrence)、
[Kth term](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence)。
追加テストはGaussian eliminationで最小次数を確認し、直接DP・独立したKitamasa法でk項目を照合します。
