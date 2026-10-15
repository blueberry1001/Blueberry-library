---
title: 離散対数
documentation_of: //blueberry/math/discrete-log.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要

a^x = b (mod m)を満たす最小の非負整数xを返します。合成数法・非互いに素の底にも対応し、解がなければstd::nulloptです。

ハッシュの期待時間O(sqrt(m)+log(m))、最悪時間O(m+log(m))、追加メモリO(sqrt(m))です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`discrete_log`](#log) | 期待 O(sqrt(m)+log(m)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `1 <= m <= INT32_MAX`。a,bは任意のint64_tで、正規化して扱います。0^0=1とします。
- ACLの `inv_mod` を再利用し、ACLにないgcd除去とbaby-step giant-stepを追加しています。

</details>

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

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [Library Checker](https://judge.yosupo.jp/problem/discrete_logarithm_mod)。固定seed・境界・愚直比較は `tests/random/math-batch-two.cpp`。
  複数実装の調査、独立実装した候補と同一環境測定は [math batch two]({{ '/docs/development/math-batch-two.html' | relative_url }}) を参照。

</details>
