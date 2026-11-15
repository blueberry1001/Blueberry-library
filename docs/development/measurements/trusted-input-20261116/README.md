# 入力形式が保証された整数パーサの試作

FastSetの[既存比較](../fast-set-lc-20261116/README.md)では、入力とqueryの先読みを含む
読み込みが大きな割合を占めた。順位上位の提出は専用I/Oを持つが、契約が異なるため、
そのまま汎用`FastInput`と同等とは扱えない。ここでは安全な`FastInput`を変更せず、
条件を限定した[独立prototype](trusted-input.hpp.txt)を比較する。元提出のソースは使用していない。
このprototypeは公開APIに追加していない。

## 条件と制約

- 入力は非対話の整数token列。空白はASCIIのspace、tab、改行、CR、vertical tab、form feed。
- 32bitまたは64bitの符号付き・符号なし整数を検証した。先頭の`+`、符号付き整数の`-`、
  任意個の先頭zero、型の上限と下限、区切りなしの最終EOFに対応する。符号なしの負号は前提外。
- 各tokenは少なくとも1桁の十進数字からなり、指定型の範囲内であることが必要。途中の無効文字、
  符号だけ、型の範囲外値などの検出・回復は提供しない。無効な入力での結果は保証しない。
- バッファは64KiB、`fread`による補充でtokenや符号が境界をまたげる。EOF以降はfalseを返し、
  出力引数を変更しない。FILE読取りエラーとEOFの区別はしないため、読取りに失敗しないことが前提。
- 他のバッファ付きreaderと同じFILEの読取りを混在させない。FILEは外部所有で、readerより長く
  生存させる。同じreaderを複数スレッドから同時に操作しない。対話的な応答を必要とする入力には使わない。
- 実装テンプレートが受け付ける他の整数型、異なるlocale・非ASCII空白、入力エラーや不正値は
  今回の検証対象ではない。既存`FastInput`のcheckedな契約を引き継ぐものではない。

## 有効入力の検査

Windows 11、MSVC 19.43.34809とWindows MSVC ABIのClang 18.0.3、C++20、`-O2`相当で、
baselineとprototypeを別binaryにした。符号付き・符号なし32/64bitそれぞれ、型の両端・符号・
符号付きzero・乱数値・最大70000個の先頭zero・最終EOFを含む。tokenの開始位置は64KiB境界の
前後に配置した。4型×2 compiler×2方式×regular file/pipeの32構成で、Pythonが生成した
期待出力と完全一致した。fixtureのhash・境界offset・終了コード・出力hashは各JSONにある。
不正入力やFILEエラーは前提外であり、正しい値への変換や検出を主張しない。

## FastSet形式の読取り比較

N=100万、Q=50万、seed 20261116で、同じbit文字列とquery型・位置を各方式に与える。
`short`は位置0〜9、`varied`は位置を一様乱数、`maximum`は位置999999とし、各query型は0〜4。
driverはbit文字列と全queryを読み、query型・位置の64bit checksumを計算する。期待値はPythonで
別に生成し、全標本をbyte単位で照合した。FastSetの構築・query実行や回答出力は含まない。
整数列以外の文字列読取りも同じreader経由で行い、prefixとqueryの所要時間を分けた。
前述の正式なFastSet driverとは動作・入力が異なり、最速提出との倍率ではない。

CPU affinity mask `0x1`、ファイル入力、warmup各1回、順序を交替した[最初の5回](windows.json)と、
時刻計測箇所を増やした[独立した15回](windows-v2.json)を別々に保存した。以下は後者の中央値ms。
`query`は整数の読取りとchecksum、`wall`はサブプロセスの起動から終了までを含む。

| 条件 | MSVC query checked / trusted | MSVC wall checked / trusted | Clang query checked / trusted | Clang wall checked / trusted |
| --- | ---: | ---: | ---: | ---: |
| short | 2.60 / 2.22 | 19.22 / 18.77 | 5.73 / 2.51 | 18.69 / 15.88 |
| varied | 10.49 / 6.43 | 27.30 / 21.46 | 8.26 / 5.33 | 22.92 / 20.63 |
| maximum | 9.59 / 6.09 | 28.01 / 22.52 | 10.25 / 6.47 | 24.55 / 20.63 |

最初の5回ではMSVC variedのwall中央値がchecked 27.47ms、trusted 35.15msで逆転した。
15回の別runでは上表のとおり逆転せず、最初の値を後の測定値で置き換えていない。
15回のMSVC variedでは、`wall - main内時間`の範囲がchecked 8.16〜20.59ms、trusted
8.32〜28.30msだった。`main`内でパース後のEOF確認・bit集計・出力・メモリ取得にかかった
時間の中央値は両方式約0.07ms。差分の残りにはプロセス起動・終了、親側の待機、OSの
スケジューリング等が含まれるが、支配要因は特定できていない。各run・各標本はJSONに保存し、
中央値だけから安定性を推測しない。OS負荷、頻度、ファイルキャッシュの揺れも残る。

Windowsでの比較は同一入力・同一compilerの各run内だけで行う。別compiler、Linux、先のFastSet
本体測定とは時間を混ぜない。ピークworking setは両方式約5.8MiBで、文字列やstdioを含む
プロセス全体の値である。初回と再測定のdriver・runnerは各JSONのhashと一致する
`driver-v1.cpp.txt`・`driver-v2.cpp.txt`および対応する`run_windows-v*.py.txt`に保存した。

## Linuxでの同入力比較

[GCC](linux-gcc.json)と[Clang](linux-clang.json)はLinux x86-64、AMD EPYC 9V74、CPU affinity
`[0]`で別々に実行した。GCC 15.2.0とClang 21.1.8、`-std=gnu++23 -O2 -DNDEBUG -mavx2
-mpopcnt -Wall -Wextra -Wshadow -Werror`、warmup各1回、測定各15回、順序交替である。
記録されたCPU機能を確認してからAVX2・POPCNTを指定した。各compilerの16構成のfile/pipe有効入力
検査と90標本の出力照合はすべて通過し、失敗はなかった。整数読取りとchecksumの中央値ms、
括弧内はプロセスwall中央値msである。

| 入力 | GCC checked / trusted | Clang checked / trusted |
| --- | ---: | ---: |
| short | 3.42 / 3.12（7.57 / 7.18） | 4.10 / 3.67（7.73 / 7.24） |
| varied | 6.91 / 4.92（10.92 / 8.86） | 7.94 / 5.49（11.52 / 9.04） |
| maximum | 6.64 / 4.69（10.74 / 8.51） | 7.67 / 5.16（11.26 / 8.71） |

単一compiler・同一入力の各run内で比較した値であり、Windowsや過去のFastSet実行時間とは
混ぜない。このdriverはquery配列の作成、FastSet構築・操作、答えの出力を行わない。
特に読み込み時の契約がcheckedとtrustedで異なるため、同等の失敗処理を持つI/O同士の比較ではない。
最大RSSはプロセス全体の高水位で、親プロセスからの起動やOSの扱いも影響し得るため、
バッファ単体のメモリとして解釈しない。compiler・入力・全標本・エラーは各JSONを参照する。
Linux上の`fast-io.hpp`はGit改行正規化後のSHA-256が
`7cbad0126b72751b56e746c34c14a019248b747320894b1b59b8038eb4aafaa1`であり、
上記Windows上のCRLFを含む実バイトhashとは異なる。

## 未完了と再現

上記はparse実験である。下記のFastSet end-to-end比較も行ったが、公式verifyと8/16bit型の
検証は未完了。安全なchecked APIの代替としない。
現行`driver.cpp`はLinux用peak RSS取得を加えた版で、上記Windows測定時のsnapshotとは異なる。
公開API化は性能・契約・利用方法を別途検討するまで行わない。

Linuxでは、同じ生成入力・file/pipe境界検査と、AVX2・POPCNTを明示した15回比較を次で実行できる。
結果出力先は新しいパスを使う。CPU機能不足、コンパイル・一致検査・RSS取得失敗は失敗として記録し、
欠測を0とみなさない。

```sh
python3 docs/development/measurements/trusted-input-20261116/run_linux.py --compiler g++ --output /tmp/trusted-gcc.json
python3 docs/development/measurements/trusted-input-20261116/run_linux.py --compiler clang++ --output /tmp/trusted-clang.json
```

## FastSetを含む同条件比較

[driver](fast-set-end-to-end.cpp)は既存FastSetを実際に構築し、queryを読みながら操作・出力する。
同一のdriverを`FastInput`とtrusted prototypeに切り替えてコンパイルする。これに既存の
`verify/data-structure/fast-set.test.cpp`と、上位提出403023または278227を加えた4方式を、
[Linux runner](run_fastset_linux.py)で同じ入力・compiler・flags・環境において比較する。
既存verifyと共通driverは、入力失敗の検査や出力の明示flushが異なり得るため別方式とする。

既存のFastSet比較と同じ生成器を使い、6分布とN=1・63・64・65・4095・4096・4097の境界を
生成する。`std::set`による期待出力と全方式・全標本をbyte単位で照合する。通常file入力、
warmup各1回、既定4回、固定CPU、実行順交替とし、process wall、最大RSS、入力・出力・ソース・
binaryのhashを記録する。欠測・timeout・不一致は失敗として残し、既存の出力JSONを上書きしない。
prototypeは入力形式と整数範囲が保証される場合のみ対象であり、無効入力やI/Oエラーの扱いは
checked版と異なる。上位提出も単一区切りやCPU機能などの独自の前提を持つため、同一入力の測定値を
同じ汎用契約の比較として解釈しない。

順位上位の提出ソースはライセンスを確認できていないため収録しない。runnerは実行時に公開APIから
取得して既知のSHA-256と照合し、一時ディレクトリでコンパイル・実行して削除する。第三者コードを
実行するので、機密情報を渡さない隔離されたCI環境で使用する。手元で取得済みの原本は`--external`で
指定でき、hash不一致は実行しない。

```sh
python3 docs/development/measurements/trusted-input-20261116/run_fastset_linux.py --compiler g++ --output /tmp/fastset-trusted-gcc.json
python3 docs/development/measurements/trusted-input-20261116/run_fastset_linux.py --compiler clang++ --output /tmp/fastset-trusted-clang.json
```

driverはWindows MSVCのN=65・Q=1000、6分布についてchecked/trustedともoracleと一致した。
これはLinux CIでの性能測定、上位提出との比較、公式ケースのverifyではない。

### 初回と再実行

同じ13入力について、AMD EPYC 9V74での[GCC初回](fast-set-linux-gcc-initial.json)・
[Clang初回](fast-set-linux-clang-initial.json)と、Intel Xeon Platinum 8573Cでの
[GCC再実行](fast-set-linux-gcc-retry.json)・[Clang再実行](fast-set-linux-clang-retry.json)を
別々に記録した。GCC 15.2.0、Clang 21.1.8、`gnu++23 -O2 -DNDEBUG -mavx2 -mpopcnt`を使い、
Clangだけは上位提出のコンパイルに必要な`-fconstexpr-steps=100000000`を全方式に付けた。
CPU affinityはいずれも`[0]`。各runは4方式のwarmup各1回・測定各4回で、13ケース・260標本すべてが
独立oracleの出力と一致し、ビルド・実行・RSS取得に失敗はなかった。6分布のNは100万、Qは50万、
seedは1729。残り7ケースは小さいサイズ境界の正しさの確認である。

4方式のprocess wall中央値をmsで示す。各セルの順序は、同一driverのchecked / trusted、
既存verify、上位提出403023。外部提出のI/Oとデータ構造は異なり、同一契約の実装差ではない。

| 初回：AMD EPYC 9V74 | GCC checked / trusted / verify / 提出 | Clang checked / trusted / verify / 提出 |
| --- | ---: | ---: |
| empty | 20.07 / 19.08 / 20.15 / 10.01 | 21.44 / 18.79 / 21.99 / 10.11 |
| full | 18.88 / 16.87 / 18.87 / 8.77 | 20.43 / 17.78 / 21.14 / 9.03 |
| half | 19.98 / 17.80 / 19.94 / 8.81 | 21.71 / 18.46 / 22.04 / 11.76 |
| sparse | 19.63 / 17.61 / 19.49 / 9.70 | 21.41 / 22.46 / 22.20 / 9.97 |
| cluster | 20.48 / 18.44 / 20.37 / 9.80 | 22.15 / 21.97 / 22.98 / 9.57 |
| updates | 16.78 / 14.89 / 16.35 / 7.19 | 20.04 / 15.04 / 18.81 / 7.75 |

| 再実行：Intel Xeon Platinum 8573C | GCC checked / trusted / verify / 提出 | Clang checked / trusted / verify / 提出 |
| --- | ---: | ---: |
| empty | 25.78 / 22.79 / 25.47 / 13.50 | 26.74 / 24.41 / 28.36 / 13.49 |
| full | 24.47 / 21.56 / 23.98 / 11.68 | 25.57 / 23.15 / 27.90 / 11.77 |
| half | 25.86 / 23.22 / 25.63 / 11.73 | 26.14 / 23.88 / 28.25 / 11.68 |
| sparse | 25.89 / 23.09 / 25.57 / 13.43 | 26.69 / 23.39 / 27.91 / 13.39 |
| cluster | 26.77 / 23.64 / 26.04 / 12.62 | 27.78 / 24.54 / 28.79 / 12.57 |
| updates | 21.16 / 17.83 / 20.44 / 8.51 | 21.00 / 18.31 / 22.28 / 8.26 |

初回には大きな外れ値があり、例えばGCC fullの提出は4標本の最大が198.70ms、
Clang sparseではtrusted中央値22.46msがcheckedの21.41msを上回った。再実行のClang sparseは
checked 26.69ms、trusted 23.39msだった。CPUが異なるため、初回と再実行の中央値を混合したり、
差をそのまま修正効果とみなしたりしない。両表に示した中央値もOS負荷等の影響を受ける。
全標本・最小/最大・RSS・失敗は各JSONにある。測定方式のrunnerは初回と再実行でファイル自身の
hashが異なり、差は自身のパスを絶対パスに解決する修正だけである。初回の
[runner snapshot](run-fastset-initial.py.txt)と現行runnerを保持した。入力・期待出力、
driver、header、上位提出原本のhashは4実行間で一致する。

同一run内ではtrustedが多くの分布でcheckedより速く、再実行GCCのhalfは25.86→23.22msだった。
一方、同runの上位提出は11.73msで、trustedへの切替だけでは差が残る。上位提出と本ライブラリでは
I/O・データ構造・入力時の前提が異なるため、この差の原因をパーサやFastSet単体へ割り当てない。
上位提出の最大RSSは大きい6分布で約8.6MiB、両adapterは約5.3〜5.4MiBだったが、
これはプロセス全体のRSSであり構造体だけの比較ではない。

この結果は、形式と範囲を保証できる入力向けの選択肢を検討する根拠になる。ただし現在の試作は
無効token・overflow・FILEエラーを検出せず、8/16bit型と公式verifyも未検証であり、
既存のcheckedな`FastInput`の置換にはできない。公開APIとしてはまだ採用せず、必要な型・失敗条件・
使い方を定めたうえで境界・error・公式ケースを検証し、残る速度差も切り分けて判断する。
