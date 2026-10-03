#pragma once

#include <bit>
#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace blueberry {

// Sparse, online Fenwick tree. Ordinary dense arrays should use ACL instead.
template <class T, class Coord = long long>
class DynamicFenwickTree {
  static_assert(std::is_integral_v<Coord> && std::is_signed_v<Coord>);

 public:
  DynamicFenwickTree() = default;
  explicit DynamicFenwickTree(Coord n) : n_(n) { assert(n >= 0); }
  DynamicFenwickTree(const DynamicFenwickTree&) = default;
  DynamicFenwickTree& operator=(const DynamicFenwickTree&) = default;
  DynamicFenwickTree(DynamicFenwickTree&& other)
      : n_(std::exchange(other.n_, 0)), data_(std::move(other.data_)) {
    other.data_.clear();
  }
  DynamicFenwickTree& operator=(DynamicFenwickTree&& other) {
    if (this == &other) return *this;
    data_ = std::move(other.data_);
    n_ = std::exchange(other.n_, 0);
    other.data_.clear();
    return *this;
  }

  Coord size() const { return n_; }
  void add(Coord p, const T& value) {
    assert(0 <= p && p < n_);
    for (Coord i = p + 1;;) {
      data_[i] += value;
      const Coord step = i & -i;
      if (step > n_ - i) break;
      i += step;
    }
  }
  T pref(Coord r) const {
    assert(0 <= r && r <= n_);
    T answer{};
    for (; r > 0; r -= r & -r) answer += value_at(r);
    return answer;
  }
  T sum(Coord l, Coord r) const {
    assert(0 <= l && l <= r && r <= n_);
    return pref(r) - pref(l);
  }
  T get(Coord p) const {
    assert(0 <= p && p < n_);
    return sum(p, p + 1);
  }
  // First p with pref(p+1) >= weight, or size(). Requires nonnegative values.
  Coord lower_bound(T weight) const {
    if (!(T{} < weight)) return 0;
    Coord p = 0;
    using U = std::make_unsigned_t<Coord>;
    for (Coord step = static_cast<Coord>(std::bit_floor(static_cast<U>(n_)));
         step > 0; step >>= 1) {
      if (step <= n_ - p) {
        T value = value_at(p + step);
        if (value < weight) { weight = weight - value; p += step; }
      }
    }
    return p;
  }

 private:
  T value_at(Coord p) const {
    auto it = data_.find(p);
    return it == data_.end() ? T{} : it->second;
  }
  Coord n_ = 0;
  std::unordered_map<Coord, T> data_;
};

}  // namespace blueberry
