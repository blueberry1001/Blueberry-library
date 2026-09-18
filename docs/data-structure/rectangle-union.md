---
title: Rectangle Union Area
documentation_of: //blueberry/data-structure/rectangle-union.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

軸平行矩形の和集合の面積をx方向の走査と被覆長segment treeで求めます。各矩形は{left,bottom,right,top}。Nは矩形数、N<=INT_MAX/8。時間O(N log(N+1))、追加メモリO(N)。空入力・幅または高さ0の矩形の寄与は0。Coordは比較可能な整数型、Areaは座標・差・積・面積の全中間値が収まる整数型を指定してください。Areaの既定値はlong long。座標の差を取る前にAreaへ変換します。負座標・重複・接触・包含を許可。浮動小数の誤差モデルは保証しません。ACLの汎用木と異なり、矩形集合を直接受け取る専用処理です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <array>
#include <vector>
#include "blueberry/data-structure/rectangle-union.hpp"
int main() {
  std::vector<std::array<int,4>> r{{0,0,2,2},{1,1,3,3}};
  assert(blueberry::rectangle_union_area(r)==7);
  assert(blueberry::rectangle_union_area(std::vector<std::array<int,4>>{})==0);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Area rectangle_union_area<Area=long long, Coord>(const std::vector<std::array<Coord,4>>& rectangles)` | O(N log(N+1)) | [開く](#op-0) |

<details class="api-operation" id="op-0" markdown="1">
<summary><code>Area rectangle_union_area&lt;Area=long long, Coord&gt;(const std::vector&lt;std::array&lt;Coord,4&gt;&gt;&amp; rectangles)</code> — O(N log(N+1))</summary>

各配列はleft<=rightかつbottom<=top。和集合の面積を返し、入力を変更しません。

{% raw %}
```cpp
std::vector<std::array<long long,4>> r{{-2,-2,2,2}};
auto area=blueberry::rectangle_union_area<__int128_t>(r);
```
{% endraw %}

注意点: 不正な端点順序は禁止。Areaのオーバーフロー検出はしません。__int128_tはGNU拡張。参照は保持せず、返り値は独立した値です。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/area_of_union_of_rectangles) と整数格子の全セル比較で検証。[sotanishyの最小被覆数方式](https://sotanishy.github.io/cp-library-cpp/misc/rectangle_union.hpp.html)、[spaghetti-sourceのBentley走査](https://github.com/spaghetti-source/algorithm/blob/master/geometry/rectangle_union.cc) を調査。コードは独立実装。測定: [DS report](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-ds.md)。
