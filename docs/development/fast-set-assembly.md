# FastSet の文字列構築と命令列

固定整数宇宙の集合を0/1文字列から構築する際、各文字の分岐と個別のbit更新を、64文字ごとのbitmap構築に置き換えました。GCC/Clangのx86ではAVX2とPOPCNTの両方に対応するCPUで32バイト比較とmovemaskを2回使い、いずれかに非対応ならSSE2、それも使えない環境では標準C++の処理に戻ります。`-mavx2` 等で全体を特定CPU向けにコンパイルした場合は、その指定に対応するCPUが必要です。公開API、計算量、入力の前提条件は変わりません。

## 上位実装の調査

2026-11-15時点のLibrary Checker公開順位では、Predecessor Problemの[提出403023](https://judge.yosupo.jp/submission/403023)が16ms、[提出278227](https://judge.yosupo.jp/submission/278227)が20msでした。どちらもAVX2の比較とmovemaskで0/1文字列を圧縮し、多段bitmapを使っています。提出403023のsource SHA-256は `59fd15abf763b5a59bb2069e1916e21ccfebbe291d116fb5240690cbce6357d7`、提出278227は `ef5b067f04c7686cce6a75ccb1426686d0a152b4afe6599f3ddeaeb39a90219a` です。取得した提出ソース内にライセンス表記は確認できなかったため、コードは流用せず方式だけを参考にしました。

順位の時間は提出プログラム全体の最大ケースであり、今回の構築・操作の個別測定とは条件が異なります。提出はI/Oにも専用実装を使っているため、順位だけから本体の速度差を算出しません。ACLに同等の固定宇宙predecessor集合はありません。

## 同条件の実測

Intel Core i7-14650HX、WSL2 Ubuntu、GCC 13.3.0とClang 18.1.3、`-std=gnu++20 -O2 -DNDEBUG`。各分布は入力を測定前に生成し、1000万文字の構築と60万操作を分けて測りました。1回のwarmup後、旧版・SSE2案・runtime AVX2案・葉優先探索案を順番を入れ替えて各3回実行しています。全checksumは一致しました。表は構築の中央値（ms）です。

| 初期文字列 | GCC 旧版 | GCC SSE2 | GCC AVX2 | Clang 旧版 | Clang SSE2 | Clang AVX2 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 全0 | 2.66 | 1.01 | 0.84 | 2.48 | 1.12 | 0.86 |
| 全1 | 12.05 | 1.38 | 1.04 | 13.16 | 1.14 | 0.98 |
| 半分が1 | 33.97 | 1.29 | 1.11 | 31.55 | 1.13 | 0.94 |
| 0.1%が1 | 2.78 | 1.16 | 1.08 | 2.88 | 1.08 | 0.98 |
| 256文字単位の塊 | 7.84 | 1.12 | 1.00 | 8.13 | 1.06 | 0.95 |

入力の生成、出力、破棄は構築時間に含めません。操作時間は同じJSONの `operations_ms` にあり、例えば半分が1のGCC版は旧5.23ms、AVX2案5.31msでした。葉優先探索案はGCCの一部で速く、Clangや分布によっては遅いため採用していません。

長さ63/64/65/256/1024については、同じ文字列の構築とnextを15万回繰り返しました。GCCで長さ1024は旧93.79ms、SSE2 16.81ms、AVX2 9.56ms。長さ63では旧8.79ms、SSE2 9.22ms、AVX2 9.32msでした。この小さい差には配置や負荷の影響も含まれます。Clangの長さ1024は旧92.30ms、SSE2 12.63ms、AVX2 9.93msです。

測定全体と各sample、checksum、memory、ソースhash、flagsは次のファイルにあります。`-march=native` は通常の `-O2` 測定とは混ぜず、同条件の別の比較として記録しています。

- [GCC結果](../../benchmark/results/fast-set-assembly-gcc.json)、[実行ログ](../../benchmark/results/fast-set-assembly-gcc.log)
- [Clang結果](../../benchmark/results/fast-set-assembly-clang.json)、[実行ログ](../../benchmark/results/fast-set-assembly-clang.log)
- [GCC native結果](../../benchmark/results/fast-set-assembly-gcc-native.json)、[実行ログ](../../benchmark/results/fast-set-assembly-gcc-native.log)
- [Clang native結果](../../benchmark/results/fast-set-assembly-clang-native.json)、[実行ログ](../../benchmark/results/fast-set-assembly-clang-native.log)
- [GCC小入力](../../benchmark/results/fast-set-small-gcc.json)、[Clang小入力](../../benchmark/results/fast-set-small-clang.json)

最初の小入力preflightは、runnerの出力実行ファイル名がinclude用ディレクトリと重なりリンクに失敗しました。測定値は生成されていません。失敗の[JSON](../../benchmark/results/fast-set-assembly-quick-failed.json)を残し、出力名を修正した後に再実行しました。

## 採用版の再測定とCPU判定

測定時のヘッダのSHA-256は `200748fd9bd66cf81b0316ee127f4d321f517e250ba25b8c3bedda7cc38e2654` です。AVX2とPOPCNTの両方を実行時に確認する変更後、同じ条件で実際の公開ヘッダを旧版・試作と再測定しました。半分が1の構築中央値は、GCCで旧34.18ms・採用版1.07ms、Clangで旧31.58ms・採用版0.97msです。対応する60万操作はGCCで旧5.23ms・採用版5.12ms、Clangで旧5.20ms・採用版5.03msでした。同一測定中の5分布すべてでchecksumが一致しています。

| 初期文字列 | GCC 旧版 / 採用版 | Clang 旧版 / 採用版 |
| --- | ---: | ---: |
| 全0 | 2.88 / 0.99ms | 2.39 / 0.81ms |
| 全1 | 12.29 / 1.13ms | 12.31 / 0.92ms |
| 半分が1 | 34.18 / 1.07ms | 31.58 / 0.97ms |
| 0.1%が1 | 2.85 / 1.00ms | 2.57 / 0.86ms |
| 256文字単位の塊 | 8.07 / 0.95ms | 7.42 / 0.80ms |

採用版の15万回の構築＋nextでは、GCCで長さ63が旧9.66ms・採用版10.00ms、長さ64が旧9.87ms・採用版3.79ms、長さ1024が旧90.92ms・採用版9.48msでした。Clangの長さ1024は旧94.74ms・採用版10.41msです。小入力と、GCCの疎な集合での操作時間には数%の変動があり、実行間の揺れやコード配置を含みます。

最終版の結果とログは [GCC](../../benchmark/results/fast-set-guarded-gcc.json)・[ログ](../../benchmark/results/fast-set-guarded-gcc.log)、[Clang](../../benchmark/results/fast-set-guarded-clang.json)・[ログ](../../benchmark/results/fast-set-guarded-clang.log)、[GCC native](../../benchmark/results/fast-set-guarded-gcc-native.json)・[ログ](../../benchmark/results/fast-set-guarded-gcc-native.log)、[Clang native](../../benchmark/results/fast-set-guarded-clang-native.json)・[ログ](../../benchmark/results/fast-set-guarded-clang-native.log) にあります。小入力は [GCC](../../benchmark/results/fast-set-small-guarded-gcc.json)・[ログ](../../benchmark/results/fast-set-small-guarded-gcc.log)、[Clang](../../benchmark/results/fast-set-small-guarded-clang.json)・[ログ](../../benchmark/results/fast-set-small-guarded-clang.log) です。POPCNT判定追加前の実測は `fast-set-final-*` として別に保存しました。

測定時の[最終版snapshot](../../benchmark/fast-set-assembly-guarded-final.hpp.txt)と、判定追加前の[ソースsnapshot](../../benchmark/fast-set-assembly-unguarded-final.hpp.txt)は、それぞれ記録済みSHA-256と一致することを確認して保存しました。測定後、`-Wshadow` 警告を解消するため、公開ヘッダの上位bitmap構築ループの変数を `i` から `j` に変更しました。公開ヘッダのGit正規化後のSHA-256は `e632e9c537a5406cd99a8716348a7e265d9f60c778fb7f30e028bd7c95154b1a` です。測定snapshotとの差はこの変数名と改行のみで、測定自体は変数名の変更前に行いました。測定ドライバ・snapshot・ログ・assemblyにはGitの改行正規化を適用せず、計測時のバイト列を保持します。

`-O2` の生成assemblyでは、GCC・ClangともAVX2 helperに `vpcmpeqb`、`vpmovmskb`、`popcntq` が現れ、SSE2側には `pcmpeqb`、`pmovmskb` が現れます。AVX2とPOPCNTの機能bitは別なので、実行時に両方を確認します。[命令抽出とhash](../../benchmark/results/fast-set-codegen-guarded/summary.json)と完全な圧縮assemblyは同じディレクトリに保存しています。

GCC/Clang、`gnu++20`/`gnu++23`、runtime判定、強制SSE2、強制scalar、AVX2のみを模擬、POPCNTのみを模擬、`-mavx2` の24構成で、それぞれ固定seed 1・2・17の[境界テスト](../../tests/random/fast-set-initialization.cpp)を通しました。機能bitの片方だけの状態はコンパイル時のbuiltin置換による模擬であり、該当する実機ではありません。[結果とログ](../../benchmark/results/fast-set-validation-guarded.json)に各コマンドと出力があります。
