// Independent comparison of compressed and dynamically allocated Li Chao trees.
#include "blueberry/data-structure/li-chao-tree.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using Integer = long long;

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
  void add_line(Integer a, Integer b) { insert(root_, -1000000, 1000001, {a, b}); }
  void add_segment(Integer l, Integer r, Integer a, Integer b) {
    segment(root_, -1000000, 1000001, l, r, {a, b});
  }
  std::optional<Integer> query(Integer x) const {
    auto* node = root_.get();
    Integer l = -1000000, r = 1000001;
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
  if (n <= 0 || (candidate != "compressed" && candidate != "dynamic") ||
      (workload != "line" && workload != "segment")) return 2;
  std::mt19937_64 random(std::strtoull(argv[4], nullptr, 10));
  std::vector<Integer> xs;
  std::vector<Operation> operations;
  for (int i = 0; i < n; ++i) {
    const Integer center = static_cast<Integer>(random() % 2000001) - 1000000;
    Integer l = static_cast<Integer>(random() % 2000002) - 1000000;
    Integer r = static_cast<Integer>(random() % 2000002) - 1000000;
    if (l > r) std::swap(l, r);
    const Integer x = static_cast<Integer>(random() % 2000001) - 1000000;
    xs.push_back(x);
    operations.push_back({l, r, -2 * center, center * center, x});
  }
  if (candidate == "compressed") measure<blueberry::LiChaoTree<>>(xs, operations, workload == "segment");
  else measure<DynamicCandidate>(xs, operations, workload == "segment");
}
