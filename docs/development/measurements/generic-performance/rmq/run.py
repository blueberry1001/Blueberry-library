#!/usr/bin/env python3
"""Run original and candidate headers serially; never overwrite evidence."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[5]
SOURCE = Path(__file__).with_name("benchmark.cpp")
HEADERS = [f"blueberry/data-structure/{name}.hpp" for name in
           ("sqrt-tree", "disjoint-sparse-table", "linear-rmq")]

def command(args):
    return subprocess.run(args, cwd=ROOT, text=True, capture_output=True, check=True)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--structure", choices=("all","sqrt","dst","linear"), default="all")
    parser.add_argument("--sqrt-recheck", action="store_true")
    parser.add_argument("--last-recheck", action="store_true")
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    baseline = ROOT / ".build/perf-baseline"
    # One fixed available logical CPU, before/after alternated within each pair.
    core = min(os.sched_getaffinity(0))
    env = {"baseline_revision": "e0d8e71", "head_revision": command(["git","rev-parse","HEAD"]).stdout.strip(),
           "platform": platform.platform(), "cpu": command(["lscpu"]).stdout,
           "logical_cpu": core, "seed": 718239, "repeats": 5, "warmups": 1,
           "harness_sha256": sha(SOURCE), "sources": {}, "configurations": []}
    for variant, directory in (("before",baseline),("after",ROOT)):
        env["sources"][variant] = {p:sha(directory/p) for p in HEADERS}
    configurations = [("gcc-release","g++",["-O2","-DNDEBUG"]),
                      ("clang-release","clang++",["-O2","-DNDEBUG"]),
                      ("gcc-assert","g++",["-O2"])]
    rows=[]
    with (out/"samples.csv").open("w", newline="") as file:
        fields=["config","structure","algebra","n","q","distribution","variant","run","build_ms","query_ms","checksum","peak_rss_kib"]
        writer=csv.DictWriter(file,fieldnames=fields); writer.writeheader(); file.flush()
        for name, compiler, flags in configurations:
            cfg={"name":name,"compiler":command([compiler,"--version"]).stdout,"flags":["-std=gnu++20",*flags]}
            env["configurations"].append(cfg)
            binaries={}
            for variant,directory in (("before",baseline),("after",ROOT)):
                binary=out/f"{name}-{variant}"
                compiled=command([compiler,*cfg["flags"],"-I"+str(directory),str(SOURCE),"-o",str(binary)])
                (out/f"compile-{name}-{variant}.log").write_text(compiled.stdout+compiled.stderr)
                binaries[variant]=binary
            (out/"environment.json").write_text(json.dumps(env,indent=2))
            cases=[]
            for n in (500000,524289):
                for distribution in ("random","short","equal","ascending","descending"):
                    if name=="gcc-assert" and distribution!="random": continue
                    cases.append(("linear","min",n,500000,distribution))
                for structure in ("sqrt","dst"):
                    for algebra in ("min","sum","affine","matrix"):
                        if name=="gcc-assert" and algebra not in ("min","affine"): continue
                        cases.append((structure,algebra,n,500000,"random"))
                        if name=="gcc-release":
                            cases.append((structure,algebra,n,500000,"short"))
                            cases.append((structure,algebra,n,100,"random"))
            if args.sqrt_recheck:
                cases = [("sqrt",algebra,524289,q,"random")
                         for algebra in ("min","sum","affine","matrix")
                         for q in (500000,5000000)]
                cases += [("sqrt","affine",524289,5000000,"short"),
                          ("sqrt","affine",500000,500000,"random"),
                          ("sqrt","affine",500000,5000000,"random")]
            if args.last_recheck:
                cases = [("sqrt",algebra,n,q,"random")
                         for algebra,n in (("matrix",500000),("min",524289))
                         for q in (500000,5000000)]
            for case in cases:
                if args.structure != "all" and case[0] != args.structure: continue
                expected=None
                for run in range(6):
                    for variant in (("before","after") if run%2==0 else ("after","before")):
                        result=command(["taskset","-c",str(core),"/usr/bin/time","-f","%M",str(binaries[variant]),*map(str,case)])
                        build,query,checksum=result.stdout.strip().split(",")
                        if expected is None: expected=checksum
                        if checksum!=expected: raise RuntimeError(f"checksum mismatch {name} {case} {variant}")
                        row=dict(zip(fields,[name,*case,variant,run,float(build),float(query),checksum,int(result.stderr.strip())]))
                        writer.writerow(row); file.flush(); rows.append(row)
                print(name,*case,"PASS",flush=True)
    summaries=[]
    groups={}
    for row in rows:
        if row["run"]==0: continue
        key=tuple(row[x] for x in fields[:7])
        groups.setdefault(key,[]).append(row)
    for key,items in groups.items():
        summary=dict(zip(fields[:7],key))
        for metric in ("build_ms","query_ms","peak_rss_kib"):
            summary[metric+"_median"]=statistics.median(x[metric] for x in items)
            summary[metric+"_min"]=min(x[metric] for x in items)
            summary[metric+"_max"]=max(x[metric] for x in items)
        summaries.append(summary)
    (out/"summary.json").write_text(json.dumps(summaries,indent=2))
    for variant,directory in (("before",baseline),("after",ROOT)):
        assert env["sources"][variant]=={p:sha(directory/p) for p in HEADERS}, "sources changed during measurement"
    print("PASS",len(rows),"samples including warmups",flush=True)

if __name__=="__main__":
    main()
