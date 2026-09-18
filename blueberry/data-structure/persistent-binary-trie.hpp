#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

namespace blueberry {
// Path copying preserves all previous versions; version 0 is the empty multiset.
template <class UInt = std::uint32_t, int Bits = std::numeric_limits<UInt>::digits>
class PersistentBinaryTrie {
  static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> && !std::is_same_v<UInt, bool>);
  static_assert(1 <= Bits && Bits <= std::numeric_limits<UInt>::digits);
  struct Node { int child[2] = {0, 0}; int count = 0; };
  std::vector<Node> nodes_{Node{}};
  std::vector<int> roots_{0};
  static void check_value([[maybe_unused]] UInt x) {
    if constexpr (Bits < std::numeric_limits<UInt>::digits) assert((x >> Bits) == 0);
  }
  int root(int version) const {
    assert(0 <= version && static_cast<std::size_t>(version) < roots_.size());
    return roots_[version];
  }
  int update(int version, UInt x, int delta) {
    int old = root(version);
    assert(nodes_.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()) - Bits - 1);
    assert(roots_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    int next = static_cast<int>(nodes_.size());
    Node node = nodes_[old]; node.count += delta; nodes_.push_back(node);
    int v = next;
    for (int bit = Bits - 1; bit >= 0; --bit) {
      int d = (x >> bit) & UInt{1};
      old = nodes_[old].child[d];
      node = nodes_[old]; node.count += delta;
      int child = static_cast<int>(nodes_.size());
      nodes_.push_back(node);
      nodes_[v].child[d] = child;
      v = child;
    }
    roots_.push_back(next);
    return static_cast<int>(roots_.size()) - 1;
  }
 public:
  PersistentBinaryTrie() = default;
  int size(int version) const { return nodes_[root(version)].count; }
  bool empty(int version) const { return size(version) == 0; }
  int insert(int version, UInt x) {
    check_value(x); assert(size(version) < std::numeric_limits<int>::max());
    return update(version, x, 1);
  }
  int erase(int version, UInt x) {
    return count(version, x) ? update(version, x, -1) : version;
  }
  int count(int version, UInt x) const {
    check_value(x); int v = root(version);
    for (int bit = Bits - 1; bit >= 0; --bit) v = nodes_[v].child[(x >> bit) & UInt{1}];
    return nodes_[v].count;
  }
  int rank(int version, UInt x) const {
    check_value(x); int v = root(version), answer = 0;
    for (int bit = Bits - 1; bit >= 0; --bit) {
      int d = (x >> bit) & UInt{1};
      if (d) answer += nodes_[nodes_[v].child[0]].count;
      v = nodes_[v].child[d];
    }
    return answer;
  }
  std::optional<UInt> kth(int version, int k) const {
    int v = root(version);
    if (k < 0 || k >= nodes_[v].count) return std::nullopt;
    UInt answer = 0;
    for (int bit = Bits - 1; bit >= 0; --bit) {
      int left = nodes_[nodes_[v].child[0]].count, d = k >= left;
      if (d) { k -= left; answer |= UInt{1} << bit; }
      v = nodes_[v].child[d];
    }
    return answer;
  }
  std::optional<UInt> xor_min(int version, UInt x) const {
    check_value(x); int v = root(version);
    if (!nodes_[v].count) return std::nullopt;
    UInt answer = 0;
    for (int bit = Bits - 1; bit >= 0; --bit) {
      int d = (x >> bit) & UInt{1};
      if (!nodes_[nodes_[v].child[d]].count) { d ^= 1; answer |= UInt{1} << bit; }
      v = nodes_[v].child[d];
    }
    return answer;
  }
};
}  // namespace blueberry
