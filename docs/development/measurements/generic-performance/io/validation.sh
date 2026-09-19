#!/usr/bin/env bash
set -euo pipefail
mkdir -p .build/fast-io-check
for compiler in g++ clang++; do
  for standard in gnu++20 gnu++23; do
    binary=".build/fast-io-check/${compiler}-${standard}"
    "$compiler" "-std=$standard" -O2 -Wall -Wextra -Wshadow -Werror -I. tests/random/fast-io.cpp -o "$binary"
    for seed in 1 2 3 4 5; do "$binary" "$seed"; done
    printf 'PASS %s %s seeds 1..5\n' "$compiler" "$standard"
  done
done
clang++ -std=gnu++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I. tests/random/fast-io.cpp -o .build/fast-io-check/sanitizer
.build/fast-io-check/sanitizer 17
printf 'PASS ASan/UBSan seed 17\n'
