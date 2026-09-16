---
title: Manacher
documentation_of: //blueberry/string/manacher.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

文字と文字間を中心とする最長回文の**長さ**をまとめて求める。
ACL にない回文列挙を補い、最長回文の取得や静的な部分列の回文判定に使える。
`N = sequence.size()` として最悪 O(N) 時間、返り値を含め O(N) メモリ。
返り値以外の作業メモリは O(1) で、区切り文字を挿入した配列は作らない。

入力は `std::string`、`std::vector<T>` など、`size()` と整数添字による
読み取りを持つ列。長さは `0 <= N <= INT_MAX / 2` とし、`size()`、添字参照、
要素同士の `==` が O(1) であることを計算量の前提とする。
`==` は安定した同値関係でなければならない。順序比較・数値変換・予約値は不要。
比較が O(C) なら全体は O(N C)。入力の内容は変更しない。
文字列ではバイト単位で比較するため、UTF-8 のコードポイント単位の回文判定には
事前の変換が必要。空入力では空の `std::vector<int>` を返す。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include <vector>
#include "blueberry/string/manacher.hpp"

int main() {
  const auto p = blueberry::manacher(std::string("abba"));
  assert((p == std::vector<int>{1, 0, 1, 4, 1, 0, 1}));
  const int l = 1, r = 3;
  assert(p[l + r - 1] >= r - l);  // "bb" is a palindrome.
  assert(blueberry::manacher(std::string{}).empty());
  const auto q = blueberry::manacher(std::vector<int>{-1, 7, -1});
  assert(q[2] == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<int> p = blueberry::manacher(const Sequence& sequence)` | 最悪 O(N) 時間、O(N) メモリ | [開く](#manacher) |

<details class="api-operation" id="manacher" markdown="1">
<summary><code>std::vector&lt;int&gt; blueberry::manacher(const Sequence&amp; sequence)</code> — 最悪 O(N)</summary>

`Sequence` はテンプレート引数として推論される。N が正なら長さ `2*N-1` の配列を返す。
`p[2*i]` は `sequence[i]` を中心とする奇数長回文の最大長で、少なくとも 1。
`p[2*i+1]` は `sequence[i]` と `sequence[i+1]` の間を中心とする偶数長回文の最大長で、
回文がなければ 0。偶数長中心は `0 <= i < N-1` に対応する。
Library Checker の Enumerate Palindromes と同じ順序・値をそのまま返す。

中心添字 `c` の返り値を `d = p[c]` とすると、その最長回文は半開区間
`[(c-d+1)/2, (c+d+1)/2)`。`d == 0` なら空区間である。
入力区間 `[l,r)`（`0 <= l <= r <= N`）の回文判定は、空区間なら true、
それ以外なら `p[l+r-1] >= r-l` を O(1) で調べればよい。

{% raw %}
```cpp
const auto p = blueberry::manacher(std::string("abacaba"));
const int center = 6;
const int length = p[center];                  // 7
const int left = (center - length + 1) / 2;   // 0
const int right = (center + length + 1) / 2;  // 7
assert(left == 0 && right == 7);
long long count = 0;
for (int x : p) count += (x + 1) / 2;         // Nonempty palindrome occurrences.
assert(count == 12);
```
{% endraw %}

注意点: 半径を返す他の Manacher 実装とは値の定義が異なる。
文字列リテラルを直接渡さず、`std::string` や `std::string_view` で包む。
N の上限により内部の `2*N` と各長さが `int` に収まる。
回文の個数は最大 `N*(N+1)/2` なので、集計には `long long` などを使う。
返り値は独立した所有配列で、呼び出し終了後に入力を変更・破棄しても影響しない。
入力の参照・イテレータを無効化せず、返り値の参照の寿命と無効化規則は通常の
`std::vector` と同じ。O(N) は 1 回の呼び出し全体の最悪計算量で、償却クエリ API ではない。

</details>

## 出典・検証

設計調査として [Nyaan の Manacher](https://nyaannyaan.github.io/library/string/manacher.hpp)
と [KACTL の Manacher](https://github.com/kth-competitive-programming/kactl/blob/main/content/strings/Manacher.h)
を比較した。Nyaan は奇数半径から区切り要素付き列へ拡張し、KACTL は偶奇の半径を
2 配列に分けて計算する。どちらも線形時間・線形メモリ。
本実装は回文の対称性から独立に実装し、偶奇 2 回の走査で最終的な長さ配列へ直接書き込む。
入力要素を複製せず、汎用列にも予約値を要求しない。参照先のコードの転載は行っていない。

- 公式問題: [Enumerate Palindromes](https://judge.yosupo.jp/problem/enumerate_palindromes)
- verify: `verify/string/enumerate-palindromes.test.cpp`
- 境界・ランダム比較: `tests/random/manacher.cpp`。空、1 文字、偶奇回文、同一文字、交互列、
  NUL/高位バイト、負整数、等値比較だけを持つ型、長さ 10 以下の全二値列、
  seed 指定のランダム列を部分区間全列挙と比較。20 万文字の同一文字列も確認する。
- 再測定用コード: `benchmark/manacher_benchmark.cpp`
- 測定ログ: `benchmark/results/manacher-wsl-gcc13.txt`。独立に書いた区切り配列版と愚直な
  中心展開版を同一入力・同一バイナリで比較し、結果配列の一致も確認する。

WSL2 / Intel Core i7-14650HX / GCC 13.3.0 / `-std=c++20 -O2 -DNDEBUG -I.`、
seed 20260914、ウォームアップ 1 回・計測 7 回。100 万要素での中央値は次のとおり。
メモリは確保する配列要素数に基づく概算で、RSS の実測ではない。

| 入力 | 本実装 | 区切り配列版 |
| --- | --- | --- |
| 26 種のランダム列 | 3.461 ms | 8.207 ms |
| 全要素同一 | 3.902 ms | 7.757 ms |
| 2 種の交互列 | 3.382 ms | 9.128 ms |
| 作業・結果配列の合計 | 約 `8*N` bytes | 約 `24*N` bytes |

6000 要素のランダム列では愚直版も 0.016 ms と速いが、全要素同一では 8.448 ms
（本実装 0.022 ms）となる。入力依存で二乗時間になるため採用しない。
小さな測定差の一般化は避け、最悪 O(N)、区切り配列不要、短い呼び出し方を採用理由とする。
測定順序・共有マシンの負荷による影響は残る。ログに各実行値と最小値も保持している。

再現コマンド（リポジトリルートで実行）:

{% raw %}
```sh
g++ -std=c++20 -O2 -DNDEBUG -I. benchmark/manacher_benchmark.cpp -o /tmp/manacher-benchmark
/tmp/manacher-benchmark
```
{% endraw %}
