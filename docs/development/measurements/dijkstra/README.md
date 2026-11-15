---
title: Dijkstraの速度比較
---

# Dijkstraの速度比較

2026-11-15に[Library Checker Shortest Path上位5提出](fastest-report.md)を調べた。
最上位の[415573](https://judge.yosupo.jp/submission/415573)はradix heapのほか、入力専用のAVX2処理、
頂点と辺の連続配置、入次数に基づくFIFO、単一終点の早期終了を組み合わせる。
他の上位例も整数キーのradix heapを利用する。[Nyaanの実装](https://nyaannyaan.github.io/library/shortest-path/dijkstra-radix-heap.hpp.html)も参考にし、
ソースをコピーせず、同距離で頂点番号の小さい順に処理するキューを独立に作成した。
公開順位は変動する。提出時の実行時間を別環境のローカル時間で割らない。

## 同等契約の候補

[benchmark.cpp](benchmark.cpp)は同じ有向グラフで全頂点の距離と親を計算する。
候補の出力は距離・親を含むchecksumで一致を検査する。
`sparse`は20万頂点に連結用の辺と80万乱数辺、`dense`は5000頂点に100万乱数辺、
`zero`は重み0、`ties`は重み0〜3、`disconnected`は始点から大部分が到達不能、
`line`は一本道、`tiny`は16頂点の同一グラフを1万回処理する。
これはAPIの負荷測定であり、最大辺数50万のLibrary Checker入力とは別である。
構築と探索を別々に計時し、外部wallとRSSには入力生成・グラフ・結果を含む。

2分・4分ヒープと、距離のradix bucket＋同距離頂点の二分ヒープを比較した。
radixでは各距離のtieでも既存の`(距離, 頂点)`順を保つ。
初期[candidates.json](candidates.json)では4分ヒープは入力やcompilerによって回帰した。
radix bucketの最小値キャッシュや小さいキューの線形走査も候補にしたが、
明確な普遍的改善を示さなかった。後者はGCCの`tiny`で特に不利だった。
[single.json](single.json)では、キューが1要素の状態を直接保持する候補が一本道を改善した。
[small-final.json](small-final.json)では16、64、65、128頂点を比較し、radixには小規模で回帰が見られた。
128頂点の候補に加え、[published-small.json](published-small.json)では129・256頂点でもGCCに回帰があり、512頂点以上ではradixが有利だった。
最終版は256頂点以下で二分ヒープを使い、より大きい整数グラフはradixを使う。
浮動小数点・独自型や64bitを超える整数型は二分ヒープを維持する。

[final.json](final.json)は初期radix候補の比較であり、最終headerの値ではない。
例えばGCC releaseでsparse探索の中央値は72.14→42.36ms、zeroは35.06→26.79msだった。
同runはCPU affinityを固定していないため、後続の固定CPU測定とは集計を混ぜない。
[hybrid.json](hybrid.json)は実行中に入力生成コードが変更されたため参考保存に限り、
採用判断に使わない。測定時の入力生成ソースは[初期版](benchmark-initial.cpp.txt)、
[拡張版](benchmark-expanded.cpp.txt)、[閾値測定版](benchmark.cpp)に分けて保存した。

## 測定時headerの同条件比較

[published-final.json](published-final.json)は測定時headerと旧headerを、同一CPUに固定し、
GCC 13.3 / Clang 18.1、`gnu++20 -O2`、assert有効・無効を分けて各5回測った。
全標本で距離と親のchecksumが一致した。[測定ソース](benchmark-published-final.cpp.txt)と
[測定時header snapshot](published-256.hpp.txt)のSHA-256はログの値と一致する。
前述のsingle-target API追加後のheaderであり、ここは全頂点版の比較である。
以下はreleaseの探索部分のみの中央値ms。入力生成とグラフ構築は含まない。

測定後、MSVCの`bool`比較警告を解消するため、整数の加算前ガードに単項`+`を追加した。
測定時の[published-256.hpp.txt](published-256.hpp.txt)と現在の公開headerの差はこの1箇所である。
2026-11-15に確認したファイルのSHA-256は、測定時snapshotが
`dc79fdb6cb8ebe8e5cbd5d948d5a9a9cecd787d7d79daf3f2c01983511d76cb2`、
公開headerが`e94c187c3408f20826655f83c29e573fcc708836e4efda00a73d749b28ef3320`だった。
公開headerの性能は修正後に再測定していないため、以下は修正前の測定結果である。
修正後の境界・ランダムテストはMSVC 14.43の`/std:c++20 /O2 /W4 /WX`で、
seed 1、2、3、1337、20261115が成功した。修正前は同設定でbool比較のC4804によりコンパイルに失敗した。

| 入力 | GCC 旧→新 | Clang 旧→新 |
| --- | ---: | ---: |
| sparse | 55.83→32.37 | 59.67→35.83 |
| dense | 5.85→4.28 | 5.46→4.60 |
| zero | 31.86→26.81 | 34.99→25.17 |
| ties | 54.01→41.04 | 56.60→39.11 |
| line | 2.64→1.85 | 1.99→1.85 |
| 257頂点 | 6.17→7.46 | 6.96→6.67 |
| 512頂点 | 14.24→7.59 | 14.93→9.52 |

GCC sparseのprocess peak RSSは57,252→57,860KiB。入力・グラフなども含む。
257頂点のGCC回帰は残る。小さな差にはOS負荷や割当の揺れも含まれる。

[thresholds.json](thresholds.json)は257/384/511/512頂点で、乱数・全0・0〜3の3種類の
辺重みをGCC/Clang各3回比較した。[生成器](benchmark-thresholds.cpp.txt)とheaderのhashも記録している。
GCCの乱数重みは257で6.87→7.02ms、384で10.71→7.62msだった一方、
0〜3の512では7.25→8.77msと逆転した。Clangでは同じ512入力が9.86→4.02ms。
いずれの閾値も全分布の改善は保証できず、256は試した小規模ケースの負担と
それ以上の改善を両立する選択である。

## 最速提出との全体比較と切り分け

[whole.json](whole.json)と[cross.json](cross.json)は、同じ入力・CPU・compiler・
`-std=gnu++23 -O3 -march=native -DNDEBUG`で各5回測ったprocess wallとpeak RSSである。
入力はLC上限内の単純有向グラフで、sparse・全重み0・一本道・到達不能を用意した。
全出力経路について始点・終点・重複頂点・辺・重みを検査し、距離を比較した。
入力はseedと生成器から再生成し、SHA-256が一致することを確認して使う。

GCCの中央値msを示す。FastIOは共通の`FastInput`/`FastOutput`であり、原提出の専用I/Oではない。
`target`は比較用の早期終了試作で、全頂点結果を返す操作とは異なる。

| 入力 | 旧heap / iostream | 旧heap / FastIO | radix / FastIO | target / FastIO | 原提出 |
| --- | ---: | ---: | ---: | ---: | ---: |
| sparse | 127.78 | 89.59 | 69.99 | 57.48 | 13.19 |
| zero | 123.44 | 85.91 | 82.17 | 82.25 | 18.72 |
| line | 84.19 | 40.45 | 37.18 | 40.11 | 18.19 |
| disconnected | 68.64 | 37.61 | 38.26 | 36.59 | 12.15 |

この表はI/Oと処理範囲も含む比較であり、radix単体の倍率ではない。
原提出は上記の専用配置・I/O・FIFOを持つ。各要因の個別寄与は未測定であり、
これらの数値を同じ全頂点APIの差とは扱わない。
交差測定は[tmp入力の消失](cross-initial-failure.log)で最初に停止し、
入力を再生成してhashを照合した。また[FastOutputの型指定誤り](cross-build-failure.json)で
再開時のcompileが失敗した。どちらも標本はなく、修正後の結果と混ぜていない。

最終の`dijkstra_path`を使った[再比較](cross-final.json)では、GCC sparseの中央値は
FastIO付き旧heap 119.85ms、全頂点radix 102.12ms、単一target 88.84ms、原提出14.89msだった。
入力・compilerは各run内で揃えたが、別runの表とはCPU負荷が違うため中央値を混ぜない。
探索前に入力グラフをCSRへコピーする試作も比較し、GCC一本道はtarget 38.33msに対し
CSR 42.08ms、到達不能は34.92msに対し37.62msだった。process peak RSSはそれぞれ
38,472→48,104KiB、22,332→31,712KiBで、入力とグラフも含む。
他の分布でも一貫した改善はなく、追加の構築と値型要件を考慮して採用しなかった。
最速原本との差には専用I/O・データ配置・FIFOなどの未分離要因が残る。

## FIFOと直接CSR構築の切り分け

2026-11-15の追加実験では、最速提出の効果を推測だけで割り当てず、入次数FIFOと
辺の連続配置をそれぞれ個別に測った。Windows 11、MSVC 14.43.34808、
`/std:c++20 /EHsc /W4 /WX /O2 /DNDEBUG`で、論理CPU 0に固定した。
10万頂点、乱数seed 20261115の有向グラフを生成し、各方式をwarmup後に順序を交替して
8回ずつ測った。両実験とも同じradix queueを使い、実験中の他の計測・コンパイルは
停止した。OSの負荷による揺れは残る。値はmsで、各runの各区間の中央値を示す。
合計は各runの区間合計の中央値であり、個別の中央値の和とは限らない。
この実験のheaderは現在のCSR追加より前の版で、[snapshot](fifo-csr-isolation-header.hpp.txt)の
SHA-256 `e94c187c3408f20826655f83c29e573fcc708836e4efda00a73d749b28ef3320`を
各JSONに記録したhashと照合済みである。

FIFOは[試験コード](fifo-isolation.cpp)で、同じvector型隣接リストと同じqueueを使い、
入次数を数える準備と探索を比較した。[計測結果](fifo-isolation-msvc-final.json)の全runで
全頂点の距離が一致した。出力と親配列は扱わず、実験の両方式には公開版にない確定済み
フラグを加えているため、公開APIそのものの実行時間ではない。

| 入力 | 準備＋探索 既存queueのみ | 準備＋探索 FIFO併用 | heap push 既存 / FIFO |
| --- | ---: | ---: | ---: |
| 乱数辺 | 15.27 | 17.36 | 221027 / 196466 |
| 重み0 | 10.47 | 12.62 | 100000 / 98182 |
| 一本道 | 0.58 | 1.01 | 100000 / 0 |
| DAG | 18.91 | 17.98 | 268545 / 178266 |

FIFOはDAGではqueue投入を減らしたが、乱数・重み0・一本道では前処理込みで遅かった。
実装では一本道でも現在のqueueが1要素を特別扱いするため、FIFOがheap投入をなくすことだけで
短縮できるとは限らない。またFIFOは同距離の親選択順を変える。例えば0→1が10、0→2が1、
1→3が0、2→3が9の順で辺を与えると、FIFOは1経由、既存の距離優先順は2経由を選ぶ。
距離だけの一致を、現在の`parent`契約の互換性とみなさない。
先に測った[予備結果](fifo-isolation-msvc.json)は他の計測と重なった可能性があるため、
この表には含めない。

Windows用Clang 18.0.3（MSVC ABI、`-std=gnu++20 -O2 -DNDEBUG`）での
[同条件比較](fifo-isolation-clang.json)も、前処理込み中央値は乱数12.39→15.92ms、
重み0は13.74→16.82ms、一本道0.62→1.02ms、DAG16.84→16.59msだった。
同じcompilerの同じrun内で比較しており、MSVCの測定値とは集計していない。

CSRは[試験コード](csr-isolation.cpp)で同じ辺配列からvectorの隣接リストとCSRを直接構築し、
隣接順とqueueを揃えた。両方式の**全距離と全親が全runで一致**した。
[計測結果](csr-isolation-msvc.json)の構築・探索中央値を以下に示す。

| 入力 | 辺数 | 構築 vector / CSR | 探索 vector / CSR | 合計 vector / CSR |
| --- | ---: | ---: | ---: | ---: |
| 乱数辺 | 499999 | 41.51 / 5.86 | 16.17 / 10.77 | 57.68 / 17.27 |
| 重み0 | 499999 | 41.89 / 12.28 | 11.32 / 7.69 | 53.29 / 19.98 |
| 一本道 | 99999 | 3.87 / 1.08 | 0.75 / 0.68 | 5.16 / 1.82 |
| DAG | 499994 | 37.68 / 6.22 | 18.51 / 13.81 | 56.69 / 20.62 |

CSRの構築は次数集計・prefix計算・辺配置を含む。vector方式は辺ごとの`push_back`を含み、
両方式とも生成済みの辺配列から開始する。入力文字列の読取り、辺配列の生成・保持、結果の
出力・破棄は計時外で、メモリも未測定である。生成時の一時辺配列とCSRの両方を必要とする
ため、競技プログラム全体で同じ倍率になるとは言えない。以前のCSRへのコピー試作と違い、
この実験は最初からCSRを作るが、`WeightedGraph`での辺追加や参照の公開契約を持たない。
既存APIを置き換えず、型・追加順・親選択・メモリ・I/O・GCC/Clangでの再測定を含め、
使い分けられる構築方法を検討する必要がある。最速提出の専用I/OやFIFOなどの残る差を、
この結果だけで説明したものではない。

Windows用Clangの[別run](csr-isolation-clang.json)では、乱数の構築は41.95→4.55ms、
探索は14.58→10.63ms、合計57.26→15.59msだった。一本道の探索だけは0.68→0.80msと
逆転したが、構築込みは3.88→1.49msだった。両方式の距離と親は全runで一致した。
これはLinux版ClangやGCCの実測ではなく、メモリ・I/Oも上と同様に未測定である。

各試験の[実行スクリプト](run_fifo_isolation.py)・[CSR用スクリプト](run_csr_isolation.py)と
JSONは入力条件、header・driver・binaryのhash、各sample、失敗を保持する。出力に記載した
binaryを再作成するには、上記MSVC flagsで各driverをコンパイルする。

## 公開CSR試作の入力から出力まで

同日、`WeightedCsrGraph<long long>`と既存`WeightedGraph<long long>`の単一終点探索を、
[実際の入出力driver](csr-end-to-end.cpp)で比較した。いずれも共通の`FastInput`と
`FastOutput`を使う。比較した構築方法は次の3つである。

1. `stream-nested`: 辺を読みながら既存graphへ直接`push_back`する。
2. `buffered-nested`: 全辺を`vector<tuple<int,int,long long>>`へ読み、既存graphを構築する。
3. `buffered-csr`: 同じ型の辺配列からCSRを構築する。辺配列は探索中も保持する。

Windows 11上、MSVC 14.43.34808のC++20とWindows Clang 18.0.3のgnu++20で、
どちらも`-O2`相当・assert無効、論理CPU 0に固定した。同じ入力・同じcompilerで
warmup各1回と測定各5回を交互に行った。[MSVCの全sample](csr-end-to-end-msvc-final.json)と
[Clangの全sample](csr-end-to-end-clang.json)には、コンパイル引数と結果、入力・header・
driver・binaryのhash、実測値を保存した。Windows固有の結果であり、Linux GCCやLinux Clangの
速度を表さない。OS負荷とファイルキャッシュの影響は残る。

この測定はCSR copy/move操作の修正前のheaderで行った。測定時headerのSHA-256は
`f37bb3a78bff973dda77e912e4e6755dfc02a141fef245162a44aa9a18478936`。
測定後のheaderから追加されたcopy/move操作のみを除いて復元した[snapshot](csr-end-to-end-measured.hpp.txt)が、
このhashと完全一致することを確認した。構築・探索のコードは修正前後で同じだが、以下は
修正後のheaderそのものの測定値ではない。

seed 20261115で10万頂点の5入力を生成し、乱数・重み0・DAGは約50万辺、一本道は99999辺、
到達不能は400999辺とした。実際の入力のhashと辺数はJSONにある。各標本の出力を3方式で
完全一致させたうえで、Pythonの独立した二分heapによる最短距離と、経路の接続・重み・
始終点・重複を検査した。これはローカルの生成入力であり、公式checkerの結果ではない。

次はプロセス起動から終了までのwall時間中央値ms。入力ファイル・出力ファイルを開く時間、
読取り、構築、探索、出力、終了処理を含み、出力hashの計算は含まない。

| 入力 | MSVC stream / buffered / CSR | Clang stream / buffered / CSR |
| --- | ---: | ---: |
| 乱数辺 | 112.23 / 83.55 / 42.76 | 116.25 / 94.54 / 44.98 |
| 重み0 | 129.13 / 104.38 / 46.92 | 94.39 / 85.28 / 42.45 |
| 一本道 | 30.91 / 26.87 / 21.36 | 20.84 / 20.53 / 16.90 |
| DAG | 130.63 / 87.87 / 52.28 | 80.32 / 75.93 / 41.96 |
| 到達不能 | 49.00 / 55.44 / 32.41 | 53.89 / 50.35 / 34.55 |

Windowsの`PeakWorkingSetSize`中央値MiBは次のとおり。入力保持と初期化を含むプロセス全体の
ピークであり、データ構造だけのメモリではない。OSページ管理・ファイルキャッシュに左右される。

| 入力 | MSVC stream / buffered / CSR | Clang stream / buffered / CSR |
| --- | ---: | ---: |
| 乱数辺 | 25.81 / 37.26 / 26.83 | 25.81 / 37.26 / 26.83 |
| 重み0 | 23.63 / 34.80 / 25.17 | 23.36 / 34.80 / 25.17 |
| 一本道 | 11.20 / 13.48 / 10.07 | 11.20 / 13.48 / 10.07 |
| DAG | 25.30 / 36.53 / 28.09 | 25.08 / 36.53 / 28.09 |
| 到達不能 | 17.05 / 25.99 / 20.93 | 16.81 / 26.00 / 20.93 |

例えばMSVC乱数入力のbuffered vector / CSRは、読取り23.58 / 23.03ms、構築43.35 / 5.12ms、
探索7.58 / 4.27ms、出力0.32 / 0.33msだった。streamは読み取りと構築を分離できず、
その合計が86.60msだった。各列は個別の中央値であり、足してwall時間と一致するとは限らない。
CSRはこの入力群では全体時間を短縮したが、streaming vectorよりピークメモリが大きい入力もある。
この結果は有望な別方式の根拠であり、既存graphでの逐次辺追加や参照の契約を置き換える根拠ではない。
最速提出との同条件比較、Linux GCC/Clang、公式verifyはこの測定では実施していない。

### copy/move修正後の再測定

修正後の[snapshot](csr-end-to-end-current.hpp.txt)のSHA-256は
`86564806927eb7f8f42b1ab2a53d099a64f935f7d699b472a3cdf5f8e6f7120f`。
同じ入力、同じ手順で[MSVC](csr-end-to-end-msvc-current.json)と
[Windows Clang](csr-end-to-end-clang-current.json)をそれぞれ単独で再実行した。
各5回・3方式の出力は一致し、5入力それぞれの距離と経路は独立した二分heapで検査した。
異なるrun間でsampleや中央値を混ぜていない。全体wall中央値msは以下のとおり。

| 入力 | MSVC stream / buffered / CSR | Clang stream / buffered / CSR |
| --- | ---: | ---: |
| 乱数辺 | 101.94 / 93.85 / 44.41 | 123.14 / 97.32 / 49.66 |
| 重み0 | 95.63 / 99.31 / 42.80 | 96.99 / 92.57 / 45.53 |
| 一本道 | 23.10 / 25.19 / 19.80 | 19.80 / 20.19 / 18.20 |
| DAG | 102.99 / 88.93 / 45.80 | 94.94 / 92.15 / 53.81 |
| 到達不能 | 49.76 / 51.92 / 32.00 | 69.87 / 68.23 / 40.71 |

ピークworking set中央値MiBは以下のとおり。入力辺配列が必要なbufferedとCSRでは、
探索中もその配列を保持している。

| 入力 | MSVC stream / buffered / CSR | Clang stream / buffered / CSR |
| --- | ---: | ---: |
| 乱数辺 | 25.81 / 37.26 / 26.83 | 25.81 / 37.26 / 26.83 |
| 重み0 | 23.63 / 34.80 / 25.17 | 23.36 / 34.80 / 25.17 |
| 一本道 | 11.20 / 13.48 / 10.07 | 11.20 / 13.48 / 10.07 |
| DAG | 25.30 / 36.53 / 28.09 | 25.08 / 36.53 / 28.09 |
| 到達不能 | 17.05 / 25.99 / 20.93 | 16.82 / 26.00 / 20.93 |

MSVCの乱数入力ではbuffered vector / CSRそれぞれ、読取り23.98 / 24.59ms、
構築47.40 / 5.27ms、探索6.72 / 6.20ms、出力0.33 / 0.30msだった。streamの
読取り＋構築は80.96ms。これは各区間の中央値であり、全体wallの内訳を足し合わせた数値ではない。
現行版についても最速提出との同条件比較とLinux環境での検証は未実施である。

測定の最初はMSVCのversion出力をWindows既定のcp932で解読しようとして失敗した。
[失敗記録](csr-end-to-end-msvc.json)には測定標本がなく、UTF-8で置換可能に直した後の結果と
混ぜていない。[生成・検査・計測スクリプト](run_csr_end_to_end.py)の`--prepare`で入力を生成し、
`--compiler msvc --output ...`または`--compiler clang --output ...`で各方式を再コンパイルして測る。
生成入力とbinaryは`.build/dijkstra-csr-end-to-end/`に置き、Gitには含めない。

## Linuxでの入出力を含む比較

PR #64と同じライブラリ本体について、GitHub ActionsのUbuntu、AMD EPYC 9V45、
GCC 15.2 / Clang 21.1、`gnu++23 -O3 -march=native -DNDEBUG`で比較した。
入力の生成後、同じ入力ファイルを3方式と[提出415573](https://judge.yosupo.jp/submission/415573)に渡し、
warmup後に順序を入れ替えて各5回実行した。提出ソースは既存記録のSHA-256と一致したものを
一時取得し、リポジトリには含めない。全出力は独立した最短距離と経路検査を通過した。
提出は専用I/O・グラフ配置・FIFOを使い、汎用APIと親選択の契約は異なる。

下表はLibrary Checkerの制約内の単純グラフでのprocess時間中央値msとpeak RSS中央値KiB。
時間は`/usr/bin/time`の起動も含み、CPUを1論理CPUに固定した。runnerは共有環境である。
逐次は入力しながら`WeightedGraph`に追加し、CSRは辺配列を読み込んでから構築する。
どちらも入力、構築、探索、出力を含み、CSRのRSSは入力辺配列の保持も含む。

| 入力 | GCC 逐次 / CSR / 提出 | Clang 逐次 / CSR / 提出 | GCC RSS 逐次 / CSR / 提出 |
| --- | ---: | ---: | ---: |
| sparse | 81.56 / 25.27 / 12.48 | 78.00 / 26.49 / 13.79 | 26216 / 25016 / 25020 |
| zero | 87.81 / 34.72 / 16.32 | 130.71 / 39.97 / 20.37 | 25060 / 23612 / 28976 |
| line | 40.77 / 33.28 / 14.77 | 43.39 / 34.70 / 13.51 | 38904 / 32980 / 46056 |
| disconnected | 57.82 / 25.83 / 10.86 | 62.16 / 25.12 / 11.39 | 22512 / 23208 / 22216 |

例えばGCCのsparseにおけるCSRの段階別中央値は入力13.12ms、構築5.34ms、探索2.48ms、
出力0.23msだった。段階別中央値の和とprocess中央値は別に集計している。提出側は段階別に
測っていないため、残る差を入力だけの差とは断定できない。4分布とも提出との差が残る。
入力・I/O・配置・キューの個別寄与をさらに調べる必要がある。
短い整数の検査をまとめるFastInputの[試作と測定](parse-short-experiment.md)も行ったが、
Windowsの比較では一貫した改善を確認できず、公開headerには採用していない。

[GCCの全標本](csr-end-to-end-linux-gcc.json)、[Clangの全標本](csr-end-to-end-linux-clang.json)、
[runner](run_csr_end_to_end_linux.py)にcompiler・flags・CPU・入力とソースのhash、各標本、
出力の検証結果を保存した。Windows測定と同じ5入力も別のsuiteとして含む。
Linux側のheader・I/OのhashはGitで改行を正規化した内容に対応し、Windows作業ツリーの
CRLFを含む実バイトhashとは異なる。
[実行ログとartifact](https://github.com/blueberry1001/Blueberry-library/actions/runs/43722849211)は
一時的な検証用PRのものであり、通常のrequired CIとは区別する。

### 上位提出の段階別計測

同じ4入力で提出415573の通常版と、原本の`PROF`指定を有効にした版を別に測った。
この実行は別のGitHub Actions runner（AMD EPYC 7763）であり、上表のEPYC 9V45の
各時間との差をそのままI/O等の寄与として扱えない。`PROF`版は時計の取得と標準エラー出力を
含み、各区間値は0.01ms単位の累積時刻の差である。GCC 15.2、同じflagsで5回の中央値は次の通り。

| 入力 | 通常版 process | PROF版 process | PROF版 入力 / 構築 / 探索 / 整形 / 出力 |
| --- | ---: | ---: | ---: |
| sparse | 17.11ms | 17.75ms | 7.85 / 2.91 / 3.84 / 0.24 / 0.09ms |
| zero | 21.67ms | 21.99ms | 7.01 / 2.98 / 8.59 / 0.16 / 0.05ms |
| line | 24.67ms | 23.11ms | 5.34 / 2.94 / 4.18 / 3.16 / 4.39ms |
| disconnected | 13.87ms | 13.23ms | 6.83 / 2.78 / 0.70 / 未測定 / 未測定ms |

入力区間には最初の領域確保なども含む。到達不能時は原本が整形・出力の計測前に戻るため、
その2区間は未測定である。通常版とPROF版の中央値が逆転した入力もあり、計測負荷を固定の
差分とはみなさない。[GCC](fastest-profile-linux-gcc.json)・[Clang](fastest-profile-linux-clang.json)の
全標本、[runner](profile_fastest_linux.py)、[CIログとartifact](https://github.com/blueberry1001/Blueberry-library/actions/runs/43724414416)
に入力・原本のhash、各標準エラー、経路の独立検証結果を保存した。

## 再現

[run.py](run.py)、[whole.py](whole.py)、[cross.py](cross.py)はリポジトリルートで動かす。
`.build/dijkstra-candidates/<候補>/blueberry/graph/dijkstra.hpp`に対応する
`*.hpp.txt`を配置し、[benchmark.cpp](benchmark.cpp)・[io-driver.cpp](io-driver.cpp)も
`.build/dijkstra-candidates`に置く。原提出はライセンス不明のため収録しない。
`https://v3.api.judge.yosupo.jp/submissions/415573`のJSONの`source`を
`.build/dijkstra-fastest/submission-415573.cpp`へ保存し、記録されたsource hashと照合する。
`scripts/fetch_lc_fastest.py`で上位の調査もできるが、順位は変わり得る。
出力先の入力hash、compiler、binary hash、
各試行、失敗を照合する。生成入力とbinaryは追跡しない。
