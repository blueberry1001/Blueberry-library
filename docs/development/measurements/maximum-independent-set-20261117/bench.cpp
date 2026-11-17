#include <algorithm>
#include <bit>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <utility>
#include <vector>
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
using namespace std;
#ifdef NYAAN
#include "nyaan.hpp"
static vector<int> solve(int n, const vector<pair<int, int>>& e) {
  vector<vector<int>> g(n);
  for (auto [a, b] : e) { g[a].push_back(b); g[b].push_back(a); }
  return max_independent_set<40>(g);
}
#else
#include "candidate.hpp"
static vector<int> solve(int n, const vector<pair<int, int>>& e) { return blueberry::maximum_independent_set(n, e); }
#endif
using Graph = vector<pair<int, int>>;
static Graph make_graph(int n, int shape, mt19937_64& rng) {
  Graph edges;
  for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) {
    bool edge = false;
    if (shape == 0) edge = false;
    else if (shape == 1) edge = true;
    else if (shape == 2) edge = v == u + 1;
    else if (shape == 3) edge = u == 0;
    else if (shape == 4) edge = u / 3 == v / 3;
    else if (shape == 5) edge = (u < n / 2) != (v < n / 2);
    else if (shape == 6) edge = rng() % 100 < 5;
    else if (shape == 7) edge = rng() % 100 < 20;
    else if (shape == 8) edge = rng() % 100 < 50;
    else if (shape == 9) edge = rng() % 100 < 80;
    else if (shape == 10) edge = rng() % 100 < 95;
    else if (shape == 11) edge = u / 5 == v / 5;
    else if (shape == 12) edge = u / 5 == v / 5 || (u % 5 == 0 && v == u + 5);
    if (edge) edges.emplace_back(u, v);
  }
  return edges;
}
int main(int argc, char** argv) {
  if (argc != 4) return 2;
  int n = atoi(argv[1]), shape = atoi(argv[2]), count = atoi(argv[3]);
  mt19937_64 rng(0x2b7e151628aed2a6ULL + n * 100 + shape);
  vector<Graph> graphs(count);
  for (auto& graph : graphs) graph = make_graph(n, shape, rng);
  vector<vector<int>> results(count);
  auto start = chrono::steady_clock::now();
  for (int i = 0; i < count; ++i) results[i] = solve(n, graphs[i]);
  auto stop = chrono::steady_clock::now();
  uint64_t checksum = 0;
  for (int i = 0; i < count; ++i) {
    const auto& result = results[i];
    checksum = checksum * 1000003 + result.size();
    uint64_t selected = 0;
    for (int v : result) {
      if (v < 0 || v >= n || (selected >> v & 1)) return 3;
      selected |= uint64_t{1} << v;
    }
    for (auto [u, v] : graphs[i]) if ((selected >> u & 1) && (selected >> v & 1)) return 4;
  }
  PROCESS_MEMORY_COUNTERS mem{};
  if (!GetProcessMemoryInfo(GetCurrentProcess(), &mem, sizeof(mem))) return 5;
  printf("%llu %.6f %zu\n", static_cast<unsigned long long>(checksum), chrono::duration<double, milli>(stop - start).count(), static_cast<size_t>(mem.PeakWorkingSetSize));
}
