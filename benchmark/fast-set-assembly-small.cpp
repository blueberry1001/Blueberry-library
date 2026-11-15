#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include "fast-set-assembly.hpp"

int main(int argc, char** argv) {
  const int n = std::atoi(argv[1]);
  const int repetitions = std::atoi(argv[2]);
  std::string bits(n, '0');
  for (int i = 0; i < n; ++i) bits[i] += i % 2;
  std::uint64_t checksum = 0;
  const auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < repetitions; ++i) {
    blueberry::FastSet tree(bits);
    checksum += tree.size() + tree.next(i % (n + 1));
  }
  const auto end = std::chrono::steady_clock::now();
  std::cout << "{\"n\":" << n << ",\"repetitions\":" << repetitions << ",\"milliseconds\":"
            << std::chrono::duration<double, std::milli>(end - start).count()
            << ",\"checksum\":" << checksum << "}\n";
}
