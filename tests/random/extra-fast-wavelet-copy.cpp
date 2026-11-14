#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <vector>
#include "blueberry/extra-fast/wavelet-matrix.hpp"

// Fail each observed allocation in the temporary copy; its exact allocation
// count may vary with the standard library and compiler optimizations.
static volatile int fail_after = -1;
void* operator new(std::size_t n) {
  const int remaining = fail_after;
  if (remaining == 0) throw std::bad_alloc();
  if (remaining > 0) fail_after = remaining - 1;
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

int main() {
  std::vector<long long> input(4096);
  for (int i = 0; i < int(input.size()); ++i) input[i] = i % 97;
  const blueberry::extra_fast::WaveletMatrix<long long> source(input);
  int failed_copies = 0;
  bool completed = false;
  for (int failure = 0; failure < 64; ++failure) {
    blueberry::extra_fast::WaveletMatrix<long long> target(std::vector<long long>(7, -7));
    bool failed = false;
    try {
      fail_after = failure;
      target = source;
    } catch (const std::bad_alloc&) {
      failed = true;
    }
    fail_after = -1;
    if (!failed) {
      // A compiler may elide allocations, but zero observed failures would
      // make this test vacuous: do not silently accept that configuration.
      assert(failed_copies > 0);
      assert(target.size() == source.size() && target.get(4095) == input[4095]);
      assert(target.count(0, 4096, 0) == source.count(0, 4096, 0));
      completed = true;
      break;
    }
    ++failed_copies;
    assert(target.size() == 7 && target.get(0) == -7);
    assert(target.kth_smallest(0, 7, 6) == -7);
    assert(target.count(0, 7, -7) == 7);
    assert(source.size() == 4096 && source.get(4095) == 4095 % 97);
    target = source;
    assert(target.size() == source.size() && target.get(4095) == input[4095]);
  }
  assert(completed);
}
