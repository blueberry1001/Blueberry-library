---
title: Math batch two — candidate comparison
---

## 調査と採用

2026-09-18 に公開一次実装を読んで計算量・境界処理・データ構造を比較し、
コードを転記せず必要なアルゴリズムを独立実装した。ACLと重複しない4ヘッダを追加し、
離散対数の逆元のみACLに委譲する。

| 対象 | 調査した一次実装 | 比較・判断 |
| --- | --- | --- |
| 整数k乗根 | [ei1333](https://ei1333.github.io/library/math/number-theory/kth-root-integer.hpp.html)、[maspypy](https://github.com/maspypy/library/blob/main/nt/integer_kth_root.hpp) | 固定32ビット試行と浮動小数点推定後の補正を調査。浮動小数点への依存を避け、bit_widthによる上限の二分探索を採用。積の比較は除算で保護する。 |
| 商列挙 | [ei1333](https://github.com/ei1333/library/blob/master/math/number-theory/enumerate-quotients.hpp)、[hitonanode](https://github.com/hitonanode/cplib-cpp/blob/master/utilities/quotients.hpp) | 商が一定の区間を飛ぶ方法と平方根で分割する方法。どちらもO(sqrt(N))。値の列挙だけを公開し平方根分割を採用。区間終端N+1を作らず符号付き64bit最大値まで扱う。 |
| 約数・倍数変換 | [Nyaan](https://nyaannyaan.github.io/library/multiplicative-function/divisor-multiple-transform.hpp.html)、[suisen](https://suisen-cp.github.io/cp-library-cpp/library/convolution/gcd_convolution.hpp.html) | 素数ごとの変換はO(N log log N)、全約数走査はO(N log N)。素数判定と変換を同じループにまとめる篩を採用。 |
| 離散対数 | [KACTL](https://github.com/kth-competitive-programming/kactl/blob/main/content/number-theory/ModLog.h)、[maspypy](https://github.com/maspypy/library/blob/main/nt/discrete_log.hpp) | KACTLの非可逆部分の判定、maspypyのモノイド作用汎用化を調査。法のgcd除去後に通常BSGSを行う短い数値専用APIとした。unordered_mapの期待O(sqrt(M))を選び、ordered map O(sqrt(M) log M)とも測定した。 |

## 同条件の測定

ベンチマークソースは `benchmark/math-batch-two.cpp`。
[生ログ](math-batch-two-benchmark.log)にはCPU・OS・GCCバージョン・フラグ・全5回の時間を保存した。
GCC 13.3.0、WSL Ubuntu、`-std=gnu++20 -O2 -Wall -Wextra`、seed=20260918。
各組の入力は同一で、checksumも一致した。共有ホスト上で他作業の負荷は存在しうる。
既存公開コードそのものの性能ランキングではなく、独立実装した候補の比較である。

| 入力 | 採用候補の中央値 / 最小 ms | 比較候補の中央値 / 最小 ms |
| --- | --- | --- |
| root: ランダムuint64 20,000件、k=2..64 | 範囲二分探索 1.065 / 1.040 | 32ビット試行 7.285 / 6.946 |
| quotient: N=10^10 | 平方根分割 1.428 / 1.350 | 区間ジャンプ 2.395 / 2.342 |
| divisor: 長さ1,000,001 | 素数篩 11.214 / 8.655 | 全約数走査 22.361 / 22.004 |
| log: M=1,000,000,007、底3、異なる目標10件 | hash BSGS 16.384 / 15.713 | ordered map BSGS 64.163 / 59.316 |

数論用標準演算はint64の範囲内、整数根はuint64全域を扱う。
境界・小さい全列挙・固定seedランダム比較は `tests/random/math-batch-two.cpp`。
公式ドライバは gcd_convolution、lcm_convolution、kth_root_integer、
enumerate_quotients、discrete_logarithm_mod の5問題に対応する。
公式実行結果は `.verification/batch-two-math` に別途保存する。生成や実行に失敗した記録を成功とみなさない。
