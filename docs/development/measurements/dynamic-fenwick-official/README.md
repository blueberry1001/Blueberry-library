# Dynamic Fenwick Tree official-case verification

2026-11-14、作業ツリーの変更を含む `123365f2738e3d883c566def538e2833f411c317` 上で
`verify/data-structure/dynamic-point-add-range-sum.test.cpp` を `make verify`、
`VERIFY_REPEATS=1` で実行した。出力先はそれぞれ `.build/dynamic-fenwick-verify` と
`.build/dynamic-fenwick-verify-retry` を指定し、ここへコピーした。

最初の実行は起動環境の PATH に `oj` がなく、ケース取得時に `FileNotFoundError: oj` で失敗した。
その[結果](initial/report.md)を残す。`/home/blueberry/.local/bin` を PATH に追加した
[再実行](retry/report.md)では、Library Checker Point Add Range Sum の全21ケースで
公式checkerによる正答判定が成功した。ケース別ログは `retry/run-mahiqdco/` に保存した。

GCC 13.3、gnu++20、assert有効。再実行中は別のテストも動いていたため、記録した時間は
性能比較に使わない。AtCoder ABC484 Eへの再提出や判定は行っていない。

release include確認では最初、`-DNDEBUG -Werror` によりACLの未使用変数警告で16ヘッダが
失敗した（[初回ログ](release-include-initial.log)）。ACLを `-isystem` にしても、
作業ディレクトリ内の別の `atcoder/` が先に読まれたため同様に失敗した。
そのディレクトリを変更せず、一時コピーから除外し、ACLを `-isystem` で指定した
再実行では全ヘッダが成功した。対象ヘッダのコピーと作業ツリーのSHA-256はどちらも
`fe2003e589185db92216e44e4452b78080fa49744dfcb8b9ca4a42103c15501b`。

全verifyを1回ずつ実行した別の[初回記録](full-first/report.md)は、時間予算900秒の終了時点で
26件が成功し、143件が未実行だった。未実行分を成功と数えず、再開結果は別に残す。
別テストと並行していたため、この記録の時間は性能比較に使わない。

実際のCI性能ゲートと同じコマンドは、同じマシンの静穏時に成功した。
[各試行](ci-gate/measurements.json)、[判定](ci-gate/assessment.json)、
[実行ログ](ci-gate/benchmark.log)を保存している。

PRの初回CIではGCC 15の20/23両構成で、別ライブラリの `extra-fast-wavelet-copy.cpp` の
割り当て失敗を固定回数期待するassertが失敗した（[実行記録](https://github.com/blueberry1001/Blueberry-library/actions/runs/43608500133)）。
例外注入を実際に観測した割り当てに対して順に行うテストへ修正し、GCC 13・Clang 18の
gnu++20/23・O0/O2/O3で成功した。GCC 15での最終結果はPRの再実行で確認する。
