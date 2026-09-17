---
title: Triangle Enumeration
documentation_of: //blueberry/graph/triangle-enumeration.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

単純無向グラフの三角形を重複なく列挙します。ACLには対応機能がありません。N頂点、M辺、端点 [0,N)、N,M ≤ INT_MAX。
自己ループと多重辺は禁止（多重辺は実行時検査しません）。次数と頂点番号の順で辺を向き付け、最悪 O(N+M√M)、作業メモリ O(N+M)。
コールバックの実行時間は別途加算します。全三角形を内部保存しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/triangle-enumeration.hpp"
int main() {
  int count = 0;
  blueberry::enumerate_triangles(3, {{0,1},{1,2},{0,2}},
    [&](int a, int b, int c) { assert(a != b && b != c && a != c); ++count; });
  assert(count == 1);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `void enumerate_triangles(int n, const vector<pair<int,int>>& edges, Callback&& emit)` | O(N+M√M) + callback | [開く](#enumerate) |

<details class="api-operation" id="enumerate" markdown="1">
<summary><code>enumerate_triangles(n, edges, emit)</code> — O(N+M√M) + callback</summary>

各三角形につき `emit(int a,int b,int c)` を1回同期呼び出しします。三頂点は異なりますが番号順・列挙順は保証しません。
空グラフや三角形なしは呼び出し0回。戻り値はvoid。入力を変更せずコールバックや入力への参照を保持しません。

{% raw %}
```cpp
long long count = 0;
blueberry::enumerate_triangles(0, {}, [&](int, int, int) { ++count; });
assert(count == 0);
```
{% endraw %}

注意点: 集計型のオーバーフローは利用者が管理してください。実行中に入力を変更するコールバックは禁止。
次数はintですが単純グラフでは N-1 以下です。計算量は最悪値です。
</details>

## 出典・検証

[hitonanodeの次数による向き付け](https://hitonanode.github.io/cplib-cpp/other_algorithms/enumerate_triangles.hpp.html) と
[maspypyの三角形列挙](https://maspypy.github.io/library/enumerate/triangle.hpp) を調査し、次数順DAGと頂点マークによる独自実装を採用。
コードの転用はありません。[Library Checker](https://judge.yosupo.jp/problem/enumerate_triangles) と三重ループの乱択比較で検証します。
測定は [graph-batch-two](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/graph-batch-two.md)。
