# HLD のパス列挙を反復化する試作

`HeavyLightDecomposition::path_query` は、両端のパス区間を始点から終点への順に callback へ渡す。3引数 callback には区間の向きも渡す。非可換演算や callback の副作用を含めて順序を変えないことを前提に、再帰を減らす2案を調べた。どちらも採用せず、ライブラリは変更していない。ACLに同じHLDはない。

- 全面反復案: 終点側の区間を一時配列に積み、始点側・同一chainの後に逆順で処理する。固定32要素は、この実験環境の32bit `int` と軽辺が通過するたび部分木の大きさが半分以下になることに基づく。より広い `int` への一般化は評価していない。
- 部分反復案: 始点側の再帰だけをループにする。単一chainは従来と同じ処理だが、末尾処理を重複して記述する試作である。

[KACTL HLD](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/HLD.h) は反復で区間を処理するが、可換な問い合わせを前提にしており、本ライブラリの順序付き callback と同じ契約ではない。高速な公開例 [Vertex Add Path Sum #349840](https://judge.yosupo.jp/submission/349840) は[著者の説明](https://codeforces.com/blog/entry/135239)によると XOR 木の構築と HLD を組み合わせる。順位が現在の最速かどうかと、その提出との同一条件・同等契約の速度比較は未確認。提出コードのコピーはしていない。

## 測定

Windows 11、Intel Family 6 Model 183 Stepping 1、N=200,000、Q=300,000、seed=1729。Windows MSVC 14.43 `/std:c++20 /O2 /DNDEBUG` と Android NDK Clang 18.0.3 の Windows MSVC target `-std=gnu++20 -O2 -DNDEBUG` を別々に使った。入力・木の構築は計時外で、3引数 callback の区間・方向・クエリ区切りを hash に含める。各条件で2回の warmup と7回を交互順で実行。Python 親プロセスを CPU 0 に固定し、子プロセスへ継承させたが、子側の affinity は個別に照合していない。I/O・木の構築・区間データ構造の費用・メモリは未測定。全試行の hash と区間数は旧版・各試作で一致した。

下表は全面反復案の[再測定](results-rerun.json)の中央値ms。0は頂点、1は辺。初回は時間のばらつきが大きく、[初回全試行](results-initial.json)を別に保存した。

| 環境・木 | 0: 旧→反復 | 1: 旧→反復 |
| --- | ---: | ---: |
| MSVC path | 4.075→4.348 | 3.773→4.095 |
| MSVC star | 6.620→6.628 | 5.698→7.067 |
| MSVC balanced | 65.760→61.685 | 63.037→61.977 |
| MSVC random | 43.056→40.613 | 45.625→42.532 |
| Clang path | 3.737→3.307 | 4.009→2.804 |
| Clang star | 6.368→5.194 | 6.140→4.816 |
| Clang balanced | 66.718→55.671 | 64.270→57.647 |
| Clang random | 40.710→34.743 | 40.948→35.108 |

MSVCの star・辺で約24%、path・辺で約9%の回帰があり、一律置換しなかった。初回のMSVC path・辺は旧版4.52〜12.43msと変動が大きく、再測定だけで回帰量を全環境に一般化しない。[部分反復案](results-v2.json)は MSVC star・辺 6.128→5.376ms だった一方、大半の balanced/random の差は数%以下であり、重複コードに見合う安定した効果とは判断しなかった。Linux GCC/Clang、Library Checker の公式ケース、最速提出との再計測は未実行。

## 正しさと再現

既存 `tests/tree_random.cpp` を旧版と両試作について Windows Clang/MSVC の C++20・assert有効で実行し成功した。最初の試行では誤ってテストも `NDEBUG` でビルドしたため、その結果は正しさの根拠にせず、assert有効に直して再実行した。[compile.json](compile.json) と [compile-v2.log](compile-v2.log) に成功したコンパイルコマンドを残す。候補の区間順・向きは性能ドライバでも全試行の hash を旧版と照合したが、これは全入力の証明ではない。

保存版は [旧ヘッダ](baseline.hpp.txt)、[全面反復](candidate.hpp.txt)、[部分反復](candidate2.hpp.txt)、[ドライバ](driver.cpp.txt) と `prepare*.py`、`run*.py`。再現時は元ヘッダと `tests/tree_random.cpp` を含む捨ててよいチェックアウトで、これらを `.build/hld-path-study/` にコピーし、ドライバの末尾 `.txt` を取り除く。`prepare.py` はチェックアウトの現行ヘッダから2版を生成し、`prepare-v2.py` はその旧版から部分反復案を作る。各 run スクリプトは `.build/hld-path-study/` に結果を上書きするため、既存データがない環境で実行する。Windows SDK・コンパイラのパスは環境に合わせて変更する。生成時の入力ファイル hash と各試行を JSON に記録した。保存時の改行はLFに正規化しており、Windows上の実バイトを対象とする元の hash と異なる場合がある。
