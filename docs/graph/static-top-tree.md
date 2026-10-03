---
title: Static Top Tree
documentation_of: //blueberry/graph/static-top-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

**辺と根を固定した木で、頂点の値を変更するたびに、根の木DPを求め直したい**ときに使います。
全頂点をたどり直す代わりに、DPの途中結果を保存し、変更の影響がある O(log(N+1)) 個の結合だけを再計算します。
頂点番号は0-indexedです。`N`は頂点数で、空木には対応しません。
1頂点だけの木には対応します。その場合、根自身の値から集約を作ります。

例えば`dp[v] = a[v] * sum(dp[子]) + b[v]`のように、子の答えをまとめ、親へ渡すDPを扱えます。
ただしDP式を渡すだけでは使えません。縦につなぐ処理と、兄弟をまとめる処理を、
利用者が3つの型と5つの関数で定義します。このページでは総和の小さい例からその作り方を説明します。

ACLには、この木DPの結合を直接扱う機能がありません。
2頂点間のパスや特定の部分木の区間集約が目的なら、
[HLD]({{ '/blueberry/graph/heavy-light-decomposition.hpp.html' | relative_url }}) とACLの区間データ構造を検討してください。
このクラスが返すのは、構築時の根に対する木全体の集約です。
パス照会、部分木照会、根変更、辺の追加・削除のAPIはありません。
辺の変更も必要な木DPには [Dynamic Top Tree]({{ '/blueberry/graph/dynamic-top-tree.hpp.html' | relative_url }}) があります。

### 計算量と入力条件

| 項目 | 保証・条件 |
| --- | --- |
| 構築 | 最悪 O(N log(N+1)) 以下 |
| 頂点値の代入 `set` | 最悪 O(log(N+1)) |
| `size` / `get` / `all_prod` | O(1) |
| 格納量・構築時の作業用メモリ | O(N) |
| 頂点数 | `1 <= N <= INT_MAX/4` |
| 入力 | `tree.size() == values.size()`、`0 <= root < N` の無向連結木 |

各辺は両端の隣接リストに1回ずつ入れ、自己辺・重辺・閉路を含めないでください。
隣接頂点も`[0,N)`の範囲です。前提を外れた入力への例外やエラー値は提供しません。

算術オーバーフローは利用者が防ぎ、必要ならmodintなどを使ってください。
上の計算量は、利用者が定義する5つの関数と、値の構築・コピー・代入・破棄がすべてO(1)の場合です。

## 最小使用例

まず、`dp[v] = value[v] + sum(dp[子])`、つまり全頂点の総和を保持します。
`set(v,x)`は加算ではなく、値をxに置き換える操作です。
総和だけなら差分を直接足し引きする方法が簡単ですが、ここでは5つの関数とAPIの使い方を確認します。

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/graph/static-top-tree.hpp"

using Value = long long;  // 1頂点の入力値。
using Path = long long;   // 縦につないだ頂点と、その横の枝の総和。
using Point = long long;  // 完成した子部分木の寄与の和。

Path add_vertex(Value value, Point light) { return value + light; }
Point add_edge(Path path) { return path; }
Path compress(Path top, Path bottom) { return top + bottom; }
Point rake(Point left, Point right) { return left + right; }
Point point_identity() { return 0; }

using Tree = blueberry::StaticTopTree<
    Value, Path, Point, add_vertex, add_edge, compress, rake, point_identity>;

int main() {
  // 根0、辺0-1, 0-2, 1-3。
  const std::vector<std::vector<int>> graph{{1, 2}, {0, 3}, {0}, {1}};
  const std::vector<Value> values{10, 20, 30, 40};
  Tree t(graph, values, 0);
  assert(t.size() == 4);
  assert(t.get(1) == 20);
  assert(t.all_prod() == 100);

  t.set(1, 7);
  assert(t.get(1) == 7);
  assert(t.all_prod() == 87);  // 10 + 7 + 30 + 40。
  assert(values[1] == 20);    // 元の入力配列は変更されない。
}
```
{% endraw %}

### 3つの型は何を表すか

総和ではどれも `long long` でしたが、役割は異なります。

| 型 | 保存するもの | 総和の例 | 後述のaffine DPの例 |
| --- | --- | --- | --- |
| `V` | 利用者が `set` で変更する1頂点分の入力 | 頂点の値 | 係数 `(a[v],b[v])` |
| `Path` | 下側との接続を残した、縦方向の集約 | そこまでの和 | 下側の答え `x` を受け取る関数 `A*x+B` |
| `Point` | 完成した子部分木から親へ渡す寄与、またはその寄与の集約 | 子部分木の和 | 完成したDP値、または複数の子のDP値の和 |

ここでいう「cluster」は、内部でまとめて保持する木の一部分です。
`Path`は下に続く部分をまだ受け取れる開いた集約、`Point`は下側を閉じて親へ渡せる完成した寄与です。
この違いを型の役割として決めると、5つの関数を書きやすくなります。

### 5つの関数を木に対応させる

内部では、各頂点の子のうち、部分木が最大の子をheavy childとし、それ以外をlight childとします。
heavy childへ進む縦の列をheavy pathと呼びます。次の木では `0 → 1 → 3` がその列です。

```text
       0        上側（根に近い）
      / \
     1   2      2の部分木を閉じ、0の横から合流させる
     |
     3          下側（子孫に近い）
```

| 関数の形 | 担当する処理 |
| --- | --- |
| `Point point_identity()` | light childがいないときの寄与を作る |
| `Point rake(Point a, Point b)` | 同じ親へ渡す、2つの子部分木の寄与をまとめる |
| `Path add_vertex(V value, Point light)` | light child全体の寄与を頂点値と合わせ、1頂点分の開いた集約を作る |
| `Path compress(Path top, Path bottom)` | 親に近い集約 `top` と、子孫に近い集約 `bottom` を縦につなぐ |
| `Point add_edge(Path path)` | 完成したheavy pathと横の枝を閉じ、親へ渡す寄与に変換する |

この木の総和例では、頂点2の `Path` は30です。`add_edge(30)` で親へ渡す `Point` にし、
`add_vertex(10,30)` で頂点0の `Path` は40になります。
残りの頂点1と3も縦につなぎ、`compress(40,compress(20,40)) = 100` です。
結合の括弧の位置は内部で変わりますが、**縦の上から下への順序は保たれます**。

`add_edge` は名前にedgeがあっても、辺を追加するAPIでも、辺の値を受け取る関数でもありません。
内部では完成したlight subtreeを `Path` から `Point` へ変換するときに呼び、元の木の各辺に1回ずつ呼ぶわけではありません。
辺の係数が必要なら、固定根から見た**子側の `V` に親との辺の情報を含める**などして、
heavy/lightのどちらでも同じ計算になるように設計します。親との辺がない根には、値を変えない係数を持たせます。
各頂点の入力は結合された木の中で一度だけ数え、境界頂点を両方の集約へ重複して足しません。

### 結合の規則と型の条件

以下の規則は型チェックだけでは検査できません。

- `compress` は結合的であること。`compress(compress(a,b),c)` と `compress(a,compress(b,c))` が同じ意味になる必要があります。可換である必要はありません。
- `rake` は結合的かつ可換で、`point_identity()` が左右の単位元であること。兄弟の列挙順に依存する集約には使えません。
- heavy childをどれに選んでも、同じ木のDP値になること。縦の結合と、横の寄与への変換を整合させてください。

最後の規則は、例えば完成した子部分木 `P`、他の子の寄与 `L`、頂点値 `v` に対し、
次の2通りが同じ寄与を表すことを意味します。

```text
子Pを縦につなぐ: add_edge(compress(add_vertex(v, L), P))
子Pを横でまとめる: add_edge(add_vertex(v, rake(L, add_edge(P))))
```

葉と非葉で式が異なるDPなら、子の有無などを `V` に含めます。
`point_identity()` という値だけから、子が0個なのか、子の集約がたまたま単位元なのかは区別できません。
`compress` 用の単位元や逆元を渡す必要はありません。空の `Path` 同士を結合するAPIでもありません。

`V`はコピー構築・コピー代入可能、`Path`と`Point`はそれに加えてデフォルト構築可能にしてください。
`V`のデフォルト構築は必要ありません。
内部で使う`Path{}` / `Point{}`自体が数学的な単位元である必要はなく、空の子の寄与は必ず`point_identity()`で指定します。

利用者が定義する5つの関数を、以降コールバックと呼びます。コールバックはテンプレート引数として渡します。
表は値渡しで表記していますが、適切なconst参照で受け取る関数も使えます。

入力の`tree`から内部構造を作り、`values`はコピーして保持します。構築後は元の配列の寿命に依存しません。
ただし`V`など自身がポインタや参照を持つ場合、その参照先まで深くコピーするわけではありません。

コールバックは同じ引数に対して同じ意味の集約値を返すようにしてください。外部の可変係数を読むと、
再計算されなかった場所に古い集約が残ります。呼び出し回数を数えるなど、返り値を変えない副作用は可能ですが、
特定の呼び出し順や回数には依存しないでください。

同じインスタンスの更新中にコールバックからそのインスタンスを再更新すると、途中の状態に割り込むため対応しません。
別インスタンスを扱う場合も、返り値に影響する共有状態を持ち込まない設計にしてください。

コールバックの例外発生後に集約の整合性を保つ保証はありません。

## 応用例: 子の答えをaffine関数でまとめる

次のDPを、頂点ごとの係数更新に対応させます。子がいないとき、和は0です。

```text
dp[v] = a[v] * sum(dp[子]) + b[v]
```

### 1. 下に続く子の答えを変数にする

light childのDP値の和を `L`、heavy childのDP値を未確定の `x` とすると、
`dp[v] = a[v] * (L+x) + b[v]` です。これは
`a[v]*x + (a[v]*L+b[v])` というaffine関数なので、`Path` に係数 `(A,B)` を保存します。
`Point` はすでに確定したDP値、兄弟の集約はその和です。

### 2. 上の関数へ、下の関数の答えを渡す

上側が `f(x)=A*x+B`、下側が `g(x)=C*x+D` なら、縦に結合した関数は
**`f(g(x))`** です。したがって `compress(top,bottom)` は `(A*C,A*D+B)` を返します。
引数の順は上→下ですが、関数を評価する順は下→上です。
例えば `f(x)=2*x+1`、`g(x)=3*x+4` なら `f(g(x))=6*x+9` で、逆順の `6*x+7` とは異なります。

### 3. 下側を閉じるときは0を入れる

完成したheavy pathの下にはもう子がいないので、`A*0+B=B` を親へ渡します。
そのため `add_edge(path)` は `path.b` です。木全体の `all_prod()` も `Path` を返すので、
根のDP値を取り出すときは `.b` を読みます。ライブラリが最後の変換を自動で呼ぶわけではありません。

`all_prod().a` は残ったheavy pathの下側へ仮に値をつないだときの係数で、全頂点の `a[v]` の積ではありません。
例えば根 `(2,1)`、2つの葉 `(3,4)` と `(5,6)` の木なら、選ばれるheavy childにより `Path` は
`(6,21)` または `(10,21)` になります。**閉じた答えである `.b=21` が同じ**ことが必要で、
内部の `Path` の全フィールドが分解方法によらず一致する必要はありません。

### 完全なコード

次の例は最小使用例とは独立してコンパイルできます。小さい整数に限定した例なので `long long` を使っています。
大きな木や係数では、乗算のたびに値が増えるため、問題に合うmodintなどへ置き換えてください。

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/static-top-tree.hpp"

struct Vertex { long long a, b; };  // 1頂点の入力係数。
struct Path { long long a, b; };    // 未確定の下側xを受け取る a*x+b。
using Point = long long;

Path add_vertex(Vertex value, Point light) {
  return {value.a, value.a * light + value.b};
}
Point add_edge(Path path) { return path.b; }  // 下側へ0を代入。
Path compress(Path top, Path bottom) {
  return {top.a * bottom.a, top.a * bottom.b + top.b};
}
Point rake(Point left, Point right) { return left + right; }
Point point_identity() { return 0; }

using Tree = blueberry::StaticTopTree<
    Vertex, Path, Point, add_vertex, add_edge, compress, rake, point_identity>;

int main() {
  const std::vector<std::vector<int>> graph{{1, 2}, {0, 3}, {0}, {1}};
  const std::vector<Vertex> values{{2, 1}, {3, 4}, {5, 6}, {7, 8}};
  Tree t(graph, values, 0);
  // dp[3]=8, dp[1]=3*8+4=28, dp[2]=6。
  assert(t.all_prod().b == 69);  // dp[0]=2*(28+6)+1。

  auto value = t.get(3);        // getは入力係数をコピーして返す。
  value.b = 10;
  assert(t.all_prod().b == 69); // コピーの変更だけでは更新されない。
  t.set(3, value);
  assert(t.all_prod().b == 81); // dp[1]=34, dp[0]=2*(34+6)+1。

  // 根を変えるには、別の構造を構築する。元のvaluesは変わっていない。
  Tree root_at_two(graph, values, 2);
  assert(root_at_two.all_prod().b == 291); // 5*(2*(3*8+4)+1)+6。

  const Path top{2, 1}, bottom{3, 4};
  assert(compress(top, bottom).b == 9);
  assert(compress(bottom, top).b == 7);   // 順序を交換できない。
}
```
{% endraw %}

### つまずきやすい点

| 症状・思い違い | 確認すること |
| --- | --- |
| `all_prod()` をそのままDP値として使ってしまう | 返り値は `Path`。総和ならその値、affine例なら `.b` が答えです。 |
| `get(v)` で部分木DPを取ろうとする | `get` は現在の入力 `V` のコピーです。部分木の照会APIはありません。 |
| 鎖のDPだけ答えが逆になる | `compress(top,bottom)` は上側へ下側を代入します。affine合成は `top(bottom(x))` です。 |
| 子の順番や隣接リスト順で答えが変わる | `rake` の可換性、heavy/lightを変えたときの整合性を確認してください。 |
| `Point{}` を空の子の寄与だと思う | 必要な値を `point_identity()` で返します。積の集約なら通常は1です。 |
| 元の `values[v]` を書き換えたのに答えが変わらない | 木は値のコピーを持ちます。更新には `set(v,new_value)` を呼びます。 |
| 辺の係数を `add_edge` だけで適用する | `add_edge` は全ての元の辺に呼ばれません。固定根に対応した子側の `V` へ係数を含めます。 |

内部では縦の `compress` と横の `rake` を二分式木にし、部分木サイズを重みとして分割します。
元の木が長い鎖でも、値の変更から根の集約までたどる**式木の高さ**は O(log(N+1)) に抑えられます。
これは元の木の高さが小さいという意味ではありません。

## 操作一覧

以下の短い操作例は、最小使用例の総和用 `Tree`、`t`、`graph`、`values` がある場面を想定しています。
`N` は対象またはコピー元の頂点数、`D` は代入先が以前保持していた頂点数です。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `StaticTopTree(const vector<vector<int>>& tree, const vector<V>& values, int root = 0)` | O(N log(N+1)) | [開く](#construct) |
| `int size() const` | O(1) | [開く](#size) |
| `V get(int v) const` | O(1) | [開く](#get) |
| `void set(int v, const V& x)` | O(log(N+1)) | [開く](#set) |
| `Path all_prod() const` | O(1) | [開く](#all-prod) |
| `StaticTopTree(const StaticTopTree& other)` | O(N) | [開く](#copy) |
| `StaticTopTree& operator=(const StaticTopTree& other)` | O(N+D) | [開く](#copy-assign) |
| `StaticTopTree(StaticTopTree&& other)` | O(1) | [開く](#move) |
| `StaticTopTree& operator=(StaticTopTree&& other)` | O(D) | [開く](#move-assign) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>StaticTopTree(const vector&lt;vector&lt;int&gt;&gt;&amp; tree, const vector&lt;V&gt;&amp; values, int root = 0)</code> — O(N log(N+1))</summary>

無向連結木と各頂点の入力値から、固定根の集約構造を作ります。省略時の根は頂点0です。

{% raw %}
```cpp
Tree from_zero(graph, values);    // 根0。
Tree from_two(graph, values, 2);  // 根2の別インスタンス。
```
{% endraw %}

注意点: `1 <= N <= INT_MAX/4`、`tree.size()==values.size()`、`0<=root<N` が必要です。
頂点・辺は概要の条件を満たしてください。構築後に元の隣接リストや値を変更しても、この木は変更されません。
入力への参照を保持しないため、それらの配列が先に破棄されても構いません。
根は以降固定され、根に依存する辺情報を `V` に含める場合は、選んだ根に合わせて用意します。
引数なしのデフォルト構築はできません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

構築時の頂点数を返します。

{% raw %}
```cpp
assert(t.size() == 4);
```
{% endraw %}

注意点: 内部の式木のノード数ではありません。`set` で値を変更しても頂点数は変わりません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>V get(int v) const</code> — O(1)</summary>

頂点 `v` の現在の入力値をコピーして返します。

{% raw %}
```cpp
auto value = t.get(1);
value += 5;
t.set(1, value);  // コピーの変更を反映したいときはsetを呼ぶ。
```
{% endraw %}

注意点: `0 <= v < N`。DPの集約値ではなく `V` です。返り値は内部要素への参照ではありません。
型 `V` 自身が参照先を共有する場合の扱いは、その型のコピー規則に従います。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void set(int v, const V&amp; x)</code> — O(log(N+1))</summary>

頂点 `v` の入力を `x` に代入し、影響する集約を再計算します。直後の `all_prod()` には更新が反映されています。

{% raw %}
```cpp
t.set(1, 7);
assert(t.get(1) == 7);
```
{% endraw %}

注意点: `0 <= v < N`。加算や部分更新ではありません。構造体の1フィールドだけを変えたい場合も、
`get` で得た値を変更し、必要な全フィールドを含む `V` を渡してください。
同じ値を代入した場合も再計算します。入力 `x` への参照は保持しません。
外部の状態を変更して、この操作を呼んでいない別頂点の意味まで変える設計は避けてください。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>Path all_prod() const</code> — O(1)</summary>

構築時の根から始まる、木全体の `Path` をコピーして返します。引数で根を指定することはできません。

{% raw %}
```cpp
auto sum = t.all_prod();  // 総和例ではPath自体がlong long。
```
{% endraw %}

注意点: 保存済みの集約を返し、この操作ではコールバックによる再計算をしません。
最終的な答えの取り出し方は `Path` の設計によります。affine例なら `t.all_prod().b` です。
返り値の変更で内部状態は変わらず、値型の返り値は後続の `set` 後も独立して保持できます。
ただし `Path` 自身が参照を保持する型なら、その参照先の寿命・共有規則に従います。

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>StaticTopTree(const StaticTopTree&amp; other)</code> — O(N)</summary>

根・内部構造・現在値・保存済み集約をコピーします。DPの構築用コールバックは呼び直しません。

{% raw %}
```cpp
auto copy = t;
copy.set(0, 100);  // tの頂点値は変わらない。
```
{% endraw %}

注意点: コピー後の更新は独立です。ただし `V` / `Path` / `Point` が持つ参照先まで複製するわけではありません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>StaticTopTree&amp; operator=(const StaticTopTree&amp; other)</code> — O(N+D)</summary>

コピー元の根・構造・値・集約で置き換え、自身への参照を返します。

{% raw %}
```cpp
Tree copy(graph, values);
copy = t;
```
{% endraw %}

注意点: `N` はコピー元、`D` はコピー先の旧頂点数です。自己代入できます。
コピー元と同じテンプレート引数の型に代入します。型が同じなら、頂点数や根が違っていても構いません。

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>StaticTopTree(StaticTopTree&amp;&amp; other)</code> — O(1)</summary>

内部配列の所有権を移し、移動先に元の根・値・集約を引き継ぎます。

{% raw %}
```cpp
// <utility>をincludeする。
auto copy = t;
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元には破棄・再代入だけを行ってください。空の有効な木になったと仮定して照会・更新してはいけません。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>StaticTopTree&amp; operator=(StaticTopTree&amp;&amp; other)</code> — O(D)</summary>

移動先の旧データを破棄し、移動元の内部配列を引き継いで、自身への参照を返します。

{% raw %}
```cpp
Tree copy(graph, values);
auto moved = t;
copy = std::move(moved);
```
{% endraw %}

注意点: `D` は移動先の旧頂点数です。移動元には破棄・再代入だけを行ってください。

</details>

## 出典・検証

[ABC351 G 公式解説](https://atcoder.jp/contests/abc351/editorial/9899) は、
固定木のDPをrake/compressでまとめる考え方の参照先です。
[Nyaan Static Top Tree](https://nyaannyaan.github.io/library/tree/static-top-tree-vertex-based.hpp.html)
とは、部分木サイズによる重み付き平衡化とcluster APIを比較しています。

[Library Checker: Point Set Tree Path Composite Sum (Fixed Root)](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root)
の [verifyコード](https://github.com/blueberry1001/Blueberry-library/blob/main/verify/graph/static-top-tree.test.cpp)
では、頂点値と辺のaffine係数の更新を扱います。辺の係数を子側の入力に持たせ、
固定された部分木の頂点数も `V` に含める、より実践的な設計例です。
この問題では、辺の加算項を部分木内の各頂点の値に一度ずつ加えるため、加算項に部分木の頂点数を掛けます。
根を変えてこの設計を使い直す場合は、親との辺と部分木の頂点数も計算し直してください。

[ローカルテスト](https://github.com/blueberry1001/Blueberry-library/blob/main/tests/random/static-top-tree.cpp)
では、このページのaffine DPをmodintで全頂点から再計算した答えと比較します。
根と頂点番号を変えた木、構築直後の答え、入力配列の破棄後の操作、コピー後の独立した更新も確認します。
また、10万頂点の鎖・星・二分木を構築し、更新後の答えと更新時のコールバック呼び出し数を検査します。
