// Reproducible, no-I/O core benchmark. Generated headers are supplied by run.py.
#include "baseline-static.hpp"
#include "query-static.hpp"
#include "prune-static.hpp"
#include "combined-static.hpp"
#include "baseline-dynamic.hpp"
#include "prune-dynamic.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using I = long long;
using Clock = std::chrono::steady_clock;
struct Operation { I l, r, a, b, x; bool update; };
struct BaselineDynamic : baseline::DynamicLiChaoTree<> {
  explicit BaselineDynamic(std::vector<I>) : baseline::DynamicLiChaoTree<>(-1000000, 1000001) {}
};
struct PruneDynamic : prunedynamic::DynamicLiChaoTree<> {
  explicit PruneDynamic(std::vector<I>) : prunedynamic::DynamicLiChaoTree<>(-1000000, 1000001) {}
};
double elapsed(Clock::time_point begin) {
  return std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}
template<class Tree>
void measure(const std::vector<I>& xs, const std::vector<Operation>& ops, bool segment) {
  auto begin = Clock::now();
  Tree tree(xs);
  const double construction = elapsed(begin);
  std::uint64_t hash = 0;
  auto update = [&](auto& t, const auto& op) {
    if (segment) t.add_segment(op.l, op.r, op.a, op.b);
    else t.add_line(op.a, op.b);
  };
  begin = Clock::now();
  for (const auto& op : ops) {
    if (op.update) update(tree, op);
    const auto value = tree.query(op.x);
    hash = hash * 1000003 + (value ? static_cast<std::uint64_t>(*value) : 12345);
  }
  const double interleaved = elapsed(begin);
  Tree batch(xs);
  begin = Clock::now();
  for (const auto& op : ops) if (op.update) update(batch, op);
  const double updates = elapsed(begin);
  std::uint64_t batch_hash = 0;
  begin = Clock::now();
  for (const auto& op : ops) {
    const auto value = batch.query(op.x);
    batch_hash = batch_hash * 1000003 + (value ? static_cast<std::uint64_t>(*value) : 12345);
  }
  const double queries = elapsed(begin);
  std::cout << "construction_ms=" << construction << " interleaved_ms=" << interleaved
            << " updates_ms=" << updates << " queries_ms=" << queries
            << " checksum=" << hash << " batch_checksum=" << batch_hash << '\n';
}
int main(int argc, char** argv) {
  if (argc != 5) return 2;
  const std::string candidate = argv[1], workload = argv[2];
  const int n = std::stoi(argv[3]);
  std::mt19937_64 random(std::stoull(argv[4]));
  std::vector<I> xs;
  std::vector<Operation> ops;
  xs.reserve(n); ops.reserve(n);
  const bool segment = workload.find("segment") != std::string::npos;
  for (int i = 0; i < n; ++i) {
    I x = static_cast<I>(random() % 2000001) - 1000000;
    I l = static_cast<I>(random() % 2000002) - 1000000;
    I r = static_cast<I>(random() % 2000002) - 1000000;
    if (l > r) std::swap(l, r);
    if (workload == "narrow-segment") r = std::min(l + static_cast<I>(random() % 32), 1000001LL);
    const I center = static_cast<I>(random() % 2000001) - 1000000;
    I a = -2 * center, b = center * center;
    if (workload == "random-line" || workload == "random-segment") {
      a = static_cast<I>(random() % 2000001) - 1000000;
      b = static_cast<I>(random() % 2000001) - 1000000;
    }
    if (workload == "duplicate-line") x %= 16;
    if (workload == "ordered-line") x = -1000000 + 2000000LL * i / std::max(n - 1, 1);
    const bool update = workload != "query-heavy-line" || i % 10 == 0;
    xs.push_back(x); ops.push_back({l, r, a, b, x, update});
  }
  if (candidate == "baseline-static") measure<baseline::LiChaoTree<>>(xs, ops, segment);
  else if (candidate == "query-static") measure<queryonly::LiChaoTree<>>(xs, ops, segment);
  else if (candidate == "prune-static") measure<pruneonly::LiChaoTree<>>(xs, ops, segment);
  else if (candidate == "combined-static") measure<combined::LiChaoTree<>>(xs, ops, segment);
  else if (candidate == "baseline-dynamic") measure<BaselineDynamic>(xs, ops, segment);
  else if (candidate == "prune-dynamic") measure<PruneDynamic>(xs, ops, segment);
  else return 2;
}
