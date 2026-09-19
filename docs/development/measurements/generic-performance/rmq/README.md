# 汎用 static range 構造の内部改善

元版は `e0d8e71`。独自の測定器だけを収録し、第三者提出のコードは含めない。
公開 API・任意型・非可換演算・空区間の契約は変更しない。

## 採用する内部変更

- **LinearRMQ**: ブロック幅を Θ(log N) の範囲の2冪へ切り上げ、除算を shift に変更。
  左端・中央・右端の順に候補を集約するため、同値は左を取る1比較でよい。
  左端 tie と任意の strict weak ordering を保ち、厳密な線形構築・空間も保つ。
- **SqrtTree**: prefix/suffix を実際の N 要素にし、末尾 padding 用入力コピーと
  存在しない要素の演算を省く。between は実際に問い合わせる中間ブロックの組だけを計算し、
  中間ブロックを持たない層には割り当てない。親の mask・子ブロック数の mask/shift を
  Layer に保持し、query ごとの算出を省く。任意 monoid・既定構築不可の S も維持する。
- **DisjointSparseTable**: 所有入力を第0層と共用し、N 要素分の重複コピーを省く。
  query に新しい分岐は追加しない。単位元不要の任意 associative operation を保つ。

## 比較条件と生ログ

`environment.json` に CPU/OS、GCC/Clang バージョン、全 flags、固定 logical CPU、
入力生成 seed、harness/header SHA-256 を保存した。
同じ条件の前後を交互に実行し、各1回の warmup と5回の測定を保存する。
CSV の `run=0` は warmup、summary は `run=1..5` の中央値・最小・最大。
入力・問い合わせ生成を計測外とし、構築と操作を別々に計測する。これは I/O の速度比較ではない。
RSS は入力・問い合わせ・標準ライブラリを含む**プロセス全体**の最大値。
全ペアで checksum を照合し、benchmark 中の header hash 不変も確認している。
公式 checker と任意型の正しさの検証は、別の verify/random test が担当する。

| ディレクトリ | 対象 | warmup込み実行数 | 採否 |
| --- | --- | ---: | --- |
| `first` | 最初の3構造候補 | 1128 | Sqrt の構築削減は維持。DST/Linear の query 回帰候補は不採用 |
| `dst-final` | 第0層共用 DST | 432 | 採用 |
| `linear-final` | 区間順を保証する LinearRMQ | 264 | 採用 |
| `sqrt-recheck` | 構築だけを変更した Sqrt、Q=5,000,000も追加 | 396 | query 回帰が再現、さらに改善 |
| `sqrt-layer` | Layer情報を前計算した Sqrt、Q=5,000,000も追加 | 396 | 採用候補の操作回帰を再確認 |
| `sqrt-final` | 同じ最終 Sqrt、主要代数/short/Q=100も確認 | 432 | 最終確認 |
| `sqrt-last-recheck` | 最終確認で幅が大きかった2条件とQ=5,000,000 | 144 | 再現性を追加確認 |

合計3,192実行（warmup 532回、測定2,660回）。

各ディレクトリは異なる実行バッチであり、別バッチの前後を混ぜて改善率を作らない。
途中候補のソースも `.hpp.txt` として残している。
全条件を GCC `-O2 -DNDEBUG` で、代表条件を Clang `-O2 -DNDEBUG` と GCC `-O2`
で確認した。C++ は gnu++20。N=500,000/524,289、Q=100/500,000、追加 Q=5,000,000。
代数は uint64_t の min/sum、mod 2^64 の非可換 affine/2x2 matrix。
LinearRMQ は random/short/equal/ascending/descending の argmin を測る。
特に N=524,289 は2冪を少し超え、padding 削減が大きく効く条件なので、N=500,000 と区別する。

## 回帰を確認して変更した点

最初の DST は第0層を省く代わりに query の特殊分岐を増やし、短区間で遅くなった。
所有入力を第0層として共用する設計に変更し、分岐を除いた。

最初の LinearRMQ は汎用 `best` 内で候補添字を並べ替えていた。
Clang の descending query が27〜34%遅くなったため、そのまま採用せず、
query の集約順を左→中央→右へ変更して並べ替えを削除した。
最終22条件は全て query 中央値が元版を下回った。
これは比較済み条件の結果であり、全入力に対する速度保証ではない。

構築だけを変えた Sqrt は、Clang/affine/N=524,289/Q=500,000 の query が
再測定でも約14.6%遅くなった。Q=5,000,000でも約7.6%遅く、構築込みは改善したが、
この回帰を理由に Layer 情報の前計算も追加した。
同じ条件の最終 query 比はそれぞれ約1.03/1.00。GCC の Q=5,000,000 は約0.95。
操作側の小さい変動を一律に高速化と呼ばず、構築・メモリ改善を分けて報告する。

`sqrt-final` では Clang/matrix/N=500,000 と GCC/assert/min/N=524,289 の
Q=500,000 query 中央値に約15.6%/16.9%の増加もあった。
前者のサンプル範囲は旧16.95〜23.61ms、新17.26〜26.68msで重なっている。
同一ソースの `sqrt-last-recheck` では約0.974/1.007倍となり、大きい差は再現しなかった。
Q=5,000,000では約0.988/1.042倍だった。
一方この追加バッチでも他の Q=500,000 条件には約10〜17%の中央値増加があり、
特定の再測定だけを取り出して全条件の query 改善を主張しない。
query の小さい差には実行変動が残る。Sqrt の採用根拠は、任意型を保った明確な
前処理演算・padding・領域削減と、構築込み/問い合わせ主体の条件を含む全結果である。

## 代表値

各行はその行で指定する同一バッチ内の中央値。時間単位は ms。

| バッチ・条件 | 構築 before→after | query before→after | peak RSS KiB before→after |
| --- | ---: | ---: | ---: |
| linear-final/GCC/min/random/N=Q=500,000 | 6.33→5.53 | 16.38→8.92 | 20,432→18,604 |
| linear-final/Clang/min/random/N=Q=500,000 | 7.19→6.13 | 14.47→7.47 | 20,440→18,556 |
| linear-final/GCC/min/short/N=Q=500,000 | 6.33→5.46 | 9.26→3.82 | 20,484→18,512 |
| dst-final/GCC/min/random/N=Q=500,000 | 27.38→26.31 | 5.69→5.34 | 89,192→85,264 |
| sqrt-layer/GCC/affine/random/N=524,289/Q=5,000,000 | 59.01→28.23 | 91.66→87.05 | 324,984→193,836 |
| sqrt-layer/Clang/affine/random/N=524,289/Q=5,000,000 | 68.80→33.49 | 130.85→130.54 | 325,004→193,880 |

DST の主な確実な改善は N 要素の重複メモリ削減。
微小な時間差まで全環境で再現すると主張しない。
初回コンパイルでは `bit_width` の戻り型と test の空 vector 初期化の曖昧さを修正した。
コンパイル失敗を測定値に含めたり0秒として扱ったりしていない。

## 再現

リポジトリ直下で、元版の3ヘッダを別 include root に保存する。

```sh
mkdir -p .build/perf-baseline/blueberry/data-structure
for name in sqrt-tree disjoint-sparse-table linear-rmq; do
  git show e0d8e71:blueberry/data-structure/$name.hpp > .build/perf-baseline/blueberry/data-structure/$name.hpp
done
python3 docs/development/measurements/generic-performance/rmq/run.py --output .build/rmq-new-run
python3 docs/development/measurements/generic-performance/rmq/run.py --sqrt-recheck --output .build/rmq-query-heavy-new-run
```

出力先が既にあれば失敗し、元の証拠を上書きしない。
Linux の `taskset` と `/usr/bin/time`、GCC、Clang、Python3 が必要。
`--structure dst` / `linear` / `sqrt` で対象を限定できる。
実行ファイルは出力先に置くが Git には追加しない。

最終 header の契約テストは `tests/random/static-rmq-contracts.cpp` と既存
`tests/random/sqrt-tree.cpp`。GCC/Clang × gnu++20/23 × seed 1..10 を実行し、
Clang ASan/UBSan の seed 17 も両方成功した。実行コマンドは `validation-final.log` に保存。
strict weak ordering の同値でも別 identity を持つ非既定構築型、逆順 comparator、
非可換 vector 連結、空入力、二冪前後を検証する。
