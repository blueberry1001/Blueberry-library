> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# GCD / LCM 畳み込み：2つの改善案の比較

同時に進行した [PR #83](https://github.com/blueberry1001/Blueberry-library/pull/83) と、[先行調査]({{ '/docs/development/measurements/divisor-convolution-20261117/' | relative_url }})の案を同一条件で比較した。PR #83 の head は `bf28f6a0928df10a4f0357a3242e89ec89b7ae7b`、対象ヘッダの Git blob は `0d5d326081d35308c0be6ba9cc8b0582b14c2041`。`pr83-original.hpp` はその原本で、`pr83-renamed.hpp` は同一バイナリで比較するため namespace だけを変更したもの。

PR #83 は2入力のゼータ変換を融合し、逆変換で再び篩を走査する。先行案は2入力の変換時に素数列を保存し逆変換で再利用するが、小入力では元の3変換に戻していた。組合せ案 `hybrid.hpp` は入力長1024未満で #83、1024以上で素数列の再利用を使う。一般の環 T、添字0、入力・返却値、単独の約数・倍数変換の仕様は維持する。

## 同一条件の操作時間

Windows 11、Intel Family 6 Model 183、論理 CPU 4 に固定し、子プロセスの affinity mask `0x10` も確認した。論理 CPU 5 は同じ物理コアの SMT sibling で、OS や他処理の干渉・周波数変動は除外できない。MSVC 14.43 は `/std:c++20 /O2 /DNDEBUG`、Windows Clang 18 は `--target=x86_64-pc-windows-msvc -std=gnu++20 -O2 -DNDEBUG`。実装を同じ実行ファイルに入れ、入力を固定し、warmup 1 回の後に実行順を交代して各12回測った中央値。vector のコピー・確保・変換・積・checksum を含み、入力生成と標準入出力は含まない。N は vector の長さで、表の時間は記載した回数の合計。

| コンパイラ・型・操作・N | 回数 | #83 | 組合せ |
| --- | ---: | ---: | ---: |
| Clang modint GCD 512 | 6000 | 24.872 ms | 25.014 ms |
| Clang modint GCD 4096 | 500 | 17.253 ms | 14.159 ms |
| Clang modint GCD 65,536 | 12 | 9.817 ms | 8.089 ms |
| Clang modint GCD 1,000,001 | 2 | 38.739 ms | 34.593 ms |
| Clang modint LCM 1,000,001 | 2 | 42.189 ms | 38.590 ms |
| MSVC modint LCM 4096 | 500 | 57.555 ms | 47.344 ms |
| MSVC modint GCD 1024 | 3000 | 24.650 ms | 25.802 ms |
| MSVC modint GCD 1,000,001 | 2 | 103.475 ms | 100.728 ms |
| Clang uint64 GCD 65,536 | 12 | 11.137 ms | 8.335 ms |
| Clang uint64 LCM 1,000,001 | 2 | 56.519 ms | 53.434 ms |

modint は 998244353 の同一実装、uint64 は 2^64 剰余の unsigned 演算。小Nには回帰した中央値もあり、差がサンプルの振れと重なるものは優劣を確定しない。`focused-results.json` は採用版の各回・checksum・コマンドを記録する。`comparison-results.json` は前提 assert を削除する前の旧版・先行案・#83・組合せを同一バイナリに入れた warmup 後8回のバランス順序比較。`comparison-pinned-results.json` は組合せ前のCPU固定試行、`comparison-unpinned-results.json` は固定前の試行で、順序が完全には均等でないため比較判断は後続試行を優先する。各試行のドライバとヘッダの snapshot を保存している。

閾値を1024または4096にした版も同一バイナリ・CPU4固定で比較した。MSVC modint GCD N=2048 は #83 37.905 ms / 閾値1024版43.608 ms / 閾値4096版37.832 ms（各1200回）と1024版の回帰を観測した。一方、Clang modint GCD では 21.604 / 18.713 / 21.187 ms、MSVC uint64 GCD では 18.514 / 16.904 / 18.942 ms、MSVC modint LCM では 44.794 / 34.552 / 44.367 ms だった。後続の別ドライバでは MSVC modint GCD の #83 と1024版が 35.895 / 31.665 ms と逆転したため、コード配置や負荷に左右される可能性がある。単一の1024を採用し、この回帰の可能性は残す。`threshold-results.json` に各回を記録する。

小入力の素数列に reserve する案も調べたが、uint64 GCD N=4095（各500回）で予約なし→ありは MSVC 18.207→23.330 ms、Clang 17.849→23.780 ms と両方で回帰したため不採用とした。`reserve-results.json` と snapshot を残す。

公式 `max_random_00` を読み、同じ実行ファイルから scanf / printf で出力する実行全体は、CPU4固定・8回中央値で GCD 411.692→410.195 ms、LCM 408.946→402.678 ms。測定範囲は GCD #83 401.950〜427.527 ms / 組合せ 395.452〜420.550 ms、LCM #83 399.168〜433.650 ms / 組合せ 391.213〜426.979 ms で、いずれも差を明確にできない。`e2e-results.json` に全回を保存する。

同入力のメモリ比較では、Windows `GetProcessMemoryInfo` の peak working set は GCD で #83 約19.81 MB、組合せ約21.35 MB、差は約1.54 MBだった。各4回、出力は null、約2ms間隔の監視で OS のピークカウンタを取得した。素数列の確保・allocator・ページ粒度が含まれ、独立した時間測定ではない。各回と peak pagefile 使用量は `memory-results.json` にある。

[Library Checker 最速 #409724](https://judge.yosupo.jp/submission/409724) は固定 mod・入力上限・AVX2・専用 I/O の実装で、上記の一般型 API と条件が違う。出典と処理の違いは先行調査に記録した。上表をその提出時間と直接比較できず、全環境で最速とは断定できない。

## 正しさ・再現

固定 problems commit `73876f1cbcf082d2b22722924b924d3d65a2f00e` で GCD / LCM 各29ケースを生成し、入力と公式 `correct.cpp`・旧版・先行案・#83・組合せの各出力を公式 hash と照合、全58ケースが一致した。Windows の改行は LF に正規化。コマンド・終了コード・hash は `official-results.json` に保存する。

非可換な 2×2 行列環と、入力長1023・1024・1025の切替境界を含む愚直比較を、#83 と組合せの両方について MSVC C++20 / C++latest、Clang gnu++20 / gnu++23、seed 1・2・17 で実行した。assert 有効と `NDEBUG` の両方が成功した。`matrix-results.json` と `matrix-debug-results.json` に記録する。Linux GCC / Clang と sanitizer はローカル未実行。

利用者の前提を繰り返す assert 2箇所と未使用 `<cassert>` を削除した前後で、Windows Clang 18 `-O2 -DNDEBUG` の GCD / LCM × modint / uint64 の生成アセンブリは7517行で完全一致した。`assert-assembly-results.json` と `assert-before.s.gz` / `assert-after.s.gz` を保存。削除前の `*-pre-assert-removal.*` と全ログも残し、記録を異なる revision の一測定として混ぜない。

`python docs/development/measurements/divisor-convolution-followup-20261117/replay.py` は `.build` 下に時刻付きコピーを作り、性能・行列・公式ケース・入出力込み・メモリの計測を行う。元の記録は上書きしない。Windows の Visual Studio・SDK・Clang のパスは各 runner 内に明記し、CPU4に固定できない場合は失敗を記録する。公式問題 repo は `LIBRARY_CHECKER_PROBLEMS` 環境変数または runner の既定パスに置く。`SHA256SUMS` は各記録・snapshot の生バイトを対象にする。
