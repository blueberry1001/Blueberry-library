---
title: Furthest Pair
documentation_of: //blueberry/geometry/furthest-pair.hpp
---

[カテゴリへ戻る]({{ '/categories/geometry.html' | relative_url }})

## 概要・前提

平面上の整数座標の点集合から、Euclid距離が最大になる2点の元の添字を求めます。
ACLには最遠点対の実装がないため、座標の組を直接受け取る関数として提供します。
`N` は重複を含む入力点数、`H` は相異なる凸包の頂点数です。
`N<=2` なら O(1)、`N>=3` で全点が同じ座標なら O(N) 時間で、凸包を作らずに返します。
それ以外の入力では、先頭の点と異なる座標が見つかるまでの確認に最悪 O(N) 時間を使った後、
内部で [Convex Hull]({{ '/blueberry/geometry/convex-hull.hpp.html' | relative_url }}) を求めます。
凸包上を回転キャリパー法で走査し、入力から元の添字を復元します。
凸包の構築は O(N log(N+1))、キャリパーの走査は O(H)、添字の復元は O(N) で、
全体の最悪時間は O(N log(N+1)) です。作業用メモリは O(N) ですが、
凸包を作らずに返す場合は O(1) です。返り値は添字2個の O(1) です。

入力点数は `N <= INT_MAX`、`Coord` は64bit以下の符号付き整数型で、
各座標は `-(2^62-1)` 以上 `2^62-1` 以下を前提とします。
浮動小数点型・符号なし整数型・128bit整数型には対応しません。
向き・面積の変化・距離の二乗の比較にはGNU拡張の `__int128` を使用し、平方根は計算しません。
**差を取る前に座標を128bitへ変換**するため、32bitの `Coord` でも元の型での減算は溢れません。

`B=2^62-1`、`D=2B=2^63-2` とすると、座標差の絶対値は `D` 以下です。
外積・距離の二乗に現れる各積の絶対値は `D^2` 以下で、`D^2 < 2^126` です。
2つの積の差や和の絶対値は `2D^2` 以下で、`2D^2 = 2^127-2^66+8 < 2^127` です。
キャリパーでは面積の増減を「凸包の辺ベクトル」と「支持点から次の頂点へのベクトル」の
外積の符号で判定するため、同じ上界が適用できます。座標の減算から積・和・差まで、
すべての中間値が符号付き128bit整数の範囲内です。
assert有効時は点数と座標範囲を検査し、座標は1点だけの入力でも検査します。
assert無効時にもこれらの前提を守ってください。

入力順は自由で、負座標・重複・一直線上の点を許可します。`N>=2` では、
元の配列の **0-indexed添字** `(i,j)` を `0<=i<j<N` に正規化して返します。
最大距離を達成する組が複数ある場合はそのうち任意の1組で、辞書順最小などの保証はありません。
全点が同じ座標でも相異なる元の添字を返します。
相異なる座標が2個以上あり全点が共線なら、両端の点を選びます。
空入力または1点だけなら `{-1,-1}` を返します。この値で配列にアクセスしないでください。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/geometry/furthest-pair.hpp"
int main() {
  using Point = std::pair<long long, long long>;
  const std::vector<Point> points{{1, 1}, {-2, -3}, {4, 5}, {0, 0}};
  const auto before = points;
  auto [i, j] = blueberry::furthest_pair(points);
  assert(i == 1 && j == 2);  // 距離の二乗は6*6+8*8=100。
  assert(points == before);
  assert((blueberry::furthest_pair(std::vector<Point>{}) == std::pair<int, int>{-1, -1}));
  assert((blueberry::furthest_pair(std::vector<Point>{{7, -2}}) == std::pair<int, int>{-1, -1}));
  const std::vector<Point> same{{7, -2}, {7, -2}};
  assert((blueberry::furthest_pair(same) == std::pair<int, int>{0, 1}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::pair<int, int> furthest_pair<Coord>(const std::vector<std::pair<Coord, Coord>>& points)` | 最悪 O(N log(N+1))、`N<=2` は O(1)、全同一座標は O(N) | [開く](#furthest-pair) |

<details class="api-operation" id="furthest-pair" markdown="1">
<summary><code>std::pair&lt;int, int&gt; furthest_pair&lt;Coord&gt;(const std::vector&lt;std::pair&lt;Coord, Coord&gt;&gt;&amp; points)</code> — O(N log(N+1))</summary>

`points` の各要素を `(x,y)` として、最大距離を達成する異なる入力要素の添字を返します。
型引数 `Coord` は通常は引数から推論されます。返り値は座標や距離ではなく元の配列の添字です。
`N<=2` または全点が同じ座標なら、点数と座標から直接答えを返します。
それ以外では、最大距離は凸包の頂点の組でも達成されるため、重複点や辺の途中の点を取り除いた凸包を使います。
頂点が3個以上なら、各辺を底辺とする三角形の面積が最大になる反対側の支持点を求めます。
辺を反時計回りに進めると支持点も同じ向きに進むので、全辺を通じた支持点の移動は O(H) です。
支持辺が平行で面積が等しくなる場合は、反対側の辺の両端を候補に含めます。
凸包上の距離は一般に単峰とは限らないため、支持点を進める判定には面積の増減を使います。
空・1点・重複・全点共線の返り値は概要に記した通りです。

{% raw %}
```cpp
// <utility>, <vector> と furthest-pair.hpp をincludeした関数内。
const std::vector<std::pair<int, int>> points{{0, 0}, {3, 0}, {0, 4}};
auto [i, j] = blueberry::furthest_pair(points);  // i == 1, j == 2
const __int128 dx = static_cast<__int128>(points[i].first) - points[j].first;
const __int128 dy = static_cast<__int128>(points[i].second) - points[j].second;
const __int128 squared_distance = dx * dx + dy * dy;  // 25
```
{% endraw %}

注意点: 引数はconst参照で受け取り、入力を変更しません。凸包を作る場合は内部コピーから構築します。
呼び出しによって元の配列の参照やイテレータは無効化されず、関数は入力への参照を保持しません。
返り値は独立した整数の組ですが、後から元の配列を並べ替えたり要素を削除したりすると、
同じ添字が同じ点を指すとは限りません。距離を求める場合も、上の例のように
**減算の前に128bitへ変換**してください。最大距離の二乗は64bitに収まるとは限りません。
点数・座標型・座標範囲は概要の制約を満たす必要があり、範囲外を結果や例外で通知するAPIではありません。
同じ最大距離の組が複数ある入力で別実装と比較するときは、距離の二乗と `0<=i<j<N` を確認してください。
添字を昇順に正規化しても、選ばれる組が辞書順最小になる保証はありません。

</details>

## 出典・検証

[Library Checker: Furthest Pair](https://judge.yosupo.jp/problem/furthest_pair)
に対応する [verifyコード](https://github.com/blueberry1001/Blueberry-library/blob/main/verify/geometry/furthest-pair.test.cpp) を用意しています。
ローカルテストでは空・重複・共線・平行な支持辺・座標境界を扱い、小さい点集合を全点対の列挙と比較します。

[KACTLのHullDiameter](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/HullDiameter.h)、
[maspypyのfurthest_pair](https://github.com/maspypy/library/blob/main/geo/furthest_pair.hpp)、
[Library Checkerの参照解答](https://github.com/yosupo06/library-checker-problems/blob/1814c4e/geo/furthest_pair/sol/correct.cpp)
を、回転キャリパーの進め方・同距離や平行な支持辺の扱い・元の添字の復元の調査資料としました。
コードはコピーせず、既存の `convex_hull` と本APIの数値範囲・添字契約に合わせて独立実装しています。
測定条件・比較候補・検証状況は [開発レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/furthest-pair.md) を参照してください。
