#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <utility>
#include <vector>
#define kth_root_integer kth_root_integer_old
#include "baseline.hpp"
#undef kth_root_integer
#include "candidate.hpp"
#if defined(__SIZEOF_INT128__)
#include "nyaan.hpp"
#endif
using U = std::uint64_t;
U nyaan(U n, int k) {
#if defined(__SIZEOF_INT128__)
  return kth_root_integral(n, static_cast<U>(k));
#else
  return blueberry::kth_root_integer_old(n,k);
#endif
}
int main(int argc, char** argv) {
  if (argc != 4) return 2;
  const int which = std::atoi(argv[1]), shape = std::atoi(argv[2]), count = std::atoi(argv[3]);
  U (*fn[])(U,int) = {blueberry::kth_root_integer_old, blueberry::kth_root_integer, nyaan};
  std::mt19937_64 rng(20261118 + shape);
  std::vector<std::pair<U,int>> input(count);
  constexpr int small_k[] = {2,3,4,5,7,8,9};
  for (int i = 0; i < count; ++i) {
    U n = rng();
    int k = shape <= 6 ? small_k[shape] : shape == 7 ? 16 : shape == 8 ? 63 : shape == 9 ? 1 + int(rng() % 65) : 2 + int(rng() % 15);
    if (shape == 10) n = rng() & ((U{1} << 32) - 1);
    input[i] = {n,k};
  }
  U checksum = 0;
  auto start = std::chrono::steady_clock::now();
  for (auto [n,k] : input) checksum += fn[which](n,k);
  auto end = std::chrono::steady_clock::now();
  std::printf("%d %d %.6f %llu\n",which,shape,std::chrono::duration<double,std::milli>(end-start).count(),static_cast<unsigned long long>(checksum));
}
