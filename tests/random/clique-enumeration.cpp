#include <algorithm>
#include <bit>
#include <cassert>
#include <random>
#include <stdexcept>
#include <vector>
#include "blueberry/graph/clique-enumeration.hpp"
using namespace std;
using Edges = vector<pair<int, int>>;
void check(int n, const Edges& edges) {
  vector<unsigned> got;
  blueberry::enumerate_cliques(n, edges, [&](const vector<int>& c) {
    unsigned mask = 0; for (int v : c) mask |= 1u << v;
    assert(popcount(mask) == int(c.size())); got.push_back(mask);
  });
  sort(got.begin(), got.end()); assert(unique(got.begin(), got.end()) == got.end());
  vector<unsigned> expected;
  for (unsigned mask = 1; mask < (1u << n); ++mask) {
    bool clique = true;
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
      if ((mask >> u & 1) && (mask >> v & 1) && find(edges.begin(), edges.end(), pair{u,v}) == edges.end() && find(edges.begin(), edges.end(), pair{v,u}) == edges.end()) clique = false;
    if (clique) expected.push_back(mask);
  }
  assert(got == expected);
}
int main() {
  for (int n = 0; n <= 5; ++n) {
    Edges all; for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) all.emplace_back(u, v);
    for (unsigned mask = 0; mask < (1u << all.size()); ++mask) {
      Edges edges; for (unsigned i = 0; i < all.size(); ++i) if (mask >> i & 1) edges.push_back(all[i]);
      check(n, edges);
    }
  }
  mt19937 rng(1729);
  for (int n = 1; n <= 10; ++n) for (int t = 0; t < 100; ++t) {
    Edges edges;
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) if (rng() % 5 < 2) edges.emplace_back(u, v);
    shuffle(edges.begin(), edges.end(), rng); for (auto& [u, v] : edges) if (rng() % 2) swap(u, v);
    check(n, edges);
  }
  Edges triangle{{0,1},{0,2},{1,2}}, edge{{0,1}};
  int calls = 0;
  blueberry::enumerate_cliques(3, triangle, [&](const vector<int>& c) {
    auto saved = c;
    int inner = 0; blueberry::enumerate_cliques(2, edge, [&](const vector<int>&) { ++inner; });
    assert(inner == 3 && c == saved); ++calls;
  });
  assert(calls == 7);
  for (const auto& e : {triangle, edge}) {
    int n = e.size() == 1 ? 2 : 3, count = 0;
    bool caught = false;
    try { blueberry::enumerate_cliques(n, e, [&](const vector<int>&) { if (++count == 2) throw runtime_error("expected"); }); }
    catch (const runtime_error&) { caught = true; }
    assert(caught && count == 2);
  }
}
