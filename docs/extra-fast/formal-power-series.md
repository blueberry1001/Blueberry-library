---
title: SIMD FPS (998244353)
documentation_of: //blueberry/extra-fast/formal-power-series.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::FPS998244353`は、法998244353に限った多項式の積と形式的冪級数の逆元・log・expを計算します。係数は`std::vector<std::uint32_t>`で渡します。AVX2対応CPUでは8係数ずつMontgomery乗算し、非対応環境ではスカラー版を使います。

要求する係数数をNとすると、各級数演算の時間はO(N log N)、追加メモリはO(N)です。入力の末尾がN項を超える場合は読みません。多項式積は結果の係数数をNとして同じ計算量です。小さい積は愚直法に切り替えます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`FPS998244353 fps(use_simd = true)`](#constructor) | O(1) |
| [`fps.convolution(a, b)`](#convolution) | O(N log N) |
| [`fps.inv(f, n = -1)`](#inv) | O(N log N) |
| [`fps.log(f, n = -1)`](#log) | O(N log N) |
| [`fps.exp(f, n = -1)`](#exp) | O(N log N) |
| [`fps.uses_simd()`](#uses-simd) | O(1) |
| [`FPS998244353::simd_available()`](#simd-available) | O(1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`FPS998244353 other(fps)`、`FPS998244353 other(std::move(fps))`](#copy-move) | O(1) |
| [`other = fps`、`other = std::move(fps)`](#copy-move) | O(1) |
| [`fps.~FPS998244353()`](#destructor) | O(1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 係数は低次から並べ、読み取る係数を`0 <= f[i] < 998244353`とします。結果もこの範囲です。負数や他のmodintからは利用側で変換してください。
- `inv`・`log`・`exp`の`n < 0`は`f.size()`を指定したものとして扱います。`n == 0`は係数を調べず空の結果を返します。入力にない係数は0です。`f.size() <= 2^23`が必要です。
- `inv`と`exp`は`n <= 2^23`、`log`は`n <= 2^22`、積は`a.size() + b.size() - 1 <= 2^23`が必要です。級数演算の前提を満たさない入力はassertの対象で、エラーを返すAPIではありません。
- 入力を変更せず、結果は独立したvectorとして所有します。返された要素の参照は、そのvectorを変更するまで有効です。オブジェクトは入力・結果への参照や作業領域を保持しません。別の呼び出しと作業領域を共有しないため、同じオブジェクトから並行して呼び出せます。
- 結果は法998244353での値です。内部では符号なし整数の剰余と64bit積を利用します。
- AVX2はx86のGCC/Clangで実行時検出します。既定はintrinsics、`BLUEBERRY_EXTRA_FAST_USE_ASM`の定義時はMontgomeryの32bit積に`vpmuludq`のinline asmを使います。旧測定用の`BLUEBERRY_EXTRA_FAST_INLINE_ASM`も同じ指定として扱います。`use_simd = false`または`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`でスカラー版を選べます。これらの結果とAPIは同じです。
- これらのマクロはinclude前に指定し、同じプログラムの全翻訳単位で設定を一致させてください。
- 法・係数型を選べる通常版[`FormalPowerSeries`]({{ '/blueberry/math/formal-power-series.hpp.html' | relative_url }})も引き続き利用できます。通常版やACLの畳み込みを置き換えません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/formal-power-series.hpp"
int main() {
  blueberry::extra_fast::FPS998244353 fps;
  using V = std::vector<std::uint32_t>;
  assert(fps.convolution(V{1, 2}, V{3, 4}) == (V{3, 10, 8}));
  assert(fps.inv(V{1, 998244352}, 4) == (V{1, 1, 1, 1}));
  assert(fps.log(V{1, 1}, 3) == (V{0, 1, 499122176}));
  assert(fps.exp(V{0, 1}, 4) == (V{1, 1, 499122177, 166374059}));
  assert(fps.exp(V{}, 3) == (V{1, 0, 0}));
}
```
{% endraw %}

<details class="api-operation" id="constructor" markdown="1">
<summary><code>FPS998244353 fps(bool use_simd = true)</code> — O(1)</summary>

計算時にAVX2を利用するかを選びます。実際に使えるときだけ有効になり、NTT表やバッファはまだ確保しません。

{% raw %}
```cpp
blueberry::extra_fast::FPS998244353 automatic, scalar(false);
```
{% endraw %}

注意点: 非対応CPUではuse_simdがtrueでもスカラー計算です。設定後の変更APIはありません。

</details>

<details class="api-operation" id="convolution" markdown="1">
<summary><code>std::vector&lt;std::uint32_t&gt; fps.convolution(const std::vector&lt;std::uint32_t&gt;&amp; a, const std::vector&lt;std::uint32_t&gt;&amp; b) const</code> — O(N log N)</summary>

多項式aとbの積を返します。Nは`a.size() + b.size() - 1`です。いずれかが空なら空を返し、他方の係数は読みません。結果の末尾の0を削除しません。小さい方の係数数が32以下ならO(a.size() b.size())の直接計算です。

{% raw %}
```cpp
const auto product = fps.convolution({1, 2}, {3, 4});  // {3, 10, 8}
```
{% endraw %}

入力の同じvectorを両引数に渡せます。作業メモリO(N)に加え、結果もO(N)を所有します。

注意点: Nの上限と係数範囲を満たしてください。結果の末尾の0も保持します。

</details>

<details class="api-operation" id="inv" markdown="1">
<summary><code>std::vector&lt;std::uint32_t&gt; fps.inv(const std::vector&lt;std::uint32_t&gt;&amp; f, int n = -1) const</code> — O(N log N)</summary>

`f * g = 1 (mod x^N)`となるgをN項返します。Nは解決後のnです。Nが正なら`!f.empty()`かつ`f[0] != 0`が必要です。入力が短い場合は0で補った級数として扱います。

{% raw %}
```cpp
const auto inverse = fps.inv({1, 998244352}, 4);  // {1, 1, 1, 1}
```
{% endraw %}

Nが0なら空を返し、定数項の前提は不要です。追加メモリO(N)、結果の長さは常にNです。

注意点: Nが正なら空入力と定数項0は不可です。失敗を表す値は返しません。

</details>

<details class="api-operation" id="log" markdown="1">
<summary><code>std::vector&lt;std::uint32_t&gt; fps.log(const std::vector&lt;std::uint32_t&gt;&amp; f, int n = -1) const</code> — O(N log N)</summary>

`log(f) mod x^N`をN項返します。Nが正なら`!f.empty()`かつ`f[0] == 1`が必要です。形式微分・逆元・積・形式積分で計算し、結果の定数項は0です。

{% raw %}
```cpp
const auto logarithm = fps.log({1, 1}, 3);  // {0, 1, 499122176}
```
{% endraw %}

Nが0なら空です。積の変換長のため`N <= 2^22`が必要です。追加メモリO(N)。

注意点: Nが正なら定数項は1が必要です。出力の長さNに末尾の0も含みます。

</details>

<details class="api-operation" id="exp" markdown="1">
<summary><code>std::vector&lt;std::uint32_t&gt; fps.exp(const std::vector&lt;std::uint32_t&gt;&amp; f, int n = -1) const</code> — O(N log N)</summary>

`exp(f) mod x^N`をN項返します。Nが正なら`f.empty()`または`f[0] == 0`が必要です。結果の定数項は1です。Newton反復で近似とその逆元の変換を再利用します。

{% raw %}
```cpp
const auto exponential = fps.exp({0, 1}, 4);  // {1, 1, 499122177, 166374059}
const auto one = fps.exp({}, 3);             // {1, 0, 0}
```
{% endraw %}

Nが0なら空です。空入力でnを省略した場合も空です。追加メモリO(N)。

注意点: Nが正なら定数項は0が必要です。入力が空でも明示したnが正なら長さnの定数級数1を返します。

</details>

<details class="api-operation" id="uses-simd" markdown="1">
<summary><code>bool fps.uses_simd() const</code> — O(1)</summary>

このオブジェクトがSIMDカーネルを利用できる設定かを返します。小さい変換は、trueでもスカラー計算です。

{% raw %}
```cpp
const bool enabled = fps.uses_simd();
```
{% endraw %}

注意点: trueはSIMDを使える設定を表し、全ての小さい計算でSIMD命令を使うという意味ではありません。

</details>

<details class="api-operation" id="simd-available" markdown="1">
<summary><code>static bool FPS998244353::simd_available()</code> — O(1)</summary>

ビルド環境と実行中のCPUがAVX2カーネルに対応するかを返します。非対応ならfalseです。

{% raw %}
```cpp
const bool supported = blueberry::extra_fast::FPS998244353::simd_available();
```
{% endraw %}

注意点: 検出結果は実行環境に依存します。AVX2対応GCC/Clangのx86環境以外ではfalseです。

</details>

<details class="api-operation" id="copy-move" markdown="1">
<summary><code>FPS998244353(const FPS998244353&amp;) / FPS998244353(FPS998244353&amp;&amp;) / operator=(const FPS998244353&amp;) / operator=(FPS998244353&amp;&amp;)</code> — O(1)</summary>

SIMD設定だけをコピーします。入力やバッファを保持しないため、移動後も両方を同じ設定で利用できます。代入は左辺への参照を返します。自己代入も可能です。

{% raw %}
```cpp
blueberry::extra_fast::FPS998244353 copied(fps), moved(std::move(copied));
copied = fps;
copied = std::move(moved);
```
{% endraw %}

この断片で`std::move`を使うときは`<utility>`もincludeします。

注意点: 移動元も有効で、コピー前と同じSIMD設定を保ちます。

</details>

<details class="api-operation" id="destructor" markdown="1">
<summary><code>~FPS998244353()</code> — O(1)</summary>

設定オブジェクトを破棄します。過去に返したvectorには影響しません。

{% raw %}
```cpp
{ blueberry::extra_fast::FPS998244353 temporary; }
```
{% endraw %}

注意点: 返されたvectorはオブジェクトと独立して所有されます。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・測定</summary>

## 出典・検証 {#出典検証}

[exp提出409662](https://judge.yosupo.jp/submission/409662)、[inv提出409370](https://judge.yosupo.jp/submission/409370)、[log提出409452](https://judge.yosupo.jp/submission/409452)、[積提出409465](https://judge.yosupo.jp/submission/409465)のSIMD Montgomery乗算、変換の再利用、専用入出力を調査しました。コードは転載せず、DIF/DITのradix-2 NTTと通常版でも用いているNewton反復から独立に実装しています。最速提出の融合radix-4 kernelや上下半分だけの積、arena、SIMD入出力までは取り込みません。

Library Checkerのconvolution_mod、inv/log/exp_of_formal_power_seriesに対応するverifyを用意し、境界と固定seedのランダムテストで直接計算・ACLを用いる通常版と照合します。[測定記録]({{ '/docs/development/measurements/extra-fast-more/fps/README.html' | relative_url }})では、構築・操作・入出力・メモリ、GCC/Clang、スカラー・intrinsics・asmを分けて比較します。

</details>
