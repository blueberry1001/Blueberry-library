#!/usr/bin/env python3
"""Static regression checks against Jekyll output (no browser required)."""
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / "_site"
home = (SITE / "index.html").read_text()
assert 'class="site-hero"' in home
assert 'class="site-nav"' in home
assert "calc(100% - 300px)" not in home, "Upstream inline sidebar CSS returned"
assert home.index("</section>") < home.index("<footer>"), "Footer must follow the content"
for category in ("data-structure", "graph", "math", "string"):
    assert f"/Blueberry-library/categories/{category}.html" in home
    content = (SITE / "categories" / f"{category}.html").read_text()
    assert "収録ライブラリ" in content and f"/blueberry/{category}/" in content
for page in ("guide.html", "benchmarks.html", "assets/js/docs.js", "assets/css/custom.css"):
    assert (SITE / page).exists(), page

headers = re.findall(r"^  path: (.+)$", (ROOT / ".verify-helper/docs/static/_data/libraries.yml").read_text(), re.M)
for header in headers:
    content = (SITE / f"{header}.html").read_text()
    operations = re.findall(r'<details\b[^>]*class="api-operation"[^>]*>(.*?)</details>', content, re.S)
    assert operations, (header, "missing expandable API docs")
    for body in operations:
        assert "<summary>" in body and "注意点:" in body, header
        assert "<pre" in body and "<code" in body, (header, "Markdown inside details was not rendered")
        assert "```cpp" not in body, header

if "--without-metrics" in sys.argv[1:]:
    print(f"PASS: 4 categories, {len(headers)} API pages, footer and navigation (before verification).")
    sys.exit(0)

metrics = json.loads((SITE / "assets/verification-metrics.json").read_text())
assert metrics["succeeded"], "Do not publish failed measurements"
paths = {r["path"] for r in metrics["results"]}
expected = {p.relative_to(ROOT).as_posix() for p in (ROOT / "verify").rglob("*.test.cpp")}
assert paths == expected, "Every verify must have a measurement"
for row in metrics["results"]:
    content = (SITE / f"{row['path']}.html").read_text()
    assert "このverifyの実行時間" in content
    assert metrics["generated_at"] in content, "Missing measured data on verify page"
    assert row["status"] == "passed"
    assert len(row["runs"]) == row["repeat_count"]
    assert row["solution_seconds"] > 0 and row["compile_seconds"] > 0
print(f"PASS: 4 categories, {len(headers)} API pages, {len(paths)} measured verifies, footer and navigation.")
