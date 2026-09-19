# Library Checker fastest: `predecessor_problem`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [403023](https://judge.yosupo.jp/submission/403023) | nandhagk | cpp | 16.0 ms | 16.0 ms | 9191424 B |
| 2 | [278227](https://judge.yosupo.jp/submission/278227) | chaihf | cpp | 20.0 ms | 20.0 ms | 11476992 B |
| 3 | [269275](https://judge.yosupo.jp/submission/269275) | mukundan314 | cpp | 20.0 ms | 20.0 ms | 11051008 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 403023 | 3507 | 150964 | no | yes | no | no | yes |
| 278227 | 415 | 10612 | no | yes | no | no | no |
| 269275 | 242 | 7666 | no | yes | no | no | no |
