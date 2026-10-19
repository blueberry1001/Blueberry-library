---
title: Sparse FPS Exp
documentation_of: //blueberry/math/sparse-exp.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

疎な形式的冪級数fから、exp(f)の先頭n項を求めます。非零項が少ない母関数の指数関数を計算するときに使います。係数型Mintには素数法のACL `static_modint`を使い、fの定数項は0とします。NTTの変換長に関する制約はありません。

N=n、Kを入力の項数、Sを同じ次数の係数を合算した後の次数1以上N未満の非零項数とします。最悪時間O(K log(K+1)+N(S+1))、返り値を含む追加メモリO(N+K)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`g = sparse_exp<Mint>(terms, n)`](#compute) | 最悪O(K log(K+1)+N(S+1)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- p=`Mint::mod()`は素数で、`0 <= n < p`を満たす必要があります。法2もn=0,1で利用できます。配列が利用可能なメモリに収まる必要があります。
- 入力は`std::vector<std::pair<int, Mint>>`です。各要素`{degree, coefficient}`は`coefficient * x^degree`を表し、すべてのdegreeは非負とします。順序は任意で、同じ次数の項や係数0の項も渡せます。
- 同じ次数の係数は法pで合算します。定数項の合計は0でなければなりません。n=0でも全入力の次数と定数項の条件を満たす必要があります。
- 次数n以上の項は結果に影響しません。次数の条件を確認した上で計算対象から除きます。入力ベクトルは変更しません。
- 係数は法pの値です。整数や有理数としての値を保持するものではありません。整数の逆元を作る漸化式は64ビット整数を使い、中間積の整数オーバーフローを避けます。
- 返り値は係数を所有する`FPS<Mint>`です。入力への参照や共有キャッシュを保持しません。返した係数への参照の寿命・無効化は`std::vector`に従います。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/sparse-exp.hpp"
using Mint = atcoder::modint998244353;
int main() {
  std::vector<std::pair<int, Mint>> terms{{1, 1}};
  auto g = blueberry::sparse_exp<Mint>(terms, 4);
  assert(g.size() == 4);
  assert(g[0] == Mint(1) && g[1] == Mint(1));
  assert(g[2] == Mint(2).inv() && g[3] == Mint(6).inv());
  assert(blueberry::sparse_exp<Mint>(terms, 0).empty());
  auto one = blueberry::sparse_exp<Mint>({}, 3);
  assert((one == std::vector<Mint>{1, 0, 0}));
}
```
{% endraw %}

<details class="api-operation" id="compute" markdown="1">
<summary><code>FPS&lt;Mint&gt; sparse_exp&lt;Mint&gt;(const std::vector&lt;std::pair&lt;int, Mint&gt;&gt;&amp; terms, int n)</code> — 最悪O(K log(K+1)+N(S+1))</summary>

`terms`が表す級数fについて、`exp(f) mod x^n`を長さちょうどnの`FPS<Mint>`で返します。nは項数で、最後に求める次数はn-1です。n=0なら空列を返します。n>0なら先頭の係数は1です。入力が空、または合算後の係数がすべて0なら`{1, 0, ..., 0}`を返します。

{% raw %}
```cpp
// Mintは最小使用例の型です。重複・順序・相殺する定数項を許します。
std::vector<std::pair<int, Mint>> terms{{1, 2}, {0, 3}, {1, -1}, {0, -3}};
auto g = blueberry::sparse_exp<Mint>(terms, 3);
assert(g[0] == Mint(1) && g[1] == Mint(1) && g[2] == Mint(2).inv());
```
{% endraw %}

注意点: 全入力の次数が非負で、定数項の合計が0であることが前提です。この条件はn=0や次数n以上の項にも適用されます。負のn、n>=p、負の次数、非零の定数項は契約違反です。計算量は償却ではありません。Sが大きい場合は、既存の`FPS<Mint>::exp(n)`による密な級数の計算も候補になります。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- g=exp(f)を形式微分した恒等式`g' = f'g`から、`g[0]=1`、`g[i] = (sum_{1 <= d <= i} d * f[d] * g[i-d]) / i`を使います。i=1..n-1の逆元はまとめて前計算します。
- [Library Checker公式問題: Exp of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse)。疎な入力から求めた全係数を検証する問題です。
- ACLの係数型を利用します。ACLには疎な形式的冪級数の指数関数を求めるAPIはありません。
- [調査・性能比較・検証記録]({{ '/docs/development/sparse-exp.html' | relative_url }})

</details>
