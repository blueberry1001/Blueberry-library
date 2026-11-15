#!/usr/bin/env python3
"""Compare common-prefix range sums with the unchanged online contract."""
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
parser.add_argument("--baseline", type=Path, required=True)
parser.add_argument("--compiler", default="g++")
parser.add_argument("--flags", default="-std=gnu++20 -O2 -DNDEBUG")
parser.add_argument("--runs", type=int, default=3)
parser.add_argument("--sanitizers", action="store_true")
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
build = root / ".build/weighted-wavelet-ranges"
build.mkdir(parents=True, exist_ok=True)
baseline = args.baseline.read_text()
old = '''  Weight sum(int l, int r, const Value& low, const Value& upper) const {
    return less(l, r, lower(upper)) - less(l, r, lower(low));
  }'''
new = '''  Weight sum(int l, int r, const Value& low, const Value& upper) const {
    const int lo = lower(low), hi = lower(upper);
    if (lo == hi) return Weight{};
    if (hi == static_cast<int>(values_.size())) return total_.sum(l, r) - less(l, r, lo);
    const int height = static_cast<int>(levels_.size());
    int first = 0;
    while (first < height && ((lo ^ hi) >> (height - 1 - first)) == 0) {
      const auto& level = levels_[first];
      const int zl = l - level.rank(l), zr = r - level.rank(r);
      if ((lo >> (height - 1 - first)) & 1) {
        l += level.zeros - zl; r += level.zeros - zr;
      } else { l = zl; r = zr; }
      ++first;
    }
    auto suffix = [&](int code) {
      int left = l, right = r;
      Weight result{};
      for (int h = first; h < height; ++h) {
        const auto& level = levels_[h];
        const int zl = left - level.rank(left), zr = right - level.rank(right);
        if ((code >> (height - 1 - h)) & 1) {
          result += level.weights.sum(zl, zr);
          left += level.zeros - zl; right += level.zeros - zr;
        } else { left = zl; right = zr; }
      }
      return result;
    };
    return suffix(hi) - suffix(lo);
  }'''
if baseline.count(old) != 1:
    raise RuntimeError("baseline sum implementation did not match")
safe = '''  Weight sum(int l, int r, const Value& low, const Value& upper) const {
    if constexpr (std::is_integral_v<Weight> && std::is_signed_v<Weight> && sizeof(Weight) <= 8) {
      using U = std::make_unsigned_t<Weight>;
      const int lo = lower(low), hi = lower(upper);
      if (lo == hi) return Weight{};
      if (hi == static_cast<int>(values_.size())) return total_.sum(l, r) - less(l, r, lo);
      const int height = static_cast<int>(levels_.size());
      int first = 0;
      while (first < height && ((lo ^ hi) >> (height - 1 - first)) == 0) {
        const auto& level = levels_[first];
        const int zl = l - level.rank(l), zr = r - level.rank(r);
        if ((lo >> (height - 1 - first)) & 1) {
          l += level.zeros - zl; r += level.zeros - zr;
        } else { l = zl; r = zr; }
        ++first;
      }
      auto suffix = [&](int code) {
        int left = l, right = r;
        U result{};
        for (int h = first; h < height; ++h) {
          const auto& level = levels_[h];
          const int zl = left - level.rank(left), zr = right - level.rank(right);
          if ((code >> (height - 1 - h)) & 1) {
            result += static_cast<U>(level.weights.sum(zl, zr));
            left += level.zeros - zl; right += level.zeros - zr;
          } else { left = zl; right = zr; }
        }
        return result;
      };
      return static_cast<Weight>(static_cast<U>(suffix(hi) - suffix(lo)));
    } else {
      return less(l, r, lower(upper)) - less(l, r, lower(low));
    }
  }'''
headers = {"baseline": baseline.encode(), "shared": baseline.replace(old, new).encode(),
           "shared-safe": baseline.replace("#include <numeric>", "#include <numeric>\n#include <type_traits>").replace(old, safe).encode()}
fenwick = (root / "benchmark/weighted-wavelet-rank-fenwick.hpp.txt").read_bytes()
driver = root / "benchmark/weighted-wavelet-range.cpp"
flags = shlex.split(args.flags)
for name, contents in headers.items():
    inc = build / name
    (inc / "blueberry/data-structure").mkdir(parents=True, exist_ok=True)
    (inc / "weighted-wavelet-matrix.hpp").write_bytes(contents)
    (inc / "blueberry/data-structure/fenwick-tree.hpp").write_bytes(fenwick)
    subprocess.run([args.compiler, *flags, "-I", str(inc), str(driver), "-o", str(build / (name + ".out"))], check=True)
if args.sanitizers:
    for name in headers:
        inc = build / name
        subprocess.run([args.compiler, "-std=gnu++20", "-O1", "-g", "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
                        "-I", str(inc), str(root / "benchmark/weighted-wavelet-overflow.cpp"), "-o", str(build / (name + "-ubsan.out"))], check=True)
sha = lambda b: hashlib.sha256(b).hexdigest()
cpu = next((s.split(":", 1)[1].strip() for s in Path("/proc/cpuinfo").read_text().splitlines() if s.startswith("model name")), platform.processor())
data = {"timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()), "cpu": cpu,
        "platform": platform.platform(), "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
        "flags": flags, "seed": 1729, "runs": args.runs, "sha256": {**{name: sha(value) for name,value in headers.items()},
        "fenwick": sha(fenwick), "driver": sha(driver.read_bytes()), "runner": sha(Path(__file__).read_bytes())}, "samples": []}
args.output.parent.mkdir(parents=True, exist_ok=True)
cases = [(200000, 100000, "random", "int64", mode) for mode in ("same", "narrow", "full", "random")]
cases += [(40000, 20000, "random", "string", "narrow"), (200000, 100000, "duplicate", "int64", "narrow")]
for n,q,shape,kind,mode in cases:
    expected = None
    variants = list(headers)
    for run in range(-1,args.runs):
        offset = (run + 1) % len(variants)
        for name in variants[offset:] + variants[:offset]:
            cmd = ["/usr/bin/time", "-f", "%M", str(build / (name + ".out")), str(n), str(q), shape, kind, mode]
            p = subprocess.run(cmd, text=True, capture_output=True)
            if p.returncode:
                data["error"] = {"command": cmd, "returncode": p.returncode, "stdout": p.stdout, "stderr": p.stderr}
                args.output.write_text(json.dumps(data, indent=2) + "\n")
                raise RuntimeError("execution failed")
            checksum, construction, operations = p.stdout.split()
            if expected is None: expected = checksum
            if checksum != expected:
                data["error"] = {"command": cmd, "expected": expected, "actual": checksum}
                args.output.write_text(json.dumps(data, indent=2) + "\n")
                raise RuntimeError("checksum mismatch")
            print(n, q, shape, kind, mode, run, name, construction, operations, flush=True)
            if run >= 0:
                data["samples"].append({"n": n, "q": q, "shape": shape, "type": kind, "mode": mode, "run": run + 1,
                    "variant": name, "build_ms": float(construction), "operations_ms": float(operations),
                    "rss_kib": int(p.stderr.strip().splitlines()[-1]), "checksum": int(checksum)})
                args.output.write_text(json.dumps(data, indent=2) + "\n")
