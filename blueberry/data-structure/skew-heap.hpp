#pragma once
#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>

namespace blueberry {
// Owning, move-only meldable heap. Compare=less selects the minimum.
template <class T, class Compare = std::less<T>>
class SkewHeap {
  struct Node { T value; Node* left = nullptr; Node* right = nullptr; };
  Node* root_ = nullptr;
  std::size_t size_ = 0;
  Compare compare_;
  Node* meld_nodes(Node* a, Node* b) {
    Node* result = nullptr;
    Node** link = &result;
    // Iterative equivalent of meld(right,b), then swap(left,right).
    while (a && b) {
      if (compare_(b->value, a->value)) std::swap(a, b);
      *link = a;
      Node* next = a->right;
      a->right = a->left; link = &a->left; a = next;
    }
    *link = a ? a : b;
    return result;
  }
 public:
  explicit SkewHeap(Compare compare = {}) : compare_(std::move(compare)) {}
  SkewHeap(const SkewHeap&) = delete;
  SkewHeap& operator=(const SkewHeap&) = delete;
  SkewHeap(SkewHeap&& other)
      : root_(std::exchange(other.root_, nullptr)), size_(std::exchange(other.size_, 0)), compare_(other.compare_) {}
  SkewHeap& operator=(SkewHeap&& other) {
    if (this != &other) {
      clear(); compare_ = other.compare_;
      root_ = std::exchange(other.root_, nullptr); size_ = std::exchange(other.size_, 0);
    }
    return *this;
  }
  ~SkewHeap() { clear(); }
  std::size_t size() const { return size_; }
  bool empty() const { return root_ == nullptr; }
  const T& top() const { assert(!empty()); return root_->value; }
  void push(const T& value) { root_ = meld_nodes(root_, new Node{value}); ++size_; }
  void pop() {
    assert(!empty());
    Node* old = root_; root_ = meld_nodes(old->left, old->right); delete old; --size_;
  }
  void meld(SkewHeap& other) {
    if (this == &other) return;
    root_ = meld_nodes(root_, other.root_); size_ += other.size_;
    other.root_ = nullptr; other.size_ = 0;
  }
  void clear() {
    // Rotate away left links to destroy arbitrarily deep heaps without recursion.
    while (root_) {
      if (root_->left) {
        Node* child = root_->left; root_->left = child->right;
        child->right = root_; root_ = child;
      } else { Node* next = root_->right; delete root_; root_ = next; }
    }
    size_ = 0;
  }
};
}  // namespace blueberry
