# Local benchmark results

Generated: 2026-09-13T07:50:24.974331+00:00 / revision: `e9b8851fc4f4c656c069937cbcf22476a3eaee4f`

Times are seconds. Medians and median absolute deviations use interleaved repetitions.
The ratio compares query time with the current Blueberry implementation under identical flags.
It is not a ratio to the Library Checker leaderboard.

| target | flags | build median | query median | query MAD | wall median | structure | ratio |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| static-rmq-random-blueberry | judge | 0.015073 | 0.147649 | 0.007769 | 0.175328 | 37902932 B | 1.000x |
| static-rmq-random-blueberry | judge-o3 | 0.013839 | 0.155942 | 0.017421 | 0.186166 | 37902932 B | 1.000x |
| static-rmq-random-flat | judge | 0.016132 | 0.086165 | 0.009922 | 0.114937 | 38000000 B | 0.584x |
| static-rmq-random-flat | judge-o3 | 0.014290 | 0.084895 | 0.009314 | 0.115665 | 38000000 B | 0.544x |
| static-rmq-random-blocked | judge | 0.005577 | 0.187276 | 0.010107 | 0.204339 | 6406276 B | 1.268x |
| static-rmq-random-blocked | judge-o3 | 0.005235 | 0.190393 | 0.008767 | 0.206050 | 6406276 B | 1.221x |
| static-rmq-small-blueberry | judge | 0.014842 | 0.071276 | 0.006736 | 0.099042 | 37902932 B | 1.000x |
| static-rmq-small-blueberry | judge-o3 | 0.013460 | 0.075351 | 0.006142 | 0.098224 | 37902932 B | 1.000x |
| static-rmq-small-flat | judge | 0.015976 | 0.073431 | 0.003008 | 0.097205 | 38000000 B | 1.030x |
| static-rmq-small-flat | judge-o3 | 0.014471 | 0.090990 | 0.022556 | 0.114944 | 38000000 B | 1.208x |
| static-rmq-small-blocked | judge | 0.005491 | 0.135017 | 0.008968 | 0.147255 | 6406276 B | 1.894x |
| static-rmq-small-blocked | judge-o3 | 0.005294 | 0.136660 | 0.003351 | 0.150395 | 6406276 B | 1.814x |

## Environment

```json
{
  "cpu": "AMD EPYC 9V74 80-Core Processor",
  "os": "Linux-6.18.35-x86_64-with-glibc2.39",
  "python": "3.12.14",
  "compiler": "g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0",
  "cpu_affinity": 0,
  "repeats": 11,
  "warmups": 2,
  "interleaved": true
}
```
