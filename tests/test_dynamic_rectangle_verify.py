import os
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class DynamicRectangleVerifyTest(unittest.TestCase):
    def test_maximum_point_coordinates(self):
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "dynamic-rectangle-sum"
            cases = {
                "dynamic-rectangle-sum": ((
                    "4 2\n1000000000 0 3\n0 1000000000 5\n"
                    "1000000000 1000000000 11\n999999999 999999999 7\n"
                    "0 0 1000000000 1000000000\n"
                    "999999999 999999999 1000000000 1000000000\n"
                ), "7\n7\n"),
                "dynamic-point-add-rectangle-sum": ((
                    "1 7\n1000000000 0 3\n1 0 0 1000000000 1000000000\n"
                    "0 0 1000000000 5\n"
                    "0 1000000000 1000000000 11\n0 999999999 999999999 7\n"
                    "1 0 0 1000000000 1000000000\n"
                    "0 999999999 999999999 4\n"
                    "1 999999999 999999999 1000000000 1000000000\n"
                ), "0\n7\n11\n"),
            }
            for name, (input_data, expected) in cases.items():
                with self.subTest(driver=name):
                    subprocess.run([
                        os.environ.get("CXX", "g++"),
                        "-std=" + os.environ.get("CXX_STANDARD", "gnu++20"),
                        "-O2", "-Wall", "-Wextra", "-Werror", "-D_GLIBCXX_ASSERTIONS",
                        "-I", str(ROOT),
                        str(ROOT / f"verify/data-structure/{name}.test.cpp"),
                        "-o", str(executable),
                    ], check=True)
                    result = subprocess.run([str(executable)], input=input_data,
                                            text=True, capture_output=True, check=True, timeout=30)
                    self.assertEqual(result.stdout, expected)


if __name__ == "__main__":
    unittest.main()
