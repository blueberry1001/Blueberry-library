# Verification timings

Commit: 0f26207aec79a8f832c7ffd77cc4ee4ff605c133 / UTC: 2026-10-20T04:10:10.537811+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/extra-fast/bit-matrix-product.test.cpp / 0 | passed | 7.997867 s | 10.124069 s | 1.910377 s | 初回（基準なし） |
| verify/extra-fast/point-add-range-sum.test.cpp / 0 | passed | 7.745852 s | 4.871159 s | 0.995351 s | 初回（基準なし） |
| verify/extra-fast/static-rmq.test.cpp / 0 | passed | 6.533339 s | 5.551650 s | 0.644214 s | 初回（基準なし） |

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
  "recorder_sha256": "3ee628d2aa014ded1096cfaa43a315a355f3aa75c0fe7cc598efb0184a1c26b6",
  "jobs": 1,
  "tle_seconds": 60
}
~~~
