# 標準stream混在修正の生ログ

- 基準revision: `737dd7ed53365c86d1ddf2cd76fc337495571d02`。
- `results.json`は同一環境・同一入力・同一flagsの測定7回と中央値・最小値。
- `benchmark.py.snapshot`は実行したスクリプトそのもの。HEADが基準revisionのcloneへ候補ヘッダを重ねて実行した。
- `sources.json`は再現コードの前後の出力、測定・検証コピーと作業ソースのhash。ローカルcloneのHEADはソースoverlayを含まないため、基準そのものの成功として扱わない。
- Windows/Linux間の改行差はraw hashとLF正規化hashを区別する。
- `focused-tests.log`は共有bridge段階の8構成・104ケースと8回のflush検証。`fin`追加後の結果は`focused-fin-tests.log`。
- `focused-fin-tests.log`は128ケースと8回のflush検証の成功後、文書例の個別コマンドでACL include指定を忘れた失敗も含む。`docs-rerun.log`は指定を直した文書例の成功結果。
- `sanitizers.cpp.snapshot`と`sanitizers.log`は独立したASan+UBSan確認。131,209byteのbinary本体に長さ・後続tokenを加えた全入力は131,233byte。
- 修正前の混在版は不正解。比較測定として0秒や成功を記録しない。
- `bridge-only/`は`fin`追加前の測定・ソースhashと、未初期化変数の警告で停止した初回`make check`のログ。最終結果と分けて保持する。

説明と結果表は[検証レポート](../../../mixed-contest-io.md)を参照。
