<div class="site-hero">
  <p class="site-hero__eyebrow">Verified C++ competitive programming library</p>
  <h1>Blueberry Library</h1>
  <p>C++20向けの競技プログラミング用ライブラリです。</p>
  <a class="site-hero__link" href="https://github.com/blueberry1001/Blueberry-library">View the project on GitHub ↗</a>
</div>

各実装はヘッダ単体で利用でき、✅が付いた項目は
[Library Checker](https://judge.yosupo.jp/) の公式テストデータで検証されています。

<div class="library-summary">
{% assign category_ids = 'data-structure,graph,math,string,utility' | split: ',' %}
{% assign category_titles = 'Data Structure,Graph,Math,String,Utility' | split: ',' %}
{% for category in category_ids %}
  {% assign entries = site.data.libraries | where: 'category', category %}
  <a href="{{ '/categories/' | append: category | append: '.html' | relative_url }}"><strong>{{ category_titles[forloop.index0] }} · {{ entries.size }}</strong><span>{{ entries | map: 'name' | join: ' / ' | escape }}</span><span>用途と操作を確認する →</span></a>
{% endfor %}
</div>

[名前・用途から検索](#library-catalog) · [全ファイルと検証状態](#library-files)

[したい操作から候補を絞る]({{ '/operations.html' | relative_url }}) · [Library Checker 全問題の対応表]({{ '/library-checker.html' | relative_url }})

## 方針

- 必要な標準ヘッダを含む自己完結した実装
- 半開区間 `[l, r)` と0-indexedを基本とするAPI
- 実問題による検証コードと計算量の明記
- `oj-bundle` で提出用の単一ファイルへ展開可能

ACLの標準機能はACLを優先し、Blueberry LibraryではACLにない機能を補います。
既存の重複実装は互換性のため残しています。

[使い方ガイド]({{ '/guide.html' | relative_url }}) · [旧版からの移行]({{ '/migration.html' | relative_url }}) · [verify実行時間]({{ '/benchmarks.html' | relative_url }})
