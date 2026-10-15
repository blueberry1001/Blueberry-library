---
title: 商列挙
documentation_of: //blueberry/math/enumerate-quotients.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

非負整数nについてfloor(n/i), 1<=i<=nの相異なる正の値を昇順に列挙します。

時間・返り値込みのメモリはO(sqrt(n))です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`enumerate_quotients`](#enumerate) | O(sqrt(n)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `0 <= n <= INT64_MAX`。ACLにはない演算です。
- 計算量はvectorの拡張を含む総時間です。
- 巨大なnでは結果自体が数十億要素になり得るので、利用できるメモリを確認してください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/enumerate-quotients.hpp"
int main() {
  assert((blueberry::enumerate_quotients(5) == std::vector<std::int64_t>{1, 2, 5}));
}
```
{% endraw %}

<details class="api-operation" id="enumerate" markdown="1">
<summary><code>std::vector&lt;std::int64_t&gt; enumerate_quotients(std::int64_t n)</code> — O(sqrt(n))</summary>

注意点:

n=0 は空vector。積や n+1 を計算しないため INT64_MAX でも整数オーバーフローはない。返されたvectorは独立して所有される。

{% raw %}
```cpp
assert((blueberry::enumerate_quotients(5) == std::vector<std::int64_t>{1, 2, 5}));
```
{% endraw %}

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [Library Checker](https://judge.yosupo.jp/problem/enumerate_quotients)。固定seed・境界・愚直比較は `tests/random/math-batch-two.cpp`。
  複数実装の調査、独立実装した候補と同一環境測定は [math batch two]({{ '/docs/development/math-batch-two.html' | relative_url }}) を参照。

</details>
