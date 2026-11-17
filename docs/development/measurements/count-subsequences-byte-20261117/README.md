# Count Subsequences の整数値域の直接表

記号の狭義弱順序、Countの演算と公開APIを保ちながら、整数の小さな値域では座標圧縮を省く。8bit環境の1バイト整数かつ `sizeof(Count)<=1024` では、N>=64のときunsigned charへ変換して256要素を使う。別の標準整数型はN>=256で、先頭・末尾・等間隔を先に調べてから値域を検査する。Wが65536、N/2+1、262144/sizeof(Count)+1のいずれも超えなければW要素の表を使う。それ以外・独自記号・proxy・boolの広い格納型は元のソート経路を使う。unsignedへの変換後、元の整数幅のunsigned型に戻して差をとるため、符号付きの最小値を含む全域でも符号付き減算は行わない。任意のユーザー定義順序に整数同値性を仮定しない。

## 上位提出の調査

2026-11-17に [Number of Subsequencesの順位表](https://judge.yosupo.jp/submissions?problem=number_of_subsequences&status=AC&order=%2Btime) を確認し、当時1位の [216264](https://judge.yosupo.jp/submission/216264) を読んだ。この提出はuint32整数と固定法を使い、最大800003個の固定容量を持つ線形探索ハッシュ表でオンラインに集計し、Unixではmmapによる専用入力を使う。本APIは非整数・独自順序・異なるCount・サイズ上限なしを維持し、固定容量とI/Oの制約は導入しない。[公式解](https://github.com/yosupo06/library-checker-problems/blob/master/enumerative_combinatorics/number_of_subsequences/sol/correct.cpp)はmapで直前値を管理する。公開提出のコードはコピーしていない。提出と本実装の同一条件・同一入力による直接実測は未実施であり、順位表の14msと以下の測定値は比較しない。

## 同条件の変更前後測定

Windows 11、Intel Family 6 Model 183 Stepping 1、Unity Android NDK Clang 18.0.3 / MSVC ABI、`gnu++20 -O2 -DNDEBUG`。CPU affinityを利用可能な最下位CPUへ固定し子プロセスに継承。固定seed 20261117、同じ入力と公開API、各条件1 warmup後に版の順序を交互にして実行した。構造の呼び出しと戻り値走査を計時し、入力生成・I/Oは除く。byte側のdriverは各繰り返しの1箇所更新と乱数生成も含み、両版で同じ処理を行う。全実行のchecksumが一致した。共有PCにつきCPU負荷・周波数変動は残る。測定時はルートの `atcoder/` がinclude検索で先に選ばれ、`atcoder/modint.hpp` のSHA-256は `aef131053e67300be6def657f1ef1eb1c80ef2c92ffc8220d7be0cccab583318`。`.deps/ac-library` 内の同名ファイルも同hashだった。

N=500,000、各ケース3回のAPI呼び出しをまとめた時間。byteは1 warmup後6回、int系は1 warmup後20回の中央値、単位ms。

| 入力 | 旧 | 最終 |
| --- | ---: | ---: |
| signed char・全同値 | 12.28 | 2.75 |
| signed char・4種類 | 30.84 | 2.60 |
| signed char・256種類 | 81.88 | 2.58 |
| int・128種類 | 75.32 | 3.98 |
| int・全幅乱数 | 186.06 | 184.70 |
| int・サンプルを外れた中央付近の1外れ値 | 12.69 | 13.41 |
| int・長い単調増加 | 27.50 | 28.13 |
| int・長い単調減少 | 32.21 | 32.90 |
| uint64・最小値付近の128種類 | 74.41 | 3.94 |

中央付近の1外れ値では約5.6%の回帰。単調列でも約2%の回帰がある。小さいNでは従来経路を使うが、signed charのN=63・1000回は1.243→1.339msで、小さい絶対差と変動が残る。全幅乱数の差は同程度のばらつきと重なり、優劣を断定しない。個別の全試行・hash・追加分布・実行環境は [byte結果](results-final.json)、[整数結果](int-results-final.json) に保存した。初期案は末尾外れ値で約13.6%回帰し、事前サンプリング導入前後の [v1](int-results-v1.json)、[v2](int-results-v2.json) と[20回の追加測定](int-results-extended.json)も残す。

## メモリと検証

直接表はbyteで最大256*sizeof(Count) <=262144 byte、広い整数で最大262144+sizeof(Count) byte。要素数はN/2+1以下かつ65536以下。もとの2本のN要素添字配列はこの経路では確保しない。値域検査に失敗した場合は追加表を作らず従来のメモリ・計算量で処理するが、失敗位置までの走査時間が加わる。

4096 byteの非default構築CountでN=8192、プロセスのピークworking setを生存中にWindows APIで別実行・各3回採取した。入力・実行ファイル・ランタイムを含め旧／最終で約3.9〜4.7 MiBだった。OS管理によるばらつきがあり、構造単体のメモリや微小差は推定しない。[memory.json](memory.json) に生値、取得回数と全hashを保存する。

旧版と公開ヘッダの最終版をWindows Clangのgnu++20/23、assert有効でコンパイルし、既存random testと2つの独立oracleをseed 1・2・17・20261117で成功した。[final-build.log](final-build.log) にACLヘッダの警告を含め全結果を保存する。追加したchar16_t・char32_t・wchar_tを含む公開テストも同じ構成で各4 seed成功し、[final-extra-test.log](final-extra-test.log)に記録した。符号付き/なし16・32・64bitの全幅端点、負から正、byte全256値、bool/proxy、比較同値性が独自の型、default構築不可Count、4096 byte Count、Nと表幅の境界、`__int128`のフォールバックを検証した。既存の公式ドライバは `vector<int>` でありbyte経路は通らないが、値域が狭い公式ケースは整数の直接表へ入る。GCC・Linux・公式verify・全体の `make test` / `make verify` / `make docs` はローカル未実施。

## 再現

保存したsnapshotは末尾の `.txt` を取り除いて `.build/subseq-byte/` へ置く。`final-build.py` は作業ツリーの公開ヘッダをfinalへコピーし、baseは `baseline.hpp.txt` の保存内容を `base/blueberry/string/count-subsequences.hpp` に置く。`bench.cpp`、`int-bench-final.cpp` はそれぞれ `bench.cpp`、`int-bench.cpp` に置き、`test.cpp`、`int-test.cpp`、`large-bench.cpp` も復元する。`python final-build.py` 後に `run-final.py`、`int-run-final.py`、`memory.py` を実行する。ドライバの出力先は `.build` 内で上書きされる。Windows SDK・MSVC・Clangのパスは環境固有のため変更が必要。測定JSONのhashは測定時のWindows実バイト列に対するもので、改行変換後のGit blob hashとは区別する。
