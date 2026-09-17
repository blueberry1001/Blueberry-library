# Library expansion and operation finder

## Goal

Prioritize useful non-ACL algorithms, with complete APIs and official verification. Make the catalogue navigable by desired operations and maintain a reproducible checklist of all official Library Checker problems.

## Deliverables

1. Data structures: Segment Tree Beats (range chmin/chmax/add/sum/min/max), Binary Trie (duplicates, erase, XOR minimum, rank/kth), and monoid aggregate Queue/Deque. Preserve noncommutative aggregation order and document amortized bounds.
2. Math: AND/OR/XOR convolution, subset convolution, and field-based matrix elimination (determinant, inverse, rank, linear system). Reuse ACL modint; do not recreate ACL convolution.
3. An operation finder alongside the existing catalogue. Filters express query, update, and input conditions. Results show suitable operations, complexity, prerequisites, and direct documentation links. ACL recommendations are clearly external, not counted as Blueberry libraries. Plain links remain useful without JavaScript.
4. A full Library Checker checklist generated from a dated official upstream snapshot, with problem IDs/titles/categories and links. Distinguish direct verify coverage, implemented API without a driver, ACL recommendation, and missing implementation. A driver does not itself certify a successful run. Reflect current verification evidence separately.

## Constraints

- C++20, header-only, namespace blueberry, zero-based half-open intervals.
- Existing public APIs stay compatible. Every new API uses templates/library.md.example with all operations and executable assertions.
- Fixed-seed brute-force tests, official verify drivers, GCC/Clang C++20/23, standalone includes, make test/verify/docs.
- Primary-source/Fastest research informs algorithm choices; no unlicensed code copying or cross-environment speed claims.
- Keep GitHub Pages and existing layout. Keyboard-accessible filters, reset, empty state, mobile layout, no new frontend runtime dependency.
- Previous PR #16 remains isolated until its checks and deployment finish. This worktree begins at its implementation head to use the replacements.

## Data flow and ownership

Library metadata remains libraries.yml. Finder operation metadata is separate and references existing header paths. Official problem inventory is versioned and refreshed explicitly; local coverage is derived from PROBLEM definitions plus reviewed mappings, not guessed from names. Build-time checks detect stale paths, unknown IDs and inconsistent counts.

## Decisions

- Deliver a finite substantial batch rather than promise all Library Checker algorithms in one pass; the checklist provides the remaining backlog.
- Use ordinary independent headers rather than a single abstraction spanning unrelated structures.
- Full official timings run in quiet slots. Parallel development tests are correctness evidence, not comparable benchmark measurements.
