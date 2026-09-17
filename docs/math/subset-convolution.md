---
title: Subset Convolution
documentation_of: //blueberry/math/subset-convolution.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

集合の要素数ごとの zeta 変換（ranked transform）で、`c[S] = Σ_{A⊆S} a[A] b[S\A]` を計算します。添字は集合の bit mask です。AND/OR 畳み込みと異なり、積に使う二集合の共通部分は空です。ACL の通常の多項式 convolution は再実装しません。

入力は同長 N、N=0 または N=2^K。自動パディングなし。T は可換環で、`T{}` はゼロ、コピー・`+=`・`-=`・`*` が必要です。除算を使わないため、整数型、ACL modint、標数2の体でも利用できます。中間値を含む全演算のオーバーフローを呼出側で防いでください。

最悪時間 O(N(1+K)²)、追加メモリ O(N(1+K)+1)。係数演算・コピーは O(1) とします。2本の連続した rank 配列を使い、各 mask での係数積を高い次数から上書きするため第3の rank 配列は不要です。zeta では `rank <= popcount(mask)`、逆変換では最終対角に寄与する次数だけを処理します。

入力は const 参照で変更せず、返り値は独立した vector です。N(K+1) が size_t に収まり、vector が確保可能なサイズであることが前提です。空同士は空、N=1 は要素積を返します。内部参照や格納領域を公開しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/subset-convolution.hpp"
int main() {
  using Mint = atcoder::modint998244353;
  std::vector<Mint> a{1, 2, 3, 4}, b{5, 6, 7, 8};
  auto c = blueberry::subset_convolution(a, b);
  assert(c[0] == 5 && c[3] == 60);
  assert(a[0] == 1 && b[0] == 5);
  assert(blueberry::subset_convolution<Mint>({}, {}).empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<T> subset_convolution(const vector<T>& a, const vector<T>& b)` | O(N(1+K)²) | [開く](#convolution) |

<details class="api-operation" id="convolution" markdown="1">
<summary><code>vector&lt;T&gt; subset_convolution(const vector&lt;T&gt;&amp; a, const vector&lt;T&gt;&amp; b)</code> — O(N(1+K)²)</summary>

集合 S の各分割 A と S\A に対する積の和を、全 mask 分まとめて返します。

{% raw %}
```cpp
auto c = blueberry::subset_convolution(a, b);
```
{% endraw %}

注意点: a.size()==b.size()、空または2冪長が必要。重なりを許す OR 畳み込みではありません。入力を破壊しません。
</details>

## 出典・検証

- [Björklund, Husfeldt, Kaski, Koivisto: Fourier meets Möbius (2006)](https://arxiv.org/abs/cs/0611101): ranked zeta/Möbius による O(K²2^K) 部分集合畳み込みの原論文。
- [Library Checker Subset Convolution](https://judge.yosupo.jp/problem/subset_convolution): `verify/math/subset-convolution.test.cpp`。
- [Fastest 352953](https://judge.yosupo.jp/submission/352953)、[339759](https://judge.yosupo.jp/submission/339759): SIMD と専用剰余演算を用いる実装を調査。命令セット依存の公開コードは転記せず、連続配列の汎用実装を採用しました。公開実行時間とローカル時間は直接比較しません。
- `tests/random/algebra-expansion.cpp`: 全分割の独立 oracle、空・長さ1を検証。
