# Template compatibilityの検証記録

基準は`f9ccb0f704ee47974b372062684b4295bdc0ebea`。初回の変更後ヘッダは実装commit `caa2911364baddf684b33076f85d6b486068b718`を用いる。

- `results.json`: GCC13.3 / Clang18.1、GNU20 `-O2 -DNDEBUG -S`、同じMany A+Bの64/128bitコードのassembly比較。全4件が完全一致した。
- `closure-results.json`: 自作ヘッダのinclude閉包のコード量。CRLFをLFに正規化し、ACL・標準ライブラリは除外した。
- `commands.txt`: assembly比較時のコマンド。`git show HEAD`のHEADは上記基準revisionであり、再現時はそのSHAを指定する。
- `run.py.snapshot`、`closure.py.snapshot`、`driver.cpp.snapshot`、`baseline-fast-io.hpp.snapshot`: 実行時のスクリプト・比較コード・変更前ヘッダの保存原本。`.snapshot`はドキュメント生成の実行対象にしないための拡張子。
- `compatibility-matrix.log`: 16構成の互換性、8構成のinclude順序、3入口の依存閉包の独立確認。
- `ubsan.log`: 独立したUBSan確認。後続の最終fixtureには空テーブル等の境界ケースも追加した。
- `compatibility-sanitizers.log`: 空の篩・組合せ表と再初期化も含む最終fixtureを、両テンプレート・`BLUEBERRY`定義時にASan+UBSanで実行した結果。
- `docs-examples-initial-warning.log`と`docs-examples-rerun.log`: 最初に検出したコンパイラ警告と、修正後の両ページ・4コンパイラ構成の確認。失敗を省略していない。
- `check-initial-coverage-failure.log`: 152 unit test成功後、再生成前の対応表を検出して失敗した初回`make check`。この実行全体を成功として扱わない。
- `check-rerun.log`: 修正後のmake check、release include-test、公式I/O verify 6本×3回、make docs、production Jekyll/check_siteの成功ログ。
- `verification/`: 公式6本の範囲限定レポートと18回の反復JSON。全verifyを実行した結果としては扱わない。
- `validation-sources.json`: ローカル検証cloneと実装commit `caa2911364baddf684b33076f85d6b486068b718`の変更ソース12ファイルのLF換算hash一致。

`results.json`の変更後ヘッダの生byte数・SHAはWindowsのCRLF checkoutで取得した。
LF換算のbyte数・SHAは`closure-results.json`にあり、両者を同じファイル表現のhashとして比較しない。
スクリプトの作業パスは実行当時のもので、再実行時はROOT/OUTを手元のパスに合わせる。
生成assemblyの一致は、今回の短縮でFast I/Oのrelease処理が変わっていないことの確認であり、実行時間の改善率ではない。
ローカルverifyは基準HEADへ変更ファイルを重ねたcloneで実行したため、生のレポートのrevisionは基準HEADを示す。実際の変更後ソースは`validation-sources.json`で区別しており、未変更のf9ccb0fの測定や、生成コード比較の速度測定として利用しない。

`comparator-followup/`はa88f9f7からの比較関数判定の追補。初回記録とは分離し、修正前header/reproducer、source hash、16構成の再実行ログを保存する。`operator() &`と明示的bool変換の返り値を回帰テストへ加えた。
