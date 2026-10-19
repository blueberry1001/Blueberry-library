// Compile with -S and an alternate snapshot header to inspect only our update.
#ifndef BLUEBERRY_EXTRA_FAST_HEADER
#define BLUEBERRY_EXTRA_FAST_HEADER "blueberry/extra-fast/segment-tree.hpp"
#endif
#include BLUEBERRY_EXTRA_FAST_HEADER

extern "C" void extra_fast_update(blueberry::extra_fast::SumSegmentTree& tree,
                                  int position, std::uint64_t value) {
  tree.add(position, value);
}
