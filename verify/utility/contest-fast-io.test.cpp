#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb"
#include "blueberry/utility/contest-fast-io.hpp"
int main() {
  // Exercise the same declarations and output shorthand as contest-template.hpp.
  LL(t);
  while (t--) {
    LL(a, b);
    if (!O(a + b)) return 1;
  }
  // Normal termination must flush the facade's pending output.
  return 0;
}
