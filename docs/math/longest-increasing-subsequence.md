---
title: Longest Increasing Subsequence
documentation_of: //blueberry/math/longest-increasing-subsequence.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

厳密増加部分列の最長解を添字列で復元する。ACL にない列アルゴリズム。
N は要素数で N ≤ INT_MAX。T の `<` は strict weak ordering、比較 O(1)。時間 O(N log(N+1))、メモリ O(N)。
値の番兵・算術演算を使わず整数最小・最大値も扱える。入力を変更・保持せず返り値は独立した所有配列。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/math/longest-increasing-subsequence.hpp"
int main() {
  auto ids = blueberry::longest_increasing_subsequence(std::vector<int>{3, 1, 2, 4});
  assert((ids == std::vector<int>{1, 2, 3}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<int> longest_increasing_subsequence(const std::vector<T>& values)` | O(N log(N+1)) | [開く](#lis) |

<details class="api-operation" id="lis" markdown="1">
<summary><code>std::vector&lt;int&gt; longest_increasing_subsequence(const std::vector&lt;T&gt;&amp; values)</code> — O(N log(N+1))</summary>

添字は昇順で対応する値は厳密増加。同値は延長しない。複数解がある場合はその一つであり辞書順最小の保証はない。
空入力には空配列を返す。計算量は最悪であり償却ではない。

{% raw %}
```cpp
auto ids = blueberry::longest_increasing_subsequence(std::vector<int>{2, 2});
assert(ids.size() == 1);
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

## 出典・検証

[Luzhiled](https://ei1333.github.io/library/dp/longest-increasing-subsequence.hpp.html) と
[sotanishy](https://sotanishy.github.io/cp-library-cpp/dp/lis.hpp) の二分探索・復元方式を調査し独立実装。
公式 Library Checker `longest_increasing_subsequence`、固定 seed の O(N²) DP・復元チェック。
候補比較は `docs/development/sequence-batch-two.md`。
