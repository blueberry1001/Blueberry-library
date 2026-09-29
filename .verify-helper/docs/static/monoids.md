---
layout: page
title: いろいろなモノイド
---

# いろいろなモノイド

「区間和」以外にも、括弧列・転倒数・最大部分配列・DPをセグ木に載せられます。
まず欲しい答えを選び、**隣り合う区間を何の情報で合成できるか**を考えます。
標準の木は外部ライブラリの ACL を使い、Blueberry は載せる演算を提供します。

**実装済み**は [monoids.hpp の API・コピーできる使用例]({{ '/blueberry/algebra/monoids.hpp.html' | relative_url }})、
**既存実装**は個別ライブラリへ進んでください。**設計例**は必要な状態と式を示した発展案で、
このヘッダに実装されていません。問題リンクは応用先であり、すべてを自動 verify 済みという意味ではありません。

## 用途から探す

<div class="library-catalog" data-catalog>
  <div class="catalog-controls" data-catalog-controls hidden>
    <label for="monoid-search">操作・答え・別名で検索（<kbd>/</kbd> で移動）</label>
    <input id="monoid-search" data-catalog-search type="search" placeholder="例: 等差数列、反転 転倒数、DP、二乗和" autocomplete="off" aria-controls="monoid-results">
    <label for="monoid-kind">載せる構造</label>
    <select id="monoid-kind" data-catalog-category aria-controls="monoid-results">
      <option value="">すべて</option><option value="segtree">セグ木</option><option value="lazy">遅延セグ木</option><option value="beats">Beats</option>
    </select>
    <button type="button" data-catalog-reset>条件をクリア</button>
    <span data-catalog-count role="status" aria-live="polite"></span>
  </div>
  <p>空白区切りは AND 検索。英字の大文字・全角も正規化します。「実装済み」「設計例」も検索できます。</p>
  <noscript><p>JavaScript が無効のため全レシピを表示しています。見出しとブラウザ内検索で探せます。</p></noscript>
  <p data-catalog-empty hidden>一致するレシピがありません。キーワードを減らすか条件をクリアしてください。</p>
  <div id="monoid-results">
  {% for recipe in site.data.monoids %}
    <article data-library data-category="{{ recipe.kind | escape }}" data-keywords="{{ recipe.keywords | escape }}" id="{{ recipe.id | escape }}">
      <h3>{{ recipe.title | escape }}</h3>
      {% assign api_anchor = recipe.api | split: '<' | first | downcase %}
      <p><strong>{{ recipe.status | escape }}</strong> · {{ recipe.structure | escape }}{% if recipe.api != '' %} · <a href="{{ '/blueberry/algebra/monoids.hpp.html' | relative_url }}#{{ api_anchor }}"><code>{{ recipe.api | escape }}</code></a>{% endif %}{% if recipe.id == 'beats' %} · <a href="{{ '/blueberry/data-structure/segment-tree-beats.hpp.html' | relative_url }}">SegmentTreeBeats API</a>{% endif %}</p>
      <p><strong>持つ情報・合成:</strong> {{ recipe.state | escape }}</p>
      <p><strong>更新・使い方:</strong> {{ recipe.action | escape }}</p>
      <p><strong>前提・注意:</strong> {{ recipe.caveat | escape }}</p>
      {% if recipe.problem_url != '' %}<p>応用問題: <a href="{{ recipe.problem_url | escape }}">{{ recipe.problem | escape }}</a>{{ recipe.problem_note | escape }}</p>{% endif %}
    </article>
  {% endfor %}
  </div>
</div>

## 等差数列加算をすぐ使う

`[l,r)` に `first, first+step, first+2*step, ...` を加算するとき、
絶対添字 `i` に対して `step*i + (first-step*l)` を加えます。
`IndexAffineSum<T>` は区間和に加えて **区間長と添字和**を持つので、ノードがどこで分割されても同じ作用を配れます。
より一般に `x_i ← a*x_i+b*i+c` を扱い、更新後の和は `a*sum+b*index_sum+c*len` です。

この表現なら通常の加算・代入・乗算・一次式による代入も同じ型で行えます。
`leaf(value,i)` の `i` は木全体で共通の座標にし、更新区間の左端を 0 に戻さないでください。
具体的な ACL の型宣言・構築・assert 付き例は [API ページ]({{ '/blueberry/algebra/monoids.hpp.html' | relative_url }})にあります。
[CSES Polynomial Queries](https://cses.fi/problemset/task/1736) は初項 1、公差 1 の例です。

## 載せる前のチェック

1. **セグ木:** `op(op(a,b),c)=op(a,op(b,c))` と単位元が必要です。交換法則は不要です。関数合成・括弧・転倒数では左右を入れ替えられません。
2. **遅延セグ木:** 区間の要約だけで更新でき、作用が合成でき、`mapping(f,op(x,y))=op(mapping(f,x),mapping(f,y))` を満たす必要があります。ACL の `composition(f,g)` は **g の後に f** です。詳細は [ACL 公式仕様](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html)を参照してください。
3. **空区間:** 葉と単位元は別です。長さ付きモノイドで `segtree(n)` / `lazy_segtree(n)` を使うと、全要素が長さ 0 の単位元になります。長さ n の零配列が欲しいときも `leaf(0)`（添字付きなら `leaf(0,i)`）を n 個並べて構築します。
4. **計算量:** 以下の定数サイズ状態は結合・作用 O(1)、木の構築とメモリ O(N)、一点変更・区間集約・通常の遅延更新 O(log N) です。行列・bitset・多項式では演算自体の費用を掛けます。Beats は別の償却評価です。
5. **数値:** 中間積・個数の積・添字和を含めて型に収まる必要があります。二乗和や転倒数は入力の最大値だけで型を決めないでください。浮動小数点は丸めにより厳密な結合則を満たしません。
6. **境界探索:** `max_right` / `min_left` には単位元で true になる単調な判定が必要です。負数のある区間和で「和 ≤ K」をそのまま使うことはできません。

## Beats は「いつでも作用できるモノイド」ではない

`chmin(x)` と区間和では、和・長さ・最大値だけでは足りません。
最大値 `max1`、**厳密に二番目の最大値** `max2`、最大値の個数 `cnt` を持つと、
`max2 < x < max1` のときだけ和を `(x-max1)*cnt` だけ変更できます。
`x >= max1` は何もしません。`x <= max2` では子へ降ります（等号も安全側に降りる）。
`chmax` は最小側も対称に管理し、加算との相互作用も保ちます。

これは失敗できる作用と再帰を含むため、通常の ACL `lazy_segtree` にこの mapping を渡すだけでは動きません。
[既存 Segment Tree Beats]({{ '/blueberry/data-structure/segment-tree-beats.hpp.html' | relative_url }})は
chmin/chmax/add と sum/min/max を実装しています。詳細な数値制約と償却計算量はそのページで確認してください。
区間 modulo・平方根・除算なども「変化しない区間を止める」発想は似ていますが、
各操作について値域・ポテンシャルと償却解析が必要で、既存 Beats がそのまま対応するわけではありません。

## さらに広げるとき

区間反転は要素順を変えるため、通常の固定区間 segtree の遅延作用にはできません。
必要なら [Implicit Treap]({{ '/blueberry/data-structure/implicit-treap.hpp.html' | relative_url }})などで正順・逆順の集約を持ちます。
「長さ付きハッシュなら衝突がなくなる」「区間和だけで中央値も求まる」といった推論にも注意してください。
状態が十分か、閉じた演算か、状態サイズが増えすぎないかを小さい愚直解との比較で確認します。
