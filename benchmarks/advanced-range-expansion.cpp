#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/data-structure/static-range-lis.hpp"
#include "blueberry/data-structure/range-sort-range-product.hpp"
#include "blueberry/data-structure/partially-retroactive-priority-queue.hpp"

using Clock = std::chrono::steady_clock;
using U64 = std::uint64_t;
double elapsed(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}
struct Hash { U64 hash = 0, power = 1; };
Hash op(Hash a, Hash b) { return {a.hash * b.power + b.hash, a.power * b.power}; }
Hash e() { return {}; }
struct Query { int type, l, r; unsigned key; int value; };

// Deliberately simple full replay oracle. Failed edits restore the old timeline.
class ReplayQueue {
  std::vector<int> kind_, value_;
  std::multiset<std::pair<int, int>> current_;
 public:
  explicit ReplayQueue(int n) : kind_(n), value_(n) {}
  bool edit(int type, int t, int value) {
    if (type == 2 ? kind_[t] == 0 : kind_[t] != 0) return false;
    const int old_kind = kind_[t], old_value = value_[t];
    kind_[t] = type == 2 ? 0 : type + 1;
    value_[t] = value;
    std::multiset<std::pair<int, int>> next;
    for (int i = 0; i < static_cast<int>(kind_.size()); ++i) {
      if (kind_[i] == 1) next.emplace(value_[i], i);
      if (kind_[i] == 2) {
        if (next.empty()) {
          kind_[t] = old_kind;
          value_[t] = old_value;
          return false;
        }
        next.erase(next.begin());
      }
    }
    current_.swap(next);
    return true;
  }
  int size() const { return static_cast<int>(current_.size()); }
  std::optional<int> min() const {
    return current_.empty() ? std::nullopt : std::optional<int>(current_.begin()->first);
  }
};

int main(int argc, char** argv) {
  if (argc != 6) return 2;
  const std::string target = argv[1], method = argv[2];
  const int n = std::stoi(argv[3]), q = std::stoi(argv[4]);
  const unsigned seed = std::stoul(argv[5]);
  if (n <= 0 || q <= 0 || (method != "library" && method != "baseline")) return 2;
  std::mt19937 rng(seed);
  U64 checksum = 0, input_hash = 0;
  auto record = [&](U64 value) { input_hash = input_hash * 1000000007 + value; };
  Clock::time_point start, built, end;
  if (target == "lis") {
    std::vector<int> input(n);
    for (int& x : input) x = int(rng() % (n / 4 + 1));
    std::vector<std::pair<int, int>> queries(q);
    for (auto& [l, r] : queries) {
      l = rng() % (n + 1);
      r = rng() % (n + 1);
      if (l > r) std::swap(l, r);
    }
    for (int x : input) record(static_cast<U64>(x));
    for (auto [l, r] : queries) { record(l); record(r); }
    start = Clock::now();
    std::unique_ptr<blueberry::StaticRangeLIS<int>> ds;
    std::vector<int> a;
    if (method == "library") ds = std::make_unique<blueberry::StaticRangeLIS<int>>(input);
    else a = input;
    built = Clock::now();
    std::vector<int> tails;
    tails.reserve(n);
    for (auto [l, r] : queries) {
      int answer;
      if (ds) answer = ds->lis(l, r);
      else {
        tails.clear();
        for (int i = l; i < r; ++i) {
          auto it = std::lower_bound(tails.begin(), tails.end(), a[i]);
          if (it == tails.end()) tails.push_back(a[i]);
          else *it = a[i];
        }
        answer = static_cast<int>(tails.size());
      }
      checksum = checksum * 1000000007 + static_cast<U64>(answer);
    }
    end = Clock::now();
  } else if (target == "range-sort") {
    std::vector<std::pair<unsigned, Hash>> input(n);
    unsigned key = 0;
    for (auto& entry : input) entry = {key++ * 2654435761u, {rng() % 1000, 1000000007}};
    std::shuffle(input.begin(), input.end(), rng);
    std::vector<Query> queries(q);
    for (auto& x : queries) {
      x.type = rng() % 100;
      x.l = rng() % (n + 1);
      x.r = rng() % (n + 1);
      if (x.l > x.r) std::swap(x.l, x.r);
      x.key = key++ * 2654435761u;
      x.value = rng() % 1000;
      if (x.type >= 70 && x.type < 85) x.l = rng() % n;
    }
    for (const auto& [k, value] : input) { record(k); record(value.hash); }
    for (auto x : queries) { record(x.type); record(x.l); record(x.r); record(x.key); record(x.value); }
    using DS = blueberry::RangeSortRangeProduct<Hash, op, e>;
    start = Clock::now();
    std::unique_ptr<DS> ds;
    std::vector<std::pair<unsigned, Hash>> a;
    if (method == "library") ds = std::make_unique<DS>(input);
    else a = input;
    built = Clock::now();
    for (auto x : queries) {
      if (x.type < 70) {
        const bool descending = x.type >= 35;
        if (ds) ds->sort(x.l, x.r, descending);
        else std::sort(a.begin() + x.l, a.begin() + x.r, [descending](const auto& u, const auto& v) {
          return descending ? u.first > v.first : u.first < v.first;
        });
      } else if (x.type < 85) {
        if (ds) ds->set(x.l, x.key, {static_cast<U64>(x.value), 1000000007});
        else a[x.l] = {x.key, {static_cast<U64>(x.value), 1000000007}};
      } else {
        Hash answer;
        if (ds) answer = ds->prod(x.l, x.r);
        else for (int i = x.l; i < x.r; ++i) answer = op(answer, a[i].second);
        checksum = checksum * 1000000007 + answer.hash;
      }
    }
    Hash answer;
    if (ds) answer = ds->all_prod();
    else for (const auto& x : a) answer = op(answer, x.second);
    checksum ^= answer.hash;
    end = Clock::now();
  } else if (target == "retro") {
    std::vector<Query> queries(q);
    for (auto& x : queries) {
      x.type = rng() % 3;
      x.l = rng() % n;
      x.value = int(rng() % 101) - 50;
    }
    for (auto x : queries) { record(x.type); record(x.l); record(static_cast<U64>(x.value)); }
    start = Clock::now();
    std::unique_ptr<blueberry::PartiallyRetroactivePriorityQueue<int>> ds;
    std::unique_ptr<ReplayQueue> baseline;
    if (method == "library") ds = std::make_unique<blueberry::PartiallyRetroactivePriorityQueue<int>>(n);
    else baseline = std::make_unique<ReplayQueue>(n);
    built = Clock::now();
    for (auto x : queries) {
      bool accepted;
      int size;
      std::optional<int> minimum;
      if (ds) {
        accepted = x.type == 0 ? ds->insert_op(x.l, x.value) : x.type == 1 ? ds->pop_op(x.l) : ds->erase_op(x.l);
        size = ds->size();
        minimum = ds->min();
      } else {
        accepted = baseline->edit(x.type, x.l, x.value);
        size = baseline->size();
        minimum = baseline->min();
      }
      checksum = checksum * 1000000007 + static_cast<U64>(size * 1000 + minimum.value_or(100) + 200) * 2 + accepted;
    }
    end = Clock::now();
  } else return 2;
  std::cout << std::setprecision(12) << "{\"target\":\"" << target << "\",\"method\":\"" << method
            << "\",\"n\":" << n << ",\"q\":" << q << ",\"seed\":" << seed
            << ",\"build_ms\":" << elapsed(start, built) << ",\"operations_ms\":" << elapsed(built, end)
            << ",\"input_hash\":\"" << input_hash << "\",\"checksum\":\"" << checksum << "\"}\n";
}
