#pragma once
/*
Source: https://judge.yosupo.jp/submission/410724
Adapted from gatagat/lap (_lapjv_cpp/lapjv.cpp), with integer arithmetic,
bounded row reduction, reusable buffers, and a Monge shortcut.
https://github.com/gatagat/lap/tree/e29341d9e0ec8c3a1f11f239eb65a31973f4a0ab

BSD 2-Clause License
Copyright (c) 2012-2025, Tomas Kazmar

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry { namespace detail {
template <class T>
std::pair<T, std::vector<int>> assignment_old(const std::vector<std::vector<T>>& cost) {
  const int n = static_cast<int>(cost.size());
  if (n == 0) return {T{}, {}};
  const int m = static_cast<int>(cost[0].size());
  std::vector<T> row_potential(n + 1), col_potential(m + 1), slack(m + 1);
  std::vector<int> matched(m + 1), previous(m + 1);
  std::vector<bool> used(m + 1);
  for (int row = 1; row <= n; ++row) {
    matched[0] = row;
    std::fill(slack.begin(), slack.end(), std::numeric_limits<T>::max());
    std::fill(used.begin(), used.end(), false);
    int col = 0;
    do {
      used[col] = true;
      const int active = matched[col];
      T delta = std::numeric_limits<T>::max();
      int next = 0;
      for (int j = 1; j <= m; ++j) if (!used[j]) {
        const T reduced = cost[active - 1][j - 1] - row_potential[active] - col_potential[j];
        if (reduced < slack[j]) { slack[j] = reduced; previous[j] = col; }
        if (slack[j] < delta) { delta = slack[j]; next = j; }
      }
      assert(next != 0);
      for (int j = 0; j <= m; ++j) {
        if (used[j]) { row_potential[matched[j]] += delta; col_potential[j] -= delta; }
        else slack[j] -= delta;
      }
      col = next;
    } while (matched[col] != 0);
    do {
      const int prev = previous[col];
      matched[col] = matched[prev]; col = prev;
    } while (col != 0);
  }
  std::vector<int> match(n);
  for (int j = 1; j <= m; ++j) if (matched[j]) match[matched[j] - 1] = j - 1;
  T total{};
  for (int i = 0; i < n; ++i) total += cost[i][match[i]];
  return {total, std::move(match)};
}
}}

namespace blueberry { namespace detail {
template<class T, class Wide>
std::pair<T, std::vector<int>> lapjv(const std::vector<std::vector<T>>& a) {
    using ll = Wide;
    using std::vector; using std::pair; using std::fill; using std::min; using std::swap; using std::size_t;
    int n = static_cast<int>(a.size());
    const ll inf = ll(1) << (8 * sizeof(ll) - 2);
    if (n == 0) return {0, {}};
    auto c = [&](int i, int j) -> ll {
        return Wide(a[i][j]);
    };
    vector<int> x(n, -1), y(n, -1);
    vector<ll> v(n);
    auto result = [&]() {
        ll total = 0;
        for (int i = 0; i < n; ++i) total += c(i, x[i]);
        return pair<T, vector<int>>{static_cast<T>(total), std::move(x)};
    };

    int directions = 3;
    for (int i = 1; i < n && directions; ++i)
        for (int j = 1; j < n; ++j) {
            ll d = c(i - 1, j - 1) + c(i, j)
                 - c(i - 1, j) - c(i, j - 1);
            if (d > 0) directions &= 2;
            if (d < 0) directions &= 1;
            if (!directions) break;
        }
    if (directions) {
        for (int i = 0; i < n; ++i)
            x[i] = directions & 1 ? i : n - 1 - i;
        return result();
    }

    vector<ll> minimum(n);
    for (int j = 0; j < n; ++j) minimum[j] = c(0, j);
    fill(y.begin(), y.end(), 0);
    for (int i = 1; i < n; ++i) {
        const auto& row = a[i];
        for (int j = 0; j < n; ++j) {
            bool better = row[j] < minimum[j];
            minimum[j] = better ? row[j] : minimum[j];
            y[j] = better ? i : y[j];
        }
    }
    for (int j = 0; j < n; ++j) v[j] = minimum[j];

    vector<char> unique(n, 1);
    for (int j = n - 1; j >= 0; --j) {
        int i = y[j];
        if (x[i] == -1) x[i] = j;
        else unique[i] = 0, y[j] = -1;
    }

    bool complete = true;
    for (int j : x) if (j == -1) { complete = false; break; }
    if (complete) return result();

    vector<int> free_rows, deferred;
    free_rows.reserve(n);
    deferred.reserve(n);
    for (int i = 0; i < n; ++i) {
        if (x[i] == -1) free_rows.push_back(i);
        else if (unique[i]) {
            ll slack = inf;
            for (int j = 0; j < n; ++j)
                if (j != x[i]) slack = min(slack, c(i, j) - v[j]);
            v[x[i]] -= slack;
        }
    }

    for (int pass = 0; pass < 2 && !free_rows.empty(); ++pass) {
        deferred.clear();
        int head = 0, work = 0;
        while (head < int(free_rows.size()) && work++ < n) {
            int i = free_rows[head++], first = 0;
            ll best = c(i, 0) - v[0], second = inf;
            for (int j = 1; j < n; ++j) {
                ll value = c(i, j) - v[j];
                if (value < best)
                    second = best, best = value, first = j;
                else
                    second = min(second, value);
            }

            bool lower = best < second;
            if (lower) {
                v[first] -= second - best;
            } else if (y[first] != -1) {
                for (int j = 0; j < n; ++j)
                    if (y[j] == -1 && c(i, j) - v[j] == best) {
                        first = j;
                        break;
                    }
            }

            int old = y[first];
            x[i] = first;
            y[first] = i;
            if (old != -1) {
                x[old] = -1;
                if (lower) free_rows[--head] = old;
                else deferred.push_back(old);
            }
        }
        deferred.insert(
            deferred.end(), free_rows.begin() + head, free_rows.end());
        free_rows.swap(deferred);
    }

    vector<int> columns(n), pred(n);
    vector<ll> dist(n);
    for (int root : free_rows) {
        for (int j = 0; j < n; ++j) {
            columns[j] = j;
            pred[j] = root;
            dist[j] = c(root, j) - v[j];
        }
        int scanned = 0, labeled = 0, sink = -1;
        ll level = 0;

        while (sink == -1) {
            if (scanned == labeled) {
                level = dist[columns[scanned]];
                for (int k = scanned; k < n; ++k) {
                    int j = columns[k];
                    if (dist[j] > level) continue;
                    if (dist[j] < level)
                        level = dist[j], labeled = scanned, sink = -1;
                    swap(columns[k], columns[labeled++]);
                    if (y[j] == -1) sink = j;
                }
                if (sink != -1) break;
            }

            while (scanned < labeled && sink == -1) {
                int assigned = columns[scanned++], i = y[assigned];
                ll offset = c(i, assigned) - v[assigned] - level;
                for (int k = labeled; k < n; ++k) {
                    int j = columns[k];
                    ll candidate = c(i, j) - v[j] - offset;
                    if (candidate >= dist[j]) continue;
                    dist[j] = candidate;
                    pred[j] = i;
                    if (candidate == level) {
                        if (y[j] == -1) {
                            sink = j;
                            break;
                        }
                        swap(columns[k], columns[labeled++]);
                    }
                }
            }
        }

        for (int k = 0; k < scanned; ++k) {
            int j = columns[k];
            v[j] += dist[j] - dist[sink];
        }
        while (sink != -1) {
            int i = pred[sink], old = x[i];
            x[i] = sink;
            y[sink] = i;
            sink = old;
        }
    }
    return result();
}

template <class T, class Wide>
std::pair<T, std::vector<int>> assignment_wide(const std::vector<std::vector<T>>& cost) {
  const int n = static_cast<int>(cost.size());
  if (!n) return {T{}, {}};
  const int m = static_cast<int>(cost[0].size());
  if (n == 1 && m == 1) return {cost[0][0], {0}};
  if (n == m) return lapjv<T, Wide>(cost);
  std::vector<Wide> dist(m), potential(m);
  std::vector<int> match(n, -1), reverse(m, -1), columns(m), previous(m);
  for (int i = n - 1; i >= 0; --i) {
    int best = 0;
    for (int j = 1; j < m; ++j)
      if (cost[i][j] < cost[i][best] || (!(cost[i][best] < cost[i][j]) && reverse[best] != -1 && reverse[j] == -1)) best = j;
    if (reverse[best] == -1) { match[i] = best; reverse[best] = i; }
  }
  for (int i = n - 1; i >= 0; --i) {
    if (match[i] != -1) continue;
    for (int j = 0; j < m; ++j) { dist[j] = cost[i][j]; columns[j] = j; previous[j] = i; }
    int scanned = 0, column, row;
    Wide distance{};
    while (true) {
      for (int j = scanned; j < m; ++j) {
        int c = columns[j]; Wide next = dist[c] - potential[c];
        if (j == scanned || next < distance) { distance = next; std::swap(columns[scanned], columns[j]); }
      }
      column = columns[scanned++]; row = reverse[column];
      if (row == -1) break;
      const auto& values = cost[row];
      for (int j = 0; j < m; ++j) {
        Wide next = Wide(values[j]) - Wide(values[column]) + dist[column];
        if (next < dist[j]) { dist[j] = next; previous[j] = row; }
      }
    }
    while (scanned--) potential[columns[scanned]] = dist[columns[scanned]] - distance;
    while (row != i) {
      row = previous[column]; reverse[column] = row;
      std::swap(column, match[row]);
    }
  }
  Wide total{};
  for (int i = 0; i < n; ++i) total += Wide(cost[i][match[i]]);
  return {static_cast<T>(total), std::move(match)};
}
}}

namespace blueberry { namespace detail {
template<class T>
std::pair<T, std::vector<int>> assignment_pr84_fast(const std::vector<std::vector<T>>& cost) {
  const int n = static_cast<int>(cost.size()), m = static_cast<int>(cost[0].size());
      std::vector<int> match(n, -1), reverse(m, -1), order(m), from(m);
      std::vector<T> distance(m), potential(m);
      for (int root = 0; root < n; ++root) {
        std::iota(order.begin(), order.end(), 0);
        for (int j = 0; j < m; ++j) { distance[j] = cost[root][j]; from[j] = root; }
        int done = 0, column;
        T shortest;
        while (true) {
          int best = done;
          T best_value = distance[order[best]] - potential[order[best]];
          for (int k = done + 1; k < m; ++k) {
            T value = distance[order[k]] - potential[order[k]];
            if (value < best_value) { best = k; best_value = value; }
          }
          std::swap(order[done], order[best]);
          column = order[done++];
          shortest = best_value;
          int active = reverse[column];
          if (active == -1) break;
          for (int k = done; k < m; ++k) {
            int j = order[k];
            T next = cost[active][j] - cost[active][column] + distance[column];
            if (next < distance[j]) { distance[j] = next; from[j] = active; }
          }
        }
        for (int k = 0; k < done; ++k) { int j = order[k]; potential[j] = distance[j] - shortest; }
        while (column != -1) { int active = from[column], next = match[active]; match[active] = column; reverse[column] = active; column = next; }
      }
      T total{};
      for (int i = 0; i < n; ++i) total += cost[i][match[i]];
      return {total, std::move(match)};
}
template<class T>
std::pair<T, std::vector<int>> assignment_pr84(const std::vector<std::vector<T>>& cost) {
  const int n = static_cast<int>(cost.size());
  if (!n) return {T{}, {}};
  const int m = static_cast<int>(cost[0].size());
  if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= sizeof(long long)) {
    T bound = static_cast<T>(std::numeric_limits<T>::max() / (static_cast<long long>(n) + 1) / 16);
    bool safe = n >= 64 && m <= static_cast<long long>(n) * 8 && bound > 0;
    for (const auto& row : cost) {
      if (!safe) break;
      bool row_safe = true;
      for (T x : row) row_safe &= x >= -bound && x <= bound;
      if (!row_safe) safe = false;
    }
    if (safe) return assignment_pr84_fast(cost);
  }
  return assignment_old(cost);
}
}}

namespace blueberry { namespace detail {
template<class T>
std::pair<T, std::vector<int>> assignment_tiny(const std::vector<std::vector<T>>& cost) {
  const int m = static_cast<int>(cost[0].size());
  if (cost.size() == 1) {
    int best = 0;
    for (int j = 1; j < m; ++j) if (cost[0][j] < cost[0][best]) best = j;
    return {cost[0][best], {best}};
  }
  int first = 0, second = 1;
  if (cost[1][second] < cost[1][first]) std::swap(first, second);
  for (int j = 2; j < m; ++j) {
    if (cost[1][j] < cost[1][first]) { second = first; first = j; }
    else if (cost[1][j] < cost[1][second]) second = j;
  }
  auto wide_sum = [](T a, T b) {
    auto x = static_cast<std::uint64_t>(static_cast<std::int64_t>(a));
    auto y = static_cast<std::uint64_t>(static_cast<std::int64_t>(b));
    auto z = x + y;
    int high = (a < 0 ? -1 : 0) + (b < 0 ? -1 : 0) + (z < x ? 1 : 0);
    return std::pair<int, std::uint64_t>{high, z};
  };
  int best = 0, other = first == 0 ? second : first;
  auto minimum = wide_sum(cost[0][best], cost[1][other]);
  for (int j = 1; j < m; ++j) {
    int k = first == j ? second : first;
    auto next = wide_sum(cost[0][j], cost[1][k]);
    if (next < minimum) { minimum = next; best = j; other = k; }
  }
  return {static_cast<T>(cost[0][best] + cost[1][other]), {best, other}};
}
}}

namespace blueberry {
template <class T>
std::pair<T, std::vector<int>> assignment(const std::vector<std::vector<T>>& cost) {
  if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8) {
    if (cost.size() == 1 || cost.size() == 2) return detail::assignment_tiny(cost);
  }
  if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 4) {
    if (cost.size() <= 4096) return detail::assignment_wide<T, std::int64_t>(cost);
  }
  else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8) {
    if (cost.size() <= 4096) {
#if !defined(__SIZEOF_INT128__)
      const auto n = cost.size();
      if (n >= 64 && cost[0].size() <= n * 8) {
        T bound = static_cast<T>(std::numeric_limits<T>::max() / (static_cast<long long>(n) + 1) / 16);
        bool small = true;
        for (const auto& row : cost) for (T x : row) {
          if (x < std::numeric_limits<std::int32_t>::min() || std::numeric_limits<std::int32_t>::max() < x) {
            small = false;
            if (x < -bound || bound < x) return detail::assignment_old(cost);
          }
        }
        if (small) return detail::assignment_wide<T, std::int64_t>(cost);
        return detail::assignment_pr84_fast(cost);
      }
#endif
      bool small = true;
      for (const auto& row : cost) {
        for (T x : row) if (x < std::numeric_limits<std::int32_t>::min() || std::numeric_limits<std::int32_t>::max() < x) { small = false; break; }
        if (!small) break;
      }
      if (small) return detail::assignment_wide<T, std::int64_t>(cost);
#if defined(__SIZEOF_INT128__)
      return detail::assignment_wide<T, __int128>(cost);
#endif
    }
  }
  return detail::assignment_pr84(cost);
}
}
