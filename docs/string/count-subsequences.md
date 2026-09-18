---
title: Count Subsequences
documentation_of: //blueberry/string/count-subsequences.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

数列や文字列の、空でない相異なる部分列を数えます。同じ値の並びは一度だけ数えます。
ACLにはない数え上げDPで、直前の同じ記号による重複を差し引きます。
入力長をN、異なる記号の数をKとし、時間O(N log(N+1))、追加メモリO(N+K)です。
添字をソートして座標圧縮するので、記号自体のコピーやハッシュは不要です。
`Sequence` は `size()` と定数時間の `operator[]` を持ち、記号の `<` は狭義弱順序を満たす必要があります。
順序で同値な記号を同一視します。比較とCountの演算を定数時間として計算量を示しています。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include <atcoder/modint>
#include "blueberry/string/count-subsequences.hpp"
int main() {
  using Mint = atcoder::modint998244353;
  assert(blueberry::count_subsequences<Mint>(std::string("aba")).val() == 6);
  assert(blueberry::count_subsequences<Mint>(std::string{}).val() == 0);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Count count_subsequences<Count>(const Sequence& sequence)` | O(N log(N+1)) | [開く](#count) |

<details class="api-operation" id="count" markdown="1">
<summary><code>Count count_subsequences&lt;Count&gt;(const Sequence&amp; sequence)</code> — O(N log(N+1))</summary>

入力の空でない相異なる部分列数を返します。空入力は0、同じ記号のみN個ならNです。
Countは0と1から構築でき、コピー代入、加算、減算が正確に行える型を指定します。
体や除算は不要なので、合成数法のmodintも使えます。

{% raw %}
```cpp
std::vector<int> a{2, 2, 2};
auto answer = blueberry::count_subsequences<long long>(a); // 3
```
{% endraw %}

注意点: 整数Countでは答えだけでなく中間値 `total + total` も表現範囲内である必要があります。
浮動小数点による近似は想定しません。通常はACLのmodintを使ってください。
入力を変更せず、参照も保持しません。戻り値は独立した値です。計算量は最悪時間で、償却ではありません。

</details>

## 出典・検証

- [Library Checker公式解法](https://github.com/yosupo06/library-checker-problems/blob/master/enumerative_combinatorics/number_of_subsequences/sol/correct.cpp) と [maspypyの数え上げDP](https://github.com/maspypy/library/blob/main/string/count_subsequence.hpp) の状態定義を調査し、座標圧縮と配列を使って独立に実装しました。
- [Number of Subsequences](https://judge.yosupo.jp/problem/number_of_subsequences) の専用verify、全部分集合からの重複除去を使うランダム比較、空・同一値・負値・小さい法で検証します。
- 比較測定の入力生成・候補実装は [benchmark/batch-three-subsequences.cpp](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-subsequences.cpp)、生ログは同ディレクトリの `batch-three-subsequences.log` に保存します。
- GCC 13.3 / gnu++20 / -O2 / N=500000、同一入力5回の中央値は、8記号で配列圧縮12.44ms・map 6.67ms、500000種類を生成範囲とする入力で47.18ms・184.81msでした。小アルファベットではmapが有利ですが、大きな入力での局所性とノード単位allocationの回避を優先して配列圧縮を採用しました。測定時の共有WSL環境の負荷も含むため、異環境への速度比は主張しません。
