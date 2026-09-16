#!/usr/bin/env bash
# Run when other verification/benchmark jobs are idle. Results are baselines,
# not a performance comparison against the external recursive implementations.
set -eu
out=${1:-.benchmark/results/graph-additions}
mkdir -p "$out"
compiler=${CXX:-g++}
{
  "$compiler" --version
  uname -a
  lscpu
  printf 'flags=-std=gnu++20 -O2 -I.\n'
} > "$out/environment.txt"
"$compiler" -std=gnu++20 -O2 -I. benchmark/graph-additions.cpp -o "$out/graph-additions"
for algorithm in lowlink rerooting; do
  for shape in path star random; do
    /usr/bin/time -v "$out/graph-additions" "$algorithm" "$shape" 200000 3 \
      > "$out/$algorithm-$shape.txt" 2> "$out/$algorithm-$shape-memory.txt"
  done
done
