# LC未対応問題の追加: 数論・数列

2026-10-17。LC対応表の未対応問題から、ACLに公開APIがない5機能を追加しました。
公開問題の一覧自体は更新せず、既存の253問に対する実装・verifyの対応を増やします。

| LC問題 | API | 用途 |
| --- | --- | --- |
| Primitive Root | `primitive_root(p)` | 64bit素数の乗法群の生成元 |
| Tetration Mod | `tetration_mod(base,height,mod)` | 底と法が互いに素でない場合も含む冪塔の剰余 |
| Bernoulli Number | `bernoulli<Mint>(n)` | B₀からBₙまで。B₁=-1/2の規約 |
| Bell Number | `bell<Mint>(n)` | 集合の分割数を0要素からn要素まで |
| Partition Function | `partition<Mint>(n)` | 整数分割数を0からnまで |

原始根は既存の64bit素因数分解を再利用します。テトレーションは底・高さが64bit、
法が正の32bit全域です。数列3種は既存FPSとACLのmodint・NTTを再利用します。
型・NTT長などの前提と境界条件は各APIページに記載しています。
既存の公開APIに変更はありません。

カタログ・一括入口・README・生成したLC対応表にも登録しました。
実装＋専用verifyありは112問から117問、未対応は133問から128問になります。
この件数はドライバの登録状況であり、検証成功の代用にはしません。

## 調査と性能

Fastestの提出ソースと複数の公開実装を調査し、コードは独立に実装しました。
取得した第三者ソースはignoredな作業領域に置き、配布するヘッダには転記していません。
比較候補・採否・コンパイラ・入力・反復ごとの生ログは以下に記録します。

- [原始根の調査・比較](lc-expansion-primitive-root.md)
- [テトレーションの調査・比較](lc-expansion-tetration.md)
- [数列3種の調査・比較](lc-expansion-fps-sequences.md)

原始根は複数の位数判定で二乗計算を共有します。Bell数は逆階乗を直接生成して
FPSの指数計算を1回にし、Bernoulli数は階乗の逆元を1回だけ求めます。
整数分割数は五角数定理で作った疎な級数を反転します。
テトレーションはgcdで法と底の共通因子を先に除き、短い累乗では分解を省きます。

構築・操作・I/O・メモリを区別し、同じ入力・コンパイラ・フラグで候補を比較します。
公開提出の表示時間とローカルの合計時間を直接比較しません。
小さな測定値にはプロセス起動や共有環境の変動が含まれ、普遍的な最速は主張しません。

## 統合検証

Windows上のソースと同一内容のLinux作業コピーで検証しました。GCC 13.3とClang 18.1、
ACLは既存の固定版を使っています。コンパイル・random testは以下を確認しています。

| 確認 | 結果 |
| --- | --- |
| `make check`（GCC、gnu++20） | 成功。147 unit tests、文書のC++例、header・verifyのcompile、各20 seedのrandom test |
| `make compile-test random-test`（GCC、gnu++23） | 成功 |
| 同上（Clang、gnu++20 / gnu++23） | 両方成功 |
| `CXXFLAGS=-DNDEBUG make include-test` | 成功。ACLを`-isystem`で読み、自作ヘッダへの警告と分離 |

`make check`は`make test`を含みます。文書例は111ページ・112実行例を確認しました。
初回のWindows作業領域では既存のsnippet compileがI/O待ちで120秒を超え、
147件中1件がtimeoutしました。この結果は成功扱いにせず、Linux上で全件を再実行しています。
初回のGit環境変数の引き継ぎによるテスト失敗も別ログに残し、環境変数を解除して再実行しました。

新規5本は公式ケース計66件をローカルで各3反復し、すべて成功しました。
ローカルで開始した全148本のverifyは未完了であり、全体成功の根拠には使いません。
異なる反復数・ソース状態の時間を混ぜません。

mainへ入った[既存ライブラリの性能改善PR #47](https://github.com/blueberry1001/Blueberry-library/pull/47)を取り込んだ後、
`fps_sequences`と既存FPSのrandom testをGCC / Clang × gnu++20 / gnu++23で再実行し、
各20 seedすべてが成功しました。数列APIの実装自体は変更していません。
取り込み前の性能測定はその時点のFPSを使った結果として保持します。

初回の[PR #48のCI](https://github.com/blueberry1001/Blueberry-library/actions/runs/39009203173)では、
コンパイラ4構成と選択した公式verify・文書検査が成功しました。
main取り込み後の[CI](https://github.com/blueberry1001/Blueberry-library/actions/runs/39009995996)は、
追加5本・一括入口1本・性能改善関連16本の計22本を選択してすべて成功しました。
残る126本は依存関係と内容が一致する既存のAC記録を保持し、再実行していません。
選択理由は「新規・変更された依存先、または有効なAC記録がない対象」です。
全体の再実行は行わず、以後も変更・影響範囲に応じたverifyを使います。
最終CIと公開確認は
[PR #48](https://github.com/blueberry1001/Blueberry-library/pull/48)の検証結果を参照してください。
CIの選択記録とソースに紐づくAC記録は、Actions artifactの`ci-plan`と
`verification-metrics`に保存されています。取得した写しは`.verification/final-ci-plan`と
`.verification/final-ci-metrics`に置いています。
