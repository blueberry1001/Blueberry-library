#include "blueberry/graph/dijkstra.hpp"
#include "blueberry/utility/fast-io.hpp"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <tuple>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")
#else
#include <sys/resource.h>
#endif

using Clock = std::chrono::steady_clock;
using Edge = std::tuple<int, int, long long>;
static double elapsed(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}
static std::uint64_t peak_bytes() {
#ifdef _WIN32
  PROCESS_MEMORY_COUNTERS info{};
  info.cb = sizeof(info);
  if (!GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) return 0;
  return static_cast<std::uint64_t>(info.PeakWorkingSetSize);
#else
  struct rusage info{};
  if (getrusage(RUSAGE_SELF, &info)) return 0;
  return static_cast<std::uint64_t>(info.ru_maxrss) * 1024;
#endif
}
template<class Graph>
int finish(const Graph& graph, int source, int target, double parse, double build, double combined) {
  auto search_begin = Clock::now();
  auto [distance, path] = blueberry::dijkstra_path(graph, source, target, 1LL << 62);
  auto search_end = Clock::now();
  blueberry::FastOutput out;
  if (path.empty()) { if (!out.writeln(-1)) return 1; }
  else {
    if (!out.writeln(distance, path.size() - 1)) return 1;
    for (std::size_t i = 1; i < path.size(); ++i)
      if (!out.writeln(path[i - 1], path[i])) return 1;
  }
  if (!out.flush()) return 1;
  auto finish_time = Clock::now();
  auto peak = peak_bytes();
  if (parse < 0) std::fprintf(stderr, "{\"parse_ms\":null,\"build_ms\":null,");
  else std::fprintf(stderr, "{\"parse_ms\":%.6f,\"build_ms\":%.6f,", parse, build);
  std::fprintf(stderr, "\"parse_build_ms\":%.6f,\"search_ms\":%.6f,\"output_ms\":%.6f,\"peak_working_set_bytes\":%llu}\n",
      combined, elapsed(search_begin, search_end), elapsed(search_end, finish_time), static_cast<unsigned long long>(peak));
  return peak ? 0 : 2;
}
int main() {
  auto begin = Clock::now();
  blueberry::FastInput<> in;
  int n, m, source, target;
  if (!in.read(n, m, source, target)) return 1;
#if defined(STREAM_NESTED)
  blueberry::WeightedGraph<long long> graph(n);
  for (int i = 0; i < m; ++i) {
    int a, b; long long c;
    if (!in.read(a, b, c)) return 1;
    graph[a].push_back({b, c});
  }
  auto ready = Clock::now();
  return finish(graph, source, target, -1, -1, elapsed(begin, ready));
#else
  std::vector<Edge> edges;
  edges.reserve(m);
  for (int i = 0; i < m; ++i) {
    int a, b; long long c;
    if (!in.read(a, b, c)) return 1;
    edges.emplace_back(a, b, c);
  }
  auto parsed = Clock::now();
#if defined(CSR_GRAPH)
  blueberry::WeightedCsrGraph<long long> graph(n, edges);
#else
  blueberry::WeightedGraph<long long> graph(n);
  for (auto [a, b, c] : edges) graph[a].push_back({b, c});
#endif
  auto ready = Clock::now();
  return finish(graph, source, target, elapsed(begin, parsed), elapsed(parsed, ready), elapsed(begin, ready));
#endif
}
