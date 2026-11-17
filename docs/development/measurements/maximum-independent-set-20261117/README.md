# Maximum Independent Set の速度調査（2026-11-17）

頂点数 40 以下の最大独立集合について、半分全列挙の従来版、候補、Nyaan の実装を比較した。ACL には同じ機能がない。Nyaan の [ソース](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/graph/max-independent-set.hpp) と CC0 ライセンスは `nyaan.hpp`, `nyaan-LICENSE.txt` に保存した。Library Checker の最速提出一覧は取得できなかったため、Nyaan を最速と断定しない。

採用候補は連結成分ごとの枝刈り探索と、予算切れ時の全体に対する半分全列挙を組み合わせたもの。残余グラフの最大次数が 2 以下ならパスと閉路を直接解く。成分間で探索予算を共有し、上限は `min(2^18, 2^ceil(N/2))` とする。DP 状態には独立集合の大きさと選択マスクを詰めて、反復中の popcount と別配列へのアクセスを減らした。時間・追加メモリの最悪計算量と公開 API は従来どおり。残余グラフを成分に分け直すことはしていない。

## 測定

Windows 11 26200、Intel64 Family 6 Model 183 Stepping 1。MSVC 19.43（`/O2`）、Clang 18.0.3（Windows MSVC target、`-O2`）、C++20、`NDEBUG`。同じ入力を使い、各条件でウォームアップ 1 回と本測定 5 回を行い、実行順を交代した。表は本測定の中央値で、単位はミリ秒。1 セルは表の個数分のグラフの合計時間で、辺列からの隣接構造と返り値の構築を含む。入力生成と答えの検査は除く。Nyaan については辺列から隣接リストへの変換も含む。各実装が同じサイズの独立集合を返し、返却頂点に辺がないことを検査した。全サンプルとプロセスのピーク作業セットは `results-*.json` に記録した。

| コンパイラ | 頂点数 | 入力、個数 | 従来版 | 採用候補 | Nyaan |
| --- | ---: | --- | ---: | ---: | ---: |
| MSVC | 40 | パス、100 | 482.736 | 0.067 | 0.299 |
| MSVC | 40 | K5 8 個、非連結、100 | 472.566 | 0.073 | 140.453 |
| MSVC | 40 | K5 8 個を橋で連結、20 | 94.981 | 4.700 | 19.022 |
| MSVC | 40 | 辺確率 20% の乱数、20 | 96.832 | 0.883 | 79.323 |
| Clang | 40 | パス、100 | 617.510 | 0.065 | 0.304 |
| Clang | 40 | K5 8 個、非連結、100 | 605.663 | 0.065 | 95.809 |
| Clang | 40 | K5 8 個を橋で連結、20 | 120.685 | 3.081 | 12.887 |
| Clang | 40 | 辺確率 20% の乱数、20 | 123.494 | 0.591 | 55.425 |

表は `results-budget.json` の同一条件から抜粋した。小さい時間はノイズの影響を受ける。CPU アフィニティは固定しておらず、他の作業が同時に動いた可能性がある。表の測定中に 2 秒間隔で記録したシステム全体の CPU 使用率は 41 サンプル、最小 12.4%、中央値 17.5%、最大 27.7%。この値にはベンチマーク自身も含む。残りの測定期間の記録は `cpu-monitor-*.json` を参照。

全入力で高速になるわけではない。例えば MSVC の 20 頂点・連結 K5 群 100 個は従来 0.362 ms、採用候補 0.448 ms、Nyaan 0.682 ms。小さい入力の探索予算を 1/2、1/4、1/8 にする案も比べたが、20〜25 頂点の別の乱数入力で fallback が増えて悪化したため採用しなかった（`results-components.json`）。上限 2^16 と 2^17 は 40 頂点の連結 K5 群で fallback した。2^18 と 2^20 はその入力を探索で解き、連結 K4/K3 群や乱数では同程度だった（`results-budget.json`, `results-hard.json`）。2^18 を超える予算が必要な入力と、2^18 の上限に達してから fallback する場合の実測コストは見つかっていない。最速を保証するものではなく、測定外のグラフでは他の実装が速い可能性がある。

## 命令と検証

`assembly-chosen-results.json` と圧縮したアセンブリにコンパイラの出力を保存した。Clang の portable ビルドは POPCNT を出力せず、`-mpopcnt` 指定時には出力した。MSVC の portable 出力には CPU 機能による分岐と代替経路がある。CPU 機能を実行時判定して `target("popcnt")` へ分岐する候補も試したが、Clang は探索部に POPCNT を生成せず、判定をキャッシュしても測定上の改善を確認できなかったため採用しなかった（`assembly-target-results.json`, `results-budget.json`）。Windows Clang の `__builtin_cpu_supports` はリンクに失敗したため、CPUID を用いた試作と失敗ログも残している。公開 API は特定の CPU 命令を必須としない。

`validation-chosen.json` に MSVC C++20/latest、Clang gnu++20/23 の結果を記録した。固定 seed 1、17、20261117 について頂点数 0〜19 の全探索との比較を行い、40 頂点の空・完全・マッチング・偶奇閉路・完全二部・複数成分・連結 K5 群も検査した。`windows.h` を先に include する検査も通過した。2^18 の自然な予算切れは今回見つからなかったため、計測用の `instrumented.hpp` では初期予算を 0 にして fallback を強制し、4 構成で答えを検査した。変更前の自然な予算の計測用ヘッダは `instrumented-natural.hpp`。予算切れ入力の探索では、2,000 個ずつの K5 群、10,000 個の乱数グラフを試したが見つからなかった。失敗・未発見のログは成功扱いせず `*-failed.json` に保存した。

`baseline.hpp` は main `a74b453e7b61134c46e4bd44713fd6e085ea8411` の原本、`chosen.hpp` が公開候補の原本。`results-initial.json` と `results-degree2.json` は `bench-initial.cpp`、`results-final.json` は `bench-pre-components.cpp`、`results-components.json` と `results-budget.json` は `bench.cpp`、`results-hard.json` は `bench-hard.cpp` を使用。各 JSON にソース、テスト、コマンド、全サンプルのハッシュと結果を残した。`SHA256SUMS` は保存した各ファイルの元バイト列の SHA-256。比較用スナップショットの改行を Git で変換しないよう、このディレクトリを `-text` にする。GCC・Linux・Library Checker の実行結果はこのローカル測定に含まれない。
