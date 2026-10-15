---
title: Prefix Function / KMP
documentation_of: //blueberry/string/prefix-function.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要

接頭辞の最長proper borderと、パターンの全一致位置を求めます。要素の `==` が同値関係である列に対応し、区切り文字なしでNULや整数列も使えます。

N=列/本文長、M=パターン長、K=一致数です。接頭辞関数はO(N)時間・メモリ、KMP検索はO(N+M+1)時間・出力を含む追加メモリO(M+K)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`prefix_function`](#prefix) | O(N) |
| [`kmp_search`](#search) | O(N+M+1) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLのZ algorithmとは異なる情報を提供します。Sequence/Text/Patternは `size()` とO(1)の添字アクセスを持ち、要素の `==` はO(1)とします。
- 各長さはINT_MAX以下です。入力を変更・保持せず、返り値は独立した所有配列です。値の算術演算は行いません。

</details>

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

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [KACTL KMP](https://github.com/kth-competitive-programming/kactl/blob/main/content/strings/KMP.h)（CC0）を調査し区切り不要方式で独立実装。
  公式 AOJ `ALDS1_14_B`（Library Checker ではない）と固定 seed の愚直比較、空文字列・重複・NUL を検証。
  候補比較は `docs/development/sequence-batch-two.md`。

</details>
