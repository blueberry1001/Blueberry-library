#!/usr/bin/env python3
"""Static regression checks against Jekyll output (no browser required)."""
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / ".build/site"
home = (SITE / "index.html").read_text()
# The landing page lists only this library, not vendored ACL or verify sources.
home_links = re.findall(r'href="([^"]+)"', home)
assert not any('/atcoder/' in href or '/verify/' in href for href in home_links), "ACL/verify leaked onto homepage"
assert "Verification Files" not in home
assert 'class="site-hero"' in home
assert 'class="site-nav"' in home
assert 'data-catalog-search' in home and 'data-catalog-controls hidden' in home
assert 'data-catalog-empty hidden' in home and 'id="library-catalog"' in home
assert "calc(100% - 300px)" not in home, "Upstream inline sidebar CSS returned"
assert home.index("</section>") < home.index("<footer>"), "Footer must follow the content"
for category in ("data-structure", "graph", "math", "string"):
    assert f"/Blueberry-library/categories/{category}.html" in home
    content = (SITE / "categories" / f"{category}.html").read_text()
    assert "収録ライブラリ" in content and f"/blueberry/{category}/" in content
    assert 'data-catalog-search' in content, (category, "missing catalog search")
for page in ("guide.html", "migration.html", "benchmarks.html", "assets/js/docs.js", "assets/css/custom.css"):
    assert (SITE / page).exists(), page

# Discovery pages must remain usable without JavaScript and have live targets.
operations = (SITE / "operations.html").read_text()
coverage = (SITE / "library-checker.html").read_text()
inventory = json.loads((ROOT / ".verify-helper/docs/static/_data/library_checker.json").read_text())
assert 'data-operation-controls hidden' in operations and 'data-operation-count' in operations
assert 'data-coverage-controls hidden' in coverage and 'data-coverage-count' in coverage
assert 'data-operation-empty hidden' in operations and 'data-coverage-empty hidden' in coverage
assert not re.search(r'<article\b[^>]*data-operation-entry[^>]*\bhidden\b', operations)
assert not re.search(r'<tr\b[^>]*data-coverage-row[^>]*\bhidden\b', coverage)
assert len(re.findall(r'<tr\b[^>]*data-coverage-row\b', coverage)) == inventory["total"]
for problem in inventory["problems"]:
    assert coverage.count('href="' + problem["url"] + '"') == 1, problem["id"]
for content in (operations, coverage):
    assert 'aria-live="polite"' in content
    for href in re.findall(r'href="(/Blueberry-library/[^"#]*\.html)"', content):
        assert (SITE / href.removeprefix('/Blueberry-library/')).is_file(), ("discovery page", href)
for page in ("operations.html", "library-checker.html"):
    assert f'/Blueberry-library/{page}' in home, (page, "missing home navigation")

migration = (SITE / "migration.html").read_text()
for link in re.findall(r'href="(/Blueberry-library/[^"#]*\.html)"', migration):
    assert (SITE / link.removeprefix('/Blueberry-library/')).is_file(), ("migration", link)
for obsolete in ("ConvexHulltrick.hpp", "DynamicFenwickTree2D.hpp", "implicit_treap.hpp",
                 "fraction.hpp", "Graph.hpp", "RollbackUnionFind.hpp", "fps.hpp"):
    assert not (ROOT / "blueberry" / obsolete).exists(), (obsolete, "obsolete source restored")
    assert not (SITE / "blueberry" / f"{obsolete}.html").exists(), (obsolete, "stale generated page")
    assert not any(f'/blueberry/{obsolete}.html' in href for href in home_links), obsolete

headers = re.findall(r"^  path: (.+)$", (ROOT / ".verify-helper/docs/static/_data/libraries.yml").read_text(), re.M)
catalog_rows = re.findall(r'<tr data-library\b[^>]*>.*?</tr>', home, re.S)
assert len(catalog_rows) == len(headers), "Search catalog must contain every modern header exactly once"
for header in headers:
    assert sum(f'/{header}.html' in row for row in catalog_rows) == 1, (header, "missing or duplicate search result")
assert not re.search(r'<tr data-library\b[^>]*\bhidden\b', home), "Catalog must work without JavaScript"
for header in headers:
    content = (SITE / f"{header}.html").read_text()
    # Hand-written API prose is rendered under the header URL, not docs/*.md.
    # Validate its cross-links as well as helper-generated dependency links.
    for href in re.findall(r'href="([^"]+)"', content):
        url = urlsplit(href)
        path = unquote(url.path)
        if url.scheme or url.netloc or not path.endswith((".html", ".md")):
            continue
        if path.startswith('/Blueberry-library/'):
            target = SITE / path.removeprefix('/Blueberry-library/')
        elif not path.startswith('/'):
            target = (SITE / f"{header}.html").parent / path
        else:
            continue
        assert target.is_file(), (header, href, "broken API documentation link")
    assert 'assets/js/copy-button.js' not in content, (header, "upstream controls duplicate copy buttons and bundle unrelated examples")
    assert 'id="bundled-source"' in content, (header, "missing expandable bundled source")
    relations = re.search(r'<div class="source-relations">(.*?)</div>', content, re.S)
    assert relations, (header, "missing source relationships")
    for link in re.findall(r'href="([^"]+)"', relations[1]):
        assert link.startswith('/Blueberry-library/') and link.endswith('.html'), (header, link)
        assert (SITE / link.removeprefix('/Blueberry-library/')).exists(), (header, link, "broken dependency/verify link")
    operations = re.findall(r'<details\b[^>]*class="api-operation"[^>]*>(.*?)</details>', content, re.S)
    assert operations, (header, "missing expandable API docs")
    for body in operations:
        assert "<summary>" in body and "注意点:" in body, header
        assert "<pre" in body and "<code" in body, (header, "Markdown inside details was not rendered")
        assert "```cpp" not in body, header

# Every direct library include in a verify has an inline, nonempty source example.
for source in (ROOT / "verify").rglob("*.test.cpp"):
    path = source.relative_to(ROOT).as_posix()
    includes = re.findall(r'#include "(blueberry/[^"\n]+)"', source.read_text())
    for header in includes:
        content = (SITE / f"{header}.html").read_text()
        match = re.search(r'<details[^>]*data-verify="' + re.escape(path) + r'"[^>]*>(.*?)</details>', content, re.S)
        assert match, (header, path, "missing related verify")
        assert '<code' in match[1] and 'PROBLEM' in match[1], (header, path, "empty source")
        assert f'/Blueberry-library/{path}.html' in match[1], (header, "missing verify link")

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
