---
title: Longest Increasing Subsequence
documentation_of: //blueberry/math/longest-increasing-subsequence.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

厳密増加部分列の最長解を添字列で復元します。Tの `<` は狭義弱順序を満たす必要があります。

Nを要素数として時間O(N log(N+1))、メモリO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`longest_increasing_subsequence`](#lis) | O(N log(N+1)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLにはない列アルゴリズムです。`N <= INT_MAX`、比較はO(1)とします。
- 値の番兵・算術演算を使わず、整数の最小・最大値も扱えます。
- 入力を変更・保持せず、返り値は独立した所有配列です。

</details>

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

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [Luzhiled](https://ei1333.github.io/library/dp/longest-increasing-subsequence.hpp.html) と
  [sotanishy](https://sotanishy.github.io/cp-library-cpp/dp/lis.hpp) の二分探索・復元方式を調査し独立実装。
  公式 Library Checker `longest_increasing_subsequence`、固定 seed の O(N²) DP・復元チェック。
  候補比較は `docs/development/sequence-batch-two.md`。

</details>
