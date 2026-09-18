# 変更範囲に応じた CI

通常の PR / main push は、変更に依存する Library Checker verifier だけを 1 回実行する。
`workflow_dispatch` は全 verifier を 3 回実行する。ローカル `make verify` の全件・既定 3 回は変更しない。
通常 CI の 1 回測定を、3 回のベンチマーク中央値と同じ条件の改善値として扱わない。

## 選択方法と記録

`scripts/plan_verification.py` が `.verification/ci-plan.json` に次を記録する。

- base / head の commit SHA と Git の変更一覧（rename は旧・新パス）
- 現在存在する全 verifier と今回選択した verifier
- 選択理由、C++ compile/random suite の要否、ドキュメント例の対象
- 基準にした成功済み main workflow の run ID / URL

PR は base SHA とチェックアウトした merge commit の merge-base、push はイベントの `before` を出発点とする。
そこから、成功済み main の `push` または `workflow_dispatch` のうち、その祖先である commit を基準にする。
直前の main CI が失敗・キャンセル中なら、さらに前の成功 commit まで変更を遡る。
これにより、次の変更が文書だけでも、その前に未検証となった C++ 変更を引き継いで検証する。
Actions API への読み取り権限・履歴がない場合、直近 100 件に成功済み祖先がない場合は全件を実行する。

各 verifier からローカル `#include` を推移的に辿り、変更ファイルを含むものを選ぶ。
引用符の相対 include、リポジトリルート指定、ローカルの山括弧 include を扱う。
base と head の両方で依存を解析するので、削除・rename 前の依存も失われない。
条件付き include はすべて依存とみなし、循環 include は訪問済み集合で打ち切る。
マクロ include、未解決のローカル include、解析失敗は安全側に倒して全件を選ぶ。

選択 manifest は artifact に保存し、verify job と実行器が HEAD・対象一覧・依存選択を再確認する。
明示的な空配列は「対象なし」であり、全件実行へのフォールバックには使わない。
対象外は `not_selected` と表示し、成功・実行済みとは表示しない。
過去の測定を現在の commit の結果として流用しない。

## 残すチェック

| 変更 | compile/random matrix | Library Checker | その他 |
| --- | --- | --- | --- |
| ライブラリ・verify・C++ テスト | 全組合せ | 依存する verifier、1 回 | Python/UI、全 docs 例、サイト検査 |
| ライブラリ文書・guide/migration | 省略 | 対象なし | 変更文書の C++ 例、全 docs 形式、Python/UI、サイト検査 |
| UI/CSS・公開メタデータ | 省略 | 対象なし | 全 docs 形式、Python/UI、サイト検査 |
| toolchain・検証 harness・未知のファイル | 全組合せ | 全件、1 回 | 従来の全検査 |
| 手動実行 | 全組合せ | 全件、3 回 | 従来の全検査 |

文書例が変更された場合は必要なコンパイルを残す。UI だけの変更では既知の重い C++ 統合テストを省くが、
Python の測定ロジック・対象選択・UI のテストは残す。新しい未知のテストファイルの変更は全件扱いになる。
ACL は workflow 内で commit を固定する。依存の revision を変えると workflow 変更として全件検証になる。

既存の required check 名 `compile (GCC, gnu++20)` などと `verify` は維持する。
ジョブ全体を対象外にせず、不要なステップだけを省く。
plan が失敗・キャンセルされた場合は既存 compile / verify ジョブ内の先頭 guard が失敗するため、
required job の単なる skipped を利用して通過しない。
Pages の公開条件、事前 Jekyll 検査、公開前のサイト検査は従来どおり。

## ベンチマーク基準

main 上で成功した全件・3 回測定だけを保存基準にする（通常は main の手動実行）。
部分検証・空選択・1 回測定で全件の基準を上書きしない。
結果がない target に 0 秒を埋めたり、古い計測を新しい revision として合算したりしない。

## ローカルで選択を確認

次のコマンドは manifest を生成するだけで、Library Checker は実行しない。
明示的 `--base` は開発者のローカル検討用で、Actions API の成功履歴検索を行わない。

```sh
python3 scripts/plan_verification.py --base <base-commit> --head HEAD
python3 scripts/verify_with_metrics.py --plan .verification/ci-plan.json --repeats 1 --timeout 3600
```

テストは `tests/test_ci_plan.py`。推移的依存、独立 target、削除・rename、空選択、未知の変更、
解析失敗、manifest 改変、失敗済み前回 push、基準 API エラーを一時 Git リポジトリで検証する。

GitHub のイベント SHA と依存ジョブの挙動は公式の
[イベント仕様](https://docs.github.com/en/actions/reference/workflows-and-actions/events-that-trigger-workflows) と
[workflow 構文](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax) に基づく。
