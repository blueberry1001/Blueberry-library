#pragma once
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {

// Indexed Sqrt Tree: O(1) monoid queries and O(sqrt(N)) point assignment.
// Root block products use a small unindexed table, whose linear update is
// O(sqrt(N)) in the original size. The static SqrtTree is kept separate.
template <class S, auto op, auto e>
class DynamicSqrtTree {
  struct Layer {
    int bits, child_bits;
    std::vector<S> prefix, suffix, between;
  };
  struct Table {
    int n = 0;
    std::vector<S> values;
    std::vector<Layer> layers;
    std::array<int, std::numeric_limits<unsigned>::digits> layer_for_bit{};

    Table() = default;
    Table(const std::vector<S>& input, bool indexed) : values(input) {
      assert(input.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
      n = static_cast<int>(input.size());
      if (n <= 2) return;
      const int height = std::bit_width(input.size() - 1);
      const std::size_t padded = std::size_t{1} << height;
      values.resize(padded, e());
      for (int bits = height; bits > 1; bits = (bits + 1) / 2) {
        const int child_bits = (bits + 1) / 2;
        for (int bit = child_bits; bit < bits; ++bit)
          layer_for_bit[bit] = static_cast<int>(layers.size());
        const bool root_index = indexed && layers.empty();
        layers.push_back({bits, child_bits, values, values,
                          std::vector<S>(root_index ? 0 : padded, e())});
        auto& layer = layers.back();
        const std::size_t block = std::size_t{1} << child_bits;
        const std::size_t span = std::size_t{1} << bits;
        for (std::size_t start = 0; start < padded; start += block)
          rebuild_block(layer, start);
        if (!root_index)
          for (std::size_t start = 0; start < padded; start += span)
            rebuild_between(layer, start);
      }
    }

    void rebuild_block(Layer& layer, std::size_t start) {
      const std::size_t end = start + (std::size_t{1} << layer.child_bits);
      layer.prefix[start] = values[start];
      for (std::size_t i = start + 1; i < end; ++i)
        layer.prefix[i] = op(layer.prefix[i - 1], values[i]);
      layer.suffix[end - 1] = values[end - 1];
      for (std::size_t i = end - 1; i > start; --i)
        layer.suffix[i - 1] = op(values[i - 1], layer.suffix[i]);
    }

    void rebuild_between(Layer& layer, std::size_t start) {
      const std::size_t block = std::size_t{1} << layer.child_bits;
      const std::size_t count = std::size_t{1} << (layer.bits - layer.child_bits);
      // count^2 <= parent span; all parents share one linear layer array.
      for (std::size_t left = 0; left < count; ++left) {
        S product = layer.suffix[start + left * block];
        layer.between[start + left * count + left] = product;
        for (std::size_t right = left + 1; right < count; ++right) {
          product = op(product, layer.suffix[start + right * block]);
          layer.between[start + left * count + right] = product;
        }
      }
    }

    void set(int p, const S& value) {
      values[p] = value;
      for (auto& layer : layers) {
        rebuild_block(layer, (static_cast<std::size_t>(p) >> layer.child_bits) << layer.child_bits);
        if (!layer.between.empty())
          rebuild_between(layer, (static_cast<std::size_t>(p) >> layer.bits) << layer.bits);
      }
    }

    template <bool Indexed>
    S prod(int l, int r, const Table* index = nullptr) const {
      if (l == r) return e();
      if (r - l == 1) return values[l];
      const int bit = std::bit_width(static_cast<unsigned>(l ^ (r - 1))) - 1;
      if (bit == 0) return op(values[l], values[r - 1]);
      const auto& layer = layers[layer_for_bit[bit]];
      const std::size_t start = (static_cast<std::size_t>(l) >> layer.bits) << layer.bits;
      const std::size_t left = (l - start) >> layer.child_bits;
      const std::size_t right = (r - 1 - start) >> layer.child_bits;
      S product = layer.suffix[l];
      if (left + 1 < right) {
        if constexpr (Indexed) {
          if (layer.between.empty()) {
            // The unindexed instantiation cannot recurse into another index.
            product = op(product, index->template prod<false>(static_cast<int>(left + 1),
                                                              static_cast<int>(right)));
            return op(product, layer.prefix[r - 1]);
          }
        }
        const std::size_t count = std::size_t{1} << (layer.bits - layer.child_bits);
        product = op(product, layer.between[start + (left + 1) * count + right - 1]);
      }
      return op(product, layer.prefix[r - 1]);
    }
  };

  Table data_, index_;
  void swap(DynamicSqrtTree& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(index_, other.index_);
  }

 public:
  DynamicSqrtTree() = default;
  explicit DynamicSqrtTree(const std::vector<S>& values) : data_(values, true) {
    if (data_.layers.empty()) return;
    const auto& root = data_.layers.front();
    const std::size_t block = std::size_t{1} << root.child_bits;
    std::vector<S> blocks;
    blocks.reserve(data_.values.size() / block);
    for (std::size_t start = 0; start < data_.values.size(); start += block)
      blocks.push_back(root.suffix[start]);
    index_ = Table(blocks, false);
  }
  DynamicSqrtTree(const DynamicSqrtTree&) = default;
  DynamicSqrtTree& operator=(const DynamicSqrtTree&) = default;
  DynamicSqrtTree(DynamicSqrtTree&& other) noexcept { swap(other); }
  DynamicSqrtTree& operator=(DynamicSqrtTree&& other) noexcept {
    if (this != &other) {
      DynamicSqrtTree moved(std::move(other));
      swap(moved);
    }
    return *this;
  }

  int size() const { return data_.n; }
  S get(int p) const {
    assert(0 <= p && p < size());
    return data_.values[p];
  }
  void set(int p, const S& value) {
    assert(0 <= p && p < size());
    data_.set(p, value);
    if (data_.layers.empty()) return;
    const auto& root = data_.layers.front();
    const int block = p >> root.child_bits;
    index_.set(block, root.suffix[static_cast<std::size_t>(block) << root.child_bits]);
  }
  S prod(int l, int r) const {
    assert(0 <= l && l <= r && r <= size());
    return data_.template prod<true>(l, r, &index_);
  }
  S all_prod() const { return prod(0, size()); }
};

}  // namespace blueberry
