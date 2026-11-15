#!/usr/bin/env python3
"""Compare weighted wavelet construction with a rank-writeback experiment."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import shlex
import subprocess
import time

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--compiler", default="g++")
parser.add_argument("--flags", default="-std=gnu++20 -O2 -DNDEBUG")
parser.add_argument("--candidate", type=Path)
parser.add_argument("--runs", type=int, default=3)
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
build = root / ".build/weighted-wavelet-rank"
build.mkdir(parents=True, exist_ok=True)
baseline = (root / "benchmark/weighted-wavelet-rank-baseline.hpp.txt").read_text()
fenwick = (root / "benchmark/weighted-wavelet-rank-fenwick.hpp.txt").read_bytes()
before = '''    std::sort(values_.begin(), values_.end());
    values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
    if (n_ == 0) return;
    const int height = std::bit_width(static_cast<unsigned>(values_.size() - 1));
    std::vector<int> codes(n_), next(n_);
    std::vector<Weight> current(weights), next_weights(n_);
    for (int i = 0; i < n_; ++i) codes[i] = lower(values[i]);'''
after = '''    if (n_ == 0) return;
    std::vector<int> codes(n_), next(n_);
    std::iota(next.begin(), next.end(), 0);
    std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });
    int distinct = 0;
    for (int i : next) {
      if (distinct == 0 || !(values_[distinct - 1] == values[i])) values_[distinct++] = values[i];
      codes[i] = distinct - 1;
    }
    values_.resize(distinct);
    const int height = std::bit_width(static_cast<unsigned>(values_.size() - 1));
    std::vector<Weight> current(weights), next_weights(n_);'''
if baseline.count(before) != 1:
    raise RuntimeError("baseline no longer matches the experiment")
rank = baseline.replace("#include <limits>", "#include <limits>\n#include <numeric>").replace(before, after)
compare_sort = '    std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });'
adaptive_sort = '''    if constexpr (std::is_integral_v<Value> && !std::is_same_v<Value, bool>) {
      using U = std::make_unsigned_t<Value>;
      U base = 0, remaining = 0;
      int passes = 0;
      if (n_ >= 64) {
        auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
        base = static_cast<U>(*minimum);
        remaining = U(static_cast<U>(*maximum) - base);
        for (U width = remaining; width; width = U(width >> 8)) ++passes;
      }
      if (n_ < 64 || n_ < 128 * passes) {
        std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });
      } else {
        for (int shift = 0; remaining; shift += 8, remaining = U(remaining >> 8)) {
          std::array<int, 256> offsets{};
          for (int p : next) ++offsets[(U(static_cast<U>(values[p]) - base) >> shift) & 255];
          int offset = 0;
          for (int& count : offsets) { int size = count; count = offset; offset += size; }
          for (int p : next) codes[offsets[(U(static_cast<U>(values[p]) - base) >> shift) & 255]++] = p;
          codes.swap(next);
        }
      }
    } else {
      std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });
    }'''
if rank.count(compare_sort) != 1:
    raise RuntimeError("rank candidate no longer matches the experiment")
radix = rank.replace("#include <algorithm>", "#include <algorithm>\n#include <array>\n#include <type_traits>").replace(compare_sort, adaptive_sort)
compression = '''    std::iota(next.begin(), next.end(), 0);
    std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });
    int distinct = 0;
    for (int i : next) {
      if (distinct == 0 || !(values_[distinct - 1] == values[i])) values_[distinct++] = values[i];
      codes[i] = distinct - 1;
    }
    values_.resize(distinct);'''
dense_compression = '''    bool dense = false;
    if constexpr (std::is_integral_v<Value> && !std::is_same_v<Value, bool>) {
      using U = std::make_unsigned_t<Value>;
      auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
      U base = static_cast<U>(*minimum);
      U span = U(static_cast<U>(*maximum) - base);
      if (static_cast<std::uint64_t>(span) <= std::min<std::uint64_t>(1u << 20, std::uint64_t(n_) * 2)) {
        std::vector<int> mapping(static_cast<std::size_t>(span) + 1, -1);
        for (const auto& v : values) mapping[U(static_cast<U>(v) - base)] = 0;
        int distinct = 0;
        for (int& rank : mapping) if (rank == 0) rank = distinct++;
        for (int i = 0; i < n_; ++i) {
          codes[i] = mapping[U(static_cast<U>(values[i]) - base)];
          values_[codes[i]] = values[i];
        }
        values_.resize(distinct);
        dense = true;
      }
    }
    if (!dense) {
      std::iota(next.begin(), next.end(), 0);
      std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });
      int distinct = 0;
      for (int i : next) {
        if (distinct == 0 || !(values_[distinct - 1] == values[i])) values_[distinct++] = values[i];
        codes[i] = distinct - 1;
      }
      values_.resize(distinct);
    }'''
if rank.count(compression) != 1:
    raise RuntimeError("rank compression no longer matches the experiment")
dense = rank.replace("#include <algorithm>", "#include <algorithm>\n#include <type_traits>").replace(compression, dense_compression)
hybrid_compression = '''    bool encoded = false;
    if (std::is_sorted(values.begin(), values.end())) {
      int distinct = 0;
      for (int i = 0; i < n_; ++i) {
        if (distinct == 0 || !(values_[distinct - 1] == values[i])) values_[distinct++] = values[i];
        codes[i] = distinct - 1;
      }
      values_.resize(distinct);
      encoded = true;
    }
    if constexpr (std::is_integral_v<Value> && !std::is_same_v<Value, bool>) {
      using U = std::make_unsigned_t<Value>;
      if (!encoded && n_ >= 64) {
        auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
        U base = static_cast<U>(*minimum);
        U span = U(static_cast<U>(*maximum) - base);
        if (static_cast<std::uint64_t>(span) <= std::min<std::uint64_t>(1u << 20, std::uint64_t(n_) * 2)) {
          std::vector<int> mapping(static_cast<std::size_t>(span) + 1, -1);
          for (const auto& v : values) mapping[U(static_cast<U>(v) - base)] = 0;
          int distinct = 0;
          for (int& item : mapping) if (item == 0) item = distinct++;
          for (int i = 0; i < n_; ++i) {
            codes[i] = mapping[U(static_cast<U>(values[i]) - base)];
            values_[codes[i]] = values[i];
          }
          values_.resize(distinct);
          encoded = true;
        } else {
          int passes = 0;
          for (U width = span; width; width = U(width >> 8)) ++passes;
          if (passes <= 6 && n_ >= 128 * passes) {
            std::iota(next.begin(), next.end(), 0);
            for (int shift = 0; span; shift += 8, span = U(span >> 8)) {
              std::array<int, 256> offsets{};
              for (int p : next) ++offsets[(U(static_cast<U>(values[p]) - base) >> shift) & 255];
              int offset = 0;
              for (int& count : offsets) { int size = count; count = offset; offset += size; }
              for (int p : next) codes[offsets[(U(static_cast<U>(values[p]) - base) >> shift) & 255]++] = p;
              codes.swap(next);
            }
            int distinct = 0;
            for (int i : next) {
              if (distinct == 0 || !(values_[distinct - 1] == values[i])) values_[distinct++] = values[i];
              codes[i] = distinct - 1;
            }
            values_.resize(distinct);
            encoded = true;
          }
        }
      }
    }
    if (!encoded) {
      std::iota(next.begin(), next.end(), 0);
      std::sort(next.begin(), next.end(), [&](int i, int j) { return values[i] < values[j]; });
      int distinct = 0;
      for (int i : next) {
        if (distinct == 0 || !(values_[distinct - 1] == values[i])) values_[distinct++] = values[i];
        codes[i] = distinct - 1;
      }
      values_.resize(distinct);
    }'''
hybrid = rank.replace("#include <algorithm>", "#include <algorithm>\n#include <array>\n#include <type_traits>").replace(compression, hybrid_compression)
sample_condition = '''int sampled = std::min(n_, 256), extreme = 0;
          for (int i = 0; i < sampled; ++i)
            extreme += values[i] == *minimum || values[i] == *maximum;
          if ((passes <= 6 || (passes <= 8 && extreme * 2 < sampled)) && n_ >= 128 * passes)'''
hybrid_sample = hybrid.replace("if (passes <= 6 && n_ >= 128 * passes)", sample_condition)
headers = {"baseline": baseline.encode(), "rank": rank.encode(), "radix": radix.encode(),
           "dense": dense.encode(), "hybrid": hybrid.encode(), "hybrid-sample": hybrid_sample.encode()}
if args.candidate:
    headers["candidate"] = args.candidate.read_bytes()
flags = shlex.split(args.flags)
driver = root / "benchmark/weighted-wavelet-rank.cpp"
for name, contents in headers.items():
    inc = build / name
    (inc / "blueberry/data-structure").mkdir(parents=True, exist_ok=True)
    (inc / "weighted-wavelet-matrix.hpp").write_bytes(contents)
    (inc / "blueberry/data-structure/fenwick-tree.hpp").write_bytes(fenwick)
    subprocess.run([args.compiler, *flags, "-I", str(inc), str(driver), "-o", str(build / (name + ".out"))], check=True)

cpu = next((line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name")), platform.processor())
sha = lambda b: hashlib.sha256(b).hexdigest()
data = {"revision": "efa8b4e97ce68395657c643fd6c04f2fba56b826", "timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "cpu": cpu, "platform": platform.platform(), "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
        "flags": flags, "runs": args.runs, "seed": 1729, "sha256": {**{name: sha(content) for name, content in headers.items()},
        "fenwick": sha(fenwick), "driver": sha(driver.read_bytes()), "runner": sha(Path(__file__).read_bytes())}, "samples": []}
args.output.parent.mkdir(parents=True, exist_ok=True)
cases = [(128, 2000, "random", "int64"), (40000, 20000, "random", "int64"),
         (200000, 20000, "random", "int64"), (200000, 20000, "duplicate", "int64"),
         (200000, 20000, "equal", "int64"), (200000, 20000, "signed-small", "int64"),
         (200000, 20000, "extrema", "int64"), (200000, 20000, "ascending", "int64"),
         (200000, 20000, "descending", "int64"), (40000, 20000, "random", "string")]
for n, q, shape, kind in cases:
    expected = None
    variants = list(headers)
    for run in range(-1, args.runs):
        offset = (run + 1) % len(variants)
        for name in variants[offset:] + variants[:offset]:
            cmd = ["/usr/bin/time", "-f", "%M", str(build / (name + ".out")), str(n), str(q), shape, kind]
            p = subprocess.run(cmd, text=True, capture_output=True)
            if p.returncode:
                data["error"] = {"command": cmd, "returncode": p.returncode, "stdout": p.stdout, "stderr": p.stderr}
                args.output.write_text(json.dumps(data, indent=2) + "\n")
                raise RuntimeError(f"{name}: return code {p.returncode}")
            checksum, construction, operations = p.stdout.split()
            if expected is None:
                expected = checksum
            if checksum != expected:
                data["error"] = {"command": cmd, "checksum": checksum, "expected": expected}
                args.output.write_text(json.dumps(data, indent=2) + "\n")
                raise RuntimeError("checksum mismatch")
            print(n, q, shape, kind, run, name, construction, operations, flush=True)
            if run >= 0:
                data["samples"].append({"n": n, "q": q, "shape": shape, "type": kind, "run": run + 1, "variant": name,
                    "build_ms": float(construction), "operations_ms": float(operations),
                    "rss_kib": int(p.stderr.strip().splitlines()[-1]), "checksum": int(checksum)})
                args.output.write_text(json.dumps(data, indent=2) + "\n")
