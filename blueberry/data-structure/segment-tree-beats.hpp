#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {
// Segment Tree Beats: range clamps, addition and sums. Half-open intervals.
// Wide internal arithmetic also covers delayed additions followed by clamps.
class SegmentTreeBeats {
  using Wide = __int128_t;
  static constexpr Wide infinity = Wide{1} << 120;
  static constexpr long long bound = 1LL << 60;
  struct Node {
    Wide sum = 0, high = -infinity, second_high = -infinity;
    Wide low = infinity, second_low = infinity, addition = 0;
    int high_count = 0, low_count = 0;
  };
  int n_ = 0;
  std::vector<Node> nodes_;
  static void check_value([[maybe_unused]] long long value) { assert(-bound < value && value < bound); }
  void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
  static long long narrow(Wide value) {
    assert(std::numeric_limits<long long>::min() <= value && value <= std::numeric_limits<long long>::max());
    return static_cast<long long>(value);
  }
  void pull(int v) {
    Node& p = nodes_[v]; const Node& a = nodes_[v * 2]; const Node& b = nodes_[v * 2 + 1];
    p.sum = a.sum + b.sum;
    p.high = std::max(a.high, b.high); p.low = std::min(a.low, b.low);
    p.high_count = (a.high == p.high ? a.high_count : 0) + (b.high == p.high ? b.high_count : 0);
    p.low_count = (a.low == p.low ? a.low_count : 0) + (b.low == p.low ? b.low_count : 0);
    p.second_high = std::max(a.high == p.high ? a.second_high : a.high, b.high == p.high ? b.second_high : b.high);
    p.second_low = std::min(a.low == p.low ? a.second_low : a.low, b.low == p.low ? b.second_low : b.low);
  }
  void build(int v, int l, int r, const std::vector<long long>& values) {
    if (r - l == 1) {
      check_value(values[l]); Node& p = nodes_[v]; p.sum = p.high = p.low = values[l];
      p.high_count = p.low_count = 1; return;
    }
    int mid = l + (r - l) / 2;
    build(v * 2, l, mid, values); build(v * 2 + 1, mid, r, values); pull(v);
  }
  void add_node(int v, int length, Wide value) {
    Node& p = nodes_[v]; p.sum += value * length; p.high += value; p.low += value;
    if (p.second_high != -infinity) p.second_high += value;
    if (p.second_low != infinity) p.second_low += value;
    p.addition += value;
  }
  void cap_node(int v, Wide value) {
    Node& p = nodes_[v];
    if (p.high <= value) return;
    p.sum += (value - p.high) * p.high_count;
    if (p.low == p.high) p.low = value;
    else if (p.second_low == p.high) p.second_low = value;
    p.high = value;
  }
  void floor_node(int v, Wide value) {
    Node& p = nodes_[v];
    if (value <= p.low) return;
    p.sum += (value - p.low) * p.low_count;
    if (p.high == p.low) p.high = value;
    else if (p.second_high == p.low) p.second_high = value;
    p.low = value;
  }
  void push(int v, int l, int r) {
    int mid = l + (r - l) / 2;
    if (nodes_[v].addition != 0) {
      add_node(v * 2, mid - l, nodes_[v].addition); add_node(v * 2 + 1, r - mid, nodes_[v].addition);
      nodes_[v].addition = 0;
    }
    cap_node(v * 2, nodes_[v].high); cap_node(v * 2 + 1, nodes_[v].high);
    floor_node(v * 2, nodes_[v].low); floor_node(v * 2 + 1, nodes_[v].low);
  }
  template <int Kind>
  void update(int v, int l, int r, int ql, int qr, Wide value) {
    if (qr <= l || r <= ql) return;
    if constexpr (Kind == 0) { if (nodes_[v].high <= value) return; }
    if constexpr (Kind == 1) { if (nodes_[v].low >= value) return; }
    if (ql <= l && r <= qr) {
      if constexpr (Kind == 0) { if (nodes_[v].second_high < value) { cap_node(v, value); return; } }
      if constexpr (Kind == 1) { if (value < nodes_[v].second_low) { floor_node(v, value); return; } }
      if constexpr (Kind == 2) {
        assert(-bound < nodes_[v].low + value && nodes_[v].high + value < bound);
        add_node(v, r - l, value); return;
      }
    }
    push(v, l, r); int mid = l + (r - l) / 2;
    update<Kind>(v * 2, l, mid, ql, qr, value); update<Kind>(v * 2 + 1, mid, r, ql, qr, value); pull(v);
  }
  template <int Kind>
  Wide query(int v, int l, int r, int ql, int qr) {
    if (qr <= l || r <= ql) {
      if constexpr (Kind == 0) return 0;
      else if constexpr (Kind == 1) return infinity;
      else return -infinity;
    }
    if (ql <= l && r <= qr) {
      if constexpr (Kind == 0) return nodes_[v].sum;
      else if constexpr (Kind == 1) return nodes_[v].low;
      else return nodes_[v].high;
    }
    push(v, l, r); int mid = l + (r - l) / 2;
    Wide a = query<Kind>(v * 2, l, mid, ql, qr), b = query<Kind>(v * 2 + 1, mid, r, ql, qr);
    if constexpr (Kind == 0) return a + b;
    else if constexpr (Kind == 1) return std::min(a, b);
    else return std::max(a, b);
  }
 public:
  explicit SegmentTreeBeats(const std::vector<long long>& values = {}) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 4));
    n_ = static_cast<int>(values.size()); nodes_.resize(4 * values.size());
    if (n_) build(1, 0, n_, values);
  }
  SegmentTreeBeats(const SegmentTreeBeats&) = default;
  SegmentTreeBeats& operator=(const SegmentTreeBeats&) = default;
  SegmentTreeBeats(SegmentTreeBeats&& other) noexcept
      : n_(std::exchange(other.n_, 0)), nodes_(std::move(other.nodes_)) { other.nodes_.clear(); }
  SegmentTreeBeats& operator=(SegmentTreeBeats&& other) noexcept {
    if (this != &other) { n_ = std::exchange(other.n_, 0); nodes_ = std::move(other.nodes_); other.nodes_.clear(); }
    return *this;
  }
  int size() const { return n_; }
  void chmin(int l, int r, long long value) { check_range(l, r); check_value(value); if (l < r) update<0>(1, 0, n_, l, r, value); }
  void chmax(int l, int r, long long value) { check_range(l, r); check_value(value); if (l < r) update<1>(1, 0, n_, l, r, value); }
  void add(int l, int r, long long value) { check_range(l, r); if (l < r) update<2>(1, 0, n_, l, r, value); }
  long long prod(int l, int r) { check_range(l, r); return l == r ? 0 : narrow(query<0>(1, 0, n_, l, r)); }
  std::optional<long long> min(int l, int r) { check_range(l, r); if (l == r) return std::nullopt; return narrow(query<1>(1, 0, n_, l, r)); }
  std::optional<long long> max(int l, int r) { check_range(l, r); if (l == r) return std::nullopt; return narrow(query<2>(1, 0, n_, l, r)); }
  long long all_prod() const { return n_ ? narrow(nodes_[1].sum) : 0; }
  long long get(int p) { assert(0 <= p && p < n_); return prod(p, p + 1); }
  void set(int p, long long value) { assert(0 <= p && p < n_); chmin(p, p + 1, value); chmax(p, p + 1, value); }
};
}  // namespace blueberry
