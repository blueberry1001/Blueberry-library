# Library Checker fastest: `partition_function`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [207699](https://judge.yosupo.jp/submission/207699) | Yuezheng_Ling_fans | cpp | 23.0 ms | 23.0 ms | 9404416 B |
| 2 | [406026](https://judge.yosupo.jp/submission/406026) | Rohan_Kapri | cpp | 25.0 ms | 25.0 ms | 8691712 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 207699 | 718 | 24027 | no | yes | yes | no | yes |
| 406026 | 718 | 24027 | no | yes | yes | no | yes |
