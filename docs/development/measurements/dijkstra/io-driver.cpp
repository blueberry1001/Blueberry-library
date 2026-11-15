#include "blueberry/graph/dijkstra.hpp"
#ifdef FAST_IO
#include "blueberry/utility/fast-io.hpp"
#else
#include <iostream>
#endif
int main() {
#ifdef FAST_IO
  blueberry::FastInput<> in;
  blueberry::FastOutput out;
  int n, m, s, t;
  if (!in.read(n) || !in.read(m) || !in.read(s) || !in.read(t)) return 1;
#else
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, m, s, t;
  if (!(std::cin >> n >> m >> s >> t)) return 1;
#endif
  blueberry::WeightedGraph<long long> graph(n);
  for (int i = 0; i < m; ++i) {
    int a, b; long long c;
#ifdef FAST_IO
    if (!in.read(a) || !in.read(b) || !in.read(c)) return 1;
#else
    if (!(std::cin >> a >> b >> c)) return 1;
#endif
    graph[a].push_back({b, c});
  }
#ifdef TARGET_ONLY
  auto result = blueberry::dijkstra(graph, s, 1LL << 62, t);
#else
  auto result = blueberry::dijkstra(graph, s, 1LL << 62);
#endif
  auto path = result.path_to(t);
#ifdef FAST_IO
  if (path.empty()) return out.writeln(-1) ? 0 : 1;
  if (!out.writeln(result.distance[t], path.size() - 1)) return 1;
  for (std::size_t i = 1; i < path.size(); ++i)
    if (!out.writeln(path[i - 1], path[i])) return 1;
#else
  if (path.empty()) { std::cout << -1 << '\n'; return 0; }
  std::cout << result.distance[t] << ' ' << path.size() - 1 << '\n';
  for (std::size_t i = 1; i < path.size(); ++i) std::cout << path[i - 1] << ' ' << path[i] << '\n';
#endif
}
