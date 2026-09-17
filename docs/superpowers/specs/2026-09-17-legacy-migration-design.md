# 旧ライブラリの検証付き移行

ユーザーの指定に従い、未検証のルート直下実装を新しいカテゴリ別 API に移行して削除する。
旧パスの互換 shim は残さず、移行表で変更を明示する。既に検証済みの旧 API 別名は維持する。

## 対象と設計

- Dynamic Li Chao Tree: 座標を事前登録せず整数半開領域で直線・線分追加と最小値取得。空集合は optional。
  arena と整数子 index で所有権を管理し、符号付き端点の差分 overflow を避ける。
- Dynamic Fenwick Tree / 2D: 未知の更新座標をオンラインで処理。1D は疎な Fenwick、
  2D は疎な外側 Fenwick と内側の平衡二分木で O(Q log N) 空間を目指す。
  座標が分かる場合の高速・省メモリな Offline Fenwick 2D は併存する。
- Implicit Treap: 列の位置で insert/erase/set/get/prod/reverse/apply。
  旧 implicit_treap は実際にはキー順多重集合なので、その機能は別の Ordered Multiset に移す。
  集約順序、lazy 合成、コピー・移動、解放済みノード再利用を独立テストする。
- Fraction: 既約な int64 分子と正の int64 分母を保持する値型。中間演算は signed 128bit、
  最終値の表現可能性とゼロ除算を契約として明示する。比較・四則演算は独立多倍長 oracle で検証する。
- Graph / RollbackUnionFind / fps の旧入口は既存の検証済み機能への移行例を用意する。
  Graph 全 API の互換性は主張しない。

## 担当と共有ファイル

Li Chao、Fenwick、Treap はそれぞれ別エージェントが実装・文書・個別検証・性能調査を担当。
親が Fraction、カタログ、all.hpp、既存文書の参照、移行ガイド、旧ファイル削除、全体検証を担当。
新ヘッダは C++20 / blueberry 名前空間。変更前の旧ファイルは新実装の検証が済むまで残す。

## 完了条件

各公開操作の説明・実行可能例、公式 Library Checker ドライバ、固定 seed の愚直比較、
境界・所有権・sanitizer 検証を追加。Fastest は設計調査として使い、同条件の測定結果だけで性能を比較する。
make test / compile-test / random-test / verify / docs、Jekyll と生成サイト検査を通す。
独立レビュー後に意味のある単位で commit、PR、CI、merge、公開サイト確認まで行う。
