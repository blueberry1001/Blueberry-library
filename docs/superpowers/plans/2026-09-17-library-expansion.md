# Library Expansion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task.

**Goal:** Add useful verified libraries, operation-based discovery, and a complete reproducible Library Checker coverage checklist.

**Architecture:** Independent C++ headers with separate tests/docs; static operation metadata and small browser filters; official problem inventory with derived repository coverage.

**Tech Stack:** C++20, ACL modint, Python 3.12, Jekyll/Liquid, vanilla JavaScript.

**Spec:** docs/superpowers/specs/2026-09-17-library-expansion-design.md

## Global Constraints

- Read IMPLEMENTATION_POLICY.md and AGENTS.md. Preserve existing APIs and distinguish ACL from local implementations.
- All intervals are half-open and all new public APIs have operation details, complexity and executable examples.
- Work only in this isolated worktree. Parent integrates all.hpp, libraries.yml, README and checklist/build wiring.
- Do not run concurrent timing comparisons; keep failed/missing evidence explicit.

### Task 1: Data structures

Files: blueberry/data-structure/{segment-tree-beats,binary-trie,aggregate-queue,aggregate-deque}.hpp, matching docs and verify drivers, tests/random/operation-structures.cpp.

- [ ] Write brute-force tests for empty/singleton, duplicates, noncommutative matrix composition, mixed range clamps/adds and extreme documented values; show missing-header failure.
- [ ] Implement concise APIs: Beats prod/chmin/chmax/add; Trie insert/erase/xor_min/kth/rank; Queue/Deque push/pop/prod/size/empty. Final signatures and assumptions are recorded in each header's docs.
- [ ] Add official range_chmin_chmax_add_range_sum, set_xor_min, queue_operate_all_composite, deque_operate_all_composite drivers.
- [ ] Compile and run fixed-seed tests with GCC/Clang, verify official cases, inspect Fastest approaches and record comparisons.
- [ ] Review memory ownership, copy/move invariants, overflow and amortized bounds.

### Task 2: Math

Files: blueberry/math/{bitwise-convolution,subset-convolution,matrix}.hpp, matching docs/verify, tests/random/algebra-expansion.cpp.

- [ ] Write small O(N²) convolution and exhaustive small-field matrix oracle tests, including zero dimensions, singular matrices and inconsistent systems; show missing-header failure.
- [ ] Implement bitwise transforms and convolution, ranked subset convolution, and field elimination returning explicit singular/inconsistent results.
- [ ] Add official bitwise/subset convolution, determinant/inverse/rank/system_of_linear_equations drivers as supported by the official inventory.
- [ ] Test all APIs and bounds; research primary/Fastest implementations, compare equivalent algorithms using the same environment.
- [ ] Review field/invertibility assumptions, empty results, memory and true asymptotic complexity.

### Task 3: Operation finder

Files: .verify-helper/docs/static/{operations.md,_data/operations.yml,assets/js/operations.js,assets/css/operations.css}, tests/test_operations.py.

- [ ] Define metadata records with library path, desired query/update/conditions, displayed operation and complexity. Include existing structures and explicit ACL choices.
- [ ] Test metadata validity and filtering combinations, including no matches and reset, before implementing behavior.
- [ ] Build an accessible static page, with filters ANDed across dimensions and clear prerequisites. No unsupported capability claims or hidden unimplemented entries.
- [ ] Verify keyboard use, state reset, direct links, mobile width and no console errors in the browser.

### Task 4: Official inventory and checklist

Files: scripts/library_checker_coverage.py, data/library-checker-problems.json, data/library-checker-mappings.yml, .verify-helper/docs/static/library-checker.md, tests/test_library_checker_coverage.py.

- [ ] Snapshot official published problems and categories with upstream revision/date and unique IDs; exclude generator test fixtures.
- [ ] Unit-test duplicate/unknown IDs, multiple verify drivers per problem, indirect includes, ignored drivers, mappings and missing evidence.
- [ ] Generate coverage rows from PROBLEM URLs and documented mappings; distinguish implementation, driver and successful run evidence.
- [ ] Provide refresh and --check commands, a searchable/filterable static checklist, and links to problem/implementation/verify.

### Task 5: Integration and release

- [ ] Register headers and finder entries, wire navigation and checks, run make test, all compiler matrices, standalone headers, official cases and docs.
- [ ] Review each domain independently and address findings. Run final whole-branch review.
- [ ] Commit logical units, create PR, require green CI, merge, verify public Pages and clean Git state.

## Execution ledger

- Ruling: execute autonomously using existing user authorization; no routine design approval pause.
- Ruling: independent worktree under ignored .build/worktrees protects the ongoing legacy verification and PR.
