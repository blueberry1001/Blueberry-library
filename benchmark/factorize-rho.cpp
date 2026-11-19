#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>
#define blueberry blueberry_baseline
#include "factorize-rho-baseline.hpp.txt"
#undef blueberry
#include "../blueberry/math/factorize.hpp"
using namespace std;
int main(int argc, char** argv) {
  vector<uint64_t> nums = {1000000016000000063ULL, 1000000040000000231ULL, 1000000030000000189ULL, 4611685975477714963ULL, 18446743979220271189ULL, 18446744030759878681ULL, 999999943999999559ULL, 999999874000003969ULL};
  uint64_t checksum = 0;
  auto start = chrono::steady_clock::now();
  for (int rep = 0; rep < 100; ++rep) for (auto n : nums) {
    auto f = argc > 1 ? blueberry::factorize(n) : blueberry_baseline::factorize(n);
    for (auto p : f) checksum ^= p + (checksum << 7) + (checksum >> 3);
  }
  cout << chrono::duration<double>(chrono::steady_clock::now() - start).count() << " " << checksum << '\n';
}
