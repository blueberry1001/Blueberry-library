---
layout: page
title: 使い方・ドキュメントの読み方
---

# 使い方・ドキュメントの読み方

## はじめに

C++20・ヘッダオンリーです。ACLにある標準機能はACLを優先し、
Blueberryではrollbackなどの非標準操作や、ACLにない構造を中心に扱います。
既存の標準機能の実装は互換用として残しています。

トップのカテゴリカードから用途・選び方と個別ライブラリに進めます。
「操作一覧」で呼び出し方・計算量を確認し、操作名を開くと返り値・使用例・注意点を読めます。
キーボードではTabで移動、Enter / Spaceで開閉できます。

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

必要なヘッダだけをincludeします。全体を読み込むなら `blueberry/all.hpp` です。
ACLも使う場合は別途用意し、そのルートも `-I` に追加します。ACL本体は同梱していません。

提出先で外部ヘッダを使えないときは、検証環境の導入後に単一ファイルにします。

{% raw %}
```console
oj-bundle main.cpp -I ./Blueberry-library > submission.cpp
g++ -std=gnu++20 -O2 submission.cpp -o submission
```
{% endraw %}

ACLも展開するときはACLのルートも `-I` で指定します。生成後のコードもコンパイルしてください。

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

## 検証する

リポジトリのルートで、Python 3.9以上とC++20対応GCCを用意します（CIはPython 3.12）。

{% raw %}
```console
python3 -m pip install -r requirements-dev.txt
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
