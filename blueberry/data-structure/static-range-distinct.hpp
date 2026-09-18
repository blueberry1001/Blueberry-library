#pragma once

#include <cassert>
#include <limits>
#include <map>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

namespace blueberry {

template <class T>
class StaticRangeDistinct {
  WaveletMatrix<int> previous_;
  static std::vector<int> previous(const std::vector<T>& a) {
    assert(a.size() < static_cast<unsigned>(std::numeric_limits<int>::max()));
    std::map<T, int> last;
    std::vector<int> result(a.size());
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
      auto [it, inserted] = last.try_emplace(a[i], -1);
      result[i] = it->second;
      it->second = i;
    }
    return result;
  }

 public:
  StaticRangeDistinct() = default;
  explicit StaticRangeDistinct(const std::vector<T>& a) : previous_(previous(a)) {}
  int size() const { return previous_.size(); }
  int count(int l, int r) const { return previous_.range_freq(l, r, l); }
};

}  // namespace blueberry
