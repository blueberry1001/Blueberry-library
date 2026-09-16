# Library Checker expansion implementation plan

> **For agentic workers:** Use superpowers:subagent-driven-development. Independent domain owners implement and self-review; a different owner reviews each result before the final whole-branch review.

**Goal:** Add twelve useful non-ACL headers verified against Library Checker, with contest-ready APIs and measured implementation choices.

**Architecture:** Independent headers in existing categories; preserve all existing APIs and publishing. Parent alone updates shared catalog, umbrella, README and aggregate reports.

**Tech Stack:** C++20, ACL static_modint/convolution, fixed-seed C++ random tests, Python, Library Checker, Jekyll.

**Spec:** docs/superpowers/specs/2026-09-17-library-checker-expansion-design.md

## Global constraints

- Header-only, blueberry namespace, zero-based indices and half-open ranges except documented Eertree roots -1/0.
- Never overwrite another owner's files or change shared catalog/all.hpp concurrently.
- Complete operation-level documentation and minimal executable examples for every public API.
- No third-party source copied into commits; source hashes and analysis may be committed.
- Preserve failed measurements; compare only equal inputs/compiler/options/environment.
- Autonomous execution and Git integration are authorized by the user. Use codex/library-checker-expansion, not direct main commits.

## Task 1: Mathematical building blocks — audit_libraries

Files: new blueberry/math/{factorize,mod-sqrt,linear-recurrence}.hpp, matching docs/math/*.md, tests/random/math-expansion.cpp, verify/math problem drivers, domain benchmark/research report.

Interfaces: is_prime(uint64_t), factorize(uint64_t); mod_sqrt(int64_t,int64_t) returns optional root; berlekamp_massey(vector<Mint>); linear_recurrence_kth(initial,coeff,uint64_t) with a[i]=sum(coeff[j]*a[i-j-1]).

- [x] Write independent trial-division/brute square-root/naive recurrence tests. Include factorize(1)=={}, prime near uint64 max, mod_sqrt(0,2)==0, no root, empty/all-zero BM and huge kth for known recurrences.
- [x] Compile before headers exist and preserve the expected missing-header failure.
- [x] Implement deterministic Miller–Rabin and Pollard–Brent, Tonelli–Shanks, BM and Bostan–Mori; check all multiplication and NTT bounds.
- [x] Run primality_test, factorize, sqrt_mod, find_linear_recurrence and kth_term_of_linearly_recurrent_sequence official drivers, random tests and comparison benchmarks.
- [x] Complete docs and independent review; report commands and logs to parent without committing shared files.

## Task 2: Stateful data structures — coverage_gaps

Files: new blueberry/data-structure/{potential-union-find,ordered-set,persistent-segment-tree}.hpp, matching docs, tests/random/data-structure-expansion.cpp, official verify drivers, domain benchmark/research report.

Interfaces: potential relation merge/diff with contradiction result; OrderedSet insert/erase/contains/rank/kth plus short size/empty; PersistentSegmentTree set(version,p,value) returns a new version and prod(version,l,r) preserves previous versions. Owner chooses precise templates following existing API style and records them in docs.

- [x] Write independent explicit-constraint graph propagation tests (including noncommutative matrices), std::set/sorted-vector comparisons, copied-array branching-version tests.
- [x] Observe missing-header failure before implementation.
- [x] Implement path compression with group order preserved; pooled ordered tree with safe copy/erase; index-pool persistent monoid tree avoiding references across allocation.
- [x] Verify unionfind_with_potential and matrix variant, ordered_set, rectangle_sum through persistent x-prefix versions. Do not claim persistent_range_affine_range_sum coverage.
- [x] Benchmark online tree alternatives and allocation/layout choices under equal conditions; complete docs and independent review.

## Task 3: Graph structures — site_usability

Files: new blueberry/graph/{hopcroft-karp,biconnected-components,eulerian-trail}.hpp, matching docs, tests/random/graph-expansion.cpp, official drivers, domain benchmark/research report.

Interfaces: HopcroftKarp(L,R,edges), size/left_match/right_match/pairs/min_vertex_cover; BiconnectedComponents(n,edges), groups/block_cut_tree; EulerianTrail<Directed>(n,edges,start=-1), exists/vertices/edges.

- [x] Brute-force maximum matching/min cover, vertex-removal block checks, exhaustive tiny edge-permutation trails; include loops, parallel edges, isolated vertices, empty graphs, disconnected edge sets and explicit invalid starts.
- [x] Observe preimplementation missing-header failure.
- [x] Implement iterative layered augmenting paths, iterative Tarjan edge stack and iterative Hierholzer. Define multigraph and isolated-vertex behavior precisely.
- [x] Run bipartitematching/min vertex cover, biconnected_components, both eulerian_trail variants; check long paths and large stars.
- [x] Measure comparison candidates, document all operations and complete independent review.

## Task 4: String structures — parent

Files: new blueberry/string/{aho-corasick,eertree,lyndon-factorization}.hpp, matching docs/string/*.md, tests/random/string-expansion.cpp, verify/string/{aho-corasick,eertree,lyndon-factorization}.test.cpp, benchmark/string-expansion.cpp.

Interfaces: AhoCorasick<26,'a'>(reserve=0), add(string_view)->node, build(), size(), parent(v), link(v), next(v,char), count(text)->per-node occurrences. Eertree<26,'a'>(reserve=0) or (string_view), add(char)->suffix id, size()->distinct palindrome count, suffix(), length(v), parent(v), link(v), count()->counts indexed 0..size. lyndon_factorization(sequence)->vector<int> boundaries.

- [x] Write brute substring occurrence, palindrome enumeration/node-id/link and exhaustive small Lyndon decomposition tests. Assert empty cases, duplicate patterns, custom alphabets, long repeated strings and repeated counts followed by extension.
- [x] Compile and record missing-header failure, then implement fixed-array Aho transitions, online suffix-link Eertree and Duval.
- [x] Execute own random tests and the three official drivers. Compare fixed transitions against an independently implemented sparse candidate on repeated/random/periodic inputs.
- [x] Complete operation docs; delegate string review to a different domain owner after that owner finishes.

## Task 5: Integration and release — parent

- [x] Register all new headers in libraries.yml and all.hpp; update README/categories and problem-to-library guide.
- [x] Preserve Fastest metadata and write source-linked observations, benchmark environment/raw runs and current coverage report.
- [x] Run make test, GCC/Clang C++20/23, release include checks, ASan/UBSan on new random suites, make docs, Jekyll and preflight check_site.py.
- [x] Resolve task reviews; inspect entire diff and request independent branch review.
- Release gate (tracked by PR checks and verification-metrics artifacts): complete make verify for all 46 drivers with three repetitions, publish measurements, and run full check_site.py.
- Integration gate (tracked by GitHub): commit coherent domain units, push, create PR with evidence, wait for CI, merge and verify Pages deployment.

## Decisions and evidence

- Baseline branch main 42781ad, clean worktree; origin/main identical, no open PRs.
- Fresh baseline make check passed: 24 Python tests, 17 docs/18 examples, 19 headers, 29 verify sources and 9 random programs ×20 seeds.
- Fastest string research: .build/lc-expansion-string, upper two sources fetched per problem, copied code excluded from tracked output.
