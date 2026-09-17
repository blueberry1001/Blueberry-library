---
title: Prefix Function / KMP
documentation_of: //blueberry/string/prefix-function.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

接頭辞の最長 proper border と全一致位置を求める。ACL の Z algorithm と異なる情報を提供する。
Sequence/Text/Pattern は size() と O(1) 添字アクセスを提供し、要素 `==` は O(1) の同値関係。
各長さ ≤ INT_MAX。N は列/本文長、M はパターン長、K は一致数。入力を変更・保持せず返り値は独立した所有配列。
区切り文字不要で NUL や整数列も使用可能。値の算術演算を行わない。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include <vector>
#include "blueberry/string/prefix-function.hpp"
int main() {
  assert((blueberry::prefix_function(std::string("ababa")) == std::vector<int>{0, 0, 1, 2, 3}));
  assert((blueberry::kmp_search(std::string("ababa"), std::string("aba")) == std::vector<int>{0, 2}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<int> prefix_function(const Sequence& sequence)` | O(N) | [開く](#prefix) |
| `std::vector<int> kmp_search(const Text& text, const Pattern& pattern)` | O(N+M+1) | [開く](#search) |

<details class="api-operation" id="prefix" markdown="1">
<summary><code>std::vector&lt;int&gt; prefix_function(const Sequence&amp; sequence)</code> — O(N)</summary>

p[i] は sequence[0..i] の接頭辞と接尾辞が一致する最大長（全体を除く）。0 ≤ p[i] ≤ i。
空入力は空配列。メモリ O(N)。後退処理の合計が O(N) なので、文字ごとの処理は償却 O(1)。

{% raw %}
```cpp
auto p = blueberry::prefix_function(std::string("aaa"));
assert((p == std::vector<int>{0, 1, 2}));
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

<details class="api-operation" id="search" markdown="1">
<summary><code>std::vector&lt;int&gt; kmp_search(const Text&amp; text, const Pattern&amp; pattern)</code> — O(N+M+1)</summary>

一致開始位置を昇順で返し重複一致も含む。空パターンは N+1 境界全て。長すぎるパターンは空配列。
補助メモリ O(M)、出力 O(K)。後退は全体で線形だが個々の文字処理の最悪時間は定数とは限らない。

{% raw %}
```cpp
auto ids = blueberry::kmp_search(std::string("aaa"), std::string("aa"));
assert((ids == std::vector<int>{0, 1}));
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

## 出典・検証

[KACTL KMP](https://github.com/kth-competitive-programming/kactl/blob/main/content/strings/KMP.h)（CC0）を調査し区切り不要方式で独立実装。
公式 AOJ `ALDS1_14_B`（Library Checker ではない）と固定 seed の愚直比較、空文字列・重複・NUL を検証。
候補比較は `docs/development/sequence-batch-two.md`。
