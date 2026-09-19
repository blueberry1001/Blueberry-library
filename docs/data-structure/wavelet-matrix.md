---
title: Wavelet Matrix
documentation_of: //blueberry/data-structure/wavelet-matrix.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

静的配列の区間 k 番目・頻度・前後要素を求める、ACL にないデータ構造です。
値を内部で座標圧縮するため、ビット幅や符号変換を指定する必要はありません。
要素数を N、異なる値の数を σ、L = max(1, ceil(log2(max(1, σ)))) とします。
構築 O(N log(N+1))、各クエリ O(L)、size は O(1)（すべて最悪計算量）です。

64 bit のビット列とブロック単位の累積 popcount を各段に保存します。
保持メモリは O(N + L ceil((N+1)/64)) ワード、構築時の補助メモリは O(N)。
値辞書の vector は入力サイズ分の容量を持ち得るため O(N) を含めています。
圧縮時は元位置を値順に並べ、順位を元位置へ書き戻します。
小配列では比較sortを使い、それ以外ではunsigned型で最小値との差を取り、必要な8bit桁だけradix分配します。
radixのbucket初期化が小さい全幅入力で支配的になるため、N<64またはNが必要桁数の128倍未満なら比較sortを選びます。
符号付きオーバーフローを起こさず、符号付き最小値・最大値や重複も同じ順序で扱います。
型のbit幅を8で割って切り上げた数をBとすると圧縮のradix部分は O(BN)、小配列の比較sortは O(N log(N+1))。
標準整数型の幅Bを定数と数えると、全体の構築計算量は上記の上界を満たします。
各要素の二分探索を省き、2本の位置配列を段構築のpartition bufferとして再利用します。
T の比較・コピーと 64 bit popcount を O(1) とします。CPU 固有命令の指定は不要です。

すべての添字区間に `0 <= l <= r <= N` が必要です。値は更新できません。
圧縮時の差分はunsignedのmodulo演算で求め、段構築は圧縮後の非負整数をビット操作します。
範囲外引数は契約違反（debug build では assert）。公開フィールドはありません。
暗黙のコピーは独立した配列を所有し、時間・追加メモリとも保持メモリ量に比例します。
ムーブ元は破棄または再代入のみ行ってください。すべての取得結果は値で返し、参照の無効化はありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"
int main() {
  blueberry::WaveletMatrix<long long> wm(std::vector<long long>{3, -1, 3, 7});
  assert(wm.size() == 4);
  assert(wm.get(1) == -1);
  assert(wm.kth_smallest(0, 4, 1) == 3);
  assert(wm.kth_largest(0, 4, 0) == 7);
  assert(wm.count(0, 4, 3) == 2);
  assert(wm.range_freq(0, 4, 3) == 1);
  assert(wm.range_freq(0, 4, -1, 3) == 1);
  assert(wm.prev_value(0, 4, 3).value() == -1);
  assert(wm.next_value(0, 4, 4).value() == 7);
  assert(!wm.next_value(0, 0, 0));
}
```
{% endraw %}

## 操作一覧

以下の操作例は最小使用例の `wm` を前提とします。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `WaveletMatrix<T> wm` | O(1) | [開く](#default) |
| `WaveletMatrix<T> wm(const std::vector<T>& a)` | O(N log(N+1)) | [開く](#construct) |
| `int wm.size() const` | O(1) | [開く](#size) |
| `T wm.get(int p) const` | O(L) | [開く](#get) |
| `T wm.kth_smallest(int l, int r, int k) const` | O(L) | [開く](#kth-smallest) |
| `T wm.kth_largest(int l, int r, int k) const` | O(L) | [開く](#kth-largest) |
| `int wm.count(int l, int r, const T& x) const` | O(L) | [開く](#count) |
| `int wm.range_freq(int l, int r, const T& upper) const` | O(L) | [開く](#freq-upper) |
| `int wm.range_freq(int l, int r, const T& lower, const T& upper) const` | O(L) | [開く](#freq-range) |
| `std::optional<T> wm.prev_value(int l, int r, const T& upper) const` | O(L) | [開く](#prev) |
| `std::optional<T> wm.next_value(int l, int r, const T& lower) const` | O(L) | [開く](#next) |

<details class="api-operation" id="default" markdown="1">
<summary><code>WaveletMatrix&lt;T&gt; wm</code> — O(1)</summary>

空配列を構築します。`size()==0` であり、空区間の頻度・前後探索を実行できます。

{% raw %}
```cpp
blueberry::WaveletMatrix<int> empty;
assert(empty.count(0, 0, 42) == 0);
```
{% endraw %}

注意点: `get` と k 番目の取得は要素が必要です。

</details>

<details class="api-operation" id="construct" markdown="1">
<summary><code>WaveletMatrix&lt;T&gt; wm(const std::vector&lt;T&gt;& a)</code> — O(N log(N+1))</summary>

配列をコピーして構築します。`T` は `bool` 以外の標準整数型、`0 <= a.size() <= INT_MAX`。負数・重複・整数型の最小値と最大値を扱います。

{% raw %}
```cpp
blueberry::WaveletMatrix<long long> wm(std::vector<long long>{3, -1, 3, 7});
```
{% endraw %}

注意点: 空配列も有効です。入力への参照を保持せず、入力変更・破棄は構築済みの状態に影響しません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int wm.size() const</code> — O(1)</summary>

入力の要素数 N を返します。

{% raw %}
```cpp
assert(wm.size() == 4);
```
{% endraw %}

注意点: 値の種類数ではなく、重複を含む要素数です。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T wm.get(int p) const</code> — O(L)</summary>

元の配列の `a[p]` を値として返します。`0 <= p < N`。

{% raw %}
```cpp
assert(wm.get(1) == -1);
```
{% endraw %}

注意点: 空配列では呼び出せません。返り値は内部への参照ではありません。

</details>

<details class="api-operation" id="kth-smallest" markdown="1">
<summary><code>T wm.kth_smallest(int l, int r, int k) const</code> — O(L)</summary>

`a[l:r]` を昇順に並べたときの 0-indexed の k 番目を返します。`0 <= k < r-l`。

{% raw %}
```cpp
assert(wm.kth_smallest(0, 4, 1) == 3);
```
{% endraw %}

注意点: 重複も別々に数えます。空区間は無効です。

</details>

<details class="api-operation" id="kth-largest" markdown="1">
<summary><code>T wm.kth_largest(int l, int r, int k) const</code> — O(L)</summary>

区間内を降順に並べたときの 0-indexed の k 番目を返します。`0 <= k < r-l`。

{% raw %}
```cpp
assert(wm.kth_largest(0, 4, 0) == 7);
```
{% endraw %}

注意点: 重複も別々に数えます。空区間は無効です。

</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>int wm.count(int l, int r, const T& x) const</code> — O(L)</summary>

`l <= i < r` かつ `a[i] == x` の要素数を返します。

{% raw %}
```cpp
assert(wm.count(0, 4, 3) == 2);
```
{% endraw %}

注意点: 空区間や入力にない値については 0 を返します。

</details>

<details class="api-operation" id="freq-upper" markdown="1">
<summary><code>int wm.range_freq(int l, int r, const T& upper) const</code> — O(L)</summary>

区間内で `a[i] < upper` の要素数を返します。

{% raw %}
```cpp
assert(wm.range_freq(0, 4, 3) == 1);
```
{% endraw %}

注意点: `upper` 自身は含みません。空区間は 0。上限値への加算は行わないため整数境界でもオーバーフローしません。

</details>

<details class="api-operation" id="freq-range" markdown="1">
<summary><code>int wm.range_freq(int l, int r, const T& lower, const T& upper) const</code> — O(L)</summary>

区間内で `lower <= a[i] < upper` の要素数を返します。`lower <= upper` が必要です。

{% raw %}
```cpp
assert(wm.range_freq(0, 4, -1, 3) == 1);
```
{% endraw %}

注意点: 値域も半開区間です。`lower==upper` または空の添字区間では 0 を返します。

</details>

<details class="api-operation" id="prev" markdown="1">
<summary><code>std::optional&lt;T&gt; wm.prev_value(int l, int r, const T& upper) const</code> — O(L)</summary>

区間内の `upper` 未満の最大値を返します。該当値がない場合は `std::nullopt`。

{% raw %}
```cpp
assert(wm.prev_value(0, 4, 3).value() == -1);
assert(!wm.prev_value(0, 0, 3));
```
{% endraw %}

注意点: 空区間は `nullopt`。`-1` などの有効値を不在の番兵に使用しません。

</details>

<details class="api-operation" id="next" markdown="1">
<summary><code>std::optional&lt;T&gt; wm.next_value(int l, int r, const T& lower) const</code> — O(L)</summary>

区間内の `lower` 以上の最小値を返します。該当値がない場合は `std::nullopt`。

{% raw %}
```cpp
assert(wm.next_value(0, 4, 4).value() == 7);
assert(!wm.next_value(0, 4, 8));
```
{% endraw %}

注意点: こちらの境界は等号を含みます。空区間では `nullopt` を返します。

</details>

## 出典・検証

- [Luzhiled の Wavelet Matrix](https://ei1333.github.io/library/structure/wavelet/wavelet-matrix.hpp.html):
  固定幅と座標圧縮版の API、rank による区間移動を調査しました。
- [Nyaan の Wavelet Matrix](https://nyaannyaan.github.io/library/data-structure-2d/wavelet-matrix.hpp.html):
  64 bit ブロックと prefix popcount の設計を調査しました。
- 本実装は上記のアルゴリズムを参考に独立に記述しています。固定 MAXLOG や CPU 固有 intrinsic を
  要求せず、値域の自動圧縮と optional による不在表現を採用しました。
- 公式検証: [Range Kth Smallest](https://judge.yosupo.jp/problem/range_kth_smallest)、
  [Static Range Frequency](https://judge.yosupo.jp/problem/static_range_frequency)。
- `tests/random/wavelet-matrix.cpp`: 整列した部分配列との全公開クエリ比較。
  空・全要素同値・昇順・降順・重複・符号付き/符号なし極値・64 bit 境界を含みます。
- 同一入力で整数 prefix 配列版と比較する測定コード・生ログ・判断は
  [追加ライブラリの測定記録](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/ADDITIONS.md)にあります。
