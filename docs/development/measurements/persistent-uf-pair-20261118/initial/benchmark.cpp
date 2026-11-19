#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/persistent-union-find.hpp"
using namespace std;
using Clock = chrono::steady_clock;
struct Merge { int version, a, b; };
struct Query { int version, a, b; };
int main(int argc, char** argv) {
  if (argc != 5) return 2;
  string mode = argv[1];
  int n = atoi(argv[2]), count = atoi(argv[3]);
  uint64_t seed = strtoull(argv[4], nullptr, 10);
  if (n < 2 || count < 1 || (mode != "chain" && mode != "branch" && mode != "self" && mode != "deep")) return 2;
  mt19937_64 rng(seed);
  vector<Merge> merges;
  merges.reserve(count + n);
  if (mode == "deep") {
    for (int width = 1; width < n; width *= 2) {
      for (int a = 0; a + width < n; a += 2 * width) merges.push_back({static_cast<int>(merges.size()), a, a + width});
      if (width > n / 2) break;
    }
  } else {
    for (int i = 0; i < count; ++i) {
      int v = mode == "branch" ? static_cast<int>(rng() % (i + 1)) : i;
      int a = rng() % n, b = mode == "self" ? a : rng() % n;
      merges.push_back({v, a, b});
    }
  }
  vector<Query> queries;
  queries.reserve(count);
  for (int i = 0; i < count; ++i) {
    int v = mode == "branch" ? rng() % (merges.size() + 1) : static_cast<int>(merges.size());
    queries.push_back({v, static_cast<int>(rng() % n), static_cast<int>(rng() % n)});
  }
  auto t0 = Clock::now();
  blueberry::PersistentUnionFind uf(n);
  auto t1 = Clock::now();
  uint64_t checksum = 0;
  for (auto x : merges) checksum += uf.merge(x.version, x.a, x.b);
  auto t2 = Clock::now();
  for (auto x : queries) checksum += uf.same(x.version, x.a, x.b);
  auto t3 = Clock::now();
  auto ms = [](auto a, auto b) { return chrono::duration<double, milli>(b - a).count(); };
  cout << "{\"construct_ms\":" << ms(t0, t1) << ",\"merge_ms\":" << ms(t1, t2)
       << ",\"same_ms\":" << ms(t2, t3) << ",\"checksum\":" << checksum
       << ",\"versions\":" << uf.versions() << ",\"merges\":" << merges.size() << "}\n";
}
