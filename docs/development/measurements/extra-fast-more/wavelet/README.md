---
title: "Wavelet Matrix の高速化測定"
---

# Wavelet Matrix の高速化測定

2026-10-20。通常版を置き換えず、`extra_fast::WaveletMatrix<T>` を個別に使う比較です。
上位提出の処理順や値域の制約を、オンラインAPIの高速化と混同しないよう分けています。

## 調査

[research.json](research.json) にLibrary Checker REST APIの取得結果・提出ソースのSHA-256を保存しています。
第三者のソースそのものは公開測定フォルダへ転載せず、ローカルの `.benchmark/extra-fast-more/wavelet/` にだけ保存しました。
提出の表示時間はLibrary Checkerの最大ケース時間であり、下のローカル測定とは比較できません。

| 提出 | 主な設計 | 今回との契約の違い |
| --- | --- | --- |
| [403008](https://judge.yosupo.jp/submission/403008) / [403000](https://judge.yosupo.jp/submission/403000) | radix/狭い値域のoffset圧縮、AVX2 stable partition・prefix構築・問い合わせfrontier | 全問い合わせを先読み。段ごとに問い合わせを処理し、前の答えに依存できない |
| [393928](https://judge.yosupo.jp/submission/393928) | bitsとprefix rankの同居、branchless batch kth | 主な解答はbatch、圧縮対象はuint32 |
| [376681](https://judge.yosupo.jp/submission/376681) | rank block、branchless batch kth、16bit radix、mmap/table I/O | value/indexを固定幅でpackし、Nの上限に依存 |
| [362490](https://judge.yosupo.jp/submission/362490) / [393938](https://judge.yosupo.jp/submission/393938) / [258498](https://judge.yosupo.jp/submission/258498) | 頻度専用hashとoffline prefix sweep | k番目や閾値未満の個数を扱わない。全問い合わせ先読み |

同じアルゴリズム上の工夫を独立に実装しました。最終headerは241行です。既存の符号付き/unsigned整数・自動圧縮・オンライン問い合わせを保ち、
rankのbitsと累積個数を1blockに置きます。bit構築とpartitionの分岐を減らし、k番目の移動をmaskで選びます。
64要素ずつbit/prefixを組み立て、最終段の不要なpartitionを省きます。
値域の幅がN未満なら既存のN要素bufferを順位表に使い、穴を詰めて圧縮します。
値域の幅で段数を増やさず、オンライン操作のO(log σ)を維持します。
x86-64のGCC/ClangではCPUを検出し、対応時だけinline asmのPOPCNTを使います。
`use_popcnt=false` は標準C++経路で、コンパイラ全体の `-march=native` が生成する命令を禁止する意味ではありません。
追加の `kth_smallest_batch` はオンラインAPIから分け、問い合わせ順を制約できるときに使います。
共通の `BLUEBERRY_EXTRA_FAST_FORCE_SCALAR` を扱い、コピー代入は一時コピーを作ってから交換する強い例外保証です。

## 測定条件

- WSL2、CPU 0固定。compiler/version、flags、OS、CPU情報、revision、入力/SHAは各 `metadata.json` に記録します。
  他プロセスやOSの負荷までは固定できません。compiler間の絶対速度差を性能差として解釈しません。
  測定時HEADに未commitの追加・変更を重ねているため、測定内容は各source SHAと同梱snapshotで特定します。
  異なるfolderの時間を、source/harness/storageの差を無視して変更前後として比較しません。
- 最終測定では入力・出力・実行ファイルをLinuxの `/tmp` に置きます。
  初期pilotはNTFS上の入力/実行ファイルとpipe出力だったため、pilotと後続実験の絶対時間は直接比較しません。
- 固定seed 20261020。smallはN=1024/Q=50000/短区間、wideはN=200000/Q=100000/値域10^9、
  duplicatesはN=200000/Q=100000/値域16、few-queryはN=200000/Q=1000/値域10^9です。
- phase測定の要素型は両実装とも `long long`。同じ入力をparseし、構築、操作、formatを独立に測ります。
  kth・完全一致count・閾値未満の個数を別々に処理します。queryの入力形式は測定用にl/r/k/xの4列です。
- onlineは通常版、新版のPOPCNT、同じ新版の標準C++経路を比較します。batchはcopy/作業領域/答えvectorの確保を操作時間に含めます。
  新版の全量batchと通常版のonlineは使い方が異なるため、同一APIの改善率には含めません。
- whole測定はLCの通常の3列kth入力で、両自作実装を `int` 型で使います。
  独立したpersistent segment tree oracleの出力を整数列へ正規化し、SHAと照合します。
  上位提出には先頭0付きの固定桁出力があるため、生byte列のSHAも別に保存します。第三者提出は原本のI/O、batch処理、固定制約を維持するため、
  wholeの差を木だけの差とは解釈しません。
- rank blockは一般的な64bit環境で16byteで、通常版の64bit bits＋32bit prefixの分離配列よりメタデータが約1/3増えます。
  メモリは入力・答え・ランタイムも含むピークで、構造単体の保持容量とは異なります。
  final phaseの `rss_kib` は `/proc/self/status` のVmHWMです。pilot-resumeは `getrusage(RUSAGE_SELF).ru_maxrss` が
  親Pythonの高水位を引き継いで全ケース42248KiBとなっており、メモリ比較には利用できません。時間・出力の比較には影響しません。

## 保存した実験

| フォルダ | 状態・目的 |
| --- | --- |
| [pilot](pilot/RUNNER-ERROR.txt) | runnerの基準path誤りでcompiler/測定の前に終了。測定値なし |
| [pilot-resume](pilot-resume/summary.json) | 初期候補、GCC/Clang×O2 release/native O3、3反復、480 samples全一致 |
| [control-pilot](control-pilot/summary.json) | /tmp・VmHWM・API整理後の基準。native O3、3反復、240 samples全一致 |
| [skip-last-pilot](skip-last-pilot/summary.json) | 最終段のpartition省略。96 samples全一致 |
| [word-build-pilot](word-build-pilot/summary.json) | 64要素ずつbit/prefixを構築。96 samples全一致 |
| [radix16-pilot](radix16-pilot/summary.json) | 16bit radix。96 samples全一致。小配列の構築回帰等により棄却 |
| [dense-encode-pilot](dense-encode-pilot/summary.json) | 狭い値域で既存bufferを順位表に再利用。96 samples全一致 |
| [branchless-count-pilot](branchless-count-pilot/summary.json) | count/range_freqのmask化。144 samples全一致。回帰が多く棄却 |
| [early-count-pilot](early-count-pilot/summary.json) | count/range_freqで区間が空になれば早期終了。144 samples全一致。回帰を伴い棄却 |
| [combined-pilot](combined-pilot/summary.json) | word+dense+最終partition省略。240 samples全一致 |
| [final](final/summary.json) | 最終header、GCC/Clang×O2 release/debug/native O3、5反復。1200 samples全一致 |
| [whole-final](whole-final/INTERRUPTED.txt) | GCC120 sample後、Clangへの診断flag誤指定で6 compile失敗しrunner停止。最終比較には不使用 |
| [whole-resume](whole-resume/summary.json) | 診断flagを修正した完全なwhole再実行。240 samples全一致、compile/run失敗なし |
| [validation](validation/results.json) | backend・dialect・標準・copy例外・sanitizer、42 commands全成功 |

pilotのGCC O2 release、wideの構築/online kthは通常版32.14/25.80ms、新版14.71/22.00msでした。
Clangでは17.29/25.60msから11.48/17.88msです。これは初期候補の比較で、追加候補の判断は後続測定へ分けます。
少QではPOPCNT版の操作時間が通常版より長い組み合わせもありました。構築や全体時間を省いて普遍的な改善とは扱いません。

## 採用・棄却

word構築・dense順位表・最終partition省略を組み合わせました。controlに対するnative O3の構築時間は、
wideでGCC 11.306→8.164ms、Clang 9.456→7.578ms、duplicatesで3.633→2.240ms / 3.005→2.059msでした。
これは同じ/tmp harnessの候補比較です。queryを変更していない候補間にもばらつきがあり、微小差を採用理由にはしていません。

- word構築は構築時間を両compilerで短縮しました。入力の64要素に対応するwordとprefixを同時に書きます。
- dense順位表は狭い値域で比較/radix sortを省き、既存bufferを使うので追加allocationを増やしません。
  実際の異なる値だけへ順位を詰め直します。unsigned差分と、値域内に収まる加算で符号付き極値にも対応します。
- 最終段ではbit/rankは必要ですがpartition結果を使わないため、partitionだけを省きました。
- radix16はsmallの構築が約0.03→0.11〜0.12msとなり、wideでもGCCは改善せず、固定の大きなhistogramを増やすため棄却しました。
- count/range_freqのbranchless化はwideのGCCでcount 15.116→17.510ms、less 20.809→24.831msと悪化しました。
  早期終了版も回帰を伴い、既存の分岐を残しました。kthのmask化が他のqueryにも有効とは決めつけません。

## 最終phase比較

代表のwide入力です。各列は5回の中央値、時間はms、メモリはVmHWMのKiBです。
parse/build/ops/format/totalはそれぞれ独立に中央値を取るため、列の和がtotalに一致するとは限りません。
全fixtures・O2 debug・count/range_freq・標準C++経路・各回の値は [final/summary.json](final/summary.json) と
[samples.jsonl](final/samples.jsonl) に保存しています。

| compiler / flags | 実装 | parse | build | kth操作 | format | total | VmHWM |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| g++ / O2-release | 通常online | 6.897 | 29.193 | 32.139 | 1.303 | 69.595 | 11080 |
| g++ / O2-release | 新online / POPCNT | 7.042 | 10.373 | 26.233 | 1.392 | 44.444 | 11060 |
| g++ / O2-release | 新batch | 6.277 | 9.281 | 7.060 | 1.216 | 24.098 | 13992 |
| g++ / native-O3 | 通常online | 6.233 | 27.787 | 23.709 | 1.082 | 60.111 | 11088 |
| g++ / native-O3 | 新online / POPCNT | 6.138 | 8.013 | 19.175 | 1.087 | 34.149 | 11096 |
| g++ / native-O3 | 新batch | 6.432 | 8.345 | 7.166 | 1.069 | 22.793 | 13988 |
| clang++ / O2-release | 通常online | 8.827 | 16.540 | 25.471 | 1.131 | 52.398 | 10832 |
| clang++ / O2-release | 新online / POPCNT | 8.674 | 9.702 | 17.656 | 1.174 | 37.988 | 11096 |
| clang++ / O2-release | 新batch | 8.764 | 9.314 | 6.387 | 1.094 | 25.622 | 13944 |
| clang++ / native-O3 | 通常online | 7.661 | 15.469 | 30.437 | 1.181 | 55.383 | 10848 |
| clang++ / native-O3 | 新online / POPCNT | 7.983 | 7.608 | 15.945 | 1.115 | 32.736 | 11108 |
| clang++ / native-O3 | 新batch | 7.844 | 7.647 | 5.813 | 1.064 | 22.334 | 13932 |

オンラインkthの操作時間も両compilerの代表入力で改善しましたが、構築の短縮とbatchの利得を含めて
「すべての問い合わせが同じ倍率で速くなる」とは解釈しません。
GCC native O3のwide `range_freq` は通常21.644msに対して新版25.224msと約16.5%遅く、
構築・入出力を含むtotalは56.418→43.106msでした。構築済みの同じ木へrange_freqを大量に繰り返す用途では、通常版が有利になり得ます。

`-march=native` 下では標準C++経路にもPOPCNTが生成されます。GCC wide kthは明示asm 19.175ms、標準C++経路18.894msで、
手書きasmの方が常に速いわけではありません。逆にO2 releaseの同じ比較は26.233ms / 31.488msでした。
全体への特殊CPU指定なしでPOPCNTを選べることと、配置・構築・移動の改善を分けて考えます。
batchはコピー等の準備も操作時間に含みますが、問い合わせ数が少ない条件では追加確保の利点が小さくなります。

## 上位提出そのものとのwhole比較

GNU++23、O2、NDEBUG、march=native、CPU 0、5反復です。すべて通常のLC入力です。
同じinput SHAについて、独立したpersistent segment treeの答えと整数列を照合し、240回すべて一致しました。
元提出のI/O、専用radix、先読み・処理順を維持しています。次の値はプロセス全体のwall timeの中央値（ms）です。

| compiler / fixture | 通常online | 新online | 新batch | 403008 | 393928 | 376681 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| g++ / small | 6.974 | 5.736 | 5.056 | 2.936 | 4.613 | 4.119 |
| g++ / wide | 51.789 | 34.651 | 22.449 | 11.521 | 17.737 | 17.722 |
| g++ / duplicates | 17.362 | 12.421 | 11.887 | 6.910 | 9.449 | 9.520 |
| g++ / few-query | 30.216 | 13.441 | 13.980 | 9.332 | 11.131 | 9.940 |
| clang++ / small | 10.036 | 5.761 | 5.502 | 3.067 | 4.084 | 3.908 |
| clang++ / wide | 57.597 | 34.358 | 22.461 | 12.255 | 17.451 | 15.890 |
| clang++ / duplicates | 17.429 | 11.498 | 11.468 | 6.035 | 9.115 | 8.239 |
| clang++ / few-query | 20.455 | 11.691 | 12.210 | 8.783 | 10.685 | 10.377 |

上位403008のwholeにはなお差があり、wideでは新batchの約半分の時間でした。
403008は全queryを先読みし、段の構築とquery処理を融合するone-shot操作です。
AVX2のpartition・prefix・gatherを使い、段を進めながら一時rankを再利用します。
こちらのWavelet Matrixは全段のrank構造を保持し、その後もonlineや別batchを呼び出せます。
393928/376681にも専用radix、固定桁出力、mmap等の違いがあります。
この比較では元提出のI/Oを個別に入れ替えていないため、残差をI/Oまたは構造の何msと断定していません。
コード長と再利用可能なAPIを優先し、より長いAVX2 stream compaction/query kernelの実装は追加していません。

メモリも同一programのpeakであり、構造単体ではありません。GCC wideの通常onlineは7040KiB、
新online 7308KiB、新batch 8684KiB、403008は13704KiBでした。
固定制約や機能の違いをなくした一般的な速度・メモリの優越を主張するものではありません。

最初のwhole runnerはClangへGCC専用の `-fno-diagnostics-show-caret` を渡したため、
**自作実装と第三者提出を含む6件のClangコンパイルが失敗**しました。
その後、空の候補リストのrotationでZeroDivisionErrorになりました。
[whole-final/INTERRUPTED.txt](whole-final/INTERRUPTED.txt) と各compiler logを残し、
Clangでは `-fno-caret-diagnostics` を選ぶよう直してwhole-resumeへ完全再実行しています。
元の失敗を外部提出の非互換や0msの結果として扱っていません。

## 担当範囲の検証

[validation/results.json](validation/results.json) は16 compileと26実行の42 commandsすべて成功です。
固定seedは20261020です。

- GCC/Clang、GNU++20/23、AT&T/Intel両dialectの8 compile構成で、hardware/標準C++の各経路をbrute forceと照合。
- 共通FORCE_SCALARを指定したGCC/Clangで、公開の利用判定を含めて標準C++経路を確認。
- コピー代入で3本のvectorそれぞれの確保を失敗させ、旧size/get/kth/countを保持することをGCC/Clang×GNU++20/23で確認。
- GCC/ClangのASan/UBSan、hardware/標準C++各経路で境界・全公開query・copy/moveを確認。

空・全同値・昇降順・重複・64要素境界、標準整数型およびGNUの128bit整数の極値を含みます。
公式verify用コードはonline kth、batch kth、static frequencyの3件を追加しています。
この節は担当範囲のtargeted検証の記録であり、統合作業のmake/公式verifyの実行結果とは分けています。

[raw/LF source digest](source-digests.json) は実際にcompilerへ渡したbyte列と、CRLFだけをLFへ正規化したSHAを併記します。
Windows writerの改行をGitが正規化する場合も、どのsource内容を測定したか照合できます。
測定当時のraw snapshot自体は変更していません。

## 再実行

各フォルダの `bench.cpp.snapshot`、`run.py.snapshot`、両headerのsnapshotがその測定で使った内容です。
runnerを `.benchmark/extra-fast-more/wavelet/` へ戻してからrepository rootで実行します。
公開headerを変更した後の再実行は、その測定と別revisionとして扱ってください。

{% raw %}
```sh
python3 .benchmark/extra-fast-more/wavelet/run.py --out .build/wavelet-rerun --runs 5
python3 .benchmark/extra-fast-more/wavelet/whole.py --out .build/wavelet-whole-rerun --runs 5
```
{% endraw %}

wholeの第三者sourceは `research.json` のURLから別途取得し、同じSHAを確認してください。
失敗・未実行・異なる候補の記録を成功結果へ置換しません。
