---
layout: page
title: 使い方
---

# 使い方

C++20・ヘッダオンリーです。必要なヘッダをincludeし、リポジトリのルートをインクルードパスに追加します。
ACLの標準機能はACLを優先し、Blueberryはrollbackなどの拡張やACLにない機能を中心に扱います。

## 導入・コンパイル

{% raw %}
```console
git clone https://github.com/blueberry1001/Blueberry-library.git
g++ -std=gnu++20 -O2 -Wall -Wextra -I ./Blueberry-library main.cpp -o main
```
{% endraw %}

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/rollback-union-find.hpp"

int main() {
  blueberry::RollbackUnionFind uf(3);
  int saved = uf.state();
  uf.merge(0, 1);
  assert(uf.same(0, 1));
  uf.rollback(saved);
  assert(!uf.same(0, 1));
}
```
{% endraw %}

全体を読み込む `blueberry/all.hpp` や、FPSなどACLに依存するヘッダには、別途ACLが必要です。
ACLのルートも `-I` に追加してください。ACL本体は同梱していません。

提出先で外部ヘッダを使えないときは、検証環境の導入後に単一ファイルにします。

{% raw %}
```console
oj-bundle main.cpp -I ./Blueberry-library > submission.cpp
g++ -std=gnu++20 -O2 submission.cpp -o submission
```
{% endraw %}

ACLも展開するときはACLのルートも `-I` で指定します。生成後のコードもコンパイルしてください。

## 必要な機能を探す

| 探し方 | 入口 |
| --- | --- |
| 名前・用途が分かる | [ライブラリ一覧]({{ '/' | relative_url }})で検索 |
| 更新・問い合わせから選びたい | [操作から探す]({{ '/operations.html' | relative_url }}) |
| ACL用の演算を組みたい | [モノイド集]({{ '/monoids.html' | relative_url }}) |
| 問題に対応する実装を知りたい | [LC対応表]({{ '/library-checker.html' | relative_url }}) |

個別ページの「使用例」で最小の使い方を確認できます。操作名を開くと、引数・返り値・計算量・注意点が読めます。
旧ヘッダからの置き換えは[移行ガイド]({{ '/migration.html' | relative_url }})を参照してください。

<details class="category-guide" markdown="1">
<summary>検索とキーボード操作</summary>

検索は日本語・英語の別名にも対応します。空白で区切ると全語を含む項目を表示し、`/`で検索欄に移動します。
ライブラリ一覧とLC対応表は、絞り込み後のURLを共有できます。
「操作から探す」では、更新なしの条件にも動的構造を含みます。候補順は速度順ではありません。
全クエリを先に集められる場合も、逐次処理できる構造を候補に含みます。
Tabで移動、Enter / Spaceで詳細を開閉できます。
コピーが許可されない場合は、選択されたコードをCtrl+C / ⌘Cでコピーしてください。

</details>

## 共通の約束

| 項目 | 約束 |
| --- | --- |
| 名前空間 | `blueberry`。ACLは `atcoder` |
| 添字・頂点 | 0-indexed |
| 区間 | 原則として半開区間 `[l, r)` |
| 計算量 | N, V, Eなどの意味は各ページの概要に記載 |
| 型の演算 | 特記がなければ比較・加算・結合演算を O(1) と仮定 |
| 償却計算量 | 1回の最悪値でなく、操作列全体で平均した上界 |
| assert | 前提違反を検出する補助。不正な入力を許す仕組みではない |

空区間・空配列が使えるかは各操作の説明を確認してください。
`long long` でもオーバーフローする入力は扱えません。
参照を返すAPIは寿命と無効化条件を確認します。
DSUの経路圧縮のように、問い合わせでも内部状態が変わる場合があります。

<details class="category-guide" markdown="1">
<summary>開発・検証する</summary>

リポジトリのルートで、Python 3.9以上とC++20対応GCCを用意します（CIはPython 3.12）。

{% raw %}
```console
make setup
make test
make verify
make docs
```
{% endraw %}

- `make test`: 計測の単体テストと、最小使用例のコンパイル・実行。
- `make verify`: 公式テストとcheckerで検証。各verifyの実行時間と基準との差を記録。
- `make docs`: verification-helperのMarkdown生成と計測結果の取り込み。
  HTMLはGitHub ActionsでJekyllビルドを確認します。
- `make benchmark-baseline`: 成功した計測結果をローカルの比較基準として保存・置換。

[実行時間の読み方]({{ '/benchmarks.html' | relative_url }})も確認してください。
`oj-verify run` の直接実行は上流コマンドのままで、Blueberryの計測記録は作りません。
普段の入口は `make verify` に統一します。

Yosupoへの自動提出ではなく、公式データとcheckerによるローカル検証です。
提出記録が必要な場合は、生成したコードをLibrary Checkerの提出画面から提出してください。

## ライブラリを追加・変更する

1. [実装方針]({{ site.github.repository_url }}/blob/main/IMPLEMENTATION_POLICY.md)でACLとの役割分担を確認。
2. ヘッダと対応する `verify/**/*.test.cpp` を追加。
3. `templates/library.md.example` をもとに `docs/<category>/<name>.md` を作成。
4. **すべての公開操作**の呼び出し方・計算量・返り値・例・注意点を記載。
5. `.verify-helper/docs/static/_data/libraries.yml` に登録。
6. `make test`・`make verify`・`make docs` と、PRのCIでHTMLビルドまで確認。
7. mainへのマージ後に検証・GitHub Pages公開が自動実行されます。

操作別の仕様整理は[Nyaan's Library](https://nyaannyaan.github.io/library/)を参考にしています。
説明・使用例はこのリポジトリの実装に合わせたものです。

</details>
