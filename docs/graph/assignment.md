---
title: Assignment (Hungarian)
documentation_of: //blueberry/graph/assignment.hpp
---

[カテゴリへ戻る]({{ "/categories/graph.html" | relative_url }})

## 概要・前提

N 行 M 列 (N ≤ M) の費用行列で、各行へ異なる列を割り当て、総費用を最小化します。ACL にはない矩形割当です。T は符号付き整数型を想定し、比較・加減算・numeric_limits<T>::max() を使います。全ての中間値（費用差、ポテンシャル、slack、総和）が T に収まり、有限の slack が max() 未満であることが必要です。禁止辺・無限大・浮動小数点の誤差処理は提供しません。入力は変更せず、補助メモリ O(N+M)、時間 O(N²M) 最悪です。N=0 は費用 0 と空配列を返します。各行の長さは等しく、N,M < INT_MAX が必要です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/assignment.hpp"
int main() {
  std::vector<std::vector<long long>> a{{4, 1, 9}, {2, 8, 3}};
  auto [cost, match] = blueberry::assignment(a);
  assert(cost == 3 && match[0] == 1 && match[1] == 0);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `pair<T, vector<int>> assignment(const vector<vector<T>>& cost)` | O(N²M) | [開く](#solve) |

<details class="api-operation" id="solve" markdown="1">
<summary><code>pair&lt;T, vector&lt;int&gt;&gt; assignment(const vector&lt;vector&lt;T&gt;&gt;& cost)</code> — O(N²M)</summary>

最小総費用と長さ N の配列 match を返します。行 i に対応する列は match[i] ∈ [0,M)。列の重複はありません。同点の解の選択は保証しません。

{% raw %}
```cpp
auto [cost, match] = blueberry::assignment(std::vector<std::vector<long long>>{{3, 1}});
```
{% endraw %}

注意点: 返り値は所有する値で入力の寿命に依存しません。各中間値のオーバーフローは検出しません。最大化は符号反転できる範囲で費用を反転してください。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/assignment) の公式ケースと固定 seed を指定できる全列挙比較で検証します。
候補比較・一次資料・測定条件は [調査記録](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-graph-research.md) を参照してください。
