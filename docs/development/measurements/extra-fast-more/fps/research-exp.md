# Library Checker fastest: `exp_of_formal_power_series`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409662](https://judge.yosupo.jp/submission/409662) | Aiyiyi | cpp | 16.0 ms | 16.0 ms | 15740928 B |
| 2 | [409661](https://judge.yosupo.jp/submission/409661) | Aiyiyi | cpp | 16.0 ms | 16.0 ms | 16404480 B |
| 3 | [409402](https://judge.yosupo.jp/submission/409402) | Aiyiyi | cpp | 16.0 ms | 16.0 ms | 17342464 B |
| 4 | [409356](https://judge.yosupo.jp/submission/409356) | Aiyiyi | cpp | 18.0 ms | 18.0 ms | 18137088 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409662 | 6942 | 324009 | yes | yes | no | no | yes |
| 409661 | 6900 | 321354 | yes | yes | no | no | yes |
| 409402 | 6226 | 289281 | yes | yes | no | no | yes |
| 409356 | 4189 | 194749 | yes | yes | no | no | yes |
