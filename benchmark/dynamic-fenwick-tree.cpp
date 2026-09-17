#include <bits/stdc++.h>
using namespace std;

#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"
#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"
// The runner extracts the historical header into its temporary build directory.
#include "legacy-dynamic-fenwick-tree-2d.hpp"

struct Query { int x, y, l, d, r, u; long long value; };

template <class Tree>
long long run(Tree& tree, const vector<Query>& queries) {
  long long checksum = 0;
  for (const auto& q : queries) {
    tree.add(q.x, q.y, q.value);
    checksum ^= tree.sum(q.l, q.d, q.r, q.u);
  }
  return checksum;
}

int main(int argc, char** argv) {
  const string mode = argc > 1 ? argv[1] : "online";
  const string shape = argc > 2 ? argv[2] : "bounded";
  const int count = argc > 3 ? atoi(argv[3]) : 60000;
  const int n = shape == "huge" ? 1000000000 : 20000;
  const int m = shape == "huge" ? 1000000000 : 20000;
  mt19937_64 rng(1729);
  vector<Query> queries(count);
  for (auto& q : queries) {
    q.x = rng() % n; q.y = rng() % m;
    if (shape == "repeated") { q.x %= 100; q.y %= 100; }
    q.l = rng() % (n + 1); q.r = rng() % (n + 1);
    q.d = rng() % (m + 1); q.u = rng() % (m + 1);
    if (q.l > q.r) swap(q.l, q.r);
    if (q.d > q.u) swap(q.d, q.u);
    q.value = static_cast<long long>(rng() % 101) - 50;
  }
  auto start = chrono::steady_clock::now();
  long long checksum = 0;
  if (mode == "online") {
    blueberry::DynamicFenwickTree2D<long long, int> tree(n, m);
    checksum = run(tree, queries);
  } else if (mode == "offline") {
    vector<pair<int, int>> points;
    points.reserve(count);
    for (auto q : queries) points.emplace_back(q.x, q.y);
    blueberry::OfflineFenwickTree2D<long long, int> tree(std::move(points));
    checksum = run(tree, queries);
  } else if (mode == "legacy" && shape != "huge") {
    DynamicFenwickTree2D<long long> tree(n, m);
    checksum = run(tree, queries);
    // The legacy type leaks its raw pointers; explicitly release for fair timing.
    for (auto p : tree.bit) delete p;
  } else return 2;
  const double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
  cout << "{\"mode\":\"" << mode << "\",\"shape\":\"" << shape << "\",\"count\":" << count
       << ",\"milliseconds\":" << ms << ",\"checksum\":" << checksum << "}\n";
}
