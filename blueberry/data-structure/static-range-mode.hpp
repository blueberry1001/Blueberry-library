#pragma once
#include <algorithm>
#include <cmath>
#include <numeric>
#include <limits>
#include <utility>
#include <type_traits>
#include <vector>

namespace blueberry {
template <class T>
std::vector<std::pair<int, int>> static_range_mode(
    const std::vector<T>& a, const std::vector<std::pair<int, int>>& queries) {
  const int n = static_cast<int>(a.size()), q = static_cast<int>(queries.size());
  if (!q) return {};
  auto [only_l, only_r] = queries[0];
  if (q >= 32 || static_cast<long long>(only_r - only_l) * 16 < n) {
    if constexpr (requires { static_cast<bool>(a[0] < a[0]); static_cast<bool>(a[0] == a[0]); }) {
      bool identical = true;
      for (int i = 1; i < q; ++i) if (queries[i] != queries[0]) { identical = false; break; }
      if (identical) {
        std::vector<int> sorted(only_r - only_l); std::iota(sorted.begin(), sorted.end(), only_l);
        std::sort(sorted.begin(), sorted.end(), [&](int x, int y) { return a[x] < a[y]; });
        int best = -1, frequency = 0;
        for (int i = 0, j; i < only_r - only_l; i = j) {
          j = i + 1; while (j < only_r - only_l && a[sorted[i]] == a[sorted[j]]) ++j;
          if (j - i > frequency) { frequency = j - i; best = sorted[i]; }
        }
        return std::vector<std::pair<int, int>>(q, {best, frequency});
      }
    }
  }
  std::vector<int> rank(n), representative;
  int m = 0;
  if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
    using U = std::make_unsigned_t<T>;
    std::vector<int> idx(n), tmp(n);
    std::iota(idx.begin(), idx.end(), 0);
    constexpr int digits = std::numeric_limits<U>::digits;
    constexpr U sign = std::is_signed_v<T> ? U(U(1) << (digits - 1)) : U(0);
    for (int shift = 0; shift < digits; shift += 8) {
      int count[256] = {};
      for (int i : idx) ++count[((U(a[i]) ^ sign) >> shift) & 255];
      int offset = 0;
      for (int& c : count) { int k = c; c = offset; offset += k; }
      for (int i : idx) tmp[count[((U(a[i]) ^ sign) >> shift) & 255]++] = i;
      idx.swap(tmp);
    }
    for (int i = 0; i < n; ++i) {
      if (!i || a[idx[i]] != a[idx[i - 1]]) { representative.push_back(idx[i]); ++m; }
      else representative.back() = idx[i];
      rank[idx[i]] = m - 1;
    }
  } else if constexpr (!std::is_move_constructible_v<T> || !std::is_move_assignable_v<T> || !std::is_swappable_v<T>) {
    std::vector<T> values = a;
    std::vector<int> idx(n);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int i, int j) { return values[i] < values[j]; });
    for (int i = 0; i < n; ++i) {
      if (!i || !(values[idx[i]] == values[idx[i - 1]])) { representative.push_back(idx[i]); ++m; }
      else representative.back() = idx[i];
      rank[idx[i]] = m - 1;
    }
  } else {
    std::vector<T> values = a;
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    m = static_cast<int>(values.size());
    representative.resize(m);
    for (int i = 0; i < n; ++i) {
      rank[i] = static_cast<int>(std::lower_bound(values.begin(), values.end(), a[i]) - values.begin());
      representative[rank[i]] = i;
    }
  }
  if (m <= 2 || m == n) {
    std::vector<std::pair<int, int>> answer(q, {-1, 0});
    if (m == 2 && m != n) {
      std::vector<int> prefix(n + 1);
      for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + (rank[i] == 0);
      for (int i = 0; i < q; ++i) {
        auto [l, r] = queries[i]; int x = prefix[r] - prefix[l], y = r - l - x;
        if (x || y) answer[i] = x >= y ? std::pair{representative[0], x} : std::pair{representative[1], y};
      }
    } else for (int i = 0; i < q; ++i) {
      auto [l, r] = queries[i];
      if (l != r) answer[i] = m == 1 ? std::pair{representative[0], r - l} : std::pair{l, 1};
    }
    return answer;
  }
  int left_values[8], right_values[8], left_count = 0, right_count = 0;
  for (auto [l, r] : queries) {
    if (left_count <= 8) {
      int i = 0; while (i < left_count && left_values[i] != l) ++i;
      if (i == left_count) { if (left_count < 8) left_values[left_count] = l; ++left_count; }
    }
    if (right_count <= 8) {
      int i = 0; while (i < right_count && right_values[i] != r) ++i;
      if (i == right_count) { if (right_count < 8) right_values[right_count] = r; ++right_count; }
    }
    if (left_count > 8 && right_count > 8) break;
  }
  if (left_count <= 8 || right_count <= 8) {
    std::vector<std::pair<int, int>> answer(q, {-1, 0});
    bool left = left_count <= right_count;
    std::vector<int> order(q), count(m); std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int x, int y) {
      auto [lx, rx] = queries[x]; auto [ly, ry] = queries[y];
      return left ? (lx != ly ? lx < ly : rx < ry) : (rx != ry ? rx < ry : lx > ly);
    });
    int key = -1, cursor = 0, frequency = 0, best = -1;
    for (int id : order) {
      auto [l, r] = queries[id]; int current_key = left ? l : r;
      if (key != current_key) { std::fill(count.begin(), count.end(), 0); key = current_key; cursor = key; frequency = 0; best = -1; }
      while (left ? cursor < r : cursor > l) {
        int x = rank[left ? cursor++ : --cursor];
        if (++count[x] > frequency) { frequency = count[x]; best = x; }
      }
      answer[id] = {best < 0 ? -1 : representative[best], frequency};
    }
    return answer;
  }
  std::vector<std::pair<int, int>> answer(q, {-1, 0});
  int width = std::max(1, static_cast<int>(n / std::sqrt(static_cast<double>(q))));
  struct Query { int block, right, left, id; };
  std::vector<Query> order; order.reserve(q);
  for (int i = 0; i < q; ++i) order.push_back({queries[i].first / width, queries[i].second, queries[i].first, i});
  std::sort(order.begin(), order.end(), [](const Query& x, const Query& y) {
    if (x.block != y.block) return x.block < y.block;
    return x.right != y.right ? x.right < y.right : x.left < y.left;
  });
  std::vector<int> count(m);
  int group = -1, boundary = 0, right = 0, maximum = 0, best = -1;
  int last_l = -1, last_r = -1, last_id = -1;
  for (auto entry : order) {
    int l = entry.left, r = entry.right;
    if (l == last_l && r == last_r) { answer[entry.id] = answer[last_id]; continue; }
    last_l = l; last_r = r; last_id = entry.id;
    if (entry.block != group) {
      for (int i = boundary; i < right; ++i) count[rank[i]] = 0;
      group = entry.block;
      boundary = static_cast<int>(std::min<long long>(n, (static_cast<long long>(group) + 1) * width));
      right = boundary; maximum = 0; best = -1;
    }
    if (r <= boundary) {
      int f = 0, id = -1;
      for (int i = l; i < r; ++i) { int x = rank[i]; if (++count[x] > f) { f = count[x]; id = x; } }
      for (int i = l; i < r; ++i) --count[rank[i]];
      answer[entry.id] = {id < 0 ? -1 : representative[id], f};
      continue;
    }
    while (right < r) { int x = rank[right++]; if (++count[x] > maximum) { maximum = count[x]; best = x; } }
    int f = maximum, id = best;
    for (int i = l; i < boundary; ++i) { int x = rank[i]; if (++count[x] > f) { f = count[x]; id = x; } }
    for (int i = l; i < boundary; ++i) --count[rank[i]];
    answer[entry.id] = {representative[id], f};
  }
  return answer;
}
}
