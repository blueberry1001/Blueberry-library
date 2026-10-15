---
title: Weighted Wavelet Matrix
documentation_of: //blueberry/data-structure/weighted-wavelet-matrix.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

固定した値列に対し、重みの一点加算と、添字区間・値域を指定した重み和を扱います。値の変更・挿入・削除はできません。Valueの `<` と `==` は整合する全順序、WeightはWeight{}を加法単位元とする可換群が必要で、負の重みも使えます。

Nは要素数、Sは異なる値の数、L=max(1,ceil(log2(max(1,S))))。構築はO(N log(N+1))、更新・値域付き和はO(L log(N+1))、メモリはO(N L)です。計算量は最悪時間です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`WM`](#default-constructor) | O(1) |
| [`WM`](#constructor) | O(N log(N+1)) |
| [`WM`](#copy-constructor) | O(N L) |
| [`WM`](#move-constructor) | O(1) |
| [`operator=`](#copy-assignment) | O(N L + M H) |
| [`operator=`](#move-assignment) | O(1 + M H) |
| [`wm.size`](#size) | O(1) |
| [`wm.add`](#add) | O(L log(N+1)) |
| [`wm.sum`](#sum-all) | O(log(N+1)) |
| [`wm.sum`](#sum-upper) | O(L log(N+1)) |
| [`wm.sum`](#sum-range) | O(L log(N+1)) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 更新しない静的な重み和にも使えます。点群をx順に並べ、yを値列にすると長方形和になります。将来追加する座標もあらかじめ登録し、重み0から加算できます。
- 頻度・順位には既存WaveletMatrix、座標指定の更新にはOfflineFenwickTree2Dが適します。本実装は添字で更新し、各段の座標二分探索をrankに置き換えます。
- ACLに同等のWavelet Matrixはありません。内部には線形構築・constクエリを備えた既存FenwickTreeを再利用します。
- `0 <= N <= INT_MAX/2`。空列・重複値を許します。Weightにはコピー・`+=`・減算が必要です。演算・比較・コピーをO(1)とみなします。
- 整数の中間和も含めて型の範囲内に収めてください。値自体への加減算はせず、符号付き整数の最小値・最大値も格納できます。
- 入力をコピーし、入力の寿命・変更に依存しません。内部参照・公開フィールドはありません。

## 操作の補足 {#操作の補足}

- `WM` は `blueberry::WeightedWaveletMatrix<Value, Weight = long long>` です。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/weighted-wavelet-matrix.hpp"
int main() {
  blueberry::WeightedWaveletMatrix<int> wm({3, 1, 3, 8}, {10, 20, 30, 40});
  assert(wm.sum(0, 4, 2, 4) == 40);
  wm.add(2, -5);
  assert(wm.sum(1, 4, 4) == 45);
  assert(wm.sum(0, 4) == 95);
  assert(wm.sum(2, 2, 0, 10) == 0);
}
```
{% endraw %}

<details class="api-operation" id="default-constructor" markdown="1">
<summary><code>WM()</code> — O(1)</summary>

空列を構築します。`WM wm; assert(wm.size() == 0);`。
注意点: `sum(0,0)` は重みの単位元です。空列への `add` はできません。


{% raw %}
```cpp
WM wm; assert(wm.size() == 0);
```
{% endraw %}

</details>

<details class="api-operation" id="constructor" markdown="1">
<summary><code>WM(const vector&lt;Value&gt;&amp; a, const vector&lt;Weight&gt;&amp; w)</code> — O(N log(N+1))</summary>

`a.size() == w.size()` が必要です。`a[i]` に初期重み `w[i]` を持たせます。
`WM wm({2, 2}, {3, 4});` は異なる添字を保ち、同値でも別々に更新できます。
注意点: 両引数をコピーします。空列も有効です。暗黙のコピーは O(N L)、ムーブ元は再代入または破棄してください。


{% raw %}
```cpp
WM wm({2, 2}, {3, 4});
```
{% endraw %}

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int wm.size() const</code> — O(1)</summary>

要素数を返します。`assert(wm.size() == 4);`。

注意点: 重複値を含み、更新しても変わりません。空列なら 0 です。


{% raw %}
```cpp
int n = wm.size();
```
{% endraw %}

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void wm.add(int p, const Weight&amp; delta)</code> — O(L log(N+1))</summary>

`0 <= p < N` に対し `w[p] += delta`。`wm.add(2, -5);`。
注意点: 値 `a[p]` は変わりません。負の加算・ゼロ加算も有効で、総和のオーバーフローに注意してください。


{% raw %}
```cpp
wm.add(2, -5);
```
{% endraw %}

</details>

<details class="api-operation" id="sum-all" markdown="1">
<summary><code>Weight wm.sum(int l, int r) const</code> — O(log(N+1))</summary>

`0 <= l <= r <= N` の全重み和。`auto total = wm.sum(0, wm.size());`。
注意点: 値域制限はなく、値型の最大値の要素も含みます。空区間は `Weight{}`。


{% raw %}
```cpp
auto total = wm.sum(0, wm.size());
```
{% endraw %}

</details>

<details class="api-operation" id="sum-upper" markdown="1">
<summary><code>Weight wm.sum(int l, int r, const Value&amp; upper) const</code> — O(L log(N+1))</summary>

`0 <= l <= r <= N` 内の `a[i] < upper` の重み和。
`auto result = wm.sum(0, 4, 4);`。境界は登録値でなくてもよく、空区間・該当なしは `Weight{}`。
注意点: 上端は含みません。最大値を含めるために `upper + 1` を計算するとオーバーフローし得ます。


{% raw %}
```cpp
auto result = wm.sum(0, 4, 4);
```
{% endraw %}

</details>

<details class="api-operation" id="sum-range" markdown="1">
<summary><code>Weight wm.sum(int l, int r, const Value&amp; low, const Value&amp; upper) const</code> — O(L log(N+1))</summary>

`0 <= l <= r <= N`、`low <= upper`。`low <= a[i] < upper` の重み和。
`auto result = wm.sum(0, 4, 2, 4);`。空添字区間・空値域・該当なしは `Weight{}`。
注意点: 重みが負なら結果も負になり得ます。返り値・中間和のオーバーフローに注意してください。


{% raw %}
```cpp
auto result = wm.sum(0, 4, 2, 4);
```
{% endraw %}

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>WM(const WM&amp; other)</code> — O(N L)</summary>

暗黙のコピーコンストラクタは値列・現在の重み・索引を複製します。

{% raw %}
```cpp
WM copy(wm);
```
{% endraw %}

注意点: N, L はコピー元の大きさです。以後、一方の更新は他方に影響しません。空列もコピーできます。
</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>WM(WM&amp;&amp; other)</code> — O(1)</summary>

暗黙のムーブコンストラクタは内部領域の所有権を移します。

{% raw %}
```cpp
// #include <utility>
WM moved(std::move(wm));
```
{% endraw %}

注意点: ムーブ元は破棄または再代入してから使用してください。ムーブ先では移動前と同じクエリ・更新が可能です。
</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>WM&amp; wm.operator=(const WM&amp; other)</code> — O(N L + M H)</summary>

暗黙のコピー代入は現在の内容を置き換え、`*this` を返します。
N, L はコピー元、M, H は代入前の代入先の要素数と層数に対応します。

{% raw %}
```cpp
WM copy;
copy = wm;
```
{% endraw %}

注意点: コピー後は独立して更新できます。自己コピー代入も有効です。旧領域の破棄・再利用のためのコストも上限に含みます。
</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>WM&amp; wm.operator=(WM&amp;&amp; other)</code> — O(1 + M H)</summary>

暗黙のムーブ代入は旧内容を破棄し、内部領域の所有権を移して `*this` を返します。
M, H は代入前の代入先の要素数と層数に対応します。

{% raw %}
```cpp
// #include <utility>
WM moved;
moved = std::move(wm);
```
{% endraw %}

注意点: ムーブ元は破棄または再代入してから使用してください。自己ムーブ代入後も同じ制限があります。旧要素の破棄を計算量に含みます。
</details>
<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Luzhiled: Wavelet Matrix Rectangle Sum](https://ei1333.github.io/library/structure/wavelet/wavelet-matrix-rectangle-sum.hpp.html): 静的累積和を各段に保持する設計。
- [maspypy: Wavelet Matrix 2D Range](https://maspypy.github.io/library/ds/wavelet_matrix/wavelet_matrix_2d_range.hpp): x 順整列と可換演算の補助構造で重み更新に対応する設計。
- 上記を設計比較として参照し、コードは既存 Blueberry の rank 表現を用いて実装しました。
- [Library Checker: Point Add Rectangle Sum](https://judge.yosupo.jp/problem/point_add_rectangle_sum)、`verify/data-structure/weighted-wavelet-matrix.test.cpp`。
- `tests/random/weighted-wavelet-matrix.cpp`: 固定 seed、空・同値・64 bit 境界・極値・負重み・更新を愚直解と比較。

</details>
