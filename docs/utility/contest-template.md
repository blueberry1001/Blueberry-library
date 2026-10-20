---
title: Contest Template（整理版・明示 opt-in）
documentation_of: //blueberry/utility/contest-template.hpp
---

## 概要

競技用の短い型別名・ループ・入出力を、必要なときだけ追加するヘッダです。
GNU libstdc++とACLが必要で、単一translation unitの提出用です。
`all.hpp`には含まれません。`main`と`init_io()`の呼び出しは自分で書きます。
同じ短縮名で高速入出力を使う場合は[Contest Fast I/O Template]({{ '/blueberry/utility/contest-fast-io.hpp.html' | relative_url }})を選べます。
両方のヘッダを同じtranslation unitでincludeすることはできません。

Nは要素数、S/Lは入出力のbyte数、V/Eはグラフの頂点/辺数です。操作ごとの計算量は下表に示します。

元の名前・定義のまま使う場合は[blueberrytemplate.h（原文）]({{ '/blueberrytemplate.html' | relative_url }})をコピーしてください。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`init_io`](#init-io) | O(1) |
| [`rep(n)`](#rep1) | O(max(0,n)) |
| [`rep(i,n)`](#rep2) | O(max(0,n)) |
| [`rep(i,a,b)`](#rep3) | O(max(0,b-a)) |
| [`rep(i,a,b,c)`](#rep4) | O(1 + max(0,(b-a)/c)) |
| [`scan`](#scan) | O(S) |
| [`print`](#print) | O(S) |
| [`ALL`](#all) | O(1) |
| [`rALL`](#rall) | O(1) |
| [`LL`](#ll) | O(S) |
| [`STR`](#str) | O(S) |
| [`YesNo`](#yesno) | O(1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`operator>>（input128）`](#input128) | O(L) |
| [`operator<<（output128）`](#output128) | O(L) |
| [`operator>>（vector）`](#input-vector) | O(S+N) |
| [`operator<<（vector）`](#output-vector) | O(S+N) |
| [`operator>>（deque）`](#input-deque) | O(S+N) |
| [`operator<<（deque）`](#output-deque) | O(S+N) |
| [`operator>>（pair）`](#input-pair) | O(S) |
| [`operator<<（pair）`](#output-pair) | O(S) |
| [`YESNO`](#yesno-upper) | O(1) |
| [`bit`](#bit) | O(1) |
| [`popc`](#popcount) | O(1) |
| [`crmp`](#in-range) | O(1) |
| [`object.fi`](#fi) | O(1) |
| [`object.se`](#se) | O(1) |
| [`container.pb`](#pb) | O(1) macro expansion |
| [`container.eb`](#eb) | O(1) macro expansion |
| [`Endl`](#endl) | O(1) macro expansion |
| [`spa`](#space) | O(1) |
| [`O`](#o) | O(S) |
| [`divide`](#divide) | O(1) |
| [`iskaibun`](#palindrome) | O(N) |
| [`ran`](#range-sum) | O(1) |
| [`zaatu`](#compress) | O(N log N) |
| [`isbipartite_graph`](#bipartite) | O(V+E) |
| [`Rande（既定）`](#rande-default) | O(1) |
| [`Rande（seed指定）`](#rande-seed) | O(1) |
| [`rng(a,b)`](#rande-range) | O(1) expected |
| [`rng(b)`](#rande-bound) | O(1) expected |
| [`ll / ld / ull / uint / str / mint / mint1 / pqg<T>`](#aliases) | O(1) |
| [`constexpr ll INF`](#inf) | O(1) |
| [`constexpr ld CPS`](#cps) | O(1) |
| [`constexpr std::array<ll,8> dx`](#dx) | O(1) |
| [`constexpr std::array<ll,8> dy`](#dy) | O(1) |
| [`bin_search / chmin / chmax / Fill / len / sum / min_of / max_of / my_distance / parse128 / to_string128`](#imports) | O(各関数の計算量) |
| [`Rande`（コピー構築）](#rande-copy) | O(1) |
| [`Rande& operator=`（コピー代入）](#rande-copy-assign) | O(1) |
| [`Rande`（移動構築）](#rande-move) | O(1) |
| [`Rande& operator=`（移動代入）](#rande-move-assign) | O(1) |
| [`~Rande`](#rande-destruct) | O(1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- グローバルな型別名・マクロ・stream overloadを追加します。名前空間内の補助関数だけが必要ならContest Helpersを使えます。
- `bits/stdc++.h`を使用します。自動終了・未呼び出しのグローバル初期化・可変グローバルテーブルはありません。
- ACLの非inline関数定義があるため、複数cppから同時にincludeしてlinkする用途には対応しません。ACLを含まないContest HelpersとCombinations自体は、複数translation unitで利用できます。
- `BLUEBERRY_REP1`～`BLUEBERRY_REP4`と`BLUEBERRY_REP_SELECT`は`rep`の展開用内部マクロです。直接呼ばず、同名のマクロを定義しないでください。
- `Rande`は暗黙のコピー・移動・代入・破棄を利用します。公開フィールドはありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <sstream>
#include "blueberry/utility/contest-template.hpp"
int main() {
  std::vector<ll> a{3, 1, 4};
  ll total = 0; rep(i, len(a)) total += a[i];
  assert(total == 8 && sum(a) == 8);
  std::sort(ALL(a)); assert(a[0] == 1);
  int x = 8; assert(chmin(x, 2) && x == 2);
  std::istringstream in("1 -2 3"); std::vector<ll> b(3); in >> b;
  std::ostringstream out; out << b; assert(out.str() == "1 -2 3");
  blueberry::Combinations<mint> comb(5); assert(comb.C(5,2).val() == 10);
}
```
{% endraw %}

<details class="api-operation" id="init-io" markdown="1">
<summary><code>void init_io(int precision = 30)</code> — O(1)</summary>

cin の tie を外し同期を無効化、cout を fixed と指定精度に設定します。

{% raw %}
```cpp
init_io(15);
```
{% endraw %}

注意点: 最初の入出力より前に呼び、precision >= 0 が必要です。自動呼出しはありません。以降 printf/scanf など C stdio と混在させないでください。対話出力は明示的に flush してください。

</details>

<details class="api-operation" id="scan" markdown="1">
<summary><code>void scan(Args&amp;... values)</code> — O(S)</summary>

cin で引数を左から順に読みます。空引数は何もしません。

{% raw %}
```cpp
ll n = 0; str s; scan(n, s);
```
{% endraw %}

注意点: S は通常の入力 byte 数です。各型の operator>> が必要です。失敗は cin.fail() で確認します。途中失敗時は既に読んだ値は変更済みです。

</details>

<details class="api-operation" id="print" markdown="1">
<summary><code>void print(const Args&amp;... values)</code> — O(S)</summary>

cout に空白区切りで出力し、末尾に改行を付けます。空引数は改行のみです。

{% raw %}
```cpp
print(1, "hello"); // 1 hello\n
```
{% endraw %}

注意点: S は通常の出力 byte 数です。各型の operator<< が必要です。改行では flush しません。失敗は cout.fail() で確認します。

</details>

<details class="api-operation" id="input128" markdown="1">
<summary><code>std::istream&amp; operator&gt;&gt;(std::istream&amp; in, __int128&amp; value)</code> — O(L)</summary>

次の空白区切り token を parse128 で厳密に読みます。

{% raw %}
```cpp
std::istringstream in("-123"); __int128 x = 0; in >> x; assert(x == -123);
```
{% endraw %}

注意点: 不正構文・overflow は failbit を設定し value を変更しません。token がない EOF でも変更しません。token は消費されます。signed 128 bit のみです。

</details>

<details class="api-operation" id="output128" markdown="1">
<summary><code>std::ostream&amp; operator&lt;&lt;(std::ostream&amp; out, __int128 value)</code> — O(L)</summary>

signed 128 bit を十進表記で出力します。

{% raw %}
```cpp
std::ostringstream out; out << __int128(-123); assert(out.str() == "-123");
```
{% endraw %}

注意点: 最小値にも対応します。hex/showbase など整数の書式 flag は適用せず文字列として出力します。width/fill は文字列の規則です。作業メモリ O(L)、L<=40。

</details>

<details class="api-operation" id="input-vector" markdown="1">
<summary><code>std::istream&amp; operator&gt;&gt;(std::istream&amp; in, std::vector&lt;T&gt;&amp; a)</code> — O(S+N)</summary>

既存 N 要素を順番に読みます。

{% raw %}
```cpp
std::vector<int> a(3); std::istringstream in("1 2 3"); in >> a;
```
{% endraw %}

注意点: 要素数は変更しません。空 vector は何も読みません。vector<bool> は proxy 型のため対象外です。失敗時は一部変更されます。入れ子の vector/deque/pair に対応します。

</details>

<details class="api-operation" id="output-vector" markdown="1">
<summary><code>std::ostream&amp; operator&lt;&lt;(std::ostream&amp; out, const std::vector&lt;T&gt;&amp; a)</code> — O(S+N)</summary>

要素を空白区切りで出力します。

{% raw %}
```cpp
std::ostringstream out; out << std::vector<int>{1,2}; assert(out.str() == "1 2");
```
{% endraw %}

注意点: 空 vector は何も出力せず、末尾空白・改行は付けません。入れ子は flat な空白列となり、形状は保存しません。

</details>

<details class="api-operation" id="input-deque" markdown="1">
<summary><code>std::istream&amp; operator&gt;&gt;(std::istream&amp; in, std::deque&lt;T&gt;&amp; a)</code> — O(S+N)</summary>

既存 N 要素を順番に読みます。

{% raw %}
```cpp
std::deque<int> a(2); std::istringstream in("4 5"); in >> a;
```
{% endraw %}

注意点: サイズ変更はありません。空・入れ子・部分失敗の扱いは vector 入力と同じです。

</details>

<details class="api-operation" id="output-deque" markdown="1">
<summary><code>std::ostream&amp; operator&lt;&lt;(std::ostream&amp; out, const std::deque&lt;T&gt;&amp; a)</code> — O(S+N)</summary>

要素を空白区切りで出力します。

{% raw %}
```cpp
std::ostringstream out; out << std::deque<int>{4,5}; assert(out.str() == "4 5");
```
{% endraw %}

注意点: 空・改行・入れ子の扱いは vector 出力と同じです。

</details>

<details class="api-operation" id="input-pair" markdown="1">
<summary><code>std::istream&amp; operator&gt;&gt;(std::istream&amp; in, std::pair&lt;T,U&gt;&amp; a)</code> — O(S)</summary>

first、second の順に読みます。

{% raw %}
```cpp
std::pair<int,int> p; std::istringstream in("4 5"); in >> p;
```
{% endraw %}

注意点: 各要素が入力可能であることが必要です。途中失敗では first だけ変更されることがあります。

</details>

<details class="api-operation" id="output-pair" markdown="1">
<summary><code>std::ostream&amp; operator&lt;&lt;(std::ostream&amp; out, const std::pair&lt;T,U&gt;&amp; a)</code> — O(S)</summary>

first と second を 1 個の空白で区切ります。

{% raw %}
```cpp
std::ostringstream out; out << std::pair<int,int>{4,5}; assert(out.str() == "4 5");
```
{% endraw %}

注意点: 各要素が出力可能であることが必要です。改行しません。

</details>

<details class="api-operation" id="rep1" markdown="1">
<summary><code>rep(n)</code> — O(max(0,n))</summary>

ll i を宣言し [0,n) を回します。

{% raw %}
```cpp
ll total = 0; rep(3) total += i; assert(total == 3);
```
{% endraw %}

注意点: 展開先スコープで i を使います。上限式は反復ごとに再評価されます。ll に収まる値を指定し、副作用のある式を避けてください。

</details>

<details class="api-operation" id="rep2" markdown="1">
<summary><code>rep(i,n)</code> — O(max(0,n))</summary>

指定した名前の ll 変数で [0,n) を回します。

{% raw %}
```cpp
ll total = 0; rep(j,3) total += j; assert(total == 3);
```
{% endraw %}

注意点: 上限を反復ごとに評価します。上限は ll の範囲です。同じ名前を外側で使うと shadow します。

</details>

<details class="api-operation" id="rep3" markdown="1">
<summary><code>rep(i,a,b)</code> — O(max(0,b-a))</summary>

ll i=a から b 未満まで 1 ずつ増加します。

{% raw %}
```cpp
ll total = 0; rep(j,2,5) total += j; assert(total == 9);
```
{% endraw %}

注意点: a>=b なら空です。端点は ll の範囲に収まる必要があります。b は反復ごとに評価します。

</details>

<details class="api-operation" id="rep4" markdown="1">
<summary><code>rep(i,a,b,c)</code> — O(1 + max(0,(b-a)/c))</summary>

ll i=a から b 未満まで c ずつ増加します。

{% raw %}
```cpp
ll total = 0; rep(j,0,6,2) total += j; assert(total == 6);
```
{% endraw %}

注意点: c>0 が必要です。降順ループは対象外です。最後の増分も ll に収まる必要があります。b/c の式は反復ごとに評価するので副作用を避けてください。

</details>

<details class="api-operation" id="all" markdown="1">
<summary><code>ALL(x)</code> — O(1)</summary>

std::begin(x), std::end(x) に展開します。

{% raw %}
```cpp
std::vector<int> a{2,1}; std::sort(ALL(a));
```
{% endraw %}

注意点: 同じ式を 2 回評価します。副作用のある式や一時コンテナは禁止です。元コンテナの iterator の寿命・無効化規則に従います。

</details>

<details class="api-operation" id="rall" markdown="1">
<summary><code>rALL(x)</code> — O(1)</summary>

std::rbegin(x), std::rend(x) に展開します。

{% raw %}
```cpp
std::vector<int> a{1,2}; std::sort(rALL(a));
```
{% endraw %}

注意点: ALL と同じ評価・寿命条件です。逆順 iterator が必要です。

</details>

<details class="api-operation" id="ll" markdown="1">
<summary><code>LL(names...)</code> — O(S)</summary>

ll 変数を宣言し scan で読みます。

{% raw %}
```cpp
LL(n, m); // ll n, m; scan(n, m);
```
{% endraw %}

注意点: 波括弧のある文スコープで使います。制御文の単独の body に置かないでください。初期値がないので入力成功を前提にします。

</details>

<details class="api-operation" id="str" markdown="1">
<summary><code>STR(names...)</code> — O(S)</summary>

str 変数を宣言し scan で読みます。

{% raw %}
```cpp
STR(s, t);
```
{% endraw %}

注意点: LL と同じスコープ条件です。string は空で構築されます。空白区切りの入力です。

</details>

<details class="api-operation" id="yesno" markdown="1">
<summary><code>YesNo(condition)</code> — O(1)</summary>

Yes または No と改行を出力します。

{% raw %}
```cpp
YesNo(true);
```
{% endraw %}

注意点: 条件式を 1 回評価し、flush しません。返り値は ostream& です。

</details>

<details class="api-operation" id="yesno-upper" markdown="1">
<summary><code>YESNO(condition)</code> — O(1)</summary>

YES または NO と改行を出力します。

{% raw %}
```cpp
YESNO(false);
```
{% endraw %}

注意点: YesNo と同じ規則です。

</details>

<details class="api-operation" id="bit" markdown="1">
<summary><code>bit(x,i)</code> — O(1)</summary>

x を i bit 右へシフトした最下位 bit を返します。

{% raw %}
```cpp
assert(bit(8ULL,3) == 1);
```
{% endraw %}

注意点: 0 <= i < x の型の bit 幅が必要です。unsigned を推奨します。負の signed 右シフトは C++20 の算術シフトです。

</details>

<details class="api-operation" id="popcount" markdown="1">
<summary><code>popc(x)</code> — O(1)</summary>

x を ull に変換し、64 bit の立っている bit 数を返します。

{% raw %}
```cpp
assert(popc(1ULL << 40) == 1);
```
{% endraw %}

注意点: 元の __builtin_popcount の 32 bit 打切りを修正しました。負値は unsigned への変換後の 64 bit 表現を数えます。

</details>

<details class="api-operation" id="in-range" markdown="1">
<summary><code>crmp(x,l,r)</code> — O(1)</summary>

l<=x && x<=r を返します。閉区間です。

{% raw %}
```cpp
assert(crmp(3,1,5));
```
{% endraw %}

注意点: x は最大 2 回評価されます。副作用のある式は禁止です。型の混在時は通常の C++ 比較変換に従います。l>r は false です。

</details>

<details class="api-operation" id="fi" markdown="1">
<summary><code>object.fi</code> — O(1)</summary>

first に展開します。

{% raw %}
```cpp
std::pair<int,int> p{1,2}; assert(p.fi == 1);
```
{% endraw %}

注意点: 元の first の型・参照寿命に従います。

</details>

<details class="api-operation" id="se" markdown="1">
<summary><code>object.se</code> — O(1)</summary>

second に展開します。

{% raw %}
```cpp
std::pair<int,int> p{1,2}; assert(p.se == 2);
```
{% endraw %}

注意点: 元の second の型・参照寿命に従います。

</details>

<details class="api-operation" id="pb" markdown="1">
<summary><code>container.pb(value)</code> — O(1) macro expansion</summary>

push_back に展開します。

{% raw %}
```cpp
std::vector<int> a; a.pb(3); assert(a[0] == 3);
```
{% endraw %}

注意点: 展開先の計算量・invalidation はコンテナによります。vector では償却 O(1)、再確保で参照が無効になります。

</details>

<details class="api-operation" id="eb" markdown="1">
<summary><code>container.eb(args...)</code> — O(1) macro expansion</summary>

emplace_back に展開します。

{% raw %}
```cpp
std::vector<std::pair<int,int>> a; a.eb(1,2); assert(a[0].first == 1);
```
{% endraw %}

注意点: 展開先の計算量・invalidation はコンテナによります。vector では償却 O(1)、再確保で参照が無効になります。

</details>

<details class="api-operation" id="endl" markdown="1">
<summary><code>Endl</code> — O(1) macro expansion</summary>

std::endl に展開します。

{% raw %}
```cpp
std::ostringstream out; out << 3 << Endl; assert(out.str() == "3\n");
```
{% endraw %}

注意点: 改行に加えて flush します。実際の時間は stream/device に依存します。

</details>

<details class="api-operation" id="space" markdown="1">
<summary><code>spa</code> — O(1)</summary>

空白文字列 " " に展開します。

{% raw %}
```cpp
std::ostringstream out; out << 1 << spa << 2; assert(out.str() == "1 2");
```
{% endraw %}

注意点: 文字列 literal の静的寿命です。

</details>

<details class="api-operation" id="o" markdown="1">
<summary><code>O(args...)</code> — O(S)</summary>

print の短縮 macro です。

{% raw %}
```cpp
O(1, "hello");
```
{% endraw %}

注意点: print と同じ空白・改行・型・失敗規則です。S は出力 byte 数です。

</details>

<details class="api-operation" id="divide" markdown="1">
<summary><code>ll divide(ll a, ll b)</code> — O(1)</summary>

floor_div の旧名です。

{% raw %}
```cpp
assert(divide(-7,3) == -3);
```
{% endraw %}

注意点: floor_div と同じ範囲条件です。0 除算と LLONG_MIN/-1 は禁止です。

</details>

<details class="api-operation" id="palindrome" markdown="1">
<summary><code>bool iskaibun(std::string_view s)</code> — O(N)</summary>

is_palindrome の旧名です。

{% raw %}
```cpp
assert(iskaibun("abba"));
```
{% endraw %}

注意点: 空・Unicode・寿命条件は is_palindrome と同じです。

</details>

<details class="api-operation" id="range-sum" markdown="1">
<summary><code>__int128 ran(ll a, ll b)</code> — O(1)</summary>

range_sum の旧名です。

{% raw %}
```cpp
assert(ran(1,4) == 10);
```
{% endraw %}

注意点: 旧 ll 返り値から __int128 に拡張しました。a>b は 0、全 ll 端点に対応します。

</details>

<details class="api-operation" id="compress" markdown="1">
<summary><code>int zaatu(std::vector&lt;ll&gt;&amp; a)</code> — O(N log N)</summary>

compress の旧名です。

{% raw %}
```cpp
std::vector<ll> a{8,2,8}; assert(zaatu(a) == 2);
```
{% endraw %}

注意点: compress と同じ破壊的な値置換・範囲・空入力・O(N) 作業メモリです。

</details>

<details class="api-operation" id="bipartite" markdown="1">
<summary><code>bool isbipartite_graph(const std::vector&lt;std::vector&lt;Index&gt;&gt;&amp; g)</code> — O(V+E)</summary>

is_bipartite の旧名です。

{% raw %}
```cpp
std::vector<std::vector<ll>> g{{1},{0}}; assert(isbipartite_graph(g));
```
{% endraw %}

注意点: 無向隣接リスト・頂点範囲・O(V) メモリ条件は is_bipartite と同じです。

</details>

<details class="api-operation" id="rande-default" markdown="1">
<summary><code>Rande()</code> — O(1)</summary>

steady_clock の時刻から 32 bit seed を作り mt19937 を構築します。

{% raw %}
```cpp
Rande rng; int x = rng(10); assert(0 <= x && x < 10);
```
{% endraw %}

注意点: 暗号用ではありません。再現性が必要なテストでは seed 指定を使ってください。状態メモリは O(1)（mt19937 1 個）です。

</details>

<details class="api-operation" id="rande-seed" markdown="1">
<summary><code>Rande rng(std::uint32_t seed)</code> — O(1)</summary>

固定 seed で mt19937 を構築します。

{% raw %}
```cpp
Rande a(7), b(7); assert(a(100) == b(100));
```
{% endraw %}

注意点: 同一標準ライブラリ・同一呼出し順では再現します。uniform_int_distribution の結果列は異なる標準ライブラリ間で一致保証がありません。

</details>

<details class="api-operation" id="rande-range" markdown="1">
<summary><code>int rng(int a, int b)</code> — O(1) expected</summary>

半開区間 [a,b) から一様乱数を返します。

{% raw %}
```cpp
Rande rng(7); int x = rng(-3,4); assert(-3 <= x && x < 4);
```
{% endraw %}

注意点: a<b が必要です。rejection sampling のため期待 O(1) であり最悪回数は保証しません。INT_MIN/INT_MAX 端点にも対応します。

</details>

<details class="api-operation" id="rande-bound" markdown="1">
<summary><code>int rng(int b)</code> — O(1) expected</summary>

rng(0,b) の省略形です。

{% raw %}
```cpp
Rande rng(7); assert(rng(1) == 0);
```
{% endraw %}

注意点: b>0 が必要です。範囲オーバーロードと同じ期待時間・再現性です。

</details>

<details class="api-operation" id="aliases" markdown="1">
<summary><code>ll / ld / ull / uint / str / mint / mint1 / pqg&lt;T&gt;</code> — O(1)</summary>

long long / long double / unsigned long long / unsigned int / std::string / ACL modint998244353 / ACL modint1000000007 / min heap の型別名です。

{% raw %}
```cpp
mint x = 2; pqg<ll> q; q.push(3); q.push(1); assert(q.top() == 1);
```
{% endraw %}

注意点: 型別名の宣言自体に実行コストはありません。各型の構築・操作・寿命・overflow は元の型に従います。using namespace std/atcoder は自動で行いません。

</details>

<details class="api-operation" id="inf" markdown="1">
<summary><code>constexpr ll INF</code> — O(1)</summary>

(1LL<<62)-(1LL<<31) の定数です。

{% raw %}
```cpp
assert(INF == 4611686016279904256LL);
```
{% endraw %}

注意点: 全型共通の無限大ではありません。INF を含む加算・乗算の overflow は利用者の責任です。inline constexpr で書換え不可です。

</details>

<details class="api-operation" id="cps" markdown="1">
<summary><code>constexpr ld CPS</code> — O(1)</summary>

CLOCKS_PER_SEC の long double 値です。

{% raw %}
```cpp
assert(CPS > 0);
```
{% endraw %}

注意点: CPU clock の単位であり、壁時計の秒数ではありません。inline constexpr で書換え不可です。

</details>

<details class="api-operation" id="dx" markdown="1">
<summary><code>constexpr std::array&lt;ll,8&gt; dx</code> — O(1)</summary>

8 近傍の x 差分 {0,1,0,-1,-1,-1,1,1} です。

{% raw %}
```cpp
assert(dx[0] == 0 && dx[1] == 1);
```
{% endraw %}

注意点: 添字は [0,8)。dy と同じ添字で使い、先頭 4 個が 4 近傍です。静的寿命で書換え不可です。

</details>

<details class="api-operation" id="dy" markdown="1">
<summary><code>constexpr std::array&lt;ll,8&gt; dy</code> — O(1)</summary>

8 近傍の y 差分 {1,0,-1,0,-1,1,-1,1} です。

{% raw %}
```cpp
assert(dy[0] == 1 && dy[1] == 0);
```
{% endraw %}

注意点: 添字は [0,8)。dx と同じ添字で使います。静的寿命で書換え不可です。

</details>

<details class="api-operation" id="imports" markdown="1">
<summary><code>bin_search / chmin / chmax / Fill / len / sum / min_of / max_of / my_distance / parse128 / to_string128</code> — O(各関数の計算量)</summary>

Contest Helpers の同名関数を global using 宣言で利用できます。

{% raw %}
```cpp
int x = 5; chmin(x, 3); assert(x == 3);
assert(parse128("-4").value() == -4);
```
{% endraw %}

注意点: 全オーバーロード・範囲・空入力・メモリ条件は Contest Helpers の操作一覧を参照してください。元 min/max の可変長 overload は std::min/std::max と衝突するため追加していません。

</details>

<details class="api-operation" id="rande-copy" markdown="1">
<summary><code>Rande(const Rande&amp; other)</code> — O(1)</summary>

乱数生成器の現在の状態を独立にコピーします。

{% raw %}
```cpp
Rande rng(7);
Rande copy(rng);
assert(rng(100) == copy(100));
```
{% endraw %}

注意点: コピー直後は同じ問い合わせ列から同じ乱数列を得ます。固定長の mt19937 状態をコピーするため、入力範囲の大きさには依存しません。

</details>

<details class="api-operation" id="rande-copy-assign" markdown="1">
<summary><code>Rande&amp; operator=(const Rande&amp; other)</code> — O(1)</summary>

乱数生成器の状態をコピーし、自身への参照を返します。

{% raw %}
```cpp
Rande rng(7), copy(9);
copy = rng;
assert(rng(100) == copy(100));
```
{% endraw %}

注意点: 自身への代入も可能です。以後の生成は各オブジェクトで独立です。固定長の状態以外に追加allocationはありません。

</details>

<details class="api-operation" id="rande-move" markdown="1">
<summary><code>Rande(Rande&amp;&amp; other)</code> — O(1)</summary>

乱数生成器の状態を移動構築します。

{% raw %}
```cpp
Rande rng(7);
Rande moved(std::move(rng));
```
{% endraw %}

注意点: 移動元の乱数列を前提にしないでください。移動元は破棄・代入・再構築して使います。固定長の状態を扱います。

</details>

<details class="api-operation" id="rande-move-assign" markdown="1">
<summary><code>Rande&amp; operator=(Rande&amp;&amp; other)</code> — O(1)</summary>

乱数生成器の状態を移動代入し、自身への参照を返します。

{% raw %}
```cpp
Rande rng(7), target(9);
target = std::move(rng);
```
{% endraw %}

注意点: 移動元の利用条件は移動構築と同じです。自己移動後の列は前提にせず、seed指定で作った Rande を代入してから使ってください。

</details>

<details class="api-operation" id="rande-destruct" markdown="1">
<summary><code>~Rande()</code> — O(1)</summary>

固定長の乱数生成器を破棄します。

{% raw %}
```cpp
{ Rande rng(7); } // スコープ終了時に破棄
```
{% endraw %}

注意点: 移動元も破棄できます。内部の mt19937 や参照・iteratorは公開しません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- 元のユーザーテンプレートから用途・短縮名を整理したものです。`templates/contest.cpp.example`を提出用mainの出発点にできます。
- streamの入れ子・128bit最小値・loop macro・乱数境界をテストします。
- [Many A + B (128 bit)](https://judge.yosupo.jp/problem/many_aplusb_128bit)の公式verifyは`init_io/scan/print`とsigned128のstream演算を使います。
- 2 translation unitのlinkは、ACLを含まないヘッダについて確認します。
- 既存APIとの対応・廃止した名前は[移行記録]({{ '/docs/development/contest-template-import.html' | relative_url }})を参照してください。

</details>
