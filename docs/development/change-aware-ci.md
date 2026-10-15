# 変更範囲に応じた CI

通常のPR・main push・手動実行は、変更の影響を受ける、または成功記録がない
Library Checker verifierだけを1回実行する。文書/UIだけの変更なら公式verifyは0件。
手動実行で `full` を有効にした場合だけ、全件を3回測定する。
ローカル `make verify` は全件・既定3回の測定コマンドとして残す。

## 成功記録と選択

`scripts/verification_state.py` は、verifier本体と推移的なローカルincludeのGit blob、
検証設定、依存ツールのrequirements、ACLの固定commitから内容の指紋を作る。
実行器のコンパイル・公式checker実行・結果判定部分も構文木で照合し、処理の変更を検出する。
レポート整形・公開処理・コメントだけの変更では、この実行条件を変えない。
保存済みACと一致するものを再利用し、不一致・新規・記録欠損の対象を検証する。
依存解析ができない対象のACは再利用しない。変更が戻った場合も内容で照合する。
実行済みの失敗記録は古いACより優先し、直前のCIの失敗や中断で未検証の変更を隠さない。

成功記録には、実際に検証したrevision・記録時刻・ActionsのURL・入力データのハッシュ・
コンパイラと測定条件を残す。未実行の対象へ現在のrevisionや測定時間を付け直さない。
通常の1回測定を、3回のベンチマーク中央値と同条件の改善値として扱わない。
公式ケースの更新も含めて再検証したい場合は、明示的な全件実行を使う。

最初の記録 `.verify-helper/verified-state.seed.json` は、成功したPR #40の
[run 38667911374](https://github.com/blueberry1001/Blueberry-library/actions/runs/38667911374)
の全143件のartifactから取り込んだ。測定元 `221b0cc` とマージ後 `dd47d59` のGit treeが
同一であることを確認している。測定元revisionや入力ハッシュをそのまま保持する。

以後の記録は、小さなJSONとしてActions cache・artifactと公開サイトの
`assets/verification-state.json` に保存する。復元時は新しい記録を優先する。
公開データを取得できない場合も保存済み/初期記録を内容照合し、証拠のない対象だけを実行する。
大きな公式テストケースと実行ファイルは永続Actions cacheへ保存しない。

## 公開と検査

`ci-plan` artifactにはbase/head・変更一覧・全対象・今回の対象・選択理由・復元した記録を残す。
verify jobと実行器は対象選択を再計算し、空配列を全件実行へ読み替えない。
今回の測定は `.verification/current.json`、保存ACは `.verification/state.json` に分ける。
公開ページは内容が一致する保存ACを表示し、verifyページに元のrevision・時刻・ログを示す。
今回の失敗は古いACで隠さず、公開前検査で有効な成功記録が全対象にあることを確認する。

| 変更 | compile/random matrix | Library Checker | その他 |
| --- | --- | --- | --- |
| ライブラリ・verify・C++テスト | GCC/Clang × GNU C++20/23 | 内容が変わった対象、1回 | Python/UI・文書例・サイト |
| 文書/UI・公開処理 | 省略（未検証対象があれば実行） | 有効な保存ACがあれば0件 | Python/UI・変更文書例・サイト |
| 検証設定・requirements・ACL revision | 全組合せ | 記録が無効になった対象、1回 | Python/UI・文書例・サイト |
| 手動 `full=true` | 全組合せ | 全件、3回 | 全検査 |

required check名とPagesの公開条件は維持する。planが失敗・中断した場合、
compile/verifyの先頭guardも失敗し、必須チェックのskipでマージを通さない。
文書の事前Jekyll buildと公開前サイト検査も継続する。
全件・3回の成功だけが性能比較のbaselineを更新し、部分検証や空選択では上書きしない。

## ローカル確認

```sh
python3 scripts/restore_verification_state.py --offline
python3 scripts/plan_verification.py --base <base-commit> --state .verification/previous-state.json
python3 scripts/verify_with_metrics.py --plan .verification/ci-plan.json --state .verification/previous-state.json --repeats 1 --timeout 3600
```

選択・再利用・改変検出・欠損・失敗の優先順位は `tests/test_ci_plan.py` と
`tests/test_verification_state.py`、表示は生成文書と公開データのテストで確認する。
