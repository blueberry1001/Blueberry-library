> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# Static Range Mode: rollback Mo と小値域の高速化

## 対象と比較

- baselineはBlueberry main `546bd5a66416021e41c46f2ebde94bb9b199fe23` の `static-range-mode.hpp`（Git blob `6d0cd20a61bdd9c7180d0e5da74a1796444daebc`）。候補は同じ公開API・任意の比較可能型・空区間・代表位置の契約を維持する。
- 頻度別連結リストのMo法を、部分区間を一時的に追加して巻き戻す rollback Mo に変更した。クエリの偏りを踏まえ、幅を `max(1, floor(N/sqrt(max(1,Q))))` とする。圧縮後の全異値・定数・2値は一括処理する。整数型以外の座標圧縮は従来通り比較ベース。整数型は符号ビットを補正した安定基数ソートを使う。
- [maspypy の公開実装](https://maspypy.github.io/library/test/2_library_checker/data_structure/static_range_mode_query.test.cpp)と[公式参照解](https://github.com/yosupo06/library-checker-problems/blob/73876f1cbcf082d2b22722924b924d3d65a2f00e/data_structure/static_range_mode_query/sol/correct.cpp)の rollback Mo、[Nyaan Mo](https://github.com/NyaanNyaan/library/blob/master/misc/mo.hpp)のQに依存するブロック幅を調べ、独立に実装した。
- 2026-11-17 閲覧時の [Library Checker 最速提出 #403022](https://judge.yosupo.jp/submission/403022) は19 ms。オンライン問い合わせ構造で、値域・頻度に応じた処理の選択、AVX2と専用I/Oを使う。Blueberryの汎用・一括APIと契約が異なり、この数値は下記のWindows測定と比較できない。同一環境で最速提出を実行した比較は未実施であり、最速を保証しない。

## 環境

Windows 11 10.0.26200、Intel Family 6 Model 183 Stepping 1、論理24 CPU、Windows Clang 18.0.3 または MSVC 19.43.34809、C++20、`-O2 -DNDEBUG` 相当。CPU固定なし、同時実行の影響を分離していない。実行間の小差を優劣と断定しない。各候補は同一入力で順番を回し、warmup後5回の中央値。

## 本体の実験

`results2.json` はコンパイルコマンド・全試行・checksumを保持。`driver2.cpp` は入力生成を計時外とし、圧縮・クエリ処理・返却vector構築を計時内とする。`rollback-radix` は一般入力向けの原型で、採用版はこれに空Q・全異値・1値・2値の経路を加えたもの。以下の分布は特殊経路の対象外である。

| 入力（N=50,000） | 実行環境 | 従来 ms | rollback Mo ms | rollback Mo + radix ms |
| --- | --- | ---: | ---: | ---: |
| Q=100、値一様 | Clang | 8.448 | 4.237 | 1.486 |
| Q=3,000、値一様 | Clang | 24.789 | 6.970 | 4.111 |
| Q=50,000、値一様 | Clang | 42.619 | 20.550 | 17.327 |
| Q=200,000、値一様 | Clang | 105.651 | 58.088 | 49.487 |
| Q=50,000、値一様 | MSVC | 48.533 | 24.605 | 19.806 |
| Q=50,000、20値 | MSVC | 53.539 | 14.686 | 14.159 |

比較候補として元の頻度別リストのまま可変幅にした案、Nyaanの幅定数を取り入れた案、比較時のブロック番号を事前計算する案も残している。`results.json` は初回、`results2.json` は独立した後続測定であり、異なる回の数値を混ぜない。

## 公式入力と同一I/O比較

固定したLibrary Checker problems commit `73876f1cbcf082d2b22722924b924d3d65a2f00e` の公式generator全11入力で、従来版と採用版の出力を公式解の最頻度および区間内の実際の出現数と照合し一致。個々の最頻値は同率で異なってよい。`official-results.json` に入力SHA-256・各回・終了状態を記録。プロセス起動・入力・出力を含む。

公式参照解は元々scanf/printfなので、そのままの数値とiostreamのBlueberryを実装速度として比較しない。`run-reference-sameio.py` は公式参照解の入出力だけをBlueberry driverと同じiostreamに差し替え、最大サイズのrandom 3・4・5を同一入力・順序を回した5回で測る。入力SHA-256が前述の検証と一致し、参照解の出力は元の公式参照解と完全一致することも確認した。

| 公式入力 | 従来 ms | 採用版 ms | 公式参照・同一I/O ms |
| --- | ---: | ---: | ---: |
| random 3 (N=Q=100,000、2値) | 240.68 | 151.00 | 208.19 |
| random 4 (N=Q=100,000) | 271.36 | 190.32 | 205.78 |
| random 5 (N=Q=100,000) | 309.79 | 206.76 | 219.88 |

すべてWindows Clang、同一フラグ、入出力・起動込み。測定器と参照解のI/Oを変えただけであり、最速提出との比較ではない。I/Oと起動が占める比率が大きい。メモリは測定していない。

## 正しさと失敗記録

`new-test.cpp` はseed 1・2・17についてWindows Clang 20/23、MSVC 20/latestで成功（`new-tests.json`）。N=0、Q=0、同じブロックの空・長区間、Q≫N、int64/uint64極値、bool、string、`<`と`==`だけを持つ型、代表位置が区間外でもよい場合を含む。`checks.json` は候補試作について別の固定seedでも検査した記録。Linux GCC/Clangと公式checker自体はここでは未実行。

初回公式runnerは生成した `params.h` のinclude guard不足でコンパイル失敗した。`official-results-initial-failed.json` に失敗を保持し、修正後に実行した。参照解の同一I/O runnerも初回はWindows既定文字コードによるUTF-8読込エラーとなり、`sameio-initial-failed.log` に保持した。失敗を測定値や成功として扱わない。

## 再実行

WindowsのVisual Studio・SDK・Clangパスを各 `run*.py` に記載している。ログを上書きしないよう本ディレクトリを新しい作業用ディレクトリへコピーしてから、`python run2.py`、`python run-newtest.py`、`python run-official.py`、`python run-reference-sameio.py` を実行する。公式runnerは固定コミットの問題リポジトリを `.verification/library-checker-problems` に必要とする。ディレクトリを別の場所にコピーした場合はrunner内のパスを調整する。
