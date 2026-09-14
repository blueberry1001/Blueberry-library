#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Li Chao Tree
 */
template <class T = long long>
class LiChaoTree {
 public:
  explicit LiChaoTree(std::vector<T> xs) {
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    assert(xs.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 4));
    // Do not retain the original capacity when the input has many duplicates.
    xs_.assign(xs.begin(), xs.end());
    lines_.resize(4 * xs_.size());
  }

  int size() const { return static_cast<int>(xs_.size()); }

  void add_line(T a, T b) {
    if (size() != 0) insert({a, b}, 1, 0, size());
  }

  void add_segment(T l, T r, T a, T b) {
    assert(l <= r);
    const int left = index(l), right = index(r);
    if (left < right) insert_segment({a, b}, left, right, 1, 0, size());
  }

  std::optional<T> query(T x) const {
    const int position = index(x);
    assert(position < size() && xs_[position] == x);
    std::optional<T> answer;
    int node = 1, left = 0, right = size();
    while (left < right) {
      if (lines_[node]) {
        const T value = lines_[node]->eval(x);
        if (!answer || value < *answer) answer = value;
      }
      if (right - left == 1) break;
      const int middle = left + (right - left) / 2;
      if (position < middle) {
        node *= 2;
        right = middle;
      } else {
        node = node * 2 + 1;
        left = middle;
      }
    }
    return answer;
  }

 private:
  struct Line {
    T a, b;
    T eval(T x) const { return a * x + b; }
  };

  int index(T x) const {
    return static_cast<int>(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
  }

  void insert(Line line, int node, int left, int right) {
    while (true) {
      if (!lines_[node]) {
        lines_[node] = line;
        return;
      }
      Line& current = *lines_[node];
      const bool better_left = line.eval(xs_[left]) < current.eval(xs_[left]);
      const bool better_right = line.eval(xs_[right - 1]) < current.eval(xs_[right - 1]);
      // Two affine functions cross at most once: discard a dominated line.
      if (better_left == better_right) {
        if (better_left) current = line;
        return;
      }
      const int middle = left + (right - left) / 2;
      const bool better_middle = line.eval(xs_[middle]) < current.eval(xs_[middle]);
      if (better_middle) std::swap(line, current);
      if (better_left != better_middle) {
        node *= 2;
        right = middle;
      } else {
        node = node * 2 + 1;
        left = middle;
      }
    }
  }

  void insert_segment(Line line, int begin, int end, int node, int left, int right) {
    if (end <= left || right <= begin) return;
    if (begin <= left && right <= end) {
      insert(line, node, left, right);
      return;
    }
    const int middle = left + (right - left) / 2;
    insert_segment(line, begin, end, node * 2, left, middle);
    insert_segment(line, begin, end, node * 2 + 1, middle, right);
  }

  std::vector<T> xs_;
  std::vector<std::optional<Line>> lines_;
};

}  // namespace blueberry
