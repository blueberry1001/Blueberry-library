#pragma once

#include <cassert>
#include <vector>
#include "multipoint-evaluation.hpp"

namespace blueberry {
template <class Mint>
std::vector<Mint> polynomial_interpolation(const std::vector<Mint>& x,
                                         const std::vector<Mint>& y) {
  assert(x.size() == y.size());
  if (x.empty()) return {};
  polynomial_detail::check_size<Mint>(0, x.size());
  using Poly = FormalPowerSeries<Mint>;
  polynomial_detail::ProductTree<Mint> tree(x);
  const auto denominator = tree.evaluate(tree.product[1].diff(), x);
  std::vector<Poly> value(2 * tree.size);
  for (int i = 0; i < static_cast<int>(x.size()); ++i) {
    assert(denominator[i] != Mint(0));
    value[tree.size + i] = Poly{y[i] / denominator[i]};
  }
  for (int i = tree.size - 1; i; --i)
    value[i] = value[2 * i] * tree.product[2 * i + 1] +
               value[2 * i + 1] * tree.product[2 * i];
  value[1].resize(x.size());
  return {value[1].begin(), value[1].end()};
}
}  // namespace blueberry
