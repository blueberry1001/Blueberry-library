#include "blueberry/data-structure/persistent-segment-tree.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <vector>

// Select either immutable snapshot through the compiler's include search path.
// These checks remain active under -DNDEBUG and perform no timed measurements.
struct Concat {
  bool reverse;
  std::string operator()(const std::string& left, const std::string& right) const {
    return reverse ? right + left : left + right;
  }
};
using Tree = blueberry::PersistentSegmentTree<std::string, Concat>;
using Sparse = std::map<int, std::string>;

std::uint64_t checksum = 14695981039346656037ULL;
std::uint64_t checks = 0;
std::string context;

void require(bool ok, const char* operation) {
  ++checks;
  if (!ok) {
    std::cerr << "FAIL " << context << " operation=" << operation << '\n';
    std::exit(1);
  }
}

void expect(const std::string& actual, const std::string& expected,
            const char* operation) {
  if (actual != expected) {
    std::cerr << "actual=[" << actual << "] expected=[" << expected << "]\n";
  }
  require(actual == expected, operation);
  for (unsigned char byte : actual) {
    checksum ^= byte;
    checksum *= 1099511628211ULL;
  }
  checksum ^= 255;  // Separate adjacent results, including empty results.
  checksum *= 1099511628211ULL;
}

std::uint64_t next_random(std::uint64_t& state) {
  state ^= state << 13;
  state ^= state >> 7;
  state ^= state << 17;
  return state;
}

// Flat iteration is independent of the tree's split points and query recursion.
std::string vector_product(const std::vector<std::string>& values, int l, int r,
                           bool reverse) {
  std::string result;
  if (reverse) {
    for (int p = r; p > l;) result += values[--p];
  } else {
    for (int p = l; p < r; ++p) result += values[p];
  }
  return result;
}

void check_vector_version(const Tree& tree,
                          const std::vector<std::string>& values,
                          int version, bool reverse) {
  const int n = static_cast<int>(values.size());
  context += " version=" + std::to_string(version);
  require(tree.size() == n, "size");
  expect(tree.all_prod(version), vector_product(values, 0, n, reverse), "all_prod");
  for (int p = 0; p < n; ++p) expect(tree.get(version, p), values[p], "get");
  for (int l = 0; l <= n; ++l) {
    for (int r = l; r <= n; ++r) {
      expect(tree.prod(version, l, r), vector_product(values, l, r, reverse), "prod");
    }
  }
}

void small_case(int n, bool dense, bool reverse) {
  std::vector<std::string> initial(n);
  if (dense) {
    for (int p = 0; p < n; ++p) {
      if (p % 3 != 1) initial[p] = std::string(1, static_cast<char>('a' + p));
    }
  }
  const auto original = initial;
  Concat operation{reverse};
  Tree tree = dense ? Tree(initial, operation, "") : Tree(n, operation, "");
  // Both constructor inputs are copied; state is specific to this tree instance.
  operation.reverse = !reverse;
  initial.clear();
  initial.shrink_to_fit();
  std::vector<std::vector<std::string>> versions{original};
  auto label = [&] {
    return "small n=" + std::to_string(n) + " dense=" + std::to_string(dense)
           + " reverse=" + std::to_string(reverse);
  };
  context = label();
  require(tree.versions() == 1, "initial versions");
  check_vector_version(tree, versions[0], 0, reverse);
  if (n == 0) return;

  std::uint64_t state = 0x7319d249ba54e807ULL;
  for (int step = 0; step < 96; ++step) {
    const int base = step % 5 == 0 ? 0 : static_cast<int>(next_random(state) % versions.size());
    const int p = step < n ? step : static_cast<int>(next_random(state) % static_cast<unsigned>(n));
    const bool apply = step % 3 == 1;
    const std::string value = step % 7 == 0 ? "" : std::string(1, static_cast<char>('A' + step % 26));
    auto expected = versions[base];
    if (apply) expected[p] = reverse ? value + expected[p] : expected[p] + value;
    else expected[p] = value;
    const int version = apply ? tree.apply(base, p, value) : tree.set(base, p, value);
    context = label() + " step=" + std::to_string(step) + " base=" + std::to_string(base);
    require(version == static_cast<int>(versions.size()), "appended version id");
    versions.push_back(expected);
    require(tree.versions() == static_cast<int>(versions.size()), "versions");
    check_vector_version(tree, versions.back(), version, reverse);
    context = label() + " prior-base step=" + std::to_string(step);
    check_vector_version(tree, versions[base], base, reverse);
  }
  for (int version = 0; version < tree.versions(); ++version) {
    context = label() + " retained history";
    check_vector_version(tree, versions[version], version, reverse);
  }
}

std::string sparse_product(const Sparse& values, int l, int r, bool reverse) {
  std::string result;
  const auto first = values.lower_bound(l), last = values.lower_bound(r);
  if (reverse) {
    auto it = last;
    while (it != first) result += (--it)->second;
  } else {
    for (auto it = first; it != last; ++it) result += it->second;
  }
  return result;
}

void sparse_limit_case(bool reverse) {
  const int n = std::numeric_limits<int>::max();
  Tree tree(n, Concat{reverse}, "");
  std::vector<Sparse> versions(1);
  const std::vector<int> positions{0, 1, 2, n / 2 - 1, n / 2, n / 2 + 1, n - 2, n - 1};
  const std::vector<int> endpoints{0, 1, 2, 3, n / 2 - 1, n / 2, n / 2 + 1,
                                   n / 2 + 2, n - 2, n - 1, n};
  std::uint64_t state = 0x3625cb782be96341ULL;
  for (int step = 0; step < 80; ++step) {
    const int base = step % 4 == 0 ? 0 : static_cast<int>(next_random(state) % versions.size());
    const int p = positions[static_cast<std::size_t>(step) % positions.size()];
    const bool apply = step % 2 == 0;
    const std::string value = step % 9 == 0 ? "" : std::string(1, static_cast<char>('a' + step % 26));
    Sparse expected = versions[base];
    if (apply) expected[p] = reverse ? value + expected[p] : expected[p] + value;
    else expected[p] = value;
    if (expected[p].empty()) expected.erase(p);
    const int version = apply ? tree.apply(base, p, value) : tree.set(base, p, value);
    context = "INT_MAX reverse=" + std::to_string(reverse) + " step=" + std::to_string(step);
    require(version == static_cast<int>(versions.size()), "appended version id");
    versions.push_back(expected);
  }
  for (int version = 0; version < tree.versions(); ++version) {
    context = "INT_MAX reverse=" + std::to_string(reverse) + " version=" + std::to_string(version);
    require(tree.size() == n, "size");
    require(tree.versions() == static_cast<int>(versions.size()), "versions");
    expect(tree.all_prod(version), sparse_product(versions[version], 0, n, reverse), "all_prod");
    for (int p : positions) {
      expect(tree.get(version, p), sparse_product(versions[version], p, p + 1, reverse), "get");
    }
    // Untouched interior paths must remain the identity in every version.
    expect(tree.get(version, n / 4), "", "missing sparse point");
    expect(tree.prod(version, 3, n / 2 - 1), "", "missing sparse interval");
    for (int l : endpoints) {
      for (int r : endpoints) {
        if (l <= r) {
          expect(tree.prod(version, l, r), sparse_product(versions[version], l, r, reverse), "prod");
        }
      }
    }
  }
}

int main() {
  for (bool reverse : {false, true}) {
    for (bool dense : {false, true}) {
      for (int n : {0, 1, 7, 11}) small_case(n, dense, reverse);
    }
    sparse_limit_case(reverse);
  }
  std::cout << "persistent-contract PASS checks=" << checks << " checksum=" << checksum << '\n';
}
