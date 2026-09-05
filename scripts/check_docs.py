#!/usr/bin/env python3
"""Check catalog/API documentation and compile/run every minimal C++ example."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
catalog = ROOT / ".verify-helper/docs/static/_data/libraries.yml"
headers = re.findall(r"^  path: (.+)$", catalog.read_text(), re.M)
expected = {p.relative_to(ROOT).as_posix() for p in (ROOT / "blueberry").glob("*/*.hpp")}
assert len(headers) == len(set(headers)), "Duplicate catalog entries"
assert set(headers) == expected, "Register every modern header in libraries.yml"
examples = []
for header in headers:
    document = ROOT / ("docs/" + header.removeprefix("blueberry/").removesuffix(".hpp") + ".md")
    text = document.read_text()
    assert f"documentation_of: //{header}" in text, document
    for heading in ("## 概要・前提", "## 最小使用例", "## 操作一覧"):
        assert heading in text, (document, heading)
    details = re.findall(r'<details class="api-operation" id="([^"]+)" markdown="1">(.*?)</details>', text, re.S)
    ids = [identifier for identifier, _ in details]
    links = re.findall(r"\[開く\]\(#([^)]*)\)", text)
    assert details and len(ids) == len(set(ids)) and set(links) == set(ids), document
    for identifier, body in details:
        assert "<summary>" in body and "O(" in body and "注意点:" in body, (document, identifier)
        assert "```cpp" in body, (document, identifier)
    blocks = re.findall(r"```cpp\n(.*?)\n```", text, re.S)
    assert blocks and "int main()" in blocks[0] and "assert(" in blocks[0], document
    examples.append((document.relative_to(ROOT), blocks[0]))

guide = ROOT / ".verify-helper/docs/static/guide.md"
examples.append((guide.relative_to(ROOT), re.findall(r"```cpp\n(.*?)\n```", guide.read_text(), re.S)[0]))
with tempfile.TemporaryDirectory(prefix="blueberry-docs-") as temporary:
    for index, (document, source) in enumerate(examples):
        executable = Path(temporary) / str(index)
        subprocess.run(
            ["g++", "-std=gnu++20", "-O2", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT),
             "-x", "c++", "-", "-o", str(executable)],
            input=source, text=True, check=True, timeout=60,
        )
        subprocess.run([str(executable)], check=True, timeout=10)
        print(f"PASS {document}", flush=True)
print(f"Checked {len(headers)} library docs and {len(examples)} executable examples.")
