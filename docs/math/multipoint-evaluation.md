---
title: Multipoint Evaluation
documentation_of: //blueberry/math/multipoint-evaluation.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

多項式の値を任意の点でまとめて求めます。ACLにはない積木・剰余木を既存FPSとACL畳み込みで実装しています。
`Mint` は素数modulusのACL `static_modint`。係数は次数昇順です。
`N=f.size()`、`M=x.size()`、`K=N+M+1` として、最悪時間 O(K log² K)、追加メモリ O(N+M log(M+1))。
`N,M < mod` かつ `4*bit_ceil(K)` 以下の2冪変換長をmodulusがサポートする範囲を十分条件とします。
サイズ・内部添字は `int` に収まる必要があります。998244353を想定します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/multipoint-evaluation.hpp"
int main() {
  using mint = atcoder::modint998244353;
  std::vector<mint> f{1, 2, 3}, x{0, 1, 2, 1};
  auto y = blueberry::multipoint_evaluation(f, x);
  assert((y == std::vector<mint>{1, 6, 17, 6}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<Mint> multipoint_evaluation(const vector<Mint>& f, const vector<Mint>& x)` | O(K log² K) | [開く](#evaluate) |

<details class="api-operation" id="evaluate" markdown="1">
<summary><code>vector&lt;Mint&gt; multipoint_evaluation(f, x)</code> — O(K log² K)</summary>

返り値の `i` 番目は `f(x[i])`。点の順序を保持し、重複点も許します。
空の係数列は零多項式、空の点列は空の結果です。末尾の零係数も許します。

{% raw %}
```cpp
std::vector<mint> zero;
auto y = blueberry::multipoint_evaluation(zero, std::vector<mint>{5});
assert(y[0] == mint(0));
```
{% endraw %}

注意点: 入力を変更せず、返り値は独立した所有値です。参照失効はありません。
modint内で演算するため係数の整数オーバーフローはありません。計算量は償却ではなく最悪値です。
`polynomial_detail` は内部実装であり公開APIではありません。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/multipoint_evaluation) に対応。
[Nyaan](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html) の小次数で直接評価する設計と
[Luzhiled](https://ei1333.github.io/library/math/fps/multipoint-evaluation.hpp.html) の剰余木を調査し、数式から独立に実装しました。
外部コードの転記はありません。固定seedのHorner法比較も実施します。
候補比較と測定条件は [追加時の調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/math-batch-four.md) を参照してください。
