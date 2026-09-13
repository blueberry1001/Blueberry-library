---
title: Z Algorithm
documentation_of: //blueberry/string/z-algorithm.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

列 `s` の各位置から始まる接尾辞と `s` 全体の最長共通接頭辞長を求めます。長さを $N$ とすると
時間・メモリは $O(N)$ です。`Sequence` は `size()` と添字アクセス、要素の等値比較を提供する
必要があります。空列にも対応します。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> z_algorithm(sequence)` | O(N) | [開く](#z-algorithm) |

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
