#define PROBLEM "https://judge.yosupo.jp/problem/log_of_formal_power_series"

#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/formal-power-series.hpp"
#include "blueberry/utility/fast-io.hpp"

using namespace std;

// 998244353専用のMontgomery NTTと級数演算を検証する。
int main() {
  blueberry::FastInput input;
  blueberry::FastOutput output;
  int n;
  if (!input.read(n)) return 0;
  vector<uint32_t> f(n);
  for (auto& x : f) input.read(x);
  blueberry::extra_fast::FPS998244353 fps;
  auto g = fps.log(f);
  for (int i = 0; i < n; ++i) {
    output.write(g[i]);
    output.write(i + 1 == n ? '\n' : ' ');
  }
}
