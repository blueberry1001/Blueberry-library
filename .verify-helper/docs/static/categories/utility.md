---
layout: category
title: Utility — 入出力・補助機能
category: utility
---

整数・文字列などを大量に読み書きするときは、FastInput / FastOutputを利用できます。
通常ファイルとpipeの両方に対応し、mmapや特定CPUの命令を必要としません。
同じFILEに対するcin/coutやscanf/printfとの混在は避けます。
対話問題では`FastInput<true>`と明示的な`FastOutput::flush()`を使います。
