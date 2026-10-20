#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"
mkdir -p .build/io-tests/final
printf 'UTC validation time: '
date -u +%Y-%m-%dT%H:%M:%SZ
git rev-parse HEAD
g++ --version
clang++ --version
sha256sum blueberry/utility/fast-io.hpp tests/random/fast-io.cpp
set -x
for compiler in g++ clang++; do
  for standard in gnu++20 gnu++23; do
    output=.build/io-tests/final/fast-io-$compiler-$standard
    "$compiler" -std=$standard -O2 -Wall -Wextra -Wshadow -Werror -I. tests/random/fast-io.cpp -o "$output"
    for seed in {1..20}; do "$output" "$seed"; done
    echo PASS "$compiler" "$standard" seeds-1-through-20
  done
done
g++ -std=gnu++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wshadow -Werror -I. tests/random/fast-io.cpp -o .build/io-tests/final/fast-io-sanitized
.build/io-tests/final/fast-io-sanitized 1
echo PASS ASan-UBSan seed-1-including-fixed-refill-boundaries
set +x
sha256sum blueberry/utility/fast-io.hpp tests/random/fast-io.cpp
echo 'All final fast I/O integer validation checks passed.'
