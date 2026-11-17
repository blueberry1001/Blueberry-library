import hashlib
import json
from pathlib import Path

here = Path(__file__).resolve().parent
data = json.loads((here/'results-final.json').read_text(encoding='utf-8'))
code = (here/'test.cpp').read_text(encoding='utf-8')
code = code.replace('  for (int g = 0; g < 7; ++g) edges.emplace_back(5 * g, 5 * g + 5);\n  check(40, edges, 8);\n', '')
raw = code.encode()
if hashlib.sha256(raw).hexdigest() != data['test_sha256']: raise RuntimeError('test mismatch')
(here/'test-pre-components.cpp').write_bytes(raw)
data = json.loads((here/'validation-final.json').read_text(encoding='utf-8'))
code = (here/'fallback-test.cpp').read_text(encoding='utf-8')
code = code.replace('  for (int g = 0; g < 7; ++g) edges.emplace_back(5 * g, 5 * g + 5);\n', '')
raw = code.encode()
if hashlib.sha256(raw).hexdigest() != data['fallback_test_sha256']: raise RuntimeError('fallback test mismatch')
(here/'fallback-test-pre-components.cpp').write_bytes(raw)
