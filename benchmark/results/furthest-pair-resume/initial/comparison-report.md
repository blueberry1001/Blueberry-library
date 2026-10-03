# furthest_pair: initial comparison results

This report derives only this experiment's retained raw measurements. It does not declare production adoption or final validation.

Production snapshot SHA256: `37dc37add43d42f2a8710852c0f3cb35e0c737baab00080a4362448b3219df4d`. Source/runner/binary/input hashes and commands are in `prepared.json`; compiler results and gates in `compile.json`/`small-checks.json`.

Timing window: 2026-10-03T08:58:40.548727+00:00 – 2026-10-03T09:00:01.416839+00:00. 1824 kernel and 384 I/O profiles, including 368 retained warmups. Five measured samples per cell. No cross-compiler/mode pooling.

The CPU is shared and unpinned. Median changes below are observations; separated sample ranges are descriptive, not statistical confidence intervals. Negative percentages mean less elapsed time. Full calls include input-copy/allocation/cleanup; stage proxies come from a separate independent caliper decomposition and are not additive production-call timings.

## Full-call comparison range by compiler/mode

| Compiler | Mode | Candidate | Median delta range | Faster cells | Separated faster/slower |
|---|---|---|---:|---:|---:|
| clang++ | release | indices | -28.63% … +50.91% | 6/19 | 2/11 |
| clang++ | release | records | -25.40% … +52.33% | 3/19 | 0/9 |
| clang++ | release | fast-cases | -97.01% … +26.21% | 10/19 | 3/0 |
| clang++ | assert | indices | -22.12% … +52.29% | 4/19 | 1/8 |
| clang++ | assert | records | -23.62% … +38.42% | 3/19 | 1/7 |
| clang++ | assert | fast-cases | -92.21% … +4.65% | 7/19 | 3/0 |
| g++ | release | indices | -5.71% … +55.73% | 3/19 | 0/6 |
| g++ | release | records | +8.11% … +49.61% | 0/19 | 0/6 |
| g++ | release | fast-cases | -97.05% … +12.64% | 9/19 | 3/0 |
| g++ | assert | indices | -11.91% … +48.19% | 4/19 | 1/7 |
| g++ | assert | records | -6.66% … +48.30% | 1/19 | 0/5 |
| g++ | assert | fast-cases | -93.68% … +13.47% | 13/19 | 3/0 |

## Fast-case shortcut: all cells

| Compiler | Mode | Input | N parameter | Before ms [min,max] | Wrapper ms [min,max] | Delta |
|---|---|---|---:|---:|---:|---:|
| clang++ | assert | all-same | 2048 | 0.028450 [0.027920,0.128353] | 0.002216 [0.002159,0.002243] | -92.21% |
| clang++ | assert | all-same | 200000 | 4.593121 [4.103227,8.940475] | 0.368876 [0.251682,0.864507] | -91.97% |
| clang++ | assert | collinear | 2048 | 0.136742 [0.133294,0.139959] | 0.135758 [0.134228,0.137676] | -0.72% |
| clang++ | assert | collinear | 200000 | 20.481035 [19.740541,24.405890] | 21.376515 [19.628114,24.517058] | +4.37% |
| clang++ | assert | duplicates | 2048 | 0.172462 [0.172378,0.188652] | 0.174649 [0.170309,0.377068] | +1.27% |
| clang++ | assert | duplicates | 200000 | 23.034550 [16.201290,39.489671] | 19.862971 [15.708604,21.497879] | -13.77% |
| clang++ | assert | late-different | 2048 | 0.030447 [0.029420,0.033861] | 0.031862 [0.030734,0.034362] | +4.65% |
| clang++ | assert | late-different | 200000 | 5.235917 [4.655631,6.287390] | 5.293741 [4.611323,5.476915] | +1.10% |
| clang++ | assert | parabola | 2048 | 0.174614 [0.172209,0.236478] | 0.179490 [0.174759,0.187291] | +2.79% |
| clang++ | assert | parabola | 200000 | 26.802647 [24.232322,30.484167] | 26.688657 [24.436604,35.063876] | -0.43% |
| clang++ | assert | random | 2048 | 0.190307 [0.184600,0.290225] | 0.186249 [0.183658,0.266558] | -2.13% |
| clang++ | assert | random | 200000 | 25.540631 [25.391638,27.049509] | 26.232821 [25.150107,34.819386] | +2.71% |
| clang++ | assert | reversed | 2048 | 0.095980 [0.095265,0.098361] | 0.098528 [0.092884,0.123632] | +2.65% |
| clang++ | assert | reversed | 200000 | 11.288347 [9.976181,15.306562] | 11.412547 [10.470428,11.576010] | +1.10% |
| clang++ | assert | sorted | 2048 | 0.097517 [0.096704,0.102355] | 0.099096 [0.096962,0.100495] | +1.62% |
| clang++ | assert | sorted | 200000 | 10.957761 [10.065848,11.261773] | 11.396909 [10.341643,19.392892] | +4.01% |
| clang++ | assert | tiny | 200000 | 5.533131 [5.329124,5.679577] | 4.525867 [4.201402,4.804181] | -18.20% |
| clang++ | assert | wide | 2048 | 0.185380 [0.181922,0.204671] | 0.186463 [0.183389,0.281506] | +0.58% |
| clang++ | assert | wide | 200000 | 25.237057 [24.698372,35.464665] | 25.824664 [24.800237,27.500605] | +2.33% |
| clang++ | release | all-same | 2048 | 0.026686 [0.025822,0.027259] | 0.001036 [0.001022,0.001059] | -96.12% |
| clang++ | release | all-same | 200000 | 4.233943 [3.751164,4.813972] | 0.126553 [0.122376,0.195757] | -97.01% |
| clang++ | release | collinear | 2048 | 0.223058 [0.135543,0.380801] | 0.200558 [0.135437,0.251075] | -10.09% |
| clang++ | release | collinear | 200000 | 21.548284 [20.727023,25.908592] | 20.641261 [19.594677,22.496484] | -4.21% |
| clang++ | release | duplicates | 2048 | 0.165727 [0.164949,0.177446] | 0.209163 [0.163859,0.524765] | +26.21% |
| clang++ | release | duplicates | 200000 | 16.049725 [15.066105,17.339595] | 15.702095 [15.171766,18.144196] | -2.17% |
| clang++ | release | late-different | 2048 | 0.029890 [0.029360,0.035284] | 0.031259 [0.030220,0.032255] | +4.58% |
| clang++ | release | late-different | 200000 | 4.592935 [4.213844,5.182025] | 4.903197 [4.490862,5.036816] | +6.76% |
| clang++ | release | parabola | 2048 | 0.199887 [0.176771,0.261290] | 0.175838 [0.172375,0.178569] | -12.03% |
| clang++ | release | parabola | 200000 | 25.276683 [24.998681,26.634036] | 24.645591 [24.418453,30.699470] | -2.50% |
| clang++ | release | random | 2048 | 0.188258 [0.184503,0.196257] | 0.188999 [0.184899,0.199348] | +0.39% |
| clang++ | release | random | 200000 | 25.978001 [25.771136,29.140367] | 26.233947 [24.123954,26.631726] | +0.99% |
| clang++ | release | reversed | 2048 | 0.095143 [0.092707,0.100159] | 0.095813 [0.094046,0.098444] | +0.70% |
| clang++ | release | reversed | 200000 | 10.087885 [9.605061,10.557748] | 10.037683 [9.575189,10.382022] | -0.50% |
| clang++ | release | sorted | 2048 | 0.099315 [0.095730,0.103061] | 0.101582 [0.099686,0.106563] | +2.28% |
| clang++ | release | sorted | 200000 | 10.577149 [9.896315,13.490439] | 11.304456 [10.075378,13.658267] | +6.88% |
| clang++ | release | tiny | 200000 | 5.641449 [5.088878,5.974055] | 3.953604 [3.856756,4.901519] | -29.92% |
| clang++ | release | wide | 2048 | 0.196696 [0.187213,0.210922] | 0.191179 [0.182253,0.280770] | -2.80% |
| clang++ | release | wide | 200000 | 25.734928 [24.526715,30.802099] | 26.524361 [25.085320,27.251914] | +3.07% |
| g++ | assert | all-same | 2048 | 0.026215 [0.025759,0.028959] | 0.002546 [0.002510,0.002687] | -90.29% |
| g++ | assert | all-same | 200000 | 4.447417 [4.107990,4.518656] | 0.280884 [0.262340,0.487043] | -93.68% |
| g++ | assert | collinear | 2048 | 0.144166 [0.143506,0.146412] | 0.143717 [0.140279,0.146550] | -0.31% |
| g++ | assert | collinear | 200000 | 20.852207 [20.241712,24.510869] | 20.939467 [19.859063,24.177710] | +0.42% |
| g++ | assert | duplicates | 2048 | 0.180576 [0.179071,0.350062] | 0.180811 [0.176960,0.198045] | +0.13% |
| g++ | assert | duplicates | 200000 | 20.018339 [18.065934,25.926880] | 18.453481 [17.763625,19.608754] | -7.82% |
| g++ | assert | late-different | 2048 | 0.027322 [0.027090,0.030464] | 0.028983 [0.027638,0.125665] | +6.08% |
| g++ | assert | late-different | 200000 | 4.083224 [4.023302,4.373283] | 4.633076 [4.217926,5.035986] | +13.47% |
| g++ | assert | parabola | 2048 | 0.186525 [0.183970,0.214011] | 0.182047 [0.181244,0.189683] | -2.40% |
| g++ | assert | parabola | 200000 | 25.818205 [25.026425,33.652871] | 26.245286 [25.069713,28.087942] | +1.65% |
| g++ | assert | random | 2048 | 0.193203 [0.187696,0.227943] | 0.188335 [0.187567,0.195878] | -2.52% |
| g++ | assert | random | 200000 | 26.692488 [24.547136,31.829279] | 27.002187 [24.296884,29.462496] | +1.16% |
| g++ | assert | reversed | 2048 | 0.094155 [0.092795,0.169040] | 0.093408 [0.093024,0.124699] | -0.79% |
| g++ | assert | reversed | 200000 | 11.479884 [10.480511,19.020393] | 11.297849 [10.257948,15.780175] | -1.59% |
| g++ | assert | sorted | 2048 | 0.104624 [0.098796,0.199468] | 0.099305 [0.098098,0.136034] | -5.08% |
| g++ | assert | sorted | 200000 | 14.602728 [12.974371,16.265599] | 12.473949 [11.088761,15.642430] | -14.58% |
| g++ | assert | tiny | 200000 | 5.126758 [4.800693,5.225418] | 4.244099 [3.976529,4.899850] | -17.22% |
| g++ | assert | wide | 2048 | 0.190700 [0.188338,0.211658] | 0.187229 [0.184515,0.187972] | -1.82% |
| g++ | assert | wide | 200000 | 26.963685 [25.569451,27.543033] | 26.047896 [25.495072,26.547610] | -3.40% |
| g++ | release | all-same | 2048 | 0.026486 [0.025388,0.027219] | 0.001137 [0.001075,0.001196] | -95.71% |
| g++ | release | all-same | 200000 | 4.269579 [3.778272,4.487662] | 0.125796 [0.124327,0.268051] | -97.05% |
| g++ | release | collinear | 2048 | 0.143482 [0.142170,0.165618] | 0.142051 [0.140482,0.146985] | -1.00% |
| g++ | release | collinear | 200000 | 22.249772 [20.217843,26.212139] | 24.032277 [21.371886,26.447171] | +8.01% |
| g++ | release | duplicates | 2048 | 0.184362 [0.179504,0.207659] | 0.207667 [0.180399,0.350367] | +12.64% |
| g++ | release | duplicates | 200000 | 18.999615 [17.750970,52.477165] | 18.556380 [18.310729,30.167653] | -2.33% |
| g++ | release | late-different | 2048 | 0.027479 [0.025921,0.146648] | 0.028708 [0.027437,0.029779] | +4.47% |
| g++ | release | late-different | 200000 | 4.178873 [3.766401,4.286546] | 4.151745 [3.924483,4.349792] | -0.65% |
| g++ | release | parabola | 2048 | 0.197712 [0.185393,0.286526] | 0.184823 [0.181628,0.208546] | -6.52% |
| g++ | release | parabola | 200000 | 27.996866 [25.255432,28.930736] | 30.497828 [26.582245,35.114537] | +8.93% |
| g++ | release | random | 2048 | 0.193923 [0.187854,0.254398] | 0.208821 [0.191128,22.439663] | +7.68% |
| g++ | release | random | 200000 | 27.214357 [26.169630,29.594555] | 26.868566 [25.738262,29.071135] | -1.27% |
| g++ | release | reversed | 2048 | 0.097147 [0.093119,0.177556] | 0.094073 [0.093205,0.094877] | -3.16% |
| g++ | release | reversed | 200000 | 10.920592 [10.663252,15.817317] | 11.474133 [10.644953,14.427195] | +5.07% |
| g++ | release | sorted | 2048 | 0.101145 [0.099192,0.103277] | 0.102157 [0.101380,0.156420] | +1.00% |
| g++ | release | sorted | 200000 | 12.554374 [12.130253,13.967537] | 12.754164 [12.297162,33.083149] | +1.59% |
| g++ | release | tiny | 200000 | 4.986037 [4.688559,5.265337] | 4.056693 [3.647192,4.254375] | -18.64% |
| g++ | release | wide | 2048 | 0.190031 [0.186540,0.377523] | 0.210666 [0.187960,0.349118] | +10.86% |
| g++ | release | wide | 200000 | 26.148361 [24.540790,28.534029] | 26.515119 [24.602877,28.796670] | +1.40% |

## Slower separated full-call comparisons

All full-call sample-separated regressions are retained here. Other overlapping regressions remain in the complete CSV.

| Compiler | Mode | Input | N parameter | Candidate | Before ms | Candidate ms | Delta |
|---|---|---|---:|---|---:|---:|---:|
| clang++ | assert | collinear | 2048 | indices | 0.136742 | 0.159472 | +16.62% |
| clang++ | assert | collinear | 2048 | records | 0.136742 | 0.167232 | +22.30% |
| clang++ | assert | collinear | 200000 | indices | 20.481035 | 28.691903 | +40.09% |
| clang++ | assert | late-different | 2048 | records | 0.030447 | 0.038447 | +26.28% |
| clang++ | assert | parabola | 200000 | indices | 26.802647 | 40.817074 | +52.29% |
| clang++ | assert | random | 200000 | indices | 25.540631 | 38.713838 | +51.58% |
| clang++ | assert | random | 200000 | records | 25.540631 | 29.225123 | +14.43% |
| clang++ | assert | reversed | 2048 | indices | 0.095980 | 0.104398 | +8.77% |
| clang++ | assert | reversed | 2048 | records | 0.095980 | 0.111387 | +16.05% |
| clang++ | assert | sorted | 2048 | indices | 0.097517 | 0.106131 | +8.83% |
| clang++ | assert | sorted | 2048 | records | 0.097517 | 0.116283 | +19.24% |
| clang++ | assert | sorted | 200000 | indices | 10.957761 | 12.550145 | +14.53% |
| clang++ | assert | sorted | 200000 | records | 10.957761 | 14.072178 | +28.42% |
| clang++ | assert | wide | 2048 | indices | 0.185380 | 0.211949 | +14.33% |
| clang++ | assert | wide | 2048 | records | 0.185380 | 0.215894 | +16.46% |
| clang++ | release | all-same | 2048 | records | 0.026686 | 0.036825 | +37.99% |
| clang++ | release | all-same | 200000 | records | 4.233943 | 6.449456 | +52.33% |
| clang++ | release | collinear | 200000 | indices | 21.548284 | 29.771459 | +38.16% |
| clang++ | release | duplicates | 2048 | indices | 0.165727 | 0.218261 | +31.70% |
| clang++ | release | duplicates | 200000 | indices | 16.049725 | 20.768243 | +29.40% |
| clang++ | release | late-different | 2048 | records | 0.029890 | 0.037427 | +25.22% |
| clang++ | release | late-different | 200000 | records | 4.592935 | 6.204210 | +35.08% |
| clang++ | release | parabola | 200000 | indices | 25.276683 | 38.145719 | +50.91% |
| clang++ | release | parabola | 200000 | records | 25.276683 | 30.538728 | +20.82% |
| clang++ | release | random | 2048 | indices | 0.188258 | 0.241339 | +28.20% |
| clang++ | release | random | 2048 | records | 0.188258 | 0.213895 | +13.62% |
| clang++ | release | random | 200000 | indices | 25.978001 | 37.071180 | +42.70% |
| clang++ | release | reversed | 2048 | indices | 0.095143 | 0.104042 | +9.35% |
| clang++ | release | reversed | 2048 | records | 0.095143 | 0.110184 | +15.81% |
| clang++ | release | reversed | 200000 | indices | 10.087885 | 12.418984 | +23.11% |
| clang++ | release | reversed | 200000 | records | 10.087885 | 12.976120 | +28.63% |
| clang++ | release | sorted | 2048 | indices | 0.099315 | 0.106784 | +7.52% |
| clang++ | release | sorted | 2048 | records | 0.099315 | 0.112797 | +13.57% |
| clang++ | release | wide | 2048 | indices | 0.196696 | 0.215432 | +9.53% |
| clang++ | release | wide | 200000 | indices | 25.734928 | 35.781453 | +39.04% |
| g++ | assert | all-same | 2048 | records | 0.026215 | 0.038878 | +48.30% |
| g++ | assert | all-same | 200000 | records | 4.447417 | 6.013232 | +35.21% |
| g++ | assert | collinear | 2048 | indices | 0.144166 | 0.168545 | +16.91% |
| g++ | assert | collinear | 2048 | records | 0.144166 | 0.166275 | +15.34% |
| g++ | assert | collinear | 200000 | indices | 20.852207 | 29.252120 | +40.28% |
| g++ | assert | late-different | 2048 | records | 0.027322 | 0.038961 | +42.60% |
| g++ | assert | late-different | 200000 | records | 4.083224 | 5.846562 | +43.18% |
| g++ | assert | parabola | 200000 | indices | 25.818205 | 37.352813 | +44.68% |
| g++ | assert | random | 200000 | indices | 26.692488 | 39.554679 | +48.19% |
| g++ | assert | tiny | 200000 | indices | 5.126758 | 5.933029 | +15.73% |
| g++ | assert | wide | 2048 | indices | 0.190700 | 0.230052 | +20.64% |
| g++ | assert | wide | 200000 | indices | 26.963685 | 38.492417 | +42.76% |
| g++ | release | all-same | 2048 | records | 0.026486 | 0.039627 | +49.61% |
| g++ | release | all-same | 200000 | records | 4.269579 | 5.722335 | +34.03% |
| g++ | release | collinear | 2048 | indices | 0.143482 | 0.169630 | +18.22% |
| g++ | release | collinear | 200000 | indices | 22.249772 | 34.148570 | +53.48% |
| g++ | release | late-different | 200000 | records | 4.178873 | 5.791316 | +38.59% |
| g++ | release | parabola | 200000 | indices | 27.996866 | 37.751037 | +34.84% |
| g++ | release | parabola | 200000 | records | 27.996866 | 31.880035 | +13.87% |
| g++ | release | random | 200000 | indices | 27.214357 | 37.938025 | +39.40% |
| g++ | release | sorted | 2048 | indices | 0.101145 | 0.108656 | +7.43% |
| g++ | release | sorted | 2048 | records | 0.101145 | 0.135844 | +34.31% |
| g++ | release | sorted | 200000 | records | 12.554374 | 15.677884 | +24.88% |
| g++ | release | wide | 200000 | indices | 26.148361 | 36.451687 | +39.40% |

## Stage, I/O and memory interpretation

`stage-comparisons.csv` includes every comparable construction/calipers/recovery and I/O phase median/min/max. Index/record layouts have no original-index recovery pass; null phases are not replaced with zero. Tiny/fast-cases have no staged decomposition. Kernel stage timing excludes input generation and objective checks; external I/O end-to-end includes process startup/pipes and objective validation after formatting. All input cases are retained to separate phases, unlike the streaming official verifier.

Allocation diagnostics are separate GCC binaries and do not contribute timings. Calls/bytes describe full algorithm calls only; bytes are cumulative requested payload, not peak-live bytes. RSS is Linux whole-process high water beneath a small parent, includes input/runtime/allocator retention and cannot establish precise algorithm-only savings. Diagnostic objectives are checked against timing identities.

The successful small gate contains all240 cross-variant profiles and six1,258-case brute runs. All12 invalid-singleton statuses were independently verified as SIGABRT(-6), although the runner accepts any nonzero status. A hypothetical small-gate JSON/schema failure would retain the rejected command/output but not earlier in-memory small rows; no such failure occurred in this experiment.

Cached KACTL/maspypy/official source contracts and licenses are described in `../research-notes.md`. The original Fastest403 remains a documented limitation; no leaderboard speed comparison or network retry was performed in this resumed performance experiment.
