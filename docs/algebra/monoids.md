---
title: いろいろなモノイド
documentation_of: //blueberry/algebra/monoids.hpp
---

**[操作から検索する・共通の使い方を見る]({{ '/monoids.html' | relative_url }})**

ACLに渡すモノイドの定義集です。必要な例を一つコピーして使えます。
セグ木には `S, op, e`、遅延セグ木にはさらに `F, mapping, composition, id` を渡します。
`composition(f, g)` は `g` の後に `f` を適用します。整数は中間計算まで型の範囲に収めてください。
各演算は、記載がなければ`O(1)`です。

{% for recipe in site.data.monoids %}
{% if recipe.code %}
{% assign definition_anchor = recipe.id %}
{% if recipe.api and recipe.api != '' %}{% assign definition_anchor = recipe.api | split: '<' | first | downcase %}{% endif %}
<article data-monoid-definition id="{{ definition_anchor | escape }}">
  <h2>{{ recipe.title | escape }}</h2>
  {% if recipe.nonconstant_cost %}<p>{{ recipe.nonconstant_cost | escape }}</p>{% endif %}
  {% if recipe.note %}<p><code>{{ recipe.note | escape }}</code></p>{% endif %}
  <pre><code class="language-cpp">{{ recipe.code | escape }}</code></pre>
  {% if recipe.caveat and recipe.caveat != '' %}<p>{{ recipe.caveat | escape }}</p>{% endif %}
  {% if recipe.problem_url and recipe.problem_url != '' %}<p>応用問題: <a href="{{ recipe.problem_url | escape }}">{{ recipe.problem | escape }}</a>{{ recipe.problem_note | escape }}</p>{% endif %}
</article>
{% endif %}
{% endfor %}

<details markdown="1">
<summary>ヘッダとして使う場合</summary>

既存の `blueberry::monoid` もそのまま使えます。例えば、等差数列加算は次のように書けます。

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/lazysegtree>
#include "blueberry/algebra/monoids.hpp"
int main() {
    using M = blueberry::monoid::IndexAffineSum<long long>;
    std::vector<M::S> v;
    for (int i = 0; i < 5; ++i) v.push_back(M::leaf(0, i));
    atcoder::lazy_segtree<M::S, M::op, M::e, M::F,
                         M::mapping, M::composition, M::id> seg(v);
    int l = 1, r = 5;
    long long first = 3, step = 2;
    seg.apply(l, r, M::F{1, step, first - step * l});
    assert(seg.prod(0, 5).sum == 24); // 0, 3, 5, 7, 9
}
```
{% endraw %}

</details>
