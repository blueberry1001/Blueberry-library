---
title: Rollback Union Find
documentation_of: //blueberry/data-structure/rollback-union-find.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

変更履歴を保存し、過去の状態へ戻せる Union Find です。経路圧縮は行わず union by size
を使うため、`merge`・`leader`・`same`・`component_size` は $O(\log N)$ です。
`undo` は $O(1)$、`rollback` は取り消す履歴数に比例します。要素番号は `[0,N)` です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/rollback-union-find.hpp"
int main() {
  blueberry::RollbackUnionFind uf(3);
  uf.merge(0, 1);
  const int checkpoint = uf.state();
  uf.merge(1, 2);
  assert(uf.components() == 1);
  uf.rollback(checkpoint);
  assert(uf.components() == 2);
  assert(uf.comp_size(0) == 2);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `RollbackUnionFind uf(n)` | O(N) | [開く](#construct) |
| `int uf.leader(v) const` | O(log N) | [開く](#leader) |
| `bool uf.merge(u, v)` | O(log N) | [開く](#merge) |
| `bool uf.same(u, v) const` | O(log N) | [開く](#same) |
| `int uf.component_size(v) const` | O(log N) | [開く](#component-size) |
| `int uf.comp_size(v) const` | O(log N) | [開く](#comp-size) |
| `int uf.components() const` | O(1) | [開く](#components) |
| `int uf.size() const` | O(1) | [開く](#size) |
| `int uf.state() const` | O(1) | [開く](#state) |
| `void uf.snapshot()` | O(1) | [開く](#snapshot) |
| `void uf.undo()` | O(1) | [開く](#undo) |
| `void uf.rollback(state)` | O(取り消し数) | [開く](#rollback-state) |
| `void uf.rollback()` | O(取り消し数) | [開く](#rollback-snapshot) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>RollbackUnionFind uf(n)</code> — O(N)</summary>

各要素を単独集合として初期化します。

{% raw %}
```cpp
blueberry::RollbackUnionFind uf(100);
```
{% endraw %}

注意点: `n` は0以上。履歴容量は操作に応じて増加します。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int uf.leader(v) const</code> — O(log N)</summary>

`v` の集合の代表を返します。経路圧縮をしないため、過去状態を安全に復元できます。

{% raw %}
```cpp
int root = uf.leader(1);
```
{% endraw %}

注意点: `v` は `[0,N)`。代表は併合やrollbackで変わることがあります。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>bool uf.merge(u, v)</code> — O(log N)</summary>

二つの集合を併合し、履歴を1件追加します。実際に併合したときだけ `true` です。

{% raw %}
```cpp
bool merged = uf.merge(0, 2);
```
{% endraw %}

注意点: 同じ集合への呼び出しも履歴を消費するため、対応するundoが必要です。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool uf.same(u, v) const</code> — O(log N)</summary>

二つの要素が同じ集合かを判定します。

{% raw %}
```cpp
bool connected = uf.same(0, 1);
```
{% endraw %}

注意点: 両方の要素番号が有効でなければなりません。

</details>

<details class="api-operation" id="component-size" markdown="1">
<summary><code>int uf.component_size(v) const</code> — O(log N)</summary>

`v` を含む集合の要素数を返します。

{% raw %}
```cpp
int count = uf.component_size(0);
```
{% endraw %}

注意点: rollback後は過去の値に戻ります。

</details>

<details class="api-operation" id="comp-size" markdown="1">
<summary><code>int uf.comp_size(v) const</code> — O(log N)</summary>

`component_size` の短い別名です。

{% raw %}
```cpp
int count = uf.comp_size(0);
```
{% endraw %}

注意点: 引数・返り値・計算量は `component_size` と同じです。

</details>

<details class="api-operation" id="components" markdown="1">
<summary><code>int uf.components() const</code> — O(1)</summary>

現在の連結成分数を返します。

{% raw %}
```cpp
int count = uf.components();
```
{% endraw %}

注意点: `merge` とrollbackで変化します。同じ集合へのmergeでは変化しません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int uf.size() const</code> — O(1)</summary>

管理要素数 $N$ を返します。

{% raw %}
```cpp
int n = uf.size();
```
{% endraw %}

注意点: 連結成分数ではありません。

</details>

<details class="api-operation" id="state" markdown="1">
<summary><code>int uf.state() const</code> — O(1)</summary>

履歴の件数を返します。この値をcheckpointとして `rollback` に渡せます。

{% raw %}
```cpp
int checkpoint = uf.state();
```
{% endraw %}

注意点: merge（実際に併合しない場合も含む）ごとに1増え、undoで1減ります。

</details>

<details class="api-operation" id="snapshot" markdown="1">
<summary><code>void uf.snapshot()</code> — O(1)</summary>

現在のstateを、引数なし `rollback()` の戻り先として保存します。

{% raw %}
```cpp
uf.snapshot();
```
{% endraw %}

注意点: スナップショットは一つだけです。新たに呼ぶと上書きされます。

</details>

<details class="api-operation" id="undo" markdown="1">
<summary><code>void uf.undo()</code> — O(1)</summary>

直前のmerge一件を取り消します。

{% raw %}
```cpp
uf.undo();
```
{% endraw %}

注意点: 履歴が空のときは呼べません。同じ集合へのmergeも一件として戻します。

</details>

<details class="api-operation" id="rollback-state" markdown="1">
<summary><code>void uf.rollback(state)</code> — O(取り消し数)</summary>

履歴件数が指定値になるまで `undo` を繰り返します。

{% raw %}
```cpp
int checkpoint = uf.state();
uf.merge(0, 1);
uf.rollback(checkpoint);
```
{% endraw %}

注意点: `0<=state<=uf.state()`。指定値より先の状態へ進めることはできません。

</details>

<details class="api-operation" id="rollback-snapshot" markdown="1">
<summary><code>void uf.rollback()</code> — O(取り消し数)</summary>

`snapshot()` で保存した履歴件数まで戻します。

{% raw %}
```cpp
uf.snapshot();
uf.merge(0, 1);
uf.rollback();
```
{% endraw %}

注意点: `snapshot()` 前の呼び出しでは初期状態（state 0）まで戻ります。

</details>
