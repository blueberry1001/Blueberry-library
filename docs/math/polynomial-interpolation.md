---
title: Polynomial Interpolation
documentation_of: //blueberry/math/polynomial-interpolation.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

異なる N 点を通る次数 N 未満の多項式を復元します。ACLにはない補間演算をACL畳み込みと既存FPSで実装します。
`Mint` は素数modulusのACL `static_modint`、`x` はmodulus上で相異なり、`x.size()==y.size()` が必須です。
時間 O(N log²(N+1)+N log mod)、追加メモリ O(N log(N+1))。逆元の計算を明示した上限です。
`N < mod`、内部添字が `int` に収まり、`4*bit_ceil(N+1)` 以下の2冪NTT長をサポートする範囲を十分条件とします。
998244353を想定します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/polynomial-interpolation.hpp"
int main() {
  using mint = atcoder::modint998244353;
  std::vector<mint> x{0, 1, 2}, y{1, 6, 17};
  auto f = blueberry::polynomial_interpolation(x, y);
  assert((f == std::vector<mint>{1, 2, 3}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<Mint> polynomial_interpolation(const vector<Mint>& x, const vector<Mint>& y)` | O(N log²(N+1)+N log mod) | [開く](#interpolate) |

<details class="api-operation" id="interpolate" markdown="1">
<summary><code>vector&lt;Mint&gt; polynomial_interpolation(x, y)</code> — O(N log²(N+1)+N log mod)</summary>

`f(x[i]) == y[i]` を満たす係数を次数昇順でちょうど N 個返します。高次の零係数を削除しません。
空入力には空列、1点には定数を返します。重複点は同じ値でも禁止です。

{% raw %}
```cpp
auto f = blueberry::polynomial_interpolation(std::vector<mint>{7}, std::vector<mint>{42});
assert(f.size() == 1 && f[0] == mint(42));
```
{% endraw %}

注意点: 入力を変更せず、返り値は独立した所有値です。参照失効はありません。
modintにより係数の整数オーバーフローはありません。計算量は最悪値です。
相異性違反は積多項式の導関数の零値でassert検出します。assert無効時にも前提は必須です。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/polynomial_interpolation) に対応。
[Luzhiled](https://ei1333.github.io/library/math/fps/polynomial-interpolation.hpp.html) の積木による合成と
[Nyaan](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html) の積木・多点評価設計を調査。
Lagrange補間の数式から独立に実装し、コードの転記はありません。
ランダム係数から点列を生成して元の係数と比較します。
[追加時の調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/math-batch-four.md) に候補比較を記載します。
