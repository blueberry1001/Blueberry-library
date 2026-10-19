# Sparse formal power series exponential measurements

Run from Linux/WSL, with the repository and pinned ACL available:

```sh
python3 benchmarks/sparse-exp/run.py --acl .deps/ac-library --output .benchmark/sparse-exp-initial
```

The output directory must not already contain `samples.jsonl`; each rerun needs a new directory. Defaults use GCC and Clang, `-O2 -DNDEBUG -std=gnu++20`, one warmup plus three measured executions per input and variant. Use `--extra-flags=-UNDEBUG` for a separate assertions-enabled measurement. Do not combine those results with release measurements.

The independent variants implement the same normalized-input contract and recurrence `g[i] = sum(d*f[d]*g[i-d])/i`. `division` computes the inverse by modular division at every coefficient, `table` precomputes inverses, and `batch` accumulates bounded groups of integer products in a portable uint64 accumulator before modular reduction. The block limit derives from `(mod-1)^2`, so no overflowing accumulator is used. The public `api` is called directly. The final API adopts bounded batching for at least four normalized terms and skips the recurrence when f squared vanishes at the requested precision. No external implementation source is included.

Inputs are deterministic, reverse ordered, and have early, late, and spread degrees, with `n=500000` and `k=1,10,100`, plus a dense `n=500,k=499` case. All output coefficient hashes must match across variants, repetitions, and compilers. Input and output hashes use FNV-1a; source and selected ACL header hashes use SHA-256.

Every execution records in-memory text parsing, complete algorithm, formatting, process wall time, and process peak RSS from `/usr/bin/time`. The independent variants additionally expose normalization, inverse-table construction, and recurrence/allocation phases. API algorithm time includes every internal phase. Formatting uses `to_chars` into an allocated string; output is hashed outside the format timer. Input generation, hashes, filesystem I/O, and process startup are excluded from the phase timers and included in process wall time where applicable. This is a computation microbenchmark; it does not replace the official problem's end-to-end file I/O measurement. For tiny inputs use phase timing rather than process wall/RSS differences.

`metadata.json` preserves machine, compiler, flags, git revision and source hashes; `samples.jsonl` preserves every raw sample, including warmups and failures; `summary.json` contains median/min successful samples. Failed and missing measurements have error records and never receive fabricated zero timings. Check sample counts and errors before interpreting summaries. The build uses the current working tree; the git revision alone does not identify uncommitted source, hence the source hashes.

`official_compare.py` additionally compares the existing public verify program with unmodified downloaded Fastest submissions on exact official `.in`/`.out` pairs. Fetch sources into an ignored or external directory using `scripts/fetch_lc_fastest.py --save-source --out-dir <directory>` (the source-fetching option is `--out-dir`). Then run:

```sh
python3 benchmarks/sparse-exp/official_compare.py --cases <official-test-directory> --source-dir <download-directory> --output .benchmark/sparse-exp-official
```

The default submission is 409367. Each compiler builds both programs with the identical `-std=gnu++20 -O2 -DNDEBUG -mavx2 -mbmi -mbmi2` flags; the machine must support those instructions. These comparison flags do not change the library's portable contract. The default selects the first lexicographic match of `max_random_*.in`, `min_K_*.in`, and `small_dense_*.in`; repeat `--case <glob>` to select different representatives or use `--all-cases`. At least three distinct cases are required. Default timing uses one warmup and three measured runs, actual file redirection, sequential execution, and per-process peak RSS. Every output is checked against official integer tokens outside the timed process, accepting legal leading zeroes and whitespace. Raw samples, compiler logs, errors, hashes, RSS logs, and failed output files are retained. Successful output files are removed after hashing and validation. The output directory must be new, and compilation/execution/missing-case failures produce a nonzero exit after saving available metadata and logs.

Use `--phases` in a fresh output directory to compile `phase_driver.cpp` in place of the uninstrumented Blueberry verifier. It times the same actual FastInput and FastOutput operations, includes `flush()` in formatting, and emits phase JSON to stderr. Keep this diagnostic separate from the uninstrumented end-to-end comparison. Phase timings are not obtained by subtracting independent process medians.
