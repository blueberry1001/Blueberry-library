---
title: Z Algorithm
documentation_of: //blueberry/string/z-algorithm.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要

列sの各位置から始まる接尾辞と、s全体の最長共通接頭辞長を求めます。要素の等値比較が可能な列に対応し、空列では空vectorを返します。

長さNに対して、時間・メモリともにO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`z_algorithm`](#z-algorithm) | O(N) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- Sequenceは `size()` と添字アクセス、要素の等値比較を提供する型です。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include "blueberry/string/z-algorithm.hpp"
int main() {
  const std::string s = "ababa";
  auto z = blueberry::z_algorithm(s);
  assert(z == std::vector<int>({5, 0, 3, 0, 1}));
}
```
{% endraw %}

<details class="api-operation" id="z-algorithm" markdown="1">
<summary><code>vector&lt;int&gt; z_algorithm(sequence)</code> — O(N)</summary>

`result[i]` は `sequence[i..]` と `sequence[0..]` の最長共通接頭辞長です。`result[0]` は列全体の長さです。

{% raw %}
```cpp
auto z = blueberry::z_algorithm(std::string("aabaaab"));
int match = z[3];
```
{% endraw %}

注意点: 空列の返り値は空vectorです。要素比較が副作用を持たず、列が計算中に変更されないようにしてください。

</details>
