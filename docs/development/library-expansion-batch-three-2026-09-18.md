---
title: Library expansion batch three (2026-09-18)
---

## 追加内容

競技用途の実装数を優先し、ACLにない12種類を追加した。既存APIの変更・削除はない。

| 分類 | 新規ライブラリ | 公式verify問題数 |
| --- | --- | ---: |
| Data Structure | Persistent Queue / Static Range Distinct / Rectangle Union Area | 3 |
| Graph | Assignment / Complement Components / Count Spanning Trees / Minimum Spanning Forest | 5 |
| Math | Polynomial Taylor Shift / Polynomial Product / Stirling Numbers Second Kind / Montmort Numbers | 4 |
| String | Count Subsequences | 1 |

標準的なDSU・modint・畳み込みはACLを使用する。登録ライブラリは54から66、
all.hppを含む公開ヘッダは55から67、公式verify driverは81から96となった。
既存Ordered SetのPredecessor Problem、Ordered MultisetのLarge Array Compositeにも
専用driverを追加し、従来の「実装あり・専用verifyなし」2件を解消した。

Library Checker公式APIを再取得した253問題のスナップショットに対し、
対応表は実装＋verifyあり70→85、専用verifyなし2→0、未対応171→158。
ACL推奨9と補助verifyのみ1はBlueberryの実装数に含めない。
対応表の「verifyあり」はdriverの存在を示し、ACの根拠は実行結果で区別する。

## 検証

- `make check COMPILE_JOBS=4`: Python45テスト、66 API文書／70実行例、67ヘッダ、96driver、26ランダムテスト×20seedを通過。
- GCC13.3 / Clang18 × gnu++20 / gnu++23のcompileとrandom各20seedを通過。サイズ前提assertの最終修正はgraph担当でも4構成を再実行した。
- `CXXFLAGS=-DNDEBUG`、ACLを`-isystem`にしたrelease header検証を4構成で通過。
- 新規4ランダムテストをClang ASan+UBSan、seed23で通過。
- 追加15driverの公式308ケースを各1回通過（DS46、Graph114、Math85、String16、既存API補完47）。
- 担当間の独立レビューで、グラフのサイズ上限とAPI文書のHTMLエスケープを修正した。

初期の対象別公式記録は`.verification/batch-three-{ds,graph,math,string,existing}/`に保存した。
これらは未commit状態の新規ファイルを検証したため、metadataのrevisionは開始時HEADの
`6a1e754`であり、同revisionの既存実装の性能測定として扱わない。
最終commitに対する全体の`make verify`と公開処理の結果はPR／GitHub Actions、
公開[測定一覧](https://blueberry1001.github.io/Blueberry-library/benchmarks.html)で追跡する。
ローカル全体実行は`VERIFY_REPEATS=1 VERIFY_TIMEOUT=7200`、CIは既定3回とし、同一測定に混ぜない。

## 候補比較と制約

比較コード・入力・コンパイラ・反復生ログは`benchmark/batch-three-*`に保存した。
Graph、DS、Mathの一次資料と採用理由はそれぞれ
[Graph調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-graph-research.md)、[DS調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-ds.md)、[Math調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-math.md)。
部分列数は[API文書](https://blueberry1001.github.io/Blueberry-library/blueberry/string/count-subsequences.hpp.html)に比較結果を記載した。
全候補は独立実装で、同一入力のchecksumを確認している。

永続queueの採用理由はO(Q)メモリであり、doubling候補より速いとの主張はしない。
区間種類数は問い合わせを事前収集しなくてよいAPIを優先した。
offline Fenwickのほうが高速な用途もある。共有WSL環境での測定値を他環境や
公開judgeの測定値と直接比較せず、全実装に対する最速性も主張しない。

## ブランチ整理

ユーザーの整理依頼に基づき、`git merge-base --is-ancestor <tip> origin/main`で
全対象の終了コード0を確認し、worktree使用中のブランチを除外した。
ローカル6本を`git branch -d`、リモート14本を名前を指定して削除した。
自動承認レビューが最初の一括操作を証拠不足で拒否したため、各対象を個別に
祖先確認し直してから承認された操作を行った。force deleteは使用していない。

別worktreeの`codex/library-expansion-operations`は保持した。
未マージのリモート`codex/docs-and-verification-metrics`、
`codex/docs-layout-and-submit-notes`、`codex/verified-library-foundation`も保持した。
削除した各tipはmainの履歴に残っている。以下に整理時のIDを記録する。

| ブランチ | 整理範囲 | tip |
| --- | --- | --- |
| codex/add-non-acl-libraries | local | b4695f96724e1cafe34a42fdd6c542e62517ab68 |
| codex/contest-readiness | local / remote | 56372e9701623ed855ccd0cc4ad4ab842cb3c23e |
| codex/contributor-guide | local / remote | 323cd629d813defa22d495f6400514b9da812fdc |
| codex/legacy-library-migration | local / remote | 4bacd78b60b957c0b53cee976574ad26c98bdd57 |
| codex/library-checker-expansion | local / remote | d1e9e58dd6c053e8824235b167cf1b437f570260 |
| codex/library-expansion-batch-two | local / remote | 48547f7c1c8147220149e33929f0a4302da77cd1 |
| codex/fps-ntt-library | remote | f240474f93955ccce15ffdf8b23d024e4c76f9c2 |
| codex/hld-fastest-compare-20260913 | remote | f1efae1ac2225aeeec379e2ee3981f0d2611ff50 |
| work/benchmark | remote | c3f62504322e68a421ff9ea1217443efcf88c0ed |
| work/ds-core | remote | 5871390867bf866176c191d2f7076eea91c8fb9a |
| work/integration | remote | 8507f18af518647f1d58c8c212dcb293b966f227 |
| work/library-audit | remote | 1e88178d928e106518d32ca11b6b8e5e39a47025 |
| work/tree | remote | 6c8c5a82929dc81fde7b93689c5043d4c23c8a44 |
| work/verify-infra | remote | 0a04b8a276a1c3703c5fbb391d430a824e976efc |
| work/verify-readability | remote | ad5abede4ffdd77530872ed34912355e7986bb54 |
