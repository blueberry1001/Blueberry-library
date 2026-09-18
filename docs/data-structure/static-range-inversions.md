---
title: Static Range Inversions
documentation_of: //blueberry/data-structure/static-range-inversions.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

静的配列の複数区間について転倒数を一括計算する。ACL Fenwick treeとMo順序を組み合わせる。
`N=a.size(), Q=queries.size()`、`N < INT_MAX, Q <= INT_MAX`。`T` はコピー可能で、
`<` が全順序、`==` がその同値関係と一致すること。比較・コピーをO(1)として評価する。
時間 O(N log(N+1) + Q log(Q+1) + (N+Q) sqrt(N+1) log(N+1))、追加メモリ O(N+Q)。
オンライン更新には対応しない。入力は変更せず、参照も保持しない。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/static-range-inversions.hpp"
int main() {
  std::vector<int> a{3, 1, 2, 1};
  auto ans = blueberry::static_range_inversions(a, {{0, 4}, {1, 3}, {2, 2}});
  assert((ans == std::vector<long long>{4, 0, 0}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<long long> static_range_inversions(const vector<T>& a, const vector<pair<int,int>>& queries)` | O(N log(N+1) + Q log(Q+1) + (N+Q) sqrt(N+1) log(N+1)) | [開く](#static-range-inversions) |

<details class="api-operation" id="static-range-inversions" markdown="1">
<summary><code>vector&lt;long long&gt; static_range_inversions(a, queries)</code> — O(N log(N+1) + Q log(Q+1) + (N+Q) sqrt(N+1) log(N+1))</summary>

各queryは `pair<int,int>{l,r}`、`0 <= l <= r <= N`。返り値の順序は入力queryと同じ。
`l <= i < j < r` かつ `a[j] < a[i]` を満たす組数を返す。同値は転倒に数えない。
空区間は0、queryが空なら空vector。N=0でも空区間queryを指定できる。
最大値はN(N-1)/2で、指定したNの制限下でlong longに収まる。
返り値は所有するvectorであり、入力の破棄後も有効。上記は最悪計算量で、償却保証ではない。

{% raw %}
```cpp
auto ans = blueberry::static_range_inversions(std::vector<int>{2, 2, 1}, {{0, 3}});
assert(ans[0] == 2);
```
{% endraw %}

注意点: 上記の入力制約と空入力の規約を守ること。

</details>

## 出典・検証

- [Nyaan Mo + BIT検証](https://github.com/NyaanNyaan/library/blob/master/verify/verify-yosupo-ds/yosupo-static-range-inversions-query.test.cpp)。
- [maspypy Mo + FenwickTree_01検証](https://github.com/maspypy/library/blob/main/test/2_library_checker/data_structure/static_range_inversions_mo_3.test.cpp)。
- [Library Checker](https://judge.yosupo.jp/problem/static_range_inversions_query)、空区間・重複を含む全区間のランダム愚直比較。
- [比較・測定](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/sequence-batch-four.md)。アルゴリズムを独自に実装し、BIT本体はACLを使用。
