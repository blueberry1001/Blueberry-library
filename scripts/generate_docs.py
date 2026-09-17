#!/usr/bin/env python3
"""Generate upstream docs, applying configured excludes before dependency analysis.

verification-helper 5.6.0 otherwise analyzes excluded Python tooling before it
filters pages. Those scripts are not C++ library entries and importlab may time out.
"""
import argparse
from pathlib import Path
from unittest.mock import patch

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

    with patch.object(configure, "_find_source_code_paths", included_sources):
        documentation.main(jobs=args.jobs)
    for page in prune_removed_sources(Path.cwd()):
        print(f"Removed obsolete generated source page: {page}")


if __name__ == "__main__":
    main()
