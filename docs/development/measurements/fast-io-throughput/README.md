# Fast I/O backendの比較測定

`benchmark/fast-io-throughput.py`と`benchmark/fast-io-throughput.cpp`で、
既存のcheckedなFast I/Oをbaselineとし、整数入力・整数出力の改善候補を比較する。
`FastInput` / `FastOutput`を直接使う経路と、`scan` / `print`から使う経路を両方測る。
型・符号・空白・EOF・不正token・overflow・FILEエラーの公開契約を保った候補を対象とする。

候補選択用の`screen-v1`から`screen-v5`までの記録を保存した。
最終版はv5を採用した。以下ではscreenと最終5回測定を分け、同じrun内のbaseline/currentだけを比較する。
Fastest提出と著名ライブラリの方式・契約差は[調査報告](research.md)、
取得条件・ソースhashは[research.json](research.json)に分けて保存した。

## baselineと比較単位

baselineはrevision `617acd6d63affe95308fee3fec1c9698096f10ad`の
`blueberry/utility/fast-io.hpp`であり、[baseline.hpp.snapshot](baseline.hpp.snapshot)に保存した。
このファイルのSHA-256は次のとおり。

`b8b9c274f93c3906b8f6fc26dd945ec0679b3c246685da3108789aa677d73b30`

runnerはinclude overlayでこのヘッダだけを差し替える。
各screenのbaselineとcurrentは、同じharness・コンパイラ・フラグ・入力・API経路で比較する。
`contest-fast-io.hpp`などのwrapperは両者で共通であり、以前のテンプレート全体との比較ではない。
候補は未commitの状態でも測るため、revisionだけでなく`environment.json`の
`source_sha256`で実際の測定ソースを識別する。

screen間ではharness・runner・対象ケースや型が変わっている。
特にv3から、入力だけのループと出力付きのループをcompile-time分岐に分けた。
v1/v2の各反復にあった実行時のmode判定とは生成コードが異なる。
また、v3の公式入力はu64、v4/v5はi64として読む。
**異なるscreenの絶対時間を引いて候補間の改善量を求めない。**
速度値を比較する単位は、同じscreen内のbaseline/currentの組に限る。

| screen | C++ harnessのSHA-256 | runnerのSHA-256 |
| --- | --- | --- |
| v1 | `be0b705762f956dfc672980f585d850d6467f85bf435f9f99b657b5788023480` | `67deebc9264855447520a5db3b41a5488358c57566f8681afe62d0b5bf58df52` |
| v2 | `be0b705762f956dfc672980f585d850d6467f85bf435f9f99b657b5788023480` | `195b2a014ace45d4339b016c4234ba48b17cf297da03393226efdd7270b242a5` |
| v3/v4/v5 | `68dd66ce31ba4c4fb9db144db90ce8c344cc8aa35edb6c6efd9ae110b7927137` | `195b2a014ace45d4339b016c4234ba48b17cf297da03393226efdd7270b242a5` |

ヘッダ・入力・期待出力のhashとcompiler全文は、各screenの`environment.json`を正とする。
full hashを保存し、短い表示名だけで同一の測定と判断しない。

## 環境と測定範囲

WSL2、Intel Core i7-14650HX、CPU affinity 2。
GCC 13.3.0とClang 18.1.3、gnu++20、`-O2 -DNDEBUG`を使う。
スクリーニングはwarmup 1回、本測定3回。各roundの実行順をseed 20261020でshuffleし、
測定対象の子プロセスを1件ずつ実行する。他のプロセスは変更・停止しない。

- `input`: parseと加算・checksumを計測する。checksumの出力はoperation timerの外。
- `output`: 和をvectorに先読みしてからtimerを開始し、format・write・flushを計測する。
- `end-to-end`: parse・加算・format・write・flushをまとめて計測する。

`setup_ms`はBackend構築だけであり、global経路のバッファは最初の呼び出しで遅延構築される。
構築時間だけを切り出して直接経路と比較せず、初期化を含むoperation/processの範囲も確認する。
`process_ms`は起動、先読み、初期化、破棄、time wrapperも含む。
最大RSSはプロセス全体で、出力専用モードの先読みvectorも含む。
このためRSSの差をそのままI/Oバッファの差とは解釈しない。
入出力バッファ自体は64 KiBずつを維持する。

stdinは通常fileとpipe、stdoutは`/tmp`の通常file。
出力は毎回期待値とbyte単位で一致するか確認する。
入力だけのモードでは、和を2^64で割った余りのchecksumと比較する。
失敗した実行は`failure.json`または`run-error.json`として残し、0 msや成功に置き換えない。

生成入力はi64/u64/i128/u128の4型。screenでは各200,000組とした。
型ごとの固定seedは20261020から20261023で、型の最小最大・符号・長い桁・短い桁を混ぜる。
和も各型の範囲に収め、Pythonの整数演算で期待出力を独立に作る。
v3以降は公式Many A+Bの`all_zero_00`、`digit_random_00`、`max_random_00`も用い、
公式の期待出力をPythonで再計算して一致確認する。各公式入力は1,000,000組である。
入力・期待出力・checksumのhashも保存する。

## 候補の採否記録

### v1: SWAR入力、標準変換による整数出力

入力は、8byteの数字検査とSWAR変換を導入した。
出力は整数tokenの最大桁数分の容量を一度確保し、64bit以下も`std::to_chars`で変換した。
128bitは10^19ごとに分割して64bitの変換に渡し、広い除算の回数を減らした。

[screen-v1](screen-v1/summary.json)では128bitの入力・出力に改善があった一方、
GCCの64bit出力に回帰があった。64bit以下の標準変換への一律置換は採用しない。

### v2: 64bit以下は既存の2桁表を維持

整数tokenごとの容量確認と128bitの分割を残し、64bit以下は従来の2桁表による変換へ戻した。
[screen-v2](screen-v2/summary.json)で、この版とbaselineを同じharnessで測った。
64bit出力の回帰を抑えたため、以後の候補の出力経路に用いた。
v1の時間を引いて2桁表だけの効果を求めることはしない。

### v3: 4桁単位の出力

40 KiBの4桁表を追加せず、既存の2桁表を2回使って4桁単位で出力する候補を評価した。
同時にharnessの入力・出力付きループをcompile-timeで分け、GCC/Clangと公式3ケースを追加した。
[screen-v3](screen-v3/summary.json)ではClangの一部に改善があったが、
GCCの64bit出力に大きな回帰があった。4桁版は採用せず、2桁版を維持する。
公式zero列の入力回帰も確認し、短い入力を別に評価する必要が分かった。

### v4: SWARのループをscalar処理から分離

scalarの各桁処理中にSWARを試す構成を改め、SWARのループをscalarループの前へ分離した。
block内に数字以外があれば位置を進めずscalarへ戻し、境界・EOFでも既存処理を使う。
出力は2桁表を維持した。

[screen-v4](screen-v4/summary.json)では長い数字列に改善があったが、
GCCの公式zero列は入力・入力から出力までの両方でbaselineより遅かった。
この段階で完成とはせず、1桁だけのtokenに対する費用を減らす候補を追加した。

### v5: 1桁入力・出力の短い経路

現在のバッファ内に1桁と区切り空白が揃っている入力を、一般の桁ループより前に処理する。
符号の検査とFILEエラーの確認は残す。出力も絶対値が10未満なら1byteを直接書く。
境界をまたぐtoken、末尾改行のないEOF、長い先頭zero列は既存のchecked経路で扱う。

[screen-v5](screen-v5/summary.json)では公式zero列の入力から出力までに改善があった。
ただしGCCの入力だけの中央値はbaselineより少し遅く、最小・最大の区間も重なる。
zero列の全モードで回帰が解消したとは判断せず、最終測定で分布とAPI経路を再確認する。

以下は採否判断の例。全てglobal経路・通常file、operation時間の中央値で、単位はms。
**各行のbaselineとcurrentだけを比較する。**

| screen | compiler | ケース・操作 | baseline | current |
| --- | --- | --- | ---: | ---: |
| v1 | GCC | 生成i64・出力 | 4.798 | 5.218 |
| v2 | GCC | 生成i64・出力 | 4.436 | 4.429 |
| v3 | GCC | 生成i64・出力 | 4.753 | 6.745 |
| v4 | GCC | 公式zero / i64・入力 | 6.027 | 10.656 |
| v4 | GCC | 公式zero / i64・入力から出力 | 11.130 | 16.078 |
| v5 | GCC | 公式zero / i64・入力 | 5.690 | 6.136 |
| v5 | GCC | 公式zero / i64・入力から出力 | 10.633 | 8.425 |
| v5 | Clang | 公式zero / i64・入力から出力 | 16.799 | 12.479 |

各screenの`summary.json`には中央値・最小・最大、`samples.jsonl`にはwarmupを含む全実行を残した。
v1/v2は各384実行、v3/v5は各1,344実行、v4は576実行で、
保存された全行のreturncodeは0、出力のbyte照合は成功している。
これは各screenの測定出力に対する確認であり、全公開契約のテスト完了を表さない。

## 再現方法

Linux/WSLの作業ディレクトリで実行する。`--output`はまだ存在しないディレクトリを指定する。
`--revision`はWindows側のgitが示した対象commitを渡すための指定である。
コンパイラ、ACL、`/usr/bin/time`、CPU affinityの利用が必要。

生成した4型を各100万組、GCC/Clang、直接/global、file/pipe、3モード、5回で測る例は次のとおり。

```console
python3 benchmark/fast-io-throughput.py \
  --output .build/fast-io-throughput/new-run \
  --baseline-header docs/development/measurements/fast-io-throughput/baseline.hpp.snapshot \
  --revision 8729666f80f04e080f945506fd3cb94be7081185 \
  --acl-root /path/to/ac-library --cpu 2 --count 1000000 --repeats 5
```

v5のscreen条件を再現する例は次のとおり。
`LC_MANY_TEST_DIR`には公式Many A+Bの`.in/.out`があるディレクトリを指定する。

```console
python3 benchmark/fast-io-throughput.py \
  --output .build/fast-io-throughput/new-screen \
  --baseline-header docs/development/measurements/fast-io-throughput/baseline.hpp.snapshot \
  --revision 8729666f80f04e080f945506fd3cb94be7081185 \
  --acl-root /path/to/ac-library --cpu 2 --count 200000 --repeats 3 \
  --official-dir "$LC_MANY_TEST_DIR" --official-type i64 \
  --official-case all_zero_00 \
  --official-case digit_random_00 \
  --official-case max_random_00
```

特定の範囲だけ測る場合は、`--profile`、`--type`、`--facade`、`--mode`、`--transport`を絞れる。
公式入力だけなら`--official-only`を指定する。
以前のscreenを正確に再現するときは、その版のsnapshotとenvironmentに合わせる。
現在のharnessで過去ヘッダを測り直した結果は、新しい比較として別ディレクトリへ保存する。

## 最終測定

対象の実装commitは`8729666f80f04e080f945506fd3cb94be7081185`。
採用ヘッダのSHA-256は`8f08950cfc12f2babdd0eaa6ed0b38ce90e999ce7e24ee5105cdbb4cfc7ad406`。
各条件でwarmup 1回、本測定5回を行った。最終3runは計3,168実行で、全てbyte照合に成功した。
各ディレクトリにbaseline/採用ヘッダ、harness、runnerのsnapshotと環境・生ログを保存した。
構築・process・最大RSS、直接API、pipe、全モードの中央値/最小/最大は各summaryを参照する。

表はglobal API・通常fileの入力から出力までのoperation時間の中央値。
速度比はbaseline中央値を採用版中央値で割った値である。

### 公式Many A+B（i64、100万組）

| compiler | ケース | baseline (ms) | 採用版 (ms) | 速度比 |
| --- | --- | ---: | ---: | ---: |
| clang | official-all_zero_00 | 17.733 | 13.282 | 1.34倍 |
| clang | official-digit_random_00 | 53.428 | 45.855 | 1.17倍 |
| clang | official-max_random_00 | 68.148 | 43.496 | 1.57倍 |
| gcc | official-all_zero_00 | 10.606 | 8.776 | 1.21倍 |
| gcc | official-digit_random_00 | 50.965 | 46.100 | 1.11倍 |
| gcc | official-max_random_00 | 68.113 | 45.463 | 1.50倍 |

[全モード・file/pipeのsummary](final-official64/summary.json) / [全実行](final-official64/samples.jsonl)

### 公式Many A+B 128bit（i128）

組数は各入力の先頭Tをそのまま用い、environmentのcasesへ記録した。

| compiler | ケース | baseline (ms) | 採用版 (ms) | 速度比 |
| --- | --- | ---: | ---: | ---: |
| clang | official-all_zero_00 | 8.556 | 6.711 | 1.27倍 |
| clang | official-carry_up_00 | 107.369 | 42.362 | 2.53倍 |
| clang | official-digit_random_00 | 71.969 | 48.233 | 1.49倍 |
| clang | official-max_random_00 | 117.944 | 50.891 | 2.32倍 |
| gcc | official-all_zero_00 | 5.712 | 4.844 | 1.18倍 |
| gcc | official-carry_up_00 | 133.222 | 47.699 | 2.79倍 |
| gcc | official-digit_random_00 | 92.350 | 50.865 | 1.82倍 |
| gcc | official-max_random_00 | 140.802 | 55.925 | 2.52倍 |

[全モード・file/pipeのsummary](final-official128/summary.json) / [全実行](final-official128/samples.jsonl)

### 各型の全範囲を混ぜた生成入力（各100万組）

| compiler | ケース | baseline (ms) | 採用版 (ms) | 速度比 |
| --- | --- | ---: | ---: | ---: |
| clang | i128 | 204.876 | 94.884 | 2.16倍 |
| clang | i64 | 67.656 | 51.416 | 1.32倍 |
| clang | u128 | 199.506 | 93.985 | 2.12倍 |
| clang | u64 | 64.579 | 47.016 | 1.37倍 |
| gcc | i128 | 253.047 | 108.349 | 2.34倍 |
| gcc | i64 | 73.685 | 54.513 | 1.35倍 |
| gcc | u128 | 241.495 | 109.032 | 2.21倍 |
| gcc | u64 | 57.282 | 48.166 | 1.19倍 |

[全モード・file/pipeのsummary](final-synthetic/summary.json) / [全実行](final-synthetic/samples.jsonl)

64bitの出力単体は概ね従来と同程度で、主な改善は入力と128bitの出力にある。
GCC・global・fileの公式zero列では、入力単体の中央値が5.735 msから5.966 msへ約4%増えた。
最小/最大の区間は重なり、入力から出力までは10.606 msから8.776 msへ改善した。
入力だけを大量に行う用途ではこの小さい差も残るため、全操作が一律に速くなるとは扱わない。
またGCC・直接APIの公式digit_random入力では、fileが26.061 msから28.506 ms、
pipeが33.209 msから39.203 msへ増えた。pipeの入力から出力までは50.072 msから52.756 ms。
global APIでの同ケースとは生成コードが異なるため、直接APIの回帰も分けて記録する。
この差を[生成コードと追加候補の記録](validation/inlining-review.md)で調べた。
baselineと採用版の関数呼び出し回数は同じであり、コード量の増加と短いtokenに対する追加判定が差と整合した。
強制inlineと64bitだけ4桁単位にする候補も別screenで評価したが、改善が安定せず採用しなかった。
追加screenは全体チェックと実行時間帯が重なるため、最終3runとは混ぜず、同じscreen内の比較として扱う。
Fastestの表示時間は別環境・別契約のため、この表の速度比には用いていない。

変更前と採用版はどちらもバッファ各64 KiBで、整数ごとのheap allocationは追加していない。
出力専用モードのRSSには先読みvectorが含まれる。単発のRSS差からバッファ削減を主張しない。
