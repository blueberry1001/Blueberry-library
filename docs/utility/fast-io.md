---
title: Fast I/O
documentation_of: //blueberry/utility/fast-io.hpp
---

[カテゴリへ戻る]({{ '/categories/utility.html' | relative_url }})

## 概要・前提

`blueberry::FastInput<Interactive = false>` / `blueberry::FastOutput`は、借用した`FILE*`に対する入出力です。
通常ファイルとpipeで利用でき、mmap・固定された入力桁数・特定CPUの命令を必要としません。
ACLには対応機能がありません。整数の符号・overflowを検査し、GNU C++環境では128bit整数も扱えます。

通常の入力objectと出力objectは各64 KiBの固定bufferを持ちます。`FastInput<true>`は対話用で
1文字ずつ`fgetc`を呼び、64 KiBが揃うのを待ちません。通常の`fread`経路はbatch用途です。
入力文字数をL、出力文字数をKとすると、
整数・文字列の全処理はO(L+K)、各objectの補助メモリはO(64 KiB)。文字列入力は格納先の長さ、
浮動小数点入力は一時tokenの長さの追加メモリを使います。FILE内部のbufferは別です。
I/O bufferを初期zero-fillせず、整数出力は2桁ずつ変換します。

空白はASCIIのspace、tab、LF、CR、vertical tab、form feedです。localeには依存しません。
整数は10進数、浮動小数点は`from_chars`の一般形式（指数、inf/nanを含む）を扱います。
boolは0/1として、`char`は1文字として、signed/unsigned charは整数として扱います。
標準整数型の全範囲とfloat/double/long doubleに対応し、128bitは処理系拡張です。

FILEはnonnullで、objectより長く生存させてください。objectはFILEを閉じません。
copy/moveはできません。同じFILEでcin/scanf等と入力を混ぜたり、objectの生存中にseekしたりしないでください。
入力は先読みするため、object破棄で未消費bufferをFILEへ戻しません。
出力も他のwriterとの混在を避け、順序を切り替える場合は双方をflushしてください。
同じobjectへの並行アクセスは外部同期が必要です。異なるFILE/object間で共有する可変状態はありません。

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

競技中は`FastInput in; FastOutput out;`でstdin/stdoutを使用します。
対話問題では`FastInput<true> in;`を使い、出力後に`out.flush()`を明示します。
通常のFastInputのままflushだけ追加しても、短い応答を待つ対話には対応できません。
`writeln`自体はflushしません。

## 操作一覧

文字数dは入力なら読み飛ばす空白も含めた消費文字数、出力なら出力の長さ、bはその時点の未出力buffer長（最大64 KiB）です。
stdio・OSのI/O待ち時間は、文字数に対する計算量とは別です。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `FastInput<Interactive> in(FILE* f = stdin)` | O(1) | [開く](#input-construct) |
| `bool in.read(T& x)`（整数） | O(d) | [開く](#read-integer) |
| `bool in.read(bool& x)` | O(d) | [開く](#read-bool) |
| `bool in.read(char& x)` | O(読み飛ばす空白数+1) | [開く](#read-char) |
| `bool in.read(std::string& x)` | O(d)、再確保を含む償却 | [開く](#read-string) |
| `bool in.read(T& x)`（浮動小数点） | O(d)+標準変換関数の費用 | [開く](#read-float) |
| `bool in.read(T& x, U& y, Rest&... rest)` | 各readの合計 | [開く](#read-many) |
| `FastOutput out(FILE* f = stdout)` | O(1) | [開く](#output-construct) |
| `out.~FastOutput()` | O(b) | [開く](#output-destroy) |
| `bool out.write(char x)` | 償却O(1)、buffer排出時O(b) | [開く](#write-char) |
| `bool out.write(bool x)` | 償却O(1) | [開く](#write-bool) |
| `bool out.write(std::string_view x)` | 償却O(d) | [開く](#write-string) |
| `bool out.write(T x)`（整数） | 償却O(d) | [開く](#write-integer) |
| `bool out.write(T x)`（浮動小数点） | 標準変換関数の費用+償却O(d) | [開く](#write-float) |
| `bool out.writeln(const Args&... values)` | 各writeと区切りの合計、償却 | [開く](#writeln) |
| `bool out.flush()` | O(b) | [開く](#flush) |

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
<summary><code>bool read(T&amp; x, U&amp; y, Rest&amp;... rest)</code> — O(総文字数)+各変換の費用</summary>

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
小数点以下の固定桁指定やlocaleはありません。inf/nan・負の0も標準変換に従います。
注意点: 変換またはFILE出力エラーでfalse。FILEエラーは以後も保持します。

{% raw %}
```cpp
out.write(1.25);
```
{% endraw %}
</details>

<details class="api-operation" id="writeln" markdown="1">
<summary><code>bool writeln(const Args&amp;... values)</code> — 償却O(総文字数)+各変換の費用</summary>

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

## 出典・検証

- [Many A + B](https://judge.yosupo.jp/problem/many_aplusb)・[128bit版](https://judge.yosupo.jp/problem/many_aplusb_128bit)で公式verify。
- `tests/random/fast-io.cpp`: 整数全範囲・overflow・不正token・buffer境界・末尾改行なし・浮動小数点・文字列・出力エラーの検証。
- 2桁変換とportable bufferを独自実装。Fastest提出のmmap/SIMD専用I/Oはコピーしていません。
- [性能改善方針]({{ '/docs/development/fastest-performance-plan.html' | relative_url }})。
