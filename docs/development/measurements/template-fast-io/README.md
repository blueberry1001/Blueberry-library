# Contest Fast I/O の比較測定

`benchmark/contest-fast-io.cpp` と `benchmark/contest-fast-io.py` で、既存の
`contest-template.hpp` の unsynced iostream、追加した `contest-fast-io.hpp` の
`scan` / `print`、直接使う `FastInput` / `FastOutput` を比較する。
既存 API の戻り値以外の呼び出し方・区切り文字・固定小数点 30 桁をそろえ、
入力だけ、出力だけ、入力から出力までを分けて測る。

## 再現条件

```console
python3 benchmark/contest-fast-io.py \
  --output .build/contest-fast-io-new-run --cpu 2 \
  --acl-root /path/to/ac-library \
  --baseline-header docs/development/measurements/template-fast-io/initial/contest-fast-io.hpp.snapshot
```

Windows 作成の worktree を WSL から使う場合は、Windows の `git rev-parse HEAD` の値を
`--revision` に渡す。既存の出力先には上書きしない。

- WSL2、Intel Core i7-14650HX。GCC 13.3 / Clang 18.1.3、gnu++20、`-O2`。
- GCC release は `-DNDEBUG`、GCC assert は `-D_GLIBCXX_ASSERTIONS`、Clang は `-DNDEBUG`。
- 整数は 1,000,000 組、各引数は ±10^18 以内の `long long`、和は型に収まる。
  固定 seed 20261020。符号・桁数を混ぜ、入力だけのケースでは和の checksum を出力する。
- 整数出力だけは 1,000,000 個、浮動小数点出力だけは 250,000 個。
  double は 1/16 刻みで二進で正確に表せる値を使い、全 backend で fixed 30 桁を出力する。
- 同一 CPU に固定し、warmup 1 回と本測定 5 回。各 round の順番は固定 seed で shuffle。
  stdin は通常ファイルと pipe、stdout は `/tmp` の通常ファイル。
- 全実行で Python から独立に生成した期待出力と byte 単位で照合する。
  ソースと入力の SHA-256、コンパイラ全文、全 sample、中央値・最小・最大を保存する。

`operation_ms` は入力・checksum、または format/write/flush の時間。
入力から出力までのケースは両方を含む。`construction_ms` は backend 初期化のみで、
global の FastInput / FastOutput は初回呼び出しで生成され、その費用は操作側に入る。`construction_ms` 単独を真のバッファ初期化費用として backend 間で比較しない。setup と操作を合計した範囲で比較する。`process_ms` は起動・破棄、出力用データ生成、
`time` wrapper も含む。RSS はプロセス全体の最大値で、出力用 vector も含む。
各 backend の計測条件は同じであり、データ生成を除いた操作時間とプロセス時間を混同しない。

## 初回結果と改善候補

`initial/` は CPU 0 での初回測定。270 sample と 54 warmup が全て byte 単位で一致した。
その後、別 Codex の無関係な benchmark が同じ CPU 0 に固定されて動いていると判明した。
こちらから他のプロセスを変更・停止せず、結果を保存したまま CPU 2 に移して測り直した。
初回の細かな速度比を採用判断には使わない。

評価した改善候補は accessor 呼び出しの削減。
変更前は variadic `scan` の引数ごとに `input()` を呼び、`print` の値・区切り文字や
再帰コンテナの各要素で `output()` を呼んでいた。変更後は `scan` / `print` の先頭で
参照を 1 回取得し、再帰出力にも同じ `FastOutput&` を渡す。
入出力の検証や対応型を変えず、static local の初期化確認を繰り返す機会を減らす。

`final-cpu2/` は保存した変更前ヘッダを include overlay として使い、変更前後と
iostream / direct を同一 round に混ぜて比較する。CPU 2 は CPU 0 の SMT sibling ではない。
初回ヘッダ・runner・benchmark の snapshot も保存した。

## 最終結果と採用判断

`final-cpu2/` の本測定 360 sample と warmup 72 件、計 432 実行が全て期待出力と byte 単位で一致した。
ソース hash の終了時チェックも一致した。以下は操作時間の中央値、単位 ms。
通常ファイルから入力するケースと、通常ファイルへ出力するケースを示す。

| compiler | 操作 | iostream | accessor 削減前 | 採用版 | 直接 Fast I/O |
| --- | --- | ---: | ---: | ---: | ---: |
| GCC release | 整数入力 | 125.79 | 44.84 | 47.65 | 46.07 |
| GCC release | 整数出力 | 38.41 | 19.88 | 17.10 | 17.58 |
| GCC release | 整数入力→出力 | 179.81 | 73.81 | 68.74 | 73.17 |
| GCC release | double fixed 30 桁出力 | 75.36 | 14.52 | 14.92 | 14.42 |
| GCC assert | 整数入力 | 124.71 | 56.67 | 51.63 | 49.30 |
| GCC assert | 整数出力 | 38.47 | 18.77 | 18.86 | 17.85 |
| GCC assert | 整数入力→出力 | 181.57 | 80.70 | 80.50 | 84.23 |
| GCC assert | double fixed 30 桁出力 | 73.36 | 14.67 | 13.75 | 14.57 |
| Clang release | 整数入力 | 117.83 | 47.29 | 46.33 | 51.93 |
| Clang release | 整数出力 | 40.92 | 18.23 | 16.61 | 15.52 |
| Clang release | 整数入力→出力 | 182.65 | 77.74 | 69.45 | 74.29 |
| Clang release | double fixed 30 桁出力 | 75.35 | 15.21 | 15.60 | 15.19 |

GCC release のばらつきと pipe の結果も示す。数値は中央値 [最小, 最大]、単位 ms。

| 操作 | accessor 削減前 | 採用版 | 直接 Fast I/O |
| --- | ---: | ---: | ---: |
| 整数入力 / file | 44.84 [43.42, 49.59] | 47.65 [43.26, 52.04] | 46.07 [43.66, 51.47] |
| 整数出力 | 19.88 [16.28, 20.78] | 17.10 [16.39, 18.59] | 17.58 [15.94, 19.79] |
| 整数入力→出力 / file | 73.81 [71.31, 75.16] | 68.74 [65.64, 90.90] | 73.17 [67.30, 88.62] |
| 整数入力 / pipe | 57.64 [56.62, 66.20] | 63.42 [58.82, 68.85] | 59.42 [58.53, 66.64] |
| 整数入力→出力 / pipe | 88.36 [85.06, 122.00] | 90.74 [80.91, 102.81] | 90.25 [81.88, 106.46] |
| double fixed 30 桁出力 | 14.52 [13.10, 16.35] | 14.92 [13.70, 18.92] | 14.42 [13.21, 16.31] |

GCC release の採用版は、iostream に対して整数入力約 2.64 倍、整数出力約 2.25 倍、
入力→出力約 2.62 倍、double 出力約 5.05 倍の速度だった。
pipe では iostream の入力 149.09 ms → 63.42 ms、入力→出力 206.47 ms → 90.74 ms。
この倍率は今回の入力と環境に限る。

accessor 削減前後は改善と悪化が混ざり、最小・最大の区間も重なる。
キャッシュだけで常に高速化したとは判断しない。一方、全型の処理を保ち、
`scan` / `print` ごとに同じ参照を使う簡潔な実装で、直接 backend と同程度の時間に収まった。
そのためキャッシュ版を採用した。今後さらに軽量化する場合は、パイプライン全体や
単一コンテナだけに特化せず、同じ契約の直接 backend に対する差を再測定する。

GCC release の整数入力→出力のプロセス時間は iostream 181.25 ms、採用版 70.23 ms。
ピーク RSS はそれぞれ 3,984 / 3,992 KiB。整数出力だけは 8 MB の vector を持つため
11,388 / 11,152 KiB、double 出力だけは 5,456 / 5,432 KiB だった。
RSS の数十〜数百 KiB の差はプロセス初期化やページ配置も含み、buffer 自体の節約とは解釈しない。
入出力本体は 64 KiB ずつの固定 buffer を使用する。

## Fastest の調査

2026-10-20 に Library Checker の読み取り専用 REST API を
`scripts/fetch_lc_fastest.py many_aplusb --limit 3 --dedup-user --save-source` で取得した。
メタデータは `fastest.json` / `fastest.md`、調査したソースの hash は `source-hashes.json`。
第三者ソース本文は `.build/` にのみ置き、成果物にもライブラリにもコピーしていない。
取得した提出本文には明示的な利用許諾を確認できなかったため、方式を調査する資料として扱った。

- [409103](https://judge.yosupo.jp/submission/409103): Linux x86-64 / AVX2。
  regular file を mmap、pipe では全入力を読み取る。64-byte の余白を設けて SIMD load を使う。
  解法本体は十進の各桁を直接加算し、carry を bitmask で処理する inline assembly。
  一般的な整数へ parse して足し、整数を format するパイプライン自体を省いている。
- [402646](https://judge.yosupo.jp/submission/402646): canard の AVX2 decimal codec。
  regular file を mmap、single delimiter の trusted unsigned 入力をペアで decode し、
  8 件ずつ SIMD formatter で出力する。提出本体の `padded_file` は pipe fallback や入力検証を持たない。
- [407732](https://judge.yosupo.jp/submission/407732): canard の schema / SIMD pipeline。
  ソースには checked streaming 実装もあるが、提出で選ぶ optimized pipeline は
  regular file・trusted numeric record・AVX2 を前提とする。

これらの表示時間 18 / 23 / 24 ms は提出時の単一最大ケースであり、ローカルの集計と比較しない。
一般的な符号付き整数・EOF/不正 token 検出・任意の空白・pipe を保つ API と契約が異なる。
この作業では、既存の portable な fread/fwrite と二桁ずつの整数 format を再利用し、
同じ checked backend の直接呼び出しを基準に wrapper の費用を調べた。
入力全体の保持、unsigned 専用化、SIMD 必須化、十進加算専用化は採用しない。
大きい固定精度の浮動小数点 format は出力長に比例する作業が必要で、今回の測定は通常の 30 桁まで。

## 記録の見方

`initial-smoke-error.json` は最初の WSL 実行が Windows 形式の `.git` を解決できなかった記録。
コンパイル・実行に到達していないので、成功や 0 ms の測定値には含めない。
`validation/` の検証記録は実装全体の確認であり、この benchmark の速度 sample とは別に扱う。
