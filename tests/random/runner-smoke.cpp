#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "expected one seed argument\n";
    return 1;
  }
  const char* environment_seed = std::getenv("BLUEBERRY_RANDOM_SEED");
  if (environment_seed == nullptr || std::string(argv[1]) != environment_seed) {
    std::cerr << "argument and BLUEBERRY_RANDOM_SEED differ\n";
    return 1;
  }
  return 0;
}
