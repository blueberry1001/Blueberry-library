# Library Checker fastest: `log_of_formal_power_series`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409452](https://judge.yosupo.jp/submission/409452) | Aiyiyi | cpp | 13.0 ms | 13.0 ms | 15474688 B |
| 2 | [409363](https://judge.yosupo.jp/submission/409363) | Aiyiyi | cpp | 14.0 ms | 14.0 ms | 18100224 B |
| 3 | [409264](https://judge.yosupo.jp/submission/409264) | Aiyiyi | cpp | 15.0 ms | 15.0 ms | 18206720 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409452 | 6291 | 292270 | yes | yes | no | no | yes |
| 409363 | 4209 | 195886 | yes | yes | no | no | yes |
| 409264 | 4114 | 190772 | yes | yes | no | no | yes |
