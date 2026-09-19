#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <atcoder/lazysegtree>
#include <atcoder/segtree>
#include "blueberry/graph/dynamic-top-tree.hpp"
#include "blueberry/graph/static-top-tree.hpp"
#include "blueberry/graph/link-cut-tree.hpp"
#include "blueberry/graph/heavy-light-decomposition.hpp"
#include "blueberry/graph/euler-tour-tree.hpp"

using U64 = std::uint64_t;
using I64 = long long;
using Clock = std::chrono::steady_clock;
struct Affine { U64 a, b; };
Affine vertex(Affine v, U64 light) { return {v.a, v.a * light + v.b}; }
U64 edge(Affine p) { return p.b; }
Affine compress(Affine x, Affine y) { return {x.a * y.a, x.a * y.b + x.b}; }
U64 rake(U64 a, U64 b) { return a + b; }
U64 identity() { return 0; }
I64 sum(I64 a, I64 b) { return a + b; }
I64 zero() { return 0; }
I64 mapping(I64 f, I64 x, int n) { return x + f * n; }
I64 compose(I64 f, I64 g) { return f + g; }
struct Segment { I64 sum; int size; };
Segment segment_op(Segment a, Segment b) { return {a.sum + b.sum, a.size + b.size}; }
Segment segment_e() { return {0, 0}; }
Segment segment_map(I64 f, Segment x) { return {x.sum + f * x.size, x.size}; }
using DynamicDP = blueberry::DynamicTopTree<Affine, Affine, U64, vertex, edge, compress, rake, identity>;
using FixedDP = blueberry::StaticTopTree<Affine, Affine, U64, vertex, edge, compress, rake, identity>;
using LCT = blueberry::LinkCutTree<I64, sum, zero>;
using ETT = blueberry::EulerTourTree<I64, sum, zero, I64, mapping, compose, zero>;
using Lazy = atcoder::lazy_segtree<Segment, segment_op, segment_e, I64, segment_map, compose, zero>;
struct Query { int type, u, v; I64 value; U64 a, b; };

double milliseconds(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}

int main(int argc, char** argv) {
  if (argc != 6) return 2;
  const std::string target = argv[1], method = argv[2];
  const int n = std::stoi(argv[3]), q = std::stoi(argv[4]);
  const unsigned seed = std::stoul(argv[5]);
  if (n <= 0 || q <= 0 || (method != "dynamic" && method != "fixed")) return 2;
  std::mt19937 rng(seed);
  std::vector<int> parent(n, -1);
  std::vector<std::vector<int>> graph(n);
  for (int v = 1; v < n; ++v) {
    parent[v] = rng() % v;
    graph[v].push_back(parent[v]);
    graph[parent[v]].push_back(v);
  }
  std::vector<I64> values(n);
  std::vector<Affine> coefficients(n);
  for (int v = 0; v < n; ++v) {
    values[v] = rng() % 101;
    coefficients[v] = {rng() % 10 + 1, rng() % 101};
  }
  std::vector<Query> queries(q);
  for (auto& x : queries) {
    x.type = rng() % 2;
    x.u = rng() % n;
    x.v = rng() % n;
    x.value = static_cast<I64>(rng() % 21) - 10;
    x.a = rng() % 10 + 1;
    x.b = rng() % 101;
  }
  U64 input_hash = 0, checksum = 0;
  auto record = [&](U64 x) { input_hash = input_hash * 1000000007 + x; };
  for (int v = 0; v < n; ++v) {
    record(static_cast<U64>(parent[v])); record(values[v]);
    record(coefficients[v].a); record(coefficients[v].b);
  }
  for (auto x : queries) {
    record(x.type); record(x.u); record(x.v); record(static_cast<U64>(x.value)); record(x.a); record(x.b);
  }
  auto answer = [&](U64 x) { checksum = checksum * 1000000007 + x; };
  Clock::time_point start, built, end;
  if (target == "dp") {
    start = Clock::now();
    std::unique_ptr<DynamicDP> dynamic;
    std::unique_ptr<FixedDP> fixed;
    if (method == "dynamic") {
      dynamic = std::make_unique<DynamicDP>(coefficients);
      for (int v = 1; v < n; ++v) if (!dynamic->link(v, parent[v])) return 3;
    } else fixed = std::make_unique<FixedDP>(graph, coefficients, 0);
    built = Clock::now();
    for (auto x : queries) {
      if (dynamic) {
        dynamic->set(x.u, {x.a, x.b});
        answer(dynamic->all_prod(0).b);
      } else {
        fixed->set(x.u, {x.a, x.b});
        answer(fixed->all_prod().b);
      }
    }
    end = Clock::now();
  } else if (target == "path") {
    start = Clock::now();
    std::unique_ptr<LCT> dynamic;
    std::unique_ptr<blueberry::HeavyLightDecomposition> hld;
    std::unique_ptr<atcoder::segtree<I64, sum, zero>> segment;
    if (method == "dynamic") {
      dynamic = std::make_unique<LCT>(values);
      for (int v = 1; v < n; ++v) if (!dynamic->link(v, parent[v])) return 3;
    } else {
      hld = std::make_unique<blueberry::HeavyLightDecomposition>(graph, 0);
      std::vector<I64> ordered(n);
      for (int v = 0; v < n; ++v) ordered[hld->in(v)] = values[v];
      segment = std::make_unique<atcoder::segtree<I64, sum, zero>>(ordered);
    }
    built = Clock::now();
    auto path_sum = [&](int u, int v) {
      if (dynamic) return dynamic->prod(u, v);
      I64 result = 0;
      hld->path_query(u, v, [&](int l, int r) { result += segment->prod(l, r); });
      return result;
    };
    for (auto x : queries) {
      if (x.type == 0) {
        if (dynamic) dynamic->set(x.u, x.value);
        else segment->set(hld->in(x.u), x.value);
      } else answer(static_cast<U64>(path_sum(x.u, x.v)));
    }
    answer(static_cast<U64>(path_sum(0, n - 1)));
    end = Clock::now();
  } else if (target == "subtree") {
    start = Clock::now();
    std::unique_ptr<ETT> dynamic;
    std::unique_ptr<Lazy> segment;
    std::vector<int> in, out;
    if (method == "dynamic") {
      dynamic = std::make_unique<ETT>(values);
      for (int v = 1; v < n; ++v) if (!dynamic->link(v, parent[v])) return 3;
    } else {
      in.resize(n); out.resize(n);
      std::vector<int> stack{0}, next(n, 0);
      std::vector<Segment> ordered;
      ordered.reserve(n);
      while (!stack.empty()) {
        const int v = stack.back();
        if (next[v] == 0) { in[v] = static_cast<int>(ordered.size()); ordered.push_back({values[v], 1}); }
        if (next[v] == static_cast<int>(graph[v].size())) {
          out[v] = static_cast<int>(ordered.size()); stack.pop_back();
        } else {
          const int w = graph[v][next[v]++];
          if (w != parent[v]) stack.push_back(w);
        }
      }
      segment = std::make_unique<Lazy>(ordered);
    }
    built = Clock::now();
    for (auto x : queries) {
      if (x.type == 0) {
        if (dynamic) dynamic->apply(x.u, parent[x.u], x.value);
        else segment->apply(in[x.u], out[x.u], x.value);
      } else {
        const I64 result = dynamic ? dynamic->prod(x.u, parent[x.u]) : segment->prod(in[x.u], out[x.u]).sum;
        answer(static_cast<U64>(result));
      }
    }
    answer(static_cast<U64>(dynamic ? dynamic->prod(0) : segment->all_prod().sum));
    end = Clock::now();
  } else return 2;
  std::cout << std::setprecision(12) << "{\"target\":\"" << target << "\",\"method\":\"" << method
            << "\",\"n\":" << n << ",\"q\":" << q << ",\"seed\":" << seed
            << ",\"build_ms\":" << milliseconds(start, built) << ",\"operations_ms\":" << milliseconds(built, end)
            << ",\"input_hash\":\"" << input_hash << "\",\"checksum\":\"" << checksum << "\"}\n";
}
