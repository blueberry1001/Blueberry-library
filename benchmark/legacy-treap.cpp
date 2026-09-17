// Same-input comparisons with independently written standard-container oracles.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <set>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/implicit-treap.hpp"
#include "blueberry/data-structure/ordered-multiset.hpp"
using mint = atcoder::modint998244353;
struct S { mint sum; int length; };
struct F { mint a, b; };
S op(S a, S b) { return {a.sum + b.sum, a.length + b.length}; }
S e() { return {0, 0}; }
S map(F f, S x) { return {f.a * x.sum + f.b * x.length, x.length}; }
F compose(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
long long sum(long long a, long long b) { return a + b; }
long long zero() { return 0; }
struct Query { int type, l, r, a, b; };
using Sequence = blueberry::ImplicitTreap<S, op, e, F, map, compose, id>;
using Bag = blueberry::OrderedMultiset<long long, sum, zero>;

std::uint64_t run_sequence(bool use_tree, const std::vector<int>& initial, const std::vector<Query>& queries) {
  std::uint64_t checksum = 0;
  if (use_tree) {
    std::vector<S> values; values.reserve(initial.size());
    for (int x : initial) values.push_back(S{x, 1});
    Sequence tree(values);
    for (const auto& q : queries) {
      if (q.type == 0) tree.insert(q.l, S{q.a, 1});
      else if (q.type == 1) tree.erase(q.l);
      else if (q.type == 2) tree.reverse(q.l, q.r);
      else if (q.type == 3) tree.apply(q.l, q.r, F{q.a, q.b});
      else checksum += tree.prod(q.l, q.r).sum.val();
    }
    checksum += tree.all_prod().sum.val();
  } else {
    std::vector<mint> values(initial.begin(), initial.end());
    for (const auto& q : queries) {
      if (q.type == 0) values.insert(values.begin() + q.l, mint(q.a));
      else if (q.type == 1) values.erase(values.begin() + q.l);
      else if (q.type == 2) std::reverse(values.begin() + q.l, values.begin() + q.r);
      else if (q.type == 3) {
        for (int i = q.l; i < q.r; ++i) values[i] = mint(q.a) * values[i] + mint(q.b);
      } else {
        mint result = 0;
        for (int i = q.l; i < q.r; ++i) result += values[i];
        checksum += result.val();
      }
    }
    mint total = 0;
    for (mint x : values) total += x;
    checksum += total.val();
  }
  return checksum;
}

std::uint64_t run_bag(bool use_tree, const std::vector<int>& initial, const std::vector<Query>& queries) {
  std::uint64_t checksum = 0;
  if (use_tree) {
    Bag bag;
    for (int x : initial) bag.insert(x);
    for (const auto& q : queries) {
      if (q.type == 0) bag.insert(q.a);
      else if (q.type == 1) checksum += bag.erase(q.a);
      else if (q.type == 2) checksum += bag.rank(q.a) + bag.count(q.a);
      else if (q.type == 3) checksum += bag.kth(q.l % bag.size()).value();
      else {
        int l = q.l % (bag.size() + 1), r = q.r % (bag.size() + 1);
        if (l > r) std::swap(l, r);
        checksum += bag.prod(l, r);
      }
    }
    checksum += bag.all_prod();
  } else {
    std::multiset<long long> bag(initial.begin(), initial.end());
    for (const auto& q : queries) {
      if (q.type == 0) bag.insert(q.a);
      else if (q.type == 1) {
        auto it = bag.find(q.a);
        if (it != bag.end()) { bag.erase(it); ++checksum; }
      } else if (q.type == 2) checksum += std::distance(bag.begin(), bag.lower_bound(q.a)) + bag.count(q.a);
      else if (q.type == 3) checksum += *std::next(bag.begin(), q.l % bag.size());
      else {
        int l = q.l % (bag.size() + 1), r = q.r % (bag.size() + 1);
        if (l > r) std::swap(l, r);
        auto it = std::next(bag.begin(), l);
        for (int i = l; i < r; ++i, ++it) checksum += *it;
      }
    }
    for (long long x : bag) checksum += x;
  }
  return checksum;
}

template <class Function>
std::uint64_t measure(const char* workload, const char* variant, int repeat, Function function) {
  const auto begin = std::chrono::steady_clock::now();
  const auto checksum = function();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - begin).count();
  std::cout << workload << '\t' << variant << '\t' << repeat << '\t' << ns << '\t' << checksum << '\n';
  return checksum;
}

int main() {
  std::mt19937_64 rng(20260917);
  std::vector<int> sequence(50000), bag(20000);
  for (int& x : sequence) x = rng() % 1000000;
  for (int& x : bag) x = rng() % 400;
  std::vector<Query> sq, bq;
  int n = sequence.size();
  for (int i = 0; i < 20000; ++i) {
    Query q{i % 5, int(rng() % (n + 1)), int(rng() % (n + 1)), int(rng() % 1000000), int(rng() % 1000000)};
    if (q.l > q.r) std::swap(q.l, q.r);
    if (q.type == 0) ++n;
    else if (q.type == 1) { q.l %= n; --n; }
    sq.push_back(q);
  }
  for (int i = 0; i < 10000; ++i)
    bq.push_back({i % 5, int(rng() % 100000), int(rng() % 100000), int(rng() % 400), 0});
  for (int repeat = -1; repeat < 5; ++repeat) {
    std::uint64_t sequence_results[2]{}, bag_results[2]{};
    for (int variant = 0; variant < 2; ++variant) {
      const bool tree = (variant + repeat + 1) % 2 == 0;
      sequence_results[tree] = measure("sequence-n50000-q20000", tree ? "implicit-treap" : "vector", repeat,
                                      [&] { return run_sequence(tree, sequence, sq); });
      bag_results[tree] = measure("multiset-n20000-q10000", tree ? "ordered-multiset" : "std-multiset-linear-rank", repeat,
                                 [&] { return run_bag(tree, bag, bq); });
    }
    if (sequence_results[0] != sequence_results[1] || bag_results[0] != bag_results[1]) return 1;
  }
}
