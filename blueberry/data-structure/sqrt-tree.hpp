#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

// Static monoid products. Square-root layers preserve left-to-right order.
template <class S, auto op, auto e>
class SqrtTree {
  struct Layer {
    int child_bits, count_bits;
    std::size_t parent_mask, count_mask;
    std::vector<S> prefix, suffix, between;
  };
  std::vector<S> values_;
  std::vector<Layer> layers_;
  std::array<int, std::numeric_limits<unsigned>::digits> layer_for_bit_{};

 public:
  SqrtTree() = default;
  explicit SqrtTree(const std::vector<S>& values) : values_(values) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    if (values.size() <= 2) return;
    const int height = std::bit_width(values.size() - 1);
    const std::size_t padded = std::size_t{1} << height;
    const std::size_t n = values.size();
    for (int bits = height; bits > 1; bits = (bits + 1) / 2) {
      const int child_bits = (bits + 1) / 2;
      for (int bit = child_bits; bit < bits; ++bit)
        layer_for_bit_[bit] = static_cast<int>(layers_.size());
      const std::size_t span = std::size_t{1} << bits;
      const std::size_t block = std::size_t{1} << child_bits;
      const std::size_t count = span / block;
      layers_.push_back({child_bits, bits - child_bits, ~(span - 1), count - 1, values, values,
                         std::vector<S>(count > 2 ? padded : 0, e())});
      auto& layer = layers_.back();
      for (std::size_t start = 0; start < n; start += block) {
        const std::size_t end = std::min(start + block, n);
        for (std::size_t i = start + 1; i < end; ++i)
          layer.prefix[i] = op(layer.prefix[i - 1], values[i]);
        for (std::size_t i = end - 1; i > start; --i)
          layer.suffix[i - 1] = op(values[i - 1], layer.suffix[i]);
      }
      // count^2 <= span, so each parent fits in its own span slots.
      for (std::size_t start = 0; start < n; start += span) {
        const std::size_t used = (std::min(span, n - start) + block - 1) / block;
        // Endpoint blocks are handled by suffix/prefix in prod().
        for (std::size_t left = 1; left + 1 < used; ++left) {
          S product = layer.suffix[start + left * block];
          layer.between[start + left * count + left] = product;
          for (std::size_t right = left + 1; right + 1 < used; ++right) {
            product = op(product, layer.suffix[start + right * block]);
            layer.between[start + left * count + right] = product;
          }
        }
      }
    }
  }

  int size() const { return static_cast<int>(values_.size()); }
  S get(int p) const {
    assert(0 <= p && p < size());
    return values_[p];
  }
  S prod(int l, int r) const {
    assert(0 <= l && l <= r && r <= size());
    if (l == r) return e();
    if (r - l == 1) return values_[l];
    const int bit = std::bit_width(static_cast<unsigned>(l ^ (r - 1))) - 1;
    if (bit == 0) return op(values_[l], values_[r - 1]);
    const auto& layer = layers_[layer_for_bit_[bit]];
    const std::size_t start = static_cast<std::size_t>(l) & layer.parent_mask;
    const std::size_t left = (static_cast<std::size_t>(l) >> layer.child_bits) & layer.count_mask;
    const std::size_t right = (static_cast<std::size_t>(r - 1) >> layer.child_bits) & layer.count_mask;
    S product = layer.suffix[l];
    if (left + 1 < right) {
      product = op(product, layer.between[start + ((left + 1) << layer.count_bits) + right - 1]);
    }
    return op(product, layer.prefix[r - 1]);
  }
  S all_prod() const { return prod(0, size()); }
};

}  // namespace blueberry
