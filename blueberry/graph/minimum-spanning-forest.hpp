#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <atcoder/dsu>

namespace blueberry {
template <class T>
std::pair<T, std::vector<int>> minimum_spanning_forest(int n, const std::vector<std::tuple<int, int, T>>& edges) {
  if (n <= 1) return {T{}, {}};
  std::vector<int> order;
  bool sorted = false;
  if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
    if (edges.size() >= 8192) {
      bool fits = true;
      std::uint32_t low = UINT32_MAX, high = 0;
      for (const auto& e : edges) {
        const auto w = std::get<2>(e);
        if constexpr (std::is_signed_v<T> && sizeof(T) > 4) {
          if (w < std::numeric_limits<std::int32_t>::min() || w > std::numeric_limits<std::int32_t>::max()) { fits = false; break; }
        } else if constexpr (std::is_unsigned_v<T> && sizeof(T) > 4) {
          if (w > std::numeric_limits<std::uint32_t>::max()) { fits = false; break; }
        }
        std::uint32_t key = static_cast<std::uint32_t>(w);
        if constexpr (std::is_signed_v<T>) key ^= 0x80000000u;
        low = std::min(low, key); high = std::max(high, key);
      }
      if (fits) {
        if (high - low <= 4095) {
          order.resize(edges.size());
          std::vector<std::size_t> count(static_cast<std::size_t>(high - low) + 1);
          auto key = [&](std::size_t i) {
            std::uint32_t w = static_cast<std::uint32_t>(std::get<2>(edges[i]));
            if constexpr (std::is_signed_v<T>) w ^= 0x80000000u;
            return w - low;
          };
          for (std::size_t i = 0; i < edges.size(); ++i) ++count[key(i)];
          std::size_t sum = 0;
          for (auto& x : count) { const auto c = x; x = sum; sum += c; }
          for (std::size_t i = 0; i < edges.size(); ++i) order[count[key(i)]++] = static_cast<int>(i);
        } else if (edges.size() < static_cast<std::size_t>(n) * 2) {
          std::vector<std::uint64_t> a(edges.size()), b(edges.size());
          std::size_t count[4][256]{};
          for (std::size_t i = 0; i < edges.size(); ++i) {
            std::uint32_t w = static_cast<std::uint32_t>(std::get<2>(edges[i]));
            if constexpr (std::is_signed_v<T>) w ^= 0x80000000u;
            a[i] = (std::uint64_t(w) << 32) | i;
            for (int p = 0; p < 4; ++p) ++count[p][(w >> (p * 8)) & 255];
          }
          for (int p = 0; p < 4; ++p) {
            std::size_t sum = 0;
            for (auto& x : count[p]) { const auto c = x; x = sum; sum += c; }
            for (auto x : a) b[count[p][(x >> (32 + p * 8)) & 255]++] = x;
            a.swap(b);
          }
          order.resize(edges.size());
          for (std::size_t i = 0; i < edges.size(); ++i) order[i] = static_cast<int>(static_cast<std::uint32_t>(a[i]));
        } else {
        struct Item { std::uint32_t key, id; int u, v; };
        std::vector<Item> a(edges.size()), b(edges.size());
        std::size_t count[4][256]{};
        for (std::size_t i = 0; i < edges.size(); ++i) {
          const auto& [u, v, weight] = edges[i];
          std::uint32_t w = static_cast<std::uint32_t>(weight);
          if constexpr (std::is_signed_v<T>) w ^= 0x80000000u;
          a[i] = {w, static_cast<std::uint32_t>(i), u, v};
          for (int p = 0; p < 4; ++p) ++count[p][(w >> (p * 8)) & 255];
        }
        for (int p = 0; p < 4; ++p) {
          std::size_t sum = 0;
          for (auto& x : count[p]) { const auto c = x; x = sum; sum += c; }
          for (auto x : a) b[count[p][(x.key >> (p * 8)) & 255]++] = x;
          a.swap(b);
        }
        atcoder::dsu dsu(n);
        T total{};
        std::vector<int> selected; selected.reserve(n - 1);
        for (const auto& x : a) {
          int u = dsu.leader(x.u), v = dsu.leader(x.v);
          if (u == v) continue;
          dsu.merge(u, v); total += std::get<2>(edges[x.id]); selected.push_back(static_cast<int>(x.id));
          if (selected.size() == static_cast<std::size_t>(n - 1)) break;
        }
        return {total, std::move(selected)};
        }
        sorted = true;
      }
    }
  }
  if (!sorted) {
    order.resize(edges.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) { return std::get<2>(edges[a]) < std::get<2>(edges[b]); });
  }
  atcoder::dsu dsu(n);
  T total{};
  std::vector<int> selected;
  selected.reserve(n - 1);
  for (int id : order) {
    const auto& [u, v, weight] = edges[id];
    int a = dsu.leader(u), b = dsu.leader(v);
    if (a == b) continue;
    dsu.merge(a, b); total += weight; selected.push_back(id);
    if (selected.size() == static_cast<std::size_t>(n - 1)) break;
  }
  return {total, std::move(selected)};
}
}
