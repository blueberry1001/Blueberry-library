# Static RMQ cost study harness

These are exact copies of the two measured, independently written experiment
files. They are research artifacts, not installed library APIs or CI harnesses.
The successful measurement's `environment.json` identifies them at their original
`benchmarks/` paths; their bytes and hashes are unchanged here. The failed first
run used the runner before adding the explicit AVX2 flag for submission 402567;
its compile log records that failure. No third-party source is redistributed.

To reproduce from the repository root on Linux, restore the experiment files:

```sh
cp docs/development/measurements/fastest-study/harness/static-rmq-cost-breakdown.cpp benchmarks/
cp docs/development/measurements/fastest-study/harness/run-static-rmq-cost-breakdown.py benchmarks/
python3 benchmarks/run-static-rmq-cost-breakdown.py \
  --problem-dir "$HOME/.cache/online-judge-tools/library-checker-problems/data_structure/staticrmq" \
  --output .build/rmq-study-new
```

The output directory must not already exist. It keeps every warmup/sample, full
output checks, compiler output, source/input hashes, compiler versions, CPU and
the official problem repository revision. It pins sequential children to one
available logical CPU; this does not eliminate host scheduling or thermal noise.
The `problem-dir` must contain generated official `in/` and `out/` files.
The measured problem revision was `1814c4e5205517e368bb57a8d1127eb961cfeaae`.
Use the successful environment manifest to check input/output hashes.

Optional original-source comparisons use `--reference-dir PATH`, with files named
`362370.cpp`, `362362.cpp`, `400989.cpp`, `402567.cpp`. Obtain their `source` field
through the public `https://v3.api.judge.yosupo.jp/submissions/ID` API. There are
two distinct hashes: `source_sha256` in `rmq-reference/details.json` identifies
the API source string encoded as UTF-8; `compiled_file_sha256` and the environment
identify the actual file compiled. The original Windows text write translated
every LF to CRLF once. To reproduce those exact file bytes, encode the API string
as UTF-8, replace each `b'\n'` with `b'\r\n'`, and write bytes (no further text-mode
conversion). Do not confuse the API hash with the compiled-file hash. Both are
preserved; this newline difference does not change the C++ code's semantics.
The runner uses GCC/C++23 and retains source pragmas. Submission 402567 additionally
requires AVX2 and is compiled with `-mavx2`; do not run it on a CPU without AVX2.
Never define `LOCAL` for these references. Their stdin must be a regular file,
not a pipe. These restrictions apply to the original references, not the
portable buffered-I/O prototype.

The prototype only handles the official RMQ integer domain. It is not a complete
public FastIO implementation. `batch` preloads queries solely to separate timing
phases; `stream` reads and answers each query in order. A `stream` query phase
includes query parsing and output, so it is not a kernel-only timing. Process
time includes `/usr/bin/time`, startup/destruction and file I/O. RSS includes
query/answer buffers. References use different optimization/ISA assumptions and
do not establish like-for-like improvements over the generic APIs.

Raw failed measurements are retained separately and never represented as zero.
The runner stops on compile failure, nonzero exit, timeout or output mismatch;
no incomplete run gets a successful summary. GCC/Clang C++20, `/usr/bin/time`,
Python 3 and generated official inputs are required. Original references also
need the C++23 library facilities used by their source.
