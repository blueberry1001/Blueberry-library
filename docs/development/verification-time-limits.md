# 公式問題の制限時間とローカル検証

`make verify` は正答性とローカル実行時間を記録します。各ケースの既定タイムアウトは60秒で、Library Checkerの問題ごとの制限時間ではありません。`passed` と表示されても、提出先で制限時間内に通ることは確認できません。ローカル環境とjudgeではCPU、コンパイラ、I/Oなどの条件も異なります。

公式問題の制限時間に対して、記録済みの最大ケース時間を一括点検できます。先に [library-checker-problems](https://github.com/yosupo06/library-checker-problems) をローカルに用意し、その参照先を指定します。

```sh
python3 scripts/audit_verify_limits.py \
  --report .verification/current.json \
  --problem-repo /path/to/library-checker-problems \
  --output .verification/limit-screening
```

`screening.md` は記録の最大ケース時間の中央値を各問題の `info.toml` にある制限時間で割り、比が1以上、0.5以上の順に調査対象として並べます。0.5は調査の優先順位を決める目安であり、合否基準ではありません。未実行・失敗・時間不明・問題の制限時間不明も明示します。verifyドライバが記録時から変更されているかハッシュがない場合は、別の問題へ誤って対応づけないため比を出しません。

`screening.json` には記録時のrevisionと環境、問題リポジトリのrevisionと変更状態、各 `info.toml` のパスとハッシュを保存します。記録時のrevisionと作業中のrevisionが異なる場合や作業ツリーに変更がある場合、現在のライブラリの性能とはみなさず、対象を再計測してください。問題データ自体が変更された可能性にも注意してください。

比率が高い対象は、同じ入力・コンパイラ・環境で複数回測り、処理時間、入力と出力、メモリを分けて調査します。judge環境での結果が必要なら実際の提出で確認してください。ローカル測定だけからACやTLEを断定しません。

CIでは検証結果のartifactを受け取る別jobで、制限時間の半分以上かかった対象の上位2件を、それぞれ3回再検証します。再検証の環境と入力ハッシュは別のartifactへ保存し、初回との条件の違いも表示します。別runnerで実行されるため、初回と再検証の時間比は算出しません。再検証でWAやREを確認した場合は、この別jobを失敗として表示します。
