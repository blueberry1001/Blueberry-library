from pathlib import Path

here = Path(__file__).resolve().parent
test = (here/'test.cpp').read_text(encoding='utf-8')
test = test.replace('#include "candidate.hpp"', '#include "blueberry/graph/maximum-independent-set.hpp"')
target = here.parent/'speed-next-main/tests/random/maximum-independent-set-branch.cpp'
target.write_text(test, encoding='utf-8', newline='\n')
