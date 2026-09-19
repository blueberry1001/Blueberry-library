#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb"
#include "blueberry/utility/fast-io.hpp"
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int t;
  if (!in.read(t)) return 1;
  while (t--) {
    long long a, b;
    if (!in.read(a, b) || !out.writeln(a + b)) return 1;
  }
  return out.flush() ? 0 : 1;
}
