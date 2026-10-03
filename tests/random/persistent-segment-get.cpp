#include "blueberry/data-structure/persistent-segment-tree.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// No default construction, assignment, equality, ordering, or arithmetic on T.
struct Value {
  std::string text;
  Value() = delete;
  explicit Value(std::string x) : text(std::move(x)) {}
  Value(const Value&) = default;
  Value& operator=(const Value&) = delete;
};
struct Concat {
  bool reverse;
  std::uint64_t* calls;
  Concat() = delete;
  Concat(bool backward, std::uint64_t& counter) : reverse(backward), calls(&counter) {}
  Value operator()(const Value& left, const Value& right) const {
    ++*calls;
    return Value(reverse ? right.text + left.text : left.text + right.text);
  }
};
static_assert(std::is_copy_constructible_v<Value>);
static_assert(!std::is_default_constructible_v<Value>);
static_assert(!std::is_copy_assignable_v<Value>);
static_assert(!std::is_move_assignable_v<Value>);
static_assert(!std::is_default_constructible_v<Concat>);
using Tree = blueberry::PersistentSegmentTree<Value, Concat>;
using Sparse = std::map<int, std::string>;

std::uint64_t seed, checks = 0, operation_calls = 0;
const char* case_label = "";
int case_size = 0, step = -1;
bool case_reverse = false;

void require(bool ok, const char* operation, int version = -1, int p = -1) {
  ++checks;
  if (ok) return;
  std::cerr << "seed=" << seed << " case=" << case_label << " n=" << case_size
            << " reverse=" << case_reverse << " step=" << step
            << " operation=" << operation << " version=" << version << " p=" << p << '\n';
  std::abort();
}

void expect(const std::string& actual, const std::string& expected,
            const char* operation, int version, int p = -1) {
  if (actual != expected) std::cerr << "expected=[" << expected << "] actual=[" << actual << "]\n";
  require(actual == expected, operation, version, p);
}

void check_get(const Tree& tree, int version, int p, const std::string& expected) {
  const int versions = tree.versions();
  operation_calls = 0;
  Value result = tree.get(version, p);
  require(operation_calls == 0, "get invokes no Op", version, p);
  require(tree.versions() == versions, "get creates no version", version, p);
  expect(result.text, expected, "get", version, p);
  result.text += "changed return";
  expect(tree.get(version, p).text, expected, "returned copy independence", version, p);
  require(operation_calls == 0, "repeated get invokes no Op", version, p);
}

// The oracle concatenates array elements in ordinary index order (or its reverse),
// without using the tree's Op or recursive split points.
std::string vector_product(const std::vector<std::string>& values, int l, int r, bool reverse) {
  std::string result;
  if (reverse) {
    for (int p = r; p > l;) result += values[--p];
  } else {
    for (int p = l; p < r; ++p) result += values[p];
  }
  return result;
}

void check_vector_version(const Tree& tree, const std::vector<std::string>& values,
                          int version, bool reverse) {
  const int n = static_cast<int>(values.size());
  require(tree.size() == n, "size", version);
  for (int p = 0; p < n; ++p) check_get(tree, version, p, values[p]);
  expect(tree.all_prod(version).text, vector_product(values, 0, n, reverse), "all_prod", version);
  for (int l : {0, n / 2, n}) {
    for (int r : {0, n / 2, n}) if (l <= r) {
      expect(tree.prod(version, l, r).text, vector_product(values, l, r, reverse), "prod", version, l);
    }
  }
}

void small_case(int n, bool dense, bool reverse, std::mt19937_64& rng) {
  case_label = dense ? "array constructor" : "identity constructor";
  case_size = n; case_reverse = reverse; step = -1;
  std::vector<std::string> original(n);
  std::vector<Value> input;
  for (int p = 0; p < n; ++p) {
    if (dense && p % 3 != 1) original[p] = std::string(1, static_cast<char>('a' + p % 26));
    input.emplace_back(original[p]);
  }
  Concat op(reverse, operation_calls);
  Tree tree = dense ? Tree(input, op, Value("")) : Tree(n, op, Value(""));
  // Inputs and the runtime Op state are copied, not retained by reference.
  input.clear(); input.shrink_to_fit(); op.reverse = !reverse;
  std::vector<std::vector<std::string>> versions{original};
  check_vector_version(tree, versions[0], 0, reverse);
  require(tree.versions() == 1, "initial version count");
  if (n == 0) return;
  Value saved = tree.get(0, 0);

  // The first two updates are siblings. Later versions alternate fresh branches,
  // consecutive descendants, and arbitrary historical parents.
  for (step = 0; step < 128; ++step) {
    const int base = step < 2 || step % 4 == 0 ? 0
                   : step % 4 == 1 ? static_cast<int>(versions.size()) - 1
                   : static_cast<int>(rng() % versions.size());
    const int p = step < 2 ? 0 : static_cast<int>(rng() % static_cast<unsigned>(n));
    const bool apply = step >= 2 && step % 3 == 1;
    const std::string text = step % 11 == 0 ? "" : std::string(1, static_cast<char>('A' + step % 26));
    Value input_value(text);
    auto expected = versions[base];
    if (apply) expected[p] = reverse ? text + expected[p] : expected[p] + text;
    else expected[p] = text;
    const int version = apply ? tree.apply(base, p, input_value) : tree.set(base, p, input_value);
    input_value.text = "changed input";
    require(version == static_cast<int>(versions.size()), "new version id", version, p);
    versions.push_back(std::move(expected));
    require(tree.versions() == static_cast<int>(versions.size()), "version count", version);
    check_get(tree, version, p, versions[version][p]);
    check_get(tree, base, p, versions[base][p]);
    const int other = version > 2 ? 1 + step % 2 : 0;
    check_get(tree, other, p, versions[other][p]);
    if (step % 8 == 0) check_vector_version(tree, versions[version], version, reverse);
  }
  // Many appends grow the node/root pools, while every prior version and a
  // previously returned value must retain its contents.
  expect(saved.text, original[0], "saved return after pool growth", 0, 0);
  for (int version = 0; version < tree.versions(); ++version) {
    const int p = static_cast<int>(rng() % static_cast<unsigned>(n));
    check_get(tree, version, p, versions[version][p]);
  }
  Tree copied(tree);
  const int copied_versions = copied.versions();
  const int own_version = tree.set(0, 0, Value("original branch"));
  const int copy_version = copied.apply(0, 0, Value("copy branch"));
  require(copied.versions() == copied_versions + 1, "copy version isolation");
  require(tree.versions() == own_version + 1, "original version isolation");
  check_get(tree, own_version, 0, "original branch");
  check_get(copied, copy_version, 0, reverse ? "copy branch" + original[0] : original[0] + "copy branch");
  check_get(tree, 0, 0, original[0]); check_get(copied, 0, 0, original[0]);
  saved.text = "changed saved value";
  check_get(tree, 0, 0, original[0]);
}

std::string sparse_point(const Sparse& values, int p) {
  const auto found = values.find(p);
  return found == values.end() ? "" : found->second;
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

void sparse_limit_case(bool reverse, std::mt19937_64& rng) {
  const int n = std::numeric_limits<int>::max();
  case_label = "INT_MAX sparse"; case_size = n; case_reverse = reverse; step = -1;
  Tree tree(n, Concat(reverse, operation_calls), Value(""));
  std::vector<Sparse> versions(1);
  const std::vector<int> points{0, 1, 2, n / 2 - 1, n / 2, n / 2 + 1, n - 2, n - 1};
  const std::vector<int> missing{3, n / 4, n / 2 + 2, n - 3};
  for (int p : points) check_get(tree, 0, p, "");
  for (step = 0; step < 160; ++step) {
    const int base = step < 2 || step % 5 == 0 ? 0 : static_cast<int>(rng() % versions.size());
    const int p = step < 2 ? n / 2 : points[static_cast<unsigned>(step) % points.size()];
    const bool apply = step >= 2 && step % 3 == 1;
    const std::string text = step % 9 == 0 ? "" : std::string(1, static_cast<char>('a' + step % 26));
    Sparse expected = versions[base];
    if (apply) expected[p] = reverse ? text + expected[p] : expected[p] + text;
    else expected[p] = text;
    if (expected[p].empty()) expected.erase(p);
    const int version = apply ? tree.apply(base, p, Value(text)) : tree.set(base, p, Value(text));
    require(version == static_cast<int>(versions.size()), "sparse version id", version, p);
    versions.push_back(std::move(expected));
    check_get(tree, version, p, sparse_point(versions[version], p));
    check_get(tree, base, p, sparse_point(versions[base], p));
    for (int q : missing) check_get(tree, version, q, "");
  }
  for (int version = 0; version < tree.versions(); ++version) {
    for (int p : points) check_get(tree, version, p, sparse_point(versions[version], p));
    for (int p : missing) check_get(tree, version, p, "");
    expect(tree.all_prod(version).text, sparse_product(versions[version], 0, n, reverse), "sparse all_prod", version);
    for (int l : {0, n / 2, n - 1}) {
      expect(tree.prod(version, l, n).text, sparse_product(versions[version], l, n, reverse), "sparse prod", version, l);
    }
  }
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261003;
  std::mt19937_64 rng(seed);
  for (bool reverse : {false, true}) {
    for (bool dense : {false, true}) {
      for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33}) small_case(n, dense, reverse, rng);
    }
    sparse_limit_case(reverse, rng);
  }
  std::cout << "seed=" << seed << " checks=" << checks << '\n';
}
