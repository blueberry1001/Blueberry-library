---
title: 整数 k 乗根
documentation_of: //blueberry/math/kth-root-integer.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

`0 <= n <= UINT64_MAX`, `1 <= k <= INT_MAX`。厳密な floor(n^(1/k)) を整数演算だけで返す。ACLにはない演算。B = bit_width(n)、時間 O(k ceil(B/k))（k=1 と k>=64 は O(1)）、追加メモリ O(1)。割り算で積の上限を確認するためオーバーフローしない。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/kth-root-integer.hpp"
int main() {
  assert(blueberry::kth_root_integer(1000, 3) == 10);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::uint64_t kth_root_integer(std::uint64_t n, int k)` | O(k ceil(B/k)) | [開く](#root) |

<details class="api-operation" id="root" markdown="1">
<summary><code>std::uint64_t kth_root_integer(std::uint64_t n, int k)</code> — O(k ceil(B/k))</summary>

注意点:

n=0 は0、n=1 は1。k>=64 なら正の n の結果は1。k<=0 は禁止。返り値は値であり参照の寿命や無効化はない。

{% raw %}
```cpp
assert(blueberry::kth_root_integer(1000, 3) == 10);
```
{% endraw %}

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/kth_root_integer)。固定seed・境界・愚直比較は `tests/random/math-batch-two.cpp`。
複数実装の調査、独立実装した候補と同一環境測定は [math batch two]({{ '/docs/development/math-batch-two.html' | relative_url }}) を参照。
