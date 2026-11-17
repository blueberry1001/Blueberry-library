#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>
#include <atcoder/convolution>

namespace blueberry {

template <class Mint>
std::vector<Mint> polynomial_product(std::vector<std::vector<Mint>> factors) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  if (factors.empty()) return {Mint(1)};
  Mint scale = 1;
  bool scaled = false, use_heap = false;
  if (factors.size() < 4) {
    for (const auto& f : factors) if (f.empty()) return {};
  } else {
    std::size_t smallest = static_cast<std::size_t>(-1), largest = 0, constants = 0, degree = 0;
    for (const auto& f : factors) {
      if (f.empty()) return {};
      degree += f.size() - 1;
      if (f.size() == 1) { ++constants; scale *= f[0]; }
      else { smallest = std::min(smallest, f.size()); largest = std::max(largest, f.size()); }
    }
    if (constants && scale == Mint(0)) return std::vector<Mint>(degree + 1);
    if (constants >= factors.size() - constants) {
      scaled = true;
      std::size_t keep = 0;
      for (std::size_t i = 0; i < factors.size(); ++i) if (factors[i].size() > 1) {
        if (keep != i) factors[keep] = std::move(factors[i]);
        ++keep;
      }
      factors.resize(keep);
    }
    use_heap = factors.size() >= 8 && largest / smallest >= 8;
  }
  if (factors.empty()) return {scale};
  if (use_heap) {
    std::vector<std::size_t> ids(factors.size());
    for (std::size_t i = 0; i < ids.size(); ++i) ids[i] = i;
    auto cmp = [&](std::size_t a, std::size_t b) { return factors[a].size() > factors[b].size(); };
    std::make_heap(ids.begin(), ids.end(), cmp);
    while (ids.size() > 1) {
      std::pop_heap(ids.begin(), ids.end(), cmp); auto a = ids.back(); ids.pop_back();
      std::pop_heap(ids.begin(), ids.end(), cmp); auto b = ids.back(); ids.pop_back();
      factors[a] = atcoder::convolution(std::move(factors[a]), std::move(factors[b]));
      ids.push_back(a); std::push_heap(ids.begin(), ids.end(), cmp);
    }
    auto result = std::move(factors[ids[0]]);
    if (scaled) for (auto& x : result) x *= scale;
    return result;
  }
  while (factors.size() > 1) {
    std::vector<std::vector<Mint>> next;
    next.reserve((factors.size() + 1) / 2);
    for (std::size_t i = 0; i < factors.size(); i += 2) {
      if (i + 1 == factors.size()) next.push_back(std::move(factors[i]));
      else next.push_back(atcoder::convolution(std::move(factors[i]), std::move(factors[i + 1])));
    }
    factors = std::move(next);
  }
  if (scaled) for (auto& x : factors[0]) x *= scale;
  return std::move(factors[0]);
}

}
