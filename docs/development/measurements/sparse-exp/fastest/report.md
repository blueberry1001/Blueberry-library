# Library Checker fastest: `exp_of_formal_power_series_sparse`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409367](https://judge.yosupo.jp/submission/409367) | Aiyiyi | cpp | 7.0 ms | 7.0 ms | 14962688 B |
| 2 | [409632](https://judge.yosupo.jp/submission/409632) | Aiyiyi | cpp | 10.0 ms | 10.0 ms | 14721024 B |
| 3 | [409631](https://judge.yosupo.jp/submission/409631) | Aiyiyi | cpp | 11.0 ms | 11.0 ms | 14692352 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409367 | 1853 | 90176 | no | yes | no | no | yes |
| 409632 | 2056 | 102055 | no | yes | no | no | yes |
| 409631 | 2056 | 102055 | no | yes | no | no | yes |
