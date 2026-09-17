#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {

template <class T>
struct LinearSystem {
  std::vector<T> particular;
  std::vector<std::vector<T>> basis;
};

// Exact field elimination. Ordinary integers and epsilon-based floating point
// elimination are not supported by this exact-zero/inverse contract.
template <class T>
class Matrix {
 public:
  Matrix() = default;
  Matrix(int rows, int cols, const T& value = T{})
      : rows_(rows), cols_(cols), data_(storage_size(rows, cols), value) {}
  Matrix(const Matrix&) = default;
  Matrix& operator=(const Matrix&) = default;
  Matrix(Matrix&& other) noexcept
      : rows_(std::exchange(other.rows_, 0)), cols_(std::exchange(other.cols_, 0)),
        data_(std::move(other.data_)) { other.data_.clear(); }
  Matrix& operator=(Matrix&& other) noexcept {
    if (this == &other) return *this;
    data_ = std::move(other.data_);
    rows_ = std::exchange(other.rows_, 0); cols_ = std::exchange(other.cols_, 0);
    other.data_.clear();
    return *this;
  }
  int rows() const { return rows_; }
  int cols() const { return cols_; }
  T& operator()(int row, int col) {
    assert(0 <= row && row < rows_ && 0 <= col && col < cols_);
    return data_[static_cast<std::size_t>(row) * cols_ + col];
  }
  const T& operator()(int row, int col) const {
    assert(0 <= row && row < rows_ && 0 <= col && col < cols_);
    return data_[static_cast<std::size_t>(row) * cols_ + col];
  }
  T det() const {
    assert(rows_ == cols_);
    Matrix work = *this;
    auto result = work.eliminate(cols_, false);
    return static_cast<int>(result.pivots.size()) == rows_ ? result.determinant : T{};
  }
  int rank() const {
    Matrix work = *this;
    return static_cast<int>(work.eliminate(cols_, false).pivots.size());
  }
  std::optional<Matrix> inv() const {
    assert(rows_ == cols_ && cols_ <= std::numeric_limits<int>::max() / 2);
    Matrix work(rows_, cols_ * 2);
    for (int i = 0; i < rows_; ++i) {
      for (int j = 0; j < cols_; ++j) work(i, j) = (*this)(i, j);
      work(i, cols_ + i) = T(1);
    }
    if (static_cast<int>(work.eliminate(cols_, true).pivots.size()) != rows_) return std::nullopt;
    Matrix result(rows_, cols_);
    for (int i = 0; i < rows_; ++i)
      for (int j = 0; j < cols_; ++j) result(i, j) = work(i, cols_ + j);
    return result;
  }
  std::optional<LinearSystem<T>> solve(const std::vector<T>& rhs) const {
    assert(rhs.size() == static_cast<std::size_t>(rows_));
    assert(cols_ < std::numeric_limits<int>::max());
    Matrix work(rows_, cols_ + 1);
    for (int i = 0; i < rows_; ++i) {
      for (int j = 0; j < cols_; ++j) work(i, j) = (*this)(i, j);
      work(i, cols_) = rhs[i];
    }
    const auto pivots = work.eliminate(cols_, true).pivots;
    const int r = static_cast<int>(pivots.size());
    for (int i = r; i < rows_; ++i) if (work(i, cols_) != T{}) return std::nullopt;
    LinearSystem<T> result{std::vector<T>(cols_), {}};
    std::vector<bool> pivot(cols_);
    for (int i = 0; i < r; ++i) {
      pivot[pivots[i]] = true; result.particular[pivots[i]] = work(i, cols_);
    }
    for (int j = 0; j < cols_; ++j) if (!pivot[j]) {
      std::vector<T> direction(cols_);
      direction[j] = T(1);
      for (int i = 0; i < r; ++i) direction[pivots[i]] = -work(i, j);
      result.basis.push_back(std::move(direction));
    }
    return result;
  }

 private:
  struct Elimination { std::vector<int> pivots; T determinant = T(1); };
  static std::size_t storage_size(int rows, int cols) {
    assert(rows >= 0 && cols >= 0);
    assert(cols == 0 || static_cast<std::size_t>(rows) <=
                           std::numeric_limits<std::size_t>::max() / static_cast<std::size_t>(cols));
    return static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols);
  }
  Elimination eliminate(int columns, bool reduced) {
    Elimination result;
    int row = 0;
    for (int col = 0; col < columns && row < rows_; ++col) {
      int pivot = row;
      while (pivot < rows_ && (*this)(pivot, col) == T{}) ++pivot;
      if (pivot == rows_) continue;
      if (pivot != row) {
        for (int j = col; j < cols_; ++j) {
          using std::swap;
          swap((*this)(pivot, j), (*this)(row, j));
        }
        result.determinant = -result.determinant;
      }
      const T diagonal = (*this)(row, col), inverse = T(1) / diagonal;
      result.determinant *= diagonal;
      (*this)(row, col) = T(1);
      for (int j = col + 1; j < cols_; ++j) (*this)(row, j) *= inverse;
      for (int i = reduced ? 0 : row + 1; i < rows_; ++i) {
        if (i == row || (*this)(i, col) == T{}) continue;
        const T factor = (*this)(i, col);
        (*this)(i, col) = T{};
        for (int j = col + 1; j < cols_; ++j) (*this)(i, j) -= factor * (*this)(row, j);
      }
      result.pivots.push_back(col); ++row;
    }
    return result;
  }
  int rows_ = 0, cols_ = 0;
  std::vector<T> data_;
};
}  // namespace blueberry
