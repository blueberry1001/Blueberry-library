#include <string>
#include "fast-set-assembly.hpp"

extern "C" [[gnu::noinline]] blueberry::FastSet* build_set(const std::string& bits) {
  return new blueberry::FastSet(bits);
}
extern "C" [[gnu::noinline]] int next_set(const blueberry::FastSet& tree, int x) {
  return tree.next(x);
}
extern "C" [[gnu::noinline]] int prev_set(const blueberry::FastSet& tree, int x) {
  return tree.prev(x);
}
