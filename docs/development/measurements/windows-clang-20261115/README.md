# Windows Clangによる追加検証

2026-11-15に、Unity 6000.5.2f1のAndroid NDK同梱Clang 18.0.3を
`x86_64-pc-windows-msvc`ターゲット、MSVC 14.43.34808の標準ライブラリと
Windows SDK 10.0.22621.0に接続して実行した。Linux版ClangやGCCの結果とは別であり、
性能測定ではない。結果・全引数・標準出力と終了コード・入力ファイルのSHA-256は
[results.json](results.json)に記録した。SHA-256はWindows上のファイルの実バイトを
対象とし、Gitの改行正規化後の値とは異なる場合がある。

`gnu++20`と`gnu++23`で、FastSet初期化、Weighted Wavelet Matrix、Dijkstraの
ランダムテストをそれぞれ`-O2 -Wall -Wextra -Wshadow -Werror`でコンパイルし、
seed 1、2、17を実行した。10構成すべてでコンパイルと各実行の終了コードは0。
assertは有効にした。

FastSetはCPU自動判定をテスト時のマクロで置換してSSE2、scalar、AVX2の各経路を
別々に検証した。AVX2を強制したテストはこのPCで実行できたが、異なるCPUでは
実行できない可能性がある。自動判定は検証していない。NDKにWindows向けの
compiler-rtがなく、判定を置換しないままMSVC linkerでリンクすると
`__cpu_model`の未解決シンボルとなった。また初回の既定linker指定は
`lld-link`が見つからず失敗した。どちらの失敗も成功した構成には含めていない。

環境変数はMSVCの`include`とWindows SDKの`ucrt`・`shared`・`um`を`INCLUDE`、
MSVCとSDKのx64ライブラリを`LIB`に加え、MSVCの`bin/Hostx64/x64`を`PATH`に
加えた。各コマンドの引数はJSONに列挙している。FastSet以外にはCPU判定の
置換を適用していない。公式verify、GCC、Linux上のClang、全公開ヘッダの
コンパイルをこの記録から推測しない。
