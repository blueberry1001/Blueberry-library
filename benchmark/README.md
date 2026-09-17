# Benchmark environment

The benchmark suite complements `make verify`: verification-helper measures complete official
Library Checker cases, while this directory contains deterministic microbenchmarks that separate
construction and query costs. No production header is replaced by a benchmark implementation.

## Run

```console
make benchmark
```

Results are written to `.benchmark/results/latest/results.json` and `report.md`. The default matrix
uses the current Library Checker-style `-O2 -march=native` flags and an `-O3` variant. Use the
portable set on machines where `-march=native` is undesirable:

```console
python3 scripts/run_benchmarks.py --flag-set portable --repeats 11 --warmups 2
```

Useful options:

- `--target 'static-rmq-small-*'` selects targets with a glob.
- `--compiler /path/to/g++` compares another compiler when run into a separate output directory.
- `--cpu 2` pins benchmark processes to one CPU with `taskset`.
- `--flag-set NAME` is repeatable; flag sets live in `benchmark/manifest.json`.

The runner compiles every selected target first, performs warmups, then rotates execution order
between repetitions. It records raw samples, median, min/max, and median absolute deviation. Targets
in the same group must emit the same checksum. Ratios are only formed inside one workload and flag
set.

## Library Checker research

```console
make research-static-rmq
```

`scripts/fetch_lc_submissions.py` uses the public read-only REST API to fetch fastest rows, optional
user-best rows, case maxima, and source feature heuristics. Responses are cached for 24 hours under
`.benchmark/lc-cache` to avoid repeatedly downloading public source. Reports contain source hashes
and feature flags, not source text. `--save-source-dir` is an explicit opt-in for local inspection;
check each submission's licensing before reusing or committing any code.

Example for several verify targets:

```console
python3 scripts/fetch_lc_submissions.py \
  --problem staticrmq --problem point_add_range_sum --problem lca \
  --user blueberry1001 --analyze-top 1 --output .benchmark/library-checker/selected
```

Library Checker leaderboard time is a maximum testcase time, not the sum used by
`scripts/verify_with_metrics.py`. Leaderboard ratios and local ratios must therefore remain separate.

## Legacy library migration comparisons

- [Dynamic Li Chao Tree](dynamic-li-chao-tree.md): online allocation/layout and midpoint/iteration variants, plus the compressed version.
- [Dynamic Fenwick Tree](dynamic-fenwick-tree.md): owned online AVL, legacy hash implementation and offline registration.
- [Fraction](fraction.md): exact rational arithmetic versus Boost and a rejected gcd fast-path experiment.
- Treap comparisons and Library Checker Fastest research: `legacy-treap.py`, `legacy-treap-fastest.py`, and `results/legacy-treap/`.
