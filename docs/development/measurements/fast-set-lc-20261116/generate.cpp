#include <fstream>
#include <random>
#include <set>
#include <string>
#include <cstdlib>

int main(int argc, char** argv) {
  if (argc != 7) return 1;
  const int n = std::atoi(argv[1]), q = std::atoi(argv[2]);
  const std::string shape = argv[3];
  std::mt19937_64 rng(std::strtoull(argv[4], nullptr, 10));
  std::ofstream input(argv[5]), expected(argv[6]);
  if (!input || !expected || n < 1 || n > 10000000 || q < 1 || q > 1000000) return 2;
  std::string bits(n, '0');
  std::set<int> oracle;
  for (int i = 0; i < n; ++i) {
    const bool present = shape == "full" || (shape == "half" && (rng() & 1)) ||
      (shape == "sparse" && rng() % 1000 == 0) || (shape == "cluster" && ((i / 256) & 1));
    if (present) { bits[i] = '1'; oracle.insert(i); }
  }
  input << n << ' ' << q << '\n' << bits << '\n';
  for (int i = 0; i < q; ++i) {
    int type = shape == "updates" ? static_cast<int>(rng() % 3) : static_cast<int>(rng() % 5);
    int x = static_cast<int>(rng() % n);
    if (i % 101 == 0) x = 0;
    if (i % 103 == 0) x = n - 1;
    input << type << ' ' << x << '\n';
    if (type == 0) oracle.insert(x);
    else if (type == 1) oracle.erase(x);
    else if (type == 2) expected << (oracle.count(x) != 0) << '\n';
    else if (type == 3) { auto it = oracle.lower_bound(x); expected << (it == oracle.end() ? -1 : *it) << '\n'; }
    else { auto it = oracle.upper_bound(x); expected << (it == oracle.begin() ? -1 : *--it) << '\n'; }
  }
  input.close(); expected.close();
  return input && expected ? 0 : 3;
}
