---
title: SIMD Bitset
documentation_of: //blueberry/extra-fast/bitset.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::Bitset` は、実行時に長さを決めるbitsetです。
部分和DPで使う `B |= B << k` を `shift_or(k)` の1回の走査にまとめ、一時bitsetを確保しません。
AVX2対応環境では、shiftとOR、bitset同士のOR/XOR/ANDを256-bit単位で処理します。

Nをbit数、W = ceil(N / 64)とします。構築・全体クリア・count・bitset演算はO(W + 1)、
一点の読み書きはO(1)、shift_orはO(W + 1)、構築直後のメモリはO(W + 1)です。いずれも最悪計算量です。
長さを実行時に決めるDPや、GF(2)の行列の行操作に向きます。CPU・サイズ・コンパイラによって通常ループの方が速い場合もあります。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`Bitset(n = 0, use_simd = true)`](#construct) | O(W + 1) |
| [`bits.size() → int`](#size) | O(1) |
| [`bits.test(p) → bool`](#test) | O(1) |
| [`bits.set(p, value = true)`](#set) | O(1) |
| [`bits.reset(p)`](#reset) | O(1) |
| [`bits.clear()`](#clear) | O(W + 1) |
| [`bits.count() → int`](#count) | O(W + 1) |
| [`bits.shift_or(k)`](#shift-or) | O(W + 1) |
| [`bits.or_with(other)`](#or-with) | O(W + 1) |
| [`bits.xor_with(other)`](#xor-with) | O(W + 1) |
| [`bits.and_with(other)`](#and-with) | O(W + 1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`bits.uses_simd() → bool`](#uses-simd) | O(1) |
| [`Bitset::simd_available() → bool`](#simd-available) | O(1) |
| [`Bitset(other)`](#copy-construct) | O(W + 1) |
| [`bits = other`](#copy-assign) | O(W + V + 1) |
| [`Bitset(std::move(other))`](#move-construct) | O(1) |
| [`bits = std::move(other)`](#move-assign) | O(V + 1) |
| [`bits.~Bitset()`](#destroy) | O(W + 1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- C++20。公開型は `blueberry::extra_fast::Bitset`。公開フィールドはありません。
  個別にincludeする実験版で、`blueberry/all.hpp` には含まれません。
- `0 <= N <= INT_MAX` で、記憶領域が確保できることが必要です。添字は0-indexedで `0 <= p < N`。
  シフト量は非負です。不正な引数はassertの対象で、`NDEBUG`時は検査しません。
- `uint64_t` をW個保持します。bit pは第p/64語の下位からp%64番目です。
  末尾のN以上のbitは常に0とし、countや後続のshiftへ混入させません。
  末尾の語も含めて確保領域内だけを読み書きし、入力・配列の前後のpaddingを要求しません。
- 一点操作以外の更新も確保済み領域内で完了します。`shift_or` を含め、更新時に追加allocationはありません。
  64-bit整数演算はunsignedで行い、型の幅以上のC++シフトは行いません。
- `or_with`、`xor_with`、`and_with` は両辺が同じbit数であることを必要とします。
  自分自身も渡せます。OR/ANDでは変化せず、XORではすべて0になります。
- GCC/Clangのx86では実行CPUのAVX2対応を検出し、対応kernelだけを `target("avx2")` でコンパイルします。
  全体の `-mavx2` は不要です。それ以外の環境や `use_simd=false` では通常のループを選びます。
  4語未満の端数は通常のループで処理します。countは `std::popcount` を使います。
- 既定はintrinsicsです。include前に `BLUEBERRY_EXTRA_FAST_USE_ASM` を定義すると、
  SIMDのOR/XOR/ANDにGNU inline asmの `vpor` / `vpxor` / `vpand` を使います。
  Intel/AT&T両dialectを扱い、asm内で隠れたメモリアクセスは行いません。
  手書きasm自体がintrinsicより速いとは保証しません。
- `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` は明示的なAVX2経路の検出を無効にします。
  設定マクロは全翻訳単位で統一し、実行中にCPUの機能が変化しないことを前提とします。
- 値は各オブジェクトが所有し、公開操作は要素への参照を返しません。コピーは独立した配列を持ちます。
  move元には破棄と再代入だけを行ってください。確保失敗では標準コンテナと同様に例外が送出されます。
  Vは代入前の代入先の語数です。
- コピー代入で長さを縮めても、以前のvectorのcapacityを保持する場合があります。
  未使用の容量をC語とすると、実際の保持領域はO(W + C + 1)です。moveではその容量も引き継ぎます。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/extra-fast/bitset.hpp"
int main() {
  blueberry::extra_fast::Bitset reachable(130);
  reachable.set(0);
  reachable.shift_or(3);
  reachable.shift_or(65);
  assert(reachable.test(0) && reachable.test(3));
  assert(reachable.test(65) && reachable.test(68));
  assert(reachable.count() == 4);
  reachable.shift_or(130);
  assert(reachable.count() == 4);
  auto copy = reachable;
  copy.xor_with(reachable);
  assert(copy.count() == 0);
  assert(reachable.count() == 4);
  blueberry::extra_fast::Bitset empty;
  empty.shift_or(0);
  assert(empty.size() == 0 && empty.count() == 0);
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>Bitset(int n = 0, bool use_simd = true)</code> — O(W + 1)</summary>

長さn、全bitが0のbitsetを作ります。引数を省略すると空のbitsetです。

{% raw %}
```cpp
blueberry::extra_fast::Bitset empty;
blueberry::extra_fast::Bitset scalar(65, false);
```
{% endraw %}

注意点: nは非負。use_simd=falseなら対応CPUでも通常ループを選びます。trueでも非対応CPUでは通常ループになります。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int bits.size() const</code> — O(1)</summary>

bit数Nを返します。

{% raw %}
```cpp
int n = reachable.size();
```
{% endraw %}

注意点: 内部の末尾paddingは含みません。空なら0です。

</details>

<details class="api-operation" id="test" markdown="1">
<summary><code>bool bits.test(int p) const</code> — O(1)</summary>

bit pが1ならtrue、0ならfalseを返します。

{% raw %}
```cpp
assert(reachable.test(68));
```
{% endraw %}

注意点: 0 <= p < size()。空の場合は呼べません。値で返すため参照の無効化はありません。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void bits.set(int p, bool value = true)</code> — O(1)</summary>

bit pをvalueへ代入します。valueを省略すると1にします。

{% raw %}
```cpp
reachable.set(5);
reachable.set(5, false);
```
{% endraw %}

注意点: 0 <= p < size()。サイズや他のbitは変えません。

</details>

<details class="api-operation" id="reset" markdown="1">
<summary><code>void bits.reset(int p)</code> — O(1)</summary>

bit pを0にします。

{% raw %}
```cpp
reachable.reset(3);
```
{% endraw %}

注意点: 0 <= p < size()。既に0でも呼べます。全体を0にする場合はclear()を使います。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void bits.clear()</code> — O(W + 1)</summary>

全bitを0にします。

{% raw %}
```cpp
reachable.clear();
assert(reachable.size() == 130 && reachable.count() == 0);
```
{% endraw %}

注意点: 長さと確保領域を維持します。空でも呼べます。

</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>int bits.count() const</code> — O(W + 1)</summary>

1であるbitの個数を返します。

{% raw %}
```cpp
int ones = reachable.count();
```
{% endraw %}

注意点: 返り値は0以上size()以下です。末尾paddingを数えず、空なら0です。

</details>

<details class="api-operation" id="shift-or" markdown="1">
<summary><code>void bits.shift_or(int k)</code> — O(W + 1)</summary>

呼び出し前のbit列をBとして、各bitをB[p] OR B[p-k]へ更新します。範囲外の参照bitは0とします。部分和DPのB |= B << kに相当します。

{% raw %}
```cpp
reachable.set(0);
reachable.shift_or(65);
assert(reachable.test(65));
```
{% endraw %}

注意点: k >= 0。k=0またはk>=size()では変化しません。N以上に押し出されたbitは捨てます。上位から処理するため、同じ呼び出しで新たに立ったbitを再利用せず、一時配列も確保しません。

</details>

<details class="api-operation" id="or-with" markdown="1">
<summary><code>void bits.or_with(const Bitset&amp; other)</code> — O(W + 1)</summary>

各位置のbitを、自分とotherのORにします。otherは変更しません。

{% raw %}
```cpp
auto other = reachable;
reachable.or_with(other);
```
{% endraw %}

注意点: size() == other.size()。同一オブジェクトなら変化せず、空同士でも呼べます。

</details>

<details class="api-operation" id="xor-with" markdown="1">
<summary><code>void bits.xor_with(const Bitset&amp; other)</code> — O(W + 1)</summary>

各位置のbitを、自分とotherのXORにします。GF(2)のベクトル加算・行操作に使えます。

{% raw %}
```cpp
auto other = reachable;
other.xor_with(reachable);
assert(other.count() == 0);
```
{% endraw %}

注意点: size() == other.size()。同一オブジェクトならすべて0になり、空同士でも呼べます。

</details>

<details class="api-operation" id="and-with" markdown="1">
<summary><code>void bits.and_with(const Bitset&amp; other)</code> — O(W + 1)</summary>

各位置のbitを、自分とotherのANDにします。otherは変更しません。

{% raw %}
```cpp
auto other = reachable;
reachable.and_with(other);
```
{% endraw %}

注意点: size() == other.size()。同一オブジェクトなら変化せず、空同士でも呼べます。

</details>

<details class="api-operation" id="uses-simd" markdown="1">
<summary><code>bool bits.uses_simd() const</code> — O(1)</summary>

このbitsetが更新にAVX2経路を選ぶかを返します。

{% raw %}
```cpp
blueberry::extra_fast::Bitset scalar(256, false);
assert(!scalar.uses_simd());
```
{% endraw %}

注意点: コンストラクタの指定とCPU機能で決まります。小さいサイズや端数ではAVX2経路内でも通常ループを使う場合があります。一点操作とcountにはこの設定を使いません。

</details>

<details class="api-operation" id="simd-available" markdown="1">
<summary><code>static bool Bitset::simd_available()</code> — O(1)</summary>

このビルド・実行CPUでAVX2経路を選べるかを返します。

{% raw %}
```cpp
bool available = blueberry::extra_fast::Bitset::simd_available();
```
{% endraw %}

注意点: x86のGCC/Clang以外やFORCE_SCALAR指定時はfalseです。結果によらずbitsetを構築できます。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>Bitset(const Bitset&amp; other)</code> — O(W + 1)</summary>

値・長さ・SIMD選択が同じ独立したコピーを構築します。

{% raw %}
```cpp
auto copy = reachable;
```
{% endraw %}

注意点: 記憶領域も複製します。コピー後の更新は互いに影響しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>Bitset&amp; bits.operator=(const Bitset&amp; other)</code> — O(W + V + 1)</summary>

otherの値・長さ・SIMD選択をコピーし、*thisへの参照を返します。

{% raw %}
```cpp
blueberry::extra_fast::Bitset copy;
copy = reachable;
```
{% endraw %}

注意点: 自己代入も可能です。代入先の以前の値は失われます。配列のコピーで確保が失敗した場合、代入先は以前の状態を維持します。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>Bitset(Bitset&amp;&amp; other)</code> — O(1)</summary>

otherの領域・値・長さ・SIMD選択を引き継ぎます。

{% raw %}
```cpp
auto copy = reachable;
auto moved = std::move(copy);
```
{% endraw %}

注意点: move元には破棄と再代入だけを行います。std::moveを使う場合は`<utility>`をincludeしてください。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>Bitset&amp; bits.operator=(Bitset&amp;&amp; other)</code> — O(V + 1)</summary>

代入先の以前の領域を解放してotherから引き継ぎ、*thisへの参照を返します。

{% raw %}
```cpp
auto copy = reachable;
blueberry::extra_fast::Bitset moved;
moved = std::move(copy);
```
{% endraw %}

注意点: move元には破棄と再代入だけを行います。std::moveを使う場合は`<utility>`をincludeしてください。

</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>bits.~Bitset()</code> — O(W + 1)</summary>

所有する配列を解放します。

{% raw %}
```cpp
{ blueberry::extra_fast::Bitset temporary(256); }
```
{% endraw %}

注意点: 通常はスコープ終了時に自動で呼ばれます。明示的に破棄した後の操作はできません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・測定</summary>

## 出典・検証 {#出典検証}

- Library Checker [Matrix Product (Mod 2)](https://judge.yosupo.jp/problem/matrix_product_mod_2) の上位10件を調べ、
  [246187](https://judge.yosupo.jp/submission/246187)・[406212](https://judge.yosupo.jp/submission/406212)で、
  行ベクトルのAVX2 XORと、手書きasmの `vpxor` / `vmovdqa` を確認しました。
  両提出は固定4096次元・block処理と、入力に合わせた機械語のJIT生成を組み合わせています。
  本bitsetは実行可能メモリ・JIT・固定次元を使わず、bit演算kernelを独立に実装しています。
- 代表提出のコード全体に適用できるライセンスは保存ソースから確認できませんでした。コードは転載していません。
  汎用DPのshift_orと、固定行列積のJIT全体は契約もアルゴリズムも異なり、公開時間をそのまま比較しません。
- 公式verifyは `verify/extra-fast/bit-matrix-product.test.cpp`。行のXORによるGF(2)行列積を検証します。
  DPのshift_or、OR/AND、一点更新、末尾bit、空入力、コピー・移動、SIMD/通常ループは
  `tests/random/extra-fast-bitset.cpp` で独立したvector<bool>の愚直解と比較します。
- [比較測定]({{ '/docs/development/measurements/extra-fast-expansion/README.html' | relative_url }})で、
  構築と操作、intrinsics/asm、通常ループ、同じbit数のstd::bitsetを分けて評価します。
  手書きasmの有無だけでは高速化を主張しません。

</details>
