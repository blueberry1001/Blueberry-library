# Isolated documentation preflight

Run on 2026-11-15 in a separate WSL2 Linux clone at
`/tmp/blueberry-docs-isolated-20261115`, using revision
`123365f2738e3d883c566def538e2833f411c317` plus the then-current working-tree
changes. The clone copied the source files and ACL dependency, but did not
copy the workspace's `.verification` directory or generated documentation.
The workspace's existing data was not changed by the build. The records in
this directory were added after the hash comparison and build.

The [hash log](blueberry-docs-isolated-20261115-hashes.log) confirms that the
copy and workspace had identical SHA-256 content for all 127 headers, 2,791
documentation files, 42 scripts, 169 verify files, 45 files under
`.verify-helper/docs`, `.verify-helper/config.toml`, and `Makefile` at the
time of comparison. Each reported digest summarizes path and file SHA-256s;
it is not a Git tree hash.

| Command | Result | Log |
| --- | --- | --- |
| `make docs` | Success; no current measurement was published | [make docs](blueberry-docs-isolated-20261115-make-docs.log) |
| `JEKYLL_ENV=production bundle exec jekyll build` | Success | [Jekyll](blueberry-docs-isolated-20261115-jekyll.log) |
| `python3 scripts/check_site.py --without-metrics` | Success; 5 categories, 123 API pages | [preflight](blueberry-docs-isolated-20261115-check-site-preflight.log) |
| `python3 scripts/check_site.py` | Failed: source-valid AC evidence for all verifiers was absent | [full check](blueberry-docs-isolated-20261115-check-site-full.log) |

The complete metrics-and-verification publication check is **not verified** by
this run. In particular, the full check failure reflects the intentionally
omitted evidence in this isolated clone; it must not be counted as a pass or
as proof that the eventual CI publication will fail. A full run requires the
matching complete, source-valid verification evidence.

Environment: Ruby 3.2.3, Bundler 2.4.20, GitHub Pages 233, Jekyll 3.10.0,
online-judge-verify-helper 5.6.0, GCC on WSL2. The gem bundle was installed
only under `/tmp/blueberry-docs-isolated-20261115-gems`; its
[resolved lockfile](Gemfile.lock) and [installation log](blueberry-docs-isolated-20261115-bundle.log)
are retained. The local gem environment is not claimed to be identical to the
GitHub Actions Pages build image.
