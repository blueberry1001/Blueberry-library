#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include <atcoder/lazysegtree>
#include <atcoder/segtree>

namespace blueberry {

// Integer time slots are fixed at construction; queries concern the present.
// Bridge prefixes count insertions eventually consumed minus delete-mins.
template <class T>
class PartiallyRetroactivePriorityQueue {
  using Item = std::pair<T, int>;
  struct Extremes { std::optional<Item> alive, dead; };
  static Extremes combine(Extremes a, Extremes b) {
    if (b.alive && (!a.alive || *b.alive < *a.alive)) a.alive = b.alive;
    if (b.dead && (!a.dead || *a.dead < *b.dead)) a.dead = b.dead;
    return a;
  }
  static Extremes identity() { return {}; }
  static int minimum(int a, int b) { return std::min(a, b); }
  static int infinity() { return std::numeric_limits<int>::max() / 2; }
  static int add(int f, int x) { return x == infinity() ? x : x + f; }
  static int compose(int a, int b) { return a + b; }
  static int zero() { return 0; }
  using Prefix = atcoder::lazy_segtree<int, minimum, infinity, int, add, compose, zero>;
  int slots_ = 0, size_ = 0;
  // 0: empty, 1: surviving insert, 2: consumed insert, 3: delete-min.
  std::vector<unsigned char> kind_;
  std::vector<std::optional<T>> values_;
  Prefix bridges_, validity_;
  atcoder::segtree<Extremes, combine, identity> extremes_;

  static int checked(int n) {
    assert(0 <= n && n < std::numeric_limits<int>::max() / 4);
    return n;
  }
  void check([[maybe_unused]] int t) const { assert(0 <= t && t < slots_); }
  void weight(int t, int delta) { bridges_.apply(t + 1, slots_ + 1, delta); }
  bool valid_change(int t, int delta) {
    validity_.apply(t + 1, slots_ + 1, delta);
    if (validity_.all_prod() >= 0) return true;
    validity_.apply(t + 1, slots_ + 1, -delta);
    return false;
  }
  int previous_bridge(int t) {
    return bridges_.min_left(t + 1, [](int x) { return x > 0; }) - 1;
  }
  int next_bridge(int t) {
    return bridges_.max_right(t + 1, [](int x) { return x > 0; });
  }
  void alive(int t) {
    if (kind_[t] == 2) weight(t, -1);
    kind_[t] = 1;
    extremes_.set(t, {Item{*values_[t], t}, std::nullopt});
    ++size_;
  }
  void dead(int t) {
    if (kind_[t] == 1) --size_;
    weight(t, 1);
    kind_[t] = 2;
    extremes_.set(t, {std::nullopt, Item{*values_[t], t}});
  }

 public:
  explicit PartiallyRetroactivePriorityQueue(int slots = 0)
      : slots_(checked(slots)), kind_(slots_), values_(slots_),
        bridges_(std::vector<int>(slots_ + 1)), validity_(std::vector<int>(slots_ + 1)),
        extremes_(slots_) {}
  int time_slots() const { return slots_; }
  int size() const { return size_; }
  bool empty() const { return size_ == 0; }
  bool has_op(int t) const { check(t); return kind_[t] != 0; }
  std::optional<T> min() const {
    const auto x = extremes_.all_prod().alive;
    return x ? std::optional<T>(x->first) : std::nullopt;
  }
  bool insert_op(int t, const T& value) {
    check(t);
    if (kind_[t] != 0) return false;
    const int bridge = previous_bridge(t);
    const auto candidate = extremes_.prod(bridge, slots_).dead;
    values_[t] = value;
    validity_.apply(t + 1, slots_ + 1, 1);
    if (candidate && Item{value, t} < *candidate) {
      dead(t);
      alive(candidate->second);
    } else alive(t);
    return true;
  }
  bool pop_op(int t) {
    check(t);
    if (kind_[t] != 0 || !valid_change(t, -1)) return false;
    const int bridge = next_bridge(t);
    const auto candidate = extremes_.prod(0, bridge).alive;
    assert(candidate);
    dead(candidate->second);
    kind_[t] = 3;
    weight(t, -1);
    return true;
  }
  bool erase_op(int t) {
    check(t);
    if (kind_[t] == 0) return false;
    if (kind_[t] == 3) {
      const int bridge = previous_bridge(t);
      const auto candidate = extremes_.prod(bridge, slots_).dead;
      assert(candidate);
      alive(candidate->second);
      weight(t, 1);
      validity_.apply(t + 1, slots_ + 1, 1);
    } else {
      if (!valid_change(t, -1)) return false;
      if (kind_[t] == 1) --size_;
      else {
        const int bridge = next_bridge(t);
        const auto candidate = extremes_.prod(0, bridge).alive;
        assert(candidate);
        dead(candidate->second);
        weight(t, -1);
      }
      extremes_.set(t, identity());
      values_[t].reset();
    }
    kind_[t] = 0;
    return true;
  }
};

}  // namespace blueberry
