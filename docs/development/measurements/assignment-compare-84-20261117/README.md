> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# Assignment の PR #84 比較

並行作業中の [PR #84](https://github.com/blueberry1001/Blueberry-library/pull/84) head `9453928201ceae304aad81d77f95ddbf992cd2f3` のヘッダ（Git blob `d0f89a1c731e3d12db552677024cb3976fb0a3ee`、保存原本 `pr84.snapshot.hpp`）と、別経路の候補（`ours.snapshot.hpp`）を直接比較した。前者は N≥64、M≤8N と値域 guard 下の最短増加路、後者は幅広整数の正方 LAPJV・矩形最短増加路で、両方とも矩形・汎用型を維持する。

`combined-v4.snapshot.hpp` は両経路を組み合わせた検証用原本。幅広経路が使えない場合に #84 の安全値域 guard と最短増加路を使い、それも使えなければ元版に戻す。MSVC の64bit入力では、int32値域と #84 guard を同一走査で確認する。N=1 は行最小、N=2 は2行目の最小・次点から各列の最適解を調べる。和の比較は符号拡張上位部と uint64 下位部の組で行い、候補和が64bitを越えても signed overflow を起こさない。戻す総和は既存 API の T に収まる前提を使う。浮動・unsigned・独自型は元版経路。

## 同じ入力での測定

Windows 11、Intel Family 6 Model 183、Windows Clang 18.0.3 / MSVC 19.43、C++20、`-O2 -DNDEBUG` 相当。全候補に同じ `vector<vector<long long>>`、同じ値と driver を渡した。入力生成・I/O は計時外、構築・探索・結果生成は計時内。子プロセスは論理 CPU 8（affinity 0x100）に固定、warmup 1回・7回中央値、順番を入れ替えた。CPU 負荷は別プロセスの影響を排除できず、特に短い入力の小差は不確実。値は ms。

| Clang、形状・内容 | 旧版 | #84 | 元候補 | 統合 |
| --- | ---: | ---: | ---: | ---: |
| 250×250 一様乱数、3回 | 7.467 | 3.238 | 1.420 | 0.788 |
| 250×250 値域 {-1,0,1}、3回 | 43.646 | 20.128 | 0.384 | 0.326 |
| 400×400 一様乱数、2回 | 15.052 | 6.103 | 1.656 | 1.492 |

| MSVC、形状・内容 | 旧版 | #84 | 元候補 | 統合 |
| --- | ---: | ---: | ---: | ---: |
| 250×250 一様乱数、3回 | 8.654 | 3.873 | 1.047 | 1.062 |
| 250×250、int32域外の一様乱数、2回 | 5.420 | 2.559 | 5.176 | 2.361 |
| 64×512、末尾だけ #84 guard 外、10回 | 0.848 | 1.026 | 0.977 | 0.994 |
| 64×513、末尾だけ guard 外、10回 | 0.741 | 0.749 | 0.909 | 0.829 |

最後の行では、統合版も元版より遅い。64bit値域の判定後に旧経路へ戻るためで、末尾まで外れ値がないと1回余分に行列を走査する。常に高速とは限らない。`comparison-all-v4.json.gz` は全入力と各回の記録。

N=2 の非常に横長な入力は専用経路で改善した。2×50000、末尾だけ int32 域外、4回の中央値は Clang #84 1.851 ms / 専用経路なし 2.954 ms / あり 0.294 ms、MSVC 1.995 / 2.080 / 0.363 ms。同じ版の他の大小・構造ケースと各回は `comparison-tiny.json.gz` にある。追加の超横長入力については `comparison-extended.json.gz` に専用経路導入前の試行も残す。

M>8N の入力をすべて旧経路へ回す案も比較した。MSVC 64×2048 の末尾逸脱は統合 0.736→旧経路 0.659 ms と改善した一方、同じ形状の低値域は 1.292→22.598 ms、32×8192 の乱数は 0.874→2.161 ms と大幅に悪化した。Clang でも同様に低値域が遅いため、この gate は採用しない。`comparison-gate.json.gz` に全試行を保存した。

Library Checker の公式14入力も旧版・#84・統合版で比較した。固定版問題 repo の公式 input hash と全生成入力が一致し、3版の返却する置換と総費用が公式解答に一致した。CPU8・Clang、起動と I/O を含む3回中央値では hand_plus は旧 243.61 / #84 194.22 / 統合 130.49 ms、multiplication_table は 246.18 / 183.55 / 131.90 ms。max_random_02 は 162.84 / 158.99 / 167.82 ms で、入力・環境による変動がある。`official-compare-results.json.gz` に全出力検証と各回を保存した。

## 検証

- `boundary-results-v2.json`：N=63/64/65、M=8N/8N+1、guard の正負境界、旧方式だけで収まる32bit overflow 反例、浮動・unsigned・独自型、N=4097 の経路切替を Clang / MSVC × C++20 / C++23 × debug / release で確認。Clang は `-Wall -Wextra -Wshadow -Werror`。
- `tiny-validation-results.json`：Python 任意精度で作った int64 6,461・int32 6,572・signed char 6,472 の計19,505行列を同じ8構成で確認。各型の極値、候補和の overflow、同点を含む。
- `independent/tiny-results.json`：別担当が LLONG_MIN/MAX・±2^62・±1・0 を含む9値の2×2/2×3全組合せを調査。最適和が int64 に収まる287,641件は Clang C++20 debug/release の両方で `__int128` brute force と一致し、残る250,392件は戻り値が型の範囲外のため除外した。
- `integrated-test-results.json`：公開用の小規模 brute force・元版との overflow 反例・特殊型・2行の極値を、同じ8構成 × 3 seed で確認。
- `both-tests-results.json`：統合ヘッダと、新規の `assignment-lapjv.cpp`・main の #84 `assignment-fast.cpp` を同じ8構成 × 3 seed で確認。新規テストには N=4097 の経路切替も含む。使用したファイルは `fixture/` に保存。
- `comparison-*.json.gz`：比較 driver は小矩形の brute force、64/65・8N境界、大きな値でも旧版と比較し、各 benchmark の総費用一致を確認した。

測定コードと入力生成を含む元ログは `gzip -dc comparison-all-v4.json.gz` のように展開できる。`raw-logs.sha256` は圧縮前、`SHA256SUMS` は保存ファイルの SHA-256。再測定はこのフォルダを別の書き込み可能なディレクトリにコピーして `python run-all-v4.py` 等を実行し、既存ログを上書きしない。両テストは `python replay-both-tests.py` で `fixture/` を一時領域へ複製して再実行でき、結果 JSON はカレントディレクトリに一意な名前で保存する。Windows の長い一時パスでも MSVC が参照できるよう、複製したテストの旧版ヘッダ include のパスだけを短くしている。runner 内の Visual Studio・Windows SDK・Clang のパスは実行環境に合わせる。

初回 wrapper では長い一時パスのまま MSVC を実行し、旧版ヘッダが見つからず失敗した。最初の一時ログは sandbox 終了時に消えたため、同じ失敗を記録した `replay-attempt-v2.json` を残す。include 短縮後の成功記録は `replay-attempt-v3.json`、結果 JSON の保存先を明示した最終版の成功記録は `replay-attempt-v4.json` と `replay-confirmation.json` である。
