#!/usr/bin/env bash
set -euo pipefail
mkdir -p .build/sequence-check
for compiler in g++ clang++; do
  for standard in gnu++20 gnu++23; do
    for test in specialized-structures wavelet-matrix static-range-lis; do
      binary=".build/sequence-check/${test}-${compiler}-${standard}"
      "$compiler" "-std=$standard" -O2 -Wall -Wextra -Wshadow -Werror -I. -isystem .deps/ac-library "tests/random/$test.cpp" -o "$binary"
      for seed in 1 2 3; do "$binary" "$seed"; done
      printf 'PASS %s %s %s seeds 1..3\n' "$compiler" "$standard" "$test"
    done
  done
done
for test in specialized-structures wavelet-matrix static-range-lis; do
  binary=".build/sequence-check/${test}-sanitizer"
  clang++ -std=gnu++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I. -isystem .deps/ac-library "tests/random/$test.cpp" -o "$binary"
  "$binary" 17
  printf 'PASS sanitizer %s seed 17\n' "$test"
done
