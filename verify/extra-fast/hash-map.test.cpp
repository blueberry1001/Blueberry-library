#define PROBLEM "https://judge.yosupo.jp/problem/associative_array"
#include "blueberry/extra-fast/hash-map.hpp"

#include <cstdint>

#include "blueberry/utility/fast-io.hpp"

// SIMD tag matching with dynamic capacity; missing reads do not insert.
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int q;
  in.read(q);
  blueberry::extra_fast::HashMap<> map;
  while (q--) {
    int t;
    std::uint64_t k;
    in.read(t, k);
    if (t == 0) {
      std::uint64_t v;
      in.read(v);
      map.insert_or_assign(k, v);
    } else
      out.writeln(map.get(k).value_or(0));
  }
}
