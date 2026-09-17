---
title: 離散対数
documentation_of: //blueberry/math/discrete-log.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

`1 <= m <= INT32_MAX`。任意の int64_t の a,b を正規化し、a^x = b (mod m) を満たす最小の非負整数 x を返す。解なしは std::nullopt。合成数法・非互いに素の底にも対応する。0^0=1 とする。ACLの inv_mod を再利用し、ACLにない gcd 除去と baby-step giant-step を追加する。ハッシュの期待時間 O(sqrt(m)+log(m))、最悪 O(m+log(m))、追加メモリ O(sqrt(m))。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/discrete-log.hpp"
int main() {
  assert(blueberry::discrete_log(2, 4, 8).value() == 2);
assert(!blueberry::discrete_log(2, 3, 8));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::optional<std::int64_t> discrete_log(std::int64_t a, std::int64_t b, std::int64_t m)` | 期待 O(sqrt(m)+log(m)) | [開く](#log) |

<details class="api-operation" id="log" markdown="1">
<summary><code>std::optional&lt;std::int64_t&gt; discrete_log(std::int64_t a, std::int64_t b, std::int64_t m)</code> — 期待 O(sqrt(m)+log(m))</summary>

注意点:

m=1 は常に0。a=0,b=0,m>1 は1。b=1 (mod m) は0。中間積は INT32_MAX^2 以下で int64_t に収まる。逆元は互いに素へ縮約した後にだけ求める。返り値は独立した値であり入力を変更しない。

{% raw %}
```cpp
assert(blueberry::discrete_log(2, 4, 8).value() == 2);
assert(!blueberry::discrete_log(2, 3, 8));
```
{% endraw %}

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/discrete_logarithm_mod)。固定seed・境界・愚直比較は `tests/random/math-batch-two.cpp`。
複数実装の調査、独立実装した候補と同一環境測定は [math batch two]({{ '/docs/development/math-batch-two.html' | relative_url }}) を参照。
