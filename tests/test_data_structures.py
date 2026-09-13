from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class DataStructureRandomTest(unittest.TestCase):
    def test_randomized_against_naive_implementations(self):
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "data-structure-random-test"
            subprocess.run(
                [
                    "g++",
                    "-std=gnu++20",
                    "-O2",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-D_GLIBCXX_ASSERTIONS",
                    "-I",
                    str(ROOT),
                    str(ROOT / "tests/data_structure_random_test.cpp"),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            subprocess.run([str(executable)], check=True, timeout=30)


if __name__ == "__main__":
    unittest.main()
