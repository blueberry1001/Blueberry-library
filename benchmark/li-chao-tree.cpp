// Independent comparison of compressed and dynamically allocated Li Chao trees.
#include "blueberry/data-structure/li-chao-tree.hpp"
#include "blueberry/data-structure/dynamic-li-chao-tree.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using Integer = long long;
Integer domain_low = -1000000, domain_high = 1000001;

class DynamicCandidate {
  struct Line {
    Integer a, b;
    Integer eval(Integer x) const { return a * x + b; }
  };
  struct Node {
    std::optional<Line> line;
    std::unique_ptr<Node> left, right;
  };
  std::unique_ptr<Node> root_;

  static void insert(std::unique_ptr<Node>& node, Integer l, Integer r, Line line) {
    if (!node) node = std::make_unique<Node>();
    if (!node->line) {
      node->line = line;
      return;
    }
    const bool at_left = line.eval(l) < node->line->eval(l);
    const bool at_right = line.eval(r - 1) < node->line->eval(r - 1);
    if (at_left == at_right) {
      if (at_left) node->line = line;
      return;
    }
    const Integer m = l + (r - l) / 2;
    const bool at_middle = line.eval(m) < node->line->eval(m);
    if (at_middle) std::swap(line, *node->line);
    if (at_left != at_middle) insert(node->left, l, m, line);
    else insert(node->right, m, r, line);
  }

  static void segment(std::unique_ptr<Node>& node, Integer l, Integer r,
                      Integer begin, Integer end, Line line) {
    if (r <= begin || end <= l) return;
    if (begin <= l && r <= end) {
      insert(node, l, r, line);
      return;
    }
    if (!node) node = std::make_unique<Node>();
    const Integer m = l + (r - l) / 2;
    segment(node->left, l, m, begin, end, line);
    segment(node->right, m, r, begin, end, line);
  }

 public:
  explicit DynamicCandidate(const std::vector<Integer>&) {}
  void add_line(Integer a, Integer b) { insert(root_, domain_low, domain_high, {a, b}); }
  void add_segment(Integer l, Integer r, Integer a, Integer b) {
    segment(root_, domain_low, domain_high, l, r, {a, b});
  }
  std::optional<Integer> query(Integer x) const {
    auto* node = root_.get();
    Integer l = domain_low, r = domain_high;
    std::optional<Integer> answer;
    while (node) {
      if (node->line) {
        const auto y = node->line->eval(x);
        if (!answer || y < *answer) answer = y;
      }
      if (r - l == 1) break;
      const Integer m = l + (r - l) / 2;
      if (x < m) { node = node->left.get(); r = m; }
      else { node = node->right.get(); l = m; }
    }
    return answer;
  }
};

class ArenaCandidate : public blueberry::DynamicLiChaoTree<Integer> {
 public:
  explicit ArenaCandidate(const std::vector<Integer>&)
      : blueberry::DynamicLiChaoTree<Integer>(domain_low, domain_high) {}
};

struct Operation { Integer l, r, a, b, x; };

template <class Tree>
void measure(const std::vector<Integer>& xs, const std::vector<Operation>& operations,
             bool segments) {
  const auto start = std::chrono::steady_clock::now();
  Tree tree(xs);
  std::uint64_t checksum = 0;
  for (const auto& op : operations) {
    if (segments) tree.add_segment(op.l, op.r, op.a, op.b);
    else tree.add_line(op.a, op.b);
    const auto answer = tree.query(op.x);
    checksum = checksum * 1000003 + (answer ? static_cast<std::uint64_t>(*answer) : 12345);
  }
  const auto end = std::chrono::steady_clock::now();
  std::cout << "milliseconds=" << std::chrono::duration<double, std::milli>(end - start).count()
            << " checksum=" << checksum << '\n';
}

int main(int argc, char** argv) {
  if (argc != 5) return 2;
  const std::string candidate = argv[1], workload = argv[2];
  const int n = std::stoi(argv[3]);
  if (n <= 0 || (candidate != "compressed" && candidate != "dynamic" && candidate != "arena") ||
      (workload != "line" && workload != "segment" && workload != "sparse-line" &&
       workload != "narrow-segment")) return 2;
  if (workload == "sparse-line") {
    domain_low = -1000000000000LL;
    domain_high = 1000000000001LL;
  }
  std::mt19937_64 random(std::strtoull(argv[4], nullptr, 10));
  std::vector<Integer> xs;
  std::vector<Operation> operations;
  for (int i = 0; i < n; ++i) {
    const Integer center = static_cast<Integer>(random() % 2000001) - 1000000;
    Integer l = static_cast<Integer>(random() % 2000002) - 1000000;
    Integer r = static_cast<Integer>(random() % 2000002) - 1000000;
    if (l > r) std::swap(l, r);
    if (workload == "narrow-segment") r = std::min(l + static_cast<Integer>(random() % 32), domain_high);
    const Integer x = static_cast<Integer>(random() % (domain_high - domain_low)) + domain_low;
    xs.push_back(x);
    operations.push_back({l, r, -2 * center, center * center, x});
  }
  const bool segments = workload == "segment" || workload == "narrow-segment";
  if (candidate == "compressed") measure<blueberry::LiChaoTree<>>(xs, operations, segments);
  else if (candidate == "arena") measure<ArenaCandidate>(xs, operations, segments);
  else measure<DynamicCandidate>(xs, operations, segments);
}
