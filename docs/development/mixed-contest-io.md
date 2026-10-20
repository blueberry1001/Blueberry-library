# Contest Fast I/Oの標準stream混在修正

## 不具合

`init_io(); LL(n,m); vector<ll> a(n); cin >> a; O(a);`へ`2 3 7 23`を渡すと、
`7 23`ではなく`0 0`を出力していた。
`LL`が使うFastInputの`fread`が後続の値まで先読みし、独立した`std::cin`のバッファには残らなかった。
従来のvector抽出演算子のテストはstringstreamを差し替えており、同じstdin上の混在を確認していなかった。

基準revisionは`737dd7ed53365c86d1ddf2cd76fc337495571d02`。
未変更の基準をコンパイルした出力`0 0`と、修正後の`7 23`を
[sources.json](measurements/template-compatibility/mixed-io/sources.json)へ保存した。

## 修正

- `init_io`で本物の`std::cin/std::cout`をFastInput/FastOutputの共有バッファへ接続する。
- `cin`のget areaの消費を、再充填時と`scan`の前にFastInputの位置へ反映する。
- `cout`の書き込み・flushをFastOutputへ渡し、`O`との出力順を共有する。
- singletonの破棄で出力を排出し、元のstreambufへ戻してからadapterを破棄する。
- 標準の書式指定・独自型のstream演算子・状態flagを維持する。
- `fin >> value`を追加し、`scan`の高速パーサーを連結可能な`>>`記法で使う。

`fin`は成功・失敗を保持し、失敗後は`fin.clear()`まで後続の読み取りを行わない。
対応型・10進数・範囲検査・区切り消費は`scan`と同じで、標準streamのmanipulatorは使わない。

FastInputの整数パーサのループは変更していない。
単体`fast-io.hpp`に追加するのは内部adapterへのfriend宣言だけで、他のアルゴリズムへの依存は増やさない。
初期化後の固定バッファは入力64 KiBと出力64 KiB、adapterの管理領域はO(1)。

標準streamを最初に使う前に`init_io()`を呼ぶ。
`scan`の区切り消費と`cin`の区切り保持の違い、putback可能な範囲、FILE直接操作との制約は
[API文書](../utility/contest-fast-io.md)に記載した。

## 比較測定

同じWSL2・Core i7-14650HX、同じ37,777,421byteの入力、同じ出力ファイル条件で比較した。
GNU++20、`-O2 -DNDEBUG`、100万組、固定seed `467203`、warmup 1回、測定7回。
前後を交互に実行し、出力hashの一致を毎回確認した。表は中央値、括弧内は最小値、単位はms。
表の修正後はcommit `ff7d44aab3e4777dcd94d6ad7cb0b23bc12a7fcc`のソースで測定した。

| compiler | 処理 | 修正前 | 修正後 |
| --- | --- | ---: | ---: |
| GCC 13.3 | scanのみ＋checksum | 25.281 (24.716) | 25.511 (25.095) |
| GCC 13.3 | printのみ | 9.749 (9.440) | 9.905 (9.750) |
| GCC 13.3 | scan/printのMany A+B | 44.059 (42.471) | 44.645 (43.984) |
| Clang 18.1 | scanのみ＋checksum | 27.403 (26.844) | 23.353 (23.058) |
| Clang 18.1 | printのみ | 10.125 (9.760) | 10.271 (9.978) |
| Clang 18.1 | scan/printのMany A+B | 47.510 (46.777) | 47.389 (46.064) |

小さい差を高速化・回帰の根拠にはしない。
標準の書式変換を通す`cin >> vector`版は、修正後でGCC 110.398ms、Clang 118.833msだった。
同じ入力・出力の`fin >> vector`版はGCC 44.255ms、Clang 47.073msで、`scan`と同程度だった。
修正前の混在版は不正解なので比較対象にせず、欠測を0秒に置き換えていない。

[全サンプル・環境・入力出力hash](measurements/template-compatibility/mixed-io/results.json)、
[測定スクリプト](measurements/template-compatibility/mixed-io/benchmark.py.snapshot)、
[測定ログ](measurements/template-compatibility/mixed-io/benchmark.log)を保存した。
再現時は基準revisionのcloneにこの変更の候補ヘッダ2個を重ね、
スクリプトをリポジトリ直下へコピーして`python3`で実行する。
測定は基準revisionへ変更を重ねたcloneで行った。cloneのHEADを未変更ソースの検証結果と扱わず、
検証コピーとの一致・改行正規化後のhashを`sources.json`で確認する。

## 回帰テスト

- GCC/Clang・GNU++20/23・高速/stream両テンプレートの8構成。
- 提示コード、空vector、交互読み取り、64 KiB境界、nested 128bit、vector<bool>、独自抽出演算子。
- getline/peek/get/unget、EOF・fail/clear、再初期化、boolalpha、書式指定、cout/Oの出力順。
- 標準streamの13ケース×8構成に`fin`の6ケース×4構成を加えた128ケースが成功。
- `endl`が入力待ちの前に排出する8回のpipe検証も成功。
- 独立したGCC ASan+UBSan検証ではNUL・0xffを含む131,209byteの読み取り、putback、EOF例外、
  出力順、singleton破棄後のstreambuf復帰を確認した。

[回帰テストログ](measurements/template-compatibility/mixed-io/focused-tests.log)、
[fin追加後の回帰テストログ](measurements/template-compatibility/mixed-io/focused-fin-tests.log)、
[sanitizerログ](measurements/template-compatibility/mixed-io/sanitizers.log)、
[独立検証ソース](measurements/template-compatibility/mixed-io/sanitizers.cpp.snapshot)を保存した。
公式verifyも64bit版は`fin`と`cin >> vector`、128bit版は`scan/cin/fin`の交互読み取りを使う。
全体のrequired checksは、この変更のPR・mainのGitHub Actionsを参照する。

初回の共有bridgeのみの確認では153 unit test・124文書例の後、verify 2本で未初期化変数の警告を検出した。
verifyの変数を0初期化して修正し、`fin`追加後の最終ソースで警告付きコンパイルの通過を確認した。
初回の測定・ソースhash・失敗ログは`mixed-io/bridge-only/`へ分けて保存し、最終ソースの結果と混同しない。
対象文書例の個別確認ではACLのinclude指定を付け忘れて失敗したため、そのログも保持し、
`CPPFLAGS=-I.deps/ac-library`を指定した[再実行](measurements/template-compatibility/mixed-io/docs-rerun.log)で通過を確認した。

## C++23のprint名衝突

初回CIのClang・GNU++23・libstdc++ 14で、文字列を先頭に取る`O/print`と`std::print`の曖昧さを検出した。
`O`を`::print`へ修飾し、`__cpp_lib_print`がある場合だけ文字列先頭の転送overloadを追加した。
元のconst参照版へ明示的に委譲し、不要コピーと再帰を避ける。
明示template引数での呼び出し・関数アドレス取得は元のoverloadを選べる。
文字列先頭とbit-fieldの組み合わせは、値に変換するかconst参照版を明示する契約をAPI文書へ記載した。

追加の出力lookupケースを含む136ケースと8回のflush検証が、GCC/Clang・GNU20/23・両facadeで成功した。
WSLの標準ライブラリ13に加え、libstdc++ 14.2.0のヘッダをローカルへ展開し、同じ8構成で成功を確認した。
実物の`std::format_string`と同signatureの候補を用いた独立した16構成の検査も成功した。
2ページの最小C++例と文書構造も再確認した。

この追補を前の測定結果へ混ぜず、[CI失敗ログ](measurements/template-compatibility/mixed-io/cpp23/initial-ci.log)、
[回帰テスト](measurements/template-compatibility/mixed-io/cpp23/focused-tests.log)、
[libstdc++ 14の検査](measurements/template-compatibility/mixed-io/cpp23/libstdcpp14.log)、
[独立検査](measurements/template-compatibility/mixed-io/cpp23/print-audit.log)、
[ソースhash](measurements/template-compatibility/mixed-io/cpp23/sources.json)を分けて保存した。
GNU20・`-O2 -DNDEBUG`のベンチマークdriver 2個をGCC/Clangで生成した4個のassemblyは、
`ff7d44a`と追補後でbyte単位に一致した。[hash](measurements/template-compatibility/mixed-io/cpp23/assembly.json)を保存した。
