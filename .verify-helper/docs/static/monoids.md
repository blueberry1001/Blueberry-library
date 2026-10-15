---
layout: page
title: モノイド一覧
body_class: monoid-page
---

# モノイド一覧

使いたい操作を開き、`S`・`F`と演算をコピーしてください。各コードは独立してACLで使えます。

<details markdown="1">
<summary>共通の使い方（ACLへの渡し方・初期化・注意点）</summary>

セグ木には `S, op, e`、遅延セグ木にはさらに `F, mapping, composition, id` を渡します。
各定義のコメントに従って葉の`S`を作り、次のどちらかで構築します。`e()`は空区間の単位元です。

{% raw %}
```cpp
#include <atcoder/segtree>
#include <atcoder/lazysegtree>
#include <vector>

// ここに選んだレシピをコピー。

// main 内: 各定義の葉の形式に合わせて全要素を埋める。
std::vector<S> init(n);
// 例: S が {sum,len} の場合
for (int i = 0; i < n; ++i) init[i] = S{a[i], 1};
```
{% endraw %}

セグ木は次のように構築します。

{% raw %}
```cpp
atcoder::segtree<S, op, e> seg(init);
```
{% endraw %}

遅延セグ木は次のように構築します。

{% raw %}
```cpp
atcoder::lazy_segtree<S, op, e, F, mapping, composition, id> seg(init);
```
{% endraw %}

共通操作は `seg.prod(l, r)`（区間の集約）、`seg.set(i, s)`（一点変更）、
遅延セグ木なら `seg.apply(l, r, f)`（区間更新）です。区間は **0-indexed の `[l, r)`**。
- **合成順:** `op(left, right)` は左から右。`composition(f, g)` は **`g` の後に `f`**。
- **計算量:** 各定義の演算は、記載がなければ`O(1)`です。木の構築は`O(N)`、区間取得・更新は`O(log N)`です。

</details>

## 用途から探す

<div class="library-catalog" data-catalog>
  <div class="catalog-controls" data-catalog-controls hidden>
    <div class="catalog-search-field"><label for="monoid-search">操作で検索 <small><kbd>/</kbd></small></label>
    <input id="monoid-search" data-catalog-search type="search" placeholder="例: 等差数列、反転 転倒数、DP、二乗和" autocomplete="off" aria-controls="monoid-results"></div>
    <div class="catalog-category-field"><label for="monoid-kind">載せる構造</label>
    <select id="monoid-kind" data-catalog-category aria-controls="monoid-results">
      <option value="">すべて</option><option value="segtree">セグ木</option><option value="lazy">遅延セグ木</option>
    </select></div>
    <button type="button" data-catalog-reset>条件をクリア</button>
    <span data-catalog-count role="status" aria-live="polite"></span>
  </div>
  <noscript><p>JavaScript が無効のため全レシピを表示しています。見出しとブラウザ内検索で探せます。</p></noscript>
  <p data-catalog-empty hidden>一致するレシピがありません。キーワードを減らすか条件をクリアしてください。</p>
  <div class="monoid-columns" aria-hidden="true"><span>更新操作</span><span>取得する値</span><span>使う構造</span></div>
  <div id="monoid-results">
  {% for recipe in site.data.monoids %}{% if recipe.code and recipe.code != '' %}
    <article data-library data-category="{{ recipe.kind | escape }}" data-keywords="{{ recipe.keywords | escape }} {{ recipe.search_aliases | escape }}" id="{{ recipe.id | escape }}">
      <details class="monoid-recipe" data-linked-details>
      <summary><span class="monoid-pair">
        <span class="monoid-update"><span class="visually-hidden">更新：</span>{{ recipe.update_label | escape }}</span>
        <span class="monoid-query"><span class="visually-hidden">取得：</span>{{ recipe.query_label | escape }}</span>
        <span class="monoid-kind">{% if recipe.kind == 'lazy' %}遅延セグ木{% else %}セグ木{% endif %}</span>
      </span></summary>
      {% if recipe.nonconstant_cost %}<p>{{ recipe.nonconstant_cost | escape }}</p>{% endif %}
      {% if recipe.note %}<p><code>{{ recipe.note | escape }}</code></p>{% endif %}
      {% if recipe.caveat != '' %}<p>{{ recipe.caveat | escape }}</p>{% endif %}
      {% if recipe.code %}<pre><code class="language-cpp">{{ recipe.code | escape }}</code></pre>{% endif %}
      </details>
    </article>
  {% endif %}{% endfor %}
  </div>
</div>

<details class="library-notes" markdown="1">
<summary>レシピを変えるとき</summary>

## レシピを変えるとき {#レシピを変えるとき}

- `op` の結合則・単位元、遅延作用の分配則を保ちます。
- `max_right` / `min_left` は、空区間で true になる単調な条件に使えます。
- 区間の順序そのものを反転する操作は通常の固定区間セグ木では扱えず、
[Implicit Treap]({{ '/blueberry/data-structure/implicit-treap.hpp.html' | relative_url }})などが必要です。

ACL の詳しい使い方は [segtree](https://atcoder.github.io/ac-library/production/document_ja/segtree.html)・
[lazy_segtree](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html) を参照してください。

</details>
