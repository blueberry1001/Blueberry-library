# Many A+Bの高速入出力方式の調査

調査日は2026-10-20。対象revisionは`617acd6d63affe95308fee3fec1c9698096f10ad`。
この資料はソースを読んだ結果と実測候補を記録する。ここでは候補の実測を行っていない。
取得した提出・第三者ライブラリの本文は`.build/research-fast-io/`にだけ保存した。
この資料と[research.json](research.json)にはメタデータと方式の説明だけを残す。

## 取得条件と表示時間

`scripts/fetch_lc_fastest.py`からLibrary Checker公開REST APIをGETした。
ACを`order=+time`、`dedupUser=true`で取得した。言語無指定の上位を各6件、
`lang=cpp`の上位を64bit版15件・128bit版12件まで確認した。
初回はsandboxのDNSエラーで失敗した。その後、読み取り専用のネットワーク取得で成功した。
失敗・取得条件・全overview・ソースhashは[research.json](research.json)に保存した。

次の時間はAPIの最大ケース時間であり、このリポジトリのローカル測定値ではない。
[公式help](https://judge.yosupo.jp/help)の現在の表示はGCP c2d-highcpu-8、
AMD EPYC 7B13の1 core制限、1 GiBメモリ制限。
[言語API](https://v3.api.judge.yosupo.jp/langs)の現在の`cpp`はGCC 15.2とACL 1.6。
各提出の当時のコンパイラ・フラグ・CPU構成・反復数まではoverviewから確定できない。
提出内の最適化pragmaも異なるため、表示値同士から方式単体の改善率を求めない。

| 問題 | 提出 | 表示最大時間 | 今回確認した方式と契約差 |
| --- | --- | ---: | --- |
| many_aplusb | [409103](https://judge.yosupo.jp/submission/409103) | 18 ms | 前回と同じ首位。AVX2と十進桁の直接加算。通常のparse/add/formatを省く問題専用経路 |
| many_aplusb | [392126](https://judge.yosupo.jp/submission/392126) | 24 ms | AVX2 decode、4桁表によるformat、batch化。trusted unsigned、1 delimiter、プロセス寿命の入力保持 |
| many_aplusb | [338380](https://judge.yosupo.jp/submission/338380) | 27 ms | SIMD intrinsicを使わず8桁SWAR。mmap、trusted unsigned、batchごとの出力容量確認、4桁表 |
| many_aplusb | [269738](https://judge.yosupo.jp/submission/269738) | 27 ms | mmapと8桁SWAR。先頭桁を空白に置き換える4桁表で桁数別に出力 |
| many_aplusb_128bit | [400962](https://judge.yosupo.jp/submission/400962) | 29 ms | mmapとAVX2の16/32桁parse。出力は10^19分割、64bit側の4桁表、512 KiB writer |
| many_aplusb_128bit | [393714](https://judge.yosupo.jp/submission/393714) | 30 ms | 同系のmmap・AVX2・10^19分割と4桁表 |
| many_aplusb_128bit | [392686](https://judge.yosupo.jp/submission/392686) | 31 ms | AVX2 parseと4桁format、ブロック内の和を一時配列に置いて入力と出力を分離 |
| many_aplusb_128bit | [212804](https://judge.yosupo.jp/submission/212804) | 42 ms | fread 128 KiB、8/4/2桁SWAR、型の最大桁数分を事前確保して出力、128bitは10^16分割。Ofast/unroll pragma |
| many_aplusb_128bit | [203527](https://judge.yosupo.jp/submission/203527) | 43 ms | fread/fwrite各64 KiB、8/4/2桁SWAR、型の最大桁数分を事前確保、128bitは10^16分割 |
| many_aplusb_128bit | [325269](https://judge.yosupo.jp/submission/325269) | 43 ms | fread/fwrite各64 KiB、128bitは8桁SWARと10^16分割、4桁表。Ofast/unroll pragma |

前回は64bit上位3件のSIMD・mmap方式が中心だった。
今回は128bitの大きな十進blockによる分割と、fread 64 KiBのscalar経路まで調べた。
`source_profile`のSIMDなし判定は「portableかつ同等契約」の証明ではない。
338380等はCPU intrinsicなしでもmmap・trusted inputに依存する。

## 公式入力の範囲

[公式問題repo](https://github.com/yosupo06/library-checker-problems/tree/1814c4e5205517e368bb57a8d1127eb961cfeaae/sample)の
info.toml・task.md・random generatorを取得した。

- [many_aplusb](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/sample/many_aplusb/info.toml): 最大1,000,000組、入力値は0から10^18。公式randomはspaceとLFで区切る。
- [many_aplusb_128bit](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/sample/many_aplusb_128bit/info.toml): 最大500,000組、入力値は±10^37。randomは負号を含み、spaceとLFで区切る。

どちらも不正token、型の範囲外、極端な長さの先頭zero、出力エラー、
任意のASCII空白の連続、pipeによる短いreadを網羅する契約テストではない。
64bit版におけるunsigned専用化は公式入力に合うが、Blueberryの符号付き入力を置き換えられない。

## 著名ライブラリのportable経路

### Nyaan

[Nyaanのfastio.hpp](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/misc/fastio.hpp)を
commit `b3981adc80a800b2584980b01821324ea6c77183`で確認した。
[root LICENSE](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/LICENSE)はCC0。
128 KiBのfread/fwriteバッファと64byteの事前余裕を使い、tokenごとのrefillで桁ループを軽くする。
整数出力は40,000byteの4桁表と10,000での除算を使う。

ただし入力は正しいtokenを前提とし、整数の範囲検査やboolのエラー結果を返さない。
符号付き最小値に対するsigned negationもBlueberryのunsigned magnitude経路とは異なる。
fwrite失敗も戻り値として扱わない。方式の比較資料として扱い、コードは取り込まない。

### maspypy

[maspypyのother/io.hpp](https://github.com/maspypy/library/blob/f2882db8097a2899fd501dc9a4d0ba6851de0044/other/io.hpp)を
commit `f2882db8097a2899fd501dc9a4d0ba6851de0044`で確認した。
128 KiBバッファ、100byteの事前余裕、残量不足時のslow path、40,000byteの4桁表を使う。
この版はEOF時にLF sentinelを追加し、freadエラーや想定外EOFはプロセスを終了する。
符号付き出力はunsigned magnitudeを使う。

EOFを考慮するportable経路でも、入力失敗をfalseで返して処理を続けるBlueberryとは契約が違う。
整数範囲の検査はなく、一部の構文確認もassertである。
取得したソースとrepo treeのlicense検索から明示的な利用許諾は確認できなかった。
方式を調査する資料に限り、コードは取り込まない。

## 同じ公開契約のまま評価する候補

以下はソースから推測した候補であり、採用判断にはローカル測定が必要。

| 候補 | 減らす処理 | 同じ契約を保つ条件 |
| --- | --- | --- |
| バッファ内tokenの直接parse | 各byteのnext()・refill分岐・pos_更新 | end_を越えて読まない。境界・EOF・不正tokenは既存のchecked経路へ戻す |
| 整数tokenごとに出力容量を確認 | 小さなstring_viewのcopy loopと容量確認 | 型の最大桁数+負号分を確保し、短いfwrite・sticky errorを維持する |
| 128bitを10^16等に分割 | 多数の128bitの/100、%100 | 上位・下位のzero埋め、signed最小値、unsigned128最大値を保つ |
| 4桁処理を既存の2桁表2回で出力 | 除算回数 | 40 KiB表を増やさず、既存の200byte表を使う版と4桁表版を比較する |
| 検査付き8桁block parse | 桁ごとの依存鎖と128bit乗算回数 | 全byteの0..9判定、block単位のoverflow検査、残量・endiannessを扱う |

128bit分割では、400962の10^19分割の商をu64にする契約をそのまま使えない。
unsigned128最大値を10^19で割ると約3.4×10^19となり、u64最大値を超える。
10^16の最大3block分割か、商をu128のまま保つ一般のloop/再帰なら全範囲を扱える。

SWAR提出の多くは「0を引いたbyteのhigh bit」だけを調べ、正しい数字列を前提にしている。
この条件だけではASCIIの`a`等を拒否できない。
検査を維持する版では0以上だけでなく9以下も確認する。
ワードを直接reinterpret_castして読むとalignment/aliasingも問題になるため、
memcpyまたはbyte処理を使い、big endianでは正規化するかscalar fallbackを残す。

バッファに64byte以上残っていても、tokenの長さは型の最大桁数以下とは限らない。
長い先頭zero列も有効な整数入力なので、固定長を仮定して読み越す実装は使わない。
unsigned演算がwrapしてから最終値だけを比較する方式も、overflow拒否の代わりにはならない。
既存の不正token消費と、失敗したscalar値を変更しない動作を保つ。

4桁表は出力bufferのサイズを変えず導入できるが、read-onlyデータが約40 KiB増える。
既存の2桁表は200byteであり、キャッシュ使用量と小さい整数の回帰も測る。
単に/100を/10000に替えるだけでは、128bitの広い除算費用が残るため、
大きなblockで64bit部分へ落とす候補と分けて評価する。

## 比較時に必要なケース

入力のみ・出力のみ・入力から出力までを分け、構築・process時間・RSSも残す。
通常fileとpipe、GCC/Clang、assert有効/releaseを区別する。
同一コンパイラ・フラグ・CPU・データで変更前後をroundごとに混ぜ、複数回の中央値・最小値を比較する。
今回のAPI取得だけではこれらの測定を代用しない。

入力は公式分布に加え、短い数と長い数、符号混在、0、各整数型の最小最大、
unsigned128最大、先頭+、連続空白、末尾改行なし、64 KiBの各境界を含める。
長い先頭zero・範囲外・数字以外・符号だけのtokenも、復帰後に次tokenを読めるか確認する。
出力は10の各冪の前後、4桁/16桁block境界、内部zero、signed最小値、
unsigned最大値、満杯付近のbuffer、短いfwriteとエラー保持を確認する。

SIMD必須化、全入力mmap、unsigned専用化、整数の検査削除、
十進文字列の直接加算は、対応型・pipe・失敗時動作を保つ本作業の採用候補から外す。
