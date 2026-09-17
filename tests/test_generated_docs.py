"""Deleting a library or verifier must also remove its cached source page."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "generate_docs", Path(__file__).resolve().parents[1] / "scripts/generate_docs.py")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class GeneratedDocumentationTest(unittest.TestCase):
    def test_removed_sources_are_pruned_without_touching_other_pages(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / ".verify-helper/markdown"
            names = ("blueberry/old.hpp.md", "blueberry/math/new.hpp.md",
                     "verify/math/old.test.cpp.md", "verify/math/new.test.cpp.md",
                     "migration.md", "blueberry/notes.md")
            for name in names:
                page = output / name
                page.parent.mkdir(parents=True, exist_ok=True)
                page.write_text("generated or static content")
            for name in ("blueberry/math/new.hpp", "verify/math/new.test.cpp"):
                source = root / name
                source.parent.mkdir(parents=True, exist_ok=True)
                source.write_text("source")
            self.assertCountEqual(MODULE.prune_removed_sources(root),
                                  ["blueberry/old.hpp.md", "verify/math/old.test.cpp.md"])
            for name in names:
                self.assertEqual((output / name).exists(), "old." not in name)
            self.assertEqual(MODULE.prune_removed_sources(root), [])

    def test_no_generated_directory_is_a_noop(self):
        with tempfile.TemporaryDirectory() as temporary:
            self.assertEqual(MODULE.prune_removed_sources(Path(temporary)), [])

    def test_external_output_is_not_removed(self):
        with tempfile.TemporaryDirectory() as temporary, tempfile.TemporaryDirectory() as external:
            root = Path(temporary)
            (root / ".verify-helper").mkdir()
            try:
                (root / ".verify-helper/markdown").symlink_to(external, target_is_directory=True)
            except OSError:
                self.skipTest("creating a directory symlink is unavailable")
            with self.assertRaises(ValueError):
                MODULE.prune_removed_sources(root)
