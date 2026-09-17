---
title: Dynamic Li Chao Tree
documentation_of: //blueberry/data-structure/dynamic-li-chao-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::DynamicLiChaoTree<T = long long>` は整数領域 `[low, high)` 上で、直線・線分の追加と最小値取得をオンラインで処理します。座標の事前登録は不要で、挿入順・傾きの順序は自由です。ACLには対応する機能がありません。クエリ座標が事前に分かる場合は、座標数に依存する [Li Chao Tree]({{ '/blueberry/data-structure/li-chao-tree.hpp.html' | relative_url }}) も利用できます。座標領域の変更、直線の削除、最大値の直接取得は提供しません。

T は標準の符号付き整数型（`signed char`、`short`、`int`、`long`、`long long`）を使います。係数、座標、答えはすべて T です。**各評価点で積 `a*x` と和 `a*x+b` の両方が T に収まること**が前提です。`char`/`short` 相当の小さい型の式には通常の整数昇格が働き、演算型は `decltype(T{} * T{} + T{})`（通常 `int`）です。その場合も積と和の値域は T 内に制限します。自動拡張・飽和・overflow検出はありません。`add_line` は全領域の整数点、`add_segment` は指定区間と領域との共通部分だけでこの条件を満たせば十分です。領域外・線分範囲外での評価はしません。

`low <= high` を要求します。領域も線分も半開区間なので、T の最大値を query の座標にはできません（答えとしては可能です）。内部の中点計算は対応する符号なし型で幅を求め、その半分を左端に足します。端点の和・符号付きの差を直接計算しないため、`[LLONG_MIN, LLONG_MAX)` のように幅が T に収まらない領域も指定できます。空領域の追加は何もせず、有効な query はありません。空の直線集合・その点を覆う線分がない場合は `std::nullopt` を返し、INF番兵を予約しません。

数学上の領域幅を V = high − low、H = ceil(log₂(V + 1))、確保済みノード数を K とします。直線追加 L 回、線分追加 S 回について、K = O(L + S H) かつ K = O(V)、メモリは O(K + 1) です。直線追加1回で新規ノードは高々1個、線分追加1回で O(H) 個です。内部の vector は幾何的に拡張するため、追加操作の計算量は**償却**です。再確保が起きた1回の操作にはさらに O(K) 時間がかかります。ノード数は `INT_MAX` 以下で、メモリに収まる必要があります。query は最悪計算量です。

公開フィールドはありません。ノードは木が所有し、破棄時に解放します。コピーは独立した木です。移動元は**同じ座標領域の空の木**として再利用でき、自己移動代入は何もしません。返り値は値なので、木の追加・コピー・移動・破棄による参照の無効化はありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include "blueberry/data-structure/dynamic-li-chao-tree.hpp"
int main() {
  blueberry::DynamicLiChaoTree<> tree(-10, 11);
  assert(!tree.query(3));
  tree.add_line(2, 1);
  tree.add_segment(0, 3, -1, -4);
  assert(tree.query(-2) == -3);
  assert(tree.query(0) == -4);
  assert(tree.query(3) == 7);  // 線分の右端は含まない
  auto copy = tree;
  copy.add_line(0, -100);
  assert(copy.query(3) == -100 && tree.query(3) == 7);
  auto moved = std::move(copy);
  assert(moved.query(3) == -100 && !copy.query(3));
}
```
{% endraw %}

## 操作一覧

Ksrc はコピー元、Kold は代入前の代入先のノード数です。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DynamicLiChaoTree<T> tree(T low, T high)` | O(1) | [開く](#constructor) |
| `DynamicLiChaoTree<T> copy(const DynamicLiChaoTree& other)` | O(Ksrc + 1) | [開く](#copy-constructor) |
| `DynamicLiChaoTree& tree.operator=(const DynamicLiChaoTree& other)` | O(Ksrc + Kold + 1) | [開く](#copy-assignment) |
| `DynamicLiChaoTree<T> moved(DynamicLiChaoTree&& other)` | O(1) | [開く](#move-constructor) |
| `DynamicLiChaoTree& tree.operator=(DynamicLiChaoTree&& other)` | O(Kold + 1) | [開く](#move-assignment) |
| `tree.~DynamicLiChaoTree()` | O(K + 1) | [開く](#destructor) |
| `void tree.add_line(T a, T b)` | 償却 O(H + 1) | [開く](#add-line) |
| `void tree.add_segment(T l, T r, T a, T b)` | 償却 O(H² + 1) | [開く](#add-segment) |
| `std::optional<T> tree.query(T x) const` | 最悪 O(H + 1) | [開く](#query) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>DynamicLiChaoTree&lt;T&gt; tree(T low, T high)</code> — O(1)</summary>

整数座標領域 `[low, high)` と空の直線集合を初期化します。座標一覧を渡す必要はありません。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> tree(-1000000000LL, 1000000001LL);
blueberry::DynamicLiChaoTree<int> empty(0, 0);
```
{% endraw %}

注意点: `low <= high` が前提です。空領域は許されますが query は呼べません。端点は構築後に変更できません。
</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>DynamicLiChaoTree&lt;T&gt; copy(const DynamicLiChaoTree&amp; other)</code> — O(Ksrc + 1)</summary>

領域と全ノードを独立にコピーします。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> tree(-2, 3);
tree.add_line(0, 5);
auto copy = tree;
copy.add_line(0, 1);
assert(tree.query(0) == 5 && copy.query(0) == 1);
```
{% endraw %}

注意点: コピー先には O(Ksrc + 1) の追加メモリが必要です。空の木もコピーできます。
</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>DynamicLiChaoTree&amp; tree.operator=(const DynamicLiChaoTree&amp; other)</code> — O(Ksrc + Kold + 1)</summary>

代入先の領域とノードをコピー元の独立したコピーで置き換え、`*this` を返します。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> source(-2, 3), target(0, 1);
source.add_line(0, 5);
target = source;
assert(target.query(-2) == 5);
```
{% endraw %}

注意点: 元の代入先の直線は失われます。自己代入は可能です。ピークメモリの上界は O(Ksrc + Kold + 1) です。
</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>DynamicLiChaoTree&lt;T&gt; moved(DynamicLiChaoTree&amp;&amp; other) noexcept</code> — O(1)</summary>

領域とノードの所有権を移します。移動元は同じ領域の空の木になります。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> source(-2, 3);
source.add_line(0, 5);
auto target = std::move(source);
assert(target.query(0) == 5 && !source.query(0));
source.add_line(0, 9);
assert(source.query(0) == 9);
```
{% endraw %}

注意点: ノードをコピーしません。追加メモリは O(1) です。取得済みの optional の寿命には影響しません。
</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>DynamicLiChaoTree&amp; tree.operator=(DynamicLiChaoTree&amp;&amp; other) noexcept</code> — O(Kold + 1)</summary>

代入先のノードを解放し、領域と所有権を移して `*this` を返します。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> source(-2, 3), target(0, 1);
source.add_line(0, 5);
target = std::move(source);
assert(target.query(-2) == 5 && !source.query(-2));
```
{% endraw %}

注意点: 自己移動代入は何も変更しません。それ以外では移動元を同じ領域の空の木として再利用できます。追加メモリは O(1) です。
</details>

<details class="api-operation" id="destructor" markdown="1">
<summary><code>tree.~DynamicLiChaoTree()</code> — O(K + 1)</summary>

所有している全ノードを解放します。返り値はありません。

{% raw %}
```cpp
{
  blueberry::DynamicLiChaoTree<> tree(0, 10);
  tree.add_line(1, 0);
}  // スコープを抜けると自動解放
```
{% endraw %}

注意点: 通常は明示的に呼びません。子ポインタによる再帰的な解放はなく、コピーした別の木には影響しません。
</details>

<details class="api-operation" id="add-line" markdown="1">
<summary><code>void tree.add_line(T a, T b)</code> — 償却 O(H + 1)</summary>

領域全体で有効な `y = ax + b` を追加します。引数は傾き・切片の順で、返り値はありません。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> tree(-2, 3);
tree.add_line(2, 1);
assert(tree.query(2) == 5);
```
{% endraw %}

注意点: 空領域では何もしません。同じ直線・同じ傾きも追加できます。全領域の整数点で積と和が T に収まる必要があります。vector 再確保を含む単発の最悪時間は O(K + H + 1) です。
</details>

<details class="api-operation" id="add-segment" markdown="1">
<summary><code>void tree.add_segment(T l, T r, T a, T b)</code> — 償却 O(H² + 1)</summary>

`[l, r)` と構築時の領域の共通部分に `y = ax + b` を追加します。引数は**左端・右端・傾き・切片**の順です。返り値はありません。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> tree(0, 10);
tree.add_segment(-5, 3, 2, 1);
assert(tree.query(0) == 1 && tree.query(2) == 5);
assert(!tree.query(3));
```
{% endraw %}

注意点: `l <= r` が前提です。空区間・空領域・共通部分がない場合は何もしません。領域外の端点を許し、実際に覆う整数点のみで積と和が T に収まれば十分です。再確保を含む単発の最悪時間は O(K + H² + 1) です。
</details>

<details class="api-operation" id="query" markdown="1">
<summary><code>std::optional&lt;T&gt; tree.query(T x) const</code> — 最悪 O(H + 1)</summary>

x を覆う直線・線分の最小値を値で返し、木は変更しません。1本もなければ `std::nullopt` です。

{% raw %}
```cpp
blueberry::DynamicLiChaoTree<> tree(-2, 3);
assert(!tree.query(1));
tree.add_line(0, 7);
const auto answer = tree.query(1);
assert(answer && *answer == 7);
```
{% endraw %}

注意点: `low <= x < high` が前提で、空領域では呼べません。事前登録は不要です。空を確認してから値を取り出してください。ノードの追加確保はありません。
</details>

## 出典・検証

動的整数領域と支配直線の棄却について [Luzhiled の Dynamic Li Chao Tree](https://ei1333.github.io/library/structure/convex-hull-trick/dynamic-li-chao-tree.hpp.html)、圧縮配列方式について [Nyaan の Li Chao Tree](https://nyaannyaan.github.io/library/segment-tree/li-chao-tree.hpp.html) を調査しました。外部コードを転載せず、直線が高々1回交差する性質から独立に実装しています。従来の生ポインタ・INF番兵・閉じた整数領域とは異なり、所有 arena、optional、半開区間を使います。

- [Line Add Get Min](https://judge.yosupo.jp/problem/line_add_get_min): `verify/data-structure/dynamic-line-add-get-min.test.cpp`。
- [Segment Add Get Min](https://judge.yosupo.jp/problem/segment_add_get_min): `verify/data-structure/dynamic-segment-add-get-min.test.cpp`。
- `tests/random/dynamic-li-chao-tree.cpp`: 固定 seed の全整数点愚直比較、空・単点・型境界、同傾き、区間外 clipping、有効範囲外を評価すると overflow する線分、コピー・移動・移動元再利用。
- 調査と同条件の性能比較は `benchmark/dynamic-li-chao-tree.md` を参照してください。
