---
title: Matrix
documentation_of: //blueberry/math/matrix.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }})

## 概要・前提

体上の行列式・逆行列・階数・連立一次方程式を扱います。ACL modint をそのまま係数型に使い、ACL にない行列消去のみ提供します。行列積・累乗・浮動小数点の数値安定化は対象外です。`Matrix<T>` は行数と列数、行優先の連続配列を所有します。`a(i,j)` で短く要素を参照できます。

T は**正確な体演算**を持つ型で、`T{}`=0、`T(1)`=1、コピー・代入・等値/不等値・単項負号・乗算・乗算代入・減算代入・除算が必要です。非零要素はすべて可逆であること。素数法 ACL `static_modint`、法を途中で変更しない素数法 `dynamic_modint` が使えます。通常の int/long long（割算の切捨て）、合成数法 modint、epsilon 判定が必要な浮動小数点には使わないでください。標数2を含む体にも対応します。

n行m列、r=rank(A)、d=m-r。演算・コピーを O(1) とみなした最悪計算量を示します。行列自身のメモリは O(nm+1)。消去は作業用コピーだけを変更し、利用者の行列は変更しません。先頭の非零 pivot を選ぶ通常の Gaussian/Gauss–Jordan 消去で、行交換を処理します。整数座標の寸法は非負int、積・作業用寸法が型と実メモリに収まることが前提です。

消去1回の逆元計算は高々 min(n,m) 回です。逆元1回のコストを I と数える型では、表の時間に O(min(n,m)I) を加えます。

全中間演算のオーバーフローは呼出側で避けてください。符号なしの剰余2^w整数も体ではありません。無効な形状・添字・右辺長は assert の対象で、releaseでも守る必要があります。0×0 の行列式は1、逆行列は存在し、階数は0。0×m と n×0 も区別して保存します。det/invは正方行列のみ、rank/solveは長方形にも対応します。

`LinearSystem<T>` は `particular` と `basis` を持つ所有値です。不整合を nullopt、一意解を basis.empty() で区別します。要素参照は親の代入・move・破棄後に使用しないでください。消去const操作では参照は有効です。copyは独立、Matrixのmove元は0×0に初期化されます。公開フィールドを利用者が変更した後の解の正しさは保証しません。

結果型の操作で使う B は `particular.size() + basis.size() + Σ basis[i].size()` とします。
solve が返した正常な結果では B=O(m(d+1)+1) ですが、利用者が自由に集成体初期化・変更した結果も、この B を用いれば評価できます。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <atcoder/modint>
#include "blueberry/math/matrix.hpp"
int main() {
  using Mint = atcoder::modint998244353;
  blueberry::Matrix<Mint> a(2, 2);
  a(0, 0) = 1; a(0, 1) = 2; a(1, 0) = 3; a(1, 1) = 4;
  assert(a.det() == Mint(-2) && a.rank() == 2);
  auto inverse = a.inv();
  assert(inverse && (*inverse)(0, 0) == Mint(-2));
  auto solution = a.solve({5, 11});
  assert(solution && solution->basis.empty());
  assert(solution->particular[0] == 1 && solution->particular[1] == 2);
  blueberry::Matrix<Mint> free(0, 3);
  assert(free.solve({})->basis.size() == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Matrix<T> a` | O(1) | [開く](#default) |
| `Matrix<T> a(int rows, int cols, const T& value = T{})` | O(nm+1) | [開く](#construct) |
| `Matrix<T> copy(const Matrix<T>& other)` | O(nm+1) | [開く](#copy-construct) |
| `Matrix<T>& a.operator=(const Matrix<T>& other)` | O(nm+n_old m_old+1) | [開く](#copy-assign) |
| `Matrix<T> moved(Matrix<T>&& other)` | O(1) | [開く](#move-construct) |
| `Matrix<T>& a.operator=(Matrix<T>&& other)` | O(n_old m_old+1) | [開く](#move-assign) |
| `a.~Matrix()` | O(nm+1) | [開く](#destroy) |
| `int a.rows() const` | O(1) | [開く](#rows) |
| `int a.cols() const` | O(1) | [開く](#cols) |
| `T& a.operator()(int row, int col)` | O(1) | [開く](#element) |
| `const T& a.operator()(int row, int col) const` | O(1) | [開く](#const-element) |
| `T a.det() const` | O(n³+n²+1) | [開く](#det) |
| `optional<Matrix<T>> a.inv() const` | O(n³+n²+1) | [開く](#inverse) |
| `int a.rank() const` | O(nm min(n,m)+nm+1) | [開く](#rank) |
| `optional<LinearSystem<T>> a.solve(const vector<T>& rhs) const` | O(n(m+1)min(n,m)+n(m+1)+m(d+1)+1) | [開く](#solve) |
| `LinearSystem<T> solution` | O(1) | [開く](#result-default) |
| `LinearSystem<T> solution{particular, basis}` | O(B+1) | [開く](#result-aggregate) |
| `LinearSystem<T> copy(const LinearSystem<T>& other)` | O(B+1) | [開く](#result-copy) |
| `LinearSystem<T>& solution.operator=(const LinearSystem<T>& other)` | O(B_old+B+1) | [開く](#result-copy-assign) |
| `LinearSystem<T> moved(LinearSystem<T>&& other)` | O(1) | [開く](#result-move) |
| `LinearSystem<T>& solution.operator=(LinearSystem<T>&& other)` | O(B_old+1) | [開く](#result-move-assign) |
| `solution.~LinearSystem()` | O(B+1) | [開く](#result-destroy) |
| `vector<T> solution.particular` | 参照取得 O(1)、コピー O(m) | [開く](#particular) |
| `vector<vector<T>> solution.basis` | 参照取得 O(1)、コピー O(md) | [開く](#basis) |

<details class="api-operation" id="default" markdown="1">
<summary><code>Matrix&lt;T&gt; a</code> — O(1)</summary>

0行0列の空行列を作ります。

{% raw %}
```cpp
blueberry::Matrix<Mint> a;
assert(a.rows() == 0 && a.cols() == 0);
```
{% endraw %}

注意点: det() は1、rank() は0、inv() は値を持つ0×0行列です。
</details>

<details class="api-operation" id="construct" markdown="1">
<summary><code>Matrix&lt;T&gt; a(int rows, int cols, const T&amp; value = T{})</code> — O(nm+1)</summary>

全要素を value とする n=rows 行、m=cols 列の行列を作ります。

{% raw %}
```cpp
blueberry::Matrix<Mint> a(2, 3, Mint(7));
assert(a(1, 2) == 7);
```
{% endraw %}

注意点: rows,cols>=0。積が size_t と利用可能メモリに収まること。0×m や n×0 の形状も保存します。
</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>Matrix&lt;T&gt; copy(const Matrix&lt;T&gt;&amp; other)</code> — O(nm+1)</summary>

形状と全要素を独立にコピーします。

{% raw %}
```cpp
auto copy = a;
```
{% endraw %}

注意点: コピー先の変更は元に影響しません。追加メモリ O(nm+1)。
</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>Matrix&lt;T&gt;&amp; a.operator=(const Matrix&lt;T&gt;&amp; other)</code> — O(nm+n_old m_old+1)</summary>

既存状態を置き換えて独立コピーを代入し、自身の参照を返します。

{% raw %}
```cpp
copy = a;
```
{% endraw %}

注意点: 自己代入は有効。代入先の既存要素への参照を保持して使用しないでください。
</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>Matrix&lt;T&gt; moved(Matrix&lt;T&gt;&amp;&amp; other)</code> — O(1)</summary>

格納領域の所有権を移し、元を0×0行列にします。

{% raw %}
```cpp
auto moved = std::move(a);
assert(a.rows() == 0 && a.cols() == 0);
```
{% endraw %}

注意点: 元は再代入可能。元の要素への参照を移動後に使用しないでください。
</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>Matrix&lt;T&gt;&amp; a.operator=(Matrix&lt;T&gt;&amp;&amp; other)</code> — O(n_old m_old+1)</summary>

旧状態を解放して所有権を移し、自身の参照を返します。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: 自己 move は何もしません。元は0×0、旧要素への参照は無効です。
</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>a.~Matrix()</code> — O(nm+1)</summary>

所有要素と格納領域をすべて解放します。

{% raw %}
```cpp
{ blueberry::Matrix<Mint> temporary(2, 2); }
```
{% endraw %}

注意点: 通常はスコープ終了時に自動実行します。要素への参照は無効になります。
</details>

<details class="api-operation" id="rows" markdown="1">
<summary><code>int a.rows() const</code> — O(1)</summary>

行数 n を返します。

{% raw %}
```cpp
int n = a.rows();
```
{% endraw %}

注意点: 0×m では0です。列数は別に保持します。
</details>

<details class="api-operation" id="cols" markdown="1">
<summary><code>int a.cols() const</code> — O(1)</summary>

列数 m を返します。

{% raw %}
```cpp
int m = a.cols();
```
{% endraw %}

注意点: n×0 では0、0×m では m です。
</details>

<details class="api-operation" id="element" markdown="1">
<summary><code>T&amp; a.operator()(int row, int col)</code> — O(1)</summary>

0-indexed の要素への変更可能な参照を返します。

{% raw %}
```cpp
a(0, 1) = Mint(3);
```
{% endraw %}

注意点: 0<=row<n、0<=col<m。代入・move・破棄後はこの参照を使用しないでください。消去操作はconstなので参照を無効化しません。
</details>

<details class="api-operation" id="const-element" markdown="1">
<summary><code>const T&amp; a.operator()(int row, int col) const</code> — O(1)</summary>

const 行列の要素への参照を返します。

{% raw %}
```cpp
const auto& view = a;
auto value = view(0, 1);
```
{% endraw %}

注意点: 同じ添字条件と参照寿命が適用されます。空行列には有効な添字がありません。
</details>

<details class="api-operation" id="det" markdown="1">
<summary><code>T a.det() const</code> — O(n³+n²+1)</summary>

正方行列の行列式を返します。行交換の符号と各 pivot を掛けます。

{% raw %}
```cpp
auto determinant = a.det();
```
{% endraw %}

注意点: n==m。特異なら0、0×0なら1。追加メモリ O(n²+1)、元の値は変更しません。非零 pivot が必ず可逆な体を要求します。
</details>

<details class="api-operation" id="inverse" markdown="1">
<summary><code>optional&lt;Matrix&lt;T&gt;&gt; a.inv() const</code> — O(n³+n²+1)</summary>

正方行列の逆行列を返します。存在しなければ nullopt。

{% raw %}
```cpp
auto inverse = a.inv();
if (inverse) { assert(inverse->rows() == a.rows()); }
```
{% endraw %}

注意点: n==m かつ2n<=INT_MAX。0×0は存在する逆行列です。追加メモリ O(n²+1)。元と返り値の要素は独立です。
</details>

<details class="api-operation" id="rank" markdown="1">
<summary><code>int a.rank() const</code> — O(nm min(n,m)+nm+1)</summary>

長方形行列を含め、体上の階数を返します。

{% raw %}
```cpp
int rank = a.rank();
```
{% endraw %}

注意点: 0<=rank<=min(n,m)。0行または0列なら0。追加メモリ O(nm+min(n,m)+1)、元を変更しません。
</details>

<details class="api-operation" id="solve" markdown="1">
<summary><code>optional&lt;LinearSystem&lt;T&gt;&gt; a.solve(const vector&lt;T&gt;&amp; rhs) const</code> — O(n(m+1)min(n,m)+n(m+1)+m(d+1)+1)</summary>

Ax=rhs の全解を表す特殊解と零空間の基底を返します。不整合なら nullopt。d=m-rank(A) とします。

{% raw %}
```cpp
auto solution = a.solve(rhs);
if (solution) { auto x = solution->particular; }
```
{% endraw %}

注意点: rhs.size()==n、m<INT_MAX。自由変数を0とした特殊解、自由列の昇順の基底を返します。追加メモリ O(n(m+1)+m(d+1)+1)。0×m は任意の x、n×0 は rhs が全0のときだけ空ベクトル1個が解です。
</details>

<details class="api-operation" id="result-default" markdown="1">
<summary><code>LinearSystem&lt;T&gt; solution</code> — O(1)</summary>

空の particular と basis を持つ結果格納型を構築します。

{% raw %}
```cpp
blueberry::LinearSystem<Mint> solution;
```
{% endraw %}

注意点: それ自体は何らかの行列の解であることを検査しません。
</details>

<details class="api-operation" id="result-aggregate" markdown="1">
<summary><code>LinearSystem&lt;T&gt; solution{particular, basis}</code> — O(B+1)</summary>

2個の vector を集成体初期化で格納します。

{% raw %}
```cpp
blueberry::LinearSystem<Mint> solution{std::vector<Mint>{1, 2}, {}};
```
{% endraw %}

注意点: lvalue を渡すとコピー、std::move を渡すと格納領域を移動します。任意に作った値の線形独立性は検査しません。
</details>

<details class="api-operation" id="result-copy" markdown="1">
<summary><code>LinearSystem&lt;T&gt; copy(const LinearSystem&lt;T&gt;&amp; other)</code> — O(B+1)</summary>

特殊解と全基底を独立にコピーします。

{% raw %}
```cpp
auto copy = solution;
```
{% endraw %}

注意点: 基底の本数 d、各 vector 長 m は有効なsolve結果についての記号です。
</details>

<details class="api-operation" id="result-copy-assign" markdown="1">
<summary><code>LinearSystem&lt;T&gt;&amp; solution.operator=(const LinearSystem&lt;T&gt;&amp; other)</code> — O(B_old+B+1)</summary>

旧vector群を置き換え、独立コピーを代入します。B_oldは代入前のBです。

{% raw %}
```cpp
copy = solution;
```
{% endraw %}

注意点: 自己代入は有効。旧vector要素への参照は保持しないでください。
</details>

<details class="api-operation" id="result-move" markdown="1">
<summary><code>LinearSystem&lt;T&gt; moved(LinearSystem&lt;T&gt;&amp;&amp; other)</code> — O(1)</summary>

標準vectorのmoveで格納領域を引き継ぎます。

{% raw %}
```cpp
auto moved = std::move(solution);
```
{% endraw %}

注意点: 元のvectorの内容は未規定で、破棄・再代入が可能です。Matrix自身のmove元0×0契約とは区別してください。
</details>

<details class="api-operation" id="result-move-assign" markdown="1">
<summary><code>LinearSystem&lt;T&gt;&amp; solution.operator=(LinearSystem&lt;T&gt;&amp;&amp; other)</code> — O(B_old+1)</summary>

旧vector群を解放し、標準vectorのmoveで置き換えます。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: 返り値は自身の参照。元のvector内容は未規定。
</details>

<details class="api-operation" id="result-destroy" markdown="1">
<summary><code>solution.~LinearSystem()</code> — O(B+1)</summary>

所有する特殊解・基底vectorを解放します。

{% raw %}
```cpp
{ blueberry::LinearSystem<Mint> temporary; }
```
{% endraw %}

注意点: 通常は自動破棄。vector要素への参照は無効になります。
</details>

<details class="api-operation" id="particular" markdown="1">
<summary><code>vector&lt;T&gt; solution.particular</code> — 参照取得 O(1)、コピー O(m)</summary>

長さmの特殊解です。solve成功時、A*particular=rhs を満たします。

{% raw %}
```cpp
const auto& x = solution.particular;
```
{% endraw %}

注意点: 公開フィールドです。利用者が書き換えると解の保証は失われます。基底を加える前の基点で、最小値等の最適性は保証しません。
</details>

<details class="api-operation" id="basis" markdown="1">
<summary><code>vector&lt;vector&lt;T&gt;&gt; solution.basis</code> — 参照取得 O(1)、コピー O(md)</summary>

d本の長さmの線形独立ベクトル。A*v=0 を満たし、全解は particular+Σ c_i*basis[i] と一意に表せます。

{% raw %}
```cpp
const auto& directions = solution.basis;
```
{% endraw %}

注意点: 一意解では空。solveは自由列を昇順に選び、各基底の対応自由変数を1にします。公開vectorの変更・再確保時は通常のvector参照無効化規則に従います。
</details>

## 出典・検証

- 公式問題: [Determinant](https://judge.yosupo.jp/problem/matrix_det)、[Inverse](https://judge.yosupo.jp/problem/inverse_matrix)、[Rank](https://judge.yosupo.jp/problem/matrix_rank)、[System of Linear Equations](https://judge.yosupo.jp/problem/system_of_linear_equations)。対応 verify/math drivers を用意しています。
- [公式の連立一次方程式仕様](https://github.com/yosupo06/library-checker-problems/blob/master/linear_algebra/system_of_linear_equations/task.md) と、[Fastest determinant 401316](https://judge.yosupo.jp/submission/401316)・[361510](https://judge.yosupo.jp/submission/361510) を調査。高速提出はブロック化・SIMD・遅延剰余などを使います。本APIは任意の正確な体を扱う単純な連続配列消去で、公開コードを転記していません。
- `tests/random/algebra-expansion.cpp`: F3 の全入力ベクトル列挙による像の大きさ・解集合との独立比較、行列式の全置換展開、逆行列の積、空・特異・不整合・コピー・moveを検証。
