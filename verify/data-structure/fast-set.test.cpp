#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include "blueberry/utility/fast-io.hpp"
#include <string>
#include "blueberry/data-structure/fast-set.hpp"
int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q; std::string bits; in.read(n, q, bits);
  blueberry::FastSet set(bits);
  while (q--) {
    int t, x; in.read(t, x);
    if (t == 0) set.insert(x);
    if (t == 1) set.erase(x);
    if (t == 2) out.writeln(set.contains(x));
    if (t == 3) { int y = set.next(x); out.writeln(y == n ? -1 : y); }
    if (t == 4) out.writeln(set.prev(x));
  }
}
