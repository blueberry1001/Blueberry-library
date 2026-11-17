# Binary Trie の削除経路の試作

`BinaryTrie::erase` は値の存在を確認した後に同じパスを再度たどる。試作では一度の探索でパスを保存し、欠損が見つかると変更せず `false` を返す。重複の削除、多重度・順位、ノードの再利用、任意ビット幅を維持する。欠損時は保存したパスが不要になるため、書き込み費用が増えうる。

過去の [Set Xor Min 調査](../../../../benchmark/results/operation-structures/report.md) では上位提出 [403013](https://judge.yosupo.jp/submission/403013) は30bit集合・最小XOR向けのbitmap階層と専用I/Oを使う。多重度と順位・全UInt幅・削除失敗の返却を持つ本APIとは条件が異なる。今回の実験は旧版と候補の同条件比較で、上位提出との直接再測定や現在の最速順位の確認ではない。ACLに同等機能はない。

## 測定

Windows 11、Intel Family 6 Model 183 Stepping 1、Unity Android NDK Clang 18 / MSVC ABI、`gnu++20 -O2 -DNDEBUG`。32bit trieに偶数値を挿入し、入力生成・初期挿入は計時外。CPU affinityは実行可能な最下位CPUへ固定し子プロセスへ継承、各1 warmup＋7回、旧版と候補の実行順を交互にした。子プロセス自身のmaskは別途確認していない。削除成否の合計・最終サイズと終了後の標本count / xor_minをchecksumに含め、各試行で一致した。ピークworking setはプロセス全体で、初期入力とruntimeも含む。

N=150,000の操作時間中央値（ms）:

| 削除入力 | 旧 | 候補 |
| --- | ---: | ---: |
| 全成功 | 37.26 | 37.95 |
| 上位で欠損 | 0.315 | 0.314 |
| 中間で欠損 | 3.872 | 3.591 |
| 最下位で欠損 | 14.89 | 20.71 |
| 25%成功 | 18.41 | 20.20 |
| 50%成功 | 24.64 | 22.25 |
| 75%成功 | 31.68 | 28.85 |
| 重複を1個削除 | 24.71 | 23.09 |
| 成功後に再挿入 | 36.36 | 35.50 |

初回は全成功が36.29→33.00ms、最下位で欠損が12.43→12.77msだった。追加計測では全成功の改善が再現せず、最下位での欠損が約39%遅くなった。N=20,000でも最下位欠損は1.039→1.183msで回帰した。これらの入力分布で安定して改善するとは言えず、**公開実装への変更は見送った**。環境の負荷・周波数変動やメモリ配置による影響は残る。計測はWindows Clangだけで、GCC・Linux・公式入力・上位提出の直接比較は未測定。

各サンプル、最低値、checksum、ピークworking set、環境は [初回](results.json) と [追加](results-v2.json) に保存する。[compile.json](compile.json) は同じ候補と旧版を Windows Clang の gnu++20/23、assert有効でコンパイルし、既存 `tests/random/operation-structures.cpp` を各3 seedで通過した記録。独立の公式verifyやsanitizerは未実施。

## 再現

末尾 `.txt` を外した [bench-v2.cpp.txt](bench-v2.cpp.txt)、[prepare.py.txt](prepare.py.txt)、[run.py.txt](run.py.txt) を `.build/trie-erase-study/` に配置し、リポジトリ直下で `python .build/trie-erase-study/prepare.py`、`python .build/trie-erase-study/run.py` を実行する。実行は同名の結果を上書きするので保存してから行う。`prepare.py` はその時点の公開ヘッダから候補を作るため、旧・候補が [baseline.hpp.txt](baseline.hpp.txt)・[candidate.hpp.txt](candidate.hpp.txt) と一致するrevisionを使用する。Windows SDK / MSVC / Clangのパスは環境に合わせて変更する。[bench-v1.cpp.txt](bench-v1.cpp.txt) は初回ドライバを同内容で再構成した保存版で、初回実行時のhashは記録していない。追加測定のdriverやscriptの保存ファイルは後から追加したため、実行時のhashと断定しない。
