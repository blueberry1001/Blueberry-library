# Dijkstra入力の整数パース候補

## 調査と結論

Linuxでの[同条件比較](csr-end-to-end-linux-gcc.json)では、Library Checker制約のsparse入力について、
GCC版CSRのprocess中央値25.27msのうち、辺配列への読取り区間は13.12ms、CSR構築は5.34ms、
探索は2.48msだった。最速提出のprocess中央値は12.48msだが、その段階別値は未測定であり、
両者の差をそのままパース差とはみなせない。一本道ではCSRの出力区間7.17msも別途検討が必要である。

現行`FastInput`は1桁の特例と8桁SWAR変換を持つが、短い複数桁の整数では、
各桁で型上限との比較を行う。[試作](parse-short-limit-candidate.hpp.txt)は、
累積値が0のときだけ、`numeric_limits<T>::digits10`桁までの変換を先に行い、
その後の桁は従来のoverflow検査へ戻す。前者で作る値は当該型の正の最大値を超えない。
先頭zeroを多数含む場合や8桁SWARの後も、残りの桁を従来どおり検査する。
不正tokenの破棄、符号、EOF、FILEエラー、公開APIは変更しない候補である。

今回の比較では一貫した全体改善が確認できず、一部の標本が大きく揺れたため、
**候補は公開headerに採用しない**。最速提出との差を解消したとは判断しない。

## 正しさの検査

[baseline](parse-baseline.hpp.txt)は測定時の公開`fast-io.hpp`の実バイトと一致し、SHA-256は
`0a73209974def4a36a5f5c2986f774e679dd3fb98ce1442f0c138249a87443d2`。
候補のSHA-256は`b91ad49739267c92e11ee7e304166c5d3c745bdcdfce9147a8277bb0749b9f21`。
公開headerは変更していない。[実行スクリプト](run_parse_candidate.py)は2つのsnapshotを
別々のinclude overlayに配置する。

既存`tests/random/fast-io.cpp`をbaselineと候補の両方でコンパイルし、MSVCとWindows Clangの
各seed 1、2、17を実行した。符号・型の上限と下限・不正token・overflow後の回復・長い先頭zero・
64KiB境界・EOF・対話モードを含む、利用可能なテストはいずれも通過した。ClangはWindows版MSVC
標準ライブラリが128bit整数型を`is_integral`として認識しないため`__SIZEOF_INT128__`を外し、
既存baselineの狭いunsigned型変換警告を抑制した。MSVCも既存のC4146を抑制した。
128bit、Linux固有のFILEエラー、pipeでの検証はこの結果に含まれず、正式な採用には別途必要である。

初回はMSVC baselineが既存C4146警告によりコンパイル失敗し、[記録](parse-short-windows.json)を残した。
その警告の抑制後、Windows Clang baselineが128bit型特性と既存のconstant-conversion警告で
コンパイル失敗した[記録](parse-short-windows-v2.json)も残した。どちらも測定値はない。
上記の範囲を明示した後の[実行結果](parse-short-windows-v3.json)には、全引数・ビルド結果・
各seed・入力、header、driver、binaryのhashを保存した。

## 性能比較

Windows 11、MSVC 19.43.34809とWindows MSVC ABIのClang 18.0.3、C++20、`-O2`相当・assert無効、
論理CPU 0。CSR単一終点driver・共通FastOutputと同じ入力を用い、2方式を実行順を交替して
各warmup 1回、測定5回。前述の5入力に対する全出力は、独立の最短距離・経路検査を通した
参照出力とbyte単位で一致した。パース区間には入力と辺vectorへの格納が含まれる。
process時間はファイルopen、起動、読取り、構築、探索、出力、終了を含み、hash計算は含まない。
OS負荷とファイルキャッシュの影響を含み、別compilerや別run間で速度比を算出しない。

| 入力 | MSVC 読取り baseline / 候補 | MSVC 全体 baseline / 候補 | Clang 読取り baseline / 候補 | Clang 全体 baseline / 候補 |
| --- | ---: | ---: | ---: | ---: |
| 乱数辺 | 16.35 / 15.82ms | 38.10 / 35.49ms | 18.06 / 17.23ms | 37.89 / 37.36ms |
| 重み0 | 13.82 / 13.02ms | 34.93 / 34.22ms | 17.01 / 15.99ms | 41.60 / 42.20ms |
| 一本道 | 2.53 / 2.51ms | 21.23 / 18.75ms | 2.82 / 4.72ms | 24.36 / 21.80ms |
| DAG | 16.98 / 16.58ms | 40.73 / 39.10ms | 19.76 / 18.62ms | 47.46 / 46.02ms |
| 到達不能 | 14.41 / 14.37ms | 29.24 / 31.71ms | 16.83 / 16.03ms | 33.35 / 34.09ms |

Clang一本道の候補は読取り5標本が2.68、4.72、4.93、2.61、4.81msとばらつき、
baselineの2.80〜2.91msと比べて中央値は悪化した。2標本はbaseline未満でもあり、
原因をコード自体の回帰と断定しない。MSVC到達不能とClang重み0・到達不能では全体中央値が
増え、他の短縮も一部は小さい。固定CPUでも分布に揺れがあるため、これだけで汎用変更の
採否を肯定できない。対応する全100標本、最大working set、入力hash、出力hashはJSONにある。

次に調査する場合は、LinuxのGCC・Clang、128bitとpipeを含む既存契約テスト、公式入力、
入力単独・出力単独・end-to-endを同条件で確認し、短桁・zero・最大値・多量の先頭zeroを分ける。
順位上位の専用parserは入力形式と型の前提が異なるため、同一契約の汎用parserとして扱わない。
