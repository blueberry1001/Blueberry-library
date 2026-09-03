# Blueberry Library

C++20向けの競技プログラミングライブラリです。

各実装はヘッダ単体で利用でき、✅が付いた項目は
[Library Checker](https://judge.yosupo.jp/) の公式テストデータで検証されています。

<div class="library-summary">
  <div><strong>Data Structure</strong><span>DSU / Fenwick / Segment Tree / Sparse Table</span></div>
  <div><strong>Graph</strong><span>Dijkstra / LCA</span></div>
  <div><strong>Math</strong><span>Prime Sieve</span></div>
  <div><strong>String</strong><span>Z Algorithm</span></div>
</div>

## 方針

- 必要な標準ヘッダを含む自己完結した実装
- 半開区間 `[l, r)` と0-indexedを基本とするAPI
- 実問題による検証コードと計算量の明記
- `oj-bundle` で提出用の単一ファイルへ展開可能

リポジトリ: [blueberry1001/Blueberry-library](https://github.com/blueberry1001/Blueberry-library)
