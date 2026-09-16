#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "blueberry/string/manacher.hpp"

// Independent candidate: materialize separators, then use one mirrored pass.
std::vector<int> transformed(const std::string& text) {
  const int n = static_cast<int>(text.size());
  if (!n) return {};
  const int m = 2 * n - 1;
  std::vector<int> symbols(m), radius(m), answer(m);
  for (int i = 0; i < n; ++i) symbols[2 * i] = static_cast<unsigned char>(text[i]) + 1;
  int left = 0, right = -1;
  for (int i = 0; i < m; ++i) {
    if (i <= right) radius[i] = std::min(right - i, radius[left + right - i]);
    while (i - radius[i] > 0 && i + radius[i] + 1 < m &&
           symbols[i - radius[i] - 1] == symbols[i + radius[i] + 1]) ++radius[i];
    if (i + radius[i] > right) {
      left = i - radius[i];
      right = i + radius[i];
    }
    answer[i] = 2 * ((radius[i] + (i & 1)) / 2) + 1 - (i & 1);
  }
  return answer;
}

std::vector<int> naive(const std::string& text) {
  const int n = static_cast<int>(text.size());
  std::vector<int> answer(n ? 2 * n - 1 : 0);
  for (int center = 0; center < 2 * n - 1; ++center) {
    int l = center / 2, r = (center + 1) / 2;
    while (l >= 0 && r < n && text[l] == text[r]) {
      answer[center] = r - l + 1;
      --l;
      ++r;
    }
  }
  return answer;
}

template <class Function>
void measure(const char* name, const std::string& text, const std::string& shape,
             const std::vector<int>& expected, Function function) {
  std::vector<double> samples;
  std::uint64_t checksum = 0;
  if (function(text) != expected) throw std::runtime_error("candidate mismatch");
  for (int run = 0; run < 7; ++run) {
    const auto start = std::chrono::steady_clock::now();
    const auto answer = function(text);
    const auto stop = std::chrono::steady_clock::now();
    samples.push_back(std::chrono::duration<double, std::milli>(stop - start).count());
    for (int value : answer) checksum += value;
  }
  std::cout << name << ',' << shape << ',' << text.size() << ',' << checksum;
  for (double sample : samples) std::cout << ',' << sample;
  std::sort(samples.begin(), samples.end());
  std::cout << ',' << samples[3] << ',' << samples.front() << '\n';
}

int main() {
  std::cout << "compiler=" << __VERSION__ << "\nflags=-std=c++20 -O2 -DNDEBUG -I.\n"
            << "seed=20260914 runs=7 warmups=1 unit=ms\n"
            << "candidate,shape,n,checksum,run1,run2,run3,run4,run5,run6,run7,median,min\n"
            << std::fixed << std::setprecision(6);
  std::mt19937_64 random(20260914);
  for (int n : {6000, 1000000}) {
    for (const std::string shape : {"random26", "equal", "alternating"}) {
      std::string text(n, 'a');
      for (int i = 0; i < n; ++i) {
        if (shape == "random26") text[i] = static_cast<char>('a' + random() % 26);
        if (shape == "alternating") text[i] = static_cast<char>('a' + i % 2);
      }
      const auto expected = blueberry::manacher(text);
      measure("two-pass", text, shape, expected, blueberry::manacher<std::string>);
      measure("transformed", text, shape, expected, transformed);
      if (n <= 6000) measure("naive", text, shape, expected, naive);
    }
  }
}
