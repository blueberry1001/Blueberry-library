"""Keep the requested personal template intact through Git, rendering and use."""
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest

from scripts.generate_docs import write_original_template_data


ROOT = Path(__file__).resolve().parents[1]
ASSET = Path('.verify-helper/docs/static/assets/code/blueberrytemplate.h')
ORIGINAL_SHA256 = '7a5b26b201eeaba1bc4fff4eca7e48ce13409ea3a0d96ce19ef6691e13781cba'


class OriginalTemplateTest(unittest.TestCase):
    def test_original_bytes_and_checkout_preservation(self):
        content = (ROOT / ASSET).read_bytes()
        self.assertEqual(len(content), 9361)
        self.assertEqual(hashlib.sha256(content).hexdigest(), ORIGINAL_SHA256)
        self.assertFalse(content.endswith(b'\n'))
        attributes = subprocess.check_output(['git', '-C', str(ROOT), 'check-attr', 'text', '--', str(ASSET)], text=True)
        self.assertIn('text: unset', attributes)

    def test_liquid_data_preserves_every_character(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / ASSET).parent.mkdir(parents=True)
            shutil.copyfile(ROOT / ASSET, root / ASSET)
            data = write_original_template_data(root)
            loaded = json.loads((root / '.verify-helper/markdown/_data/original_template.json').read_text())
            self.assertEqual(data, loaded)
            self.assertEqual(data['code'].encode('utf-8'), (ROOT / ASSET).read_bytes())
            self.assertEqual(data['sha256'], ORIGINAL_SHA256)

    def test_original_names_with_user_defined_main(self):
        source = '''#include ".verify-helper/docs/static/assets/code/blueberrytemplate.h"
#include <cassert>
void _main() {
    LL(a, b); STR(s);
    assert(a == 7 && b == 5 && s == "word");
    ll total = 0;
    rep(3) total += i;
    rep(j, 2) total += j;
    rep(j, 2, 4) total += j;
    rep(j, 0, 4, 2) total += j;
    assert(total == 11);
    vector<ll> values{3, 1, 3};
    assert(min(values) == 1 && max(values) == 3 && sum(values) == 7);
    assert(zaatu(values) == 2 && (values == vector<ll>{1, 0, 1}));
    assert(divide(-7, 3) == -3 && ran(1, 3) == 6);
    COMinit(16, 998244353);
    assert(COM(5, 2, 998244353) == 10 && nPr(5, 2, 998244353) == 20);
    UnionFind uf(3); uf.merge(0, 1);
    assert(uf.connected(0, 1) && uf.find(0) == uf.find(1) && uf.size(0) == 2);
    cout << a + b << spa << s << Endl;
}
'''
        flags = shlex.split(os.environ.get('CPPFLAGS', '-I.deps/ac-library'))
        compilers = [compiler for compiler in ('g++', 'clang++') if shutil.which(compiler)]
        if not compilers:
            self.skipTest('GCC or Clang is required for the original template smoke test')
        with tempfile.TemporaryDirectory() as temporary:
            for compiler in compilers:
                for standard in ('gnu++20', 'gnu++23'):
                    with self.subTest(compiler=compiler, standard=standard):
                        binary = Path(temporary) / (compiler.replace('+', 'p') + standard)
                        subprocess.run([compiler, f'-std={standard}', '-O2', '-I.', *flags,
                                        '-x', 'c++', '-', '-o', str(binary)], input=source, text=True,
                                       cwd=ROOT, check=True, capture_output=True, timeout=60)
                        result = subprocess.run([str(binary)], input='7 5 word\n', text=True,
                                                capture_output=True, check=True, timeout=5)
                        self.assertEqual(result.stdout, '12 word\n')


if __name__ == '__main__':
    unittest.main()
