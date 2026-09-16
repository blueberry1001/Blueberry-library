# Verification timings

Commit: 42781adfa194c5851a2a1d1fa3adb642e2db1272 / UTC: 2026-09-16T22:28:19.727821+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/data-structure/ordered-set.test.cpp / 0 | passed | 4.410675 s | 35.502939 s | 2.176451 s | 初回（基準なし） |
| verify/data-structure/persistent-point-set-range-composite.test.cpp / 0 | passed | 2.641165 s | 22.948909 s | 3.508152 s | 初回（基準なし） |
| verify/data-structure/persistent-rectangle-sum.test.cpp / 0 | passed | 3.905104 s | 19.613973 s | 2.573750 s | 初回（基準なし） |
| verify/data-structure/unionfind-with-potential-non-commutative-group.test.cpp / 0 | passed | 4.346157 s | 6.198969 s | 0.669122 s | 初回（基準なし） |
| verify/data-structure/unionfind-with-potential.test.cpp / 0 | passed | 2.315117 s | 3.957325 s | 0.357873 s | 初回（基準なし） |

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
