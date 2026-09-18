#pragma once

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
#include <atcoder/convolution>

namespace blueberry {

// Balanced product tree. Empty list is one; an empty factor is zero.
template <class Mint>
std::vector<Mint> polynomial_product(std::vector<std::vector<Mint>> factors) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  if (factors.empty()) return {Mint(1)};
  for (const auto& f : factors) if (f.empty()) return {};
  while (factors.size() > 1) {
    std::vector<std::vector<Mint>> next;
    next.reserve((factors.size() + 1) / 2);
    for (std::size_t i = 0; i < factors.size(); i += 2) {
      if (i + 1 == factors.size()) {
        next.push_back(std::move(factors[i]));
      } else {
        [[maybe_unused]] std::size_t length = 1;
        while (length < factors[i].size() + factors[i + 1].size() - 1) length *= 2;
        assert((Mint::mod() - 1) % length == 0);
        next.push_back(atcoder::convolution(std::move(factors[i]), std::move(factors[i + 1])));
      }
    }
    factors = std::move(next);
  }
  return std::move(factors[0]);
}

}  // namespace blueberry
