#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <utility>
#include <vector>
#include "candidate.hpp"
using namespace std;
static void check(int n, const vector<pair<int, int>>& edges, int expected) {
  auto a = blueberry::maximum_independent_set(n, edges);
  if (static_cast<int>(a.size()) != expected) {
    fprintf(stderr, "n=%d expected=%d actual=%zu\n", n, expected, a.size());
    for (auto [u, v] : edges) fprintf(stderr, "%d %d\n", u, v);
    abort();
  }
  assert(is_sorted(a.begin(), a.end()));
  uint64_t selected = 0;
  for (int v : a) { assert(0 <= v && v < n && ((selected >> v) & 1) == 0); selected |= uint64_t{1} << v; }
  for (auto [u, v] : edges) assert(!((selected >> u & 1) && (selected >> v & 1)));
}
int main(int argc, char** argv) {
  uint64_t seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  fprintf(stderr, "seed=%llu\n", static_cast<unsigned long long>(seed));
  mt19937_64 rng(seed);
  for (int n = 0; n <= 19; ++n) for (int t = 0; t < 20; ++t) {
    int density = t * 5;
    vector<pair<int, int>> edges;
    vector<unsigned> adj(n);
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) if (rng() % 100 < density) {
      edges.emplace_back(u, v); adj[u] |= 1u << v; adj[v] |= 1u << u;
    }
    vector<unsigned char> good(size_t{1} << n); good[0] = 1;
    int expected = 0;
    for (unsigned s = 1; s < (1u << n); ++s) {
      int v = countr_zero(s); unsigned rest = s & (s - 1);
      good[s] = good[rest] && !(adj[v] & rest);
      if (good[s]) expected = max(expected, popcount(s));
    }
    check(n, edges, expected);
  }
  vector<pair<int, int>> edges;
  check(40, edges, 40);
  for (int u = 0; u < 40; ++u) for (int v = u + 1; v < 40; ++v) edges.emplace_back(u, v);
  check(40, edges, 1);
  edges.clear();
  for (int u = 0; u < 40; u += 2) edges.emplace_back(u, u + 1);
  check(40, edges, 20);
  edges.clear();
  for (int u = 0; u < 40; ++u) edges.emplace_back(u, (u + 1) % 40);
  check(40, edges, 20);
  edges.clear();
  for (int i = 0; i < 39; i += 3) { edges.emplace_back(i, i + 1); edges.emplace_back(i + 1, i + 2); edges.emplace_back(i + 2, i); }
  check(40, edges, 14);
  edges.clear();
  for (int i = 0; i < 20; ++i) for (int j = 20; j < 40; ++j) edges.emplace_back(i, j);
  check(40, edges, 20);
  edges.clear();
  for (int i = 0; i < 39; ++i) edges.emplace_back(i, (i + 1) % 39);
  check(40, edges, 20);
  edges.clear();
  for (int g = 0; g < 8; ++g) for (int i = 0; i < 5; ++i) for (int j = i + 1; j < 5; ++j) edges.emplace_back(5 * g + i, 5 * g + j);
  check(40, edges, 8);
  edges.clear();
  for (int g = 0; g < 4; ++g) for (int i = 0; i < 5; ++i) edges.emplace_back(5 * g + i, 5 * g + (i + 1) % 5);
  for (int g = 0; g < 3; ++g) for (int i = 0; i < 6; ++i) edges.emplace_back(20 + 6 * g + i, 20 + 6 * g + (i + 1) % 6);
  check(40, edges, 19);
  printf("passed seed=%llu\n", static_cast<unsigned long long>(seed));
}
