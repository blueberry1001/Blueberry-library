---
title: Li Chao Tree
documentation_of: //blueberry/data-structure/li-chao-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::LiChaoTree<T = long long>` は直線 `y = ax + b` と、半開区間にだけ有効な線分を追加し、指定座標での最小値を求めます。ACLに対応する機能はありません。
クエリ座標を先に収集する座標圧縮方式です。挿入順・傾きの順序は自由ですが、登録後の座標追加や直線の削除はできません。未知の座標をオンラインで問い合わせる場合は [Dynamic Li Chao Tree]({{ '/blueberry/data-structure/dynamic-li-chao-tree.hpp.html' | relative_url }}) を使います。

入力座標数を M、重複除去後を N とします。メモリは O(N)、構築中の入力を含めたピークは O(M + N) です。内部に最大 4N 個の `optional<Line>` を確保し、更新による追加確保はありません。N は `INT_MAX / 4` 以下で、確保可能なメモリに収まる必要があります。以下の対数は空入力を含め `log(N + 1)` と表記し、計算量は償却ではなく最悪です。

T はコピー可能な順序付き数値型で、`a * x + b` と大小比較が正確に行える必要があります。通常は符号付き整数を使います。**積 `a * x` と和 `a * x + b` の両方が T に収まること**が前提です。線分の場合はその有効範囲内の登録座標、直線の場合はすべての登録座標でこの条件が必要です。整数の自動拡張や飽和処理はありません。浮動小数点の丸め誤差・NaNや、剰余演算の値域では正しさを保証しません。

空の直線集合の答えは `std::nullopt` です。数値の最大値・最小値も有効な答えとして区別でき、INF番兵を予約する必要はありません。公開フィールドはありません。座標は所有コピー、返り値も値なので、元の座標配列や木を更新しても取得済みの答えの寿命に影響しません。木のコピーは独立した O(N) のコピーです。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/li-chao-tree.hpp"
int main() {
  blueberry::LiChaoTree<long long> tree({3, -2, 0, 3});
  assert(tree.size() == 3);
  assert(!tree.query(0));
  tree.add_line(2, 1);
  tree.add_segment(0, 3, -1, -4);
  assert(tree.query(-2) == -3);
  assert(tree.query(0) == -4);
  assert(tree.query(3) == 7);  // 右端3は線分の範囲外
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LiChaoTree<T> tree(std::vector<T> xs)` | O(M log(M + 1)) | [開く](#constructor) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `void tree.add_line(T a, T b)` | O(log(N + 1)) | [開く](#add-line) |
| `void tree.add_segment(T l, T r, T a, T b)` | O(log²(N + 1)) | [開く](#add-segment) |
| `std::optional<T> tree.query(T x) const` | O(log(N + 1)) | [開く](#query) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>LiChaoTree&lt;T&gt; tree(std::vector&lt;T&gt; xs)</code> — O(M log(M + 1))</summary>

クエリに現れる座標の集合を受け取り、ソート・重複除去して所有します。直線集合は空で初期化します。入力配列の順序と重複は任意で、呼び出し元の配列は変更しません。

{% raw %}
```cpp
blueberry::LiChaoTree<long long> tree({5, 0, 5, -3});
blueberry::LiChaoTree<long long> empty({});
```
{% endraw %}

注意点: 空座標は許可され、追加操作は何も変更しません。その木に有効な query はありません。N の上限は `INT_MAX / 4` です。登録した座標の変更操作はありません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

重複を除いた登録座標の個数 N を返します。直線の本数ではありません。

{% raw %}
```cpp
blueberry::LiChaoTree<> tree({1, 1, 4});
assert(tree.size() == 2);
```
{% endraw %}

注意点: 空入力では 0 です。直線・線分を追加しても値は変わりません。

</details>

<details class="api-operation" id="add-line" markdown="1">
<summary><code>void tree.add_line(T a, T b)</code> — O(log(N + 1))</summary>

すべての登録座標に有効な直線 `ax + b` を追加します。傾き a と切片 b の順です。返り値はありません。

{% raw %}
```cpp
blueberry::LiChaoTree<> tree({-1, 2});
tree.add_line(3, 4);
assert(tree.query(2) == 10);
```
{% endraw %}

注意点: 同じ直線や傾きが同じ直線を何度でも追加できます。更新は単調に最小値を改善します。N = 0 では何もしません。全登録座標で積と和が T に収まる必要があります。

</details>

<details class="api-operation" id="add-segment" markdown="1">
<summary><code>void tree.add_segment(T l, T r, T a, T b)</code> — O(log²(N + 1))</summary>

元の座標で `l <= x < r` を満たす登録座標だけに有効な `ax + b` を追加します。引数は **左端・右端・傾き・切片** の順で、l と r は圧縮後の添字ではありません。返り値はありません。

{% raw %}
```cpp
blueberry::LiChaoTree<> tree({0, 3, 8});
tree.add_segment(1, 8, 2, 5);
assert(!tree.query(0));
assert(tree.query(3) == 11);
assert(!tree.query(8));
```
{% endraw %}

注意点: `l <= r` が前提です。端点の事前登録は不要で、登録座標の最小値・最大値の外でも構いません。空区間、登録点を含まない区間、N = 0 では何もしません。有効範囲内の登録点で積と和が T に収まる必要があります。半開区間なので型の最大値そのものを線分の左端だけで覆うことはできませんが、`add_line` はその座標にも有効です。

</details>

<details class="api-operation" id="query" markdown="1">
<summary><code>std::optional&lt;T&gt; tree.query(T x) const</code> — O(log(N + 1))</summary>

x を覆う追加済み直線・線分の最小値を返します。1本もなければ `std::nullopt` を返します。座標探索の二分探索を含む計算量です。

{% raw %}
```cpp
blueberry::LiChaoTree<> tree({6});
assert(!tree.query(6));
tree.add_line(0, 9);
const auto answer = tree.query(6);
assert(answer && *answer == 9);
```
{% endraw %}

注意点: **x は構築時に登録した座標であること**が前提で、未登録の座標は assert 対象です。空の座標集合では呼べません。状態を変更せず、返す optional は独立した値です。値を取り出す前に存在を確認してください。

</details>

## 出典・検証

設計調査には [Nyaan の座標圧縮 Li Chao Tree](https://nyaannyaan.github.io/library/segment-tree/li-chao-tree.hpp) と [Luzhiled の Dynamic Li Chao Tree](https://ei1333.github.io/library/structure/convex-hull-trick/dynamic-li-chao-tree.hpp.html) を参照しました。前者の配列配置と後者の動的座標領域を比較し、コードの転載ではなく、直線の交差回数を使うアルゴリズムから独立して実装しています。配列方式は事前に座標を集める必要がある一方、更新ごとのメモリ確保がなく、数値領域の幅ではなく座標数に依存します。本実装では番兵や座標末尾への加算によるパディングを使いません。

- [Line Add Get Min](https://judge.yosupo.jp/problem/line_add_get_min): 任意順の直線追加と最小値取得。
- [Segment Add Get Min](https://judge.yosupo.jp/problem/segment_add_get_min): 半開区間への線分追加と空の答え。
- `tests/random/li-chao-tree.cpp`: 再現可能な seed による全点の愚直比較、重複・単一点・空座標、等しい傾き、空区間、範囲外端点、型の最大値・最小値とコピーの独立性。
- 性能比較の再現方法・入力・実測値は `benchmark/li-chao-tree.md` を参照してください。
