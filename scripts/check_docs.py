#!/usr/bin/env python3
"""Check catalog/API documentation and compile/run every minimal C++ example."""
import os
from pathlib import Path
import re
import shlex
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
    protected = re.findall(r"{% raw %}\s*```cpp\n(.*?)\n```\s*{% endraw %}", text, re.S)
    assert len(blocks) == len(protected), (document, "Protect C++ braces from Liquid using raw/endraw")
    assert blocks and "int main()" in blocks[0] and "assert(" in blocks[0], document
    examples.append((document.relative_to(ROOT), blocks[0]))

for name in ("guide", "migration"):
    guide = ROOT / f".verify-helper/docs/static/{name}.md"
    blocks = re.findall(r"```cpp\n(.*?)\n```", guide.read_text(), re.S)
    for index, block in enumerate(blocks):
        assert "int main()" in block and "assert(" in block, (guide, index)
        examples.append((f"{guide.relative_to(ROOT)} example {index + 1}", block))
compiler = os.environ.get("CXX", "g++")
standard = os.environ.get("CXX_STANDARD", "gnu++20")
compile_command = [
    compiler,
    *shlex.split(os.environ.get("CPPFLAGS", "")),
    f"-std={standard}",
    "-O2",
    "-Wall",
    "-Wextra",
    "-Werror",
    *shlex.split(os.environ.get("CXXFLAGS", "")),
    "-I",
    str(ROOT),
    "-x",
    "c++",
    "-",
]
with tempfile.TemporaryDirectory(prefix="blueberry-docs-") as temporary:
    for index, (document, source) in enumerate(examples):
        executable = Path(temporary) / str(index)
        subprocess.run(
            [*compile_command, "-o", str(executable)],
            input=source, text=True, check=True, timeout=60,
        )
        subprocess.run([str(executable)], check=True, timeout=10)
        print(f"PASS {document}", flush=True)
print(f"Checked {len(headers)} library docs and {len(examples)} executable examples.")
