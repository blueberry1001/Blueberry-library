from pathlib import Path

here = Path(__file__).resolve().parent
code = (here/'degree2-16.hpp').read_text(encoding='utf-8')
code = code.replace('#include <algorithm>\n', '')
code = code.replace('  unsigned budget = std::min(1U << 16, 1U << ((n + 1) / 2));', '  unsigned budget = 1U << ((n + 1) / 2);\n  if (budget > (1U << 16)) budget = 1U << 16;')
(here/'final.hpp').write_text(code, encoding='utf-8', newline='\n')
target = here.parent/'speed-next-main/blueberry/graph/maximum-independent-set.hpp'
target.write_text(code, encoding='utf-8', newline='\n')
