> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# Static Range Mode の候補統合と直接比較

## 比較対象

- `our.hpp`: 先行調査の rollback Mo、整数型の基数圧縮、2値以下／全異値向け経路。`baseline.hpp` は高速化前の旧版。
- `pr82.hpp`: [PR #82](https://github.com/blueberry1001/Blueberry-library/pull/82) の head `35c83bdb40115ab91c3fa7bd53c16ef5161c45e5`、対象Git blob `8f93a7e8137b7a5b462a752001a2343c8ad2a5c0`。一致した元workspaceファイルを読み取り専用でコピーした。固定端点・同一区間・重複query向け処理、rollback Mo。
- `combined-query.hpp`: 両者を組み合わせた候補。整数の基数圧縮・2値・全異値と、同一区間・少数種類の端点・重複queryを使用し、一般入力はPR #82のquery配列を用いたrollback Mo。copy-onlyまたはswap禁止型は値コピーと添字ソートで対応し、非const等値演算しか持たない型も扱う。同一区間の直接処理は、const要素での比較・等値判定が可能な型に限る。

実装はAPIの型・問い合わせ順序・空区間・非const等値・copy-onlyの互換性を維持する。全入力で最速とは限らず、最速提出との同条件比較も含まない。

## 環境・条件

Windows 11、Intel Family 6 Model 183、Windows Clang 18.0.3、`--target=x86_64-pc-windows-msvc -std=gnu++20 -O2 -DNDEBUG`。プロセスaffinityをCPU0 (`0x1`) に固定し、生成される子プロセスも継承する。ほかの論理CPUや物理コアの共有による負荷は測定していない。

`cross.cpp` は同じseed・型・配列・クエリで処理を生成し、入力生成とテストを除いたライブラリ呼び出し全体を測る。warmup 1回の後、実行順を回して各5回。頻度の全件checksumが各候補で一致し、各実行で最大24問い合わせを愚直計算でも確認する。全試行、フラグ、CPU mask、SHA-256、失敗は `results.json` に保持。数値は中央値ms。

| N=100,000 | Q | 入力 | 先行案 | PR #82 | 統合候補 |
| --- | ---: | --- | ---: | ---: | ---: |
| 一般 | 100,000 | 2値 | 2.746 | 74.004 | 2.701 |
| 一般 | 100,000 | 20値 | 51.651 | 49.297 | 45.872 |
| 一般 | 100,000 | 316値 | 53.036 | 51.357 | 47.065 |
| 一般 | 100,000 | 50,000値 | 77.232 | 78.087 | 68.769 |
| 一般 | 100,000 | 全異値 | 3.912 | 85.655 | 3.573 |
| 同一区間 | 100,000 | 20値 | 19.229 | 1.146 | 1.077 |
| 同一左端 | 100,000 | 50,000値 | 38.094 | 25.250 | 15.763 |
| 同一右端 | 100,000 | 50,000値 | 47.118 | 23.376 | 14.628 |
| 64区間の反復 | 100,000 | 50,000値 | 31.026 | 24.139 | 13.341 |
| 短い単一区間 | 1 | 50,000値 | 2.798 | 0.006 | 0.008 |
| 配列全域1件 | 1 | 50,000値 | 3.201 | 8.736 | 2.872 |
| 一般 | 100 | 50,000値 | 4.578 | 13.873 | 4.983 |

端点8／9種類、短い区間群、Q=1000、符号付き64bit値、string、Q=0を含む全24条件と各回はJSONを参照。Q=100やstringでは統合候補に小さな回帰が見られ、負荷によるばらつきと分離できていない。`results-unpinned.json` は最初のCPU非固定の測定、`results-fixed-first.json` はその後の3候補測定、`results-before-copyonly.json` はcopy-only追加前、`results-before-constexpr.json` は `if constexpr` 整理前の4候補測定で、最終結果と区別して保存した。非固定回のログとそのドライバも残す。

## 公式入力と検証

Library Checker problems `73876f1cbcf082d2b22722924b924d3d65a2f00e` の公式生成11入力について、PR #82と最終候補の出力を公式参照の最頻度および実際の区間内出現回数と照合し全件一致。`official-results.json` はCPU0固定、同一iostream driver、起動・I/O込み、warmup後3回。公式参照プログラム自体はscanf/printfであるため、その時間は両候補と直接比較できない。

| 公式入力 | PR #82 ms | 統合候補 ms |
| --- | ---: | ---: |
| random 3、N=Q=100,000 | 337.14 | 254.92 |
| random 4、N=Q=100,000 | 305.11 | 298.55 |
| random 5、N=Q=100,000 | 345.14 | 326.51 |

`verify-results.json` は Clang gnu++20／23、MSVC C++20／latest、各seed 1・2・17のテスト成功を記録する。`verify.cpp` は空・極値・bool・string、等値が非constのみ、copy-only+非const等値、NoSwap+非const等値、端点1／2／8／9／16種類、Q≫Nを含む。`verify-copy.cpp` はcopy-onlyとNoSwapのconst等値型について、全区間と同一区間100件を追加検査し、4構成の成功を `verify-copy-results.json` に記録。`strict-results.json` は非const等値を含む `verify.cpp` に対する Clang gnu++20／23 の `-Wall -Wextra -Werror` 成功を記録する。Linux GCCおよび公式checker自体は未実行。

最初の `verify.cpp` 準備とstrict runner生成で、PowerShellから渡したPythonの引用エラーが発生した。変更は行われず、`verify-prep-initial-failed.log` と `strict-prep-initial-failed.log` に残して修正した。

## 再実行

ファイル群を別の作業ディレクトリにコピーしてから `python run.py`、`python run-verify.py`、`python run-copy.py`、`python run-official.py` を実行する。runnerはWindows上のコンパイラ・SDK・固定問題リポジトリのパスを含むので、環境に応じて調整する。結果を上書きしないため、元の測定ディレクトリでは再実行しない。公式入力の一時データと実行バイナリは配布に含めない。
