#include "blueberry/extra-fast/static-rmq.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

std::uint64_t seed;

template <class T>
void query(const blueberry::extra_fast::StaticRangeMin<T>& rmq,
           const std::vector<T>& values, int l, int r) {
  T expected = std::numeric_limits<T>::max();
  for (int p = l; p < r; ++p) expected = std::min(expected, values[p]);
  const T actual = rmq.prod(l, r);
  if (actual != expected) {
    std::cerr << "seed=" << seed << " signed=" << std::is_signed_v<T>
              << " n=" << values.size() << " l=" << l << " r=" << r
              << " expected=" << expected << " actual=" << actual << " input=";
    for (T value : values) std::cerr << value << ',';
    std::cerr << '\n';
    std::exit(1);
  }
}

template <class T>
void test(std::mt19937_64& rng) {
  using RMQ = blueberry::extra_fast::StaticRangeMin<T>;
  for (bool use_simd : {false, true}) {
    for (int n : {0, 1, 2, 7, 8, 9, 15, 16, 17, 23, 24, 31, 32, 33,
                  63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513,
                  1023, 1024, 1025, 4095, 4096, 4097, 65535, 65536, 65537}) {
      for (int distribution = 0; distribution < 5; ++distribution) {
        std::vector<T> values(n);
        for (int i = 0; i < n; ++i) {
          if (distribution == 0) values[i] = static_cast<T>(rng());
          else if (distribution == 1) values[i] = static_cast<T>(i);
          else if (distribution == 2) values[i] = static_cast<T>(n - i);
          else if (distribution == 3) values[i] = std::numeric_limits<T>::max();
          else values[i] = i % 3 == 0 ? std::numeric_limits<T>::lowest()
                         : i % 3 == 1 ? std::numeric_limits<T>::max() : T(0);
        }
        RMQ rmq(values, use_simd);
        assert(rmq.size() == n);
        assert(rmq.uses_simd() == (use_simd && RMQ::simd_available()));
        for (int i = 0; i < n; ++i) assert(rmq.get(i) == values[i]);
        query(rmq, values, 0, n);
        assert(rmq.all_prod() == rmq.prod(0, n));
        if (n <= 129) {
          for (int l = 0; l <= n; ++l)
            for (int r = l; r <= n; ++r) query(rmq, values, l, r);
        } else {
          for (int step = 0; step < 160; ++step) {
            int l = static_cast<int>(rng() % (n + 1));
            int r = static_cast<int>(rng() % (n + 1));
            if (l > r) std::swap(l, r);
            query(rmq, values, l, r);
            // SIMD loads must never read past the last element, even for tails.
            for (int length : {0, 1, 7, 8, 9, 15, 16}) {
              query(rmq, values, l, std::min(n, l + length));
              query(rmq, values, n - length, n);
            }
          }
          for (int boundary = 16; boundary < std::min(n, 2048); boundary += 16)
            for (int l = boundary - 8; l <= boundary + 1; ++l)
              for (int r = boundary; r <= std::min(n, boundary + 17); ++r)
                if (l <= r) query(rmq, values, l, r);
        }
        RMQ copied(rmq), assigned;
        assigned = rmq;
        auto* same = &assigned;
        assigned = *same;
        RMQ moved(std::move(copied));
        RMQ move_assigned(std::vector<T>{T(1), T(2)});
        move_assigned = std::move(assigned);
        for (const RMQ* other : {&moved, &move_assigned}) {
          query(*other, values, 0, n);
          if (n) query(*other, values, n / 2, n);
        }
        // Move leaves an empty valid object; self-move preserves the object.
        assert(copied.size() == 0 && assigned.size() == 0);
        assert(copied.prod(0, 0) == std::numeric_limits<T>::max());
        auto* self_moved = &moved;
        moved = std::move(*self_moved);
        query(moved, values, 0, n);
        RMQ owned(std::move(values), use_simd);
        assert(owned.all_prod() == rmq.all_prod());
      }
    }
  }
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  test<std::int32_t>(rng);
  test<std::uint32_t>(rng);
}
