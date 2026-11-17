> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# GCD / LCM 畳み込みの性能調査

`blueberry/math/divisor-convolution.hpp` の変更前と変更後を比較した。一般の環 T、両変換と添字0の契約を維持する。長さ512以上では2系列のゼータ変換を同じ素数走査にまとめ、素数列を逆変換でも使う。小さい入力は旧経路のままとした。追加メモリは O(N) のままだが、素数列の保持分は増える。

## 比較対象

- 旧版と採用版はそれぞれ `baseline.hpp` と `final.hpp`。公開ヘッダの本体と同じである。
- [Nyaan の倍数・約数変換](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/multiplicative-function/divisor-multiple-transform.hpp) と [GCD 畳み込み](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/multiplicative-function/gcd-convolution.hpp) を同じ係数型で実行した。`nyaan.hpp` は素数列挙と依存コードを結合した原本で、同梱の `nyaan-LICENSE.txt` は CC0。Nyaan 版の添字0は積のままなので、比較時のみ Blueberry と同じ零元に直す。
- 2026-11-17 に確認した [Library Checker 最速提出 #409724](https://judge.yosupo.jp/submission/409724) は 13 ms。[著者の実装・測定](https://github.com/7ue9ueue/lc-opt/tree/main/problems/convolution/gcd_convolution)は固定 mod 998244353・N≤10^6・AVX2・専用 I/O を前提とし、小さい素数の変換や2系列の入力・積・逆変換を融合し、大きい素数は wheel と分割走査でキャッシュを利用する。汎用 API と型・上限・I/O が異なるため、この提出時間と以下の操作時間を直接比較できない。固定上限や特定 ISA を一般 API に持ち込まず、共通走査と一時データの再利用を採用した。

## 同条件の測定

Windows 11、Intel Family 6 Model 183、MSVC 14.43.34808 / Windows Clang 18、`-O2 -DNDEBUG` 相当、各入力・実装で warmup 1 回、順序を交代して5回計測した中央値。CPU 固定なしで、他処理の負荷は分離できない。時刻は呼び出し時の vector コピー・確保・計算・結果の checksum を含み、事前の入力生成と標準入出力は含まない。N は vector の長さで、整数 mod 998244353 の同一係数型を使用した。

| 条件 | 旧版 | 採用版 | Nyaan |
| --- | ---: | ---: | ---: |
| Clang GCD、N=1,000,001 | 17.868 ms | 14.434 ms | 16.754 ms |
| Clang LCM、N=1,000,001 | 19.046 ms | 15.511 ms | 未測定 |
| MSVC GCD、N=1,000,001 | 47.010 ms | 44.829 ms | 49.953 ms |
| MSVC LCM、N=1,000,001 | 47.649 ms | 42.717 ms | 未測定 |
| Clang GCD、N=512、1000回 | 4.316 ms | 3.599 ms | 4.901 ms |
| Clang GCD、N=128、5000回 | 5.588 ms | 6.000 ms | 9.148 ms |

512未満は旧経路を使うが分岐などの影響を含み、N=128の例は回帰している。`results.json` は各試行と checksum、`results1.json`〜`results5.json` は試作・分岐境界の比較を保存する。N=1,000,001 における `uint64_t` の再測定は MSVC GCD 21.915→21.980 ms で、変化を確定できない。先行測定 21.451→22.513 ms を含め `u64-results*.json` に残す。

公式 `max_random_00` を Windows Clang で読み、結果を同じファイルへ出力する実行全体は5回中央値で GCD 384.954→380.596 ms、LCM 386.380→383.410 ms。scanf / printf の入出力とプロセス起動を含み、小差はノイズと分離できない。`end-to-end-results.json` に各回を保存する。プロセスのピークメモリは未測定。長さ1,000,001の場合、採用版の素数配列は `size_t` 78498個以上を保持する。

## 正しさ

固定した Library Checker problems commit `73876f1cbcf082d2b22722924b924d3d65a2f00e` から GCD / LCM の各29ケースを生成し、入力と、公式 `correct.cpp`・旧版・採用版の出力を公式 `hash.json` に照合した。Windows 出力の改行のみ LF に正規化し、全58ケースが一致した。生成・実行コマンド、終了コード、hash は `official-results.json` を参照。

`tests/random/divisor-convolution-fused.cpp` は非可換環の2×2行列で直接和と比較し、空入力、添字0、長さ511・512・513を含む。MSVC C++20 / C++latest と Clang gnu++20 / gnu++23 で固定seed 1・2・17 が成功した。`matrix-results.json` に記録。汎用型の旧版との差分テストも `results.json` に含む。Linux GCC / Clang と sanitizer はローカル未実行。

独立レビューでは二項演算子を持たず複合代入のみを持つ非可換環でも旧版・候補・愚直解を比較した。コマンドと結果は `independent-review/` に保存した。

## 再現

`python docs/development/measurements/divisor-convolution-20261117/replay.py` は `.build` 配下に時刻付きコピーを作り、上記の操作時間、uint64、行列、公式ケース、I/O込み比較を順に実行する。記録済みファイルは上書きしない。Visual Studio・Windows SDK・Clang のパスは各 runner に明記してあり、異なる環境では変更する。公式問題 repo は環境変数 `LIBRARY_CHECKER_PROBLEMS` または runner の既定パスに置く。各実行の失敗・欠測は成功やゼロに置き換えない。
