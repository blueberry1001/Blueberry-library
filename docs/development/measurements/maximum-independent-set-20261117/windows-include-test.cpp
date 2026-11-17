#include <windows.h>
#include "candidate.hpp"
int main() {
  auto answer = blueberry::maximum_independent_set(1, {});
  return answer.size() != 1 || answer[0] != 0;
}
