---
title: Convex Hull
documentation_of: //blueberry/geometry/convex-hull.hpp
---

[カテゴリへ戻る]({{ '/categories/geometry.html' | relative_url }})

## 概要・前提

平面上の整数座標の点集合から、凸包の頂点をAndrewのmonotone chain法で求めます。
ACLには凸包の実装がないため、座標の組を直接受け取る関数として提供します。
`N` は重複を含む入力点数、`H` は返される頂点数です。最悪時間は O(N log(N+1))、
作業用メモリは O(N) です。返り値の要素数は `H` ですが、再確保を抑えるために予約した領域を
返却時にも保持するので、その確保容量は O(N) になり得ます。

`Coord` は64bit以下の符号付き整数型で、各座標は `-(2^62-1)` 以上 `2^62-1` 以下を前提とします。
浮動小数点型・符号なし整数型・128bit整数型は対応しません。向きの判定にはGNU拡張の
`__int128` を使用します。**差を取る前に座標を128bitへ変換**するため、
32bitの `Coord` でも元の型での減算オーバーフローは起こりません。

`B=2^62-1` とすると、座標差の絶対値は `2B=2^63-2` 以下、外積の各積は
`(2B)^2 < 2^126`、その差の絶対値も `2(2B)^2 < 2^127` です。
この範囲では外積の全中間値が符号付き128bit整数に収まります。
assert有効時は、座標の範囲を1点だけの入力でも検査します。assert無効時にもこの前提を守ってください。

入力順は自由で、負座標・重複・一直線上の点を許可します。結果は重複のない凸包の頂点だけで、
辺の途中の点は除外します。3点以上の結果は反時計回りで、先頭は辞書順最小の点
（xが最小、同じxならyが最小）です。末尾に先頭を再掲しません。
空入力なら空、相異なる点が1個ならその1点、2個以上で全点が共線なら辞書順の両端2点を返します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/geometry/convex-hull.hpp"
int main() {
  using Point = std::pair<long long, long long>;
  std::vector<Point> points{{2, 2}, {0, 0}, {2, 0}, {0, 2}, {1, 1}, {1, 0}, {0, 0}};
  auto hull = blueberry::convex_hull(points);
  const std::vector<Point> expected{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
  assert(hull == expected);
  assert(points.size() == 7);  // lvalue入力は変更しない。
  assert(blueberry::convex_hull(std::vector<Point>{}).empty());
  const std::vector<Point> line{{2, 2}, {-2, -2}, {0, 0}};
  assert((blueberry::convex_hull(line) == std::vector<Point>{{-2, -2}, {2, 2}}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<std::pair<Coord, Coord>> convex_hull<Coord>(std::vector<std::pair<Coord, Coord>> points)` | 最悪 O(N log(N+1)) | [開く](#convex-hull) |

<details class="api-operation" id="convex-hull" markdown="1">
<summary><code>std::vector&lt;std::pair&lt;Coord, Coord&gt;&gt; convex_hull&lt;Coord&gt;(std::vector&lt;std::pair&lt;Coord, Coord&gt;&gt; points)</code> — O(N log(N+1))</summary>

`points` の各要素を `(x,y)` として、凸包の極端点の座標を返します。型引数 `Coord` は
通常は引数から推論されます。座標を辞書順にソートし、下側・上側の鎖をそれぞれ線形時間で構築します。
重複点と辺の途中の点を取り除き、反時計回り・辞書順最小点始まりの順序を保証します。
空・1点・全点共線の返り値は概要に記した通りです。

{% raw %}
```cpp
// <utility>, <vector> と convex-hull.hpp をincludeした関数内。
std::vector<std::pair<int, int>> points{{0, 0}, {1, 0}, {0, 1}};
auto hull = blueberry::convex_hull(std::move(points));
// hull == {{0, 0}, {1, 0}, {0, 1}}
```
{% endraw %}

注意点: 引数は値で受け取ります。lvalueを渡すと O(N) のコピーが発生し、元の配列とその参照は
変更・無効化されません。不要になった配列は `std::move(points)` で渡せますが、その後の
元の配列の内容は未規定です。関数は入力への参照を保持せず、返り値は独立して所有できます。
座標型と座標範囲は概要の制約を満たす必要があります。範囲外を結果や例外で通知するAPIではありません。
本関数は頂点の座標を返し、元の添字や同一座標の出現数を保持しません。
出力の隣接点は末尾から先頭へもつながりますが、`H<=2` では多角形の面積や辺を前提とする処理を分けてください。

</details>

## 出典・検証

[Library Checker: Static Convex Hull](https://judge.yosupo.jp/problem/static_convex_hull)
に対応する [verifyコード](https://github.com/blueberry1001/Blueberry-library/blob/main/verify/geometry/static-convex-hull.test.cpp) を用意しています。
ローカルテストでは空・重複・共線・座標境界を扱い、小さい点集合を独立した支持辺の列挙と比較します。

[KACTLのConvexHull](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/ConvexHull.h)
（Unlicense）と [maspypyのconvex_hull](https://github.com/maspypy/library/blob/main/geo/convex_hull.hpp)
を、monotone chainの構築順・重複/共線点の扱い・値と添字の返し方の調査資料としました。
コードはコピーせず、本APIの数値範囲と所有権に合わせて独立実装しています。
測定条件・比較候補・検証結果は [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/convex-hull.md) を参照してください。
