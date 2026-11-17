# 整数k乗根の高速化

uint64全域・正のkに対するfloorを整数演算だけで厳密に返す契約を維持し、k=2〜8は整数Newton法、それ以外はbit幅で探索範囲を絞る。GCC/Clangの大きなkでは乗算overflow検出と二乗法を使い、それ以外のcompilerでは除算で積を保護する。

Newton法の初期値は `2^ceil(bit_width(n)/k)`。k=2〜8では初期値の `k-1` 乗は最大でも `2^60` で、反復中は値が増えないためuint64の範囲に収まる。次の値が減らなくなったときの値は整数k乗根である。kがbit幅以上なら、n>1の場合の答えは1と分かる。

## 比較対象と条件

- 既存実装、候補、[NyaanのCC0実装](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/math/kth-root-integral.hpp)、[ei1333の実装](https://ei1333.github.io/library/math/number-theory/kth-root-integer.hpp.html)が使う固定32bit試行を同条件で比較した。前者は浮動小数点で初期値を推定し、既存APIの「整数演算のみ」と条件が異なる。後者は公開された方式の独立試作であり、bit幅を絞らず32回試す。
- Library Checker公開順位の現時点の最速提出は取得できず、最速提出との比較は未実施。既知の2つのライブラリとの比較を最速保証とみなさない。
- Windows 11、Intel Family 6 Model 183、MSVC 14.43.34808およびWindows Clang 18、gnu++20相当の最適化 `-O2 -DNDEBUG`。CPU固定なし、他セッションの処理が干渉する可能性がある。
- 本体のみの比較は入力を生成後に操作区間を計時し、各ケースをwarmup1回＋5回ずつ候補の順序をずらして実行した。入力・出力・構築は計時外。
- 実行全体の比較は、固定した Library Checker problems commit `73876f1cbcf082d2b22722924b924d3d65a2f00e` の公式generatorと公式正解を使用し、500,000件規模の公式ケースを測った。起動・入力・出力・終了を含み、warmup1回＋3回の中央値。Windows Clang・同一flags・同一入力である。メモリ使用量は未測定。
- Windows全体のCPU負荷を計測した試作には、100%超の不整合10件と欠損7件があり、負荷値として採用しない。元ログは残す。CPUの同一コアでの干渉は不明なため、小さな差を確定的な優劣に使わない。

## 実ヘッダの操作時間

20万件ずつの入力について、実公開ヘッダと旧ヘッダを同じdriverで測った中央値。k=2とk=3はいずれも64bit一様乱数である。値はmsで、NyaanのWindows MSVC版は `__int128` が使えないため未測定。

| コンパイラ・入力 | 既存 | 最終 | Nyaan |
| --- | ---: | ---: | ---: |
| MSVC k=2 | 44.010 | 3.223 | — |
| MSVC k=3 | 30.449 | 4.020 | — |
| Clang k=2 | 36.605 | 3.160 | 5.530 |
| Clang k=3 | 25.540 | 4.243 | 5.504 |
| Clang k=9 | 10.427 | 7.333 | 8.924 |
| Clang k=16 | 5.544 | 3.217 | 9.203 |

11種類の分布と各回の値・コマンドは `final-results.json` に保存した。測定はCPU固定なし。Windows全体の負荷の有効値中央値はMSVC測定22.5%、Clang測定20.0%で、測定対象自身の負荷も含む。Clang測定には負荷値の欠損または不整合が1件ある。細かい差は干渉と分離できない。

## 公式入力での実行全体

| 入力 | 既存 ms | 最終 ms | Nyaan ms |
| --- | ---: | ---: | ---: |
| all_k2-0 | 918.18 | 788.16 | 823.72 |
| all_k2-1 | 888.06 | 732.18 | 754.08 |
| all_k3-0 | 715.40 | 688.83 | 719.72 |
| all_k3_2-0 | 849.83 | 772.09 | 832.18 |
| near_border-0 | 573.30 | 569.55 | 601.23 |
| random-1 | 632.09 | 615.77 | 651.39 |

測定には入出力とプロセス起動を含む。15ケースの全sample、入力と公式解のSHA-256、実行コマンド、失敗の有無は `official-results.json` に保存した。near_border-1 や near_border_2-1 は最終版がわずかに遅く、負荷や誤差と分離できていない。

## 正しさ

追加した境界・乱択比較では、MSVC C++20/C++latest と Windows Clang gnu++20/gnu++23 の各構成で、0〜32767とk=1〜70、完全冪の前後、符号なし64bit最大付近、固定seed 200,000件など計2,562,903件を旧版と照合した。各構成で最終ヘッダを含め全候補が一致した。公式15入力は既存実装・最終版・Nyaanの出力が、公式正解の出力とすべて一致した。

新規の `tests/random/kth-root-integer-boundaries.cpp` 自体も4構成とseed 1、2、17で実行し、すべて成功した。コマンド・終了コード・所要時間は `boundary-results.json` に保存した。

`verify-results.json` と `official-results.json` はWindows上の記録で、Linux GCC/Clang、sanitizer、全ライブラリの `make test` / `make verify` / `make docs` は含まない。失敗や未測定を成功扱いにはしない。

## 再現

- `baseline.hpp`、`candidate.hpp`、`nyaan.hpp` はそれぞれ比較対象の原本。raw SHA-256は `ad2c46419f39959f9f40b5be1ac476631a4f8e108ae3275846dcc6943e4e077b`、`9edcf7550a10e192956a0dd5d73854ba661fff67751787d94eb37fef046bc448`、`86a5f9f2b247bf0c67af8f00980236f9556b0fe053f50d942b9ed163527e5bef`。Nyaan原本のライセンスを `nyaan-LICENSE.txt` に保存した。
- 初回〜第3回の候補・runner・全記録は `benchmark-*.cpp.txt`、`run-*.py.txt`、`results-*.json` に保存。前述の負荷記録の不整合もそのまま保持する。
- `python docs/development/measurements/kth-root-20261117/replay.py candidates` で候補比較、`final` で実ヘッダの操作時間、`validate` で境界比較、`official` で公式入力生成と出力比較を再実行する。WindowsのVisual Studio・SDK・Clangのパスは各runner内で指定しており、異なる環境では変更が必要。`official` には固定版の問題リポジトリを `.verification/library-checker-problems` に用意する。毎回別の `.build/kth-root-replay-*` を作り、既存ログを上書きしない。
