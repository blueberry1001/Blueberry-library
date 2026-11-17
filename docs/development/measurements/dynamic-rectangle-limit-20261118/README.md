# 動的 2D Fenwick の制限時間点検

2026-11-18。`verify/data-structure/dynamic-rectangle-sum.test.cpp` は、事前登録の不要なオンライン 2D Fenwick に静的 Rectangle Sum の点を追加し、矩形和を問い合わせる。公式問題の制限時間 5 秒に対してローカル測定の余裕が小さかったため、同じソースと入力で再測定した。ACL に未知座標の点加算・矩形和を扱う同等 API はない。

測定はいずれも revision `179d99a39514b3321a163218273b394cdf7e1203`、WSL 2、Intel Core i7-14650HX、GCC 13.3、`-std=gnu++20 -O2 -pipe -Wall -Wextra -Wshadow -D_GLIBCXX_ASSERTIONS`、直列実行。初回の Library Checker 問題リポジトリは `73876f1cbcf082d2b22722924b924d3d65a2f00e`、両回の入力と期待出力の SHA-256 は `fc30ff14ff46358b69d772613533ce5933753b0a38fda80d0ab2823948127eb4` で一致した。再測定時の問題リポジトリ revision は測定ログに直接記録していない。各回の公式ケースの最大時間と全ケースの実行時間合計は次の通り。単位は秒で、実行時間にはプロセス起動と I/O を含み、コンパイルとチェッカーは含まない。

| 記録 | 最大ケース時間の各回 | 中央値 | 全ケース合計の各回 | 中央値 |
| --- | --- | ---: | --- | ---: |
| 全件検証時 | 4.960、4.867、4.745 | 4.867 | 34.701、33.746、33.601 | 33.746 |
| 直接再測定 | 6.323、6.069、6.179 | 6.179 | 44.708、44.410、43.865 | 44.410 |

同じコード・入力でも大きく変動しており、この差の原因は切り分けていない。初回の最大ケースは `max_random_00`、`max_random_00`、`max_random_02`、再測定はそれぞれ `max_random_00`、`max_random_02`、`max_random_01`。各最大ケースのメモリ使用量は約 209〜211 MB。ローカル環境と提出先では CPU や実行条件が異なるため、5 秒を上回った測定から公式提出の TLE を断定しない。

初回の `initial.json` は時間予算で終了した全件検証から、このドライバの結果だけを抜粋した記録である。ドライバ自体は 3 回とも AC だが、元レポート全体の `succeeded` は未実行の別対象があるため false のまま保存した。未実行の 91 対象は、同じ revision・コンパイラ・3 回の条件で別出力に再開し、全 171 対象の AC ログを照合した。全件の元レポートは `full-verify-initial.json` と `full-verify-resume.json`、照合手順は `audit-full-reports.py.txt` に保存した。ケース別ログは実行元の WSL checkout の `.verification/run-*` と `.verification/resume-20261117/run-*` に残しているため、全件のログ照合にはその checkout が必要である。

`recheck.json` は同じ `Recorder`・公式ケース・チェッカーを直接呼び出した 3 回の独立した再測定である。どちらの測定も、座標範囲を `10^9+1` に広げる修正前のドライバを対象としており、修正後の性能は直接測っていない。全対象の依存関係解析が長時間続いた前段の試行は、測定開始前に停止し `interrupted-preparation.log` に残した。

`initial-run-*.json` と `recheck-run-*.json` は各回のケース別生ログ、`screening.json` は問題の `info.toml` と制限時間の照合結果、`checker-sha256.txt` は使用したチェッカーバイナリのハッシュである。`verifier-original.cpp.txt` と `recheck-direct.py.txt` は測定時のドライバと再測定方法を保存する。公開ヘッダの変更案と異なる入力分布での採否は [探索共有の比較](../dynamic-rectangle-sum-study-20261118/README.md) に分けて記録した。
