#!/usr/bin/env bash
# Rerun the changed facade after caching one singleton reference per call.
set -euo pipefail
root=$(git rev-parse --show-toplevel)
cd "$root"
acl=${ACL_ROOT:-.deps/ac-library}
output_dir=.build/io-tests/current
mkdir -p "$output_dir"
printf 'UTC validation time: '
date -u +%Y-%m-%dT%H:%M:%SZ
printf 'Base commit: '
git rev-parse HEAD
uname -a
g++ --version
clang++ --version
sources=(
  blueberry/utility/contest-fast-io.hpp
  blueberry/utility/fast-io.hpp
  blueberry/utility/internal/contest-template-base.hpp
  tests/random/contest-fast-io.cpp
  verify/utility/contest-fast-io.test.cpp
  verify/utility/contest-fast-io-128bit.test.cpp
)
for source in "${sources[@]}"; do
  printf 'Git blob %s: ' "$source"
  git hash-object "$source"
done
sha256sum "${sources[@]}"
set -x
for compiler in g++ clang++; do
  output="$output_dir/random-$compiler"
  "$compiler" -std=gnu++20 -O2 -Wall -Wextra -Wshadow -Werror -I. -isystem "$acl" \
    tests/random/contest-fast-io.cpp -o "$output"
  for seed in {1..20}; do "$output" "$seed"; done
  echo PASS "$compiler" gnu++20 random-seeds-1-through-20
done
for source in verify/utility/contest-fast-io.test.cpp verify/utility/contest-fast-io-128bit.test.cpp; do
  output="$output_dir/$(basename "$source")"
  g++ -std=gnu++20 -O2 -Wall -Wextra -Wshadow -Werror -I. -isystem "$acl" "$source" -o "$output"
  printf '3\n1 2\n-8 3\n0 0' | "$output" > "$output.actual"
  diff -u <(printf '3\n-5\n0\n') "$output.actual"
  echo PASS "$source" sample-without-final-newline
done
g++ -std=gnu++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Wall -Wextra -Wshadow -Werror -I. -isystem "$acl" \
  tests/random/contest-fast-io.cpp -o "$output_dir/sanitized"
"$output_dir/sanitized" 42
echo PASS ASan-UBSan seed-42
set +x
echo 'All targeted current-snapshot validations passed.'
