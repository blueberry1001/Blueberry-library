#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <vector>
#include "blueberry/extra-fast/wavelet-matrix.hpp"

// Fail each allocation in the temporary copy; the destination must stay usable.
static int fail_after = -1;
void* operator new(std::size_t n) {
  if (fail_after == 0) throw std::bad_alloc();
  if (fail_after > 0) --fail_after;
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

int main() {
  std::vector<long long> input(4096);
  for (int i = 0; i < int(input.size()); ++i) input[i] = i % 97;
  const blueberry::extra_fast::WaveletMatrix<long long> source(input);
  for (int failure = 0; failure < 3; ++failure) {
    blueberry::extra_fast::WaveletMatrix<long long> target(std::vector<long long>(7, -7));
    bool failed = false;
    try {
      fail_after = failure;
      target = source;
    } catch (const std::bad_alloc&) {
      failed = true;
    }
    fail_after = -1;
    assert(failed);
    assert(target.size() == 7 && target.get(0) == -7);
    assert(target.kth_smallest(0, 7, 6) == -7);
    assert(target.count(0, 7, -7) == 7);
    assert(source.size() == 4096 && source.get(4095) == 4095 % 97);
    target = source;
    assert(target.size() == source.size() && target.get(4095) == input[4095]);
  }
}
