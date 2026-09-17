# Contributor instructions

Read IMPLEMENTATION_POLICY.md before changing a library. Standard ACL features
are ACL-first; prioritize non-ACL functionality. Preserve supported API compatibility.
The user authorized removal of obsolete root headers during the 2026-09-17 migration;
do not restore them. Keep migration guidance in the published migration page.

For every new or changed public API:

- Maintain docs/<category>/<name>.md with documentation_of pointing to the header.
- Use templates/library.md.example: overview/preconditions/memory, a compilable
  minimal C++ main with assertions, and an operation table with call + complexity.
- Document every public constructor, overload, operation and exposed field.
- Provide a <details class="api-operation" id="..." markdown="1"> per operation
  (including <summary>signature — complexity</summary>, example and caveats).
- State bounds, return values, empty cases, type/algebra assumptions, overflow,
  lifetime/invalidation and amortized complexity when relevant.
- Wrap fenced code examples with Liquid raw/endraw tags outside the fences, so
  C++ nested initialization braces are not interpreted as Liquid templates.
- Register the library in .verify-helper/docs/static/_data/libraries.yml.
- Add official verify coverage and boundary/randomized tests as appropriate.
- Run make test, make verify and make docs. Do not claim unrun tests passed.
- Judge performance only under comparable inputs/compiler/environment; preserve
  measurement logs and do not replace failed/missing measurements with zero.

Keep GitHub Pages/verification-helper as the publishing system. Do not restore
the upstream minimal-theme fixed sidebar or its inline theme_fix width rules.
The user authorizes autonomous commits, pushes, PR creation and merges for this
repository. Work on a dedicated branch, review the diff and required checks,
and never bypass branch protection or failing required checks. A later user
instruction restricting these actions takes precedence.

# Blueberry-library — Codex Development Guide

このリポジトリは競技プログラミング用C++ライブラリです。

最重要目標は次の3点です。

1. 競技中に短く使える
2. 十分高速である
3. APIを思い出しやすい

単に一般的なソフトウェアとして綺麗な設計にするのではなく、競技プログラミングでの実用性を優先してください。

---

## 0. 作業開始時

作業を開始したら、コードを書く前に以下を確認してください。

1. `git status`
2. 現在のbranch
3. 最新の `main`
4. リポジトリ構造
5. `README` / `AGENTS.md` / docs
6. 関連する既存実装
7. 関連するverifyコード
8. open PRや他branchとの競合可能性

ユーザーが特定のbranchや担当範囲を指定している場合は、それを最優先してください。

ユーザーから明示的な指示がない限り、`main` に直接commitしないでください。

---

## 1. Git方針

新規作業は原則として専用branchで行います。

例:

```text
work/ds-core
work/tree
work/graph
work/math
work/string
work/verify
work/benchmark
fix/sparse-table
feat/hld
```

作業前に最新の `main` を基準にしてください。

他のWorkやCodexが並列で作業している可能性があります。

そのため、

- 担当外のファイルを不用意に変更しない
- 無関係なformat変更をしない
- repository全体の一括renameを勝手に行わない
- 他branchの変更を上書きしない
- conflict解消のために他担当の実装を独断で削除しない

ことを徹底してください。

未commitのユーザー変更が存在する場合、それを消さないでください。

`git reset --hard`
`git clean -fd`
などの破壊的操作は、明示的な許可なしでは行わないでください。

---

## 2. 並列開発

複数の作業を並列化できる場合は、互いに編集範囲が重ならないように分解してください。

望ましい担当例:

```text
Data Structure
Tree
Graph
Math
String
Benchmark / Performance Research
Verify / CI
Documentation / Audit
```

原則として、同一ソースファイルを複数担当が同時に大規模変更しない構成にしてください。

特に、

- Benchmark担当は原則ライブラリ本体を書き換えない
- Verify担当はアルゴリズム自体を変更しない
- Audit担当は原則コードを変更しない
- Tree担当がSparseTableを利用しても、SparseTable本体の変更はData Structure担当に任せる

など、役割の境界を維持してください。

独立した実装・検証・レビューにはサブエージェントを活用してください。
統合担当は各担当の報告だけで完了を判断せず、差分と検証結果を確認してください。

---

## 3. C++方針

競技プログラミング用コードなので、一般的な業務コードより簡潔さを重視します。

ただし、短さのためにcorrectnessや性能を犠牲にしないでください。

### API

AtCoder Libraryに一般的な命名が存在する場合、それを優先してください。

例:

```cpp
prod()
set()
get()
apply()
max_right()
min_left()
```

`product()` のように長く、ACL等でより一般的な短い名称がある場合は `prod()` を優先します。

APIは、

- 短い
- 意味を思い出しやすい
- 他ライブラリと一貫している

ことを重視してください。

不要に抽象化しないでください。

---

## 4. 実装方針

### 優先順位

実装を評価するときは、おおむね

```text
correctness
↓
計算量
↓
実測性能
↓
使いやすさ
↓
コードの短さ
```

の順に考えてください。

ただし、競プロ用途なので最後の2つも重要です。

### 性能

hot pathでは特に以下を確認してください。

- 不要なallocation
- vectorの不要な再確保
- 不要なcopy
- 不要なpair / tuple生成
- `std::function`
- virtual dispatch
- 不要なbranch
- 再帰オーバーヘッド
- cache locality
- O(N log N) と O(N) の差
- 定数倍

「理論計算量が同じだから十分」と判断せず、Library Checker等で差が大きい場合は実装上の原因も調査してください。

ただし、可読性を大きく落として数%だけ速くするような変更は慎重に判断してください。

---

## 5. ACLとの関係

AtCoder Libraryに既に優秀な実装があり、競技中にもACLを利用できるものについては、無理に自作しない選択肢も検討してください。

一方、

- APIを拡張したい
- ACLに存在しない
- 学習・検証上意味がある
- 本ライブラリ独自の使いやすさがある

場合は自作して構いません。

トップページなどの「Library Files」の一覧には、外部ライブラリであるACLを自作ライブラリとして含めないでください。

---

## 6. Tree

Tree系では、用途が重複する実装を無意味に統合しないでください。

例えば、

- Doubling LCA
- O(1) LCA
- HLD

は用途や定数倍が異なるため、共存して構いません。

検討対象:

```text
LCA
k-th ancestor
distance
Euler Tour
HLD
subtree
path
vertex query
edge query
```

HLDでは競技中に短く使えるAPIを重視してください。

---

## 7. Verify

新規実装や重要な変更には可能な限りverifyを追加してください。

優先順位:

1. Library Checker
2. random test
3. brute-force comparison
4. compile test

Library Checkerに対応問題が存在する場合は積極的に利用してください。

verifyコードも人間が読みやすい状態にしてください。

方針:

```cpp
using namespace std;
```

を使用して構いません。

verifyコード内で一般的なSegTree等が必要で、ACLで簡潔に書ける場合は、同等機能をverifyコード内に再実装せずACLを利用してください。

verifyコードには、何を検証しているか分かる程度の簡潔なコメントを付けてください。

---

## 8. Random Test

アルゴリズム的に可能なら、小さい入力についてbrute forceとの比較を行ってください。

例:

```text
LCA
HLD
DSU
SparseTable
SegTree
Graph shortest path
String algorithms
```

ランダムテストは固定seedまたは失敗ケースを再現できる仕組みにしてください。

失敗した場合には、最小限でも

```text
seed
input
expected
actual
```

が再現できる状態が望ましいです。

本番ライブラリにデバッグ出力を残さないでください。

---

## 9. Benchmark

性能改善では、可能な限り変更前後を測定してください。

1回だけの実行時間で判断せず、複数回実行してください。

可能なら以下を記録してください。

```text
compiler
compiler version
flags
input size
runs
median
min
```

典型的には、

```text
-O2
-O3
-march=native
```

などの差も必要に応じて確認してください。

Library Checkerの高速提出を参考にする場合、

- アルゴリズム
- memory layout
- 定数倍削減
- I/O
- compiler依存
- 実装テクニック

のどこに差があるか分析してください。

他人のコードをそのままコピーするのではなく、考え方を分析して本ライブラリ向けに実装してください。

---

## 10. include

各headerは可能な限り単独でincludeしてコンパイルできる状態を維持してください。

暗黙のinclude依存を作らないでください。

例:

```cpp
#include "library/foo.hpp"
```

だけで必要な型や標準ライブラリが利用可能であることを確認してください。

他のheaderが偶然includeしている標準headerに依存しないでください。

---

## 11. Compiler

少なくとも主要な変更について、

```text
g++
clang++
```

の両方を意識してください。

GNU拡張を使用する場合は、それが意図的か確認してください。

現在リポジトリが採用しているC++標準を勝手に変更しないでください。

---

## 12. Documentation

新規・変更APIは冒頭の必須ドキュメント形式に従い、各ライブラリページに以下を記載してください。

```text
概要
使い方
public API
計算量
verify
```

verifyコード全文をトップページへ大量に掲載しないでください。

verifyコードや対応問題への情報は、関連するライブラリページに配置してください。

トップページはライブラリ一覧として読みやすい状態を維持してください。

---

## 13. 既存コードの変更

既存コードを改善するとき、

「綺麗だから」
「一般的な設計だから」

だけを理由に大規模な書き換えをしないでください。

変更には少なくとも次のいずれかの理由が必要です。

```text
bug fix
performance
API improvement
code size
maintainability
verification
compiler compatibility
```

特に既にverify済みの高速な実装を、抽象化だけを目的に遅くしないでください。

---

## 14. 新規ライブラリ

新規ライブラリを追加する前に、

1. 既に同等実装が存在しないか
2. ACLに存在しないか
3. 使用頻度は十分か
4. Library Checkerでverify可能か
5. 他のライブラリとのAPI一貫性

を確認してください。

「実装できるから」という理由だけで追加しないでください。

---

## 15. 調査

必要に応じて著名な競プロライブラリを参考にして構いません。

例:

```text
AtCoder Library
Nyaan
ei1333
suisen
hitonanode
maspypy
```

ただし、コードのコピーを目的にするのではなく、

- API
- アルゴリズム
- 実装上の工夫
- 計算量
- 定数倍

を比較するために利用してください。

---

## 16. 作業中に問題を見つけた場合

担当外の問題を発見しても、緊急性がなければその場で大量に修正しないでください。

代わりに、

```text
問題
影響
該当ファイル
推奨修正
優先度
```

を残してください。

明らかなコンパイルエラーなど、現在の作業を妨げるものだけ最小限修正して構いません。

---

## 17. 作業終了時

作業を終了する前に可能な範囲で以下を実行してください。

```text
format / lint（存在する場合）
compile test
existing tests
random tests
Library Checker verify
git diff
git status
```

その上で変更内容を確認してください。

不要な

```text
debug print
temporary file
benchmark binary
core dump
generated garbage
```

をcommitしないでください。

---

## 18. Commit

commitは意味のある単位にしてください。

例:

```text
feat(tree): add heavy-light decomposition
perf(ds): reduce sparse table query overhead
test(tree): add random tests for lca
docs(hld): document path query API
```

巨大な `update` commit 1個にまとめるより、レビュー可能な単位を優先してください。

ただし細かすぎるcommit乱造も避けてください。

---

## 19. Push / PR

このリポジトリでは、ユーザーからcommit・push・PRのmergeを自律的に行う許可を得ています。
通常の改善は専用branchで実装し、差分レビューと必要な検証を済ませて、push・PR作成・mergeまで進めてください。
同じ操作について毎回確認を求める必要はありません。以後ユーザーが制限を指定した場合は、その指示を優先してください。

required checksの失敗やbranch protectionを回避してmergeしないでください。
mainへの直接commit、force push、他担当の変更の破棄は、この許可に含みません。

PR本文には最低限、

```text
概要
主な変更
API変更
計算量
性能への影響
verify / test結果
互換性への影響
懸念点
```

を記載してください。

---

## 20. 最終報告

作業後は簡潔に以下を報告してください。

```text
実装した内容
主な変更ファイル
テスト結果
benchmark結果（あれば）
branch
commit
PR
残課題
```

コマンドログを大量に貼る必要はありません。

---

## 21. 現行の検証・公開手順

- 最初のローカル確認には `make check` を使います。unit test、docsのC++例、
  headerの単独・重複include、verifyコードのcompile、random testを実行します。
- 主要なC++変更はGCC / Clang、`gnu++20` / `gnu++23` で確認します。
  release時のheader確認には `CXXFLAGS=-DNDEBUG make include-test` を使い、
  ACL由来の警告を分離するときは `CPPFLAGS="-isystem <ACLのパス>"` を指定します。
- Library Checkerの実行は `make verify`、繰り返し回数は `VERIFY_REPEATS`、
  全体の時間予算は `VERIFY_TIMEOUT` で指定します。既定値はMakefileを参照してください。
  timeoutで未実行になったtargetを成功扱いにせず、再開結果と元の失敗ログを両方残してください。
  異なるrevision・compiler・入力の結果を同一測定としてまとめないでください。
- 公開前には `make docs`、Jekyll build、`python3 scripts/check_site.py` を実行します。
  公開完了はmainのGitHub Actionsと実際のGitHub Pagesの両方で確認してください。
- catalogの登録元は `.verify-helper/docs/static/_data/libraries.yml` です。
  検索UIやcopyボタンの変更では `tests/docs_ui_test.cjs` と実ブラウザで確認し、
  mobile表示、キーボード操作、JavaScript無効時の一覧を維持してください。
- catalog登録の `blueberry/<category>/*.hpp` と入口 `blueberry/all.hpp` を検証対象として扱います。
  旧ルート直下ヘッダ7個は削除済みです。パス・APIの変更は公開 `migration.html` の
  元ファイル `.verify-helper/docs/static/migration.md` に記載し、旧入口を復活させないでください。

具体的な実行例と検証・測定結果はREADME、docs/development以下のレポートを参照してください。
文書のみの変更では影響範囲に応じた確認を行い、実行していない検証を実行済みと書かないでください。

---

# Autonomous workflow

ユーザーから大きな依頼を受けた場合、以下の順番で自律的に進めてください。

```text
Inspect
↓
Scope
↓
Implement
↓
Compile
↓
Test
↓
Benchmark if relevant
↓
Review diff
↓
Commit
↓
Push / PR / Merge after review and required checks
↓
Report
```

途中で細かい選択肢が発生しても、既存コードや本指示から合理的に判断できる場合は作業を止めず進めてください。

ユーザー確認が必要なのは主に、

- 大規模なAPI破壊
- 大量削除
- repository構造の根本変更
- 複数の設計案があり今後の互換性へ大きく影響する

場合です。

---

# Core principle

このライブラリでは、

> 短く打てて、高速で、正しく、思い出しやすい

ことを最優先してください。

一般的な「綺麗なライブラリ設計」よりも、実際の競技中に使いやすいかどうかを基準に判断してください。
