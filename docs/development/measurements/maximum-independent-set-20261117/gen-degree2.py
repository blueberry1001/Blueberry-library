from pathlib import Path

here = Path(__file__).resolve().parent
for budget in (10, 12, 14, 16):
    p = here / f'branch{budget}.hpp'
    code = p.read_text(encoding='utf-8')
    code = code.replace('    if (degree <= 1) {\n      while (remaining) {\n        const int v = std::countr_zero(remaining);', '''    if (degree <= 2) {
      while (remaining) {
        int v = -1;
        for (std::uint64_t bits = remaining; bits; bits &= bits - 1) {
          if (budget == 0) return false;
          --budget;
          int candidate = std::countr_zero(bits);
          if (std::popcount(adj[candidate] & remaining) <= 1) { v = candidate; break; }
        }
        if (v == -1) v = std::countr_zero(remaining);''')
    (here / f'degree2-{budget}.hpp').write_text(code, encoding='utf-8', newline='\n')
