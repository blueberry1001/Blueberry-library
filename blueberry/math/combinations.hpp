#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {
// Mint is a field type: 1, ..., limit must all be invertible.
template<class Mint> class Combinations {
  std::vector<Mint> fact_, inv_fact_;
 public:
  explicit Combinations(int limit) {
    assert(limit >= 0 && limit < std::numeric_limits<int>::max());
    fact_.resize(static_cast<std::size_t>(limit) + 1);
    inv_fact_.resize(static_cast<std::size_t>(limit) + 1);
    fact_[0] = 1;
    for (int i = 1; i <= limit; ++i) fact_[i] = fact_[i - 1] * i;
    inv_fact_[limit] = fact_[limit].inv();
    for (int i = limit; i > 0; --i) inv_fact_[i - 1] = inv_fact_[i] * i;
  }
  int limit() const { return static_cast<int>(fact_.size()) - 1; }
  Mint fact(int n) const { assert(0 <= n && n <= limit()); return fact_[n]; }
  Mint inv_fact(int n) const { assert(0 <= n && n <= limit()); return inv_fact_[n]; }
  Mint C(int n, int k) const {
    if (n < 0 || k < 0 || k > n) return 0;
    assert(n <= limit());
    return fact_[n] * inv_fact_[k] * inv_fact_[n - k];
  }
  Mint P(int n, int k) const {
    if (n < 0 || k < 0 || k > n) return 0;
    assert(n <= limit());
    return fact_[n] * inv_fact_[n - k];
  }
};
}  // namespace blueberry
