# Library Checker fastest: `shortest_path`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [415573](https://judge.yosupo.jp/submission/415573) | (anonymous) | cpp | 28.0 ms | 28.0 ms | 48242688 B |
| 2 | [404592](https://judge.yosupo.jp/submission/404592) | Rohan_Kapri | cpp | 40.0 ms | 40.0 ms | 20021248 B |
| 3 | [403027](https://judge.yosupo.jp/submission/403027) | nandhagk | cpp | 40.0 ms | 40.0 ms | 19984384 B |
| 4 | [412926](https://judge.yosupo.jp/submission/412926) | sounansya | cpp | 43.0 ms | 43.0 ms | 19107840 B |
| 5 | [410532](https://judge.yosupo.jp/submission/410532) | twins_fuyu | cpp | 44.0 ms | 44.0 ms | 30162944 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 415573 | 264 | 8671 | no | yes | no | no | yes |
| 404592 | 3295 | 141948 | no | yes | no | no | yes |
| 403027 | 3312 | 139795 | no | yes | no | no | yes |
| 412926 | 1253 | 44221 | no | yes | no | no | yes |
| 410532 | 294 | 7953 | no | no | no | no | no |
