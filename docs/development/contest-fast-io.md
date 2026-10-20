---
title: テンプレート形式の高速入出力
---

## 実装

`blueberry/utility/contest-fast-io.hpp`を追加した。
`blueberrytemplate`の`scan`、`print`、`LL`、`STR`、`O`、`YesNo`、`YESNO`を、
既存のcheckedな`FastInput` / `FastOutput`上で使える。
`init_io(precision = 30)`はコンテナ内部も含めた浮動小数点出力の固定精度を設定する。
`scan`、`print`、`flush`は成功をboolで返す。`LL`と`STR`は戻り値を捨てるため、正しい入力を前提とする。

- `FastInput::read` / `FastOutput::write`にpair、vector、deque、arrayの再帰処理を追加した。
  入力先の形状を維持し、vector<bool>、空形状、途中失敗に対応する。
- `FastOutput::write(value, precision)`を追加した。128byteのstack領域で足りない場合だけ
  動的領域を確保する。従来の`write(value)`は最短round-trip表現のまま。
- 型別名・補助関数・stream overload・共通マクロを内部ヘッダへ移し、
  既存の`contest-template.hpp`の公開APIとiostreamによる動作を維持した。
- 両テンプレートの同時includeは明示的なコンパイルエラーにした。
  `all.hpp`への追加や、原文`blueberrytemplate.h`の変更は行っていない。
- 高速版はstdin/stdoutを借用するバッチ用途。対話用途は既存の`FastInput<true>`を使う。
  `main`を自分で書き、正常終了または明示的な`flush()`で出力する。

公開APIの契約と実行例は[Contest Fast I/O]({{ '/blueberry/utility/contest-fast-io.hpp.html' | relative_url }})と
[Fast I/O]({{ '/blueberry/utility/fast-io.hpp.html' | relative_url }})、提出用の出発点は
`templates/contest-fast-io.cpp.example`にまとめた。

## 性能

[測定条件・全ログ・採否](measurements/template-fast-io/README.md)を保存した。
GCC 13.3、`-O2 -DNDEBUG`、同一CPU・入力、5回の中央値では、
100万組の整数の入力から出力までがiostream 179.81msに対し68.74msだった。
25万個のdoubleを固定小数点30桁で出力する時間は75.36msに対し14.92msだった。

初回測定は別作業とCPUコアが重複していたため保存したまま採用判断から外し、
別の物理コアに固定して測り直した。変更前・採用版・iostream・直接Fast I/Oを
各roundに混ぜ、432実行全てで期待出力とのbyte一致とソースhashの不変を確認した。
accessorの参照を呼び出しごとに1回取得する改善も比較したが、効果はケースごとに混在した。
細かな速度差やFastest提出時の表示時間を一般的な高速化率として扱わない。

Fastest提出3件を調査し、mmap・AVX2・十進桁の直接加算・バッチ変換などを確認した。
対応型・符号・空白・EOF/不正入力・pipeの契約が異なるため、直接の速度比は示さない。
既存のportable backendを再利用し、第三者の提出コードはコピーしていない。

## 検証の再現

`validation/`は[測定資料の検証ログ](measurements/template-fast-io/validation/)を指す。
初回の空引数`print()`の警告エラーも`new-api-initial.log`に保存し、
修正後の結果と区別した。キャッシュ版の再検証は`new-api-current.sh`で再現できる。
64bitドライバは正常終了時のflush、128bitドライバは明示的なflushを検証する。

`make verify`に任意の`VERIFY_PATHS`を追加した。省略時は従来どおり全対象を検証する。
この変更に関係する公式ドライバだけを実行する場合は次のように指定できる。

```console
make verify VERIFY_REPEATS=3 VERIFY_TIMEOUT=1200 VERIFY_PATHS="verify/utility/contest-fast-io.test.cpp verify/utility/contest-fast-io-128bit.test.cpp verify/utility/fast-io.test.cpp verify/utility/fast-io-128bit.test.cpp verify/utility/contest-template-128bit.test.cpp verify/utility/all-headers.test.cpp"
```

ローカルのC++確認結果は次のとおり。

- `make check`: unit test 149件、114ページの構造と115個の使用例、
  header 114件・verify 152件の厳格警告付きコンパイル、乱数テスト112本×20 seedが成功。
- 新APIはGCC/Clangとgnu++20/23の組合せでも確認した。キャッシュ版の再検証と
  ASan/UBSan seed 42、正常終了時のsingleton flushも成功。
- `make verify`: 上記6ドライバの公式ケースを各3回実行し、全てAC。
  公式結果のrevisionは`46a723272443c91bef717730e7931d4492e51236`。
  外側のシェルによる終了値の記録だけが失敗したため、そのエラーを残し、
  `official-result-check.log`で対象・反復数・成功状態を独立に確認した。

- `make docs`はLinux領域の同一コミットで公開前モードとして成功。
  6件だけの測定レポートは公開用に流用せず保存した。
- `JEKYLL_ENV=production bundle exec github-pages build`と
  `python3 scripts/check_site.py --without-metrics`で5カテゴリ・113APIページの
  サイト構造検証が成功。新規ページ間のURLを公開先に修正した。

各試行の環境の違いと結果は[検証ログ一覧](measurements/template-fast-io/validation/README.md)にまとめた。
全件のAC証跡と測定レポートを含む公開整合性はPRとmainのGitHub Actionsで確認する。
未実行・timeout・失敗を成功や0秒として扱わない。
