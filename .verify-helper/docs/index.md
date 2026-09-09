<div class="site-hero">
  <p class="site-hero__eyebrow">Verified C++ competitive programming library</p>
  <h1>Blueberry Library</h1>
  <p>C++20向けの競技プログラミング用ライブラリです。</p>
  <a class="site-hero__link" href="https://github.com/blueberry1001/Blueberry-library">View the project on GitHub ↗</a>
</div>

各実装はヘッダ単体で利用でき、✅が付いた項目は
[Library Checker](https://judge.yosupo.jp/) の公式テストデータで検証されています。

<div class="library-summary">
  <a href="{{ '/categories/data-structure.html' | relative_url }}"><strong>Data Structure</strong><span>Rollback Union Find / Sparse Table ほか</span><span>用途と操作を確認する →</span></a>
  <a href="{{ '/categories/graph.html' | relative_url }}"><strong>Graph</strong><span>Dijkstra / LCA</span><span>最短路・木のクエリを確認する →</span></a>
  <a href="{{ '/categories/math.html' | relative_url }}"><strong>Math</strong><span>Prime Sieve</span><span>素数判定・列挙を確認する →</span></a>
  <a href="{{ '/categories/string.html' | relative_url }}"><strong>String</strong><span>Z Algorithm</span><span>文字列の一致を確認する →</span></a>
</div>

## 方針

- 必要な標準ヘッダを含む自己完結した実装
- 半開区間 `[l, r)` と0-indexedを基本とするAPI
- 実問題による検証コードと計算量の明記
- `oj-bundle` で提出用の単一ファイルへ展開可能

ACLの標準機能はACLを優先し、Blueberry LibraryではACLにない機能を補います。
既存の重複実装は互換性のため残しています。

[使い方ガイド]({{ '/guide.html' | relative_url }}) · [verify実行時間]({{ '/benchmarks.html' | relative_url }})
