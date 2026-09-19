# Fast I/O: 構造と入出力を分けた比較

旧SqrtTree（`e0d8e71`）/新SqrtTree × unsynced iostream/Fast I/Oを、
同じStatic RMQ公式入力で比較する。両方の構造に同じFast I/Oを使い、
構造の改善とI/Oの改善を分離する。第三者提出のソースは含めない。

```sh
python3 docs/development/measurements/generic-performance/io/run.py \
  --problem-dir /path/to/library-checker-problems/data_structure/staticrmq \
  --output .build/io-new-run
```

入力は公式generatorが生成済みの`in/*.in`と`out/*.out`を使う。
GCC `-O2 -DNDEBUG`、GCC `-O2 -D_GLIBCXX_ASSERTIONS`、Clang `-O2 -DNDEBUG`で、
`max_random_00` / `small_width_query_00` / `small_values_00`を測る。
C++はgnu++20。同一の論理CPUに固定し、各条件warmup 1回+5回を逐次実行する。
他のコンパイル・benchmarkとは同時に実行しない。

stdinは通常ファイル/pipeの両方、stdoutは通常ファイル。各回の出力を公式期待出力と全件照合する。
`total_ms`は入力・構築・query入力/出力・flushを含む。`process_ms`は起動・破棄・time wrapperも含む。
`input_ms`は初期配列の入力、`build_ms`は構築、`query_io_ms`はquery入力+prod+結果出力/flush。
RSSはプロセス全体の最大値。構造だけのquery費用は別のRMQ benchmarkを参照する。

全sample、warmup、compiler/flags、CPU、入力・ソースhashを保存する。
既存の出力先には上書きしない。失敗時の診断を残し、欠損を0msで補わない。
この測定の値をLibrary CheckerのFastest表示値と同一環境の比較として扱わない。

`interactive.cpp`は`FastInput<true>`を使う。stdinを閉じずに1要求ずつ送り、
返答を受け取ってから次の要求を送ることで、64 KiB/EOF待ちを起こさないことを確認する。
部分応答・応答なし・EOFの失敗も5秒以内に検出する。
整数型ごとの境界・不正token・FILEエラー・float/文字列は`tests/random/fast-io.cpp`で検証する。

## 採用結果

`results/`に360測定・72warmupを保存し、全432実行で公式期待出力と一致した。
GCC 13.3 / Clang 18.1.3、WSL2 / Core i7-14650HX、CPU 0。
`interactive.json`の3設定とも、stdinを開いたままの2回の対話に成功した。

公式`max_random_00`（N=Q=500,000）、GCC `-O2 -DNDEBUG`の中央値、単位ms。

| 構造 | I/O | stdin | 入力 | 構築 | queryとI/O | 合計 |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| 旧 | iostream | file | 14.77 | 11.06 | 47.38 | 73.07 |
| 新 | iostream | file | 14.33 | 8.75 | 49.44 | 73.50 |
| 旧 | Fast I/O | file | 4.54 | 11.24 | 19.01 | 34.87 |
| 新 | Fast I/O | file | 4.65 | 8.47 | 18.52 | 31.31 |
| 旧 | iostream | pipe | 17.30 | 10.83 | 55.27 | 84.51 |
| 新 | Fast I/O | pipe | 6.88 | 8.51 | 22.50 | 38.16 |

各列の中央値を独立に取るので、その和が合計列の中央値と完全に一致するとは限らない。
fileのプロセス時間は旧+iostream 75.13→新+Fast I/O 33.17ms。
Clangの同じfile条件では合計76.02→35.88ms、GCC assert有効では79.07→37.84msだった。
I/Oのみ変えた旧Sqrtでも大きく短縮しており、この差全体をSqrtTreeの高速化とは呼ばない。
新Sqrt+iostreamのGCC中央値はほぼ同じで、入力/output費用が構築短縮を上回る条件も示している。

`validation.sh` / `validation.log` / `validation.json`はGCC/Clang×gnu++20/23の各5seedと
Clang ASan/UBSan seed17の成功記録。文字列literalのbool誤変換を防ぐ制約、整数全幅、
128bitの端点、float/double/long double、埋め込みNUL、buffer境界、FILEエラーを含む。
