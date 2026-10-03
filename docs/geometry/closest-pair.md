---
title: Closest Pair
documentation_of: //blueberry/geometry/closest-pair.hpp
---

[カテゴリへ戻る]({{ '/categories/geometry.html' | relative_url }})

## 概要・前提

平面上の整数座標の点集合から、Euclid距離が最小になる2点の元の添字を求めます。
ACLには最近点対の実装がないため、座標の組を直接受け取る関数として提供します。
`N` は重複を含む入力点数です。`N>=2` では、異なる入力要素の **0-indexed添字**
`(i,j)` を `0<=i<j<N` に正規化し、最小距離を達成する組のうち **辞書順最小** の組を返します。
辞書順ではまず `i` を比べ、同じなら `j` を比べます。座標の辞書順ではありません。
空入力または1点だけなら `{-1,-1}` を返します。この値で配列にアクセスしないでください。
負座標、重複、一直線上の点を許可します。重複点がある場合の最小距離は0で、
この場合にも元の添字の組が辞書順最小になるよう選びます。

入力点数は `N <= INT_MAX`、`Coord` は64bit以下の符号付き整数型で、
各座標は `-B` 以上 `B` 以下、`B=2^62-1` を前提とします。
浮動小数点型・符号なし整数型・128bit整数型には対応しません。
assert有効時は点数と座標範囲を検査し、座標は1点だけの入力でも検査します。
assert無効時にも同じ前提を守ってください。

距離は平方根を使わず、GNU拡張の符号付き `__int128` による二乗で比較します。
**差を取る前に座標を128bitへ変換**するため、32bitの `Coord` でも元の型での減算は溢れません。
座標差の絶対値は `D=2B=2^63-2` 以下なので、各差の二乗は `D^2` 以下で、`D^2 < 2^126` です。
距離の二乗の上界は `2D^2 = 2^127-2^66+8 < 2^127` で、
減算・二乗・和および候補を絞る比較を整数で正確に行えます。
符号付き64bit整数の全範囲を座標として受け付けるわけではありません。

最悪時間は O(N log(N+1))、作業用メモリは O(N)、そのうち再帰スタックは O(log(N+1)) です。
`N<=2` は範囲検査を含めて O(1) 時間・O(1) 作業用メモリで返します。
返り値は添字2個の O(1) で、入力を並べ替えたり変更したりしません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/geometry/closest-pair.hpp"
int main() {
  using Point = std::pair<long long, long long>;
  const std::vector<Point> points{{0, 0}, {1, 0}, {-1, 0}, {20, 20}};
  const auto before = points;
  auto [i, j] = blueberry::closest_pair(points);
  assert(i == 0 && j == 1);  // (0,2)も距離1だが、添字の組は(0,1)が小さい。
  assert(points == before);

  const std::vector<Point> duplicates{{9, 9}, {1, 1}, {1, 1}, {9, 9}};
  assert((blueberry::closest_pair(duplicates) == std::pair<int, int>{0, 3}));
  assert((blueberry::closest_pair(std::vector<Point>{}) == std::pair<int, int>{-1, -1}));
  assert((blueberry::closest_pair(std::vector<Point>{{7, -2}}) == std::pair<int, int>{-1, -1}));

  auto movable = points;
  assert((blueberry::closest_pair(std::move(movable)) == std::pair<int, int>{0, 1}));
  assert(movable == points);  // const参照で受け取るため、std::moveしても消費しない。
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::pair<int, int> closest_pair<Coord>(const std::vector<std::pair<Coord, Coord>>& points)` | 最悪 O(N log(N+1))、`N<=2` は O(1) | [開く](#closest-pair) |

<details class="api-operation" id="closest-pair" markdown="1">
<summary><code>std::pair&lt;int, int&gt; closest_pair&lt;Coord&gt;(const std::vector&lt;std::pair&lt;Coord, Coord&gt;&gt;&amp; points)</code> — O(N log(N+1))</summary>

`points` の各要素を `(x,y)` として、最近点対の元の添字を返します。
型引数 `Coord` は通常は引数から推論されます。返り値は座標や距離ではありません。
`N=2` なら `{0,1}`、全点が同じ座標で `N>=2` なら `{0,1}` です。
同じ最小距離の組が複数あるときも、入力順で決まる添字の辞書順最小を返します。

内部では座標と元の添字を作業用配列へコピーし、`(x,y,添字)` の順で一度整列します。
同じ座標の各グループから添字が最小の2個を取り出し、距離0の候補全体の辞書順最小を選びます。
重複がなければ、x順の配列を半分ずつに分ける決定的な分割統治を行います。
3点以下の部分問題は全点対を調べてy順に整列し、それ以外は左右から返るy順の列を
共通の作業配列へマージします。点のコピー用vectorと、マージ・候補走査で共有するvectorを
それぞれ1回だけ確保し、各再帰呼び出しで全体を再整列したり、別の配列を確保したりしません。

左右の探索後に保持している最短距離の二乗を `d` とし、分割線からのx方向の差の二乗が `d` 以下の点を調べます。
y順の候補についても、y方向の差の二乗が現在の `d` 以下の間を調べます。
**どちらの境界にも等号を含める**ことで、既知の最小距離と同じで添字の組が小さい候補を落としません。
重複は先に処理済みなので、この走査での距離は正です。
候補走査の開始時の `d` を `d0`、その距離を `δ=sqrt(d0)>0` として固定して考えます。
左右それぞれの内部では、2点の距離が `δ` 以上あります。
走査中に `d` が小さくなって以前の候補が残っていても、各候補のx座標は分割線から `δ` 以内で、
比較する点のy座標差も `δ` 以下です。この幅 `2δ`・高さ `δ` の領域を
例えば一辺 `δ/2` のマスに分けると、同じマス内の距離は `δ` 未満なので、各マスに入る点は
左右それぞれ高々1個で、1点と比較する候補数は境界上の点を含めても定数個です。
ここでの平方根は計算量の説明に使う記号で、実装が浮動小数点演算を行うわけではありません。
したがって候補走査とマージは各再帰段で線形となり、全体で最悪 O(N log(N+1)) です。

{% raw %}
```cpp
// <utility>, <vector> と closest-pair.hpp をincludeした関数内。
const std::vector<std::pair<int, int>> points{{0, 0}, {3, 0}, {0, 4}};
auto [i, j] = blueberry::closest_pair(points);  // i == 0, j == 1
const __int128 dx = static_cast<__int128>(points[i].first) - points[j].first;
const __int128 dy = static_cast<__int128>(points[i].second) - points[j].second;
const __int128 squared_distance = dx * dx + dy * dy;  // 9
```
{% endraw %}

注意点: 引数はconst参照です。lvalueだけでなく `std::move(points)` や一時的なvectorを渡しても、
関数が入力の要素を移動して消費することはありません。元の配列の参照・イテレータは無効化されず、
関数は入力への参照を保持しません。後から元の配列を並べ替えたり要素を削除したりすると、
返した添字が同じ点を指すとは限りません。一時的なvectorは呼び出し後に寿命が終わるため、
返した添字から座標も参照したいときは入力を手元に保持してください。
距離を自分で計算する場合も、上の例のように減算の前に128bitへ変換してください。
最小距離の二乗でも64bitに収まるとは限りません。範囲外の入力を結果や例外で通知するAPIではありません。

</details>

## 出典・検証

対応する公式問題は [Library Checker: Closest Pair](https://judge.yosupo.jp/problem/closest_pair) です。
[公式checker](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/closest_pair/checker.cpp)
は異なる添字で最小距離を達成しているかを検査します。公式問題の座標範囲は絶対値 `10^9` 以下で、
同距離の組の辞書順最小は要求しません。そのため、本APIの辞書順の保証、空・1点、
より広い座標境界と入力の保持は、公式問題に加えてローカルテストで確認する対象です。

[Library Checkerの分割統治による参照解答](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/closest_pair/sol/correct.cpp) と
[KACTLのClosestPair](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/ClosestPair.h)
を、y順のマージと候補の絞り込み、走査線法との違いを調べる資料としました。
KACTLは走査線と順序付き集合を使い、距離の平方根で探索窓を決めて座標の組を返します。
これらの参照実装と本APIでは、座標範囲、同距離の場合の選び方、返り値の契約が異なります。
ライブラリ本体へコードをコピーせず、整数での比較、添字の辞書順、共通作業配列を使う契約に合わせて独立実装しています。
