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

  static void multiply(const int* a, const int* b, int n, int* row, int* scratch) {
    if (n <= 1) { if (n) row[0] = 0; return; }
    const int middle = n / 2;
    const int capacity = n - middle;
    int* column = scratch;
    int* x = column + n;
    int* y = x + capacity;
    int* xp = y + capacity;
    int* yp = xp + capacity;
    int* sub = yp + capacity;
    for (int half = 0; half < 2; ++half) {
      int nx = 0, ny = 0;
      for (int i = 0; i < n; ++i) {
        if ((a[i] >= middle) == (half != 0)) {
          x[nx] = a[i] - half * middle;
          xp[nx++] = i;
        }
        if ((b[i] >= middle) == (half != 0)) {
          y[ny] = b[i] - half * middle;
          yp[ny++] = i;
        }
      }
      assert(nx == ny);
      multiply(x, y, nx, sub, sub + capacity);
      for (int i = 0; i < nx; ++i) {
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
  }

  static std::vector<int> partial_product(const std::vector<int>& a,
                                         const std::vector<int>& b,
                                         std::vector<int>& scratch) {
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
    std::vector<int> product(n);
    multiply(x.data(), y.data(), n, product.data(), scratch.data());
    std::vector<int> result(n, -1);
    const int offset = n - static_cast<int>(rows.size());
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
      const int j = product[offset + i];
      if (j < static_cast<int>(columns.size())) result[rows[i]] = columns[j];
    }
    return result;
  }

  static std::vector<int> build_diagram(const std::vector<int>& p,
                                      std::vector<int>& scratch) {
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
      const auto child = build_diagram(sub, scratch);
      padded[half].resize(n);
      std::iota(padded[half].begin(), padded[half].end(), 0);
      for (int i = 0; i < static_cast<int>(sub.size()); ++i)
        padded[half][positions[i]] = child[i] < 0 ? -1 : positions[child[i]];
    }
    return partial_product(padded[0], padded[1], scratch);
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
    // Each multiplication frame uses n + 5*ceil(n/2) ints; siblings share it.
    // One constructor-local buffer also serves all outer recursion frames.
    std::size_t scratch_size = 0;
    for (std::size_t n = values.size(); n > 1; n = (n + 1) / 2)
      scratch_size += n + 5 * ((n + 1) / 2);
    std::vector<int> row;
    {
      std::vector<int> scratch(scratch_size);
      row = build_diagram(rank, scratch);
    }
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
