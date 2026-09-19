"""Incremental documentation compilation must retain complete structural checks."""
import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

SPEC = importlib.util.spec_from_file_location("check_docs", Path(__file__).resolve().parents[1] / "scripts/check_docs.py")
check_docs = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(check_docs)
SOURCE = "#include <cassert>\nint main() { assert(1 == 1); }"


def page(header, source=SOURCE):
    return (
        f"documentation_of: //{header}\n## 概要・前提\n## 最小使用例\n"
        "{% raw %}\n```cpp\n" + source + "\n```\n{% endraw %}\n"
        "## 操作一覧\n[開く](#get)\n"
        '<details class="api-operation" id="get" markdown="1">\n'
        "<summary>get — O(1)</summary>\n注意点: bounds\n"
        "{% raw %}\n```cpp\nget();\n```\n{% endraw %}\n</details>\n"
    )


class CheckDocsSelectionTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.write(".verify-helper/docs/static/_data/libraries.yml", "- name: a\n  path: blueberry/test/a.hpp\n- name: b\n  path: blueberry/test/b.hpp\n")
        for name in ("a", "b"):
            self.write(f"blueberry/test/{name}.hpp", "#pragma once\n")
            self.write(f"docs/test/{name}.md", page(f"blueberry/test/{name}.hpp"))
        for name, count in (("guide", 1), ("migration", 3)):
            self.write(f".verify-helper/docs/static/{name}.md", ("```cpp\n" + SOURCE + "\n```\n") * count)
        self.root_patch = patch.object(check_docs, "ROOT", self.root)
        self.root_patch.start()
        self.addCleanup(self.root_patch.stop)

    def write(self, path, content):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content, encoding="utf-8")
        return target

    def selection(self, paths):
        return str(self.write("selection.json", json.dumps(paths)))

    def run_main(self, args):
        with contextlib.redirect_stdout(io.StringIO()):
            return check_docs.main(args)

    def test_default_compiles_every_example_once(self):
        with patch.object(check_docs, "compile_examples") as compile_examples:
            self.run_main([])
        examples = compile_examples.call_args.args[1]
        self.assertEqual(len(examples), 6)
        self.assertEqual(len({label for _, label, _ in examples}), 6)

    def test_structure_only_never_launches_compiler(self):
        with patch.object(check_docs.subprocess, "run") as run:
            self.run_main(["--structure-only"])
        run.assert_not_called()

    def test_empty_selection_never_launches_compiler(self):
        with patch.object(check_docs.subprocess, "run") as run:
            self.run_main(["--paths-file", self.selection([])])
        run.assert_not_called()

    def test_library_selection_excludes_other_pages(self):
        with patch.object(check_docs, "compile_examples") as compile_examples:
            self.run_main(["--paths-file", self.selection(["docs/test/b.md"])])
        self.assertEqual([x[0] for x in compile_examples.call_args.args[1]], ["docs/test/b.md"])

    def test_migration_selection_keeps_all_three_examples(self):
        migration = ".verify-helper/docs/static/migration.md"
        with patch.object(check_docs, "compile_examples") as compile_examples:
            self.run_main(["--paths-file", self.selection([migration, migration])])
        examples = compile_examples.call_args.args[1]
        self.assertEqual([x[0] for x in examples], [migration] * 3)
        self.assertEqual([x[1] for x in examples], [f"{migration} example {i}" for i in (1, 2, 3)])

    def test_structure_outside_selection_still_fails(self):
        self.write("docs/test/a.md", "broken structure")
        for args in (["--structure-only"], ["--paths-file", self.selection(["docs/test/b.md"])]):
            with self.subTest(args=args), self.assertRaises(AssertionError):
                self.run_main(args)

    def test_summary_template_brackets_must_be_html_escaped(self):
        document = page("blueberry/test/a.hpp")
        for signature, valid in (("vector<int>", False), ("vector&lt;int&gt;", True)):
            with self.subTest(signature=signature):
                self.write("docs/test/a.md", document.replace("get — O(1)", f"<code>{signature}</code> — O(1)"))
                if valid:
                    self.run_main(["--structure-only"])
                else:
                    with self.assertRaisesRegex(AssertionError, "Escape template brackets"):
                        self.run_main(["--structure-only"])

    def test_unknown_non_document_and_noncanonical_paths_fail(self):
        for path in ("docs/test/missing.md", "blueberry/test/a.hpp", "docs/test/../test/a.md", "/docs/test/a.md", "docs\\test\\a.md"):
            with self.subTest(path=path), self.assertRaises(ValueError):
                self.run_main(["--paths-file", self.selection([path])])

    def test_selection_schema_is_strict(self):
        for invalid in ({}, "docs/test/a.md", [1], [None]):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                self.run_main(["--paths-file", self.selection(invalid)])

    @unittest.skipUnless(shutil.which(os.environ.get("CXX", "g++")), "C++ compiler required")
    def test_selected_invalid_cpp_rejected_by_real_compiler(self):
        self.write("docs/test/a.md", page("blueberry/test/a.hpp", "#include <cassert>\nint main() { assert(undeclared_identifier); }"))
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_main(["--paths-file", self.selection(["docs/test/a.md"])])

    @unittest.skipUnless(shutil.which(os.environ.get("CXX", "g++")), "C++ compiler required")
    def test_only_selected_valid_cpp_runs_with_real_compiler(self):
        self.write("docs/test/a.md", page("blueberry/test/a.hpp", "#include <cassert>\nint main() { assert(undeclared_identifier); }"))
        self.assertEqual(self.run_main(["--paths-file", self.selection(["docs/test/b.md"])]), 0)


if __name__ == "__main__":
    unittest.main()
