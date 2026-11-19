# Library Checker fastest: `persistent_unionfind`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409831](https://judge.yosupo.jp/submission/409831) | (anonymous) | cpp | 15.0 ms | 15.0 ms | 6852608 B |
| 2 | [404578](https://judge.yosupo.jp/submission/404578) | Rohan_Kapri | cpp | 18.0 ms | 18.0 ms | 15785984 B |
| 3 | [218108](https://judge.yosupo.jp/submission/218108) | sortA0329 | cpp | 18.0 ms | 18.0 ms | 16453632 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409831 | 188 | 4781 | no | no | no | no | yes |
| 404578 | 2519 | 104698 | no | yes | no | no | yes |
| 218108 | 2519 | 104698 | no | yes | no | no | yes |
