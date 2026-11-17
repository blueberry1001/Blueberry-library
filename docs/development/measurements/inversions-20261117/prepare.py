from pathlib import Path
root = Path('.build/inversions-experiment')
src = (root / 'baseline.hpp').read_text().replace('namespace baseline {', 'namespace blueberry {')
old = '  const int block = std::max(1, static_cast<int>(std::sqrt(std::max(1, n))));'
width = '  const int block = std::max(1, static_cast<int>(n / std::sqrt(std::max(1, q))));'
compare = '    int bx = queries[x].first / block, by = queries[y].first / block;'
precompute = '  std::vector<int> blocks(q);\n  for (int i = 0; i < q; ++i) blocks[i] = queries[i].first / block;'
assert old in src and compare in src
for name, adapt, pre in [('baseline', False, False), ('width', True, False), ('blocks', False, True), ('both', True, True), ('guarded', False, False), ('shortguard', False, False)]:
    s = src.replace('namespace blueberry {', f'namespace {name} {{')
    if adapt: s = s.replace(old, width)
    if name == 'guarded': s = s.replace(old, '  const int block = std::max(1, static_cast<int>(q < n ? n / std::sqrt(std::max(1, q)) : std::sqrt(std::max(1, n))));')
    if name == 'shortguard': s = s.replace(old, '  int block = std::max(1, static_cast<int>(std::sqrt(std::max(1, n))));\n  if (q < n) for (auto [l, r] : queries) if (r - l > block) { block = std::max(block, static_cast<int>(n / std::sqrt(std::max(1, q)))); break; }')
    if pre: s = s.replace('  std::sort(order.begin()', precompute + '\n  std::sort(order.begin()').replace(compare, '    int bx = blocks[x], by = blocks[y];')
    (root / (name + '.hpp')).write_text(s)
