# Library Checker fastest: `point_add_rectangle_sum`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [412387](https://judge.yosupo.jp/submission/412387) | ArcAki | rust | 93.0 ms | 93.0 ms | 17281024 B |
| 2 | [401818](https://judge.yosupo.jp/submission/401818) | toomer | rust | 115.0 ms | 115.0 ms | 34455552 B |
| 3 | [206958](https://judge.yosupo.jp/submission/206958) | oldyan | cpp | 151.0 ms | 151.0 ms | 35921920 B |
| 4 | [404520](https://judge.yosupo.jp/submission/404520) | Rohan_Kapri | cpp | 155.0 ms | 155.0 ms | 35975168 B |
| 5 | [272598](https://judge.yosupo.jp/submission/272598) | Richard1211 | cpp | 168.0 ms | 168.0 ms | 36945920 B |
| 6 | [208767](https://judge.yosupo.jp/submission/208767) | (anonymous) | cpp | 178.0 ms | 178.0 ms | 63451136 B |
| 7 | [222124](https://judge.yosupo.jp/submission/222124) | maspy | cpp | 180.0 ms | 180.0 ms | 47255552 B |
| 8 | [390734](https://judge.yosupo.jp/submission/390734) | edon8618 | cpp | 187.0 ms | 187.0 ms | 38633472 B |
| 9 | [246249](https://judge.yosupo.jp/submission/246249) | bandiaoz | cpp | 187.0 ms | 187.0 ms | 35737600 B |
| 10 | [81486](https://judge.yosupo.jp/submission/81486) | CYMario | cpp17 | 187.0 ms | 187.0 ms | 19443712 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 412387 | 1715 | 52366 | no | yes | no | no | yes |
| 401818 | 105 | 113502 | no | yes | no | no | yes |
| 206958 | 696 | 34186 | no | yes | no | no | yes |
| 404520 | 692 | 34106 | no | yes | no | no | yes |
| 272598 | 1744 | 113908 | no | yes | yes | no | yes |
| 208767 | 808 | 41975 | no | yes | no | no | yes |
| 222124 | 1419 | 40173 | no | yes | yes | no | yes |
| 390734 | 244 | 7113 | no | yes | yes | no | yes |
| 246249 | 696 | 38213 | no | yes | no | no | no |
| 81486 | 309 | 9133 | no | no | no | no | yes |
