---
title: Disjoint Set Union
documentation_of: //blueberry/data-structure/disjoint-set-union.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

通常はACLの `atcoder::dsu` を推奨します。この実装は互換用です。Nは要素数、αは逆Ackermann関数。メモリ O(N)、添字は `[0,N)` です。

## 最小使用例

```cpp
#include <cassert>
#include "blueberry/data-structure/disjoint-set-union.hpp"
int main() {
  blueberry::DisjointSetUnion dsu(4);
  assert(dsu.merge(0, 1));
  assert(!dsu.merge(1, 0));
  assert(dsu.same(0, 1));
  assert(dsu.component_size(0) == 2);
  assert(dsu.groups().size() == 3);
}
```

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DisjointSetUnion dsu(n)` | O(N) | [開く](#construct) |
| `bool dsu.merge(u, v)` | 償却 O(α(N)) | [開く](#merge) |
| `int dsu.leader(v)` | 償却 O(α(N)) | [開く](#leader) |
| `bool dsu.same(u, v)` | 償却 O(α(N)) | [開く](#same) |
| `int dsu.component_size(v)` | 償却 O(α(N)) | [開く](#component-size) |
| `int dsu.size() const` | O(1) | [開く](#size) |
| `vector<vector<int>> dsu.groups()` | O(Nα(N)) | [開く](#groups) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>DisjointSetUnion dsu(n)</code> — O(N)</summary>

n個の独立した集合を構築します。

```cpp
blueberry::DisjointSetUnion dsu(4);
```

注意点: n>=0。n=0では要素を指定する操作は不可です。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>bool dsu.merge(u, v)</code> — 償却 O(α(N))</summary>

異なる集合を併合した場合true、すでに同じならfalseを返します。

```cpp
bool changed = dsu.merge(0, 1);
```

注意点: ACLと異なり返り値は代表元ではなくboolです。代表元は変化し得ます。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int dsu.leader(v)</code> — 償却 O(α(N))</summary>

vの集合の代表元を返します。

```cpp
int root = dsu.leader(0);
```

注意点: 最小頂点とは限りません。経路圧縮で内部状態を変更するためconstメソッドではありません。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool dsu.same(u, v)</code> — 償却 O(α(N))</summary>

同一連結成分かを返します。

```cpp
bool connected = dsu.same(0, 1);
```

注意点: u,vは有効な添字でなければなりません。

</details>

<details class="api-operation" id="component-size" markdown="1">
<summary><code>int dsu.component_size(v)</code> — 償却 O(α(N))</summary>

vの集合の要素数を返します。

```cpp
int count = dsu.component_size(0);
```

注意点: 全要素数のsize()と区別してください。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int dsu.size() const</code> — O(1)</summary>

全要素数Nを返します。

```cpp
int n = dsu.size();
```

注意点: 連結成分数ではなく、mergeしても変わりません。

</details>

<details class="api-operation" id="groups" markdown="1">
<summary><code>vector&lt;vector&lt;int&gt;&gt; dsu.groups()</code> — O(Nα(N))</summary>

各連結成分の頂点列を返します。各グループ内は昇順です。

```cpp
auto groups = dsu.groups();
```

注意点: 呼び出しごとに O(N) の追加メモリが必要です。グループ間の順番は代表元に依存します。

</details>
