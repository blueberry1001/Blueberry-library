#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb_128bit"
#include "blueberry/utility/contest-template.hpp"
int main() {
  init_io();
  int t;
  scan(t);
  // Imported stream operators retain the full signed 128-bit range.
  rep(t) {
    __int128 a, b;
    scan(a, b);
    print(a + b);
  }
}
