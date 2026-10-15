#!/usr/bin/env python3
"""Generate upstream docs, applying configured excludes before dependency analysis.

verification-helper 5.6.0 otherwise analyzes excluded Python tooling before it
filters pages. Those scripts are not C++ library entries and importlab may time out.
"""
import argparse
from contextlib import contextmanager
import json
from pathlib import Path
import subprocess
from unittest.mock import patch


def current_failed_verifiers(basedir, revision):
    """Only an existing verifier's explicit failure at this HEAD proves failure."""
    basedir = Path(basedir).resolve()
    try:
        report = json.loads((basedir / ".verification/current.json").read_text(encoding="utf-8"))
        if (not isinstance(report, dict) or report.get("schema") != 1 or
                report.get("revision") != revision or not isinstance(report.get("results"), list) or
                any(not isinstance(row, dict) for row in report["results"])):
            return set()
        failed = set()
        for row in report["results"]:
            name = row.get("path")
            if row.get("status") != "failed" or not isinstance(name, str):
                continue
            path = Path(name)
            if (path.is_absolute() or path.as_posix() != name or ".." in path.parts or
                    not name.startswith("verify/") or not name.endswith(".test.cpp")):
                continue
            source = (basedir / path).resolve()
            if source.is_relative_to(basedir) and source.is_file():
                failed.add(source)
        return failed
    except (OSError, ValueError, RuntimeError):
        return set()


@contextmanager
def documentation_marker_statuses(marker_class, failed, basedir):
    """Keep cached AC; turn missing/stale records into waiting, not fabricated WA."""
    original_verified = marker_class.is_verified
    basedir = Path(basedir).resolve()

    def is_failed(marker, path):
        return (basedir / path).resolve() in failed

    def is_verified(marker, path):
        return not is_failed(marker, path) and original_verified(marker, path)

    with patch.object(marker_class, "is_verified", is_verified), \
            patch.object(marker_class, "is_failed", is_failed):
        yield


def prune_removed_sources(basedir):
    """The upstream renderer overwrites pages but never removes obsolete ones."""
    basedir = Path(basedir).resolve()
    output = basedir / ".verify-helper/markdown"
    if not output.exists():
        return []
    if not output.resolve().is_relative_to(basedir):
        raise ValueError("documentation output must stay inside the repository")
    removed = []
    for directory, pattern in (("blueberry", "*.hpp.md"), ("verify", "*.test.cpp.md")):
        for page in (output / directory).rglob(pattern):
            if not page.resolve().is_relative_to(output.resolve()):
                raise ValueError("generated source page points outside documentation output")
            source = basedir / page.relative_to(output).with_suffix("")
            if not source.is_file():
                page.unlink()
                removed.append(page.relative_to(output).as_posix())
    return removed


def main():
    import onlinejudge_verify.config as config
    import onlinejudge_verify.documentation.configure as configure
    import onlinejudge_verify.documentation.main as documentation
    import onlinejudge_verify.marker as marker_module

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-j", "--jobs", type=int, default=2)
    args = parser.parse_args()
    config.set_config_path(Path(".verify-helper/config.toml"))
    render_config = documentation.load_render_config(basedir=Path.cwd())
    excluded = [Path(path) for path in render_config.config_yml.get("exclude", [])]
    find_sources = configure._find_source_code_paths

    def included_sources(*, basedir):
        return [path for path in find_sources(basedir=basedir)
                if not configure.is_excluded(path.relative_to(basedir), excluded_paths=excluded)]

    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
    failed = current_failed_verifiers(Path.cwd(), revision)
    with patch.object(configure, "_find_source_code_paths", included_sources), \
            documentation_marker_statuses(marker_module.VerificationMarker, failed, Path.cwd()):
        documentation.main(jobs=args.jobs)
    for page in prune_removed_sources(Path.cwd()):
        print(f"Removed obsolete generated source page: {page}")


if __name__ == "__main__":
    main()
