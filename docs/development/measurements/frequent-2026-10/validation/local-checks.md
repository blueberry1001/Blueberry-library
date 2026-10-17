# 統合検証結果（2026-10-17）

Ubuntu 24.04 / WSL2、GCC 13.3.0、Python 3.12.3、既存venvで実行した。以下は正しさの検証であり、他の検証との並列実行を含む。経過時間を性能比較に使用しない。

## 成功結果

| 検証 | 結果 | ログ |
| --- | --- | --- |
| `make check`（既定GCC / gnu++20、retry 2） | exit 0。Python unit 147件、106ライブラリ文書・107実行例、公開106ヘッダ、143 verifyプログラムのcompile、random 50プログラム×20seed（1..20）がすべて成功 | `make-check-retry-2.log`、`check-retry-2-status.txt` |
| releaseの`make include-test`（`CXXFLAGS=-DNDEBUG`） | exit 0。公開106ヘッダの単独・重複includeがすべて成功、`Summary: 0 failure(s)` | `release-include-overlay.log`、`release-include-overlay-status.txt` |

`make check`のcompile段階とrandom段階は、それぞれ`Summary: 0 failure(s)`。unit結果は`Ran 147 tests in 98.489s / OK`、文書結果は`Checked 106 library docs and 107 executable examples.`。coverageの253公開問題の整合検査も成功した。

release include検証は、元リポジトリのignoredなroot `atcoder/`を変更せず、`/tmp/blueberry-release-include-20261017-sqwiw6zd`のコピーで実行した。全blueberryヘッダとcatalog・Makefile・check_cpp.py・templateassetの計111ファイルをbytesのまま複製し、コピー元とコピー先のSHA-256一致を`release-overlay-manifest.json`へ記録した。本体ヘッダは通常の`-I`、ACLだけは本リポジトリの`.deps/ac-library`を`-isystem`として指定した。本体のsystem header扱いや警告抑制はしていない。

## 失敗と対処の記録

1. 初回`make check`はunit 147件成功後、`library_verification.json`のstale判定で停止した。新設`tests/random/lca-boundary.cpp`の関連テスト一覧が未再生成だった。統合担当がcoverage生成物を更新した。原ログは`make-check.log`。初回のWSL引数渡しで末尾のexit記録が空になったため、その後は保存したshell scriptを実行してexit値を記録した。
2. `make check` retry 1はunit 147件・coverage整合検査成功後、FPS文書の主操作表が13行となり、上限12行の形式検査で停止した（exit 2）。統合担当がeval行を既存の追加操作表へ移した。原ログは`make-check-retry-1.log`、exit記録は`check-retry-1-status.txt`。
3. 元checkoutでのrelease include検証は11件失敗（exit 2）。rootのignoredな`atcoder/`コピーが通常の`-I`で先に解決され、ACL由来のNDEBUG時unused-variable警告が`-Werror`となった。本体の警告ではない。rootのコピーは公式verifyと共有中のため退避せず、上記のhash一致overlayで依存解決だけを分離して成功した。原ログは`release-include-test.log`、exit記録は`release-include-status.txt`。

この担当は本体・テスト・公開文書・生成coverageの修正をしていない。Git common configの別linked worktreeを指す`core.worktree`とCI用user設定も途中で確認し、統合担当が復旧した。unit testが原因とは断定していない。

## 範囲

この担当による追加実行はここで終了する。Library Checker公式実行、Clang/C++23・sanitizer、make docs/Jekyll/公開確認の結果は、各担当のログを参照する。この資料では実行済みと扱わない。
