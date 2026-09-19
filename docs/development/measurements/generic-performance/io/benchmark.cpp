#include "blueberry/utility/fast-io.hpp"
#include "blueberry/data-structure/sqrt-tree.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <vector>
int op(int a, int b) { return std::min(a, b); }
int e() { return std::numeric_limits<int>::max(); }
struct Streams {
  template<class... T> bool read(T&... x) { return bool((std::cin >> ... >> x)); }
  bool writeln(int x) { std::cout << x << '\n'; return bool(std::cout); }
  bool flush() { std::cout.flush(); return bool(std::cout); }
};
template<class Input, class Output> int run(Input& in, Output& out) {
  using Clock = std::chrono::steady_clock;
  const auto start = Clock::now();
  int n, q; if (!in.read(n, q)) return 1;
  std::vector<int> a(n); for (auto& x : a) if (!in.read(x)) return 2;
  const auto loaded = Clock::now();
  blueberry::SqrtTree<int, op, e> tree(a);
  const auto built = Clock::now();
  while (q--) { int l, r; if (!in.read(l, r) || !out.writeln(tree.prod(l, r))) return 3; }
  if (!out.flush()) return 4;
  const auto end = Clock::now();
  const auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b-a).count(); };
  std::fprintf(stderr, "{\"input_ms\":%.6f,\"build_ms\":%.6f,\"query_io_ms\":%.6f,\"total_ms\":%.6f}\n",
               ms(start,loaded),ms(loaded,built),ms(built,end),ms(start,end));
  return 0;
}
int main(int argc, char** argv) {
  if (argc != 2) return 5;
  if (std::string_view(argv[1]) == "streams") {
    std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
    Streams io; return run(io, io);
  }
  blueberry::FastInput in; blueberry::FastOutput out;
  return run(in, out);
}
