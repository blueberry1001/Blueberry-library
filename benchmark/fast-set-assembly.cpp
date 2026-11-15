#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "fast-set-assembly.hpp"

struct Operation { int type, x; };

int main(int argc, char** argv) {
  const int n = argc > 1 ? std::atoi(argv[1]) : 10000000;
  const int q = argc > 2 ? std::atoi(argv[2]) : 600000;
  const std::string shape = argc > 3 ? argv[3] : "half";
  std::mt19937_64 rng(1729);
  std::string bits(n, '0');
  for (int i = 0; i < n; ++i) {
    if (shape == "full") bits[i] = '1';
    else if (shape == "half") bits[i] += rng() & 1;
    else if (shape == "sparse") bits[i] += rng() % 1000 == 0;
    else if (shape == "cluster") bits[i] += (i >> 8) & 1;
  }
  std::vector<Operation> ops(q);
  for (auto& op : ops) op = {static_cast<int>(rng() % 5), static_cast<int>(rng() % n)};
  using Clock = std::chrono::steady_clock;
  const auto t0 = Clock::now();
  blueberry::FastSet tree(bits);
  const auto t1 = Clock::now();
  std::uint64_t checksum = tree.size();
  for (const auto op : ops) {
    if (op.type == 0) checksum += tree.insert(op.x);
    if (op.type == 1) checksum += tree.erase(op.x);
    if (op.type == 2) checksum += tree.contains(op.x);
    if (op.type == 3) checksum += static_cast<unsigned>(tree.next(op.x));
    if (op.type == 4) checksum += static_cast<unsigned>(tree.prev(op.x) + 1);
  }
  const auto t2 = Clock::now();
  std::cout << "{\"shape\":\"" << shape << "\",\"n\":" << n << ",\"q\":" << q
            << ",\"build_ms\":" << std::chrono::duration<double, std::milli>(t1 - t0).count()
            << ",\"operations_ms\":" << std::chrono::duration<double, std::milli>(t2 - t1).count()
            << ",\"checksum\":" << checksum << "}\n";
}
