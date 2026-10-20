import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
FACADES = ('contest-template', 'contest-fast-io')
FORBIDDEN = ('blueberry/math/factorize.hpp', 'blueberry/math/combinations.hpp',
             'blueberry/math/prime-sieve.hpp', 'blueberry/graph/topological-sort.hpp')


def external_flags():
    result = []
    for flag in shlex.split(os.environ.get('CPPFLAGS', '-I.deps/ac-library')):
        if flag == '-I':
            result.append('-isystem')
        elif flag.startswith('-I'):
            result.extend(('-isystem', flag[2:]))
        else:
            result.append(flag)
    return result


class TemplateCompatibilityTest(unittest.TestCase):
    def test_original_calls_and_stream_containers(self):
        compilers = [name for name in ('g++', 'clang++') if shutil.which(name)]
        if not compilers:
            self.skipTest('GCC or Clang is required')
        flags = external_flags()
        with tempfile.TemporaryDirectory() as directory:
            for compiler in compilers:
                for standard in ('gnu++20', 'gnu++23'):
                    for facade in FACADES:
                        for debug in (False, True):
                            with self.subTest(compiler=compiler, standard=standard, facade=facade, debug=debug):
                                binary = Path(directory) / 'compatibility'
                                command = [compiler, f'-std={standard}', '-O2', '-Wall', '-Wextra', '-Werror', '-I.', *flags,
                                           f'-DTEMPLATE_HEADER="blueberry/utility/{facade}.hpp"']
                                if facade == 'contest-fast-io':
                                    command.append('-DFAST_FACADE')
                                if debug:
                                    command.append('-DBLUEBERRY=0')
                                command += ['tests/fixtures/template-compatibility.cpp', '-o', str(binary)]
                                compiled = subprocess.run(command, cwd=ROOT, text=True, capture_output=True, timeout=90)
                                self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
                                result = subprocess.run([str(binary)], input='7 5 word 1 2 3 4\n',
                                                        text=True, capture_output=True, timeout=10)
                                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                                expected = ('1 4 5\n' if debug else '') + '12 word 1 2 3 4\n1.25\nYes\nNO\n'
                                self.assertEqual(result.stdout, expected)

    def test_all_header_include_orders(self):
        compilers = [name for name in ('g++', 'clang++') if shutil.which(name)]
        if not compilers:
            self.skipTest('GCC or Clang is required')
        for compiler in compilers:
            for facade in FACADES:
                for reverse in (False, True):
                    with self.subTest(compiler=compiler, facade=facade, reverse=reverse):
                        headers = ['blueberry/all.hpp', f'blueberry/utility/{facade}.hpp']
                        if reverse:
                            headers.reverse()
                        source = ''.join(f'#include "{header}"\n' for header in headers) + 'int main(){}\n'
                        result = subprocess.run([compiler, '-std=gnu++20', '-Wall', '-Wextra', '-Werror',
                                                 '-fsyntax-only', '-I.', *external_flags(), '-x', 'c++', '-'],
                                                input=source, cwd=ROOT, text=True, capture_output=True, timeout=90)
                        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_io_dependency_closure(self):
        compiler = next((name for name in ('g++', 'clang++') if shutil.which(name)), None)
        if compiler is None:
            self.skipTest('GCC or Clang is required')
        flags = external_flags()
        for header in (*FACADES, 'fast-io'):
            with self.subTest(header=header):
                result = subprocess.run([compiler, '-std=gnu++20', '-MM', '-I.', *flags, '-x', 'c++', '-'],
                                        input=f'#include "blueberry/utility/{header}.hpp"\n',
                                        cwd=ROOT, text=True, capture_output=True, check=True, timeout=60)
                dependencies = result.stdout.replace('\\\n', ' ').split()
                for forbidden in FORBIDDEN:
                    self.assertNotIn(forbidden, dependencies)
                if header == 'fast-io':
                    self.assertEqual([path for path in dependencies if path.startswith('blueberry/')],
                                     ['blueberry/utility/fast-io.hpp'])


if __name__ == '__main__':
    unittest.main()
