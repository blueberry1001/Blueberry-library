#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <atcoder/convolution>

namespace blueberry {

namespace fps_detail {

// ACL's butterfly is the same radix-4 NTT used by atcoder::convolution.  The
// extra wrapper lets Newton iterations reuse one transformed operand.
template <class Mint>
inline void ntt(std::vector<Mint>& a) {
  atcoder::internal::butterfly(a);
}

template <class Mint>
inline void intt(std::vector<Mint>& a) {
  atcoder::internal::butterfly_inv(a);
  const Mint inv_size = Mint(static_cast<int>(a.size())).inv();
  for (Mint& value : a) value *= inv_size;
}

template <class Mint>
std::optional<Mint> mod_sqrt(Mint value) {
  const std::int64_t mod = Mint::mod();
  if (value == Mint(0)) return Mint(0);
  if (mod == 2) return value;
  if (value.pow((mod - 1) / 2) != Mint(1)) return std::nullopt;
  if (mod % 4 == 3) return value.pow((mod + 1) / 4);

  std::int64_t q = mod - 1;
  int twos = 0;
  while ((q & 1) == 0) q >>= 1, ++twos;

  Mint non_residue = Mint(2);
  while (non_residue.pow((mod - 1) / 2) == Mint(1)) ++non_residue;
  Mint c = non_residue.pow(q);
  Mint x = value.pow((q + 1) / 2);
  Mint t = value.pow(q);
  int order = twos;
  while (t != Mint(1)) {
    int i = 1;
    Mint t2 = t * t;
    while (t2 != Mint(1)) {
      t2 *= t2;
      ++i;
      if (i == order) return std::nullopt;
    }
    Mint b = c;
    for (int j = 0; j < order - i - 1; ++j) b *= b;
    x *= b;
    const Mint b2 = b * b;
    t *= b2;
    c = b2;
    order = i;
  }
  const Mint other = -x;
  return x.val() <= other.val() ? x : other;
}

}  // namespace fps_detail

/**
 * @brief NTT-friendly formal power series over an ACL static_modint.
 *
 * Coefficients are stored in increasing order.  Multiplication uses ACL's
 * convolution; inverse and exponential use ACL's internal radix-4 butterfly
 * so that the transformed approximation can be reused during Newton steps.
 * The modulus must be prime, and every requested degree must be smaller than
 * the modulus.  For NTT-based operations, the modulus must support the
 * required power-of-two transform (998244353 is the intended default).
 */
template <class Mint>
struct FormalPowerSeries : std::vector<Mint> {
  using Base = std::vector<Mint>;
  using FPS = FormalPowerSeries<Mint>;
  using Sparse = std::vector<std::pair<int, Mint>>;
  using Base::Base;
  using Base::operator=;

  FPS pre(int degree) const {
    if (degree <= 0) return {};
    FPS result(this->begin(), this->begin() + std::min<int>(this->size(), degree));
    result.resize(degree, Mint(0));
    return result;
  }

  FPS rev(int degree = -1) const {
    if (degree < 0) degree = static_cast<int>(this->size());
    FPS result = pre(degree);
    std::reverse(result.begin(), result.end());
    return result;
  }

  void shrink() {
    while (!this->empty() && this->back() == Mint(0)) this->pop_back();
  }

  FPS& operator+=(const FPS& rhs) {
    if (rhs.size() > this->size()) this->resize(rhs.size(), Mint(0));
    for (int i = 0; i < static_cast<int>(rhs.size()); ++i) (*this)[i] += rhs[i];
    return *this;
  }

  FPS& operator-=(const FPS& rhs) {
    if (rhs.size() > this->size()) this->resize(rhs.size(), Mint(0));
    for (int i = 0; i < static_cast<int>(rhs.size()); ++i) (*this)[i] -= rhs[i];
    return *this;
  }

  FPS& operator+=(const Mint& value) {
    if (this->empty()) this->resize(1, Mint(0));
    (*this)[0] += value;
    return *this;
  }

  FPS& operator-=(const Mint& value) {
    if (this->empty()) this->resize(1, Mint(0));
    (*this)[0] -= value;
    return *this;
  }

  FPS& operator*=(const Mint& value) {
    for (Mint& coefficient : *this) coefficient *= value;
    return *this;
  }

  FPS& operator/=(const Mint& value) {
    assert(value != Mint(0));
    return *this *= value.inv();
  }

  FPS& operator*=(const FPS& rhs) {
    if (this->empty() || rhs.empty()) {
      this->clear();
      return *this;
    }
    const Base& lhs_base = *this;
    const Base& rhs_base = rhs;
    Base result = atcoder::convolution(lhs_base, rhs_base);
    this->assign(result.begin(), result.end());
    return *this;
  }

  FPS& operator/=(const FPS& rhs) {
    FPS divisor = rhs;
    divisor.shrink();
    assert(!divisor.empty());
    if (this->size() < divisor.size()) {
      this->clear();
      return *this;
    }
    const int quotient_degree = static_cast<int>(this->size() - divisor.size() + 1);
    FPS quotient = (rev().pre(quotient_degree) * divisor.rev().inv(quotient_degree))
                       .pre(quotient_degree)
                       .rev(quotient_degree);
    this->assign(quotient.begin(), quotient.end());
    return *this;
  }

  FPS& operator%=(const FPS& rhs) {
    FPS quotient = *this / rhs;
    *this -= quotient * rhs;
    shrink();
    return *this;
  }

  std::pair<FPS, FPS> div_mod(const FPS& rhs) const {
    FPS quotient = *this / rhs;
    FPS remainder = *this - quotient * rhs;
    remainder.shrink();
    return {std::move(quotient), std::move(remainder)};
  }

  FPS& operator*=(const Sparse& sparse) {
    if (this->empty() || sparse.empty()) {
      this->clear();
      return *this;
    }
    FPS result(this->size(), Mint(0));
    for (const auto& [degree, coefficient] : sparse) {
      assert(degree >= 0);
      if (degree >= static_cast<int>(this->size())) continue;
      for (int i = 0; i + degree < static_cast<int>(this->size()); ++i) {
        result[i + degree] += (*this)[i] * coefficient;
      }
    }
    *this = std::move(result);
    return *this;
  }

  FPS& operator/=(const Sparse& sparse) {
    assert(!sparse.empty());
    Sparse terms = sparse;
    std::sort(terms.begin(), terms.end());
    assert(terms.front().first == 0 && terms.front().second != Mint(0));
    const Mint inverse_constant = terms.front().second.inv();
    for (int i = 0; i < static_cast<int>(this->size()); ++i) {
      Mint value = (*this)[i];
      for (int j = 1; j < static_cast<int>(terms.size()) && terms[j].first <= i; ++j) {
        value -= (*this)[i - terms[j].first] * terms[j].second;
      }
      (*this)[i] = value * inverse_constant;
    }
    return *this;
  }

  FPS& operator<<=(int shift) {
    assert(shift >= 0);
    this->insert(this->begin(), shift, Mint(0));
    return *this;
  }

  FPS& operator>>=(int shift) {
    assert(shift >= 0);
    if (shift >= static_cast<int>(this->size())) this->clear();
    else this->erase(this->begin(), this->begin() + shift);
    return *this;
  }

  FPS operator-() const {
    FPS result(this->size());
    for (int i = 0; i < static_cast<int>(this->size()); ++i) result[i] = -(*this)[i];
    return result;
  }

  FPS operator+(const FPS& rhs) const { return FPS(*this) += rhs; }
  FPS operator-(const FPS& rhs) const { return FPS(*this) -= rhs; }
  FPS operator*(const FPS& rhs) const { return FPS(*this) *= rhs; }
  FPS operator/(const FPS& rhs) const { return FPS(*this) /= rhs; }
  FPS operator%(const FPS& rhs) const { return FPS(*this) %= rhs; }
  FPS operator+(const Mint& value) const { return FPS(*this) += value; }
  FPS operator-(const Mint& value) const { return FPS(*this) -= value; }
  FPS operator*(const Mint& value) const { return FPS(*this) *= value; }
  FPS operator/(const Mint& value) const { return FPS(*this) /= value; }
  FPS operator*(const Sparse& sparse) const { return FPS(*this) *= sparse; }
  FPS operator/(const Sparse& sparse) const { return FPS(*this) /= sparse; }
  FPS operator<<(int shift) const { return FPS(*this) <<= shift; }
  FPS operator>>(int shift) const { return FPS(*this) >>= shift; }

  FPS dot(const FPS& rhs) const {
    FPS result(std::min(this->size(), rhs.size()));
    for (int i = 0; i < static_cast<int>(result.size()); ++i) result[i] = (*this)[i] * rhs[i];
    return result;
  }

  Mint eval(Mint x) const {
    Mint result = 0;
    Mint power = 1;
    for (const Mint& coefficient : *this) {
      result += coefficient * power;
      power *= x;
    }
    return result;
  }

  Mint operator()(Mint x) const { return eval(x); }

  // Multiply by (1 + coefficient * x^degree), keeping the current length.
  void multiply(int degree, Mint coefficient) {
    assert(degree >= 0);
    if (degree == 0) {
      *this *= coefficient + Mint(1);
      return;
    }
    if (degree >= static_cast<int>(this->size())) return;
    for (int i = static_cast<int>(this->size()) - degree - 1; i >= 0; --i) {
      (*this)[i + degree] += (*this)[i] * coefficient;
    }
  }

  // Divide by (1 + coefficient * x^degree), keeping the current length.
  void divide(int degree, Mint coefficient) {
    assert(degree >= 0);
    if (degree == 0) {
      *this /= coefficient + Mint(1);
      return;
    }
    if (degree >= static_cast<int>(this->size())) return;
    for (int i = 0; i + degree < static_cast<int>(this->size()); ++i) {
      (*this)[i + degree] -= (*this)[i] * coefficient;
    }
  }

  FPS diff() const {
    if (this->size() <= 1) return {};
    FPS result(this->size() - 1);
    for (int i = 1; i < static_cast<int>(this->size()); ++i) {
      result[i - 1] = (*this)[i] * Mint(i);
    }
    return result;
  }

  FPS integral() const {
    assert(this->size() < static_cast<std::size_t>(Mint::mod()));
    FPS result(this->size() + 1);
    for (int i = 0; i < static_cast<int>(this->size()); ++i) {
      result[i + 1] = (*this)[i] / Mint(i + 1);
    }
    return result;
  }

  FPS inv(int degree = -1) const {
    assert(!this->empty() && (*this)[0] != Mint(0));
    if (degree < 0) degree = static_cast<int>(this->size());
    if (degree <= 0) return {};
    assert(degree < Mint::mod());

    FPS result(degree);
    result[0] = (*this)[0].inv();
    if (degree <= 64) {
      for (int i = 1; i < degree; ++i) {
        Mint value = 0;
        for (int j = 1; j <= i && j < static_cast<int>(this->size()); ++j) {
          value += (*this)[j] * result[i - j];
        }
        result[i] = -value * result[0];
      }
      return result;
    }

    for (int length = 1; length < degree; length <<= 1) {
      const int transform_size = length << 1;
      std::vector<Mint> source(transform_size, Mint(0));
      std::vector<Mint> approximation(transform_size, Mint(0));
      for (int i = 0; i < std::min<int>(this->size(), transform_size); ++i) source[i] = (*this)[i];
      for (int i = 0; i < length; ++i) approximation[i] = result[i];
      fps_detail::ntt(source);
      fps_detail::ntt(approximation);
      for (int i = 0; i < transform_size; ++i) source[i] *= approximation[i];
      fps_detail::intt(source);
      std::fill(source.begin(), source.begin() + length, Mint(0));
      fps_detail::ntt(source);
      for (int i = 0; i < transform_size; ++i) source[i] *= approximation[i];
      fps_detail::intt(source);
      for (int i = length; i < std::min(transform_size, degree); ++i) result[i] = -source[i];
    }
    return result;
  }

  FPS log(int degree = -1) const {
    assert(!this->empty() && (*this)[0] == Mint(1));
    if (degree < 0) degree = static_cast<int>(this->size());
    if (degree <= 0) return {};
    return (diff() * inv(degree)).pre(degree - 1).integral().pre(degree);
  }

  FPS sqrt(int degree = -1) const {
    assert(degree != 0);
    if (degree < 0) degree = static_cast<int>(this->size());
    if (degree <= 0) return {};
    if (this->empty()) return FPS(degree, Mint(0));

    int shift = 0;
    while (shift < static_cast<int>(this->size()) && (*this)[shift] == Mint(0)) ++shift;
    if (shift == static_cast<int>(this->size())) return FPS(degree, Mint(0));
    if (shift & 1) return {};
    const int half_shift = shift / 2;
    if (half_shift >= degree) return FPS(degree, Mint(0));

    const auto root = fps_detail::mod_sqrt((*this)[shift]);
    if (!root) return {};
    const int target = degree - half_shift;
    FPS shifted = (*this >> shift).pre(target);
    FPS result{*root};
    const Mint inverse_two = Mint(2).inv();
    for (int length = 1; length < target; length <<= 1) {
      const int next = std::min(length << 1, target);
      result = (result + shifted.pre(next) * result.inv(next)).pre(next) * inverse_two;
    }
    result <<= half_shift;
    result.resize(degree, Mint(0));
    return result;
  }

  template <class RootFunction>
  FPS sqrt_with(RootFunction get_root, int degree = -1) const {
    assert(degree != 0);
    if (degree < 0) degree = static_cast<int>(this->size());
    if (degree <= 0) return {};
    if (this->empty()) return FPS(degree, Mint(0));
    int shift = 0;
    while (shift < static_cast<int>(this->size()) && (*this)[shift] == Mint(0)) ++shift;
    if (shift == static_cast<int>(this->size())) return FPS(degree, Mint(0));
    if (shift & 1) return {};
    const int half_shift = shift / 2;
    if (half_shift >= degree) return FPS(degree, Mint(0));
    const int target = degree - half_shift;
    FPS shifted = (*this >> shift).pre(target);
    FPS result{get_root(shifted[0])};
    const Mint inverse_two = Mint(2).inv();
    for (int length = 1; length < target; length <<= 1) {
      const int next = std::min(length << 1, target);
      result = (result + shifted.pre(next) * result.inv(next)).pre(next) * inverse_two;
    }
    result <<= half_shift;
    result.resize(degree, Mint(0));
    return result;
  }

  FPS exp(int degree = -1) const {
    if (degree < 0) degree = static_cast<int>(this->size());
    if (degree <= 0) return {};
    assert(this->empty() || (*this)[0] == Mint(0));
    assert(degree < Mint::mod());
    if (degree == 1) return FPS{Mint(1)};

    std::vector<Mint> inverse_numbers(degree + 1, Mint(0));
    inverse_numbers[1] = Mint(1);
    const std::int64_t mod = Mint::mod();
    for (int i = 2; i <= degree; ++i) {
      inverse_numbers[i] = -inverse_numbers[mod % i] * Mint(mod / i);
    }

    auto integral_in_place = [&](FPS& series) {
      const int old_size = static_cast<int>(series.size());
      series.insert(series.begin(), Mint(0));
      for (int i = 1; i <= old_size; ++i) series[i] *= inverse_numbers[i];
    };
    auto diff_in_place = [](FPS& series) {
      if (series.empty()) return;
      series.erase(series.begin());
      Mint multiplier = 1;
      for (Mint& value : series) {
        value *= multiplier;
        multiplier += Mint(1);
      }
    };

    FPS result{Mint(1), this->size() > 1 ? (*this)[1] : Mint(0)};
    FPS inverse_series{Mint(1)};
    FPS inverse_transform{Mint(1), Mint(1)};

    // This is the divide-and-conquer Newton recurrence for exp.  Each round
    // updates both the result and the inverse of the result while reusing the
    // forward transform of the current approximation.
    for (int length = 2; length < degree; length <<= 1) {
      FPS transformed = result;
      transformed.resize(length << 1, Mint(0));
      fps_detail::ntt(transformed);

      FPS previous_inverse = inverse_transform;
      FPS correction(length, Mint(0));
      for (int i = 0; i < length; ++i) correction[i] = transformed[i] * previous_inverse[i];
      fps_detail::intt(correction);
      std::fill(correction.begin(), correction.begin() + length / 2, Mint(0));
      fps_detail::ntt(correction);
      for (int i = 0; i < length; ++i) correction[i] *= -previous_inverse[i];
      fps_detail::intt(correction);
      inverse_series.insert(inverse_series.end(), correction.begin() + length / 2, correction.end());

      inverse_transform = inverse_series;
      inverse_transform.resize(length << 1, Mint(0));
      fps_detail::ntt(inverse_transform);

      FPS error(this->pre(length));
      error.resize(length, Mint(0));
      diff_in_place(error);
      error.push_back(Mint(0));
      fps_detail::ntt(error);
      for (int i = 0; i < length; ++i) error[i] *= transformed[i];
      fps_detail::intt(error);
      error -= result.diff();
      error.resize(length << 1, Mint(0));
      for (int i = 0; i < length - 1; ++i) {
        error[length + i] = error[i];
        error[i] = Mint(0);
      }
      fps_detail::ntt(error);
      for (int i = 0; i < length * 2; ++i) error[i] *= inverse_transform[i];
      fps_detail::intt(error);
      error.pop_back();
      integral_in_place(error);
      for (int i = length; i < std::min<int>(this->size(), length * 2); ++i) error[i] += (*this)[i];
      std::fill(error.begin(), error.begin() + length, Mint(0));
      fps_detail::ntt(error);
      for (int i = 0; i < length * 2; ++i) error[i] *= transformed[i];
      fps_detail::intt(error);
      result.insert(result.end(), error.begin() + length, error.end());
    }
    return result.pre(degree);
  }

  FPS pow(std::int64_t exponent, int degree = -1) const {
    assert(exponent >= 0);
    if (degree < 0) degree = static_cast<int>(this->size());
    if (degree <= 0) return {};
    if (exponent == 0) {
      FPS result(degree, Mint(0));
      result[0] = Mint(1);
      return result;
    }
    int first = 0;
    while (first < static_cast<int>(this->size()) && (*this)[first] == Mint(0)) ++first;
    if (first == static_cast<int>(this->size())) return FPS(degree, Mint(0));
    const __int128 shift = static_cast<__int128>(first) * exponent;
    if (shift >= degree) return FPS(degree, Mint(0));
    const int shifted = static_cast<int>(shift);
    const int target = degree - shifted;
    const Mint leading = (*this)[first];
    FPS normalized = (*this >> first).pre(target);
    normalized *= leading.inv();
    FPS result = (normalized.log(target) * Mint(exponent)).exp(target);
    result *= leading.pow(exponent);
    result <<= shifted;
    result.resize(degree, Mint(0));
    return result;
  }

  FPS mod_pow(std::int64_t exponent, const FPS& modulus) const {
    assert(exponent >= 0 && !modulus.empty());
    FPS base = *this % modulus;
    FPS result{Mint(1)};
    while (exponent > 0) {
      if (exponent & 1) result = (result * base) % modulus;
      exponent >>= 1;
      if (exponent) base = (base * base) % modulus;
    }
    return result;
  }
};

template <class Mint>
using FPS = FormalPowerSeries<Mint>;

}  // namespace blueberry
