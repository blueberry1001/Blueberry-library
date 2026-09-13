#!/usr/bin/env python3
"""Fetch and profile the fastest AC submissions from Library Checker.

The script uses the public v3 REST API used by judge.yosupo.jp's "Fastest"
view.  It only performs GET requests.  Times returned by the API are seconds;
the report displays milliseconds, matching the web UI.  Source files are
saved only when --save-source is supplied.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import sys
from typing import Any, Dict, List, Optional
from urllib.error import HTTPError, URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen


DEFAULT_BASE_URL = os.environ.get(
    "LC_REST_BASE_URL", "https://v3.api.judge.yosupo.jp"
)
USER_AGENT = "Blueberry-library/fastest-submissions"


def get_json(base_url: str, path: str, params: Optional[Dict[str, Any]] = None) -> Any:
    """GET one JSON endpoint and raise a useful error on HTTP/network failure."""

    url = base_url.rstrip("/") + "/" + path.lstrip("/")
    if params:
        url += "?" + urlencode(params)
    request = Request(url, headers={"Accept": "application/json", "User-Agent": USER_AGENT})
    try:
        with urlopen(request, timeout=30) as response:
            return json.load(response)
    except HTTPError as error:
        body = error.read().decode("utf-8", errors="replace")[:500]
        raise RuntimeError(f"Library Checker API returned HTTP {error.code}: {body}") from error
    except URLError as error:
        raise RuntimeError(f"could not reach Library Checker API: {error.reason}") from error


def source_profile(source: str) -> Dict[str, Any]:
    """Return small, reproducible heuristics useful during implementation review."""

    lower = source.lower()
    return {
        "lines": source.count("\n") + (1 if source else 0),
        "bytes": len(source.encode("utf-8")),
        "includes": source.count("#include"),
        "uses_acl_convolution": "atcoder/convolution" in lower,
        "uses_ntt": bool(re.search(r"\b(?:ntt|butterfly|fft)\b", lower)),
        "uses_simd": bool(re.search(r"immintrin|avx|sse|neon|__m\d+", lower)),
        "has_target_pragma": "#pragma gcc target" in lower,
        "uses_fast_io": bool(re.search(r"fast.?io|fread|fwrite|read\(", lower)),
        "uses_vector": "vector<" in lower or "std::vector" in lower,
    }


def fetch_fastest(
    problem: str,
    *,
    limit: int,
    base_url: str,
    lang: Optional[str] = None,
    dedup_user: bool = False,
) -> List[Dict[str, Any]]:
    """Fetch the fastest overview rows and their source/case details."""

    query: Dict[str, Any] = {
        "problem": problem,
        "status": "AC",
        "order": "+time",
        "limit": limit,
    }
    if lang:
        query["lang"] = lang
    if dedup_user:
        query["dedupUser"] = "true"
    overview = get_json(base_url, "/submissions", query)
    rows = overview.get("submissions", [])
    result: List[Dict[str, Any]] = []
    for row in rows:
        submission_id = int(row["id"])
        detail = get_json(base_url, f"/submissions/{submission_id}")
        source = detail.get("source", "") or ""
        cases = detail.get("case_results", []) or []
        case_max = max((float(case.get("time", 0.0)) for case in cases), default=None)
        result.append(
            {
                "id": submission_id,
                "overview": row,
                "source": source,
                "case_max_seconds": case_max,
                "profile": source_profile(source),
                "url": f"https://judge.yosupo.jp/submission/{submission_id}",
            }
        )
    return result


def format_ms(seconds: Any) -> str:
    if seconds is None:
        return "-"
    return f"{float(seconds) * 1000:.1f} ms"


def make_report(problem: str, rows: List[Dict[str, Any]], base_url: str) -> str:
    lines = [
        f"# Library Checker fastest: `{problem}`",
        "",
        f"REST endpoint: `{base_url.rstrip('/')}/submissions`",
        "Times are the API's maximum testcase time, converted from seconds to milliseconds.",
        "The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.",
        "",
        "| rank | submission | user | language | max time | max case | memory |",
        "| ---: | ---: | --- | --- | ---: | ---: | ---: |",
    ]
    for rank, item in enumerate(rows, 1):
        row = item["overview"]
        user = row.get("user_name") or "(anonymous)"
        memory = row.get("memory", -1)
        lines.append(
            f"| {rank} | [{item['id']}](https://judge.yosupo.jp/submission/{item['id']}) "
            f"| {user} | {row.get('lang', '-')} | {format_ms(row.get('time'))} "
            f"| {format_ms(item['case_max_seconds'])} | {memory} B |"
        )
    lines += ["", "## Source heuristics", ""]
    lines += [
        "These flags are intentionally shallow indicators for deciding what to benchmark locally; "
        "they are not proof that one implementation is faster.",
        "",
        "| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |",
        "| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |",
    ]
    for item in rows:
        profile = item["profile"]
        yes = lambda key: "yes" if profile[key] else "no"
        lines.append(
            f"| {item['id']} | {profile['lines']} | {profile['bytes']} | {yes('uses_ntt')} "
            f"| {yes('uses_simd')} | {yes('has_target_pragma')} | {yes('uses_acl_convolution')} "
            f"| {yes('uses_fast_io')} |"
        )
    return "\n".join(lines) + "\n"


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("problem", help="Library Checker problem id, e.g. exp_of_formal_power_series")
    parser.add_argument("--limit", type=int, default=5, help="number of fastest rows to fetch (1-1000)")
    parser.add_argument("--lang", help="optional language id filter, such as C++23")
    parser.add_argument("--dedup-user", action="store_true", help="keep only the fastest row per user")
    parser.add_argument("--base-url", default=DEFAULT_BASE_URL, help="REST API base URL")
    parser.add_argument(
        "--out-dir",
        type=Path,
        help="write fastest.json/report.md (and optional sources) to this directory",
    )
    parser.add_argument("--save-source", action="store_true", help="save fetched source as submission-ID.cpp")
    return parser.parse_args(argv)


def main(argv: Optional[List[str]] = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    if not 1 <= args.limit <= 1000:
        raise SystemExit("--limit must be between 1 and 1000")
    rows = fetch_fastest(
        args.problem,
        limit=args.limit,
        base_url=args.base_url,
        lang=args.lang,
        dedup_user=args.dedup_user,
    )
    report = make_report(args.problem, rows, args.base_url)
    if args.out_dir:
        args.out_dir.mkdir(parents=True, exist_ok=True)
        serializable = [{key: value for key, value in item.items() if key != "source"} for item in rows]
        (args.out_dir / "fastest.json").write_text(
            json.dumps(serializable, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
        )
        (args.out_dir / "report.md").write_text(report, encoding="utf-8")
        if args.save_source:
            for item in rows:
                (args.out_dir / f"submission-{item['id']}.cpp").write_text(
                    item["source"], encoding="utf-8"
                )
    else:
        sys.stdout.write(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
