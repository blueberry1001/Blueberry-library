---
title: Offline Fenwick Tree 2D
documentation_of: //blueberry/data-structure/offline-fenwick-tree-2d.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

更新する座標を先に列挙できる場合の、点加算・矩形和です。ACLの1次元Fenwick Treeを
そのまま使えない疎な2次元座標に対応します。構築後は、更新と問い合わせを任意の順で処理できます。
**座標の追加はできません。** 未知の更新座標が逐次到着する用途には使えません。
旧 `DynamicFenwickTree2D.hpp` とは独立した、検証済みの新しい入口です。

`OfflineFenwickTree2D<T, Coord = long long>` の `T` は加法可換群（`T{}` が零、
`+=`, `+`, `-` とコピーを使用）、`Coord` は一貫した大小・等値比較ができる型です。
整数座標を推奨し、浮動小数点のNaNは不可です。負座標・重複座標を扱え、座標への±1演算はしません。
入力点数を $P$、重複を除いた点数を $N$ とし、$P,N < 2^{30}$ を前提にします。
構築は $O(P\log(P+1)+N\log^2(N+1))$、構築後メモリは $O(N\log(N+1))$。
構築中は入力バッファを含めて $O(P+N\log(N+1))$ です。重複除去後は入力の余剰容量を解放します。
更新・和は最悪 $O(\log^2(N+1))$、型の比較・算術は定数時間とします。
すべての途中の和・差が `T` に収まる必要があります。符号付き整数のオーバーフローは不可です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"
int main() {
  blueberry::OfflineFenwickTree2D<long long> bit({{1, 2}, {3, 4}, {1, 2}});
  bit.add(1, 2, 5);
  bit.add(3, 4, 7);
  assert(bit.size() == 2);
  assert(bit.sum(0, 0, 3, 4) == 5); // [0,3) × [0,4)
  assert(bit.pref(4, 5) == 12);     // x < 4, y < 5
  bit.add(1, 2, -2);
  assert(bit.sum(1, 2, 2, 3) == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `OfflineFenwickTree2D<T, Coord> bit(points)` | O(P log(P+1) + N log²(N+1)) | [開く](#construct) |
| `int bit.size()` | O(1) | [開く](#size) |
| `void bit.add(x, y, value)` | O(log²(N+1)) | [開く](#add) |
| `T bit.pref(x, y)` | O(log²(N+1)) | [開く](#pref) |
| `T bit.sum(left, down, right, up)` | O(log²(N+1)) | [開く](#sum) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>OfflineFenwickTree2D&lt;T, Coord&gt; bit(vector&lt;pair&lt;Coord, Coord&gt;&gt; points)</code> — O(P log(P+1) + N log²(N+1))</summary>

更新する全座標を渡します。順序は任意で、重複はまとめられます。重みはすべて零です。

{% raw %}
```cpp
blueberry::OfflineFenwickTree2D<long long, int> bit({{2, -1}, {8, 3}});
blueberry::OfflineFenwickTree2D<long long> empty({});
```
{% endraw %}

注意点: 空入力は有効です。引数は値で受け取り、`std::move(points)` で入力のコピーを避けられます。
構築後は入力の寿命に依存せず、通常のコピーは独立した状態を持ちます（O(N log(N+1))）。
参照を返す操作はなく、更新で利用者の参照が無効化されることはありません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int bit.size() const</code> — O(1)</summary>

重複を除いた登録点数を返します。更新回数やx座標の種類数ではありません。

{% raw %}
```cpp
int registered_points = bit.size();
```
{% endraw %}

注意点: 空構築なら0。加算によって登録点数は変わりません。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void bit.add(Coord x, Coord y, const T&amp; value)</code> — O(log²(N+1))</summary>

登録済みの点 `(x,y)` の重みに `value` を加えます。負の加算も可能です。

{% raw %}
```cpp
bit.add(1, 2, 5LL);
bit.add(1, 2, -5LL);
```
{% endraw %}

注意点: xとyがそれぞれ登録されているだけでは不十分で、**組 `(x,y)` 自体**を構築時に登録してください。
未登録点の更新は契約違反です（assertで検出）。空構築に対しては更新できません。
更新ごとの動的メモリ確保はありません。

</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>T bit.pref(Coord x, Coord y) const</code> — O(log²(N+1))</summary>

登録点のうち `point.x < x && point.y < y` の重みの総和を返します。

{% raw %}
```cpp
long long prefix = bit.pref(4, 5);
```
{% endraw %}

注意点: 境界は登録座標でなくてもよく、該当点がなければ `T{}` です。
最大整数座標上の点を含めたい場合は、その座標より大きい境界を表せる `Coord` を選んでください。
半開区間のため、境界そのものにある点は含みません。

</details>

<details class="api-operation" id="sum" markdown="1">
<summary><code>T bit.sum(Coord left, Coord down, Coord right, Coord up) const</code> — O(log²(N+1))</summary>

矩形 `[left,right) × [down,up)` の重みの総和を返します。引数の順番はLibrary Checkerと同じです。

{% raw %}
```cpp
long long rectangle = bit.sum(0, 0, 4, 5);
assert(bit.sum(0, 0, 0, 5) == 0);
```
{% endraw %}

注意点: `left <= right`, `down <= up` が必要です。幅または高さが0なら `T{}` を返します。
問い合わせ座標の事前登録は不要です。4つのprefixを包除するため、途中の差も `T` に収めてください。

</details>

## 出典・検証

- [Library Checker: Point Add Rectangle Sum](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
- `tests/random/offline-fenwick-tree-2d.cpp`: 点の全列挙との比較、負更新、重複、空矩形、整数座標の両端、コピーの独立性。
- 設計比較: [Nyaan FenwickRangeTree](https://github.com/NyaanNyaan/library/blob/master/data-structure-2d/fenwick-tree-on-range-tree.hpp) は登録点ごとの外側BIT、[suisen FenwickTree2D](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/datastructure/fenwick_tree/fenwick_tree_2d.hpp) は密な格子。本実装は異なるx座標だけを外側BITに使い、各節点のyを圧縮する標準アルゴリズムを独立実装しています。密な全座標分の確保や、更新時のハッシュ表・節点確保を避けます。
