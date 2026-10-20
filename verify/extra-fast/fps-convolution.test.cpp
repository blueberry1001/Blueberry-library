#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"

#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/formal-power-series.hpp"
#include "blueberry/utility/fast-io.hpp"

using namespace std;

// 同じSIMD NTTの積を、FPSのNewton反復とは独立に検証する。
int main() {
  blueberry::FastInput input;
  blueberry::FastOutput output;
  int n, m;
  if (!input.read(n, m)) return 0;
  vector<uint32_t> a(n), b(m);
  for (auto& x : a) input.read(x);
  for (auto& x : b) input.read(x);
  auto c = blueberry::extra_fast::FPS998244353().convolution(a, b);
  for (int i = 0; i < int(c.size()); ++i) {
    output.write(c[i]);
    output.write(i + 1 == int(c.size()) ? '\n' : ' ');
  }
}
