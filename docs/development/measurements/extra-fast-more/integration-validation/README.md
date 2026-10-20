---
title: 高速版追加の統合検証
---

# 統合検証

実装は`d8ee799`、FPSの警告・一覧隔離修正は`d9b160a`。性能測定と、このページの正しさ・公開確認の所要時間は区別する。最新mainのPR55を取り込んだFastIOで検証した。測定時の旧I/Oへ結果を読み替えない。

## 最終結果

| 確認 | 結果 | 記録 |
| --- | --- | --- |
| `make check`、GCC13.3 / GNU++20 | 149 Python tests、123 library docs / 124実行例、123 headers、167 verify compile、132 random programs × 20 seedsが成功 | [manifest](checks/final-d9b160a/manifest.json)、[log](checks/final-d9b160a/make-check-gcc20.log) |
| release単独・重複include | GCC13.3 / Clang18.1 × GNU++20 / GNU++23、各123 headers成功 | 同manifest、各release log |
| `make verify VERIFY_REPEATS=1 VERIFY_TIMEOUT=1800` | 新規13＋I/O更新の依存25、計38本で全ケースAC | [d8 report](official-d8ee799/current.json)、[log](official-d8ee799/make-verify.log) |
| FPS警告修正後の同コマンド | 影響4本を新sourceから再実行し全ケースAC | [report](official-results.json)、[plan](official-plan.json)、[log](make-verify.log) |
| source-valid AC記録 | 現在の全167本に一致。未選択163本を今回の実測として数えない | [state](source-valid-state.json)、[manifest](manifest.json) |
| `make docs` | 成功 | [log](make-docs.log) |
| Jekyll production build | 成功 | [log](jekyll-build.log) |
| `python3 scripts/check_site.py` | 5カテゴリ・122 API pages、測定4本・明示未選択163本、リンク・ナビゲーション・footerを確認 | [log](check-site.log) |
| `node tests/docs_ui_test.cjs` | 検索・カテゴリ・ソース切替・copy/anchor/fallbackが成功 | [log](ui/docs-ui-unit.log) |
| Playwright / Edge 155.0.4283.62 | 専用10件、検索/reset、API/戻るリンク、通常6ページからの隔離、キーボード、1440/390/320px、JS無効、consoleを確認 | [log](ui/final.log)、[runner](ui/runner.cjs.snapshot) |

make checkは`COMPILE_JOBS=8`、`CPPFLAGS=-isystem .deps/ac-library`、seed1..20、各random実行のtimeout5秒。release includeは`CXXFLAGS=-DNDEBUG`。assert有効のcheckとは別に実行した。各コマンド・exit code・compiler版・source hashはmanifestにある。

ブラウザ用の専用プラグインが提供されていないセッションだったため、通常のPlaywrightとインストール済みEdgeを使った。生成済みLinuxサイトをUNC経由で読み、1440pxと390pxの[screenshot](ui/extra-fast-more-desktop.png)・[mobile menu](ui/extra-fast-more-mobile-menu.png)も目視した。JavaScript無効時にも全10件のリンクが残り、横方向のはみ出しはなかった。

## 失敗と修正

- 最初の公式verifyはPATHに`oj`がなく起動に失敗した。[missing-oj](missing-oj/current.json)と原logを保持し、`/home/blueberry/.local/bin`をPATHへ追加して同じ全38対象を実行し直した。
- 最初のmake checkはcoverage生成物の一致確認で停止した。新FPS random testのmacro includeだけでは高速版依存が静的に見えず、通常FPSの一覧へ混ざるのが原因だった。literal includeのdefault分岐を明記し、既存の除外方針を維持した。生成物の手修正で通していない。
- 同時にGCC release includeでprivate FPS引数のshadow警告を検出。仮引数名だけを変更した。旧GCCの失敗・旧Clangの成功も[初回ログ](checks/initial-d8ee799/manifest.json)に保持する。[12条件のbyte同一性](../fps/shadow-warning-fix/results.json)、新sourceのstrict build16件、独立oracleの128実行も成功した。
- 初回site検査はREADMEがindex.htmlとして出力され測定リンクが切れていた。既存レポートと同じfront matterを追加し、README.htmlへ固定した。[初回log](site-initial-readme-index/check-site.log)を保持する。次の検査でMap文書の共通制約が段落だったため、規定どおり箇条書きへ整えた。[修正前log](site-initial-contract-lists/check-site.log)も保持し、最終検査は成功した。
- UIの初回runnerはFenwick検索を1件と誤って仮定していた。既存の多分木も関連aliasを持つため2件が正しく、結果を狭めず対象ライブラリが表示される期待値へ直した。[初回log](ui/initial-filter-count.log)とsourceを保持した。

測定ソース・入力・compiler・revisionが異なる結果を一つの改善率に混ぜず、失敗・欠測を0秒に置き換えていない。再実行用のwrapperは`scripts/*.snapshot`、各検証runnerは各folderに保存する。
