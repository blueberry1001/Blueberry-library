#include "blueberry/utility/fast-io.hpp"
int main() {
  blueberry::FastInput<true> in;
  blueberry::FastOutput out;
  long long value;
  while (in.read(value)) if (!out.writeln(value + 1) || !out.flush()) return 1;
}
