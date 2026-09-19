#!/usr/bin/env python3
"""Check all documentation structure, then compile selected or all examples."""
import argparse
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def collect_examples(root):
    """Validate the complete catalog and return (path, label, source) examples."""
    catalog = root / ".verify-helper/docs/static/_data/libraries.yml"
    headers = re.findall(r"^  path: (.+)$", catalog.read_text(encoding="utf-8"), re.M)
    expected = {p.relative_to(root).as_posix() for p in (root / "blueberry").glob("*/*.hpp")}
    assert len(headers) == len(set(headers)), "Duplicate catalog entries"
    assert set(headers) == expected, "Register every modern header in libraries.yml"
    examples = []
    for header in headers:
        document = root / ("docs/" + header.removeprefix("blueberry/").removesuffix(".hpp") + ".md")
        text = document.read_text(encoding="utf-8")
        assert f"documentation_of: //{header}" in text, document
        for heading in ("## 概要・前提", "## 最小使用例", "## 操作一覧"):
            assert heading in text, (document, heading)
        details = re.findall(r'<details class="api-operation" id="([^"]+)" markdown="1">(.*?)</details>', text, re.S)
        ids = [identifier for identifier, _ in details]
        links = re.findall(r"\[開く\]\(#([^)]*)\)", text)
        assert details and len(ids) == len(set(ids)) and set(links) == set(ids), document
        for identifier, body in details:
            assert "<summary>" in body and "O(" in body and "注意点:" in body, (document, identifier)
            summary = re.search(r"<summary>(.*?)</summary>", body, re.S)
            assert summary, (document, identifier, "Close the summary element")
            for code in re.findall(r"<code>(.*?)</code>", summary[1], re.S):
                assert "<" not in code and ">" not in code, (document, identifier, "Escape template brackets in HTML code")
            assert "```cpp" in body, (document, identifier)
        blocks = re.findall(r"```cpp\n(.*?)\n```", text, re.S)
        protected = re.findall(r"{% raw %}\s*```cpp\n(.*?)\n```\s*{% endraw %}", text, re.S)
        assert len(blocks) == len(protected), (document, "Protect C++ braces from Liquid using raw/endraw")
        assert blocks and "int main()" in blocks[0] and "assert(" in blocks[0], document
        path = document.relative_to(root).as_posix()
        examples.append((path, path, blocks[0]))

    for name in ("guide", "migration"):
        guide = root / f".verify-helper/docs/static/{name}.md"
        blocks = re.findall(r"```cpp\n(.*?)\n```", guide.read_text(encoding="utf-8"), re.S)
        path = guide.relative_to(root).as_posix()
        for index, block in enumerate(blocks):
            assert "int main()" in block and "assert(" in block, (guide, index)
            examples.append((path, f"{path} example {index + 1}", block))
    return headers, examples


def selected_paths(root, paths_file, examples):
    """Require canonical repository paths for documented library/guide pages."""
    paths = json.loads(Path(paths_file).read_text(encoding="utf-8"))
    if not isinstance(paths, list) or any(not isinstance(path, str) for path in paths):
        raise ValueError("--paths-file must contain a JSON array of document path strings")
    allowed = {path for path, _, _ in examples}
    allowed.update(f".verify-helper/docs/static/{name}.md" for name in ("guide", "migration"))
    unknown = set(paths) - allowed
    if unknown:
        raise ValueError(f"Unknown or non-document selected paths: {sorted(unknown)}")
    return set(paths)


def compile_examples(root, examples):
    compiler = os.environ.get("CXX", "g++")
    standard = os.environ.get("CXX_STANDARD", "gnu++20")
    compile_command = [
        compiler,
        *shlex.split(os.environ.get("CPPFLAGS", "")),
        f"-std={standard}", "-O2", "-Wall", "-Wextra", "-Werror",
        *shlex.split(os.environ.get("CXXFLAGS", "")),
        "-I", str(root), "-x", "c++", "-",
    ]
    with tempfile.TemporaryDirectory(prefix="blueberry-docs-") as temporary:
        for index, (_, document, source) in enumerate(examples):
            executable = Path(temporary) / str(index)
            subprocess.run(
                [*compile_command, "-o", str(executable)],
                input=source, text=True, check=True, timeout=60,
            )
            subprocess.run([str(executable)], check=True, timeout=10)
            print(f"PASS {document}", flush=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--structure-only", action="store_true", help="Validate all structure without compiling examples")
    parser.add_argument("--paths-file", type=Path, help="JSON array of repository document paths whose examples to compile")
    args = parser.parse_args(argv)
    headers, examples = collect_examples(ROOT)
    selected = examples
    if args.paths_file is not None:
        paths = selected_paths(ROOT, args.paths_file, examples)
        selected = [example for example in examples if example[0] in paths]
    if args.structure_only:
        selected = []
    if selected:
        compile_examples(ROOT, selected)
    print(f"Checked {len(headers)} library docs and {len(selected)} executable examples.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
