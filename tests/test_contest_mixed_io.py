import os
from pathlib import Path
import queue
import random
import shlex
import shutil
import subprocess
import tempfile
import threading
import unittest


ROOT = Path(__file__).resolve().parents[1]
MASK = (1 << 64) - 1


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


def checksum(values):
    result = 0
    for value in values:
        result = (result * 1000003 + value) & MASK
    return result


class ContestMixedIoTest(unittest.TestCase):
    def test_shared_input_output_and_stream_operations(self):
        compilers = [name for name in ('g++', 'clang++') if shutil.which(name)]
        if not compilers:
            self.skipTest('GCC or Clang is required')
        rng = random.Random(967718)
        numbers = [rng.randrange(-10**12, 10**12) for _ in range(45000)]
        number_hash = checksum(numbers)
        characters = ''.join(chr(rng.randrange(33, 127)) for _ in range(150000))
        character_hash = checksum(map(ord, characters))
        low, high = -(1 << 127), (1 << 127) - 1
        nested = [low, high, -123456789012345678901234567890, 0, 1, -1, 7, 23]
        nested_text = ' '.join(map(str, nested))
        cases = [
            ('reproducer', '2 3 7 23', '7 23\n'),
            ('reproducer', '0 3', '\n'),
            ('alternate', f'{len(numbers)}\n' + ' '.join(map(str, numbers)),
             f'{number_hash}\n{number_hash}\n'),
            ('nested', f'19 {nested_text} 1 0 1 0 {low}',
             f'19 {nested_text} 1 0 1 0 {low}\n{nested_text}\n'),
            ('custom', '3 7 word 23', '3 7 word 23\n'),
            ('cin-first', '2 7 23 71', '7 23 71\n'),
            ('line', '2 \talpha beta\n23', '2 97 a alpha beta 23\n'),
            ('chars', f'{len(characters)}\n{characters} 71', f'{character_hash} 71\n'),
            ('state', '1 broken 9', '1 1 broken 9\n1 1\n1 1\n1\n'),
            ('bool', 'true false 29', '1 0 29\n'),
            ('order', '', 'ab\nc\nd\n'),
            ('print-lookup', '', 'word 7\nliteral 7\nword 2 3\nfixed 9\nword 7\n\n5\nword 5\n'),
            ('output-large', '24000', ''.join(f'{i} {-i}\n' for i in range(24000))),
            ('precision', '2 7 23 9', '7 23 1.250\n2.500\n9 3.8\n4.3\n'),
        ]
        fin_nested = f'{high} -7 0 23'
        fin_cases = [
            ('fin-reproducer', '2 3 7 23', '7 23\n'),
            ('fin-types', f'2 3 7 23 1 0 1 0 -5 9 11 13 17 word Z -1.25 {low} {fin_nested}',
             f'1 0 2 3 7 23 1 0 1 0 -5 9 11 13 17 word Z -1.250 {low} {fin_nested}\n'),
            ('fin-alternate', f'{len(numbers)}\n' + ' '.join(map(str, numbers)),
             f'{number_hash} 1 0\n'),
            ('fin-fail', '7 broken 23 71',
             '1 0 1 1 7 8 9\n0 1\n1 0 23\n71\n0 1 1 71\n'),
            ('fin-scan-fail', 'broken 23', '0 1 0 11\n1 0 23\n'),
            ('fin-precision', '1.25 3.76', '1.2500 3.7600\n1.2500 3.7600\n'),
        ]
        with tempfile.TemporaryDirectory() as directory:
            for compiler in compilers:
                for standard in ('gnu++20', 'gnu++23'):
                    for facade in ('contest-fast-io', 'contest-template'):
                        with self.subTest(compiler=compiler, standard=standard, facade=facade):
                            binary = Path(directory) / 'mixed-io'
                            command = [compiler, f'-std={standard}', '-O2', '-Wall', '-Wextra', '-Werror',
                                       '-I.', *external_flags(),
                                       f'-DTEMPLATE_HEADER="blueberry/utility/{facade}.hpp"']
                            if facade == 'contest-fast-io':
                                command.append('-DFAST_FACADE')
                            command += ['tests/fixtures/contest-mixed-io.cpp', '-o', str(binary)]
                            compiled = subprocess.run(command, cwd=ROOT, text=True, capture_output=True, timeout=90)
                            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
                            selected_cases = cases + (fin_cases if facade == 'contest-fast-io' else [])
                            for mode, data, expected in selected_cases:
                                with self.subTest(mode=mode):
                                    result = subprocess.run([str(binary), mode], input=data, text=True,
                                                            capture_output=True, timeout=15)
                                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                                    self.assertEqual(result.stdout, expected)
                            self.check_endl_flush(binary)

    def check_endl_flush(self, binary):
        process = subprocess.Popen([str(binary), 'flush'], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True)
        lines = queue.Queue()

        def read_ready():
            lines.put(process.stdout.readline())
            lines.put(process.stdout.readline())

        reader = threading.Thread(target=read_ready, daemon=True)
        reader.start()
        try:
            self.assertEqual(lines.get(timeout=5), 'pending\n')
            self.assertEqual(lines.get(timeout=5), '\n')
            stdout, stderr = process.communicate('17', timeout=10)
            self.assertEqual(process.returncode, 0, stdout + stderr)
            self.assertEqual(stdout, '17\n')
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()
            reader.join(timeout=1)


if __name__ == '__main__':
    unittest.main()
