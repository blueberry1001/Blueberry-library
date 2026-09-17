---
title: Dynamic Fenwick Tree 2D
documentation_of: //blueberry/data-structure/dynamic-fenwick-tree-2d.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

整数領域 `[0,n) × [0,m)` で **未知の座標へのオンライン点加算** と半開長方形の和を扱う。
外側の Fenwick セルをハッシュ表で疎に保存し、各セル内の y 座標は部分木和を持つ AVL 木に保存する。
全 AVL ノードは整数添字で結ぶ一つの `vector` に所有する。巨大な n,m に比例する初期化や、
二重の動的 Fenwick のような y 方向の O(log m) 個のセル生成を避ける。

ACL に二次元 Fenwick はない。更新予定座標が先に分かる場合は、よりコンパクトな
[Offline Fenwick Tree 2D]({{ '/blueberry/data-structure/offline-fenwick-tree-2d.hpp.html' | relative_url }}) を優先する。
両者とも `add(x,y,value)`・`pref(x,y)`・`sum(left,down,right,up)` を使うが、
本構造の問い合わせ端点は構築時の領域内に限る。負座標は呼び出し側で平行移動する。

`DynamicFenwickTree2D<T, Coord = long long>` の `Coord` は符号付き整数型。
`0 <= n,m <= numeric_limits<Coord>::max()`。`T{}` は加算単位元、コピー・`+`・`+=`・`-`
を要求し、可換加法群を仮定する。整数・ACL modint に対応する。`T` の全中間演算の
オーバーフローは呼び出し側で防ぐ。浮動小数点の演算順による誤差には注意する。
座標演算は最大の `Coord` でも安全。公開フィールド・外部に貸し出す参照はない。
無効な添字・逆向き区間は `assert` 違反で、release でも満たすべき事前条件。

`Q` をこれまで更新した異なる点の数、`L=1+floor(log2(max(1,n)))`、`H=1+log2(Q+1)`、
`S` を保存した AVL ノード数、`R` を外側の保存セル数とする。
`R <= S = O(QL)`、メモリ O(1+S)。ノード添字は `int` なので `S < INT_MAX` が必要。
ゼロ加算・ゼロに戻ったノードも保持する。重複点への加算は新しい AVL ノードを作らない。

操作表はハッシュが十分分散する平均計算量を示す。AVL 木の高さは最悪 O(H) だが、
ハッシュ衝突が集中する読み取りの最悪は O(L(R+H))。
更新は `vector` 再確保 O(S) とハッシュ再構築の最悪 O(R²) を含め、
単一操作の最悪 O(L(R+H)+S+R²)、平均・償却 O(LH)。`T` の演算・コピーは O(1) とする。
読み取りは割当・挿入を行わない。

旧 `DynamicFenwickTree2D<T>` からは名前空間と include を変更し、`sum(x,y)` を
`pref(x,y)` に置換する。長方形の引数順は維持する。旧 `N,M,bit` の直接操作は不可。
旧構造の raw pointer の共有・未解放を廃止し、コピーは独立、move 元は空となる。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"
int main() {
  blueberry::DynamicFenwickTree2D<long long> tree(1000000000000LL, 1000000000000LL);
  tree.add(3, 7, 5);
  tree.add(999999999999LL, 2, -1); // future coordinates need no registration
  assert(tree.pref(4, 8) == 5);
  assert(tree.sum(3, 7, 4, 8) == 5);
  assert(tree.get(999999999999LL, 2) == -1);
  assert(tree.pref(tree.height(), tree.width()) == 4);
  auto copy = tree;
  copy.add(3, 7, 2);
  assert(tree.get(3, 7) == 5 && copy.get(3, 7) == 7);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DynamicFenwickTree2D<T,Coord> tree` | O(1) | [開く](#default) |
| `DynamicFenwickTree2D<T,Coord> tree(n,m)` | O(1) | [開く](#construct) |
| `DynamicFenwickTree2D<T,Coord> copy(other)` | O(1+S) | [開く](#copy-construct) |
| `tree = other` → tree の参照 | O(1+S+S_old) | [開く](#copy-assign) |
| `DynamicFenwickTree2D<T,Coord> moved(std::move(other))` | O(1) | [開く](#move-construct) |
| `tree = std::move(other)` → tree の参照 | O(1+S_old) | [開く](#move-assign) |
| `Coord tree.height() const` | O(1) | [開く](#height) |
| `Coord tree.width() const` | O(1) | [開く](#width) |
| `void tree.add(Coord x, Coord y, const T& value)` | 平均・償却 O(LH) | [開く](#add) |
| `T tree.pref(Coord x, Coord y) const` | 平均 O(LH) | [開く](#pref) |
| `T tree.sum(Coord left, Coord down, Coord right, Coord up) const` | 平均 O(LH) | [開く](#sum) |
| `T tree.get(Coord x, Coord y) const` | 平均 O(LH) | [開く](#get) |
| `tree.~DynamicFenwickTree2D()` | O(1+S) | [開く](#destroy) |

<details class="api-operation" id="default" markdown="1">
<summary><code>DynamicFenwickTree2D&lt;T,Coord&gt; tree</code> — O(1)</summary>

注意点: `n=m=0` の空構造を作る。`pref(0,0)` と `sum(0,0,0,0)` は `T{}`。
更新・`get` が可能な点はない。

{% raw %}
```cpp
blueberry::DynamicFenwickTree2D<long long> empty;
assert(empty.pref(0, 0) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="construct" markdown="1">
<summary><code>DynamicFenwickTree2D&lt;T,Coord&gt; tree(Coord n, Coord m)</code> — O(1)</summary>

注意点: `n,m >= 0` の領域を全点ゼロで初期化。どちらかがゼロなら空で、領域内の和はゼロ。
最大の `Coord` も有効で初期メモリは O(1)。

{% raw %}
```cpp
blueberry::DynamicFenwickTree2D<long long> tree(1000000000, 1000000000);
assert(tree.pref(1000000000, 1000000000) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>DynamicFenwickTree2D copy(const DynamicFenwickTree2D&amp; other)</code> — O(1+S)</summary>

注意点: 全状態を独立にコピーし、追加 O(1+S) メモリを使う。更新・破棄は元とコピーで独立。

{% raw %}
```cpp
auto copy = tree;
assert(copy.pref(copy.height(), copy.width()) == tree.pref(tree.height(), tree.width()));
```
{% endraw %}
</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>DynamicFenwickTree2D&amp; tree.operator=(const DynamicFenwickTree2D&amp; other)</code> — O(1+S+S_old)</summary>

注意点: 旧 `S_old` ノードを置き換え、独立コピーを作る。返り値は `tree` 自身の参照。自己代入も有効。

{% raw %}
```cpp
blueberry::DynamicFenwickTree2D<long long> copy;
copy = tree;
```
{% endraw %}
</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>DynamicFenwickTree2D moved(DynamicFenwickTree2D&amp;&amp; other)</code> — O(1)</summary>

注意点: 格納領域の所有権を移す。元は `height()==width()==0` の空構造で、空問い合わせ・再代入が可能。
例には `<utility>` を含める。外部参照を貸し出す API はない。

{% raw %}
```cpp
auto moved = std::move(tree);
assert(tree.height() == 0 && tree.width() == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>DynamicFenwickTree2D&amp; tree.operator=(DynamicFenwickTree2D&amp;&amp; other)</code> — O(1+S_old)</summary>

注意点: 旧状態を解放し所有権を移す。元は空、返り値は `tree` の参照。自己 move は何もしない。

{% raw %}
```cpp
blueberry::DynamicFenwickTree2D<long long> moved;
moved = std::move(tree);
assert(tree.pref(0, 0) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="height" markdown="1">
<summary><code>Coord tree.height() const</code> — O(1)</summary>

注意点: x 座標の上限 `n` を返す。点数・保存セル数ではない。

{% raw %}
```cpp
auto n = tree.height();
assert(tree.pref(n, 0) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="width" markdown="1">
<summary><code>Coord tree.width() const</code> — O(1)</summary>

注意点: y 座標の上限 `m` を返す。点数ではない。

{% raw %}
```cpp
auto m = tree.width();
assert(tree.pref(0, m) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void tree.add(Coord x, Coord y, const T&amp; value)</code> — 平均・償却 O(LH)</summary>

注意点: `0 <= x < n, 0 <= y < m` に `value` を加算。未来の座標の事前登録は不要。
負の値・同一点の繰り返し加算が可能。新規ノードは最大 O(L)。値の加算オーバーフローに注意。
単一操作の最悪計算量・再確保の扱いは概要を参照。

{% raw %}
```cpp
tree.add(3, 7, -2);
```
{% endraw %}
</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>T tree.pref(Coord x, Coord y) const</code> — 平均 O(LH)</summary>

注意点: `0 <= x <= n, 0 <= y <= m`。`[0,x) × [0,y)` の和を値で返す。
どちらかの端点がゼロなら `T{}`。保存されていない点はゼロで、読み取り時の割当はない。

{% raw %}
```cpp
auto total = tree.pref(tree.height(), tree.width());
assert(tree.pref(0, tree.width()) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="sum" markdown="1">
<summary><code>T tree.sum(Coord left, Coord down, Coord right, Coord up) const</code> — 平均 O(LH)</summary>

注意点: `0 <= left <= right <= n`、`0 <= down <= up <= m`。
`[left,right) × [down,up)` の和を値で返す。幅または高さゼロなら `T{}`。
引数順は **左・下・右・上**。包除原理の減算を含む中間値も型の範囲内に収める。

{% raw %}
```cpp
assert(tree.sum(3, 7, 4, 8) == tree.get(3, 7));
assert(tree.sum(3, 7, 3, 8) == 0);
```
{% endraw %}
</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T tree.get(Coord x, Coord y) const</code> — 平均 O(LH)</summary>

注意点: `0 <= x < n, 0 <= y < m` の点の現在値を返す。未更新ならゼロ。
返り値はコピーで、値の変更が構造へ影響することはない。

{% raw %}
```cpp
auto value = tree.get(3, 7);
```
{% endraw %}
</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>tree.~DynamicFenwickTree2D()</code> — O(1+S)</summary>

注意点: スコープ終了時にハッシュ表・AVL ノードをすべて解放する。明示呼び出しは通常不要。

{% raw %}
```cpp
{ blueberry::DynamicFenwickTree2D<long long> temporary(5, 7); temporary.add(2, 3, 1); }
```
{% endraw %}
</details>

## 出典・検証

- [Nyaan: Dynamic Binary Indexed Tree 2D](https://nyaannyaan.github.io/library/data-structure-2d/dynamic-binary-indexed-tree-2d.hpp.html): 二重 Fenwick の比較対象。本実装は疎な外側・内側の AVL・値所有を独立に記述。
- [Library Checker: Point Add Rectangle Sum](https://judge.yosupo.jp/problem/point_add_rectangle_sum)、[Rectangle Sum](https://judge.yosupo.jp/problem/rectangle_sum): `verify/data-structure/dynamic-point-add-rectangle-sum.test.cpp` と `dynamic-rectangle-sum.test.cpp`。前者は入力を先読みせず順次処理する。
- [Fastest C++ 提出 206958](https://judge.yosupo.jp/submission/206958): 事前登録した点の wavelet 型構造・専用 I/O を調査。未知座標のオンライン追加という要件が異なるため、その手法へ置換していない。公開時間はローカル結果と直接比較しない。
- `tests/random/dynamic-fenwick-tree.cpp`: seed 指定可能な愚直列挙比較、負の値、最大整数座標、空長方形、昇順挿入、独立コピー・move。
