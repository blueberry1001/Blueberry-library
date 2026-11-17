# Subset Convolution の小入力経路（2026-11-17）

公開 API の同長・空または2冪長・可換環という前提、空入力、入力の所有権を維持しつつ、少数ビットでは各集合の部分集合を直接列挙する候補を調べた。直接列挙は K ビットについて 3^K 個の積を計算するが、採用対象を K≤7 に固定する場合、従来の O(N(1+K)²) という漸近上界は変わらない。K≤7 では直接列挙の積の回数も既存の ranked 方式以下である。const 参照からの積を加算できない型では、従来と同じ可変要素を掛ける ranked 方式へフォールバックする。

## 高速提出との比較

[adamant による高速提出の解説](https://codeforces.com/blog/entry/151162)と [CP-Algorithms の実装](https://lib.cp-algorithms.com/cp-algo/math/subset_convolution.hpp)を調査した。提出者は公開当時の首位と報告しており、mask ごとにランクをまとめ、上位ビットのブロック処理、XOR 変換、SIMD の剰余演算を用いている。同実装の XOR 変換は2の冪の逆元、専用の剰余型や SIMD 環境を伴うため、本ライブラリが保証する標数2を含む任意の可換環と同じ条件ではない。現在の順位と同条件の直接実測は未確認である。従来の高速提出調査は [algebra-expansion.md](../../../../benchmark/algebra-expansion.md) を参照。

## 方法と初回結果

Windows 11、Intel64 Family 6 Model 183 Stepping 1、Unity 同梱 Clang 18.0.3、MSVC ABI、`-std=gnu++20 -O2 -DNDEBUG`。固定 seed・同じ入力・CPU affinity 固定。入力生成とプロセス起動は計時外、入力を参照する畳み込み、出力の1要素の利用と vector の破棄を含む。同じバイナリで旧版・候補を別プロセスで交互実行し、ウォームアップ後の中央値と最小値、全サンプルとピーク working set を JSON に保存した。検証した全ケースでチェックサムは一致した。プロセス全体の working set はランタイムを含む。取得に失敗したときは測定をエラー終了とする。

初回は小サイズの直接列挙と、popcount を全 mask について先に求める別案を比較した。後者は大きい入力で安定した優位が確認できなかったため候補から外した。型互換性の修正後、最終ヘッダを直接使い、CPU affinity 0x40、ウォームアップ1回後の11回測定を記録した [quiet-results.json](quiet-results.json) の例：

| 型 | K | 反復 | 旧版中央値 ms | 候補中央値 ms |
| --- | ---: | ---: | ---: | ---: |
| uint32_t | 0 | 500000 | 46.105 | 14.757 |
| uint32_t | 4 | 6172 | 3.380 | 0.368 |
| uint32_t | 7 | 228 | 1.416 | 0.243 |
| modint998244353 | 0 | 500000 | 45.454 | 14.930 |
| modint998244353 | 4 | 6172 | 3.649 | 0.588 |
| modint998244353 | 7 | 228 | 1.720 | 0.448 |

型互換性の修正前、K=8以上では旧アルゴリズムを通るにもかかわらず、K=12/14 における測定には大きな変動があり、初回では uint32_t の K=14 が 13.185→18.686 ms だった。追加の21回計測 [focus-results.json](focus-results.json) でも K=12 は 4.612→5.987 ms、K=14 は 20.239→23.286 ms と差が見えた一方、全体に外れ値と範囲の重複が大きかった。

修正前に小入力部分を別関数に分離した案、さらに `noinline` とした案も加え、4種類を交互に21回測定した [outlined-results.json](outlined-results.json)。同条件の旧版→当時の候補の中央値は、uint32_t の K=12 で 2.390→2.386 ms、K=14 で 26.437→27.234 ms、K=18 で 86.549→86.138 ms。modint の K=12 で 3.080→3.060 ms、K=14 で 30.065→30.735 ms、K=18 で 97.748→97.897 ms。前回の大きな差は再現せず、すべての入力・環境で回帰がないとまでは判断できない。分離案も通常経路に明白な優位がなく、条件付きの compiler 属性は不要と判断した。

型互換性修正後の CPU 0x1 に固定した [final-results-contended.json](final-results-contended.json) では大きな外れ値が頻出し、同一ケースで測定値が数倍以上変動したため性能結論には使わない。別の CPU 0x40 に固定した最終11回測定では通常経路の K=12 が uint32_t 1.524→1.504 ms、modint 1.689→1.723 ms、K=18 がそれぞれ 83.021→87.126 ms、94.762→92.302 ms だった。入力・ビルド・全サンプルは各 JSON の記録を参照し、異なる affinity の結果を混ぜていない。

## 正しさと再現

`tests/random/algebra-expansion.cpp` に K=0/1/6/7/8 の境界、`operator==` を持たない独自可換環、両引数の同一 vector 参照、K=7/8 の GF(2) 愚直比較を追加した。レビューで、変更前の実装が可変の内部要素同士を掛ける一方、直接列挙は const 参照同士を掛けるという型要件の差を発見した。const 参照の積を加算できない型は従来方式へフォールバックし、非 const 参照でしか乗算できない型を K=0/1/6/7/8・同一入力参照で検証した。Clang の C++20/23、`-Wall -Wextra -Wshadow -Werror`、seed 1/17/731 で通過。[test.json](test.json) にコマンド・出力・終了コードを保存した。初回 [test-initial.json](test-initial.json) は、ローカルの ACL コピーを通常 include として先に探索し ACL の `_umul128` 警告がエラーになったため失敗した。ACL を `-isystem`、本体を `-iquote` として再実行した。GCC、Linux、Library Checker および `make test`・`make verify`・`make docs` は本環境では未実行。

`prepare.py.txt` は保存された旧版 snapshot を名前空間だけ変えて benchmark の `baseline.hpp` を生成する。`prepare-final.py.txt` は最終版の実ヘッダを読み込む driver と runner を生成する。`run-final.py.txt --build` の後に `prepare-quiet.py.txt` と `run-quiet.py.txt` を実行すると [quiet-results.json](quiet-results.json) を再生成する。`prepare-focus.py.txt` と `prepare-outlined.py.txt` は追加計測用 runner・候補を生成する。実行ファイルは記録に含めず、ソース・実行ファイルの SHA-256 を記録している。初回ビルドでは ACL ヘッダ由来の `_umul128` 警告が出たがコンパイルは成功した。分離案の最初のビルドは Windows SDK の `small` マクロと候補関数名が衝突して失敗し、[build-outlined-initial.json](build-outlined-initial.json) に保存した。候補関数を改名し、ACL のみを system include にして再ビルドした。
