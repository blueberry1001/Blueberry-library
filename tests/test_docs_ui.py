"""Run the browser-script behavior checks without third-party packages."""
import os
from pathlib import Path
import shutil
import subprocess
import unittest


class DocumentationUiTest(unittest.TestCase):
    def test_documentation_interactions(self):
        node = os.environ.get("NODE") or shutil.which("node")
        if not node:
            self.skipTest("Node.js is required for documentation interaction checks")
        subprocess.run(
            [node, str(Path(__file__).with_name("docs_ui_test.cjs"))],
            check=True, timeout=30,
        )
