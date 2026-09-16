# String expansion research — 2026-09-17

Fastest調査は[metadata](fastest-metadata.json)に取得日時・提出ID・source SHA-256を保存。
上位2件ずつのソースを調査用に取得したが、ソース自体は転載していない。

| 問題 | 上位提出 | 調査結果と採否 |
| --- | --- | --- |
| [Aho Corasick](https://judge.yosupo.jp/problem/aho_corasick) | [307744](https://judge.yosupo.jp/submission/307744), [380150](https://judge.yosupo.jp/submission/380150) | 1位は疎な辺配列とfailure探索、2位はdense表と遷移の一括コピー。構造出力だけの公式問題と、本文を検索する汎用APIでは測定対象が異なる。検索時の1文字O(1)を優先してdense表を独立実装 |
| [Eertree](https://judge.yosupo.jp/problem/eertree) | [394036](https://judge.yosupo.jp/submission/394036), [400577](https://judge.yosupo.jp/submission/400577) | どちらも疎な子ノード表現。前者はhash順序の子pool、後者は疎な辺と専用allocation/I/O。小固定字母での直接遷移と実装の単純さを比較判断 |
| [Lyndon Factorization](https://judge.yosupo.jp/problem/lyndon_factorization) | [394259](https://judge.yosupo.jp/submission/394259), [338512](https://judge.yosupo.jp/submission/338512) | 線形Duval法。前者は汎用range/projection、提出全体に独自I/Oを含む。Blueberryはsize/index/<だけを要求する短いAPI、標準I/Oのverifyと関数単体測定を分離 |

## 同条件比較

`python3 benchmark/run_string_expansion.py`。自作の疎な遷移候補と製品のdense表を比較し、
すべての出現数vectorから計算するchecksum一致を確認する。入力生成・process起動・I/Oを時間から除外。
GCC13.3、GNU C++20、`-O2 -DNDEBUG`、Intel Core i7-14650HX、WSL2 Linux。
seed1729、本文20万文字、warmup1回＋交互順5回。中央値・最小・各生値・source hashは
[計測JSON](../lc-expansion-string-2026-09-17.json)を参照。

測定中は公式検証が並走しており、CPU・I/O負荷による揺れがある。僅差を採否の根拠にせず、
Fastestの表示時間とローカルの時間は直接比較しない。RSSは入力・processを含む。

| 入力 | Aho dense / sparse 中央値ms | Eertree dense / sparse 中央値ms |
| --- | ---: | ---: |
| ランダム26文字種 | 123.71 / 167.51 | 3.15 / 17.00 |
| 同一文字 | 12.44 / 11.54 | 31.97 / 9.79 |
| 周期文字列 | 8.44 / 6.11 | 47.92 / 8.94 |

Eertreeの疎候補は長い同一文字・周期文字列では時間とメモリに有利で、同一文字のpeak RSSは
dense約37MiB、sparse約14MiB。一方、ランダム26文字種ではdenseが大きく速い。
すべての入力で一方が優越するとは判断しない。今回は固定小文字種で直接遷移できるdense実装を採用し、
O(σN)のメモリ上限を文書化した。疎表現を追加する際は文字種・操作契約を分けて検討する。

Ahoもランダム辞書ではdenseが速く、長い少数のパターンではsparseが省メモリ。
一般的な文字列辞書に対する1文字O(1)遷移と、実装の予測しやすさを優先する。
Lyndonの同条件基準中央値はrandom0.61ms、同一文字1.87ms、周期0.80ms。比較対象はないので高速化とは呼ばない。
