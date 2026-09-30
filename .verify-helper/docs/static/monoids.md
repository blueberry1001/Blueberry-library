---
layout: page
title: いろいろなモノイド
---

# いろいろなモノイド

**欲しい操作を選んで、`S`・`F` と演算をコピーするためのレシピ集です。**
木には ACL を使います。Blueberry のヘッダは不要です。
各コードは独立しているので、使いたいものを一つコピーしてください。

<details markdown="1">
<summary>共通の使い方（ACLへの渡し方・初期化・注意点）</summary>

セグ木には `S, op, e`、遅延セグ木にはさらに `F, mapping, composition, id` を渡します。
各レシピの `leaf(...)` で葉を作り、次のどちらかで構築します。

{% raw %}
```cpp
#include <atcoder/segtree>
#include <atcoder/lazysegtree>
#include <vector>

// ここに選んだレシピをコピー。

// main 内: 各レシピの初期化例に合わせて全要素を埋める。
std::vector<S> init(n);
for (int i = 0; i < n; ++i) init[i] = leaf(a[i]);
```
{% endraw %}

セグ木の場合:

{% raw %}
```cpp
atcoder::segtree<S, op, e> seg(init);
```
{% endraw %}

遅延セグ木の場合:

{% raw %}
```cpp
atcoder::lazy_segtree<S, op, e, F, mapping, composition, id> seg(init);
```
{% endraw %}

共通操作は `seg.prod(l, r)`（区間の集約）、`seg.set(i, leaf(...))`（一点変更）、
遅延セグ木なら `seg.apply(l, r, f)`（区間更新）です。区間は **0-indexed の `[l, r)`**。
下の「使う例」は、そのレシピでの初期化と操作の例です。

- **葉と空区間は別:** 長さを持つレシピでは `seg(n)` の全要素は長さ 0。零配列も `leaf(0)` を並べます。添字を持つものは `leaf(0, i)`。
- **合成順:** `op(left, right)` は左から右。`composition(f, g)` は **`g` の後に `f`**。
- **型と計算量:** 中間積まで型に収まることが前提です。必要なら `long long` を `modint` 等に変更します（順序比較・ビット演算を使うレシピは除く）。結合・作用が `O(1)` なら構築 `O(N)`、操作 `O(log N)`。

</details>

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
  <p>空白区切りは AND 検索。英字の大文字・全角も正規化します。</p>
  <noscript><p>JavaScript が無効のため全レシピを表示しています。見出しとブラウザ内検索で探せます。</p></noscript>
  <p data-catalog-empty hidden>一致するレシピがありません。キーワードを減らすか条件をクリアしてください。</p>
  <div id="monoid-results">
  {% for recipe in site.data.monoids %}
    <article data-library data-category="{{ recipe.kind | escape }}" data-keywords="{{ recipe.keywords | escape }}" id="{{ recipe.id | escape }}">
      <h3>{{ recipe.title | escape }}</h3>
      <p><code>{{ recipe.structure | escape }}</code>{% if recipe.cost %} · {{ recipe.cost | escape }}{% endif %}</p>
      <p><strong>S・結合:</strong> <code>{{ recipe.state | escape }}</code></p>
      <p><strong>{% if recipe.kind == 'lazy' %}F・作用{% else %}操作{% endif %}:</strong> <code>{{ recipe.action | escape }}</code></p>
      <p>{{ recipe.caveat | escape }}</p>
      {% if recipe.code %}<pre><code class="language-cpp">{{ recipe.code | escape }}</code></pre>{% endif %}
      {% if recipe.usage %}<details><summary>使う例</summary><pre><code class="language-cpp">{{ recipe.usage | escape }}</code></pre></details>{% endif %}
      {% if recipe.id == 'beats' %}
      <p><code>chmin(x)</code> と区間和には、和・最大値 <code>max1</code>・厳密に二番目の最大値 <code>max2</code>・最大値の個数 <code>cnt</code> を持ちます。<code>max2 &lt; x &lt; max1</code> なら和を <code>(x-max1)*cnt</code> だけ変更し、最大値を <code>x</code> にします。<code>x &gt;= max1</code> は何もせず、それ以外は子へ降ります。</p>
      <p>子へ降りる判定が必要なので、通常の ACL <code>lazy_segtree</code> に <code>mapping</code> を渡すだけでは扱えません。<a href="{{ '/blueberry/data-structure/segment-tree-beats.hpp.html' | relative_url }}">Segment Tree Beats</a> を使います。</p>
      {% endif %}
      {% if recipe.problem_url != '' %}<p>応用問題: <a href="{{ recipe.problem_url | escape }}">{{ recipe.problem | escape }}</a>{{ recipe.problem_note | escape }}</p>{% endif %}
    </article>
  {% endfor %}
  </div>
</div>

## レシピを変えるとき

`op` の結合則・単位元、遅延作用の分配則を保ちます。
`max_right` / `min_left` は、空区間で true になる単調な条件に使えます。
区間の順序そのものを反転する操作は通常の固定区間セグ木では扱えず、
[Implicit Treap]({{ '/blueberry/data-structure/implicit-treap.hpp.html' | relative_url }})などが必要です。

ACL の詳しい使い方は [segtree](https://atcoder.github.io/ac-library/production/document_ja/segtree.html)・
[lazy_segtree](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html) を参照してください。
問題リンクは応用先です。問題ごとの初期化・型・出力形式に合わせて使ってください。
