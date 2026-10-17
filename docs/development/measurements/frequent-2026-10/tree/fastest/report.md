# Library Checker fastest: `lca`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [278239](https://judge.yosupo.jp/submission/278239) | chaihf | cpp | 24.0 ms | 24.0 ms | 16572416 B |
| 2 | [370969](https://judge.yosupo.jp/submission/370969) | (anonymous) | cpp | 27.0 ms | 27.0 ms | 18579456 B |
| 3 | [370928](https://judge.yosupo.jp/submission/370928) | (anonymous) | cpp | 27.0 ms | 27.0 ms | 18599936 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 278239 | 391 | 9978 | no | yes | no | no | no |
| 370969 | 379 | 8731 | no | yes | no | no | no |
| 370928 | 322 | 8210 | no | yes | no | no | no |
