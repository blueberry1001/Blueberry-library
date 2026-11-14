// Reproduction of the sparse 1D prefix-count workload seen in ABC484 E.
// Nyaan reference: https://github.com/NyaanNyaan/library (CC0-1.0).
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#if defined(NYAAN)
using namespace std;  // Nyaan's header expects the standard namespace imported.
#include "dynamic-binary-indexed-tree.hpp"
using Tree = DynamicFenwickTree<long long, long long>;
#else
#include "dynamic-fenwick-tree.hpp"
using Tree = blueberry::DynamicFenwickTree<long long>;
#endif

struct Query { long long p, l, r; };
constexpr long long n = 3000000000000000000LL;
constexpr long long offset = n / 2;

long long range_sum(const Tree& tree, long long l, long long r) {
#if defined(NYAAN)
  return tree.sum(l, r);
#else
  return tree.sum(l, r);
#endif
}

int main(int argc, char** argv) {
  const int count = argc > 1 ? std::atoi(argv[1]) : 300000;
  const std::string shape = argc > 2 ? argv[2] : "walk";
  std::mt19937_64 rng(1729);
  std::vector<Query> queries;
  queries.reserve(count);
  long long now = 0;
  for (int i = 0; i < count; ++i) {
    if (shape == "walk") now += static_cast<long long>(rng() % 2001) - 1000;
    else if (shape == "diffuse") now += static_cast<long long>(rng() % 2000000000001ULL) - 1000000000000LL;
    else if (shape == "wide") now = static_cast<long long>(rng() % 2000000000000001ULL) - 1000000000000000LL;
    else now += 1;
    long long scale = shape == "diffuse" || shape == "wide" ? 10000000000000LL : 2000000LL;
    long long left = static_cast<long long>(rng() % scale);
    long long right = left + static_cast<long long>(rng() % scale);
    queries.push_back({now + offset, now - right + offset, now - left + offset + 1});
  }
  auto start = std::chrono::steady_clock::now();
  Tree tree(n);
  long long checksum = 0;
  for (const auto& q : queries) {
    tree.add(q.p, 1);
    checksum += range_sum(tree, q.l, q.r);
  }
  auto finish = std::chrono::steady_clock::now();
  std::cout << shape << ' ' << count << ' ' << checksum << ' '
            << std::chrono::duration<double, std::milli>(finish - start).count() << '\n';
}
