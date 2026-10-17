# Library Checker fastest: `bernoulli_number`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [17797](https://judge.yosupo.jp/submission/17797) | Nyaan | cpp | 54.0 ms | 54.0 ms | 17674240 B |
| 2 | [65329](https://judge.yosupo.jp/submission/65329) | anonymous | cpp | 85.0 ms | 85.0 ms | 13901824 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 17797 | 1 | 47108 | yes | yes | no | no | yes |
| 65329 | 722 | 25563 | yes | yes | yes | no | no |
