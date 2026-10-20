---
title: POPCNT Wavelet Matrix
documentation_of: //blueberry/extra-fast/wavelet-matrix.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::WaveletMatrix<T>` は静的な整数列の区間k番目・頻度・前後要素を求めます。
rankのbit列と累積個数を同じブロックに置き、x86-64の対応CPUではPOPCNT命令を使います。
独立したk番目の問い合わせをまとめる `kth_smallest_batch` も備えます。通常版とは別に個別includeして使います。
値を自動で座標圧縮し、符号やビット幅の指定は不要です。値は更新できません。添字区間は `0 <= l <= r <= N` です。

Nは要素数、σは異なる値の数、L=max(1,ceil(log2(max(1,σ))))。構築は最悪O(N log(N+1))、各オンラインクエリは最悪O(L)、Q件のbatchは最悪O(Q(L+1))、sizeとCPU判定はO(1)です。構築直後の保持メモリはO(N+L ceil((N+1)/64))ワード、構築補助メモリはO(N)、batchの追加メモリはO(Q)です。
POPCNTの非対応環境では標準C++のpopcountへ切り替えます。入力分布やCPUによって通常版より遅い場合があります。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`WaveletMatrix<T> wm`](#default) | O(1) |
| [`WaveletMatrix<T> wm(a, use_popcnt = true)`](#construct) | O(N log(N+1)) |
| [`wm.size() → int`](#size) | O(1) |
| [`wm.get(p) → T`](#get) | O(L) |
| [`wm.kth_smallest(l,r,k) → T`](#kth-smallest) | O(L) |
| [`wm.kth_largest(l,r,k) → T`](#kth-largest) | O(L) |
| [`wm.count(l,r,x) → int`](#count) | O(L) |
| [`wm.range_freq(l,r,upper) → int`](#freq-upper) | O(L) |
| [`wm.range_freq(l,r,lower,upper) → int`](#freq-range) | O(L) |
| [`wm.prev_value(l,r,upper) → optional<T>`](#prev) | O(L) |
| [`wm.next_value(l,r,lower) → optional<T>`](#next) | O(L) |
| [`wm.kth_smallest_batch(queries) → vector<T>`](#batch) | O(Q(L+1)) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`wm.uses_popcnt() → bool`](#uses-popcnt) | O(1) |
| [`WaveletMatrix<T>::popcnt_available() → bool`](#popcnt-available) | O(1) |
| [`WaveletMatrix<T> copy(other)`](#copy-construct) | O(M) |
| [`wm = other`](#copy-assign) | O(M+V) |
| [`WaveletMatrix<T> moved(std::move(other))`](#move-construct) | O(1) |
| [`wm = std::move(other)`](#move-assign) | O(V) |
| [`wm.~WaveletMatrix()`](#destroy) | O(M) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLにないデータ構造です。64bitのbit列と32bitの累積popcountを1つのblockにし、全段を連続領域に置きます。
  一般的な64bit環境ではblockはpadding込みで16byteです。通常版の分離配列よりメタデータの量は増えます。値辞書のvectorは入力サイズ分の容量を持ち得るため、保持メモリのO(N)項を含めています。
- 値域の幅 `max(a)-min(a)` がN未満なら既存のpartition bufferを順位表へ再利用し、穴を詰めて圧縮します。
  この経路はO(N)で、別の値域用allocationはありません。値域が広いときは元位置を値順に並べ、順位を元位置へ書き戻します。
  小配列では比較sort、それ以外ではunsigned型で最小値との差を取り、必要な8bit桁だけradix分配します。
- radixのbucket初期化が小さい全幅入力で支配的になるため、N<64またはNが必要桁数の128倍未満なら比較sortを選びます。各要素の二分探索を省き、2本の位置配列を段構築のpartition bufferに再利用します。
- bit列と累積個数は64要素ずつまとめて構築し、最後の段では後続処理に不要なpartitionを行いません。
- 圧縮時の差分はunsignedのmodulo演算、段構築は圧縮後の非負整数のビット操作です。符号付きoverflowを起こさず、符号付き最小値・最大値や重複も同じ順序で扱います。
- 型のbit幅を8で割って切り上げた数をBとすると、radix部分はO(BN)、比較sortはO(N log(N+1))です。標準整数型のBを定数とすれば、上記の構築上界を満たします。
- Tの比較・コピーと64bit popcountをO(1)とします。GCC/Clangのx86-64では実行CPUのPOPCNT対応を検出します。
  対応時はGNU inline asmで `popcnt` を使い、Intel/AT&T両dialectを扱います。対象CPUの指定は不要です。
  `use_popcnt=false` や他環境では `std::popcount` を使います。コンパイラ全体に `-march=native` を指定した場合は、
  この標準C++経路にもPOPCNT等が生成され得ます。CPU機能は実行中に変わらないことを前提とします。
  `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` を定義すると明示的なPOPCNTを無効にし、`popcnt_available()` もfalseになります。
  これはコンパイラの自動命令生成を禁止するものではありません。設定マクロは全翻訳単位で統一してください。
- 範囲外引数は契約違反で、debug buildではassertです。公開フィールドはありません。
- 暗黙のコピーは独立した配列を所有します。ムーブ元には破棄・再代入のみ行ってください。取得結果は値を返し、参照の無効化はありません。
  M=max(1,N+L ceil((N+1)/64))、Vは代入前の代入先の保持要素数です。
  コピー元が保持していた未使用容量をコピー後に引き継ぐとは限りません。ムーブはその容量も引き継ぎます。
  記憶領域の確保失敗は標準vectorと同様に例外です。`blueberry/all.hpp` には含まれません。

## 操作の補足 {#操作の補足}

- 以下の操作例は最小使用例の `wm` を前提とします。

</details>

## 最小使用例

{% raw %}
```cpp
#include <array>
#include <cassert>
#include <vector>
#include "blueberry/extra-fast/wavelet-matrix.hpp"
int main() {
  blueberry::extra_fast::WaveletMatrix<long long> wm(std::vector<long long>{3, -1, 3, 7});
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
  const std::vector<std::array<int, 3>> queries{{0, 4, 1}, {1, 3, 0}};
  assert((wm.kth_smallest_batch(queries) == std::vector<long long>{3, -1}));
}
```
{% endraw %}

<details class="api-operation" id="default" markdown="1">
<summary><code>WaveletMatrix&lt;T&gt; wm</code> — O(1)</summary>

空配列を構築します。`size()==0` であり、空区間の頻度・前後探索を実行できます。

{% raw %}
```cpp
blueberry::extra_fast::WaveletMatrix<int> empty;
assert(empty.count(0, 0, 42) == 0);
```
{% endraw %}

注意点: `get` と k 番目の取得は要素が必要です。

</details>

<details class="api-operation" id="construct" markdown="1">
<summary><code>WaveletMatrix&lt;T&gt; wm(const std::vector&lt;T&gt;& a, bool use_popcnt = true)</code> — O(N log(N+1))</summary>

配列をコピーして構築します。`use_popcnt` はPOPCNT経路の使用許可で、非対応CPUでは `true` でも標準C++経路です。`T` は `bool` 以外の標準整数型、`0 <= a.size() <= INT_MAX`。負数・重複・整数型の最小値と最大値を扱います。

{% raw %}
```cpp
blueberry::extra_fast::WaveletMatrix<long long> wm(std::vector<long long>{3, -1, 3, 7});
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
<summary><code>int wm.count(int l, int r, T x) const</code> — O(L)</summary>

`l <= i < r` かつ `a[i] == x` の要素数を返します。

{% raw %}
```cpp
assert(wm.count(0, 4, 3) == 2);
```
{% endraw %}

注意点: 空区間や入力にない値については 0 を返します。

</details>

<details class="api-operation" id="freq-upper" markdown="1">
<summary><code>int wm.range_freq(int l, int r, T upper) const</code> — O(L)</summary>

区間内で `a[i] < upper` の要素数を返します。

{% raw %}
```cpp
assert(wm.range_freq(0, 4, 3) == 1);
```
{% endraw %}

注意点: `upper` 自身は含みません。空区間は 0。上限値への加算は行わないため整数境界でもオーバーフローしません。

</details>

<details class="api-operation" id="freq-range" markdown="1">
<summary><code>int wm.range_freq(int l, int r, T lower, T upper) const</code> — O(L)</summary>

区間内で `lower <= a[i] < upper` の要素数を返します。`lower <= upper` が必要です。

{% raw %}
```cpp
assert(wm.range_freq(0, 4, -1, 3) == 1);
```
{% endraw %}

注意点: 値域も半開区間です。`lower==upper` または空の添字区間では 0 を返します。

</details>

<details class="api-operation" id="prev" markdown="1">
<summary><code>std::optional&lt;T&gt; wm.prev_value(int l, int r, T upper) const</code> — O(L)</summary>

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
<summary><code>std::optional&lt;T&gt; wm.next_value(int l, int r, T lower) const</code> — O(L)</summary>

区間内の `lower` 以上の最小値を返します。該当値がない場合は `std::nullopt`。

{% raw %}
```cpp
assert(wm.next_value(0, 4, 4).value() == 7);
assert(!wm.next_value(0, 4, 8));
```
{% endraw %}

注意点: こちらの境界は等号を含みます。空区間では `nullopt` を返します。

</details>

<details class="api-operation" id="batch" markdown="1">
<summary><code>std::vector&lt;T&gt; wm.kth_smallest_batch(const std::vector&lt;std::array&lt;int,3&gt;&gt;& queries) const</code> — O(Q(L+1))</summary>

各要素 `{l,r,k}` をk番目の問い合わせとして処理し、入力順の答えを返します。
すべて `0 <= l <= r <= N`、`0 <= k < r-l` が必要です。段ごとに独立した問い合わせをまとめて処理し、
入力のqueriesやwmを変更しません。追加メモリはO(Q)です。

{% raw %}
```cpp
assert((wm.kth_smallest_batch({{0, 4, 1}, {1, 3, 0}}) == std::vector<long long>{3, -1}));
```
{% endraw %}

注意点: queriesが空なら空vectorを返し、空のwmでも呼び出せます。個々の空区間は無効です。
前の答えに依存する問い合わせはオンラインAPIを使ってください。少数の問い合わせではコピーの分だけ不利になり得ます。

</details>

<details class="api-operation" id="uses-popcnt" markdown="1">
<summary><code>bool wm.uses_popcnt() const</code> — O(1)</summary>

このオブジェクトの問い合わせが明示的なPOPCNT経路を使うかを返します。

{% raw %}
```cpp
assert(!blueberry::extra_fast::WaveletMatrix<int>(std::vector<int>{1}, false).uses_popcnt());
```
{% endraw %}

注意点: 標準C++経路でコンパイラが生成する命令の有無は表しません。

</details>

<details class="api-operation" id="popcnt-available" markdown="1">
<summary><code>static bool WaveletMatrix&lt;T&gt;::popcnt_available()</code> — O(1)</summary>

この実装で実行CPUのPOPCNT経路を利用できるかを返します。

{% raw %}
```cpp
const bool available = blueberry::extra_fast::WaveletMatrix<int>::popcnt_available();
(void)available;
```
{% endraw %}

注意点: GCC/Clangのx86-64以外、または `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` 指定時はfalseです。全体を特殊なCPUオプションでコンパイルしたコードの移植性は、そのオプションに従います。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>WaveletMatrix&lt;T&gt; copy(const WaveletMatrix&lt;T&gt;& other)</code> — O(M)</summary>

値辞書とrank配列を独立した領域へコピーします。追加メモリはO(M)です。

{% raw %}
```cpp
auto copied = wm;
assert(copied.get(0) == wm.get(0));
```
{% endraw %}

注意点: コピー元への依存や要素参照を持ちません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>wm = const WaveletMatrix&lt;T&gt;& other</code> — O(M+V)</summary>

コピー元の値とCPU経路で置き換え、`*this` への参照を返します。自己代入も可能です。
一時コピーを作ってから交換し、確保失敗時は代入先を保持します。追加メモリ上界はO(M)です。

{% raw %}
```cpp
auto copied = wm;
copied = wm;
assert(copied.size() == wm.size());
```
{% endraw %}

注意点: コピー元と独立した配列です。成功時は以前の代入先の領域を解放します。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>WaveletMatrix&lt;T&gt; moved(WaveletMatrix&lt;T&gt;&amp;&amp; other)</code> — O(1)</summary>

確保領域を移動して受け取ります。追加メモリはO(1)です。

{% raw %}
```cpp
auto copied = wm;
auto moved = std::move(copied);
assert(moved.size() == wm.size());
```
{% endraw %}

注意点: 移動元への操作は破棄と再代入だけにしてください。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>wm = WaveletMatrix&lt;T&gt;&amp;&amp; other</code> — O(V)</summary>

既存領域を解放し、移動元の領域を引き継いで `*this` への参照を返します。追加メモリはO(1)です。

{% raw %}
```cpp
auto copied = wm;
blueberry::extra_fast::WaveletMatrix<long long> moved;
moved = std::move(copied);
assert(moved.size() == wm.size());
```
{% endraw %}

注意点: 自己移動後も破棄と再代入だけにしてください。

</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>wm.~WaveletMatrix()</code> — O(M)</summary>

保持している辞書とrank配列を解放します。

{% raw %}
```cpp
{ auto temporary = wm; }
```
{% endraw %}

注意点: 通常はスコープを抜けると自動で呼ばれます。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・測定</summary>

## 出典・検証 {#出典検証}

- [LC 403008](https://judge.yosupo.jp/submission/403008)・[403000](https://judge.yosupo.jp/submission/403000):
  AVX2で段構築とbatch問い合わせを処理するoffline方式を調査しました。前の答えに依存するオンライン問い合わせとは契約が異なります。
- [LC 393928](https://judge.yosupo.jp/submission/393928)・[376681](https://judge.yosupo.jp/submission/376681):
  rankのbit列と累積個数を同じblockに置き、段ごとにk番目の問い合わせを処理する設計を調査しました。コードは転載していません。
- [LC 362490](https://judge.yosupo.jp/submission/362490): 頻度専用のhashとoffline prefix sweepで、Wavelet Matrixの代替としては扱っていません。
- 公式問題は [Range Kth Smallest](https://judge.yosupo.jp/problem/range_kth_smallest) と
  [Static Range Frequency](https://judge.yosupo.jp/problem/static_range_frequency)。onlineとbatchの両方にverifyコードがあります。
- `tests/random/extra-fast-wavelet.cpp` は全公開クエリを整列した部分配列と比較します。
  空・重複・各整数型の極値・64bit境界、hardware/標準C++の両経路を扱います。
  `tests/random/extra-fast-wavelet-copy.cpp` はコピー代入の各allocationを失敗させ、以前の内容を保持することを確認します。
- [測定記録]({{ '/docs/development/measurements/extra-fast-more/wavelet/README.html' | relative_url }})に入力・環境・生ログと不利な条件を残しています。

</details>
