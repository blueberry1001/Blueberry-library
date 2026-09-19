#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace blueberry {
// Dense integer universe, with a 64-ary hierarchy of nonempty words.
class FastSet {
  int universe_ = 0, count_ = 0;
  std::vector<std::vector<std::uint64_t>> levels_;
 public:
  FastSet() = default;
  explicit FastSet(int universe) : universe_(universe) {
    assert(universe >= 0);
    std::size_t words = static_cast<std::size_t>(universe);
    while (words > 0) {
      words = (words + 63) / 64;
      levels_.emplace_back(words, 0);
      if (words == 1) break;
    }
  }
  explicit FastSet(const std::string& bits) : FastSet(checked_size(bits.size())) {
    for (int i = 0; i < universe_; ++i) {
      assert(bits[i] == '0' || bits[i] == '1');
      if (bits[i] == '1') { levels_[0][i / 64] |= std::uint64_t{1} << (i % 64); ++count_; }
    }
    for (std::size_t h = 1; h < levels_.size(); ++h)
      for (std::size_t i = 0; i < levels_[h - 1].size(); ++i)
        if (levels_[h - 1][i]) levels_[h][i / 64] |= std::uint64_t{1} << (i % 64);
  }
  FastSet(const FastSet&) = default;
  FastSet& operator=(const FastSet&) = default;
  FastSet(FastSet&& other) noexcept
      : universe_(std::exchange(other.universe_, 0)), count_(std::exchange(other.count_, 0)),
        levels_(std::move(other.levels_)) { other.levels_.clear(); }
  FastSet& operator=(FastSet&& other) noexcept {
    if (this != &other) {
      universe_ = std::exchange(other.universe_, 0); count_ = std::exchange(other.count_, 0);
      levels_ = std::move(other.levels_); other.levels_.clear();
    }
    return *this;
  }
  int size() const { return count_; }
  int universe_size() const { return universe_; }
  bool empty() const { return count_ == 0; }
  bool contains(int x) const {
    assert(0 <= x && x < universe_);
    return (levels_[0][x / 64] >> (x % 64)) & 1;
  }
  bool insert(int x) {
    if (contains(x)) return false;
    ++count_;
    for (auto& row : levels_) { row[x / 64] |= std::uint64_t{1} << (x % 64); x /= 64; }
    return true;
  }
  bool erase(int x) {
    if (!contains(x)) return false;
    --count_;
    for (auto& row : levels_) {
      row[x / 64] &= ~(std::uint64_t{1} << (x % 64));
      if (row[x / 64]) break;
      x /= 64;
    }
    return true;
  }
  int next(int x) const {
    if (x >= universe_ || empty()) return universe_;
    std::size_t i = static_cast<std::size_t>(std::max(x, 0));
    for (std::size_t h = 0; h < levels_.size(); ++h) {
      if (i / 64 >= levels_[h].size()) break;
      auto word = levels_[h][i / 64] & (~std::uint64_t{0} << (i % 64));
      if (!word) { i = i / 64 + 1; continue; }
      i = (i / 64) * 64 + std::countr_zero(word);
      for (std::size_t g = h; g > 0; --g) i = i * 64 + std::countr_zero(levels_[g - 1][i]);
      return static_cast<int>(i);
    }
    return universe_;
  }
  int prev(int x) const {
    if (x < 0 || empty()) return -1;
    std::size_t i = static_cast<std::size_t>(std::min(x, universe_ - 1));
    for (std::size_t h = 0; h < levels_.size(); ++h) {
      auto word = levels_[h][i / 64] & (~std::uint64_t{0} >> (63 - i % 64));
      if (!word) { if (i / 64 == 0) break; i = i / 64 - 1; continue; }
      i = (i / 64) * 64 + 63 - std::countl_zero(word);
      for (std::size_t g = h; g > 0; --g) i = i * 64 + 63 - std::countl_zero(levels_[g - 1][i]);
      return static_cast<int>(i);
    }
    return -1;
  }
  void clear() {
    for (auto& row : levels_) std::fill(row.begin(), row.end(), 0);
    count_ = 0;
  }
 private:
  static int checked_size(std::size_t n) {
    assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    return static_cast<int>(n);
  }
};
}  // namespace blueberry
