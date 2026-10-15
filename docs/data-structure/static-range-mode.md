---
title: Static Range Mode
documentation_of: //blueberry/data-structure/static-range-mode.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

静的配列の複数区間の最頻値と頻度を一括計算します。オンライン問合せ・更新には対応しません。Tの `<` は全順序、`==` はその同値関係と一致する必要があります。

N=a.size()、Q=queries.size()。時間はO(N log(N+1)+Q log(Q+1)+(N+Q)sqrt(N+1))、追加メモリはO(N+Q)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`static_range_mode`](#static-range-mode) | O(N log(N+1) + Q log(Q+1) + (N+Q) sqrt(N+1)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLにない操作をMo順序と頻度別連結リストで実装します。`N < INT_MAX, Q <= INT_MAX` が必要です。
- Tはコピー可能で、比較・コピーをO(1)とします。入力は変更しません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/static-range-mode.hpp"
int main() {
  std::vector<int> a{8, 3, 8, 8};
  auto ans = blueberry::static_range_mode(a, {{0, 3}, {2, 2}});
  assert(a[ans[0].first] == 8 && ans[0].second == 2);
  assert(ans[1].first == -1 && ans[1].second == 0);
}
```
{% endraw %}

<details class="api-operation" id="static-range-mode" markdown="1">
<summary><code>vector&lt;pair&lt;int,int&gt;&gt; static_range_mode(a, queries)</code> — O(N log(N+1) + Q log(Q+1) + (N+Q) sqrt(N+1))</summary>

queryは `pair<int,int>{l,r}`、`0 <= l <= r <= N`。返り値の順序はqueryと同じ。
各要素は `{index, frequency}`。`a[index]` が区間の最頻値の1つ、frequencyが区間内出現数。
**indexは値を識別する配列全体の代表位置であり、query区間の外にある場合がある。**
同頻度の候補の選択は未規定。空区間は `{-1,0}`、queryが空なら空vector。
N=0でも空区間queryを指定できる。頻度はN以下でintに収まる。
入力への参照は保持しないが、最頻値の復元には呼び出し時と同じ配列を使うこと。
時間は最悪計算量。頻度変更は1段だけなので最大頻度の更新も各O(1)。

{% raw %}
```cpp
std::vector<int> a{4, 4, 4};
auto ans = blueberry::static_range_mode(a, {{0, 1}});
assert(a[ans[0].first] == 4 && ans[0].second == 1);
```
{% endraw %}

注意点: 上記の入力制約と空入力の規約を守ること。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Nyaan Mo順序](https://github.com/NyaanNyaan/library/blob/master/misc/mo.hpp)。
- [suisen static range mode](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/range_query/static_range_mode_query.hpp): ブロック間前計算によるオンライン方式と比較。
- [Library Checker](https://judge.yosupo.jp/problem/static_range_mode_query)、空区間・同率を含む全区間のランダム愚直比較。
- [比較・測定](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/sequence-batch-four.md)。参照コードは転載せず独自実装。

</details>
