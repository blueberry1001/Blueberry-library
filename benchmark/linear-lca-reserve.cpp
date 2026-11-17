#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/graph/linear-lca.hpp"
using namespace std;
using clock_type = chrono::steady_clock;
int main(int argc, char** argv) {
  if (argc != 3) return 1;
  int n = atoi(argv[1]);
  string shape = argv[2];
  mt19937 gen(1729);
  vector<vector<int>> tree(n);
  for (int v = 1; v < n; ++v) {
    int p = shape == "star" ? 0 : shape == "path" ? v - 1 : gen() % v;
    tree[p].push_back(v); tree[v].push_back(p);
  }
  vector<pair<int, int>> queries(200000);
  for (auto& [u, v] : queries) { u = gen() % n; v = gen() % n; }
  auto start = clock_type::now();
  blueberry::LinearLCA lca(tree);
  auto built = clock_type::now();
  uint64_t checksum = 0;
  for (auto [u, v] : queries) checksum = checksum * 1000003 + lca.lca(u, v);
  auto finished = clock_type::now();
  cout << checksum << ' ' << chrono::duration<double, milli>(built - start).count() << ' ' << chrono::duration<double, milli>(finished - built).count() << '\n';
}
