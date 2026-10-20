---
title: Fast I/O
documentation_of: //blueberry/utility/fast-io.hpp
---

[カテゴリへ戻る]({{ '/categories/utility.html' | relative_url }})

## 概要

`FastInput<Interactive = false>` / `FastOutput`で、整数・文字列・浮動小数点と、pair/vector/deque/arrayの入れ子を読み書きします。
通常ファイルとpipeに対応し、引数を省略するとstdin/stdoutを使います。
対話問題では`FastInput<true>`を選び、出力後に`out.flush()`を呼びます。

入力文字数をL、出力文字数をK、再帰的に訪れる要素・コンテナの数をNとすると、
整数・文字列とコンテナの全処理は償却O(L+K+N)です。
通常の入出力オブジェクトはそれぞれ64 KiBのバッファを持ち、`FastInput<true>`の補助メモリはO(1)です。
浮動小数点は標準変換の費用を加え、一時tokenや出力作業領域に追加メモリを使います。
`scan/print`などの短縮名は[Contest Fast I/O Template]({{ '/docs/utility/contest-fast-io.html' | relative_url }})で使えます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`FastInput<Interactive>`](#input-construct) | O(1) |
| [`in.read`（整数）](#read-integer) | O(d) |
| [`in.read`](#read-bool) | O(d) |
| [`in.read`](#read-char) | O(読み飛ばす空白数+1) |
| [`in.read`](#read-string) | O(d)、再確保を含む償却 |
| [`in.read`（浮動小数点）](#read-float) | O(d)+標準変換関数の費用 |
| [`in.read`](#read-many) | 各readの合計 |
| [`FastOutput`](#output-construct) | O(1) |
| [`out.write`](#write-char) | 償却O(1)、buffer排出時O(b) |
| [`out.write`](#write-bool) | 償却O(1) |
| [`out.write`](#write-string) | 償却O(d) |
| [`out.write`（整数）](#write-integer) | 償却O(d) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`FastInput（コピー構築、使用不可）`](#input-copy) | 使用不可 |
| [`FastInput（コピー代入、使用不可）`](#input-copy-assign) | 使用不可 |
| [`in.~FastInput`](#input-destroy) | O(1) |
| [`in.read（pair）`](#read-pair) | 償却O(d+N)+各変換の費用 |
| [`in.read（vector）`](#read-vector) | 償却O(d+N)+各変換の費用 |
| [`in.read（deque）`](#read-deque) | 償却O(d+N)+各変換の費用 |
| [`in.read（array）`](#read-array) | 償却O(d+N)+各変換の費用 |
| [`FastOutput（コピー構築、使用不可）`](#output-copy) | 使用不可 |
| [`FastOutput（コピー代入、使用不可）`](#output-copy-assign) | 使用不可 |
| [`out.write（固定精度の浮動小数点）`](#write-float-fixed) | 標準変換関数の費用+償却O(d) |
| [`out.write（pair）`](#write-pair) | 償却O(d+N)+各変換の費用 |
| [`out.write（vector）`](#write-vector) | 償却O(d+N)+各変換の費用 |
| [`out.write（deque）`](#write-deque) | 償却O(d+N)+各変換の費用 |
| [`out.write（array）`](#write-array) | 償却O(d+N)+各変換の費用 |
| [`out.~FastOutput`](#output-destroy) | O(b) |
| [`out.write`（浮動小数点）](#write-float) | 標準変換関数の費用+償却O(d) |
| [`out.writeln`](#writeln) | 各writeと区切りの合計、償却 |
| [`out.flush`](#flush) | O(b) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 整数の符号・オーバーフローを検査します。標準整数型の全範囲とfloat/double/long doubleに対応し、GNU C++環境では128bit整数も扱えます。
- 空白はASCIIのspace、tab、LF、CR、vertical tab、form feedです。localeには依存しません。
- 整数は10進数、浮動小数点は`from_chars`の一般形式（指数、inf/nanを含む）です。
- boolは0/1、`char`は1文字、signed/unsigned charは整数として扱います。
- 文字列入力には格納先の長さ、浮動小数点入力には一時tokenの長さの追加メモリが必要です。固定精度の浮動小数点出力は通常128byteのstack領域を使い、収まらないときだけO(precision + 型の最大10進指数)の作業領域を確保します。FILE内部のバッファは別です。
- pair/vector/deque/arrayは再帰的に処理し、要素も対応するread/writeを持つ型であることが必要です。入力前にコンテナのサイズ・形状を用意します。入れ子の深さに応じたstack領域が必要です。
- 任意のstream演算子は呼びません。ACL modint等は整数に読み込んでから変換してください。公開フィールドはありません。

### 操作の補足 {#操作の補足}

- 文字数dは、入力では読み飛ばす空白も含めた消費文字数、出力では出力の長さです。Nは再帰的に訪れる要素・コンテナの数で、空コンテナも数えます。bはその時点の未出力バッファ長（最大64 KiB）です。stdio・OSのI/O待ち時間は計算量に含めません。

### FILEの寿命と併用 {#fileの寿命と併用}

- nonnullの`FILE*`を借用します。FILEはオブジェクトより長く生存させてください。FILEを閉じる処理は呼び出し側で行います。
- コピー・移動はできません。同じFILEでcin/scanfなどと入力を混ぜたり、オブジェクトの生存中にseekしたりしないでください。
- 先読みした未消費の入力は、オブジェクトを破棄してもFILEへ戻しません。
- 出力も他のwriterとの混在を避け、順序を切り替える場合は双方をflushしてください。
- 同じオブジェクトへの並行アクセスは外部同期が必要です。異なるFILE/オブジェクト間で共有する可変状態はありません。

### 実装の補足 {#実装の補足}

- 通常の入力はbatch向けの`fread`経路です。対話用は1文字ずつ`fgetc`を呼びます。通常入力のままflushだけを追加しても、短い応答を待つ対話には対応できません。
- `writeln`自体はflushしません。
- mmap・固定入力桁数・特定CPUの命令に依存しません。バッファを初期zero-fillせず、整数出力は2桁ずつ変換します。ACLには対応機能がありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdio>
#include <string>
#include "blueberry/utility/fast-io.hpp"
int main() {
  std::FILE* f = std::tmpfile();
  if (!f) return 1;
  {
    blueberry::FastOutput out(f);
    const bool ok = out.writeln(-123, "hello", 1.25) && out.flush();
    assert(ok);
    if (!ok) return 1;
  }
  std::rewind(f);
  {
    blueberry::FastInput in(f);
    int x = 0; std::string word; double y = 0;
    const bool ok = in.read(x, word, y);
    assert(ok && x == -123 && word == "hello" && y == 1.25);
    if (!ok) return 1;
    assert(!in.read(x)); // EOF。xは変更しない。
  }
  std::fclose(f);
}
```
{% endraw %}

<details class="api-operation" id="input-construct" markdown="1">
<summary><code>FastInput&lt;Interactive = false&gt;(FILE* file = stdin)</code> — O(1)</summary>

読み取り可能なnonnull FILEを借用します。空ファイルも有効です。
注意点: copy constructor/assignmentはdeletedで、moveもできません。既定のdestructorはFILEを閉じません。
`Interactive=false`はbatch用の64 KiB先読み、`true`は対話用のfgetc経路で補助メモリO(1)。
選択はコンパイル時で、batchの1文字ごとの処理にmode分岐を追加しません。
テンプレート引数を省略した`FastInput in;`はC++20の推論でfalseを使用します。

{% raw %}
```cpp
blueberry::FastInput in; // stdin。FILEはobject破棄まで有効にする。
// 対話問題では代わりに blueberry::FastInput<true> in;
```
{% endraw %}
</details>

<details class="api-operation" id="read-integer" markdown="1">
<summary><code>bool read(T&amp; value)</code>（整数） — O(d)</summary>

bool/char以外の整数型Tに、空白区切りの10進整数を読み込みます。先頭の`+`/`-`を扱い、
注意点: unsignedへの負数は`-0`を含め失敗します。符号だけ・非数字混入・範囲外はfalse。
空白/EOFまでそのtokenを消費し、失敗時はvalueを変更しません。範囲外計算でsigned overflowを起こしません。
EOF前の最後の数は末尾改行なしでも成功します。tokenなしのEOFまたはFILEの読み取りエラーもfalseです。
不正tokenの後の次tokenは、次のreadで読めます。FILEエラーの詳細は借用元FILEの`ferror`で確認できます。

{% raw %}
```cpp
long long x = 0;
if (!in.read(x)) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="read-bool" markdown="1">
<summary><code>bool read(bool&amp; value)</code> — O(d)</summary>

unsigned整数としてtokenを読み、0ならfalse、1ならtrueを格納します。
それ以外の数・負符号・true/falseという文字列は失敗し、valueを変更しません。
注意点: 返り値は読み取り成功かどうかであり、読み取ったboolの値とは別です。

{% raw %}
```cpp
bool flag = false;
if (!in.read(flag)) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="read-char" markdown="1">
<summary><code>bool read(char&amp; value)</code> — O(空白数+1)</summary>

空白を読み飛ばし、次の1byteを取得します。token全体ではなく1文字だけ消費します。
EOF/FILEエラーならfalseでvalueを変更しません。注意点: UTF-8の1文字ではなくbyte単位です。

{% raw %}
```cpp
char command = '?';
if (!in.read(command)) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="read-string" markdown="1">
<summary><code>bool read(std::string&amp; value)</code> — 償却O(d)</summary>

空白区切りの非空tokenを格納し、既存capacityを再利用します。tokenなしのEOFではfalseでvalueは不変。
途中のFILE読み取りエラーではfalseで、valueに読み取り済みの部分が残る場合があります。
通常の末尾EOFは成功です。改行を含む行入力・空tokenの読み取りではありません。
注意点: 再確保時にはvalueへの既存参照・iteratorが無効になります。allocation例外は通常どおり送出されます。

{% raw %}
```cpp
std::string word;
if (!in.read(word)) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="read-float" markdown="1">
<summary><code>bool read(T&amp; value)</code>（浮動小数点） — O(d)+標準変換の費用</summary>

float/double/long doubleをlocale非依存で読み込みます。`from_chars`の一般形式に加え先頭`+`を許します。
token全体が有効で範囲内の場合のみtrue。範囲外・不正形式・EOF・FILEエラーではfalseでvalueは不変です。
一時tokenにO(d)メモリを使います。hex形式やlocale小数点は対象外。
注意点: 丸めとinf/nanの扱いは処理系の標準変換に従います。

{% raw %}
```cpp
double x = 0;
if (!in.read(x)) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="read-many" markdown="1">
<summary><code>bool read(T&amp; x, U&amp; y, Rest&amp;... rest)</code> — 償却O(d+N)+各変換の費用</summary>

2個以上の引数を左から読み、最初の失敗で停止します。既に成功した引数は更新済みで、
後続の引数/tokenは触りません。注意点: グループ全体を巻き戻す操作ではありません。

{% raw %}
```cpp
int n = 0, q = 0;
if (!in.read(n, q)) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="output-construct" markdown="1">
<summary><code>FastOutput(FILE* file = stdout)</code> — O(1)</summary>

書き込み可能なnonnull FILEを借用します。copy constructor/assignmentはdeletedでmoveもできません。
注意点: ファイルのcloseやseekはobject破棄後に行ってください。

{% raw %}
```cpp
blueberry::FastOutput out; // stdout
```
{% endraw %}
</details>

<details class="api-operation" id="output-destroy" markdown="1">
<summary><code>~FastOutput()</code> — O(b)</summary>

flushを試みますが、destructorから失敗を返しません。出力エラーを検出したい場合は破棄前にflushの戻り値を確認します。
注意点: FILEは閉じません。異常終了や`std::exit`による自動objectのdestructor省略には依存しないでください。

{% raw %}
```cpp
{ blueberry::FastOutput out; out.writeln("done"); } // scope末尾でflush
```
{% endraw %}
</details>

<details class="api-operation" id="write-char" markdown="1">
<summary><code>bool write(char value)</code> — 償却O(1)</summary>

1byteをbufferへ追加します。trueは受け付けたことを表し、OSへの送出成功までは保証しません。
buffer排出時のFILEエラー以降はfalseを返します。注意点: 整数としてcharの値を出す場合はint等にcastします。

{% raw %}
```cpp
out.write(' ');
```
{% endraw %}
</details>

<details class="api-operation" id="write-bool" markdown="1">
<summary><code>bool write(T value)</code>（T = bool） — 償却O(1)</summary>

falseを文字`0`、trueを文字`1`として出力します。改行・空白は追加しません。
注意点: bool型だけを受け付けます。文字列literalをboolへ暗黙変換しません。

{% raw %}
```cpp
out.write(true); // "1"
```
{% endraw %}
</details>

<details class="api-operation" id="write-string" markdown="1">
<summary><code>bool write(std::string_view value)</code> — 償却O(d)</summary>

byte列をそのまま追加します。空文字列も有効で、埋め込まれたNULも出力します。
stringや文字列literalからも呼べます。呼び出し中だけ元のbyte列が有効ならよく、参照を保持しません。
注意点: 改行や空白は自動追加しません。既にFILEエラーになっている場合は空文字列でもfalse。

{% raw %}
```cpp
out.write("answer: ");
```
{% endraw %}
</details>

<details class="api-operation" id="write-integer" markdown="1">
<summary><code>bool write(T value)</code>（整数） — 償却O(d)</summary>

bool/char以外の整数型を10進数で出力します。signed最小値・unsigned最大値も扱い、
GNU環境ではsigned/unsigned 128bitに対応します。空白・改行は付けません。
注意点: overflowするsigned絶対値の計算は行いません。

{% raw %}
```cpp
out.write(std::numeric_limits<long long>::min());
```
{% endraw %}
</details>

<details class="api-operation" id="write-float" markdown="1">
<summary><code>bool write(T value)</code>（浮動小数点） — 標準変換の費用+償却O(d)</summary>

float/double/long doubleを`to_chars`の一般形式・最短round-trip表現で出力します。
このオーバーロードには小数点以下の固定桁指定やlocaleはありません。固定桁には`write(value, precision)`を使います。inf/nan・負の0も標準変換に従います。
注意点: 変換またはFILE出力エラーでfalse。FILEエラーは以後も保持します。

{% raw %}
```cpp
out.write(1.25);
```
{% endraw %}
</details>

<details class="api-operation" id="writeln" markdown="1">
<summary><code>bool writeln(const Args&amp;... values)</code> — 償却O(d+N)+各変換の費用</summary>

引数間を1個のspaceで区切り、末尾にLFを付けます。引数0個ではLFだけ。
注意点: 最初のwrite失敗で後続の出力を停止します。途中までの出力は取り消しません。flushはしません。

{% raw %}
```cpp
out.writeln(123, "ok");
out.writeln();
```
{% endraw %}
</details>

<details class="api-operation" id="flush" markdown="1">
<summary><code>bool flush()</code> — O(b)</summary>

自分のbufferをFILEへ排出し、さらに`fflush`します。短いfwrite結果にも対応し、送信済み部分を再送しません。
エラー時はfalseで以後のwrite/flushもfalse。空bufferでもfflushを呼びます。
注意点: 対話問題では相手の返答を読む前に呼びます。物理媒体へのfsyncを保証する操作ではありません。

{% raw %}
```cpp
if (!out.flush()) return 1;
```
{% endraw %}
</details>

<details class="api-operation" id="input-copy" markdown="1">
<summary><code>FastInput(const FastInput&amp;) = delete</code> — O(1)（宣言のみ）</summary>

コピー構築は禁止です。同じFILEのバッファ状態を複製しません。

{% raw %}
```cpp
static_assert(!std::is_copy_constructible_v<blueberry::FastInput<>>);
```
{% endraw %}

注意点: 独立したFILEについて別のオブジェクトを構築してください。移動構築もできません。

</details>

<details class="api-operation" id="input-copy-assign" markdown="1">
<summary><code>FastInput&amp; operator=(const FastInput&amp;) = delete</code> — O(1)（宣言のみ）</summary>

コピー代入は禁止です。構築後のFILE付け替えはできません。

{% raw %}
```cpp
static_assert(!std::is_copy_assignable_v<blueberry::FastInput<>>);
```
{% endraw %}

注意点: 移動代入もできません。代入の代わりに適切なFILEで新しいオブジェクトを構築します。

</details>

<details class="api-operation" id="output-copy" markdown="1">
<summary><code>FastOutput(const FastOutput&amp;) = delete</code> — O(1)（宣言のみ）</summary>

コピー構築は禁止です。同じFILEのバッファ状態を複製しません。

{% raw %}
```cpp
static_assert(!std::is_copy_constructible_v<blueberry::FastOutput>);
```
{% endraw %}

注意点: 独立したFILEについて別のオブジェクトを構築してください。移動構築もできません。

</details>

<details class="api-operation" id="output-copy-assign" markdown="1">
<summary><code>FastOutput&amp; operator=(const FastOutput&amp;) = delete</code> — O(1)（宣言のみ）</summary>

コピー代入は禁止です。構築後のFILE付け替えはできません。

{% raw %}
```cpp
static_assert(!std::is_copy_assignable_v<blueberry::FastOutput>);
```
{% endraw %}

注意点: 移動代入もできません。代入の代わりに適切なFILEで新しいオブジェクトを構築します。

</details>

<details class="api-operation" id="input-destroy" markdown="1">
<summary><code>~FastInput()</code> — O(1)</summary>

入力バッファを破棄します。借用しているFILEは閉じません。

{% raw %}
```cpp
{ blueberry::FastInput in(f); } // fは開いたまま
```
{% endraw %}

注意点: 先読み済みの未消費byteはFILEへ戻しません。オブジェクト破棄後に同じ位置から他のreaderへ引き継ぐ用途には使えません。

</details>

<details class="api-operation" id="read-pair" markdown="1">
<summary><code>bool read(std::pair&lt;T, U&gt;&amp; value)</code> — 償却O(d+N)+各変換の費用</summary>

first、secondの順に再帰的に読み、両方成功したときtrueを返します。

{% raw %}
```cpp
std::pair<int, std::vector<int>> value{0, std::vector<int>(2)};
if (!in.read(value)) return 1;
```
{% endraw %}

注意点: 最初の失敗で停止し、既に読んだ部分は変更済みです。secondの失敗でもfirstは巻き戻しません。コンテナのサイズ変更は行いません。要素の文字列等の確保は各readに従います。

</details>

<details class="api-operation" id="read-vector" markdown="1">
<summary><code>bool read(std::vector&lt;T, Alloc&gt;&amp; values)</code> — 償却O(d+N)+各変換の費用</summary>

既存の要素を先頭から再帰的に読み、全て成功ならtrueを返します。
サイズを変更せず、空コンテナは何も読まずtrueです。`vector<bool>`も一時bool経由で読み取れます。

{% raw %}
```cpp
std::vector<int> values(2);
if (!in.read(values)) return 1;
```
{% endraw %}

注意点: 最初の失敗で停止し、成功した要素は更新済みです。参照・iteratorはこの操作によって無効化しませんが、要素自身のstring等の再確保は各型の規則に従います。

</details>

<details class="api-operation" id="read-deque" markdown="1">
<summary><code>bool read(std::deque&lt;T, Alloc&gt;&amp; values)</code> — 償却O(d+N)+各変換の費用</summary>

既存の要素を先頭から再帰的に読み、全て成功ならtrueを返します。
サイズを変更せず、空コンテナは何も読まずtrueです。

{% raw %}
```cpp
std::deque<int> values(2);
if (!in.read(values)) return 1;
```
{% endraw %}

注意点: 最初の失敗で停止し、成功した要素は更新済みです。参照・iteratorはこの操作によって無効化しませんが、要素自身のstring等の再確保は各型の規則に従います。

</details>

<details class="api-operation" id="read-array" markdown="1">
<summary><code>bool read(std::array&lt;T, M&gt;&amp; values)</code> — 償却O(d+N)+各変換の費用</summary>

既存の要素を先頭から再帰的に読み、全て成功ならtrueを返します。
サイズを変更せず、空コンテナは何も読まずtrueです。

{% raw %}
```cpp
std::array<int, 2> values{};
if (!in.read(values)) return 1;
```
{% endraw %}

注意点: 最初の失敗で停止し、成功した要素は更新済みです。参照・iteratorはこの操作によって無効化しませんが、要素自身のstring等の再確保は各型の規則に従います。

</details>

<details class="api-operation" id="write-float-fixed" markdown="1">
<summary><code>bool write(T value, int precision)</code> — 標準変換の費用+償却O(d)</summary>

float/double/long doubleをlocale非依存のfixed形式で出力します。
有限値は小数点以下precision桁に丸めます。0なら小数部分を出しません。
通常128byteのstack領域に変換し、足りない場合だけ作業領域を確保します。

{% raw %}
```cpp
out.write(1.25, 3); // "1.250"
```
{% endraw %}

注意点: precision >= 0が必要です。inf/nan・負の0・丸め方はto_charsに従います。極端なprecisionは時間・メモリを増やし、確保失敗は例外になります。変換またはFILEエラーでfalse。出力に失敗しても取り消さず、FILEエラーは保持します。改行・空白は付けません。

</details>

<details class="api-operation" id="write-pair" markdown="1">
<summary><code>bool write(const std::pair&lt;T, U&gt;&amp; value)</code> — 償却O(d+N)+各変換の費用</summary>

firstとsecondを1個の空白で区切って再帰的に出力します。成功ならtrueです。

{% raw %}
```cpp
out.write(std::pair<int, int>{1, 2}); // "1 2"
```
{% endraw %}

注意点: 最初の失敗で停止し、途中までの出力は取り消しません。改行・flushをせず、参照も保持しません。空コンテナを要素に持っても間の区切りは出します。

</details>

<details class="api-operation" id="write-vector" markdown="1">
<summary><code>bool write(const std::vector&lt;T, Alloc&gt;&amp; values)</code> — 償却O(d+N)+各変換の費用</summary>

要素を1個の空白で区切って再帰的に出力し、成功ならtrueを返します。
空コンテナは何も出力せずtrueです。`vector<bool>`にも対応します。

{% raw %}
```cpp
out.write(std::vector<int>{1, 2}); // "1 2"
```
{% endraw %}

注意点: 末尾に空白・改行を付けずflushもしません。形状やサイズは保存せず、空の入れ子でも要素間の区切りは残ります。最初の失敗で停止し、途中までの出力は取り消しません。参照を保持しません。既にFILEエラーでも空コンテナはwriteを呼ばずtrueになります。

</details>

<details class="api-operation" id="write-deque" markdown="1">
<summary><code>bool write(const std::deque&lt;T, Alloc&gt;&amp; values)</code> — 償却O(d+N)+各変換の費用</summary>

要素を1個の空白で区切って再帰的に出力し、成功ならtrueを返します。
空コンテナは何も出力せずtrueです。

{% raw %}
```cpp
out.write(std::deque<int>{1, 2}); // "1 2"
```
{% endraw %}

注意点: 末尾に空白・改行を付けずflushもしません。形状やサイズは保存せず、空の入れ子でも要素間の区切りは残ります。最初の失敗で停止し、途中までの出力は取り消しません。参照を保持しません。既にFILEエラーでも空コンテナはwriteを呼ばずtrueになります。

</details>

<details class="api-operation" id="write-array" markdown="1">
<summary><code>bool write(const std::array&lt;T, M&gt;&amp; values)</code> — 償却O(d+N)+各変換の費用</summary>

要素を1個の空白で区切って再帰的に出力し、成功ならtrueを返します。
空コンテナは何も出力せずtrueです。

{% raw %}
```cpp
out.write(std::array<int, 2>{1, 2}); // "1 2"
```
{% endraw %}

注意点: 末尾に空白・改行を付けずflushもしません。形状やサイズは保存せず、空の入れ子でも要素間の区切りは残ります。最初の失敗で停止し、途中までの出力は取り消しません。参照を保持しません。既にFILEエラーでも空コンテナはwriteを呼ばずtrueになります。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Many A + B](https://judge.yosupo.jp/problem/many_aplusb)・[128bit版](https://judge.yosupo.jp/problem/many_aplusb_128bit)で公式verify。
- `tests/random/fast-io.cpp`: 整数全範囲・overflow・不正token・buffer境界・末尾改行なし・浮動小数点・文字列・出力エラーの検証。
- `tests/random/contest-fast-io.cpp`: 固定精度・入れ子コンテナ・vector&lt;bool&gt;とテンプレート形式APIの検証。
- 2桁変換とportable bufferを独自実装。Fastest提出のmmap/SIMD専用I/Oはコピーしていません。
- [性能改善方針]({{ '/docs/development/fastest-performance-plan.html' | relative_url }})。

</details>
