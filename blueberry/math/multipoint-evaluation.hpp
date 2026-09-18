#pragma once

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <vector>
#include "formal-power-series.hpp"

namespace blueberry {
namespace polynomial_detail {
template <class Mint>
void check_size(std::size_t n, std::size_t m) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  assert(n < static_cast<std::size_t>(Mint::mod()));
  assert(m < static_cast<std::size_t>(Mint::mod()));
  assert(n + m + 1 <= static_cast<std::size_t>(INT_MAX / 8));
  [[maybe_unused]] std::size_t length = 1;
  while (length < n + m + 1) length *= 2;
  assert((Mint::mod() - 1) % (4 * length) == 0);
}
template <class Mint>
struct ProductTree {
  using Poly = FormalPowerSeries<Mint>;
  int size = 1;
  std::vector<Poly> product;
  explicit ProductTree(const std::vector<Mint>& x) {
    while (size < static_cast<int>(x.size())) size *= 2;
    product.resize(2 * size, Poly{Mint(1)});
    for (int i = 0; i < static_cast<int>(x.size()); ++i)
      product[size + i] = Poly{-x[i], Mint(1)};
    for (int i = size - 1; i; --i)
      product[i] = product[2 * i] * product[2 * i + 1];
  }
  std::vector<Mint> evaluate(const Poly& f, const std::vector<Mint>& x) const {
    std::vector<Mint> result(x.size());
    auto visit = [&](auto&& self, const Poly& a, int v, int l, int r) -> void {
      if (l >= static_cast<int>(x.size())) return;
      if (a.size() <= 32 || r - l <= 32) {
        for (int i = l; i < std::min(r, static_cast<int>(x.size())); ++i) {
          Mint value = 0;
          for (auto it = a.rbegin(); it != a.rend(); ++it) value = value * x[i] + *it;
          result[i] = value;
        }
        return;
      }
      const int mid = (l + r) / 2;
      self(self, a % product[2 * v], 2 * v, l, mid);
      self(self, a % product[2 * v + 1], 2 * v + 1, mid, r);
    };
    visit(visit, f % product[1], 1, 0, size);
    return result;
  }
};
}  // namespace polynomial_detail

template <class Mint>
std::vector<Mint> multipoint_evaluation(const std::vector<Mint>& f,
                                      const std::vector<Mint>& x) {
  if (f.empty() || x.empty()) return std::vector<Mint>(x.size());
  polynomial_detail::check_size<Mint>(f.size(), x.size());
  return polynomial_detail::ProductTree<Mint>(x).evaluate(
      FormalPowerSeries<Mint>(f.begin(), f.end()), x);
}
}  // namespace blueberry
