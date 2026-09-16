---
title: Formal Power Series
documentation_of: //blueberry/math/formal-power-series.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

係数を次数の昇順に保持する形式的冪級数（FPS）です。`Mint` は ACL の
`static_modint`（素数 modulus）を想定します。積分・級数演算の要求項数は modulus 未満、
平方根の modulus は奇素数が前提です。NTTを使う操作では、各内部畳み込みの出力長 `L` に対して
`2^ceil(log2(L))` が `mod-1` の2進因子以下である必要があります。`inv(n)` と `exp(n)` の
変換長は `2^ceil(log2(n))`、積・log・sqrt・多項式除算では中間の積の長さも考慮してください。標準設定は
`atcoder::modint998244353` です。`M(N)` を ACL の畳み込みの計算量
（998244353 では $O(N\log N)$）とすると、メモリ使用量は $O(N)$ です。

積の一般的な処理はACLの `convolution` を利用し、FPS固有の逆元・log・expなどの
Newton反復だけをこのヘッダで提供します。ライブラリ本体は `blueberry` 名前空間に
あり、旧パス `blueberry/fps.hpp` からも互換名 `FPS` を利用できます。

## 最小使用例

`f = 1 + x` の逆級数を5項求める例です。`pre(n)` は不足分を0で埋めて必ず `n` 項を返します。

{% raw %}
```cpp
#include <cassert>
#include <atcoder/modint>
#include "blueberry/math/formal-power-series.hpp"

using mint = atcoder::modint998244353;
using fps = blueberry::FormalPowerSeries<mint>;

int main() {
  fps f{1, 1};
  fps g = f.inv(5);
  assert(((f * g).pre(5) == fps{1, 0, 0, 0, 0}));

  fps x{0, 1};
  fps e = x.exp(5);
  assert(e[0] == 1);
  assert(e[1] == 1);
  assert(e[2] == mint(2).inv());

  fps::Sparse sparse{{2, 3}, {0, 1}};
  assert(((f * sparse) / sparse) == f);
  assert((fps{0, 1}.sqrt(1) == fps{0}));
  assert(f.mod_pow(0, fps{2}).empty());
}
```
{% endraw %}

## 操作一覧

`N` は保持する項数、`S` は疎級数の項数、`M(N)` は長さ `N` の畳み込みの計算量です。
`std::vector` から継承した添字・イテレータ・`size()` なども利用できます。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `FormalPowerSeries<Mint> f(n) / f(values)` | O(N) | [開く](#construct) |
| `f.pre(n)`, `f.rev(n)` | O(N) | [開く](#pre-rev) |
| `f.shrink()` | 償却 O(N) | [開く](#shrink) |
| `f +=/-= g`, `f + g`, `f - g`, `-f` | O(max(N, M)) | [開く](#add-sub) |
| `f +=/-= scalar`, `f + scalar`, `f - scalar` | O(1) / O(N) | [開く](#scalar-add-sub) |
| `f *= scalar`, `f /= scalar`, `f * scalar`, `f / scalar` | O(N) | [開く](#scalar) |
| `f *= g`, `f * g` | O(M(N+M)) | [開く](#multiply) |
| `f /= g`, `f / g`, `f %= g`, `f % g`, `f.div_mod(g)` | O(M(N+M)) | [開く](#divide) |
| `f *= sparse`, `f * sparse` | O(NS) | [開く](#sparse) |
| `f /= sparse`, `f / sparse` | O(NS + S log S) | [開く](#sparse) |
| `f <<= d`, `f >>= d`, `f << d`, `f >> d` | O(N+d) | [開く](#shift) |
| `f.dot(g)` | O(min(N, M)) | [開く](#dot) |
| `f.eval(x)`, `f(x)` | O(N) | [開く](#eval) |
| `f.multiply(d, c)`, `f.divide(d, c)` | O(N) | [開く](#factor) |
| `f.diff()`, `f.integral()` | O(N) / O(N log mod) | [開く](#calculus) |
| `f.inv(n)` | O(M(N)) | [開く](#inv) |
| `f.log(n)` | O(M(N)) | [開く](#log) |
| `f.sqrt(n)`, `f.sqrt_with(root, n)` | O(M(N) log N) | [開く](#sqrt) |
| `f.exp(n)` | O(M(N) log N) | [開く](#exp) |
| `f.pow(k, n)` | O(M(N) log N) | [開く](#pow) |
| `f.mod_pow(k, modulus)` | O(M(N+D) + M(D) log(k+1)) | [開く](#mod-pow) |
| `f[i]`, `f.size()`, range-for/iterators | O(1) | [開く](#vector-interface) |

以下の詳細では、返り値、使用例、境界条件をまとめています。例は必要なヘッダと有効な
`f`・`g` が既にある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>FormalPowerSeries&lt;Mint&gt; f(n) / f(values)</code> — O(N)</summary>

`std::vector<Mint>` のコンストラクタを継承しています。係数は `f[i]` が $x^i$ の係数です。

{% raw %}
```cpp
fps a(8);             // 8項を0で初期化
fps b{1, 2, 3};       // 1 + 2x + 3x^2
```
{% endraw %}

注意点: `n` は0以上。空級数も作れますが、`inv`/`log`/`exp`/`sqrt` の前提を満たすかは別途確認してください。

</details>

<details class="api-operation" id="pre-rev" markdown="1">
<summary><code>f.pre(n) / f.rev(n)</code> — O(N)</summary>

`pre(n)` は先頭 `n` 項を取り、不足分を0で埋めます。`rev(n)` は `pre(n)` を反転します。
引数を省略した `rev()` は現在の長さを使います。

{% raw %}
```cpp
fps head = f.pre(10);
fps reversed = f.rev(10);
```
{% endraw %}

注意点: `n<=0` なら空級数です。反転は入力全体を反転する操作ではなく、指定長の切り出し後に反転します。

</details>

<details class="api-operation" id="shrink" markdown="1">
<summary><code>void f.shrink()</code> — 償却 O(N)</summary>

末尾の0係数を削除して長さを詰めます。

{% raw %}
```cpp
fps f{1, 2, 0, 0};
f.shrink();             // {1, 2}
```
{% endraw %}

注意点: ゼロ級数は空になります。次数を固定長として扱う反復中は呼び出さないでください。

</details>

<details class="api-operation" id="add-sub" markdown="1">
<summary><code>f +=/-= g; f + g; f - g; -f</code> — O(max(N, M))</summary>

加減算は短い側に合わせて0を補い、長い側まで長さを拡張します。単項マイナスも利用できます。

{% raw %}
```cpp
fps sum = f + g;
f -= g;
fps negated = -f;
```
{% endraw %}

注意点: 係数型の加減算が定義されている必要があります。結果の末尾0は自動では削除しません。

</details>

<details class="api-operation" id="scalar-add-sub" markdown="1">
<summary><code>f += c; f -= c; f + c; f - c</code> — 更新 O(1)、コピーを返す版 O(N)</summary>

`Mint` の値を定数項だけに加減算します。空級数には定数項を作り、長さ1にします。

{% raw %}
```cpp
fps f{1, 2};
f += mint(3);                 // {4, 2}
assert((f - mint(4) == fps{0, 2}));
```
{% endraw %}

注意点: `+=` / `-=` は自身への参照、二項演算子は独立したFPSを返します。
空級数への更新では再確保が起こりえます。

</details>

<details class="api-operation" id="scalar" markdown="1">
<summary><code>f *= c; f /= c; f * c; f / c</code> — O(N)</summary>

全係数をスカラー `c` 倍（または `c` で割る）します。

{% raw %}
```cpp
f *= mint(3);
fps scaled = f / mint(2);
```
{% endraw %}

注意点: 割り算では `c!=0` が必要です。空級数へのスカラー倍は空のままです。
`f *= f[i]` のように係数を引数にしても、更新前の値で全項を掛けます。保持長・参照の寿命は変えません。

</details>

<details class="api-operation" id="multiply" markdown="1">
<summary><code>f *= g; f * g</code> — O(M(N+M))</summary>

ACLの `atcoder::convolution` で多項式積を計算します。入力長を `N`,`M` とすると、返り値の長さは
空入力でなければ `N+M-1` です。

{% raw %}
```cpp
fps product = f * g;
f *= g;
```
{% endraw %}

注意点: NTTを使うには `Mint` がACLの `static_modint` で、変換長が modulus のNTT能力内に収まる必要があります。

</details>

<details class="api-operation" id="divide" markdown="1">
<summary><code>f /= g; f / g; f %= g; f % g; f.div_mod(g)</code> — O(M(N+M))</summary>

`g` の最高次係数を使った反転多項式の除算で商を求めます。`div_mod` は `{商, 余り}` を返します。
`f %= g` は余りだけを残します。

{% raw %}
```cpp
auto [quotient, remainder] = f.div_mod(g);
fps q = f / g;
```
{% endraw %}

注意点: `g` はゼロ多項式でない必要があります。`g` の末尾0は内部で除去します。
商の保持長は `max(0, f.size()-trimmed_g.size()+1)`、余りの末尾0は除去され、
ゼロの余りは空級数です。空の被除数からは空の商・余りを返します。定数で割った余りも空です。
余りの次数は `g` の次数未満です。更新演算で再確保・要素の削除が起こりえます。

</details>

<details class="api-operation" id="sparse" markdown="1">
<summary><code>f *= s; f * s; f /= s; f / s</code> — 積 O(NS)、商 O(NS + S log S)</summary>

疎な係数列 `s={(次数,係数)}` との積・商を、畳み込みを使わずに計算します。
商では次数0の項を定数項として逐次的に解きます。積・商とも保持長 `N` までで打ち切ります。
各二項演算子はコピー、代入演算子は自身への参照を返します。追加メモリは積 O(N)、商 O(S) です。

{% raw %}
```cpp
fps::Sparse s{{0, 1}, {2, mint(3)}};  // 1 + 3x^2
f *= s;
f /= s;
```
{% endraw %}

注意点: `s` の次数は非負で、並び順は任意です。同じ次数の項は加算して扱います。
除算では次数0の係数の合計が非零でなければなりません。係数型への大小比較は不要です。
商だけが内部で次数順に並べ替えます。空の `s` を掛けると空級数になり、空の `s` で割ることはできません。
空の `f` と有効な `s` の積・商は空級数です。積の代入は格納領域を置き換えるため参照を無効化します。

</details>

<details class="api-operation" id="shift" markdown="1">
<summary><code>f &lt;&lt;= d`, `f &gt;&gt;= d`, `f &lt;&lt; d`, `f &gt;&gt; d</code> — O(N+d)</summary>

左シフトは先頭に0を `d` 個追加し、右シフトは先頭 `d` 項を削除します。

{% raw %}
```cpp
fps delayed = f << 2;  // x^2 f
f >>= 1;
```
{% endraw %}

注意点: `d` は非負。`d` が長さ以上の右シフトは空級数になります。左シフトは長さも増加します。

</details>

<details class="api-operation" id="dot" markdown="1">
<summary><code>f.dot(g)</code> — O(min(N, M))</summary>

同じ次数の係数を要素積し、短い側の長さのFPSを返します。

{% raw %}
```cpp
fps coefficientwise = f.dot(g);
```
{% endraw %}

注意点: 多項式積ではありません。次数の異なる係数は捨てられます。

</details>

<details class="api-operation" id="eval" markdown="1">
<summary><code>f.eval(x) / f(x)</code> — O(N)</summary>

Horner法相当の逐次計算で $f(x)$ を評価します。

{% raw %}
```cpp
mint value = f.eval(mint(2));
assert(value == f(mint(2)));
```
{% endraw %}

注意点: 係数のmodulus上で評価します。大きな点集合をまとめて評価する用途では別の多点評価実装を使ってください。

</details>

<details class="api-operation" id="factor" markdown="1">
<summary><code>f.multiply(d, c) / f.divide(d, c)</code> — O(N)</summary>

現在の長さを保ったまま `(1 + c x^d)` を掛ける／割ります。

{% raw %}
```cpp
f.multiply(3, mint(-1));  // f *= 1 - x^3 (保持長まで)
f.divide(3, mint(-1));
```
{% endraw %}

注意点: `d` は非負。次数 `d` 以上の項だけが更新され、範囲外の高次項は切り捨てられます。`d=0` の除算では `1+c` が0でない必要があります。

</details>

<details class="api-operation" id="calculus" markdown="1">
<summary><code>f.diff() / f.integral()</code> — O(N) / O(N log mod)</summary>

形式微分と形式積分を計算します。`integral` の定数項は0です。

{% raw %}
```cpp
fps derivative = f.diff();
fps antiderivative = derivative.integral();
```
{% endraw %}

注意点: `integral` は `1,2,...,N` の逆元を個別に求めるため O(N log mod)、
保持項数はmodulus未満でなければなりません。微分は空または定数級数から空級数、空級数の積分は `{0}` を返します。

</details>

<details class="api-operation" id="inv" markdown="1">
<summary><code>fps f.inv(n = f.size())</code> — O(M(N))</summary>

`f[0]!=0` を満たす級数の逆元を、`f * f.inv(n) = 1 (mod x^n)` となるようNewton反復で求めます。

{% raw %}
```cpp
fps reciprocal = f.inv(100);
assert((f * reciprocal).pre(100)[0] == 1);
```
{% endraw %}

注意点: `f` は空でなく定数項が非零、`n>=0`。NTT変換長の上限を超える場合は使えません。返り値は常にちょうど `n` 項（`n=0` は空）です。

</details>

<details class="api-operation" id="log" markdown="1">
<summary><code>fps f.log(n = f.size())</code> — O(M(N))</summary>

定数項1の級数について `integral(f.diff() / f)` を `n` 項に切り詰めて返します。

{% raw %}
```cpp
fps logarithm = f.log(100);  // f[0] == 1
```
{% endraw %}

注意点: `f[0]==1` が必須です。`n=0` は空級数。形式積分の次数制約とNTT変換長制約を満たしてください。

</details>

<details class="api-operation" id="sqrt" markdown="1">
<summary><code>fps f.sqrt(n = f.size()) / f.sqrt_with(root, n)</code> — O(M(N) log N)</summary>

平方根の定数項をTonelli--Shanks法で求め、Newton反復で `g^2 = f (mod x^n)` を解きます。
`x^n` 未満にある最初の非零項の次数が奇数、またはその係数に平方根がない場合は空級数を返します。
`x^n` 以上の項は存在判定に影響しません。成功時はちょうど `n` 項を返します。
`sqrt_with` は定数項の平方根を返す関数を利用する版です。

{% raw %}
```cpp
fps root = f.sqrt(100);
fps square{4, 4, 1};
fps root_with_callback = square.sqrt_with([](mint a) {
  assert(a == mint(4));
  return mint(2);             // この例の非零定数項の平方根
}, 3);
assert(((root_with_callback * root_with_callback).pre(3) == square));
```
{% endraw %}

注意点: 奇素数modulusを前提とします。`n=0` は空級数です。入力が空、または `x^n` 未満がすべて0なら
長さ `n` のゼロ級数を返し、コールバックは呼びません。`sqrt_with` のコールバックは正しい平方根を返す責任があり、
非平方剰余に対する検査は行いません。非零の平方根は符号が2通りあり、どちらを返すかに依存しないでください。

</details>

<details class="api-operation" id="exp" markdown="1">
<summary><code>fps f.exp(n = f.size())</code> — O(M(N) log N)</summary>

定数項0の級数について `g' = f' g`, `g[0]=1` を満たす指数級数を、Newton反復と変換再利用で求めます。

{% raw %}
```cpp
fps exponential = f.exp(100);  // f[0] == 0
assert(exponential[0] == 1);
```
{% endraw %}

注意点: `f[0]==0` が必須です。modulusより小さい次数とNTT変換長制約を満たしてください。大きな次数では一時バッファも含めて $O(N)$ メモリを使います。

</details>

<details class="api-operation" id="pow" markdown="1">
<summary><code>fps f.pow(k, n = f.size())</code> — O(M(N) log N)</summary>

非負整数 `k` について `f^k (mod x^n)` を計算します。先頭の0を分離してから
`exp(k * log(normalized))` を適用するため、`f[0]==0` の場合も扱えます。

{% raw %}
```cpp
fps power = f.pow(5, 100);
```
{% endraw %}

注意点: `k>=0`, `n>=0`。先頭0の次数を `v` とすると `v*k>=n` なら全係数0です。`k=0` は定数項1を返します。

</details>

<details class="api-operation" id="mod-pow" markdown="1">
<summary><code>fps f.mod_pow(k, modulus)</code> — O(M(N+D) + M(D) log(k+1))</summary>

多項式を `modulus` で割った余りの環で二分累乗します。

{% raw %}
```cpp
fps relation{1, 0, 1};       // x^2 + 1
fps result = f.mod_pow(1'000'000'000LL, relation);
```
{% endraw %}

注意点: `modulus` はゼロ多項式でなく、指数は非負です。初期の被除数の長さを `N`、
modulusの次数を `D` とします。最初に `f` を剰余化した後、`D>=1` では積の長さは高々 `2D-1` です。
返り値の末尾0は除去されます。`k=0` は `1 % modulus`、非零定数を法とすると指数によらず空級数を返します。
入力の末尾0は許されます。計算中の長さがmodulusとNTTの制約に収まる必要があります。

</details>

<details class="api-operation" id="vector-interface" markdown="1">
<summary><code>f[i]`, `f.size()`, range-for/iterators</code> — O(1)</summary>

`std::vector<Mint>` を公開継承しているため、添字アクセス、`size`, `empty`, `resize`、イテレータなどをそのまま使えます。

{% raw %}
```cpp
for (int i = 0; i < (int)f.size(); ++i) f[i] = mint(i);
```
{% endraw %}

注意点: `resize` などで次数を変更すると、各FPS操作が要求する定数項・長さの前提も変わります。参照やイテレータはvectorと同じく再確保で無効化されます。

</details>

## 出典・検証

設計は [ACL convolution の仕様](https://github.com/atcoder/ac-library/blob/master/document_en/convolution.md)、
[NyaanNyaan/library のFPS実装](https://github.com/NyaanNyaan/library/tree/master/fps)、
[Luzhiled Library のFPS解説](https://ei1333.github.io/luzhiled/snippets/math/formal-power-series.html)
を比較して決めました。積はACLの公開APIを使い、NTT反復の変換再利用とFPS固有の操作は
Blueberry側で再実装しています。コードをそのままコピーせず、API・境界条件・コメントを
このリポジトリの方針に合わせています。

次のLibrary Checker公式問題を `verification-helper` の標準checkerで検証します。

- [Inv of Formal Power Series](https://judge.yosupo.jp/problem/inv_of_formal_power_series)
- [Log of Formal Power Series](https://judge.yosupo.jp/problem/log_of_formal_power_series)
- [Exp of Formal Power Series](https://judge.yosupo.jp/problem/exp_of_formal_power_series)
- [Sqrt of Formal Power Series](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series)
- [Pow of Formal Power Series](https://judge.yosupo.jp/problem/pow_of_formal_power_series)
- [Division of Polynomials](https://judge.yosupo.jp/problem/division_of_polynomials)

`tests/random/formal-power-series.cpp` は二次時間の独立した実装と、逆元・積・商・余り・exp・log・
pow・sqrt・mod_powを固定seedで比較します。疎係数の並び替え・重複、係数自身を引数にするスカラー倍、
空級数・次数0・NTTへの切替境界も検証します。

### Fastest 提出との比較

Library Checker の「Fastest」は、同じ問題の AC 提出を Yosupo 側の実行環境で最大ケース時間順に
並べた値です。`make verify` の集計（複数ケース・複数回・CI runner）とは測定対象が異なるため、
数値をそのまま倍率比較しません。比較するときは、問題・入力・コンパイラ・最適化フラグ・CPUを揃え、
solution 実行時間とコンパイル時間を分けます。

公開実装の構成を調べるには、読み取り専用の次のスクリプトを使えます。REST API の `+time` 順を
取得し、各提出のケース最大時間と NTT/SIMD/ACL 畳み込みなどの特徴を `report.md` にまとめます。

{% raw %}
```bash
python3 scripts/fetch_lc_fastest.py exp_of_formal_power_series \
  --limit 10 --out-dir .verification/lc/exp --save-source
```
{% endraw %}

このスクリプトは提出・ソースの取得だけを行い、自動提出やアカウント操作はしません。Fastest 上位には
AVX2 の手書き Montgomery 算術や専用 NTT が含まれることがあり、ACL `static_modint` と公開 API を
使う本ライブラリとの速度差はアルゴリズムだけでなく係数演算・SIMD・I/O・測定環境にも由来します。
そのため、まず同一条件のローカルベンチマークで候補を比較し、可読性・移植性を損なう最適化は採用しません。

公式問題の最大入力（`N=500000`、modulus `998244353`）を対象に、CIではコンパイル時間と
solution実行時間を分離して直列3回計測します。測定値は同じrunner・compiler・verifyコードの
基準がある場合だけ比較し、Yosupo提出画面の時間とは区別します。
