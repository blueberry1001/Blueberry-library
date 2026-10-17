#define PROBLEM "https://judge.yosupo.jp/problem/bell_number"
#include "blueberry/utility/fast-io.hpp"
#include <atcoder/modint>
#include "blueberry/math/bell.hpp"

// Verify every term, including the zero-th term, with the official checker.
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n;
  if (!in.read(n)) return 0;
  auto a = blueberry::bell<atcoder::modint998244353>(n);
  for (int i = 0; i <= n; ++i) {
    if (i) out.write(' ');
    out.write(a[i].val());
  }
  out.write('\n');
}
