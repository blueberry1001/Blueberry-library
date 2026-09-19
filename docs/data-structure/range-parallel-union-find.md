---
title: Range Parallel Union Find
documentation_of: //blueberry/data-structure/range-parallel-union-find.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`RangeParallelUnionFind` は同じ長さの2区間について対応頂点同士をまとめて併合します。通常のACL DSUを各2冪長に配置する拡張です。
Nは頂点数、Qは呼出し数。構築・メモリ O(N log(N+1))、全操作合計 O((N log(N+1)+Q) α(N))。
長さKの単独range_mergeは最悪 O(K log(N+1)) が安全な上限で、各DSU操作の償却を使えば O(K α(N))。
繰返し同じ大区間を指定しても、併合済みブロックを再帰展開しません。通常のleader/same/comp_sizeは償却 O(α(N))。
callbackは実際に異なる底辺成分が併合されたときだけ `(残る代表,吸収される代表)` を受け取り、全期間で高々N-1回です。
callback時間は上記に別加算。callbackから同じUFを変更しないでください。副作用の巻き戻し・例外保証は提供しません。
0<=N<=INT_MAX。全区間は半開に対応。空入力・長さ0・重なった区間・自分自身への併合も許します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/range-parallel-union-find.hpp"
int main() {
  blueberry::RangeParallelUnionFind uf(6);
  std::vector<int> sum(6, 1);
  uf.range_merge(0, 3, 3, [&](int keep, int drop) { sum[keep] += sum[drop]; });
  assert(uf.same(0, 3) && !uf.same(0, 1));
  assert(uf.comp_size(4) == 2 && sum[uf.leader(4)] == 2 && uf.components() == 3);
}
```
{% endraw %}

## 操作一覧

以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `RangeParallelUnionFind(int n = 0)` | O(N log(N+1)) | [開く](#constructor) |
| `int object.size() const` | O(1) | [開く](#size) |
| `int object.components() const` | O(1) | [開く](#components) |
| `int object.leader(int p)` | 償却 O(α(N)) | [開く](#leader) |
| `bool object.same(int a, int b)` | 償却 O(α(N)) | [開く](#same) |
| `int object.comp_size(int p)` | 償却 O(α(N)) | [開く](#comp-size) |
| `void object.merge(int a, int b)` | 償却 O(α(N)) | [開く](#merge) |
| `void object.merge(int a, int b, F callback)` | 償却 O(α(N)) | [開く](#merge-callback) |
| `void object.range_merge(int a, int b, int length)` | O(K log(N+1)) | [開く](#range-merge) |
| `void object.range_merge(int a, int b, int length, F callback)` | O(K log(N+1)) | [開く](#range-callback) |
| `Type(const Type& other)` | O(M) | [開く](#copy-constructor) |
| `Type(Type&& other)` | O(1) | [開く](#move-constructor) |
| `Type& object.operator=(const Type& other)` | O(M + D) | [開く](#copy-assignment) |
| `Type& object.operator=(Type&& other)` | O(1 + D) | [開く](#move-assignment) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>RangeParallelUnionFind(int n = 0)</code> — O(N log(N+1))</summary>

n個の孤立頂点で構築します。

{% raw %}
```cpp
blueberry::RangeParallelUnionFind uf(6);
```
{% endraw %}

注意点: n>=0。空の構造も可。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

頂点数を返します。

{% raw %}
```cpp
int n = object.size();
```
{% endraw %}

注意点: 更新で変わりません。

</details>

<details class="api-operation" id="components" markdown="1">
<summary><code>int object.components() const</code> — O(1)</summary>

底辺の成分数を返します。

{% raw %}
```cpp
int n = object.components();
```
{% endraw %}

注意点: 空なら0。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int object.leader(int p)</code> — 償却 O(α(N))</summary>

頂点の代表を返します。

{% raw %}
```cpp
int r = object.leader(0);
```
{% endraw %}

注意点: 0<=p<N。内部で経路圧縮します。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool object.same(int a, int b)</code> — 償却 O(α(N))</summary>

2頂点の連結性を返します。

{% raw %}
```cpp
bool yes = object.same(0, 1);
```
{% endraw %}

注意点: 0<=a,b<N。

</details>

<details class="api-operation" id="comp-size" markdown="1">
<summary><code>int object.comp_size(int p)</code> — 償却 O(α(N))</summary>

頂点の成分サイズを返します。

{% raw %}
```cpp
int n = object.comp_size(0);
```
{% endraw %}

注意点: 0<=p<N。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>void object.merge(int a, int b)</code> — 償却 O(α(N))</summary>

1組の頂点を併合します。

{% raw %}
```cpp
object.merge(0, 1);
```
{% endraw %}

注意点: 0<=a,b<N。同じ成分なら変化なし。

</details>

<details class="api-operation" id="merge-callback" markdown="1">
<summary><code>void object.merge(int a, int b, F callback)</code> — 償却 O(α(N))</summary>

併合成功時にcallback(keep,drop)を呼びます。

{% raw %}
```cpp
object.merge(0, 1, [](int keep, int drop) { (void)keep; (void)drop; });
```
{% endraw %}

注意点: callback自身の時間は別。同一成分なら呼びません。

</details>

<details class="api-operation" id="range-merge" markdown="1">
<summary><code>void object.range_merge(int a, int b, int length)</code> — O(K log(N+1))</summary>

i=0..length-1についてa+iとb+iを併合します。

{% raw %}
```cpp
object.range_merge(0, 3, 3);
```
{% endraw %}

注意点: K=length。0<=a,b<=N、0<=K<=min(N-a,N-b)。全期間の償却上限は概要参照。

</details>

<details class="api-operation" id="range-callback" markdown="1">
<summary><code>void object.range_merge(int a, int b, int length, F callback)</code> — O(K log(N+1))</summary>

各底辺の併合成功時にcallbackを呼びます。

{% raw %}
```cpp
object.range_merge(0, 3, 3, [](int keep, int drop) { (void)keep; (void)drop; });
```
{% endraw %}

注意点: callback時間は別。呼出し順は規定しません。K=0なら呼ばず、変更もありません。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>Type(const Type&amp; other)</code> — O(M)</summary>

全履歴・補助索引を独立にコピーします。

{% raw %}
```cpp
Type copied(object);
```
{% endraw %}

注意点: 以後の更新は互いに影響しません。M は保持する全要素・ノード・版数です。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>Type(Type&amp;&amp; other)</code> — O(1)</summary>

内部領域の所有権を移します。

{% raw %}
```cpp
Type moved(std::move(object));
```
{% endraw %}

注意点: <utility> が必要です。ムーブ元は破棄または再代入してから使用してください。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>Type&amp; object.operator=(const Type&amp; other)</code> — O(M + D)</summary>

コピーで置き換え、*this を返します。

{% raw %}
```cpp
Type copied; copied = object;
```
{% endraw %}

注意点: D は代入前の宛先保持量。旧領域の破棄も含みます。自己代入は有効です。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>Type&amp; object.operator=(Type&amp;&amp; other)</code> — O(1 + D)</summary>

旧領域を破棄し所有権を移し、*this を返します。

{% raw %}
```cpp
Type moved; moved = std::move(object);
```
{% endraw %}

注意点: D は旧宛先保持量。<utility> が必要です。自己ムーブを含めムーブ元は破棄または再代入してください。

</details>

## 出典・検証

- [maspypy](https://maspypy.github.io/library/ds/unionfind/parallel_unionfind.hpp) と [MtSaka](https://mtsaka.github.io/library/test/yosupo/data_structure/range_parallel_unionfind.test.cpp) の階層化を調査し、ACL DSUを使って独立実装。
- `verify/data-structure/range-parallel-unionfind.test.cpp`: [Range Parallel Unionfind](https://judge.yosupo.jp/problem/range_parallel_unionfind)。
- `tests/random/persistence-and-range-union.cpp`: 重なり・空区間・コールバック成分和・全頂点対を愚直併合と比較。
