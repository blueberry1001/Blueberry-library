# Suffix Automaton の構築性能（2026-11-16）

`append` の未登録遷移を `try_emplace` の一度の検索で追加し、既存遷移を見つけたときも返されたiteratorから遷移先を得る。cloneの遷移mapは一度だけコピーし、vectorにはmoveする。記号にhashやmoveを要求せず、比較可能なコピー可能型、オンライン追加、元のAPIと計算量を維持する。

## 再現方法と条件

```sh
mkdir -p /tmp/sam-base/blueberry/string
git show 5b1ac35cbe23dcdba1ca2613236b728cf6346dd1:blueberry/string/suffix-automaton.hpp > /tmp/sam-base/blueberry/string/suffix-automaton.hpp
python3 benchmark/run_suffix_automaton.py \
  --baseline /tmp/sam-base --candidate . --n 200000 --queries 1200 \
  --warmups 1 --repeats 5 --cpu 0 --output .benchmark/suffix-automaton.json
```

CPU番号は実行環境に合わせる。GCC 14.2.0、`-std=gnu++20 -O2 -DNDEBUG`、Xeon Platinum 8573Cの一つの論理CPUに固定し、warmup 1回後、同じ固定seed 1729の入力を交互に各5回実行した。長さ20万の入力生成と結果出力・破棄を計時から除き、構築には確保を含む。クエリは種類ごとに1,200本、長さ16または256で、既存部分列、存在しない記号、部分的な不一致を含む。各実装の入力・状態数・部分文字列数・count・LCSの3要素のチェックサムは一致した。

下は**構築のみ**の中央値（ミリ秒）。CPU負荷やallocator配置による変動を含み、Library Checker提出時間や処理全体の改善率ではない。

| 入力 | 変更前 | 変更後 |
| --- | ---: | ---: |
| char、同一文字 | 18.740 | 17.298 |
| char、周期5 | 18.072 | 17.702 |
| char、4種類の一様乱数 | 91.925 | 84.641 |
| char、96種類の一様乱数 | 114.470 | 98.511 |
| int、65,536種類の一様乱数 | 140.201 | 120.295 |

初回countの集計、構築後のcount・LCSも別に計時した。例えばint広alphabetの長い一致LCSは5.050→5.517msと中央値が増え、5試行の範囲は重なった。検索コードは変更しておらず、入力やallocatorの配置による計測変動もあるため、問い合わせの一律高速化は主張しない。RSSはプロセス全体の最大値で、入力・クエリ・runtimeを含むため、構造自体の増減とは解釈しない。各試行・最小値・最大値・RSS・source hash・環境は [results.json](results.json) に記録する。

## 公式入力での処理全体

生成済みLibrary Checker公式データ（問題リポジトリrevision `73876f1cbcf082d2b22722924b924d3d65a2f00e`）の代表ケースを同じverifyドライバ・コンパイラ設定で実行した。次はプロセス起動、入力、構築、問い合わせ、出力、終了を含むwall時間の中央値（ms）。全出力は各回公式checkerに通り、前後の出力も一致した。入力生成・ファイルopen・checkerは計時外で、共有CPUの負荷変動を含む。

| 問題・入力 | 変更前 | 変更後 |
| --- | ---: | ---: |
| Number of Substrings、all_same_00 | 53.17 | 51.82 |
| Number of Substrings、max_random_00 | 364.24 | 334.81 |
| Longest Common Substring、all_same_00 | 57.78 | 56.52 |
| Longest Common Substring、max_random_00 | 448.55 | 411.21 |

[全試行・入力hash・checkerの結果](official-results.json)。同じ問題checkoutで各問題のケースとcheckerを生成後、次のように再実行できる。

```sh
python3 benchmark/run_suffix_automaton_official.py \
  --problems-root /path/to/library-checker-problems \
  --baseline /tmp/sam-base --candidate . --cases all_same_00,max_random_00 \
  --warmups 1 --repeats 5 --cpu 0 --output .benchmark/sam-official.json
```

## 別々の試作と不採用案

長さ15万の別測定では、4種類の一様乱数の構築が、cloneのmoveのみで62.20→57.36ms、遷移探索の統合のみで64.19→61.16msだった。96種類の乱数ではcloneのみ75.76→80.20ms、探索のみ76.03→65.43ms。別試行の中央値であり、それぞれを直接足して最終改善を説明しない。両変更を併用した前段の結果を含め、[cloneのみ](trial-clone.json)、[探索のみ](trial-insert.json)、[併用](trial-append.json)に全試行を残す。

LCSの`contains`と`find`をまとめる試作は、[同じ構築版との比較](trial-lcs-isolated.json)で、長さ256の一致クエリ1,000本が同一文字で1.540→1.980ms、周期5で1.543→2.023msに回帰した。不一致クエリでは短縮する場合もあるため、今回のLCS変更は採用しない。[初回の併用試作](trial-lcs.json)も保存する。

試作差分は[clone](trial-clone.patch)、[探索](trial-insert.patch)、[両変更](trial-append.patch)、[LCSを含む案](trial-combined.patch)。当時のrunnerは`trial-run.py.txt`として保存した。再実行時は`run.py`に戻し、同じディレクトリに`benchmark/suffix-automaton.cpp`を`benchmark.cpp`として置く。
