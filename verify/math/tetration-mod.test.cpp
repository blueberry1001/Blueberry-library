#define PROBLEM "https://judge.yosupo.jp/problem/tetration_mod"
#include <cstdint>
#include "blueberry/math/tetration-mod.hpp"
#include "blueberry/utility/fast-io.hpp"

int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int t = 0;
  if (!in.read(t)) return 0;
  while (t--) {
    std::uint64_t a = 0, b = 0;
    std::uint32_t m = 1;
    in.read(a, b, m);
    // Non-coprime bases, height zero, and 0^0 follow the official convention.
    out.writeln(blueberry::tetration_mod(a, b, m));
  }
}
