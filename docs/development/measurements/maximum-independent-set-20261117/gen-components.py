from pathlib import Path

here = Path(__file__).resolve().parent
code = (here/'final.hpp').read_text(encoding='utf-8')
old = '''  if (search(search, (std::uint64_t{1} << n) - 1, 0, 0)) {
    std::vector<int> result;
    for (int v = 0; v < n; ++v) if ((branch_answer >> v) & 1) result.push_back(v);
    return result;
  }
'''
new = '''  std::uint64_t remaining = (std::uint64_t{1} << n) - 1, branch_result = 0;
  bool complete = true;
  while (remaining) {
    std::uint64_t component = 0, frontier = std::uint64_t{1} << std::countr_zero(remaining);
    while (frontier) {
      const int v = std::countr_zero(frontier);
      const auto bit = std::uint64_t{1} << v;
      frontier &= ~bit;
      remaining &= ~bit;
      component |= bit;
      frontier |= adj[v] & remaining;
    }
    branch_answer = 0;
    branch_size = 0;
    if (!search(search, component, 0, 0)) { complete = false; break; }
    branch_result |= branch_answer;
  }
  if (complete) {
    std::vector<int> result;
    for (int v = 0; v < n; ++v) if ((branch_result >> v) & 1) result.push_back(v);
    return result;
  }
'''
if code.count(old) != 1: raise RuntimeError('candidate format changed')
code = code.replace(old, new)
(here/'components.hpp').write_text(code, encoding='utf-8', newline='\n')
for shift in (1,2,3):
    sample = code.replace('  if (budget > (1U << 16)) budget = 1U << 16;', f'  if (budget > (1U << 16)) budget = 1U << 16;\n  if (n <= 28) budget >>= {shift};\n  if (budget == 0) budget = 1;')
    (here/f'components-scaled{1<<shift}.hpp').write_text(sample, encoding='utf-8', newline='\n')
