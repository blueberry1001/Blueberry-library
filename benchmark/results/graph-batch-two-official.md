# Verification timings

Commit: 2207ff8602a23cfbe682d02af388e652dd739e2f / UTC: 2026-09-17T22:50:30.130644+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/graph/cycle-detection-directed.test.cpp / 0 | passed | 2.260947 s | 11.183301 s | 1.516619 s | 初回（基準なし） |
| verify/graph/cycle-detection-undirected.test.cpp / 0 | passed | 3.535620 s | 8.041013 s | 1.392469 s | 初回（基準なし） |
| verify/graph/tree-diameter.test.cpp / 0 | passed | 2.540251 s | 8.706479 s | 1.379215 s | 初回（基準なし） |
| verify/graph/triangle-enumeration.test.cpp / 0 | passed | 2.691565 s | 1.806198 s | 0.322008 s | 初回（基準なし） |

## Environment

~~~json
{
  "cpu": "Intel(R) Core(TM) i7-14650HX",
  "os": "Linux-5.15.167.4-microsoft-standard-WSL2-x86_64-with-glibc2.39",
  "python": "3.12.3",
  "tools": {
    "online-judge-verify-helper": "5.6.0",
    "online-judge-tools": "11.5.1",
    "online-judge-api-client": "10.10.1"
  },
  "method": "oj-elapsed-log-v1",
  "recorder_sha256": "bd41d8e0b425cb1616cd57fdac85c06e2238986eafcfd961d0fc89a6175d1953",
  "jobs": 1,
  "tle_seconds": 60
}
~~~
