#pragma once
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <atcoder/convolution>

namespace product_bench {
template<class Mint>
std::vector<Mint> heap(std::vector<std::vector<Mint>> factors) {
  if (factors.empty()) return {Mint(1)};
  for (const auto& f : factors) if (f.empty()) return {};
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
  return std::move(factors[ids[0]]);
}
template<class Mint, bool UseHeap>
std::vector<Mint> fold(std::vector<std::vector<Mint>> factors) {
  Mint constant = 1;
  std::vector<std::vector<Mint>> nonconstant;
  nonconstant.reserve(factors.size());
  for (auto& f : factors) {
    if (f.empty()) return {};
    if (f.size() == 1) constant *= f[0];
    else nonconstant.push_back(std::move(f));
  }
  std::vector<Mint> result;
  if constexpr (UseHeap) result = heap(std::move(nonconstant));
  else result = blueberry::polynomial_product(std::move(nonconstant));
  for (auto& x : result) x *= constant;
  return result;
}
template<class Mint>
std::vector<Mint> hybrid(std::vector<std::vector<Mint>> factors) {
  if (factors.size() < 4) return blueberry::polynomial_product(std::move(factors));
  std::size_t smallest = static_cast<std::size_t>(-1), largest = 0, constants = 0, degree = 0;
  Mint scale = 1;
  for (const auto& f : factors) {
    if (f.empty()) return {};
    degree += f.size() - 1;
    if (f.size() == 1) { ++constants; scale *= f[0]; }
    else { smallest = std::min(smallest, f.size()); largest = std::max(largest, f.size()); }
  }
  if (constants && scale == Mint(0)) return std::vector<Mint>(degree + 1);
  if (constants < factors.size() - constants) constants = 0;
  if (constants) {
    std::size_t keep = 0;
    for (std::size_t i = 0; i < factors.size(); ++i) if (factors[i].size() > 1) {
      if (keep != i) factors[keep] = std::move(factors[i]);
      ++keep;
    }
    factors.resize(keep);
  }
  bool use_heap = factors.size() >= 8 && largest / smallest >= 8;
  auto result = use_heap ? heap(std::move(factors)) : blueberry::polynomial_product(std::move(factors));
  if (constants) for (auto& x : result) x *= scale;
  return result;
}
}
