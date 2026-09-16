# Contest readiness implementation plan

> **For agentic workers:** Use superpowers:subagent-driven-development for independent tasks; coordinate shared catalog changes in the parent task.

**Goal:** Make supported libraries easier to find, copy, and use correctly in contests, repair demonstrated failures, and fill useful non-ACL gaps.

**Architecture:** Keep C++20 header-only APIs and the existing Pages/verification-helper site. Add independent graph and offline geometry-query building blocks; preserve existing public entrypoints. Separate unsupported historical snippets from the supported catalog.

**Tech Stack:** C++20, ACL, Python unittest, Library Checker, Jekyll, plain JavaScript.

**Spec:** User request and IMPLEMENTATION_POLICY.md; existing APIs and templates/library.md.example define compatibility and documentation structure.

## Global constraints

- C++20, header-only, blueberry namespace, zero-based indices and half-open ranges.
- ACL-first for standard algorithms; no removal of legacy files or public APIs.
- Every new/changed public API has operation-level documentation, an executable example, catalog registration and appropriate official/random verification.
- Preserve GitHub Pages/verification-helper publishing and the current responsive layout.
- Preserve the pre-existing AGENTS.md edit and commit only task-owned files.
- User subsequently authorized commits, pushes and PR merges without further confirmation. Require independent review and successful verification before merging codex/contest-readiness.

## Tasks and ownership

- [x] FPS: reproduce sparse division compilation, aliased scalar multiplication, truncated square root and constant-modulus exponentiation failures; fix them and compare small random inputs with independent polynomial arithmetic. Owner: audit_libraries; header/docs/new tests/official polynomial division.
- [x] Graph: add iterative LowLink and rerooting with official two-edge-connected-components and tree-path-composite-sum drivers, random brute-force comparisons and deep paths. Owner: coverage_gaps; new graph files only.
- [x] Site: searchable catalog, current category summaries, copyable examples and opening nested API anchors. Owner: site_usability; static frontend and focused behavior/site tests, excluding catalog YAML.
- [x] Shortest paths: retain generic Cost compatibility, saturate integral relaxations before overflow, validate boundary/custom-infinity/path restoration behavior against Floyd-Warshall. Owner: parent.
- [x] Offline 2D Fenwick: register update coordinates, point add and rectangle sum in O(log^2 N); official point-add rectangle-sum and independent brute-force random tests. Owner: parent.
- [x] Integration: register headers, update include umbrella and practical task-to-library guide, document legacy limitations and current verification evidence. Owner: parent.
- [x] Verification: make test, make check, GCC/Clang C++20/23 compile/random, official make verify, make docs, rendered site checks and browser interaction checks when available.
- [x] Review: independent cross-review of code and docs, resolve important findings, inspect diff and commit coherent task units.

## Progress

- Baseline: origin/main fetched; current branch includes prior non-ACL additions at b4695f9 and is one commit ahead of main. No open PRs at start. Existing AGENTS.md edit preserved.
- 2026-09-16: baseline Python tests and 15 documentation examples passed; compile/random baseline running. Three audit tasks dispatched with disjoint scopes.
- Final matrix: GCC/Clang C++20/23 compile/random passed; release double-include 76/76 passed; 9 randomized programs under ASan/UBSan passed. Whole-change source and rendered-site review approved after resolving memory, release-warning and documentation findings.
- Official campaign: the original 1500-second sweep passed 28 drivers and left one unrun (exit 1). Resuming that driver passed; all 29 drivers / 624 cases ran three times. Original and resumed records retained separately. Metrics-inclusive site check passed; see docs/development/contest-readiness-2026-09-16.md for provenance and measured tradeoffs. The final commit is also gated by GitHub CI before merge.
