---
title: Maximum Independent Set
documentation_of: //blueberry/graph/maximum-independent-set.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

単純無向グラフの最大独立集合を半分全列挙で厳密に求めます。ACLにはありません。
頂点数 `0 <= N <= 40`、頂点番号は `[0,N)`、辺数を M とします。
自己ループ・重複辺は禁止です。辺の向きは任意です。
時間 O(M + 2^(ceil(N/2)))、追加メモリ O(N + 2^(ceil(N/2)))。
固定幅整数のビット演算を使い、指定範囲ではシフト・集合サイズのオーバーフローはありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/maximum-independent-set.hpp"
int main() {
  auto vertices = blueberry::maximum_independent_set(3, {{0, 1}, {1, 2}});
  assert((vertices == std::vector<int>{0, 2}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> maximum_independent_set(int n, const vector<pair<int,int>>& edges)` | O(M + 2^(ceil(N/2))) | [開く](#solve) |

<details class="api-operation" id="solve" markdown="1">
<summary><code>maximum_independent_set(n, edges)</code> — O(M + 2^(ceil(N/2)))</summary>

どの2頂点間にも辺がない最大サイズの集合を昇順の頂点番号で返します。同サイズの解の選択は保証しません。
`n=0` では空、辺が空なら全頂点を返します。入力は変更しません。
返り値は独立した所有値で、入力の寿命に依存しません。計算量は最悪値で、償却ではありません。

{% raw %}
```cpp
auto all = blueberry::maximum_independent_set(2, {}); // {0, 1}
```
{% endraw %}

右半分の部分集合ごとの最適解と、左半分の独立性・許可される右頂点をDPします。
40頂点でも指数メモリを必要とするため、大きい一般グラフには使用できません。

注意点: 上記の入力条件と寿命の制約を満たしてください。

</details>

## 出典・検証

[Nyaan の分枝探索](https://nyaannyaan.github.io/library/graph/max-independent-set.hpp.html) と
[Luzhiled のランダム貪欲法](https://ei1333.github.io/library/graph/others/maximum-independent-set.hpp.html) を比較調査。
入力による探索時間の差と近似解を避けるため、半分全列挙を独立実装しました。外部コードの転載はありません。
[Library Checker](https://judge.yosupo.jp/problem/maximum_independent_set) と固定seedの全探索比較で検証します。
候補の同一環境測定は [追加調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/graph-batch-four.md) を参照してください。
