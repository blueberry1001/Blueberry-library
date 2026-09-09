---
title: Rollback Union Find
documentation_of: //blueberry/data-structure/rollback-union-find.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

mergeの履歴を巻き戻せるUnion Findです。経路圧縮はせずunion by sizeを使用します。ACLのdsuにはない機能です。Nは要素数、Hは現在の履歴数、H_maxはこれまでの履歴数の最大値、Kは取り消す履歴数。確保メモリ O(N+H_max)。rollback後も履歴vectorの確保領域は保持されます。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/rollback-union-find.hpp"
int main() {
  blueberry::RollbackUnionFind uf(4);
  uf.merge(0, 1);
  int saved = uf.state();
  uf.merge(1, 2);
  assert(uf.same(0, 2));
  uf.rollback(saved);
  assert(!uf.same(0, 2));
  uf.snapshot();
  uf.merge(2, 3);
  uf.rollback();
  assert(uf.components() == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `RollbackUnionFind uf(n)` | O(N) | [開く](#construct) |
| `bool uf.merge(u, v)` | 償却 O(log N) | [開く](#merge) |
| `int uf.leader(v) const` | O(log N) | [開く](#leader) |
| `bool uf.same(u, v) const` | O(log N) | [開く](#same) |
| `int uf.component_size(v) const` | O(log N) | [開く](#component-size) |
| `int uf.components() const` | O(1) | [開く](#components) |
| `int uf.size() const` | O(1) | [開く](#size) |
| `int uf.state() const` | O(1) | [開く](#state) |
| `void uf.snapshot()` | O(1) | [開く](#snapshot) |
| `void uf.undo()` | O(1) | [開く](#undo) |
| `void uf.rollback(target_state) / uf.rollback()` | O(K) | [開く](#rollback) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>RollbackUnionFind uf(n)</code> — O(N)</summary>

n個の独立した集合、空の履歴、snapshot位置0で構築します。

{% raw %}
```cpp
blueberry::RollbackUnionFind uf(4);
```
{% endraw %}

注意点: n>=0。要素を指定する操作は0<=v<Nが必要です。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>bool uf.merge(u, v)</code> — 償却 O(log N)</summary>

異なる集合を併合したらtrueを返します。すでに同じ集合でも履歴を1件追加します。

{% raw %}
```cpp
bool changed = uf.merge(0, 1);
```
{% endraw %}

注意点: 履歴vectorの確保を含めるため償却です。falseでもundo対象です。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int uf.leader(v) const</code> — O(log N)</summary>

vの集合の代表元を返します。

{% raw %}
```cpp
int root = uf.leader(0);
```
{% endraw %}

注意点: 経路圧縮をしないため問い合わせは履歴を消費しません。代表元はmergeで変化し得ます。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool uf.same(u, v) const</code> — O(log N)</summary>

同一集合に属するかを返します。

{% raw %}
```cpp
bool connected = uf.same(0, 1);
```
{% endraw %}

注意点: 問い合わせは履歴を消費しません。

</details>

<details class="api-operation" id="component-size" markdown="1">
<summary><code>int uf.component_size(v) const</code> — O(log N)</summary>

vの集合の要素数を返します。

{% raw %}
```cpp
int count = uf.component_size(0);
```
{% endraw %}

注意点: 全要素数size()とは異なります。

</details>

<details class="api-operation" id="components" markdown="1">
<summary><code>int uf.components() const</code> — O(1)</summary>

現在の連結成分数を返します。

{% raw %}
```cpp
int groups = uf.components();
```
{% endraw %}

注意点: 有効なmergeで1減り、そのmergeのundoで1増えます。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int uf.size() const</code> — O(1)</summary>

全要素数Nを返します。

{% raw %}
```cpp
int n = uf.size();
```
{% endraw %}

注意点: rollbackでは変わりません。

</details>

<details class="api-operation" id="state" markdown="1">
<summary><code>int uf.state() const</code> — O(1)</summary>

履歴数Hを返し、rollback先として保存できます。

{% raw %}
```cpp
int saved = uf.state();
```
{% endraw %}

注意点: 永続的なバージョンIDではありません。巻き戻して分岐した場合、同じ値が別の状態を示します。

</details>

<details class="api-operation" id="snapshot" markdown="1">
<summary><code>void uf.snapshot()</code> — O(1)</summary>

引数なしrollback()の戻り先を現在のstateで上書きします。

{% raw %}
```cpp
uf.snapshot();
uf.merge(0, 1);
uf.rollback();
```
{% endraw %}

注意点: 1つだけ保存できます。ネストした探索ではstate()の返り値を自分で保存します。

</details>

<details class="api-operation" id="undo" markdown="1">
<summary><code>void uf.undo()</code> — O(1)</summary>

直前のmerge呼び出しを1回取り消します。

{% raw %}
```cpp
uf.merge(0, 1);
uf.undo();
```
{% endraw %}

注意点: 履歴が空では呼べません。失敗したmergeも1件分です。

</details>

<details class="api-operation" id="rollback" markdown="1">
<summary><code>void uf.rollback(target_state) / uf.rollback()</code> — O(K)</summary>

指定state、または最後のsnapshotまで戻ります。

{% raw %}
```cpp
int saved = uf.state();
uf.merge(1, 2);
uf.rollback(saved);
```
{% endraw %}

注意点: 0<=target_state<=state()。消した履歴への移動は不可。snapshotより前までundoした場合は保存位置を設定し直してください。

</details>
