#include "blueberry/data-structure/fast-set.hpp"
#include "blueberry/utility/fast-io.hpp"
#include "trusted-input.hpp.txt"
#include <string>

#ifdef TRUSTED
using Input = TrustedInput;
template<class T> bool integer(Input& in, T& value) { return in.read_int(value); }
bool word(Input& in, std::string& value) { return in.read_word(value); }
#else
using Input = blueberry::FastInput<>;
template<class T> bool integer(Input& in, T& value) { return in.read(value); }
bool word(Input& in, std::string& value) { return in.read(value); }
#endif

int main() {
  Input in;
  blueberry::FastOutput out;
  int n, q;
  std::string bits;
  if (!integer(in, n) || !integer(in, q) || !word(in, bits)) return 1;
  blueberry::FastSet set(bits);
  while (q--) {
    int type, x;
    if (!integer(in, type) || !integer(in, x)) return 2;
    if (type == 0) set.insert(x);
    if (type == 1) set.erase(x);
    if (type == 2 && !out.writeln(set.contains(x))) return 3;
    if (type == 3) { int y = set.next(x); if (!out.writeln(y == n ? -1 : y)) return 3; }
    if (type == 4 && !out.writeln(set.prev(x))) return 3;
  }
  return out.flush() ? 0 : 4;
}
