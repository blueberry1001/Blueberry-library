#include <chrono>
#include <array>
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
  uint64_t success = 0, same = 0, old_nodes = 0, new_nodes = 0;
  array<uint64_t, 64> shared_hist{};
  auto path_length = [&](int p) {
    int l = 0, r = n, depth = 1;
    while (r - l > 1) {
      int m = l + (r - l) / 2;
      if (p < m) r = m; else l = m;
      ++depth;
    }
    return depth;
  };
  for (auto x : merges) {
    int a = uf.leader(x.version, x.a), b = uf.leader(x.version, x.b);
    if (a == b) ++same;
    else {
      ++success;
      int l = 0, r = n, shared = 0;
      while (r - l > 1) {
        ++shared;
        int m = l + (r - l) / 2;
        if (a < m && b < m) r = m;
        else if (a >= m && b >= m) l = m;
        else break;
      }
      ++shared_hist[shared];
      auto old = path_length(a) + path_length(b);
      old_nodes += old;
      new_nodes += old - shared;
    }
    checksum += uf.merge(x.version, x.a, x.b);
  }
  auto t2 = Clock::now();
  for (auto x : queries) checksum += uf.same(x.version, x.a, x.b);
  auto t3 = Clock::now();
  auto ms = [](auto a, auto b) { return chrono::duration<double, milli>(b - a).count(); };
  (void)t0; (void)t1; (void)t2; (void)t3; (void)ms;
  cout << "{\"success\":" << success << ",\"same\":" << same
       << ",\"expected_old_nodes\":" << old_nodes << ",\"expected_new_nodes\":" << new_nodes
       << ",\"observed_nodes\":" << blueberry::persistent_array_append_count
       << ",\"checksum\":" << checksum << ",\"versions\":" << uf.versions()
       << ",\"merges\":" << merges.size() << ",\"shared_histogram\":[";
  for (int i = 0; i < 64; ++i) cout << (i ? "," : "") << shared_hist[i];
  cout << "]}\n";
}
