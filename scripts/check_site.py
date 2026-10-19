#!/usr/bin/env python3
"""Static regression checks against Jekyll output (no browser required)."""
import json
import html
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import unquote, urlsplit

from verify_with_metrics import validate_plan_against_repository, validate_publication_report

ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / ".build/site"
home = (SITE / "index.html").read_text()
# Every page using the shared head must initialize the existing GA4 stream once.
for page in SITE.rglob("*.html"):
    content = page.read_text(encoding="utf-8")
    if '/assets/js/docs.js' not in content:
        continue  # Redirect pages do not use the site's layout.
    head = content.split('</head>', 1)[0]
    assert head.count('https://www.googletagmanager.com/gtag/js?id=G-6NFBCLHFK6') == 1, page
    assert len(re.findall(r'''gtag\(['"]config['"],\s*['"]G-6NFBCLHFK6['"]\)''', head)) == 1, page
# The landing page lists only this library, not vendored ACL or verify sources.
home_links = re.findall(r'href="([^"]+)"', home)
assert not any('/atcoder/' in href or '/verify/' in href for href in home_links), "ACL/verify leaked onto homepage"
assert "Verification Files" not in home
assert 'class="site-hero"' in home
assert 'C++20・ヘッダオンリーの競技プログラミング用ライブラリ。' not in home
assert '標準機能はACLを優先してください。検証コードは' not in home
assert '<label for="library-search" class="visually-hidden">ライブラリを検索</label>' in home
assert 'class="site-license"' in home and '>CC0 1.0</a>' in home
assert 'CC0 1.0 Universal' in (ROOT / 'LICENSE').read_text()
assert 'class="site-nav"' in home
nav = re.search(r'<nav class="site-nav".*?</nav>', home, re.S)[0]
assert '/library-checker.html' in nav.split('<details', 1)[0], "LC coverage needs a direct navigation link"
assert '個人用テンプレート' not in home.split('<main', 1)[-1].split('<table id="library-results"', 1)[0], (
    "Personal template guidance should not interrupt library discovery")
assert '<main id="main-content">' in home
assert re.search(r'<details class="library-files" id="library-files">', home)
assert 'site-hero__eyebrow' not in home and 'class="library-summary"' not in home
assert 'data-catalog-search' in home and 'data-catalog-controls hidden' in home
assert 'data-catalog-empty hidden' in home and 'id="library-catalog"' in home
assert "calc(100% - 300px)" not in home, "Upstream inline sidebar CSS returned"
assert "This project is maintained by" not in home
assert "Hosted on GitHub Pages" not in home
assert "<footer>" not in home, "Unneeded theme footer returned"
assert 'class="skip-link"' in home, "Keyboard users need a direct route to content"
assert 'data-catalog-choice=""' in home, "All categories must remain selectable"
assert 'data-catalog-choice="algebra"' not in home, "Empty compatibility category leaked onto homepage"
assert not any(href.endswith('/blueberry/algebra/monoids.hpp.html') for href in home_links), (
    "Compatibility monoids belong on the monoid page")
catalog_script = (SITE / "assets/js/docs.js").read_text()
original = (ROOT / '.verify-helper/docs/static/assets/code/blueberrytemplate.h').read_bytes()
assert (SITE / 'assets/code/blueberrytemplate.h').read_bytes() == original, "Original template download changed bytes"
template_page = (SITE / 'blueberrytemplate.html').read_text()
original_code = re.search(r'<code\b[^>]*id="original-template-code"[^>]*>(.*?)</code>', template_page, re.S)
assert original_code and html.unescape(original_code[1]).encode('utf-8') == original, "Original template copy changed bytes"
assert '/Blueberry-library/blueberrytemplate.html' in home, "Missing original template entry"
assert '/Blueberry-library/blueberrytemplate.html' in (SITE / 'categories/utility.html').read_text()
assert 'event.preventDefault()' in catalog_script and 'choice.dataset.catalogChoice' in catalog_script, (
    "Category selection must filter the catalog without navigating away")
categories = ("data-structure", "graph", "math", "string", "utility")
for category in categories:
    assert f"/Blueberry-library/categories/{category}.html" in home
    assert f'data-catalog-choice="{category}"' in home, (category, "missing same-page category choice")
    content = (SITE / "categories" / f"{category}.html").read_text()
    assert "収録ライブラリ" in content and f"/blueberry/{category}/" in content
    assert 'data-catalog-search' in content, (category, "missing catalog search")
for page in ("guide.html", "comparisons.html", "benchmarks.html", "assets/js/docs.js", "assets/css/custom.css"):
    assert (SITE / page).exists(), page
assert not (SITE / 'migration.html').exists(), "Retired migration page must not survive an incremental build"
assert '/migration.html' not in nav
assert '/comparisons.html' in nav.split('<details', 1)[1], "Comparisons belong in the reference menu"
guide = (SITE / 'guide.html').read_text()
guide_sections = re.findall(r'<h2\b[^>]*>(.*?)</h2>', guide, re.S)
assert guide_sections[0] == '必要な機能を探す' and guide_sections[-1] == 'ローカルで使用したい場合'
for retired_copy in ('検索とキーボード操作', '開発・検証する', '旧ヘッダ', '参照を返すAPI', '経路圧縮'):
    assert retired_copy not in guide, retired_copy

# Discovery pages must remain usable without JavaScript and have live targets.
operations = (SITE / "operations.html").read_text()
coverage = (SITE / "library-checker.html").read_text()
for retired_copy in ('「verifyあり」は検証コードがある問題です。', '対応状況・データの出典', '開発時の更新方法'):
    assert retired_copy not in coverage, retired_copy
inventory = json.loads((ROOT / ".verify-helper/docs/static/_data/library_checker.json").read_text())
for content, prefix in ((operations, "operation"), (coverage, "coverage")):
    # Kramdown serializes valueless HTML attributes as attr="".
    for suffix in ("controls", "empty"):
        assert re.search(r'<[^>]+\bdata-' + prefix + '-' + suffix +
                         r'\b[^>]*\bhidden(?:\s|=|>)', content), (prefix, suffix)
    assert f'data-{prefix}-count' in content
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

monoids = (SITE / "monoids.html").read_text()
assert '/Blueberry-library/monoids.html' in home
assert 'data-catalog-search' in monoids and 'aria-live="polite"' in monoids
assert not re.search(r'<article\b[^>]*data-library[^>]*\bhidden\b', monoids)
assert 'JavaScript' in monoids and '共通の使い方' in monoids
for retired_copy in ('葉と空区間は別', '中間積まで型に収まることが前提です。', '演算が定数時間でない場合はその費用を掛けます。'):
    assert retired_copy not in monoids, retired_copy
recipes = json.loads((ROOT / '.verify-helper/docs/static/_data/monoids.yml').read_text())
assert '<body class="monoid-page">' in monoids and 'class="monoid-columns"' in monoids
assert re.findall(r'<article\b[^>]*data-library[^>]*\bid="([^"]+)"', monoids) == [recipe['id'] for recipe in recipes if recipe.get('code')], (
    "Render the curated monoid order before JavaScript filtering")
for recipe in recipes:
    if not recipe.get('code'):
        continue
    article = re.search(r'<article\b[^>]*\bid="' + re.escape(recipe['id']) + r'"[^>]*>(.*?)</article>', monoids, re.S)
    assert article, recipe['id']
    assert 'data-linked-details' in article[1] and '<summary>' in article[1], recipe['id']
    summary = re.search(r'<summary>(.*?)</summary>', article[1], re.S)[1]
    assert '→' not in summary and 'class="monoid-pair"' in summary, recipe['id']
    for field, class_name in (('update_label', 'monoid-update'), ('query_label', 'monoid-query')):
        assert recipe[field] and f'class="{class_name}"' in summary and html.escape(recipe[field]) in summary, recipe['id']
    assert 'class="monoid-kind"' in summary, recipe['id']
    if recipe.get('code'):
        code = re.search(r'<pre><code\b[^>]*>(.*?)</code></pre>', article[1], re.S)
        assert code and html.unescape(code[1]).rstrip('\n') == recipe['code'].rstrip('\n'), (recipe['id'], 'copy text differs')
        assert '<em>' not in article[1], (recipe['id'], 'formula interpreted as emphasis')
assert sum(bool(recipe.get('code')) for recipe in recipes) == 35
assert '使う例' not in monoids and '操作一覧' not in monoids
assert '応用問題:' not in monoids and 'id="beats"' not in monoids, 'Only monoid definitions belong in the cookbook'
assert 'href=""' not in monoids, "Recipes without example problems must not render empty links"
for href in re.findall(r'href="(/Blueberry-library/[^"#]*\.html)"', monoids):
    assert (SITE / href.removeprefix('/Blueberry-library/')).is_file(), ("monoids", href)

for obsolete in ("ConvexHulltrick.hpp", "DynamicFenwickTree2D.hpp", "implicit_treap.hpp",
                 "fraction.hpp", "Graph.hpp", "RollbackUnionFind.hpp", "fps.hpp"):
    assert not (ROOT / "blueberry" / obsolete).exists(), (obsolete, "obsolete source restored")
    assert not (SITE / "blueberry" / f"{obsolete}.html").exists(), (obsolete, "stale generated page")
    assert not any(f'/blueberry/{obsolete}.html' in href for href in home_links), obsolete

headers = re.findall(r"^  path: (.+)$", (ROOT / ".verify-helper/docs/static/_data/libraries.yml").read_text(), re.M)
from library_checker_coverage import read_catalog
ordinary_headers = set(read_catalog(ROOT, include_extra_fast=False))
extra_fast_headers = set(headers) - ordinary_headers
catalog_rows = re.findall(r'<tr data-library\b[^>]*>.*?</tr>', home, re.S)
catalog_names = [html.unescape(re.search(r'<a\b[^>]*>(.*?)</a>', row, re.S)[1]) for row in catalog_rows]
assert catalog_names == sorted(catalog_names, key=str.lower), "Catalog must use predictable name order"
assert all('class="catalog-category"' in row for row in catalog_rows), "Show each result's category"
assert len(catalog_rows) == len(ordinary_headers), "Search catalog must contain every ordinary header exactly once"
for header in ordinary_headers:
    assert sum(f'/{header}.html' in row for row in catalog_rows) == 1, (header, "missing or duplicate search result")
extra_fast = (SITE / 'extra-fast.html').read_text()
assert '>さらに高速なライブラリ</a>' in nav.split('<details', 1)[1]
assert '/extra-fast.html' not in nav.split('<details', 1)[0], "Specialized entry belongs in the reference dropdown"
extra_rows = re.findall(r'<tr data-library\b[^>]*>.*?</tr>', extra_fast, re.S)
assert len(extra_rows) == len(extra_fast_headers)
benchmarks = (SITE / 'benchmarks.html').read_text()
assert not re.search(r'href="[^"]*/verify/extra-fast/', benchmarks), "Specialized measurements leaked into ordinary discovery"
for header in extra_fast_headers:
    assert sum(f'/{header}.html' in row for row in extra_rows) == 1
    for public_page in [home, operations, coverage, (SITE / 'verification-gaps.html').read_text(),
                        *((SITE / 'categories' / f'{category}.html').read_text() for category in categories)]:
        assert f'/{header}.html' not in public_page, (header, "Specialized API leaked into ordinary discovery")
assert not re.search(r'<tr data-library\b[^>]*\bhidden\b', extra_fast), "Specialized catalog must work without JavaScript"
assert not re.search(r'<tr data-library\b[^>]*\bhidden\b', home), "Catalog must work without JavaScript"
compatibility_monoids = SITE / 'blueberry/algebra/monoids.hpp.html'
assert compatibility_monoids.is_file(), "Removing a catalog entry must preserve its supported source page"
assert (SITE / 'blueberry/all.hpp.html').is_file(), "The umbrella include page must remain available"
for header in [*headers, 'blueberry/algebra/monoids.hpp']:
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
    assert 'id="bundled-source"' in content, (header, "missing bundled source")
    assert 'class="library-brief"' in content and 'class="library-details"' in content, (
        header, "Library docs must separate quick reference from complete API details")
    positions = [content.index(token) for token in (
        'class="library-brief"', 'id="source-code"', 'data-source-viewer',
        'class="library-details"', 'id="document-references"')]
    assert positions == sorted(positions), (header, "Expected brief → implementation → details → references")
    assert '<!-- library-implementation -->' not in content, (header, "Implementation split marker leaked into rendered docs")
    before_source = content[:content.index('id="source-code"')]
    assert 'class="api-operation"' not in before_source, (header, "Full API documentation must follow the copyable implementation")
    assert not re.search(r'<details\b[^>]*(?:class="(?:api-operation|extra-operations)"|class="source-relations-details")[^>]*\bopen\b', content), (
        header, "Supplementary operations and relationships must start collapsed")
    assert content.index('id="document-references"') < content.index('class="source-relations"'), (
        header, "Source relationships belong in the final reference section")
    if 'class="verification-examples"' in content:
        assert content.index('class="library-details"') < content.index('class="verification-examples"'), (
            header, "Verify source examples must follow the documentation details")
    brief = re.search(r'<div class="library-brief">(.*?)</div>\s*<h2 id="source-code">', content, re.S)
    assert brief and '<table>' in brief[1], (header, "Missing compact operation reference")
    for table_header in re.findall(r'<thead>(.*?)</thead>', brief[1], re.S):
        assert len(re.findall(r'<th\b', table_header)) == 2, (header, "Quick reference needs only operation and complexity")
    relations = re.search(r'<div class="source-relations">(.*?)</div>', content, re.S)
    assert relations, (header, "missing source relationships")
    for link in re.findall(r'href="([^"]+)"', relations[1]):
        assert link.startswith('/Blueberry-library/') and link.endswith('.html'), (header, link)
        assert (SITE / link.removeprefix('/Blueberry-library/')).exists(), (header, link, "broken dependency/verify link")
    notes = re.findall(r'<details\b([^>]*\bclass="library-notes"[^>]*)>(.*?)</details>', content, re.S)
    assert notes, (header, "Missing supplemental notes")
    for attributes, body in notes:
        assert not re.search(r"\bopen(?:\s|=|$)", attributes), (header, "Notes must start collapsed")
        assert '<summary>' in body, (header, "Missing notes label")
        assert not re.search(r'^#{2,3} ', body, re.M), (header, "Notes headings were not rendered")
    assert '<ul>' in notes[0][1], (header, "Render common supplemental contracts as a list")
    premise_id = '使い方前提' if header == 'blueberry/algebra/monoids.hpp' else '概要前提'
    assert f'id="{premise_id}"' in notes[0][1], (header, "Preserve the premise deep link inside its notes")
    document = (ROOT / ('docs/' + header.removeprefix('blueberry/').removesuffix('.hpp') + '.md')).read_text()
    for anchor in re.findall(r'^#{2,3} .* \{#([^}]+)\}$', document, re.M):
        assert content.count(f'id="{anchor}"') == 1, (header, anchor, "Preserve each pinned notes anchor exactly once")
    api_operations = re.findall(r'<details\b[^>]*class="api-operation"[^>]*>(.*?)</details>', content, re.S)
    if '<h2 id="最小使用例">' in content:
        assert '<a href="#最小使用例">使用例</a>' in unquote(content), (header, "Missing direct route to the usage example")
    if header == 'blueberry/algebra/monoids.hpp':
        assert not api_operations and '操作一覧' not in content
        assert len(re.findall(r'<article\b[^>]*data-monoid-definition', content)) == 35
        for recipe in recipes:
            if not recipe.get('code'):
                continue
            anchor = recipe['api'].split('<')[0].lower() if recipe['api'] else recipe['id']
            section = re.search(r'<article\b[^>]*id="' + anchor + r'"[^>]*>(.*?)</article>', content, re.S)
            assert section, anchor
            code = re.search(r'<pre><code\b[^>]*>(.*?)</code></pre>', section[1], re.S)
            assert code and html.unescape(code[1]).rstrip('\n') == recipe['code'].rstrip('\n'), anchor
    else:
        assert api_operations, (header, "missing expandable API docs")
    for body in api_operations:
        assert "<summary>" in body and "注意点:" in body, header
        assert "<pre" in body and "<code" in body, (header, "Markdown inside details was not rendered")
        assert "```cpp" not in body, header

# Every direct library include in a verify has an inline, nonempty source example.
for source in (ROOT / "verify").rglob("*.test.cpp"):
    path = source.relative_to(ROOT).as_posix()
    verify_page = (SITE / f"{path}.html").read_text()
    assert 'data-source-viewer' in verify_page, path
    assert 'data-source-toggle' in verify_page and '>Bundle</button>' in verify_page, path
    assert '<details class="bundled-source"' not in verify_page, path
    assert 'id="unbundled"' in verify_page and 'id="bundled-source"' in verify_page, path
    if '/extra-fast/' in path:
        for ordinary_header in ordinary_headers:
            ordinary_page = (SITE / f'{ordinary_header}.html').read_text()
            assert f'/{path}.html' not in ordinary_page, (ordinary_header, path, "Specialized verify leaked through dependency graph")
    includes = re.findall(r'#include "(blueberry/[^"\n]+)"', source.read_text())
    for header in includes:
        if '/extra-fast/' in path and header not in extra_fast_headers:
            continue  # Shared dependencies do not advertise specialized experiments.
        content = (SITE / f"{header}.html").read_text()
        match = re.search(r'<details[^>]*data-verify="' + re.escape(path) + r'"[^>]*>(.*?)</details>', content, re.S)
        assert match, (header, path, "missing related verify")
        assert '<code' in match[1] and 'PROBLEM' in match[1], (header, path, "empty source")
        assert f'/Blueberry-library/{path}.html' in match[1], (header, "missing verify link")

# Comparison help must be reachable both from recommendations and each API page.
import yaml
comparisons = yaml.safe_load((ROOT / ".verify-helper/docs/static/_data/comparisons.yml").read_text())
comparison_page = (SITE / 'comparisons.html').read_text()
assert 'id="comparison-guide"' not in operations and 'class="library-comparison"' not in operations
for family in comparisons:
    identifier = f'comparison-{family["id"]}'
    assert comparison_page.count(f'id="{identifier}"') == 1, identifier
    assert f'href="/Blueberry-library/comparisons.html#{identifier}"' in operations, (identifier, "no recommendation links")
    for header in family["headers"]:
        assert f'id="{identifier}"' in (SITE / f"{header}.html").read_text(), header
assert "__BASEURL__" not in comparison_page
assert "U√N + Q" in comparison_page and "速度の分岐点ではありません" in comparison_page
for href in re.findall(r'href="(/Blueberry-library/[^"#]*\.html)"', comparison_page):
    assert (SITE / href.removeprefix('/Blueberry-library/')).is_file(), ("comparisons", href)
gap_page = (SITE / "verification-gaps.html").read_text()
evidence = json.loads((ROOT / ".verify-helper/docs/static/_data/library_verification.json").read_text())
assert {row["path"] for row in evidence["libraries"]} == ordinary_headers
for row in evidence["libraries"]:
    assert f'/{row["path"]}.html' in gap_page, row["path"]
assert "all.hpp" not in " ".join(driver["path"] for row in evidence["libraries"] for driver in row["direct"])
assert "主機能の一部に公式verifyなし" in gap_page

if "--without-metrics" in sys.argv[1:]:
    print(f"PASS: {len(categories)} categories, {len(headers)} API pages, footer and navigation (before verification).")
    sys.exit(0)

from verification_state import load_state, valid_successes

revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
verification_state = json.loads((SITE / "assets/verification-state.json").read_text())
assert verification_state.get("schema") == 1 and verification_state.get("revision") == revision
saved_successes = valid_successes(load_state(ROOT / ".verification/state.json"), ROOT)
assert verification_state.get("records") == saved_successes, "Published AC must match unchanged source/dependency fingerprints"
expected = {p.relative_to(ROOT).as_posix() for p in (ROOT / "verify").rglob("*.test.cpp")}
assert expected <= set(saved_successes), "Publication must retain valid AC evidence for every verifier, including unselected ones"
metrics = json.loads((SITE / "assets/verification-metrics.json").read_text())
currently_passed = {row["path"] for row in metrics["results"] if row["status"] == "passed"}
for path, record in saved_successes.items():
    content = (SITE / f"{path}.html").read_text()
    assert 'data-verification-status="TEST_ACCEPTED"' in content, (path, "Valid historical AC was lost")
    assert f'data-saved-verification="{path}"' in content, (path, "Missing saved verification provenance")
    assert record["verified_at"] in content and record["verified_revision"] in content, (path, "Missing actual verified time/revision")
    if path not in currently_passed:
        assert "以前のACを保持" in content and "再計測を意味しません" in content, (path, "Historical AC must not look like current measurement")

paths = {r["path"] for r in metrics["results"]}
selected = validate_publication_report(metrics, expected, revision)
if metrics.get("selection") is not None:
    validate_plan_against_repository(metrics["selection"], ROOT)
for row in metrics["results"]:
    content = (SITE / f"{row['path']}.html").read_text()
    assert "このverifyの実行時間" in content
    assert metrics["generated_at"] in content, "Missing measured data on verify page"
    if row["status"] == "not_selected":
        assert "今回未実行" in content, "Unselected verify must not look measured"
    if row["status"] == "passed":
        assert 'data-verification-status="TEST_ACCEPTED"' in content, (
            row["path"], "Successful measurement must render an accepted verification heading")
for header in headers:
    content = (SITE / f"{header}.html").read_text()
    assert not re.search(r'data-verification-status="LIBRARY_(?:ALL|SOME)_WA"', content), (
        header, "Published source-valid AC must not render a failed library heading")
print(f"PASS: {len(categories)} categories, {len(headers)} API pages, {len(selected)} measured verifies, "
      f"{len(paths) - len(selected)} intentionally unselected, footer and navigation.")
