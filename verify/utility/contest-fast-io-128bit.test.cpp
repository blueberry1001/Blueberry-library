#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb_128bit"
#include "blueberry/utility/contest-fast-io.hpp"
int main() {
  int t;
  if (!scan(t)) return 1;
  while (t--) {
    __int128 a, b;
    if (!scan(a, b) || !print(a + b)) return 1;
  }
  return flush() ? 0 : 1;
}
