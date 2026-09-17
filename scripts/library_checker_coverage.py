#!/usr/bin/env python3
"""Reproducible inventory of published Library Checker problems and local coverage.

Refresh explicitly with --refresh --upstream /path/to/library-checker-problems.
Ordinary generation and --check are offline. A verify driver is not proof of AC.
"""
from __future__ import annotations

import argparse
from collections import Counter
from datetime import datetime, timezone
import json
from pathlib import Path
import re
import subprocess
try:
    import tomllib
except ModuleNotFoundError:  # Python 3.9/3.10: verification-helper already requires toml.
    import toml as tomllib
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
API = "https://v3.api.judge.yosupo.jp/problems"
UPSTREAM = "https://github.com/yosupo06/library-checker-problems"
LABELS = {"verify": "実装＋verifyあり", "implemented": "実装あり・専用verifyなし",
          "acl": "ACL推奨", "driver": "補助verifyのみ", "missing": "未対応"}
ID = re.compile(r"[a-zA-Z0-9_]+\Z")
PROBLEM = re.compile(r'^\s*#\s*define\s+PROBLEM\s+"https://judge\.yosupo\.jp/problem/([a-zA-Z0-9_]+)"', re.M)
INCLUDE = re.compile(r'^\s*#\s*include\s+"(blueberry/[^"\n]+)"', re.M)


def make_inventory(problems, categories, revision, fetched_at):
    category_map = {}
    category_order = []
    for category in categories["categories"]:
        name = category["name"]
        category_order.append(name)
        for identifier in category["problems"]:
            if identifier in category_map:
                raise ValueError(f"Duplicate category membership: {identifier}")
            category_map[identifier] = name
    rows = []
    seen = set()
    for problem in problems:
        identifier, title = problem["name"], problem["title"]
        if not ID.fullmatch(identifier) or identifier in seen or not isinstance(title, str) or not title:
            raise ValueError(f"Invalid/duplicate published problem: {identifier!r}")
        seen.add(identifier)
        rows.append({"id": identifier, "title": title,
                     "category": category_map.get(identifier, "Uncategorized")})
    if not rows:
        raise ValueError("Refusing an empty published problem inventory")
    if any(row["category"] == "Uncategorized" for row in rows):
        category_order.append("Uncategorized")
    order = {name: i for i, name in enumerate(category_order)}
    rows.sort(key=lambda row: (order[row["category"]], row["id"]))
    return {"schema": 1, "fetched_at": fetched_at, "api_url": API,
            "upstream_url": UPSTREAM, "upstream_revision": revision,
            "categories": category_order, "problems": rows}


def cpp_without_comments(text):
    # Keep quoted URLs intact, and preserve newlines for directive anchoring.
    token = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/'
    return re.sub(token, lambda m: "\n" * m[0].count("\n") if m[0].startswith(("//", "/*")) else m[0], text)


def dependencies(root, text, catalog, seen=None):
    seen = set() if seen is None else seen
    result = set()
    for path in INCLUDE.findall(cpp_without_comments(text)):
        target = root / path
        if ".." in Path(path).parts or not target.is_file():
            raise ValueError(f"Missing or unsafe local include: {path}")
        # all.hpp's A+B smoke test does not implement every problem/library.
        if path == "blueberry/all.hpp" or path in seen:
            continue
        seen.add(path)
        if path in catalog:
            result.add(path)
        result.update(dependencies(root, target.read_text(encoding="utf-8"), catalog, seen))
    return result


def build_rows(root, inventory, mappings, catalog):
    problems = {item["id"]: item for item in inventory["problems"]}
    if len(problems) != len(inventory["problems"]):
        raise ValueError("Duplicate inventory problem IDs")
    unknown = mappings.keys() - problems.keys()
    if unknown:
        raise ValueError(f"Mappings refer to unknown published problems: {sorted(unknown)}")
    for identifier, mapping in mappings.items():
        if mapping.get("status") not in ("implemented", "acl") or not mapping.get("note"):
            raise ValueError(f"Invalid reviewed mapping: {identifier}")
        if mapping["status"] == "implemented" and not mapping.get("headers"):
            raise ValueError(f"Implementation mapping has no header: {identifier}")
        for path in mapping.get("headers", []):
            if path not in catalog or not (root / path).is_file():
                raise ValueError(f"Unknown mapped library: {path}")
        if mapping["status"] == "acl" and not mapping.get("url", "").startswith("https://atcoder.github.io/ac-library/"):
            raise ValueError(f"ACL mapping must link to official ACL documentation: {identifier}")
    drivers = {identifier: [] for identifier in problems}
    ignored = {identifier: [] for identifier in problems}
    used = {identifier: set() for identifier in problems}
    for source in sorted((root / "verify").rglob("*.test.cpp")):
        text = cpp_without_comments(source.read_text(encoding="utf-8"))
        ids = PROBLEM.findall(text)
        if not ids:
            continue
        if len(ids) != 1 or ids[0] not in problems:
            raise ValueError(f"Unknown or ambiguous PROBLEM in {source}: {ids}")
        identifier = ids[0]
        path = source.relative_to(root).as_posix()
        if re.search(r'^\s*#\s*define\s+IGNORE\b', text, re.M):
            ignored[identifier].append(path)
            continue
        drivers[identifier].append(path)
        used[identifier].update(dependencies(root, text, catalog))
    rows = []
    for problem in inventory["problems"]:
        identifier = problem["id"]
        mapping = mappings.get(identifier, {})
        headers = used[identifier] | set(mapping.get("headers", []))
        status = mapping.get("status", "missing")
        if drivers[identifier]:
            status = "verify" if headers else "driver"
        rows.append({**problem, "url": f"https://judge.yosupo.jp/problem/{identifier}",
                     "status": status, "label": LABELS[status],
                     "headers": [{"path": path, "name": catalog[path]} for path in sorted(headers)],
                     "verifies": drivers[identifier], "ignored_verifies": ignored[identifier],
                     "note": mapping.get("note", ""), "external_url": mapping.get("url", "")})
    return rows


def md_escape(text):
    return text.replace("\\", "\\\\").replace("[", "\\[").replace("]", "\\]").replace("\n", " ")


def render_markdown(inventory, rows):
    counts = Counter(row["status"] for row in rows)
    lines = ["# Library Checker 全問題チェックリスト", "",
             "このファイルは `python3 scripts/library_checker_coverage.py` で生成します。直接編集しません。", "",
             f"公式公開問題一覧: {inventory['fetched_at']} / **{len(rows)} 問題**。",
             f"[公式一覧API]({inventory['api_url']}) / [カテゴリの固定リビジョン]({UPSTREAM}/blob/{inventory['upstream_revision']}/categories.toml)", "",
             "チェック済みは対応するローカル実装があることを示します。verifyありはドライバの存在であり、ACや現在の実装での成功を意味しません。",
             "実行結果は各verifyページと[測定一覧](https://blueberry1001.github.io/Blueberry-library/benchmarks.html)で確認してください。",
             "ACL推奨は外部ライブラリです。Blueberryの実装数・チェック済み数には含めません。", "",
             " / ".join(f"{label}: {counts[status]}" for status, label in LABELS.items()), ""]
    for category in inventory["categories"]:
        selected = [row for row in rows if row["category"] == category]
        if not selected:
            continue
        lines += [f"## {category}", ""]
        for row in selected:
            checked = "x" if row["status"] in ("verify", "implemented") else " "
            links = [f"[{md_escape(header['name'])}](../{header['path']})" for header in row["headers"]]
            links += [f"[verify](../{path})" for path in row["verifies"]]
            if row["external_url"]:
                links.append(f"[ACL]({row['external_url']})")
            suffix = " — " + ", ".join(links) if links else ""
            note = " — " + row["note"] if row["note"] else ""
            lines.append(f"- [{checked}] [{md_escape(row['title'])}]({row['url']}) (`{row['id']}`): {row['label']}{suffix}{note}")
        lines.append("")
    return "\n".join(lines)


def read_catalog(root):
    text = (root / ".verify-helper/docs/static/_data/libraries.yml").read_text(encoding="utf-8")
    return {match[1]: match[0] for match in re.findall(r'^- name: (.+)\n(?:(?!- name:)[\s\S])*?^  path: (.+)$', text, re.M)}


def write_or_check(path, text, check):
    if check:
        if not path.is_file() or path.read_text(encoding="utf-8") != text:
            raise ValueError(f"Stale coverage output: {path}. Run python3 scripts/library_checker_coverage.py")
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--refresh", action="store_true")
    parser.add_argument("--upstream", type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    inventory_path = ROOT / "data/library-checker-problems.json"
    if args.refresh:
        if args.check or args.upstream is None:
            parser.error("--refresh requires --upstream and cannot combine with --check")
        revision = subprocess.check_output(["git", "-C", str(args.upstream), "rev-parse", "HEAD"], text=True).strip()
        # Read the committed blob, not an uncommitted category edit.
        categories = tomllib.loads(subprocess.check_output(["git", "-C", str(args.upstream), "show", f"{revision}:categories.toml"], text=True))
        request = Request(API, headers={"User-Agent": "Blueberry-library/coverage", "Accept": "application/json"})
        with urlopen(request, timeout=30) as response:
            problems = json.load(response)["problems"]
        inventory = make_inventory(problems, categories, revision, datetime.now(timezone.utc).isoformat())
        write_or_check(inventory_path, json.dumps(inventory, ensure_ascii=False, indent=2) + "\n", False)
    inventory = json.loads(inventory_path.read_text(encoding="utf-8"))
    mappings = json.loads((ROOT / "data/library-checker-mappings.json").read_text(encoding="utf-8"))
    rows = build_rows(ROOT, inventory, mappings, read_catalog(ROOT))
    payload = {key: value for key, value in inventory.items() if key != "problems"}
    payload.update({"total": len(rows), "counts": dict(Counter(row["status"] for row in rows)),
                    "statuses": [{"id": key, "label": value} for key, value in LABELS.items()], "problems": rows})
    write_or_check(ROOT / ".verify-helper/docs/static/_data/library_checker.json", json.dumps(payload, ensure_ascii=False, indent=2) + "\n", args.check)
    write_or_check(ROOT / "docs/library-checker-checklist.md", render_markdown(inventory, rows), args.check)
    print(f"{'Checked' if args.check else 'Generated'} {len(rows)} published problems: {dict(Counter(row['status'] for row in rows))}")


if __name__ == "__main__":
    main()
