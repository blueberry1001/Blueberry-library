#pragma once
// Explicit opt-in: iostream interface. Excluded from blueberry/all.hpp.
#ifdef BLUEBERRY_CONTEST_FAST_IO_INCLUDED
#error Use only one contest I/O template in a translation unit.
#endif
#define BLUEBERRY_CONTEST_STREAM_IO_INCLUDED
#include "blueberry/utility/internal/contest-template-base.hpp"

#define YesNo(x) (std::cout << ((x) ? "Yes" : "No") << '\n')
#define YESNO(x) (std::cout << ((x) ? "YES" : "NO") << '\n')

inline void init_io(int precision = 30) {
  assert(precision >= 0);
  std::cin.tie(nullptr);
  std::ios::sync_with_stdio(false);
  std::cout << std::fixed << std::setprecision(precision);
}
template<class... Args> void scan(Args&... values) { (void)(std::cin >> ... >> values); }
template<class... Args> void print(const Args&... values) {
  bool first = true;
  [[maybe_unused]] auto one = [&](const auto& value) {
    if (!first) std::cout << ' ';
    first = false;
    std::cout << value;
  };
  (one(values), ...);
  std::cout << '\n';
}
