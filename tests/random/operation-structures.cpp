#include "blueberry/data-structure/segment-tree-beats.hpp"
#include "blueberry/data-structure/binary-trie.hpp"
#include "blueberry/data-structure/aggregate-queue.hpp"
#include "blueberry/data-structure/aggregate-deque.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

std::uint64_t test_seed;
int test_step;
void check(bool ok, const char* label) {
  if (!ok) { std::cerr << "seed=" << test_seed << " step=" << test_step << " " << label << '\n'; std::exit(1); }
}
struct Affine {
  long long a, b;
  Affine() = delete;
  Affine(long long x, long long y) : a(x), b(y) {}
  bool operator==(const Affine&) const = default;
};
constexpr long long modulus = 1000000007;
long long counted_ops = 0;
long long counted_sum(long long a, long long b) { ++counted_ops; return a + b; }
long long sum_zero() { return 0; }
Affine compose(Affine f, Affine g) { return {f.a * g.a % modulus, (f.b * g.a + g.b) % modulus}; }
Affine unit() { return {1, 0}; }
Affine fold(const std::deque<Affine>& a) { auto result = unit(); for (auto v : a) result = compose(result, v); return result; }
int main(int argc, char** argv) {
  test_seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(test_seed);
  using Beats = blueberry::SegmentTreeBeats;
  for (int round = 0; round < 30; ++round) {
    int n = static_cast<int>(rng() % 70);
    std::vector<long long> a(n);
    for (auto& v : a) v = static_cast<long long>(rng() % 1000) - 500;
    Beats tree(a);
    for (test_step = 0; test_step < 1500; ++test_step) {
      int l = static_cast<int>(rng() % (n + 1)), r = static_cast<int>(rng() % (n + 1));
      if (l > r) std::swap(l, r);
      long long x = static_cast<long long>(rng() % 1000) - 500;
      switch (rng() % 5) {
        case 0: tree.chmin(l, r, x); for (int i = l; i < r; ++i) a[i] = std::min(a[i], x); break;
        case 1: tree.chmax(l, r, x); for (int i = l; i < r; ++i) a[i] = std::max(a[i], x); break;
        case 2: tree.add(l, r, x); for (int i = l; i < r; ++i) a[i] += x; break;
        case 3: if (n) { int p = rng() % n; tree.set(p, x); a[p] = x; } break;
        default: break;
      }
      long long expected = 0, total = 0;
      for (int i = l; i < r; ++i) expected += a[i];
      for (auto v : a) total += v;
      check(tree.prod(l, r) == expected && tree.all_prod() == total, "beats sum");
      check(tree.size() == n, "beats size");
      if (l == r) check(!tree.min(l, r) && !tree.max(l, r), "beats empty minmax");
      else {
        check(tree.min(l, r) == *std::min_element(a.begin() + l, a.begin() + r), "beats min");
        check(tree.max(l, r) == *std::max_element(a.begin() + l, a.begin() + r), "beats max");
        check(tree.get(l) == a[l], "beats get");
      }
      if (test_step % 100 == 0) {
        auto copy = tree; auto moved = std::move(copy);
        check(copy.size() == 0 && copy.all_prod() == 0, "beats move source");
        copy = Beats(std::vector<long long>{7});
        check(copy.get(0) == 7 && moved.all_prod() == total, "beats move reuse");
        Beats assigned; assigned = tree; assigned = std::move(moved);
        check(assigned.all_prod() == total && moved.size() == 0, "beats assignments");
      }
    }
  }
  Beats wide(std::vector<long long>{0, 0});
  for (int i = 0; i < 100; ++i) { wide.add(0, 2, 1LL << 59); wide.chmin(0, 2, 0); }
  check(wide.get(0) == 0 && wide.get(1) == 0, "beats large pending add after caps");
  wide.set(0, (1LL << 60) - 1); wide.set(1, -(1LL << 60) + 1);
  check(wide.all_prod() == 0, "beats documented extremes");

  std::vector<long long> cancellation(18, (1LL << 60) - 1);
  for (int i = 9; i < 18; ++i) cancellation[i] = -cancellation[i];
  Beats cancel(cancellation);
  check(cancel.all_prod() == 0 && cancel.prod(0, 8) == 8 * ((1LL << 60) - 1), "wide internal segment sums");
  auto* beats_alias = &cancel; cancel = *beats_alias; cancel = std::move(*beats_alias);
  check(cancel.all_prod() == 0, "beats self assignments");
  blueberry::BinaryTrie<std::uint64_t> trie;
  std::vector<std::uint64_t> sorted;
  for (test_step = 0; test_step < 10000; ++test_step) {
    const std::uint64_t x = rng() % 2 ? rng() % 30 : rng();
    if (sorted.empty() || (rng() % 3 && sorted.size() < 120)) {
      trie.insert(x); sorted.insert(std::upper_bound(sorted.begin(), sorted.end(), x), x);
    } else {
      const auto value = rng() % 2 ? x : sorted[rng() % sorted.size()];
      auto it = std::lower_bound(sorted.begin(), sorted.end(), value);
      bool present = it != sorted.end() && *it == value;
      check(trie.erase(value) == present, "trie erase one");
      if (present) sorted.erase(it);
    }
    check(trie.size() == static_cast<int>(sorted.size()) && trie.empty() == sorted.empty(), "trie size");
    check(trie.rank(x) == std::lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin(), "trie rank");
    check(trie.count(x) == std::count(sorted.begin(), sorted.end(), x), "trie duplicates");
    check(!trie.kth(-1) && !trie.kth(trie.size()), "trie kth bounds");
    if (!sorted.empty()) {
      int k = rng() % sorted.size();
      check(trie.kth(k) == sorted[k], "trie kth");
      auto best = std::numeric_limits<std::uint64_t>::max();
      for (auto value : sorted) best = std::min(best, value ^ x);
      check(trie.xor_min(x) == best, "trie full width xor");
    } else check(!trie.xor_min(x), "empty xor");
    if (test_step % 100 == 0) {
      auto copy = trie; auto moved = std::move(copy); check(copy.empty(), "trie moved from");
      copy.insert(0); check(copy.count(0) == 1, "trie moved source reuse");
      blueberry::BinaryTrie<std::uint64_t> other; other = trie; other = std::move(moved);
      check(other.size() == trie.size() && moved.empty(), "trie assignments");
    }
  }
  trie.clear(); check(trie.empty() && !trie.xor_min(0), "trie clear");
  for (int i = 0; i < 10000; ++i) { trie.insert(std::uint64_t{1} << 63); check(trie.erase(std::uint64_t{1} << 63), "trie node reuse"); }
  blueberry::BinaryTrie<unsigned, 1> one; one.insert(0); one.insert(1); one.insert(1);
  check(one.count(1) == 2 && one.rank(1) == 1 && one.kth(2) == 1, "one bit trie");

  using Queue = blueberry::AggregateQueue<Affine, compose, unit>;
  using Deque = blueberry::AggregateDeque<Affine, compose, unit>;
  Queue queue; Deque deque; std::deque<Affine> qa, da;
  for (test_step = 0; test_step < 10000; ++test_step) {
    Affine f(rng() % modulus, rng() % modulus);
    if (qa.empty() || (rng() % 2 && qa.size() < 80)) { queue.push(f); qa.push_back(f); }
    else { queue.pop(); qa.pop_front(); }
    if (da.empty() || (rng() % 2 && da.size() < 80)) {
      if (rng() % 2) { deque.push_front(f); da.push_front(f); }
      else { deque.push_back(f); da.push_back(f); }
    } else if (rng() % 2) { deque.pop_front(); da.pop_front(); }
    else { deque.pop_back(); da.pop_back(); }
    check(queue.prod() == fold(qa) && deque.prod() == fold(da), "noncommutative folds");
    check(queue.size() == static_cast<int>(qa.size()) && deque.size() == static_cast<int>(da.size()), "aggregate sizes");
    if (!qa.empty()) check(queue.front() == qa.front() && queue.back() == qa.back(), "queue ends");
    if (!da.empty()) check(deque.front() == da.front() && deque.back() == da.back(), "deque ends");
    if (test_step % 100 == 0) {
      auto qc = queue; auto dc = deque; auto qm = std::move(qc); auto dm = std::move(dc);
      check(qc.empty() && dc.empty(), "aggregate moved sources");
      qc.push(f); dc.push_front(f); check(qc.prod() == f && dc.prod() == f, "aggregate moved reuse");
      Queue qx; Deque dx; qx = queue; dx = deque; qx = std::move(qm); dx = std::move(dm);
      check(qx.prod() == fold(qa) && dx.prod() == fold(da) && qm.empty() && dm.empty(), "aggregate assignment");
    }
  }
  queue.clear(); deque.clear(); check(queue.prod() == unit() && deque.prod() == unit(), "empty fold");
  for (int i = 0; i < 1000; ++i) deque.push_back(Affine(i + 1, i));
  for (int i = 0; i < 1000; ++i) { if (i % 2) deque.pop_back(); else deque.pop_front(); }
  check(deque.empty(), "alternating rebuild boundary");
  counted_ops = 0;
  blueberry::AggregateDeque<long long, counted_sum, sum_zero> counted_deque;
  blueberry::AggregateQueue<long long, counted_sum, sum_zero> counted_queue;
  for (int i = 0; i < 20000; ++i) { counted_deque.push_back(1); counted_queue.push(1); }
  for (int i = 0; i < 20000; ++i) {
    if (i % 2) counted_deque.pop_back(); else counted_deque.pop_front();
    counted_queue.pop();
  }
  check(counted_ops < 20 * 20000, "aggregate amortized operation count");
  auto* trie_alias = &trie; trie = *trie_alias; trie = std::move(*trie_alias);
  check(trie.empty(), "trie self assignments");
  queue.push(Affine(2, 3)); deque.push_back(Affine(4, 5));
  auto* queue_alias = &queue; queue = *queue_alias; queue = std::move(*queue_alias);
  auto* deque_alias = &deque; deque = *deque_alias; deque = std::move(*deque_alias);
  check(queue.prod() == Affine(2, 3) && deque.prod() == Affine(4, 5), "aggregate self assignments");
  std::cout << "PASS operation-structures seed=" << test_seed << '\n';
}
