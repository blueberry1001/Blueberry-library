#!/usr/bin/env python3
"""Collect Library Checker fastest/user submissions and profile source features.

Only public GET endpoints are used. Source text is never written unless
--save-source-dir is explicitly supplied; JSON reports contain hashes and
heuristic feature flags instead.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import time
from typing import Any, Dict, List, Optional, Sequence
from urllib.error import HTTPError, URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen


DEFAULT_BASE_URL = os.environ.get("LC_REST_BASE_URL", "https://v3.api.judge.yosupo.jp")
USER_AGENT = "Blueberry-library/benchmark-research"


class LibraryCheckerClient:
    def __init__(
        self, base_url: str = DEFAULT_BASE_URL, *, timeout: float = 20, retries: int = 2,
        cache_dir: Optional[Path] = None, max_age_hours: float = 24, refresh: bool = False,
    ) -> None:
        self.base_url = base_url.rstrip("/")
        self.timeout = timeout
        self.retries = retries
        self.cache_dir = cache_dir
        self.max_age_seconds = max_age_hours * 3600
        self.refresh = refresh

    def _url(self, path: str, params: Optional[Dict[str, Any]]) -> str:
        url = self.base_url + "/" + path.lstrip("/")
        if params:
            url += "?" + urlencode(sorted(params.items()))
        return url

    def _cache_path(self, url: str) -> Optional[Path]:
        if self.cache_dir is None:
            return None
        return self.cache_dir / (hashlib.sha256(url.encode()).hexdigest() + ".json")

    def get_json(self, path: str, params: Optional[Dict[str, Any]] = None) -> Any:
        url = self._url(path, params)
        cache_path = self._cache_path(url)
        if cache_path and cache_path.is_file() and not self.refresh:
            age = time.time() - cache_path.stat().st_mtime
            if age <= self.max_age_seconds:
                return json.loads(cache_path.read_text(encoding="utf-8"))["data"]

        error: Optional[Exception] = None
        for attempt in range(self.retries + 1):
            request = Request(url, headers={"Accept": "application/json", "User-Agent": USER_AGENT})
            try:
                with urlopen(request, timeout=self.timeout) as response:
                    payload = json.load(response)
                if cache_path:
                    cache_path.parent.mkdir(parents=True, exist_ok=True)
                    cached = {"url": url, "fetched_at": datetime.now(timezone.utc).isoformat(), "data": payload}
                    cache_path.write_text(json.dumps(cached, ensure_ascii=False) + "\n", encoding="utf-8")
                return payload
            except (HTTPError, URLError, TimeoutError, OSError, ValueError) as caught:
                error = caught
                if attempt < self.retries:
                    time.sleep(min(2 ** attempt, 4))
        raise RuntimeError(f"Library Checker API request failed after {self.retries + 1} attempts: {url}: {error}")


def source_profile(source: str) -> Dict[str, Any]:
    lower = source.lower()
    identifiers = set(re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", source))
    identifier_lower = {identifier.lower() for identifier in identifiers}
    return {
        "sha256": hashlib.sha256(source.encode()).hexdigest(),
        "bytes": len(source.encode()),
        "lines": source.count("\n") + (1 if source else 0),
        "uses_blueberry": "blueberry/" in lower,
        "uses_iostream": "<iostream>" in lower,
        "uses_buffered_c_io": bool(re.search(r"\b(?:fread|fwrite|getc_unlocked|putc_unlocked)\b", lower)),
        "uses_posix_io": bool(re.search(r"\b(?:read|write)\s*\(", lower)),
        "uses_simd": bool(re.search(r"immintrin|\bavx\d*\b|\bsse\d*\b|__m\d+", lower)),
        "has_target_pragma": "#pragma gcc target" in lower,
        "uses_acl": "atcoder/" in lower,
        "uses_nested_vector": bool(
            re.search(r"(?:std::)?vector\s*<\s*(?:std::)?vector\s*<", lower)
        ),
        "mentions_sparse_table": any("sparsetable" in item or "sparse_table" in item for item in identifier_lower),
        "mentions_blocked_rmq": any(
            "blocked" in item and ("sparse" in item or "rmq" in item) for item in identifier_lower
        ),
        "uses_bit_scan": bool(re.search(r"__builtin_(?:clz|ctz)|\b(?:bit_width|countl_zero|countr_zero)\b", lower)),
    }


def public_detail(detail: Dict[str, Any]) -> Dict[str, Any]:
    source = detail.get("source") or ""
    case_results = detail.get("case_results") or []
    return {
        "overview": detail.get("overview", {}),
        "case_count": len(case_results),
        "max_case_seconds": max((float(case.get("time", 0)) for case in case_results), default=None),
        "max_case_memory_bytes": max((int(case.get("memory", 0)) for case in case_results), default=None),
        "profile": source_profile(source),
    }


def fetch_problem(
    client: LibraryCheckerClient, problem: str, *, language: str, limit: int, user: Optional[str],
    analyze_top: int, source_dir: Optional[Path],
) -> Dict[str, Any]:
    fastest_payload = client.get_json("/submissions", {
        "problem": problem, "status": "AC", "lang": language, "order": "+time",
        "dedupUser": "true", "limit": limit,
    })
    fastest = fastest_payload.get("submissions", [])
    own: List[Dict[str, Any]] = []
    if user:
        own_payload = client.get_json("/submissions", {
            "problem": problem, "status": "AC", "lang": language, "order": "+time",
            "user": user, "limit": 1,
        })
        own = own_payload.get("submissions", [])

    details: Dict[str, Any] = {}
    detail_ids = [int(row["id"]) for row in fastest[:analyze_top]]
    if own:
        detail_ids.append(int(own[0]["id"]))
    for submission_id in dict.fromkeys(detail_ids):
        detail = client.get_json(f"/submissions/{submission_id}")
        details[str(submission_id)] = public_detail(detail)
        if source_dir is not None:
            source_dir.mkdir(parents=True, exist_ok=True)
            (source_dir / f"{problem}-{submission_id}.cpp").write_text(
                detail.get("source") or "", encoding="utf-8"
            )

    fastest_time = float(fastest[0]["time"]) if fastest else None
    own_time = float(own[0]["time"]) if own else None
    fastest_memory = int(fastest[0]["memory"]) if fastest else None
    own_memory = int(own[0]["memory"]) if own else None
    return {
        "problem": problem,
        "language": language,
        "fastest_count": fastest_payload.get("count"),
        "fastest": fastest,
        "user_best": own[0] if own else None,
        "time_ratio_user_to_fastest":
            own_time / fastest_time if own_time is not None and fastest_time else None,
        "memory_ratio_user_to_fastest":
            own_memory / fastest_memory if own_memory is not None and fastest_memory else None,
        "details": details,
    }


def yes_no(value: Any) -> str:
    return "yes" if value else "no"


def markdown_report(report: Dict[str, Any]) -> str:
    lines = [
        "# Library Checker submission research",
        "",
        f"Generated: {report['generated_at']}",
        "",
        "Leaderboard time is the maximum testcase time reported by Library Checker. "
        "Ratios are descriptive only: compiler generation, CPU, testcase version, and submitted code may differ.",
        "",
        "| problem | fastest | user best | time ratio | memory ratio |",
        "| --- | ---: | ---: | ---: | ---: |",
    ]
    for item in report["problems"]:
        fastest = item["fastest"][0] if item["fastest"] else None
        own = item["user_best"]
        fastest_text = f"[{fastest['time'] * 1000:.1f} ms](https://judge.yosupo.jp/submission/{fastest['id']})" if fastest else "—"
        own_text = f"[{own['time'] * 1000:.1f} ms](https://judge.yosupo.jp/submission/{own['id']})" if own else "—"
        time_ratio = item["time_ratio_user_to_fastest"]
        memory_ratio = item["memory_ratio_user_to_fastest"]
        lines.append(
            f"| `{item['problem']}` | {fastest_text} | {own_text} "
            f"| {f'{time_ratio:.3f}x' if time_ratio is not None else '—'} "
            f"| {f'{memory_ratio:.3f}x' if memory_ratio is not None else '—'} |"
        )
    lines += [
        "",
        "## Source feature heuristics",
        "",
        "These flags select candidates for local benchmarking; they do not establish causality.",
        "",
        "| problem | submission | role | lines | buffered I/O | blocked RMQ | nested vector | SIMD | target pragma |",
        "| --- | ---: | --- | ---: | :---: | :---: | :---: | :---: | :---: |",
    ]
    for item in report["problems"]:
        fastest_id = item["fastest"][0]["id"] if item["fastest"] else None
        own_id = item["user_best"]["id"] if item["user_best"] else None
        for submission_id, detail in item["details"].items():
            profile = detail["profile"]
            role = "fastest" if int(submission_id) == fastest_id else "user best" if int(submission_id) == own_id else "top"
            lines.append(
                f"| `{item['problem']}` | [{submission_id}](https://judge.yosupo.jp/submission/{submission_id}) "
                f"| {role} | {profile['lines']} | {yes_no(profile['uses_buffered_c_io'])} "
                f"| {yes_no(profile['mentions_blocked_rmq'])} | {yes_no(profile['uses_nested_vector'])} "
                f"| {yes_no(profile['uses_simd'])} | {yes_no(profile['has_target_pragma'])} |"
            )
    return "\n".join(lines) + "\n"


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--problem", action="append", required=True, help="problem id; repeat for several")
    parser.add_argument("--user", help="also collect this user's fastest AC")
    parser.add_argument("--lang", default="cpp", help="exact Library Checker language id")
    parser.add_argument("--limit", type=int, default=5, help="fastest rows per problem (1-1000)")
    parser.add_argument("--analyze-top", type=int, default=1, help="fetch source details for this many rows")
    parser.add_argument("--base-url", default=DEFAULT_BASE_URL)
    parser.add_argument("--timeout", type=float, default=20)
    parser.add_argument("--retries", type=int, default=2)
    parser.add_argument("--cache-dir", type=Path, default=Path(".benchmark/lc-cache"))
    parser.add_argument("--cache-max-age-hours", type=float, default=24)
    parser.add_argument("--refresh", action="store_true")
    parser.add_argument("--output", type=Path, help="directory for submissions.json and report.md")
    parser.add_argument(
        "--save-source-dir", type=Path,
        help="opt-in: save third-party sources locally; review their licenses before reuse or commit",
    )
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    if not 1 <= args.limit <= 1000:
        raise SystemExit("--limit must be in [1, 1000]")
    if not 0 <= args.analyze_top <= args.limit:
        raise SystemExit("--analyze-top must be in [0, limit]")
    if args.timeout <= 0 or args.retries < 0 or args.cache_max_age_hours < 0:
        raise SystemExit("timeout must be positive; retries and cache age must be non-negative")

    client = LibraryCheckerClient(
        args.base_url, timeout=args.timeout, retries=args.retries, cache_dir=args.cache_dir,
        max_age_hours=args.cache_max_age_hours, refresh=args.refresh,
    )
    report = {
        "schema": 1,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "api_base_url": args.base_url.rstrip("/"),
        "user": args.user,
        "problems": [
            fetch_problem(
                client, problem, language=args.lang, limit=args.limit, user=args.user,
                analyze_top=args.analyze_top, source_dir=args.save_source_dir,
            )
            for problem in args.problem
        ],
    }
    markdown = markdown_report(report)
    if args.output:
        args.output.mkdir(parents=True, exist_ok=True)
        (args.output / "submissions.json").write_text(
            json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
        )
        (args.output / "report.md").write_text(markdown, encoding="utf-8")
    else:
        sys.stdout.write(markdown)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
