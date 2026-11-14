# Dynamic Fenwick Tree: sparse prefix counting

ABC484 E's submission motivated these synthetic, deterministic workloads. They
do not reproduce AtCoder's hidden data or its complete program. The tree has
length 3e18 and coordinate offset 1.5e18; 300,000 positive and negative steps
are generated with `mt19937_64(1729)`. Each iteration adds the preceding prefix
and asks for the number of previous prefixes in the interval determined by the
new prefix and fixed `L` and `R`.

- `contest-near`: steps in [-1000, 1000], L=-1e12, R=1e12.
- `contest-diffuse`: steps in [-1e12, 1e12], L=-1e12, R=1e12.
- `contest-broad`: same steps, L=-1e18, R=1e18.

The auxiliary `walk`, `diffuse`, `wide`, and `ascending` cases use changing
query bounds and a different update order; `wide` can exceed the contest's
per-step limit. They explore the data structure, not the contest input contract.
The program builds the input before timing, times construction and interleaved
updates/queries, and excludes parsing, formatting and destruction. Peak RSS
includes generated input and the process. It checks that every variant's
checksum agrees. It does not establish the actual submission's verdict or time.

## Initial hash candidate

On Intel Core i7-14650HX, WSL2 Linux, GCC 13.3.0, `-std=gnu++20 -O2`, one
warmup then three measured runs per shape and variant in rotated order. Times
are medians in milliseconds; peak RSS is median KiB. The Nyaan implementation
uses a random hash multiplier, so results may vary. Competing compilation and
tests were stopped during the final measured runs. Release (`-DNDEBUG`):

| Shape | Before ms / KiB | Candidate ms / KiB | Nyaan ms / KiB |
| --- | ---: | ---: | ---: |
| contest-near | 216 / 22,128 | 172 / 28,748 | 135 / 22,580 |
| contest-diffuse | 2,184 / 203,216 | 996 / 305,168 | 706 / 208,836 |
| contest-broad | 2,370 / 203,264 | 985 / 305,132 | 767 / 208,820 |

Assertions enabled, respectively: near 221 / 188 / 145 ms, diffuse 2,254 /
981 / 645 ms, broad 2,025 / 822 / 650 ms. The separate runs should not be
interpreted as an assert-overhead comparison because system load varies.
The candidate improves on the before revision but is slower and uses more
memory than Nyaan on these cases. The measured difference is not yet attributed
to a single cause; `optional<T>` storage and hashing are possible factors.

### Follow-up candidates

Additional four-way runs use the same compiler, input, flags, warmup and three
measured trials. The two candidate columns are measured in the same run. They
are distinct from the earlier measurements; system load and frequency varied.
All checksums matched. Full samples and hashes are in the linked JSON files.

- [Compact storage](results/dynamic-fenwick-1d-compact.json) replaces optional
  values with plain storage only for selected trivial types, leaving generic
  types on the optional path. Median optional / compact / Nyaan ms: near
  135 / 136 / 113, diffuse 708 / 743 / 542, broad 837 / 758 / 554. Diffuse
  peak RSS was 305,156 / 206,884 / 208,876 KiB, respectively. The memory
  reduction is clear; the speed difference depends on the distribution.
- [Fixed multiplication hash](results/dynamic-fenwick-1d-multiply.json)
  changes only the compact variant's splitmix hash to multiplication and uses
  the high bits for bucket selection. Median fixed multiplication / splitmix /
  Nyaan ms: near 124 / 162 / 134, diffuse 935 / 1,210 / 885, broad 1,067 /
  1,123 / 763. Individual samples fluctuate substantially. The collision
  experiment below makes the fixed multiplier unsuitable as a replacement.
- [Unsigned add](results/dynamic-fenwick-1d-unsigned.json) changes only the
  compact variant's update loop to use unsigned coordinates. Median unsigned /
  signed / Nyaan ms: near 208 / 202 / 185, diffuse 988 / 828 / 780, broad
  980 / 989 / 841. No consistent speedup was established.

The [collision experiment](results/dynamic-fenwick-1d-collision.json) uses
2,048 updates at spacing 2^30 or generated against the fixed multiplier. For
the latter, each update's first visited Fenwick cell `i=p+1` is
chosen via the multiplier's inverse modulo 2^64 so that the top 20 bits of
`i * multiplier` are zero. If capacity is at most 2^20, these cells all start
at the same bucket. This input is tailored only to the fixed multiplier, not
the Nyaan implementation's randomized multiplier; it does not establish a
worst-case bound for any candidate. Median fixed multiplication / splitmix /
Nyaan times: stride 1.39 / 1.87 / 2.14 ms, targeted collision 9.70 / 2.91 /
4.62 ms. At [20,000 targeted updates](results/dynamic-fenwick-1d-collision-large.json),
the same medians were 655 / 32.2 / 42.4 ms, with RSS 28,236 / 28,200 /
28,548 KiB. Actual table capacity was not instrumented. This demonstrates a
substantial input-dependent regression, so the fixed multiplier is rejected.

An [augmented AVL prototype](results/dynamic-fenwick-1d-avl.json) keeps one
node per distinct updated coordinate, instead of hash entries for Fenwick
cells. It supports only `long long` coordinates and values and is a benchmark
prototype, not an implementation validated for the public API. Against the
compact Fenwick candidate and Nyaan in the same runs, median AVL / compact /
Nyaan ms were near 73 / 155 / 138, diffuse 121 / 890 / 715, broad 77 / 739 /
521, and ascending 65 / 142 / 117. Diffuse median RSS was 31,668 / 206,884 /
208,868 KiB. All measured checksums agree, but that does not verify its full
API, type requirements, overflow behavior, or node limits. The alternative
changes the aggregate intervals maintained internally, so its compatibility
with existing numeric contracts needs separate analysis. No adoption is
claimed by these measurements.

### Integrated implementation

After integrating the AVL-based path for signed integral values of at most
64 bits, the actual public header was measured against the original version,
the compact hash candidate and Nyaan in separate processes under the same conditions.
Each case used `long long` coordinates and values and the contest-shaped
driver. Median times and RSS are shown below; all checksums matched.

| Shape | Public ms / KiB | Compact ms / KiB | Nyaan ms / KiB | Original ms / KiB |
| --- | ---: | ---: | ---: | ---: |
| contest-near | 60 / 28,200 | 116 / 22,528 | 102 / 22,524 | 151 / 21,992 |
| contest-diffuse | 106 / 43,544 | 706 / 206,884 | 525 / 208,804 | 1,813 / 203,184 |
| contest-broad | 73 / 44,368 | 652 / 206,856 | 490 / 208,764 | 1,766 / 203,216 |

The three measured samples, exact header hashes, CPU, compiler and flags are
in [final JSON](results/dynamic-fenwick-1d-final.json) and the adjacent run
log. These are local synthetic measurements, not AtCoder verdicts; they
exclude I/O and cover only the specified integral specialization and query
distributions.

All individual measurements, checksums, environment, flags, and SHA-256s are
in [release JSON](results/dynamic-fenwick-1d-release.json) and
[assert JSON](results/dynamic-fenwick-1d-assert.json); their execution logs are
adjacent. [Exploratory measurements](results/dynamic-fenwick-1d.json) preceded
the final run and may overlap other compilation; their exact driver is saved
as `results/dynamic-fenwick-1d-exploratory-driver.cpp`. They must not be mixed
with the final measurements. The initial runner attempts failed to compile:
first the output file had the same name as an include directory; after that was
fixed, the Nyaan header required `using namespace std` before inclusion. No
timings were produced by those failed attempts; their full output was not
saved. The errors were fixed before all recorded runs.

## Reproduction

Baseline is revision `123365f2738e3d883c566def538e2833f411c317`. The
[Nyaan source](https://github.com/NyaanNyaan/library/tree/b3981adc80a800b2584980b01821324ea6c77183/data-structure)
is revision `b3981adc80a800b2584980b01821324ea6c77183` (CC0-1.0). Fetch the
two files into the build directory and compare the checked-out candidate:

```sh
mkdir -p .build/dynamic-fenwick-1d/reference
git show 123365f2738e3d883c566def538e2833f411c317:blueberry/data-structure/dynamic-fenwick-tree.hpp > .build/dynamic-fenwick-1d/baseline.hpp
ref=b3981adc80a800b2584980b01821324ea6c77183
for f in dynamic-binary-indexed-tree.hpp hash-map-variable-length.hpp; do
  curl -fsSL "https://raw.githubusercontent.com/NyaanNyaan/library/$ref/data-structure/$f" -o ".build/dynamic-fenwick-1d/reference/$f"
done
python3 benchmark/run_dynamic_fenwick_1d.py --baseline .build/dynamic-fenwick-1d/baseline.hpp --nyaan .build/dynamic-fenwick-1d/reference --candidate benchmark/results/dynamic-fenwick-1d-optional-candidate.hpp.txt --driver benchmark/results/dynamic-fenwick-1d-contest-driver.cpp --runs 3 --output .build/dynamic-fenwick-1d/release.json
python3 benchmark/run_dynamic_fenwick_1d.py --baseline .build/dynamic-fenwick-1d/baseline.hpp --nyaan .build/dynamic-fenwick-1d/reference --candidate benchmark/results/dynamic-fenwick-1d-optional-candidate.hpp.txt --driver benchmark/results/dynamic-fenwick-1d-contest-driver.cpp --runs 3 --asserts --shapes contest-near contest-diffuse contest-broad --output .build/dynamic-fenwick-1d/assert.json
python3 benchmark/run_dynamic_fenwick_1d.py --baseline .build/dynamic-fenwick-1d/baseline.hpp --nyaan .build/dynamic-fenwick-1d/reference --candidate benchmark/results/dynamic-fenwick-1d-final-candidate.hpp.txt --compare benchmark/results/dynamic-fenwick-1d-compact-candidate.hpp.txt --driver benchmark/results/dynamic-fenwick-1d-contest-driver.cpp --runs 3 --shapes contest-near contest-diffuse contest-broad --output .build/dynamic-fenwick-1d/final.json
```

The exact candidate header and driver hashes are in the JSON. The final
candidate snapshot preserves the measured working-tree bytes, including line
endings; the Git-normalized public header has the same C++ contents. The measured candidate headers and the initial
contest driver are preserved in `results/dynamic-fenwick-1d-*-candidate.hpp.txt`
and `results/dynamic-fenwick-1d-contest-driver.cpp`. Pass `--candidate` and
`--compare` with these saved headers to rerun candidate comparisons. The current
driver additionally supports `--shapes stride adversarial`; use `--count 2048`
or `20000` to reproduce the collision experiments. No I/O, official data, submitted AtCoder run, Clang
performance, or isolated operation phases were measured here.
