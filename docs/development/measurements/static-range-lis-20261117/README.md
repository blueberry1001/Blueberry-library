# Static Range LIS の一時配列確保

構築時、Monge積の入力列は長さNが決まっている。部分積の有効行・列の数も既存の逆写像を作るときに数えられるため、各一時配列の容量を先に確保する。子の順位列と位置列も、順位が0〜N-1の順列であることから、要素数をそれぞれ `floor(N/2)` または `ceil(N/2)` と求めて確保する。要素の並び、子の再帰順、重複値に対する逆順順位付け、計算量、APIは変えない。内部配列の容量は実装やライブラリに依存するが、使用領域の漸近上界は増やさない。

過去の [Fastest調査](../../fastest-other-structures-study.md) で確認した [331206](https://judge.yosupo.jp/submission/331206) と [393940](https://judge.yosupo.jp/submission/393940) は、全問い合わせ既知・公式の順列入力を前提としたオフライン方式である。[382253](https://judge.yosupo.jp/submission/382253) はMonge積とWaveletを用いるが、値域の固定など前提が異なる。今回の測定は汎用的なオンライン問い合わせAPIを維持した旧版・候補の比較であり、提出プログラムとの同条件比較や現在の最速の証明ではない。ACLに区間LISの同等機能はない。

## Windows Clang での前後比較

Windows 11、Intel Family 6 Model 183 Stepping 1、Unity Android NDK Clang 18.0.3、MSVC標準ライブラリ、`--target=x86_64-pc-windows-msvc -std=gnu++20 -O2 -DNDEBUG`。両者とも同じ旧WaveletMatrixの固定コピーを使用し、他作業中のWavelet変更は取り込まない。ランダム順列・昇順・降順・8値の重複をseed 20261117で生成し、各2万問い合わせを行う。入力生成は構築時間の計時外。各条件でwarmup 1回、実測5回。4候補の実行順は回ごとに循環・反転し、計時全体の親プロセスを利用可能CPUの最下位ビットに固定した。子プロセスは親の設定を継承するが、子ごとのmaskは別途確認していない。チェックサムは全候補で一致した。

構築の中央値、単位ms。query・プロセス全体も [results.json](results.json) に記録する。負荷・周波数の揺れが大きく、特にランダム入力で試行範囲が重なるため小さな差を断定しない。

| N | 分布 | 旧 | 子列のみ | 積のみ | 両方 |
| ---: | --- | ---: | ---: | ---: | ---: |
| 16,384 | random | 116.45 | 72.79 | 67.04 | 58.38 |
| 16,384 | ascending | 51.21 | 47.98 | 39.36 | 31.91 |
| 16,384 | reverse | 61.32 | 50.11 | 47.50 | 38.93 |
| 16,384 | duplicate | 58.77 | 57.32 | 51.14 | 41.71 |
| 65,536 | random | 435.78 | 434.91 | 419.65 | 354.27 |
| 65,536 | ascending | 271.80 | 322.08 | 197.65 | 177.27 |
| 65,536 | reverse | 250.89 | 260.26 | 212.98 | 207.30 |
| 65,536 | duplicate | 277.64 | 249.85 | 224.26 | 202.48 |

独立したメモリ計測では同じN=65,536・4分布で旧版と両方候補を各3回実行した。`GetProcessMemoryInfo` の peak working set 中央値は順に、random 9.63→8.71 MiB、ascending 9.36→8.39 MiB、reverse 9.70→8.64 MiB、duplicate 9.70→8.68 MiB。値にはプロセス・入力・ランタイムも含まれ、データ構造単独のメモリではない。全試行・引数は [memory.json](memory.json)。GCC・Linux・公式入力・I/O分離は未測定。

公開ヘッダを固定した最終再計時では、同じN=65,536・2万問い合わせ、1 warmup＋7回の構築中央値は random 268.29→212.91 ms、ascending 163.22→113.69 ms、reverse 170.35→118.86 ms、duplicate 207.04→146.11 ms。前表と異なる時刻での測定のため、数値を相互に比較しない。全生値・対象hashは [final-results.json](final-results.json)。固定した旧WaveletMatrixを両版で使い、結果hashは全試行で一致した。

## 正しさと再現

旧版と各候補はWindows Clangのassert有効 `gnu++20 -O2` で、既存random testをseed 1、17、731で実行した。公開ヘッダの最終版はさらに `gnu++20` / `gnu++23`、assert有効 / 無効の各構成で同じ3 seedを通過した。テストには空列、全区間、昇順・降順・重複、比較可能だがデフォルト構築できない型が含まれる。各実行は愚直なLISとの比較に成功した。初回は既存testが `<string>` を明示せず `std::stoul` を使うためWindows Clangでコンパイルに失敗し、[失敗ログ](compile-initial-failure.json)を保存した。再実行時は全候補に `-include string` を与えた。[compile.json](compile.json) と [final-validation.json](final-validation.json) にコマンドと終了コードを記録する。最初の測定版は本体と同じ処理順・式で、最終版では既存の前提assert・説明コメントを削除した。正式なLibrary Checker・GCC・sanitizer・`make test` / `make verify` / `make docs` の実行は別途必要。

[baseline.hpp.txt](baseline.hpp.txt)、[candidate.hpp.txt](candidate.hpp.txt)、[wavelet.hpp.txt](wavelet.hpp.txt) は測定時の固定コピー。記録JSONのSHA-256は測定時の実バイト列を対象とし、checkout時に改行が変化する場合は一致しない。[prepare-original.py.txt](prepare-original.py.txt) は測定時のスクリプト。[prepare.py.txt](prepare.py.txt) は保存済み固定コピーから同じ候補を作る再現用で、Windows固有のcompiler・SDKパスは環境に合わせて変更する。

再現する場合、保存した `bench.cpp.txt`、`prepare.py.txt`、`run.py.txt`、`memory.cpp.txt`、`memory.py.txt` をそれぞれ末尾の `.txt` を外して `.build/lis-reserve-study/` にコピーし、リポジトリルートで `python .build/lis-reserve-study/prepare.py`、`python .build/lis-reserve-study/run.py`、`python .build/lis-reserve-study/memory.py` を順に実行する。出力は同ディレクトリ内に書かれ、既存の同名JSONを上書きする。

公開ヘッダの検証と最終再計時は、上記の準備後に `final-test.py.txt` と `final-run.py.txt` も同じようにコピーし、`python .build/lis-reserve-study/final-test.py`、`python .build/lis-reserve-study/final-run.py` を実行する。前者は実行時の作業ツリーにある公開ヘッダを固定し、後者はその固定コピーと保存された旧版を比較する。別revisionでの再現には、対象ヘッダを確認してから実行する。
