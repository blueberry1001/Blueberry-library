#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

#include "blueberry/data-structure/wavelet-matrix.hpp"

namespace blueberry {

// Semi-local LIS: recursively multiply subunit Monge matrices represented
// by their permutation diagrams. The wavelet matrix counts diagram points.
template <class T>
class StaticRangeLIS {
  int n_ = 0;
  WaveletMatrix<int> diagram_;

  static std::vector<int> multiply(const std::vector<int>& a,
                                   const std::vector<int>& b) {
    const int n = static_cast<int>(a.size());
    if (n <= 1) return std::vector<int>(n, 0);
    const int middle = n / 2;
    std::vector<int> row(n), column(n);
    for (int half = 0; half < 2; ++half) {
      std::vector<int> x, y, xp, yp;
      for (int i = 0; i < n; ++i) {
        if ((a[i] >= middle) == (half != 0)) {
          x.push_back(a[i] - half * middle);
          xp.push_back(i);
        }
        if ((b[i] >= middle) == (half != 0)) {
          y.push_back(b[i] - half * middle);
          yp.push_back(i);
        }
      }
      auto sub = multiply(x, y);
      for (int i = 0; i < static_cast<int>(x.size()); ++i) {
        row[xp[i]] = yp[sub[i]];
        column[yp[sub[i]]] = xp[i];
      }
    }
    struct Cursor { int column = 0, delta = 0; } left, right;
    int r = n;
    auto advance = [&](Cursor& cursor) {
      const int c = cursor.column++;
      cursor.delta += b[c] < middle ? column[c] >= r : column[c] < r;
    };
    auto rise = [&](Cursor& cursor) {
      cursor.delta -= a[r] < middle ? row[r] >= cursor.column : row[r] < cursor.column;
    };
    while (r != 0) {
      while (right.column < n) {
        auto next = right;
        advance(next);
        if (next.delta != 0) break;
        right = next;
      }
      --r;
      rise(left);
      rise(right);
      while (left.delta != 0) advance(left);
      if (left.column > right.column) row[r] = right.column;
    }
    return row;
  }

  static std::vector<int> partial_product(const std::vector<int>& a,
                                         const std::vector<int>& b) {
    const int n = static_cast<int>(a.size());
    std::vector<int> inverse_a(n, -1), inverse_b(n, -1), x, y, rows, columns;
    for (int i = 0; i < n; ++i) {
      if (a[i] >= 0) inverse_a[a[i]] = i;
      if (b[i] >= 0) inverse_b[b[i]] = i;
    }
    for (int i = 0; i < n; ++i) if (inverse_a[i] < 0) x.push_back(i);
    for (int i = 0; i < n; ++i) if (a[i] >= 0) {
      x.push_back(a[i]);
      rows.push_back(i);
    }
    for (int i = 0; i < n; ++i) if (inverse_b[i] >= 0) {
      y.push_back(inverse_b[i]);
      columns.push_back(i);
    }
    for (int i = 0; i < n; ++i) if (b[i] < 0) y.push_back(i);
    const auto product = multiply(x, y);
    std::vector<int> result(n, -1);
    const int offset = n - static_cast<int>(rows.size());
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
      const int j = product[offset + i];
      if (j < static_cast<int>(columns.size())) result[rows[i]] = columns[j];
    }
    return result;
  }

  static std::vector<int> build_diagram(const std::vector<int>& p) {
    const int n = static_cast<int>(p.size());
    if (n <= 1) return std::vector<int>(n, -1);
    const int middle = n / 2;
    std::vector<int> padded[2];
    for (int half = 0; half < 2; ++half) {
      std::vector<int> sub, positions;
      for (int i = 0; i < n; ++i) if ((p[i] >= middle) == (half != 0)) {
        sub.push_back(p[i] - half * middle);
        positions.push_back(i);
      }
      const auto child = build_diagram(sub);
      padded[half].resize(n);
      std::iota(padded[half].begin(), padded[half].end(), 0);
      for (int i = 0; i < static_cast<int>(sub.size()); ++i)
        padded[half][positions[i]] = child[i] < 0 ? -1 : positions[child[i]];
    }
    return partial_product(padded[0], padded[1]);
  }

 public:
  StaticRangeLIS() = default;
  explicit StaticRangeLIS(const std::vector<T>& values) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    n_ = static_cast<int>(values.size());
    std::vector<int> order(n_), rank(n_);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
      if (values[a] < values[b]) return true;
      if (values[b] < values[a]) return false;
      return a > b;  // Equal values must never form a strictly increasing pair.
    });
    for (int i = 0; i < n_; ++i) rank[order[i]] = i;
    auto row = build_diagram(rank);
    for (int& x : row) if (x < 0) x = n_;
    diagram_ = WaveletMatrix<int>(row);
  }
  int size() const { return n_; }
  int lis(int l, int r) const {
    assert(0 <= l && l <= r && r <= n_);
    return r - l - diagram_.range_freq(l, n_, r);
  }
};

}  // namespace blueberry
