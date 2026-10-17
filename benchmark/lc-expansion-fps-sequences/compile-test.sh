#!/usr/bin/env bash
set -euo pipefail
mkdir -p .build/fps-sequences
for cxx in g++ clang++; do
  "$cxx" --version | head -1
  for standard in gnu++20 gnu++23; do
    echo "$cxx $standard -O2 -Wall -Wextra -Wshadow -Werror"
    "$cxx" -std="$standard" -O2 -Wall -Wextra -Wshadow -Werror -iquote . -isystem .deps/ac-library \
      tests/random/fps_sequences.cpp -o .build/fps-sequences/random
    .build/fps-sequences/random 20261017
    .build/fps-sequences/random 731
    for name in bernoulli bell partition; do
      printf '#include "blueberry/math/%s.hpp"\n#include "blueberry/math/%s.hpp"\nint main() {}\n' "$name" "$name" > .build/fps-sequences/include.cpp
      "$cxx" -std="$standard" -O2 -DNDEBUG -Wall -Wextra -Wshadow -Werror -iquote . -isystem .deps/ac-library \
        .build/fps-sequences/include.cpp -o .build/fps-sequences/include
      "$cxx" -std="$standard" -O2 -Wall -Wextra -Wshadow -Werror -iquote . -isystem .deps/ac-library \
        "verify/math/$name.test.cpp" -o ".build/fps-sequences/$name"
    done
  done
done
