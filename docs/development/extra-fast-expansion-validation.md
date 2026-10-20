---
title: 高速版追加の検証記録
---

# 高速版追加の検証記録

2026年10月20日。[実装・設計](extra-fast-expansion.md)と[比較測定](measurements/extra-fast-expansion/README.md)を補足する。
公開C++実装はcommit `0f26207`で固定し、以後の変更は測定・検証記録に限る。

## ソースと検証範囲

- 区間和177行、静的RMQ254行、bitset191行。
- 実際に検証したbyteのSHA-256は[固定記録](measurements/extra-fast-expansion/validation/source-freeze.json)と各runnerのJSONに保存する。
- 通常の一覧・検索・`blueberry/all.hpp`に高速版を混ぜず、専用ページから3ライブラリへアクセスする。
- 公式verifyの行列積はbitsetのXORを検証するdriverであり、JIT版の行列積アルゴリズムの再実装ではない。

## ローカル検証

完了した各コマンドと結果は測定ディレクトリの`validation/`に保存する。途中のログを全件成功として扱わない。
GCC 13.3 / Clang 18.1、C++20 / C++23を用い、通常intrinsics・手書きasm・Intel構文asm・SIMD無効を区別する。
seed・compiler flags・source hash・log hashを記録し、失敗を再現できるようにする。

| 検査 | 結果・記録 |
| --- | --- |
| `make check COMPILE_JOBS=8` | Python 149 tests、115文書・116実行例、115headerの単独/重複include、152verify compile、114random programs×20 seedsに合格 |
| release header | `-DNDEBUG`、ACLを`-isystem`で指定した全115headerに合格 |
| 新2headerのcompiler / standard / backend | 全32構成でcompile成功、各seed 1 / 20261020の愚直比較に合格 |
| 新2headerのASan + UBSan | GCC / Clang、seed 20261020で合格 |
| 区間和のbackend | GCC / Clangの4経路とGCC ASan + UBSanに合格 |

全体検査は[check記録](measurements/extra-fast-expansion/validation/check.json)、新2headerの詳細は[backend記録](measurements/extra-fast-expansion/validation/backends.json)、
区間和は[実行コマンド](measurements/extra-fast-expansion/validation/check-sum.sh.snapshot)と
[ログ](measurements/extra-fast-expansion/validation/sum-backends.log)に保存した。
棄却したbitsetのunroll候補だけはGCCの警告をエラー扱いにしてcompileに失敗する。
[再現記録](measurements/extra-fast-expansion/validation/bitset-unroll.json)はその失敗を保ち、公開版の失敗と区別する。

## 公式ケースとAC根拠

今回変更した区間和と新RMQ・bitsetの3targetは、公式checkerで各3回すべてACだった。
行列積26ケース・区間和21ケース・RMQ27ケースの計74ケースを3反復した。
[公式実行記録](measurements/extra-fast-expansion/validation/official.json)と
[測定report](measurements/extra-fast-expansion/validation/official/report.md)を保存する。
これは正誤検証の記録であり、上位提出と同条件の比較値は別の測定reportを参照する。

全対象の`make verify VERIFY_REPEATS=1 VERIFY_TIMEOUT=120`も実行した。
4件passed・148件not_run・0件failedで予算停止したため、全体成功とは扱わない。
[元のlog](measurements/extra-fast-expansion/validation/full-verify-budget.log)と
[部分report](measurements/extra-fast-expansion/validation/full-budget/report.md)、
[復元記録](measurements/extra-fast-expansion/validation/budget.json)を保持する。
今回変更した3targetの3反復検証は別出力先で完了しており、この部分実行で上書きしていない。

## 公開確認

`make docs`とproduction Jekyll build、`python3 scripts/check_site.py`は成功した。
[最終サイト検査](measurements/extra-fast-expansion/validation/site-final-pages-linux.log)は5カテゴリ・114 APIページを確認した。
現在のソースに一致するAC根拠は152件で、既存149件と今回実測した3件を統合した。
[根拠照合](measurements/extra-fast-expansion/validation/site-receipt-validation.log)後、空の選択planで
公開用の152行を`not_selected`として生成した。全152件を新たに実測したという意味ではない。
[make docs](measurements/extra-fast-expansion/validation/docs-final-linux.log)と
[Jekyll](measurements/extra-fast-expansion/validation/jekyll-final-pages-linux.log)の原文logも保持する。
Jekyll 3では生成されたGemfileのある`.verify-helper/markdown/`を作業ディレクトリとして起動する。
最初にrepo rootから起動した際はPages pluginが読み込まれず、既存のfrontmatterなし文書へのリンク検査で停止した。
[元の失敗](measurements/extra-fast-expansion/validation/site-preflight-linux.log)を保持し、
[実行場所を修正した検査](measurements/extra-fast-expansion/validation/site-preflight-pages-linux.log)の成功と区別する。
公開ソースの修正は行っていない。

実ブラウザはPlaywrightとheadless Microsoft Edgeで確認した。Browser pluginが利用できないため既存の環境を使用した。
幅1440 / 390 / 320、右上メニューのキーボード操作、専用検索とリセット、3APIへの往復、通常6ページへの非混入、
JavaScript無効時の一覧、console errorなしを確認した。スクリーンショットも目視した。
[実行script](measurements/extra-fast-expansion/validation/browser-ui.cjs.snapshot)、
[結果](measurements/extra-fast-expansion/validation/browser-ui.log)、
[記録](measurements/extra-fast-expansion/validation/browser-ui.json)を保存した。
`node tests/docs_ui_test.cjs`も成功した。
