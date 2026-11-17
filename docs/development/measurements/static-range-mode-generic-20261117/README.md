> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# Static Range Mode の型要件確認

一般型 T はコピー可能、`<` は全順序、`==` はその同値関係と一致する契約である。従来の一般型経路は T の配列を `std::sort` するため、移動・交換も必要だった。実際にコピー可能で move を明示的に delete した型、および move は可能だが ADL の swap を delete した型でコンパイルが失敗する。

型が move 構築、move 代入、交換のいずれかをできない場合は、T 自体を移動せず添字を `a[i] < a[j]` でソートし、`==` で同値をまとめる。それ以外の型は従来の処理を使う。代表位置はこれまでどおり配列中の同じ値の位置で、同率時の選択は規定しない。

## 検証

- `copy-only.cpp` は copy 構築・代入が可能で move 構築・代入が delete された型の最小例。旧版では Windows Clang 18 gnu++20 のコンパイルが失敗し、エラー全文を `copy-result.json` に保存した。`no-swap.cpp` は move 可能だが swap を delete した型の例で、旧版失敗・候補版成功の記録を `no-swap-result.json` に保存した。いずれも失敗を成功やゼロに置き換えていない。
- `copy-only-test.cpp` は両型について空・単要素・定数・2値・全異値・3値以上の混在と、全半開区間を直接数え、頻度と代表位置の値を照合した。MSVC 14.43 C++20 / C++latest と Windows Clang 18 gnu++20 / gnu++23 の4構成が成功した。コマンドと結果は `fallback-result.json` に保存した。
- `check.cpp` は整数の符号付き・符号なし8/16/32/64bit、bool、char8_t/16_t/32_t、wchar_t の最小値・最大値などを含む配列で全区間を愚直比較する。旧版と候補版ともに上記4構成で成功し、それぞれ `result.json`、`candidate-result.json` に保存した。
- 通常型に追加分岐が生成されないことを、Windows Clang 18 `--target=x86_64-pc-windows-msvc -std=gnu++20 -O2 -DNDEBUG -fno-ident -S` で `int` と可動な Key の両インスタンスを生成して確認した。旧版・候補版は双方8808行、非圧縮 SHA-256 は `2a3a84a3e1893e4e9870b2a86e587a0ad2850e94c0e3c253bdf540f827dfaa33` で完全一致。コマンドと値は `assembly-result.json`、差分は空の `assembly.diff`、実データは `baseline.s.gz` / `candidate.s.gz` に保存。旧試行の出力も同じ非圧縮 SHA-256 で、`assembly-result-v1.json` と `assembly-v1.diff` を残している。

アセンブリの一致はこのコンパイラ・flags・対象型での結果であり、他の環境の実行時間を保証しない。Linux GCC / Clang と sanitizer はローカル未実行。

## 再現

`python docs/development/measurements/static-range-mode-generic-20261117/replay.py` は `.build` 配下の新しい時刻付きコピーで、旧版の期待どおりのコンパイル失敗、特殊型の修正後テスト、既存型の全区間比較、アセンブリ生成・同一性検査を実行する。保存済みファイルは上書きしない。Visual Studio、Windows SDK、Clang のパスは各スクリプト内に記載している。個別に実行した場合も生成ファイルを上書きするため、作業用コピーを使用すること。
