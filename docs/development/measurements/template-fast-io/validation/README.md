# 検証ログ

C++実装は`46a723272443c91bef717730e7931d4492e51236`のもの。
その後のAPIページ間リンク修正はC++コードと使用例を変更していない。
最新mainとの統合後の全体検証・完全な公開証跡はPRとmainのCIで確認する。

| 記録 | 結果・範囲 |
| --- | --- |
| `make-check.log` | 成功。unit 149件、docs 114ページ・使用例115個、header 114件、verify 152件、乱数112本×20 seed |
| `new-api-initial.log` | 初回の空引数printの警告エラーと、修正後GCC/Clang × gnu++20/23の検証を区別して保存 |
| `new-api-current.sh` / `.log` | accessorキャッシュ後のGCC/Clang・固定seed・ASan/UBSan再検証が成功。環境とhashを記録 |
| `new-api-normal-exit.log` | 正常終了時のsingleton flushに成功 |
| `make-verify.log` | 6ドライバの公式ケースを各3回実行し全てAC。外側の終了値記録だけが空変数で失敗し、その失敗も末尾に保存 |
| `official-result-check.log` / `official-results.json` | 成功状態、対象6件、反復3回を独立に確認。`official-report.md`は当該測定の一覧 |
| `make-docs.log` | ページ生成後、局所測定を完全な公開用レポートとして扱えず公開検証が停止した初回ログ |
| `native-docs-initial.log` | 任意の保存済み検証ファイルが存在せずコピー時に停止。make docsには未到達 |
| `native-docs-publish-failure.log` | Linux領域でページ生成後、同じ局所測定レポートの公開検証で停止 |
| `native-docs.log` | 局所測定を別名保存した標準preflightモードでmake docsに成功。後続のサイト検証では当初のAPIページURLの誤りを検出 |
| `jekyll-default-env-failure.log` | 最初の開発環境ビルドで本番用解析タグがなく、サイト検証が停止 |
| `jekyll-cli-and-links-failure.log` | 通常のjekyll CLIではGitHub Pages既定プラグインの読込みが異なることと、API URLの誤りを確認 |
| `native-site-refresh-unavailable.log` | 終了済みの一時コピーを再参照できず、追加の更新試行には未到達。最終確認は次のWindows worktree側ログで実施 |
| `jekyll-preflight.log` | 成功。CIと同じGitHub Pages CLI・production設定でbuildし、check_site --without-metricsが5カテゴリ・113APIページで成功 |

preflightは全ライブラリのACを主張するものではない。
公開用の完全な測定・source-valid AC証跡の検証はCIで維持し、ローカルの部分測定で置き換えない。
