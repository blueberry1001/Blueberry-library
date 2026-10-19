---
title: さらに高速なライブラリの実装・検証記録
---

# さらに高速なライブラリの実装・検証記録

2026年10月19日。既存のライブラリを維持したまま、
`blueberry/extra-fast/segment-tree.hpp` に整数和専用の16分木を追加した。
通常の一覧・検索・カテゴリ・操作案内・LC対応表には混ぜず、右上の「資料」から
「さらに高速なライブラリ」へ進む導線にした。`blueberry/all.hpp` は変更していない。

## 設計と採否

- [Fastest調査](extra-fast-research.md)で4問題180提出を確認し、木本体のSIMDとI/Oの手書きasmを区別した。
- `uint64_t` の点加算・点代入・区間和を、2⁶⁴を法とする演算として実装した。
  AVX2は実行時検出し、通常のループへの切り替えも用意した。
- 手書きasmにintrinsics以上の優位は見られず、intrinsicsを既定にした。
  `BLUEBERRY_EXTRA_FAST_USE_ASM` で `vpaddq` を明示した実験版を選べる。
- 更新するvectorの一部を省く候補は、同じコードからループ開始位置だけを変えた対照実験で遅くなり、採用しなかった。
- 構築時のprefixをSIMD化する候補も比較した。prefix部分の短縮は小さく、総構築時間への寄与に見合わないため採用しなかった。
- [測定コード・生ログ・集計・環境](measurements/extra-fast/README.md)には、失敗した起動、初回、候補、最終版を区別して保存する。
  異なるCPU affinityやソースhashの測定を同じ比較として合算しない。

## ローカル検証

Ubuntu / WSL2、Intel Core i7-14650HX、GCC 13.3.0 / Clang 18.1.3を使用した。

| 確認 | 結果 |
| --- | --- |
| `make check COMPILE_JOBS=8`（`make test`を含む） | 成功。Python 149件、文書の実行例113件、header 112件、verify compile 149件、random 108本×20 seed |
| 新APIのGCC / Clang × GNU C++20 / 23 × default / scalar / asm | 12構成で境界・ランダム比較成功 |
| ASan / UBSan | 新APIの境界・ランダム比較成功 |
| GNU inline asmのIntel記法 `-masm=intel` | 境界・ランダム比較成功 |
| 新APIの公式Point Add Range Sum | 全21ケース×3回成功 |
| 全件 `make verify VERIFY_REPEATS=1 VERIFY_TIMEOUT=120` | 時間予算で未完了。4件成功、145件未実行、失敗判定0件 |

全件verifyが未実行の対象を成功とは扱わない。新APIの公式検証は全件verifyとは別の出力先で実行した。
全体確認の後に更新省略候補を固定4vectorへ戻し、最終版も新APIのコンパイラ構成と公式ケースで再確認する。
公開前の文書生成・サイト・実ブラウザ・CIの結果は、完了後に追記する。

## 環境の問題と再実行

最初の `make check` は、WSL用の `GIT_DIR` / `GIT_WORK_TREE` が一時Gitリポジトリを作るテストにも
継承されて干渉したため停止した。テストが変更した設定と専用ブランチのfixtureだけを戻し、
作業ファイルを保持したまま、環境変数を使わず再実行して成功した。
Windows / WSL双方でGitを使えるよう、作業用worktreeのgitdirには相対パスを使用した。

最初の文書生成では、追加した計測用Pythonが依存解析の対象となってtimeoutした。
既存方針と同様に計測用C++ / Pythonを公開文書のソース解析から除外して再実行する。
元のログは作業ディレクトリ内の `.build/logs/` に保存し、成功結果で上書きしない。
