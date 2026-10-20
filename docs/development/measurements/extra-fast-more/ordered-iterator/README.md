# OrderedMap / OrderedSet の反復子検証

これは正しさの確認ログです。`results.json` の経過時間はコンパイル・テストの実行時間であり、性能比較には使いません。最終検証は root の全性能測定が終了してから開始し、他の正しさ検証と並行して実行しました。

## 先行確認

`smoke/` は反復子の初版を確認した記録です。最終公開版とは分けて保存しています。

- GCC / Clang、GNU++20、`-O2 -Wall -Wextra -Wshadow -Werror` で、専用反復子テストと既存 associative テストをそれぞれコンパイル・実行。
- GCC の ASan / UBSan で専用反復子テストをコンパイル・実行。
- 合計 10 コマンド、失敗 0 件。seed は `20261020`。
- ヘッダ SHA-256: `a58c367cf6a96ddfb3e57cb4b4fa7f5a0ca6dca2d69a242995831fc5e9e9a418`。

## 最終確認

`final/` の対象は、compact key 配列・葉リンクによる bounds・optional の in-place 構築・`erase(pos)` の初期化済み次キー退避を含む最終ヘッダです。

- ヘッダ SHA-256: `30a686b7a5185aef2052e94608d802dada79fc7dab9f1234bc84ff37758bc0cc`。
- GCC / Clang × GNU++20 / GNU++23 × intrinsics / asm ATT / asm Intel / FORCE_SCALAR の 16 構成。`-O2 -DNDEBUG -Wall -Wextra -Wshadow -Werror`。
- intrinsics は `20261020` と `20261021` の 2 seed、それ以外は `20261020`。
- 両コンパイラの GNU++20、`-O1 -g`、ASan / UBSan も確認。
- コンパイラ情報 2 件、コンパイル 18 件、実行 22 件の合計 42 コマンド、失敗 0 件。
- テスト内では 32/64-bit の signed/unsigned key と scalar/SIMD 選択を確認。FORCE_SCALAR 構成では実行時の SIMD 使用を無効にする。

専用テストでは bidirectional iterator concept、実 lvalue 参照、const 変換、前後の移動、逆順走査、`--end()`、`find()`、`erase(pos)` の後続返却、範囲 for による値更新、葉の split/merge/recycle、root collapse、copy/move/swap、空集合と再利用を確認します。

各フォルダの `metadata.json` に対象ソースの SHA-256、`results.json` にコマンド・終了値・ログの SHA-256 を記録しています。ソースと runner の snapshot も保存し、検証の前後で対象ソースが変わっていないことを runner で確認しました。`completion.json` には完了件数とログ digest の照合結果を保存しています。
