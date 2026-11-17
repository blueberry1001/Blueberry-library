# Matrix の行ポインタ化の検討

既存の汎用体APIで行のアドレス計算を外へ出し、消去中の演算子呼び出しを減らす候補を比較した。公開API、対応型と計算量は変更しない。[上位提出の調査](../../../benchmark/algebra-expansion.md)では、行列式の高速提出がブロック化、SIMD、剰余演算の遅延を用いている。一方、それらは法・型や命令セットが固定された実装であり、この汎用候補と同条件ではない。

Windows 11、Clang 18.0.3（Unity Android NDK、MSVC ABI）、`-std=gnu++20 -O2 -DNDEBUG`、Intel CPU。親プロセスを利用可能な最下位CPUへ固定し子へ継承、同じ乱数seed 1729、各1 warmup＋6回を交互順に実行。結果の全要素をhashし、同じ入力で両候補の値が一致した。測定は公開関数の呼び出しから返り値走査までを含み、入力生成は含まない。構築・出力I/OとRSSは分離測定していない。密行列N=300の中央値はdet 15.906→15.996ms、rank 14.857→15.146ms、inv 66.645→66.116ms、solve 22.772→23.582ms。ばらつきを含み、安定した改善を示さないため不採用。全サンプル・環境・ハッシュは [results.json](results.json)。

変更前と試作の両方で `tests/random/algebra-expansion.cpp` のseed 1・2・17がWindows Clang gnu++20、assert有効で成功。GCC、Linux、公式ケースは未実行。測定器、build・runnerと両ヘッダのsnapshotを同ディレクトリに保存した。保存時に改行をLFに正規化したヘッダをコンパイルし、hashは測定時の実バイトを表す。再実行する場合は、`.build/matrix-tune/`に保存ファイルを復元し、`prepare.py.txt`, `bench.cpp.txt`, `build.py.txt`, `run.py.txt` の `.txt` を除く。`prepare.py` は現在の公開ヘッダから候補を生成するため、元版と異なるrevisionでは保存済みsnapshotを使う。Windows SDKとcompilerのパスは環境依存である。
