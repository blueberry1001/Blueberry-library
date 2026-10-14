# 個人用テンプレートのライブラリ化

2026-10-15、ユーザー所有の `/home/blueberry/lib/blueberrytemplate.h` を調べ、短い記法と既存の検証済み実装を併用できる形へ整理した。
通常の補助関数は `blueberry/utility/contest-helpers.hpp`、明示 opt-in の短縮記法は `blueberry/utility/contest-template.hpp`、組合せ前計算は `blueberry/math/combinations.hpp`。
既存の公開 API・古い入口の削除方針は変えていない。

## 元の機能との対応

| 元の機能 | 移行先・採否 |
| --- | --- |
| `_main()` と自動 `main()` | 利用者が `int main()` を書く。`templates/contest.cpp.example` を用意 |
| main 冒頭の stream 設定 | 明示的な `init_io(precision=30)` |
| `quick_exit(0)` | 採用しない。通常 return により stream flush と destructor を実行 |
| `ll/ld/ull/uint/str`、`mint/mint1/pqg<T>` | opt-in ヘッダの同名型別名 |
| `using namespace std/atcoder` | main 側で必要なものを指定。ヘッダは自動 using を追加しない |
| `rep` の 1～4 引数 | 同じ短い記法。4 引数は正の step。終了時の増分も ll 内に収める |
| `ALL/rALL`、`fi/se/pb/eb`、`spa/Endl/O` | opt-in macro。副作用を持つ式・macro 衝突の条件を文書化 |
| `LL/STR`、`scan/print` | 同じ短い記法。空引数も利用可能。print の改行は flush しない |
| vector/deque/pair の stream 演算 | 同名 global operator を opt-in で提供。入れ子にも対応 |
| `YesNo/YESNO` | 同名 macro。改行するが flush しない。対話では明示 flush |
| `INF/CPS/dx/dy` | 書換え不可の inline constexpr。4 近傍・8 近傍の順序を維持 |
| `bit/popc/crmp` | opt-in macro。popc は 64 bit を数え、旧 32 bit 打切りを修正 |
| `chmin/chmax` | `blueberry::chmin/chmax` と opt-in using |
| `sum/len`、vector の min/max | `sum/len/min_of/max_of`。sum は += のみで使える。空 min/max は禁止 |
| 可変長 min/max | std::min/std::max の 2 引数・initializer_list を利用。曖昧になる global overload は追加しない |
| `Fill` | 同名 API。多次元配列を pointer cast で平坦化せず、部分配列ごとに代入 |
| `my_distance/iskaibun` | 距離は同名。回文は `blueberry::is_palindrome` と旧名 wrapper |
| signed128 出力 | 全範囲対応 `to_string128` と opt-in `operator<<` |
| `parsetoint128` | `parse128` の optional 返り値へ移行。符号を保持し、不正文字・範囲外を検出 |
| `divide` | `blueberry::floor_div` と旧名 wrapper。最小値の negation を除去。0 除算・min/-1 は禁止 |
| `bin_search` | 同名 API。std::midpoint と unsigned 距離により ll 全端点を扱う |
| `ran` | `blueberry::range_sum` と旧名 wrapper。signed128 返り値、a>b は 0 |
| `zaatu` | `blueberry::compress` と旧名 wrapper。map の node allocation をソート済み vector へ変更 |
| `enumdiv` | `blueberry::divisors(uint64_t)`。既存 factorize から生成し昇順を返す |
| `soinsu_bunkai` | 既存 `blueberry::factorize(uint64_t)`。返り値は昇順の素因数列で、指数が必要なら連続同値を数える |
| `Era(n)`、global `isprime` | 既存 `PrimeSieve(limit)` の `primes()/is_prime()`。旧 n は排他的、新 limit は包含上限。n<=1 は `PrimeSieve(0)` で空の prime 列 |
| `COMinit/COM/nPr`、global fac/finv/invv | `Combinations<Mint>(limit)`、`C(n,k)/P(n,k)`。limit は包含上限、0/1 も安全、modulus ごとに独立 |
| `UnionFind` | ACL `atcoder::dsu`。find→leader、connected→same、merge/size は同名 |
| `topo_sort(G, indegree, v)` | 既存 `blueberry::topological_sort(vector<vector<int>>)`。入次数は内部で計算、cycle は nullopt。旧版の辞書順最小保証はなく FIFO の O(V+E) |
| `isbipartite_graph` | `blueberry::is_bipartite` と旧名 wrapper。反復 BFS にして深いグラフの再帰 overflow を避ける |
| `Rande` | opt-in 同名型。明示 seed constructor を追加、乱数生成器はオブジェクト内で所有 |
| コメント内の MOD/extgcd | global MOD を追加しない。modint、ACL inv_mod/crt 等を用途に応じて使う |

COM は prime modint で `0 <= limit < modulus` を要求する。n/k が無効なら 0、範囲内の有効問い合わせは前計算済みであることが必要。
dynamic_modint の modulus は表の利用中に変更できない。元の COMINIT により別 MAX/MOD が無視される状態は持たない。
signed128 の最小値を signed のまま負にする処理、parse が '-' を無視する処理、小さい sieve/COM の範囲外 access、二分探索の端点和・距離の overflow は引き継がなかった。

## ACL と複数 translation unit

通常の helper と Combinations は inline/template 定義だけであり、独立した有限体の型を使った 2 translation unit の link を検証する。
opt-in contest-template は元の atcoder/all を含むため、ACL 同様の単一 translation unit の提出用として扱う。
ACL の floor_sum_unsigned、countr_zero などの非 inline 定義に起因する複数定義エラーを確認し、第三者ヘッダの修正は行わなかった。
初回の失敗結果を `benchmark/results/contest-template-import/validation-initial-acl-odr-failure.json` に残す。

## 検証

`benchmark/contest-template-import-validate.py` は GCC 13.3 / Clang 18.1.3、gnu++20/23、-O2 -Wall -Wextra -Wshadow -Werror で以下を確認する。
各ヘッダの単独・重複 include、3 文書の main、seed 1～5 の境界/random test、generic finite-field による Combinations と helpers の 2 translation unit の link。
sum は operator+ のない += 専用型でも確認する。二部判定は全彩色 brute force と比較し、10 万頂点の path を反復処理する。
stream は vector<pair>、pair<vector,int>、vector<vector>、deque<pair<deque,int>> の入れ子を確認する。追加の Many A + B (128 bit) driver は opt-in の init_io/scan/print と signed128 stream 演算を公式 case で直接確認する。

`benchmark/contest-template-import-official.py` は upstream が生成した binomial_coefficient_prime_mod の全 20 cases と公式 checker を使う。
GCC/Clang、gnu++20/23 の 4 組合せを区別し、入力 hash・upstream revision・各 case の実行/判定結果を残す。
make test / make verify / make docs の全体確認と catalog 登録は統合担当の作業で、この focused report の成功結果で置き換えない。

## Fastest 調査と採用判断

2026-10-15 の公開 REST API で [223248](https://judge.yosupo.jp/submission/223248) と [196412](https://judge.yosupo.jp/submission/196412) を調査した。
223248 の factorial table は fac/finv の 2 配列、逆元 1 回、逆順の乗算であり、今回の Combinations と同じ前計算手順だった。
common modulus を static 型へ分岐し、固定 1e7 上限・custom buffered I/O・AVX2 target を使う。
公開 source は調査専用の .build 下に置き、コードは移植・再配布しない。URL・source hash・機械的 profile を測定資料に残す。

対応型を保つ Mint generic の表を採用し、original COM の 3 個の int64 配列を 2 個の Mint 配列へ削減した。
ACL modint の場合 N=1e6 で論理的な表の要素 memory は 24,000,024 B から 8,000,008 B（allocator/capacity overhead は除く）。
Mint::raw を使う候補も同じ N/Q と compiler で 5 回測り、GCC ではほぼ同じ、Clang の構築は改善が見えなかったため不採用とした。
raw のない体でも使える簡潔な一般実装を残し、before/raw-candidate/final の測定を別ファイルに保存する。

verify driver は common modulus を ACL static_modint へ分岐し、それ以外は dynamic_modint、入力出力は既存 FastInput/FastOutput を利用する。
全問い合わせを 1 回読み込み、実際の最大 n まで前計算するため、不必要な固定上限の表を作らない。追加の query memory は O(T)。
元テンプレートの stream による scan/print は使い方を保ち、高速 I/O が必要な用途には既存 Fast I/O を案内する。
最初の fastest 比較に assert flag の差があった結果は `fastest-local-initial-unmatched-flags.json` として保存し、性能判断には使用しない。
同じ -O2/-DNDEBUG/compiler/公式最大入力で再コンパイルした比較、I/O の採用前後、表の構築/操作を分離した 5 回測定は、下記の自動生成 report を参照する。
AVX2 等の source 固有条件と、query 保存・汎用 API の差は保持する。source の Yosupo 表示値をローカルの合計時間に直接比較しない。

[測定ログと集計](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/results/contest-template-import/report.md)
