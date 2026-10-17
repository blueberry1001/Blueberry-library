#include <iostream>
#include <string>
#include <atcoder/modint>
#include "blueberry/math/bernoulli.hpp"
#include "blueberry/math/bell.hpp"
#include "blueberry/math/partition.hpp"
#include "blueberry/utility/fast-io.hpp"

int main(int argc, char** argv) {
  if (argc != 2) return 2;
  int n;
#ifdef USE_FAST_IO
  blueberry::FastInput in;
  blueberry::FastOutput out;
  if (!in.read(n)) return 1;
#else
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cin >> n;
#endif
  using Mint = atcoder::modint998244353;
  const std::string kind = argv[1];
  auto a = kind == "bernoulli" ? blueberry::bernoulli<Mint>(n)
         : kind == "bell" ? blueberry::bell<Mint>(n) : blueberry::partition<Mint>(n);
  for (int i = 0; i <= n; ++i) {
#ifdef USE_FAST_IO
    if (i) out.write(' ');
    out.write(a[i].val());
#else
    std::cout << (i ? " " : "") << a[i].val();
#endif
  }
#ifdef USE_FAST_IO
  out.write('\n');
#else
  std::cout << '\n';
#endif
}
