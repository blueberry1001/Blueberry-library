#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"

#include "blueberry/graph/dijkstra.hpp"
#include "blueberry/utility/fast-io.hpp"

int main() {
  blueberry::FastInput<> in;
  blueberry::FastOutput out;
  int n, m, s, t;
  if (!in.read(n) || !in.read(m) || !in.read(s) || !in.read(t)) return 0;
  blueberry::WeightedGraph<long long> graph(n);
  for (int i = 0; i < m; ++i) {
    int a, b; long long c;
    if (!in.read(a) || !in.read(b) || !in.read(c)) return 0;
    graph[a].push_back({b, c});
  }
  auto [distance, path] = blueberry::dijkstra_path(graph, s, t, 1LL << 62);
  if (path.empty()) { out.writeln(-1); return 0; }
  out.writeln(distance, path.size() - 1);
  for (std::size_t i = 1; i < path.size(); ++i) out.writeln(path[i - 1], path[i]);
}
