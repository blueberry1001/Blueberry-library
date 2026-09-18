---
title: Rooted Tree Isomorphism
documentation_of: //blueberry/graph/rooted-tree-isomorphism.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

根付き木の各頂点を根とする部分木を、子の順序を無視した同型類に分類します。ACLにはありません。
`parent[v]` は親番号、唯一の根では `-1` です。頂点数 N は `int` の範囲内で、親番号は `[0,N)`。
空配列も許可します。非空なら連結・非循環な木が必要です。
子のクラス番号を整列し、完全なベクトルを辞書キーにするためハッシュ衝突はありません。
時間 O(N log(N+1))、追加メモリ O(N)。整数演算のオーバーフローは前提範囲内でありません。
再帰DFSを使わないため、深いパスでも呼び出しスタックは増えません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/rooted-tree-isomorphism.hpp"
int main() {
  auto id = blueberry::rooted_tree_isomorphism({-1, 0, 0, 1, 2});
  assert(id[1] == id[2]);
  assert(id[3] == id[4]);
  assert(id[0] != id[1]);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> rooted_tree_isomorphism(const vector<int>& parent)` | O(N log(N+1)) | [開く](#classify) |

<details class="api-operation" id="classify" markdown="1">
<summary><code>rooted_tree_isomorphism(parent)</code> — O(N log(N+1))</summary>

長さ N のクラス番号配列を返します。番号は `[0,K)` の連続した整数で、K は同型類の数です。
同じ呼び出し内では `id[u]==id[v]` と部分木同型が同値です。
異なる呼び出し間で番号を比較してはいけません。根は0番でなくてもよく、親番号が子より大きくても構いません。
空入力は空出力です。入力は変更せず、返り値の寿命は入力から独立しています。
辞書検索で比較するキー長の総和は O(N) であり、記載した計算量は最悪値です。

{% raw %}
```cpp
auto id = blueberry::rooted_tree_isomorphism({1, -1, 1});
assert(id[0] == id[2]); // 葉同士
```
{% endraw %}

注意点: 上記の入力条件と寿命の制約を満たしてください。

</details>

## 出典・検証

[spaghetti-source の木同型](https://github.com/spaghetti-source/algorithm/blob/master/graph/tree_isomorphism.cc) の子ラベル整列と
[Nyaan の tree hash](https://nyaannyaan.github.io/library/tree/tree-hash.hpp.html) の確率的分類を調査。
衝突を排除する完全キー方式を独立実装しました。外部コードの転載はありません。
[Library Checker](https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification)、括弧列とのランダム比較、長さ20万のパスで検証します。
候補の同一環境測定は [追加調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/graph-batch-four.md) に記載します。
