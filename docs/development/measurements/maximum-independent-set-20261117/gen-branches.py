from pathlib import Path

here = Path(__file__).resolve().parent
packed = (here / 'packed.hpp').read_text(encoding='utf-8')
branch = '''  std::uint64_t branch_answer = 0;
  int branch_size = 0;
  unsigned budget = std::min(1U << BUDGET_BITS, 1U << ((n + 1) / 2));
  auto search = [&](auto&& self, std::uint64_t remaining, std::uint64_t chosen, int count) -> bool {
    if (budget == 0) return false;
    --budget;
    if (count + std::popcount(remaining) <= branch_size) return true;
    int vertex = -1, degree = -1;
    for (std::uint64_t bits = remaining; bits; bits &= bits - 1) {
      if (budget == 0) return false;
      --budget;
      int v = std::countr_zero(bits), d = std::popcount(adj[v] & remaining);
      if (d > degree) { vertex = v; degree = d; }
    }
    if (degree <= 1) {
      while (remaining) {
        const int v = std::countr_zero(remaining);
        chosen |= std::uint64_t{1} << v;
        remaining &= ~((std::uint64_t{1} << v) | adj[v]);
        ++count;
      }
      if (count > branch_size) { branch_size = count; branch_answer = chosen; }
      return true;
    }
    const auto bit = std::uint64_t{1} << vertex;
    return self(self, remaining & ~(bit | adj[vertex]), chosen | bit, count + 1) &&
           self(self, remaining & ~bit, chosen, count);
  };
  if (search(search, (std::uint64_t{1} << n) - 1, 0, 0)) {
    std::vector<int> result;
    for (int v = 0; v < n; ++v) if ((branch_answer >> v) & 1) result.push_back(v);
    return result;
  }
'''
for budget in (10, 12, 14, 16):
    code = packed.replace('#include <bit>', '#include <algorithm>\n#include <bit>')
    code = code.replace('  const int a = n / 2, b = n - a;', branch.replace('BUDGET_BITS', str(budget)) + '  const int a = n / 2, b = n - a;')
    (here / f'branch{budget}.hpp').write_text(code, encoding='utf-8', newline='\n')
