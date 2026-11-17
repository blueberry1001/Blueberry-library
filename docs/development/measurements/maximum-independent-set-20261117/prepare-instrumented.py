from pathlib import Path

here = Path(__file__).resolve().parent
code = (here/'final.hpp').read_text(encoding='utf-8')
code = code.replace('namespace blueberry {', 'namespace blueberry {\ninline bool budget_exhausted = false;')
code = code.replace('  unsigned budget =', '  budget_exhausted = false;\n  unsigned budget =')
code = code.replace('if (budget == 0) return false;', 'if (budget == 0) { budget_exhausted = true; return false; }')
(here/'instrumented.hpp').write_text(code, encoding='utf-8', newline='\n')
