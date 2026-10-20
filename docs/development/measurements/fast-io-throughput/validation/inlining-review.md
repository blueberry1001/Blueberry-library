# GCC direct 入力の回帰とインライン判断

採用版 v5（`8729666`、header SHA-256 `8f08950cfc12f2babdd0eaa6ed0b38ce90e999ce7e24ee5105cdbb4cfc7ad406`）を維持する。
追加した `always_inline` と 4 桁 SWAR は `.build` 内の実験だけにとどめ、どちらも採用しない。
ライブラリ本体と正式 benchmark runner は変更していない。

## 調査対象

正式測定の GCC / direct / `digit_random_00` は、入力単体で file 26.06→28.51 ms、
pipe 33.21→39.20 ms の回帰が残った。同じ pipe の入力→出力は 50.07→52.76 ms。
global / file の同入力は 31.81→29.06 ms で、呼び出し方によって結果が異なる。
正式値は `../final-official64/` に保存してある。

GCC 13.3、gnu++20、`-O2 -DNDEBUG -DIO_FACADE=0`、同じ
`benchmark/fast-io-throughput.cpp` から baseline と v5 の assembly と
`-fopt-info-inline-optimized-missed` を取得した。コード生成は CPU 4 で行い、
以下の追加実験は正式測定と同じ CPU 2 に固定した。
コマンドは `inlining/assembly-commands.txt`、解析値は `inlining/analysis.json` に保存した。

## 確認できたこと

`FastInput<false>::read<long long>` は **baseline と v5 の両方で別関数として残る**。
両方とも benchmark の `run<long long>` から static call site が 6 個あり、
入力ループでは 1 組につき 2 回呼ぶ。`next()` / `first()` は読み取り関数内へ inline されている。
GCC のログは両方で `max-inline-insns-single limit reached` を示す。
したがって「v5 だけ関数呼び出しが増えた」ことは回帰の説明にならない。

| `read<long long>` の生成コード | baseline | v5 |
| --- | ---: | ---: |
| 関数の機械語サイズ | 1,213 B | 1,994 B |
| static 命令数 | 246 | 406 |
| static 条件分岐数（無条件 jump を除く） | 37 | 59 |
| 保存レジスタの push 数 | 6 | 6 |
| push とは別の stack 予約 | 8 B | 40 B |

これらは **関数全体の static 数**であり、1 token ごとの実行命令数や分岐数ではない。
refill・EOF・不正 token 処理も含む。`fread` / `ferror` の call site が 6→8 個になっていることも、
実行時の I/O 回数が同じ割合で増えたことを意味しない。
関数の抽出 assembly は `inlining/baseline-read-i64.s.txt` と `inlining/current-read-i64.s.txt`。

v5 は 1 桁専用判定の後、8 桁 block を試し、失敗した token を scalar で処理する。
`digit_random_00` の 2,000,000 token のうち、2–7 桁は 666,392 個（33.32%）、
1 桁は 110,944 個（5.55%）。2–7 桁では block の試行費用を払っても 8 桁まとめ処理の利益を得られない。
assembly でも、block 試行に入る際の 64bit mask 定数のロード、
レジスタから stack への退避、digit mask の検査を確認できる。
長い入力での利益と短い token での余分な処理、関数本体の拡大・register pressure が
観測された回帰と整合する。hardware counter を取得していないため、寄与率までは断定しない。

## 最後に評価した 2 候補

baseline・v5・以下の 2 overlay を同じ GCC オプションでビルドした。
入力は `/tmp` の同じ通常ファイルまたは pipe。各条件 warmup 1 回＋5 回を交互に実行した。
両公式ケースについて、全 variant の入力→出力を公式期待出力と byte 単位で照合した後、
入力単体を checksum 付きで測った。追加実験の全 96 実行でも checksum が一致した。

- `always-inline`: 整数の `read(T&)` に `__attribute__((always_inline))` を付けるだけ。
- `word4`: 64bit 以下だけ uint32_t の 4 桁 SWAR / base 10,000 を使い、128bit は既存の 8 桁処理を維持する。
  32bit 即値 mask による定数ロードと register pressure の軽減を狙った。

以下は追加実験内の入力単体中央値、単位 ms。この時点では root の全体テストが並行しており、
最大値に大きな外れ値もある。正式測定と混ぜず、この実験内の比較として扱う。

| 入力 / stdin | baseline | v5 | always-inline | word4 |
| --- | ---: | ---: | ---: | ---: |
| digit_random / file | 28.38 | 31.81 | 30.70 | 31.07 |
| digit_random / pipe | 34.97 | 42.80 | 44.45 | 39.86 |
| max_random / file | 33.89 | 24.19 | 21.85 | 32.77 |
| max_random / pipe | 46.77 | 37.80 | 38.96 | 47.20 |

`always-inline` は短い token の回帰を一貫して解消せず、pipe では v5 より遅い中央値になった。
実験用の全型・全 mode を含む実行ファイルも 52,064→60,288 B に増えた。
これは実行ファイル全体のサイズで、単一関数のサイズではない。
強制 inline を標準実装へ追加する根拠にはしない。

`word4` は digit_random で小さな差にとどまる一方、max_random / file は
v5 の 24.19→32.77 ms、pipe は 37.80→47.20 ms になった。
長い整数の処理単位を半分にする不利益があり、採用しない。
これ以上の候補追加や再測定は行わず、正式測定で得た global API・長い 64bit・128bit の改善と、
残る direct / 短い token の tradeoff を報告する。

## 保存資料

`inlining/` に追加実験の environment / compile / raw samples / summary、
解析スクリプト、抽出 assembly、GCC inline ログ、2 overlay の header snapshot を保存した。
追加実験の`current_header`はPythonの`read_text()`でLFへ改行正規化した内容のhashであり、
`current-lf-fast-io.hpp.snapshot`と一致する。コンパイル元のCRLFファイルは正式測定と同じ
`8f08950cfc12f2babdd0eaa6ed0b38ce90e999ce7e24ee5105cdbb4cfc7ad406`で、原本の変更はない。
正式値は変更していない。`.o` や実行バイナリはこの成果物へコピーしていない。

先行した試作では、出力名と overlay ディレクトリ名が衝突してコンパイルが一度失敗した。
修正後の予備測定は `/mnt/c` のファイルを直接開いており、正式測定の `/tmp` と条件が異なったため、
採否判断には使わない。その記録は `.build/fast-io-throughput/inlining-review/` に残し、
上表には修正した `two-candidates.py` の結果だけを用いた。失敗を 0 ms として補っていない。
