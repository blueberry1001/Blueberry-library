#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <ostream>

namespace blueberry {

// Reduced int64 rational. Every result must fit after normalization.
// Signed 128-bit intermediates cover every product/sum of two valid values.
class Fraction {
 public:
  constexpr Fraction(std::int64_t numerator = 0, std::int64_t denominator = 1) {
    assign(numerator, denominator);
  }

  constexpr Fraction(const Fraction&) = default;
  constexpr Fraction(Fraction&&) = default;
  constexpr Fraction& operator=(const Fraction&) = default;
  constexpr Fraction& operator=(Fraction&&) = default;

  constexpr std::int64_t num() const { return num_; }
  constexpr std::int64_t den() const { return den_; }

  constexpr Fraction operator+() const { return *this; }
  constexpr Fraction operator-() const {
    assert(num_ != std::numeric_limits<std::int64_t>::min());
    Fraction result = *this;
    result.num_ = -num_;
    return result;
  }

  friend constexpr Fraction operator+(const Fraction& a, const Fraction& b) {
    return from_wide(Wide(a.num_) * b.den_ + Wide(b.num_) * a.den_,
                     Wide(a.den_) * b.den_);
  }
  friend constexpr Fraction operator-(const Fraction& a, const Fraction& b) {
    return from_wide(Wide(a.num_) * b.den_ - Wide(b.num_) * a.den_,
                     Wide(a.den_) * b.den_);
  }
  friend constexpr Fraction operator*(const Fraction& a, const Fraction& b) {
    return from_wide(Wide(a.num_) * b.num_, Wide(a.den_) * b.den_);
  }
  friend constexpr Fraction operator/(const Fraction& a, const Fraction& b) {
    assert(b.num_ != 0);
    return from_wide(Wide(a.num_) * b.den_, Wide(a.den_) * b.num_);
  }

  constexpr Fraction& operator+=(const Fraction& b) { return *this = *this + b; }
  constexpr Fraction& operator-=(const Fraction& b) { return *this = *this - b; }
  constexpr Fraction& operator*=(const Fraction& b) { return *this = *this * b; }
  constexpr Fraction& operator/=(const Fraction& b) { return *this = *this / b; }

  friend constexpr bool operator==(const Fraction& a, const Fraction& b) {
    return a.num_ == b.num_ && a.den_ == b.den_;
  }
  friend constexpr bool operator!=(const Fraction& a, const Fraction& b) { return !(a == b); }
  friend constexpr bool operator<(const Fraction& a, const Fraction& b) {
    return Wide(a.num_) * b.den_ < Wide(b.num_) * a.den_;
  }
  friend constexpr bool operator>(const Fraction& a, const Fraction& b) { return b < a; }
  friend constexpr bool operator<=(const Fraction& a, const Fraction& b) { return !(b < a); }
  friend constexpr bool operator>=(const Fraction& a, const Fraction& b) { return !(a < b); }

  friend std::ostream& operator<<(std::ostream& out, const Fraction& x) {
    out << x.num_;
    if (x.den_ != 1) out << '/' << x.den_;
    return out;
  }

 private:
  using Wide = __int128_t;
  using UWide = __uint128_t;
  std::int64_t num_ = 0, den_ = 1;

  constexpr void assign(Wide n, Wide d) {
    assert(d != 0);
    if (d < 0) n = -n, d = -d;
    UWide a = n < 0 ? UWide(-n) : UWide(n), b = UWide(d);
    while (b) {
      const UWide r = a % b;
      a = b;
      b = r;
    }
    n /= Wide(a);
    d /= Wide(a);
    assert(n >= std::numeric_limits<std::int64_t>::min());
    assert(n <= std::numeric_limits<std::int64_t>::max());
    assert(d <= std::numeric_limits<std::int64_t>::max());
    num_ = static_cast<std::int64_t>(n);
    den_ = static_cast<std::int64_t>(d);
  }

  static constexpr Fraction from_wide(Wide n, Wide d) {
    Fraction result;
    result.assign(n, d);
    return result;
  }
};

}  // namespace blueberry
