import os
import pathlib
import subprocess
import tempfile
import unittest


class TreeRandomTest(unittest.TestCase):
    def test_random_and_deep_tree(self):
        root = pathlib.Path(__file__).resolve().parents[1]
        source = root / "tests" / "tree_random.cpp"
        with tempfile.TemporaryDirectory() as temporary_directory:
            executable = pathlib.Path(temporary_directory) / "tree_random"
            subprocess.run(
                [
                    os.environ.get("CXX", "g++"),
                    "-std=" + os.environ.get("CXX_STANDARD", "gnu++20"),
                    "-O2",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(root),
                    str(source),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            subprocess.run([str(executable)], check=True, timeout=30)


if __name__ == "__main__":
    unittest.main()
