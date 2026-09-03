#pragma once

#include <cassert>
#include <vector>

namespace blueberry {

/**
 * @brief Eratosthenesの篩
 */
class PrimeSieve {
 public:
  explicit PrimeSieve(int limit) {
    assert(limit >= 0);
    is_prime_.assign(limit + 1, true);
    is_prime_[0] = false;
    if (limit >= 1) is_prime_[1] = false;
    for (long long p = 2; p * p <= limit; ++p) {
      if (!is_prime_[static_cast<int>(p)]) continue;
      for (long long multiple = p * p; multiple <= limit; multiple += p) {
        is_prime_[static_cast<int>(multiple)] = false;
      }
    }
    for (int value = 2; value <= limit; ++value) {
      if (is_prime_[value]) primes_.push_back(value);
    }
  }

  bool is_prime(int value) const {
    assert(0 <= value && value < static_cast<int>(is_prime_.size()));
    return is_prime_[value];
  }

  const std::vector<int>& primes() const { return primes_; }

  int limit() const { return static_cast<int>(is_prime_.size()) - 1; }

 private:
  std::vector<bool> is_prime_;
  std::vector<int> primes_;
};

}  // namespace blueberry
